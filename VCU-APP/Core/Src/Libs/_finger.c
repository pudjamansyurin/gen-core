/*
 * _finger.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "DMA/_dma_finger.h"
#include "Libs/_finger.h"

/* External variables ---------------------------------------------------------*/
//extern osMutexId_t FingerRecMutexHandle;

/* Private variables ----------------------------------------------------------*/
static finger_t finger;

/* Private functions ----------------------------------------------------------*/
static void lock(void);
static void unlock(void);
static void ConvertImage(uint8_t *error);
static void GetImage(uint8_t *error, uint8_t enroll);
static void DebugResponse(int8_t res, char *msg);

/* Public functions implementation --------------------------------------------*/
void FINGER_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
  uint8_t verified = 0;

  finger.h.uart = huart;
  finger.h.dma = hdma;
  fz3387_init(&(finger.scanner));

  // Inititalize Module
  do {
    Log("Finger:Init\n");

    // control
    //	  HAL_UART_Init(huart);
    MX_UART4_Init();
    FINGER_DMA_Start(huart, hdma);
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
  HAL_UART_DeInit(finger.h.uart);
}

uint8_t FINGER_Enroll(uint8_t id) {
  const TickType_t scan_time = (FINGER_SCAN_TIMEOUT);
  uint8_t res, timeout, error = 0;
  TickType_t tick;

  lock();
  if (!error) {
    res = fz3387_getTemplateCount();

    DebugResponse(res, "Retrieve OK");
    Log("Finger:TemplateCount = %u\n", finger.scanner.templateCount);

    error = (res != FINGERPRINT_OK) || (finger.scanner.templateCount >= FINGER_USER_MAX);
  }

  if (!error) {
    Log("Finger:Waiting for valid finger to enroll as #%u\n", id);

    tick = _GetTickMS();
    do {
      timeout = ((_GetTickMS() - tick) > scan_time);

      GetImage(&error, 1);
    } while (error && !timeout);

    error = (res != FINGERPRINT_OK) || timeout;
  }

  if (!error)
    ConvertImage(&error);

  if (!error) {
    Log("Finger:Creating model for #%u\n", id);

    res = fz3387_createModel();
    DebugResponse(res, "Prints matched!");

    error = (res != FINGERPRINT_OK);
  }

  if (!error) {
    Log("\nFinger:ID #%u\n", id);

    res = fz3387_storeModel(id);
    DebugResponse(res, "Stored!");

    error = (res != FINGERPRINT_OK);
  }

  unlock();

  return !error;
}

uint8_t FINGER_DeleteID(uint8_t id) {
  uint8_t res;

  lock();
  res = fz3387_deleteModel(id);
  DebugResponse(res, "Deleted!");
  unlock();

  return (res == FINGERPRINT_OK);
}

uint8_t FINGER_EmptyDatabase(void) {
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

int8_t FINGER_Auth(void) {
  uint8_t res, error = 0;
  int8_t id = -1;

  lock();
  if (!error)
    GetImage(&error, 0);

  if (!error)
    ConvertImage(&error);

  if (!error) {
    res = fz3387_fingerFastSearch();
    DebugResponse(res, "Found a print match!");

    error = (res != FINGERPRINT_OK);
  }

  if (!error) {
    Log("\nFinger:Found ID #%u with confidence of %u\n", 
        finger.scanner.id, finger.scanner.confidence);

    if (finger.scanner.confidence > FINGER_CONFIDENCE_MIN)
      id = finger.scanner.id;
  }
  unlock();

  return id;
}

int8_t FINGER_AuthFast(void) {
  int8_t id = -1;

  lock();
  if (fz3387_getImage() == FINGERPRINT_OK)
    if (fz3387_image2Tz(1) == FINGERPRINT_OK)
      if (fz3387_fingerFastSearch() == FINGERPRINT_OK)
        if (finger.scanner.confidence > FINGER_CONFIDENCE_MIN)
          id = finger.scanner.id;
  unlock();

  return id;
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
  //	osMutexAcquire(FingerRecMutexHandle, osWaitForever);
  GATE_FingerDigitalPower(GPIO_PIN_SET);
}

static void unlock(void) {
  GATE_FingerDigitalPower(GPIO_PIN_RESET);
  //	osMutexRelease(FingerRecMutexHandle);
}

static void GetImage(uint8_t *error, uint8_t enroll) {
  uint8_t res;

  res = fz3387_getImage();

  if (enroll && res == FINGERPRINT_NOFINGER)
    Log(".\n");
  else 
    DebugResponse(res, "Image taken");

  *error = (res != FINGERPRINT_OK);
}

static void ConvertImage(uint8_t *error) {
  uint8_t res;

  res = fz3387_image2Tz(1);
  DebugResponse(res,"Image converted");

  *error = (res != FINGERPRINT_OK);
}

static void DebugResponse(int8_t res, char *msg) {
  switch (res) {
    case FINGERPRINT_OK:
      Log("Finger:%s\n", msg);
      break;
    case FINGERPRINT_NOFINGER:
      Log("Finger:No finger detected\n");
      break;
    case FINGERPRINT_NOTFOUND :
      Log("Finger:Did not find a match\n");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Log("Finger:Communication error\n");
      break;
    case FINGERPRINT_ENROLLMISMATCH :
      Log("Finger:Fingerprints did not match\n");
      break;
    case FINGERPRINT_IMAGEMESS:
      Log("Finger:Image too messy\n");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Log("Finger:Imaging error\n");
      break;
    case FINGERPRINT_FEATUREFAIL:
      Log("Finger:Could not find finger print features\n");
      break;
    case FINGERPRINT_INVALIDIMAGE:
      Log("Finger:Could not find finger print features\n");
      break;
    case FINGERPRINT_BADLOCATION :
      Log("Finger:Could not execute in that location\n");
      break;
    case FINGERPRINT_FLASHERR :
      Log("Finger:Error writing to flash\n");
      break;
    default:
      Log("Finger:Unknown error: 0x%02X\n", res);
      break;
  }
}
