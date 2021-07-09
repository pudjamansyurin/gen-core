/*
 * _finger.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_finger.h"

#include "DMA/_dma_finger.h"
#include "Drivers/_r307.h"
#include "usart.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t FingerRecMutexHandle;
#endif

/* Private constants
 * --------------------------------------------*/
#define FINGER_TIMEOUT_MS ((uint16_t)5000)
#define FINGER_CONFIDENCE_MIN_PERCENT ((uint8_t)75)
#define FINGER_USER_MAX ((uint8_t)5)

/* Private enums
 * --------------------------------------------*/
typedef enum {
  FGR_REG_HIDE = 0,
  FGR_REG_SHOW,
} FGR_REG;

/* Private types
 * --------------------------------------------*/
typedef struct {
  finger_data_t d;
  finger_db_t db;
  UART_HandleTypeDef *puart;
  DMA_HandleTypeDef *pdma;
} finger_t;

/* Private variables
 * --------------------------------------------*/
static finger_t FGR = {.d = {0}, .puart = &huart4, .pdma = &hdma_uart4_rx};

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
static void IndicatorShow(uint8_t ok);
static void IndicatorHide(void);
static uint8_t AuthFast(void);
static uint8_t GenerateID(uint8_t *theId);
static uint8_t ConvertImage(uint8_t slot);
static uint8_t GetImage(uint32_t timeout);
static void DebugResponse(uint8_t res, const char *msg);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t FGR_Init(void) {
  uint8_t ok;
  uint32_t tick;

  lock();
  printf("FGR:Init\n");

  tick = tickMs();
  do {
    MX_UART4_Init();
    FINGER_DMA_Start(FGR.puart, FGR.pdma);
    GATE_FingerReset();

    ok = FGR_Probe();
    if (!ok) delayMs(500);
  } while (!ok && tickIn(tick, FINGER_TIMEOUT_MS));

  FGR.d.verified = ok;
  unlock();

  printf("FGR:%s\n", ok ? "OK" : "Error");
  return ok;
}

void FGR_DeInit(void) {
  lock();
  FGR_Flush();
  GATE_FingerShutdown();
  FINGER_DMA_Stop();
  HAL_UART_DeInit(FGR.puart);
  unlock();
}

uint8_t FGR_Probe(void) {
  uint8_t ok;

  lock();
  GATE_FingerChipPower(1);
  ok = R307_checkPassword() == FP_OK;
  GATE_FingerChipPower(0);
  unlock();

  return ok;
}

void FGR_Verify(void) {
  if (FGR.d.verified) return;

  lock();
  FGR.d.verified = FGR_Probe();
  if (!FGR.d.verified) {
    FGR_DeInit();
    delayMs(500);
    FGR_Init();
  }
  unlock();
}

void FGR_Flush(void) {
  lock();
  memset(&(FGR.d), 0, sizeof(finger_data_t));
  unlock();
}

uint8_t FGR_Fetch(void) {
  uint16_t templateCount;
  uint8_t res;

  lock();
  res = R307_getTemplateCount(&templateCount);
  for (uint8_t id = 1; id <= FINGER_USER_MAX; id++)
    FGR.db[id - 1] = (R307_loadModel(id) == FP_OK);
  unlock();

  return res == FP_OK;
}

uint8_t FGR_Enroll(uint8_t *id, uint8_t *ok) {
  uint8_t res;

  lock();
  res = GenerateID(id);

  *ok = (res == FP_OK && *id > 0);

  if (*ok) {
    IndicatorShow(1);
    printf("FGR:Waiting for valid finger to enroll as #%u\n", *id);
    *ok = GetImage(FINGER_SCAN_MS);
  }
  if (*ok) *ok = ConvertImage(1);

  if (*ok) {
    IndicatorHide();
    while (R307_getImage() != FP_NOFINGER) {
      delayMs(50);
    }

    IndicatorShow(1);
    printf("FGR:Waiting for valid finger to enroll as #%u\n", *id);
    *ok = GetImage(FINGER_SCAN_MS);
  }
  if (*ok) *ok = ConvertImage(2);

  IndicatorHide();
  if (*ok) {
    while (R307_getImage() != FP_NOFINGER) {
      delayMs(50);
    }

    printf("FGR:Creating model for #%u\n", *id);
    res = R307_createModel();
    DebugResponse(res, "Prints matched!");
    *ok = (res == FP_OK);
  }

  if (*ok) {
    printf("FGR:ID #%u\n", *id);
    res = R307_storeModel(*id);
    DebugResponse(res, "Stored!");
    *ok = (res == FP_OK);
  }
  unlock();

  IndicatorShow(*ok);
  delayMs(250);
  IndicatorHide();

  return (res == FP_OK);
}

