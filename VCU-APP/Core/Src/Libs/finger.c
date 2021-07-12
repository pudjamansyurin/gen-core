/*
 * finger.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/finger.h"

#include "DMA/dma_finger.h"
#include "Drivers/r307.h"
#include "usart.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t FingerRecMutexHandle;
#endif

/* Private constants
 * --------------------------------------------*/
#define FGR_TIMEOUT_MS ((uint16_t)5000)
#define FGR_CONFIDENCE_MIN ((uint8_t)75)
#define FGR_USER_MAX ((uint8_t)5)

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
static void Lock(void);
static void UnLock(void);
static void IndicatorShow(uint8_t ok);
static void IndicatorHide(void);
static uint8_t AuthFast(void);
static uint8_t GenerateID(uint8_t *theId);
static uint8_t ConvertImage(uint8_t slot);
static uint8_t GetImage(uint32_t timeout);
static void Debug(uint8_t res, const char *msg);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t FGR_Init(void) {
  uint8_t ok;
  uint32_t tick;

  Lock();
  printf("FGR:Init\n");

  tick = tickMs();
  do {
    MX_UART4_Init();
    FINGER_DMA_Start(FGR.puart, FGR.pdma);
    GATE_FingerReset();

    ok = FGR_Probe();
    if (!ok) delayMs(500);
  } while (!ok && tickIn(tick, FGR_TIMEOUT_MS));

  FGR.d.verified = ok;
  UnLock();

  printf("FGR:%s\n", ok ? "OK" : "Error");
  return ok;
}

void FGR_DeInit(void) {
  Lock();
  FGR_Flush();
  GATE_FingerShutdown();
  FINGER_DMA_Stop();
  HAL_UART_DeInit(FGR.puart);
  UnLock();
}

uint8_t FGR_Probe(void) {
  uint8_t ok;

  Lock();
  GATE_FingerChipPower(1);
  ok = R307_checkPassword() == FP_OK;
  GATE_FingerChipPower(0);
  UnLock();

  return ok;
}

void FGR_Verify(void) {
  if (FGR.d.verified) return;

  Lock();
  FGR.d.verified = FGR_Probe();
  if (!FGR.d.verified) {
    FGR_DeInit();
    delayMs(500);
    FGR_Init();
  }
  UnLock();
}

void FGR_Flush(void) {
  Lock();
  memset(&(FGR.d), 0, sizeof(finger_data_t));
  UnLock();
}

uint8_t FGR_Fetch(void) {
  uint16_t templateCount;
  uint8_t res;

  Lock();
  res = R307_getTemplateCount(&templateCount);
  for (uint8_t id = 1; id <= FGR_USER_MAX; id++)
    FGR.db[id - 1] = (R307_loadModel(id) == FP_OK);
  UnLock();

  return res == FP_OK;
}

uint8_t FGR_Enroll(uint8_t *id, uint8_t *ok) {
  uint8_t res;

  Lock();
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
    Debug(res, "Prints matched!");
    *ok = (res == FP_OK);
  }

  if (*ok) {
    printf("FGR:ID #%u\n", *id);
    res = R307_storeModel(*id);
    Debug(res, "Stored!");
    *ok = (res == FP_OK);
  }
  UnLock();

  IndicatorShow(*ok);
  delayMs(250);
  IndicatorHide();

  return (res == FP_OK);
}

uint8_t FGR_DeleteID(uint8_t id) {
  uint8_t res;

  Lock();
  res = R307_deleteModel(id);
  Debug(res, "Deleted!");
  UnLock();

  return (res == FP_OK);
}

uint8_t FGR_ResetDB(void) {
  uint8_t res;

  Lock();
  res = R307_emptyDatabase();
  Debug(res, "Reseted!");
  UnLock();

  return (res == FP_OK);
}

uint8_t FGR_SetPassword(uint32_t password) {
  uint8_t res;

  Lock();
  res = R307_setPassword(password);
  Debug(res, "Password applied!");
  UnLock();

  return (res == FP_OK);
}

void FGR_Authenticate(void) {
  uint8_t id, ok;

  Lock();
  if (GetImage(50)) {
    id = AuthFast();
    ok = id > 0;
    if (ok) FGR.d.id = FGR.d.id ? 0 : id;

    IndicatorShow(ok);
    delayMs(250);
    IndicatorHide();
  }
  UnLock();
}

const finger_data_t *FGR_IO_Data(void) { return &(FGR.d); }

uint8_t FGR_IO_ID(void) { return FGR.d.id; }

void FGR_IO_ClearID(void) { FGR.d.id = 0; }

void FGR_IO_ClearDB(void) { memset(FGR.db, 0, FGR_USER_MAX); }

uint8_t FGR_IO_DB(uint8_t idx) { return FGR.db[idx]; }

/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(FingerRecMutexHandle, osWaitForever);
#endif
  //	GATE_FingerChipPower(1);
}

static void UnLock(void) {
  //	GATE_FingerChipPower(0);
#if (APP)
  osMutexRelease(FingerRecMutexHandle);
#endif
}

static uint8_t AuthFast(void) {
  uint16_t id = 0, confidence;

  Lock();
  if (R307_image2Tz(1) == FP_OK) {
    if (R307_fingerFastSearch(&id, &confidence) == FP_OK) {
      if (confidence < FGR_CONFIDENCE_MIN) id = 0;
    } else
      id = 0;
  }
  UnLock();

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
      Debug(res, "Image taken");

    delayMs(50);
  } while (!ok && tickIn(tick, timeout));

  return ok;
}

static uint8_t ConvertImage(uint8_t slot) {
  uint8_t res;

  res = R307_image2Tz(slot);
  Debug(res, "Image converted");

  return (res == FP_OK);
}

static uint8_t GenerateID(uint8_t *theId) {
  uint16_t templateCount;
  uint8_t res;

  *theId = 0;

  res = R307_getTemplateCount(&templateCount);
  if (res == FP_OK) {
    printf("FGR:TemplateCount = %u\n", templateCount);

    if (templateCount <= FGR_USER_MAX) {
      for (uint8_t id = 1; id <= FGR_USER_MAX; id++)
        if (R307_loadModel(id) != FP_OK) {
          *theId = id;
          break;
        }
    }
  }

  return res;
}

static void Debug(uint8_t res, const char *msg) {
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
