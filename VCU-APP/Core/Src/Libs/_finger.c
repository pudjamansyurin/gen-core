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
  uint8_t timeout, error = 0;
  TickType_t tick;
  int p;

  lock();
  if (!error) {
    // check number of stored id
    p = fz3387_getTemplateCount();
    // check response
    switch (p) {
      case FINGERPRINT_OK:
        Log("Finger:Retrieve OK\n");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Log("Finger:Communication error\n");
        break;
      default:
        Log("Finger:Unknown error\n");
        break;
    }

    Log("Finger:TemplateCount = %u\n", finger.scanner.templateCount);

    error = (p != FINGERPRINT_OK) || (finger.scanner.templateCount >= FINGER_USER_MAX);
  }

  if (!error) {
    //  Take Image
    Log("\nFinger:Waiting for valid finger to enroll as #%u\n", id);

    // set timeout guard
    tick = _GetTickMS();
    do {
      // handle timeout
      timeout = ((_GetTickMS() - tick) > scan_time);

      // send command
      GATE_LedToggle();

      // check response
      GetImage(&error, 1);
    } while (error && !timeout);

    error = (p != FINGERPRINT_OK) || timeout;
  }

  if (!error)
    //	put image to buffer 1
    ConvertImage(&error);

  if (!error) {
    //	Create Register model
    Log("\nFinger:Creating model for #%u\n", id);

    // Compare model from buffer 1 & 2
    p = fz3387_createModel();
    if (p == FINGERPRINT_OK) 
      Log("Finger:Prints matched!\n");
    else if (p == FINGERPRINT_PACKETRECIEVEERR) 
      Log("Finger:Communication error\n");
    else if (p == FINGERPRINT_ENROLLMISMATCH) 
      Log("Finger:Fingerprints did not match\n");
    else 
      Log("Finger:Unknown error\n");

    error = (p != FINGERPRINT_OK);
  }

  if (!error) {
    // debug
    Log("\nFinger:ID #%u\n", id);

    //	Store in memory
    p = fz3387_storeModel(id);
    if (p == FINGERPRINT_OK) 
      Log("Finger:Stored!\n");
    else if (p == FINGERPRINT_PACKETRECIEVEERR) 
      Log("Finger:Communication error\n");
    else if (p == FINGERPRINT_BADLOCATION) 
      Log("Finger:Could not store in that location\n");
    else if (p == FINGERPRINT_FLASHERR) 
      Log("Finger:Error writing to flash\n");
    else 
      Log("Finger:Unknown error\n");

    error = (p != FINGERPRINT_OK);
  }

  unlock();

  return !error;
}

uint8_t FINGER_DeleteID(uint8_t id) {
  int8_t p;

  lock();
  p = fz3387_deleteModel(id);
  if (p == FINGERPRINT_OK) 
    Log("Finger:Deleted!\n");
  else if (p == FINGERPRINT_PACKETRECIEVEERR) 
    Log("Finger:Communication error\n");
  else if (p == FINGERPRINT_BADLOCATION) 
    Log("Finger:Could not delete in that location\n");
  else if (p == FINGERPRINT_FLASHERR) 
    Log("Finger:Error writing to flash\n");
  else {
    Log("\nFinger:Unknown error: 0x%02X\n", p);
  }
  unlock();

  return (p == FINGERPRINT_OK);
}

uint8_t FINGER_EmptyDatabase(void) {
  int8_t p;

  lock();
  p = fz3387_emptyDatabase();
  unlock();

  return (p == FINGERPRINT_OK);
}

uint8_t FINGER_SetPassword(uint32_t password) {
  int8_t p;

  lock();
  p = fz3387_setPassword(password);
  unlock();

  return (p == FINGERPRINT_OK);
}

int8_t FINGER_Auth(void) {
  int8_t p, id = -1;
  uint8_t error = 0;

  lock();
  if (!error)
    // scan the finger print
    GetImage(&error, 0);

  if (!error)
    // OK success!, convert the image taken
    ConvertImage(&error);

  if (!error) {
    // Find in the model
    p = fz3387_fingerFastSearch();
    if (p == FINGERPRINT_OK) 
      Log("Finger:Found a print match!\n");
    else if (p == FINGERPRINT_PACKETRECIEVEERR) 
      Log("Finger:Communication error\n");
    else if (p == FINGERPRINT_NOTFOUND) 
      Log("Finger:Did not find a match\n");
    else 
      Log("Finger:Unknown error\n");

    error = (p != FINGERPRINT_OK);
  }

  if (!error) {
    // found a match!
    Log("\nFinger:Found ID #%u with confidence of %u\n", 
      finger.scanner.id, finger.scanner.confidence);

    // compare the tolerance
    if (finger.scanner.confidence > FINGER_CONFIDENCE_MIN)
      id = finger.scanner.id;
  }
  unlock();

  return id;
}

int8_t FINGER_AuthFast(void) {
  int8_t p, id = -1;

  lock();

  // scan the finger print
  p = fz3387_getImage();

  // OK success!, convert the image taken
  if (p == FINGERPRINT_OK) 
    p = fz3387_image2Tz(1);

  // Find in the model
  if (p == FINGERPRINT_OK) 
    p = fz3387_fingerFastSearch();

  // found a match!
  if (p == FINGERPRINT_OK)
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
  int p;

  p = fz3387_getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Log("Finger:Image taken\n");
      break;
    case FINGERPRINT_NOFINGER:
      if (enroll)
        Log(".\n");
      else
        Log("Finger:No finger detected\n");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Log("Finger:Communication error\n");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Log("Finger:Imaging error\n");
      break;
    default:
      Log("Finger:Unknown error\n");
      break;
  }

  *error = (p != FINGERPRINT_OK);
}

static void ConvertImage(uint8_t *error) {
  int p;

  p = fz3387_image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Log("Finger:Image converted\n");
      break;
    case FINGERPRINT_IMAGEMESS:
      Log("Finger:Image too messy\n");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Log("Finger:Communication error\n");
      break;
    case FINGERPRINT_FEATUREFAIL:
      Log("Finger:Could not find finger print features\n");
      break;
    case FINGERPRINT_INVALIDIMAGE:
      Log("Finger:Could not find finger print features\n");
      break;
    default:
      Log("Finger:Unknown error\n");
      break;
  }

  *error = (p != FINGERPRINT_OK);
}

