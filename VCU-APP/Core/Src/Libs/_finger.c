/*
 * _finger.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_finger.h"
#include "DMA/_dma_finger.h"
#include "usart.h"


/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t FingerRecMutexHandle;
#endif

/* Private variables
 * ----------------------------------------------------------*/
static finger_t finger = {.puart = &huart4, .pdma = &hdma_uart4_rx};

/* Private functions
 * ----------------------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t Scan(uint8_t id, uint8_t slot, uint32_t timeout);
static uint8_t GenerateID(uint8_t *theId);
static uint8_t ConvertImage(uint8_t slot);
static uint8_t GetImage(void);
static void DebugResponse(uint8_t res, char *msg);

/* Public functions implementation
 * --------------------------------------------*/
void FINGER_Init(void) {
  uint8_t verified = 0;

  // Initiate Module
  do {
    printf("Finger:Init\n");

    // control
    //	  HAL_UART_Init(huart);
    MX_UART4_Init();
    FINGER_DMA_Start(finger.puart, finger.pdma);
    GATE_FingerReset();

    // verify password and check hardware
    lock();
    verified = fz3387_verifyPassword();
    unlock();

    _DelayMS(500);
  } while (!verified);
}

void FINGER_DeInit(void) {
  GATE_FingerShutdown();
  FINGER_DMA_Stop();
  HAL_UART_DeInit(finger.puart);
}

uint8_t FINGER_Fetch(uint8_t *db) {
  uint16_t templateCount;
  uint8_t res;

  lock();
  res = fz3387_getTemplateCount(&templateCount);
  for (uint8_t id = 1; id <= FINGER_USER_MAX; id++)
    *(db++) = (fz3387_loadModel(id) == FINGERPRINT_OK);
  unlock();

  return res == FINGERPRINT_OK;
}

uint8_t FINGER_Enroll(uint8_t *id, uint8_t *valid) {
  uint8_t res;

  lock();
  res = GenerateID(id);

  *valid = (res == FINGERPRINT_OK && *id > 0);

  if (*valid) {
    GATE_LedWrite(1);
    *valid = Scan(*id, 1, 5000);
  }

  if (*valid) {
    GATE_LedWrite(0);
    while (fz3387_getImage() != FINGERPRINT_NOFINGER) {
      _DelayMS(50);
    }

    GATE_LedWrite(1);
    *valid = Scan(*id, 2, 5000);
  }
  GATE_LedWrite(0);

  if (*valid) {
    printf("Finger:Creating model for #%u\n", *id);
    res = fz3387_createModel();
    DebugResponse(res, "Prints matched!");
    *valid = (res == FINGERPRINT_OK);
  }

  if (*valid) {
    printf("Finger:ID #%u\n", *id);
    res = fz3387_storeModel(*id);
    DebugResponse(res, "Stored!");
    *valid = (res == FINGERPRINT_OK);
  }
  unlock();

  if (*valid) {
    GATE_LedBlink(200);
    _DelayMS(100);
    GATE_LedBlink(200);
  }

  return (res == FINGERPRINT_OK);
}

uint8_t FINGER_DeleteID(uint8_t id) {
  uint8_t res;

  lock();
  res = fz3387_deleteModel(id);
  DebugResponse(res, "Deleted!");
  unlock();

  return (res == FINGERPRINT_OK);
}

uint8_t FINGER_Flush(void) {
  uint8_t res;

  lock();
  res = fz3387_emptyDatabase();
  DebugResponse(res, "Flushed!");
  unlock();

  return (res == FINGERPRINT_OK);
}

uint8_t FINGER_SetPassword(uint32_t password) {
  uint8_t res;

  lock();
  res = fz3387_setPassword(password);
  DebugResponse(res, "Password applied!");
  unlock();

  return (res == FINGERPRINT_OK);
}

