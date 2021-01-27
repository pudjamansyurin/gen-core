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

/* Private variables ----------------------------------------------------------*/
static finger_t finger;

/* Private functions ----------------------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint8_t ConvertImage(void);
static uint8_t GetImage(uint8_t enroll);
static void DebugResponse(uint8_t res, char *msg);

/* Public functions implementation --------------------------------------------*/
void FINGER_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
  uint8_t verified = 0;

  finger.h.uart = huart;
  finger.h.dma = hdma;
  fz3387_init(&(finger.scanner));

  // Initiate Module
  do {
    printf("Finger:Init\n");

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
  uint8_t res, error = 0;
  TickType_t tick;

  lock();
  res = fz3387_getTemplateCount();
  DebugResponse(res, "Retrieve OK");
  printf("Finger:TemplateCount = %u\n", finger.scanner.templateCount);

  if (res == FINGERPRINT_OK && finger.scanner.templateCount < FINGER_USER_MAX) {
    printf("Finger:Waiting for valid finger to enroll as #%u\n", id);

    tick = _GetTickMS();
    do {
      error = !GetImage(1);
      _DelayMS(10);
    } while (error && (_GetTickMS() - tick) < FINGER_SCAN_TIMEOUT);

    if (!error)
      if (ConvertImage()) {
        printf("Finger:Creating model for #%u\n", id);

        res = fz3387_createModel();
        DebugResponse(res, "Prints matched!");

        if (res == FINGERPRINT_OK) {
          printf("Finger:ID #%u\n", id);

          res = fz3387_storeModel(id);
          DebugResponse(res, "Stored!");
        }
      }
  }
  unlock();

  return (!error && res == FINGERPRINT_OK);
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
  uint8_t res;
  int8_t id = -1;

  lock();
  if (GetImage(0))
    if (ConvertImage()) {
      res = fz3387_fingerFastSearch();
      DebugResponse(res, "Found a print match!");
      if (res == FINGERPRINT_OK) {
        printf("Finger:Found ID #%u with confidence of %u\n",
            finger.scanner.id, finger.scanner.confidence);

        if (finger.scanner.confidence > FINGER_CONFIDENCE_MIN)
          id = finger.scanner.id;
      }
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
        if (finger.scanner.confidence >= FINGER_CONFIDENCE_MIN)
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

static uint8_t GetImage(uint8_t enroll) {
  uint8_t res;

  res = fz3387_getImage();

  if (enroll && res == FINGERPRINT_NOFINGER)
    printf(".\n");
  else
    DebugResponse(res, "Image taken");

  return (res == FINGERPRINT_OK);
}

static uint8_t ConvertImage(void) {
  uint8_t res;

  res = fz3387_image2Tz(1);
  DebugResponse(res, "Image converted");

  return (res == FINGERPRINT_OK);
}

static void DebugResponse(uint8_t res, char *msg) {
  switch (res) {
    case FINGERPRINT_OK:
      printf("Finger:%s\n", msg);
      break;
    case FINGERPRINT_NOFINGER:
      printf("Finger:No finger detected\n");
      break;
    case FINGERPRINT_NOTFOUND :
      printf("Finger:Did not find a match\n");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      printf("Finger:Communication error\n");
      break;
    case FINGERPRINT_ENROLLMISMATCH :
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
    case FINGERPRINT_BADLOCATION :
      printf("Finger:Could not execute in that location\n");
      break;
    case FINGERPRINT_FLASHERR :
      printf("Finger:Error writing to flash\n");
      break;
    default:
      printf("Finger:Unknown error: 0x%02X\n", res);
      break;
  }
}