uint8_t FGR_DeleteID(uint8_t id) {
  uint8_t res;

  lock();
  res = R307_deleteModel(id);
  DebugResponse(res, "Deleted!");
  unlock();

  return (res == FP_OK);
}

uint8_t FGR_ResetDB(void) {
  uint8_t res;

  lock();
  res = R307_emptyDatabase();
  DebugResponse(res, "Reseted!");
  unlock();

  return (res == FP_OK);
}

uint8_t FGR_SetPassword(uint32_t password) {
  uint8_t res;

  lock();
  res = R307_setPassword(password);
  DebugResponse(res, "Password applied!");
  unlock();

  return (res == FP_OK);
}

void FGR_Authenticate(void) {
  uint8_t id, ok;

  lock();
  if (GetImage(50)) {
    id = AuthFast();
    ok = id > 0;
    if (ok) FGR.d.id = FGR.d.id ? 0 : id;

    IndicatorShow(ok);
    delayMs(250);
    IndicatorHide();
  }
  unlock();
}

const finger_data_t *FGR_IO_Data(void) { return &(FGR.d); }

uint8_t FGR_IO_ID(void) { return FGR.d.id; }

void FGR_IO_ClearID(void) { FGR.d.id = 0; }

void FGR_IO_ClearDB(void) { memset(FGR.db, 0, FINGER_USER_MAX); }

uint8_t FGR_IO_DB(uint8_t idx) { return FGR.db[idx]; }

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(FingerRecMutexHandle, osWaitForever);
#endif
  //	GATE_FingerChipPower(1);
}

static void unlock(void) {
  //	GATE_FingerChipPower(0);
#if (APP)
  osMutexRelease(FingerRecMutexHandle);
#endif
}

static uint8_t AuthFast(void) {
  uint16_t id = 0, confidence;

  lock();
  if (R307_image2Tz(1) == FP_OK) {
    if (R307_fingerFastSearch(&id, &confidence) == FP_OK) {
      if (confidence < FINGER_CONFIDENCE_MIN_PERCENT) id = 0;
    } else
      id = 0;
  }
  unlock();

  return id;
}

static void IndicatorShow(uint8_t ok) {
  FGR.d.registering = (FGR_REG_SHOW & 0x01) | ((!ok & 0x01) << 1);
}

static void IndicatorHide(void) { FGR.d.registering = (FGR_REG_HIDE & 0x01); }

static uint8_t GetImage(uint32_t timeout) {
  uint8_t ok, res;
  uint32_t tick;

  tick = tickMs();
  do {
    res = R307_getImage();
    ok = res == FP_OK;

    if (res == FP_NOFINGER)
      printf(".\n");
    else
      DebugResponse(res, "Image taken");

    delayMs(50);
  } while (!ok && tickIn(tick, timeout));

  return ok;
}

static uint8_t ConvertImage(uint8_t slot) {
  uint8_t res;

  res = R307_image2Tz(slot);
  DebugResponse(res, "Image converted");

  return (res == FP_OK);
}

static uint8_t GenerateID(uint8_t *theId) {
  uint16_t templateCount;
  uint8_t res;

  *theId = 0;

  res = R307_getTemplateCount(&templateCount);
  if (res == FP_OK) {
    printf("FGR:TemplateCount = %u\n", templateCount);

    if (templateCount <= FINGER_USER_MAX) {
      for (uint8_t id = 1; id <= FINGER_USER_MAX; id++)
        if (R307_loadModel(id) != FP_OK) {
          *theId = id;
          break;
        }
    }
  }

  return res;
}

static void DebugResponse(uint8_t res, const char *msg) {
#if FINGER_DEBUG
  switch (res) {
    case FP_OK:
      printf("FGR:%s\n", msg);
      break;
    case FP_NOFINGER:
      printf("FGR:No finger detected\n");
      break;
    case FP_NOTFOUND:
      printf("FGR:Did not find a match\n");
      break;
    case FP_PACKETRECIEVEERR:
      printf("FGR:Communication error\n");
      break;
    case FP_ENROLLMISMATCH:
      printf("FGR:Fingerprints did not match\n");
      break;
    case FP_IMAGEMESS:
      printf("FGR:Image too messy\n");
      break;
    case FP_IMAGEFAIL:
      printf("FGR:Imaging error\n");
      break;
    case FP_FEATUREFAIL:
      printf("FGR:Could not find finger print features fail\n");
      break;
    case FP_INVALIDIMAGE:
      printf("FGR:Could not find finger print features invalid\n");
      break;
    case FP_BADLOCATION:
      printf("FGR:Could not execute in that location\n");
      break;
    case FP_FLASHERR:
      printf("FGR:Error writing to flash\n");
      break;
    default:
      printf("FGR:Unknown error: 0x%02X\n", res);
      FGR_Verify();
      break;
  }
#endif
}