uint8_t FINGER_Auth(void) {
  uint16_t id, confidence;
  uint8_t res, theId = 0;
  uint8_t valid;

  lock();
  valid = (GetImage() == FINGERPRINT_OK);

  if (valid)
    valid = (ConvertImage(1));

  if (valid) {
    res = fz3387_fingerFastSearch(&id, &confidence);
    DebugResponse(res, "Found a print match!");
    valid = (res == FINGERPRINT_OK);
  }

  if (valid) {
    printf("Finger:Found ID #%u with confidence of %u\n", id, confidence);
    if (confidence > FINGER_CONFIDENCE_MIN)
      theId = id;
  }

  unlock();

  return theId;
}

uint8_t FINGER_AuthFast(void) {
  uint16_t id, confidence;
  uint8_t theId = 0;

  lock();
  if (fz3387_getImage() == FINGERPRINT_OK)
    if (fz3387_image2Tz(1) == FINGERPRINT_OK)
      if (fz3387_fingerFastSearch(&id, &confidence) == FINGERPRINT_OK)
        if (confidence > FINGER_CONFIDENCE_MIN)
          theId = id;
  unlock();

  return theId;
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
  osMutexAcquire(FingerRecMutexHandle, osWaitForever);
#endif
  GATE_FingerDigitalPower(GPIO_PIN_SET);
}

static void unlock(void) {
  GATE_FingerDigitalPower(GPIO_PIN_RESET);
#if (RTOS_ENABLE)
  osMutexRelease(FingerRecMutexHandle);
#endif
}

static uint8_t Scan(uint8_t id, uint8_t slot, uint32_t timeout) {
  TickType_t tick = _GetTickMS();
  uint8_t valid = 0;

  printf("Finger:Waiting for valid finger to enroll as #%u\n", id);

  do {
    valid = (GetImage() == FINGERPRINT_OK);
    _DelayMS(10);
  } while (!valid && (_GetTickMS() - tick) < timeout);

  if (valid)
    valid = ConvertImage(slot);

  return valid;
}

static uint8_t GetImage(void) {
  uint8_t res;

  res = fz3387_getImage();

  if (res == FINGERPRINT_NOFINGER)
    printf(".\n");
  else
    DebugResponse(res, "Image taken");

  return res;
}

static uint8_t ConvertImage(uint8_t slot) {
  uint8_t res;

  res = fz3387_image2Tz(slot);
  DebugResponse(res, "Image converted");

  return (res == FINGERPRINT_OK);
}

static uint8_t GenerateID(uint8_t *theId) {
  uint16_t templateCount;
  uint8_t res;

  *theId = 0;

  res = fz3387_getTemplateCount(&templateCount);
  if (res == FINGERPRINT_OK) {
    printf("Finger:TemplateCount = %u\n", templateCount);

    if (templateCount <= FINGER_USER_MAX) {
      for (uint8_t id = 1; id <= FINGER_USER_MAX; id++)
        if (fz3387_loadModel(id) != FINGERPRINT_OK) {
          *theId = id;
          break;
        }
    }
  }

  return res;
}

static void DebugResponse(uint8_t res, char *msg) {
  switch (res) {
  case FINGERPRINT_OK:
    printf("Finger:%s\n", msg);
    break;
  case FINGERPRINT_NOFINGER:
    printf("Finger:No finger detected\n");
    break;
  case FINGERPRINT_NOTFOUND:
    printf("Finger:Did not find a match\n");
    break;
  case FINGERPRINT_PACKETRECIEVEERR:
    printf("Finger:Communication error\n");
    break;
  case FINGERPRINT_ENROLLMISMATCH:
    printf("Finger:Fingerprints did not match\n");
    break;
  case FINGERPRINT_IMAGEMESS:
    printf("Finger:Image too messy\n");
    break;
  case FINGERPRINT_IMAGEFAIL:
    printf("Finger:Imaging error\n");
    break;
  case FINGERPRINT_FEATUREFAIL:
    printf("Finger:Could not find finger print features\n");
    break;
  case FINGERPRINT_INVALIDIMAGE:
    printf("Finger:Could not find finger print features\n");
    break;
  case FINGERPRINT_BADLOCATION:
    printf("Finger:Could not execute in that location\n");
    break;
  case FINGERPRINT_FLASHERR:
    printf("Finger:Error writing to flash\n");
    break;
  default:
    printf("Finger:Unknown error: 0x%02X\n", res);
    break;
  }
}
