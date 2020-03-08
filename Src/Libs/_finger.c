/*
 * _finger.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

#include "_finger.h"

/* External variable ---------------------------------------------------------*/
extern uint16_t fingerID, fingerConfidence;
extern osMutexId FingerRecMutexHandle;
/* Private variable ---------------------------------------------------------*/

void Finger_On(void) {
  osRecursiveMutexWait(FingerRecMutexHandle, osWaitForever);
  FZ3387_SET_POWER(0);
}

void Finger_Off(void) {
  FZ3387_SET_POWER(1);
  osDelay(50);
  osRecursiveMutexRelease(FingerRecMutexHandle);
}

void Finger_Init(void) {
  uint8_t verified = 0;

  //	 verify password and check hardware
  do {
    LOG_StrLn("Finger_Init");

    Finger_On();
    verified = FZ3387_verifyPassword();
    Finger_Off();

    osDelay(500);
  } while (!verified);
}

uint8_t Finger_Enroll(uint8_t id) {
  TickType_t tick;
  const TickType_t timeout_tick = pdMS_TO_TICKS(FINGER_SCAN_TIMEOUT*1000);
  int p = -1, error = 0;

  Finger_On();
  //	Take Image
  LOG_Str("\nWaiting for valid finger to enroll as #");
  LOG_Int(id);
  LOG_Enter();

  // set timeout guard
  tick = osKernelSysTick();
  while (p != FINGERPRINT_OK && !error) {
    // handle timeout
    if ((osKernelSysTick() - tick) > timeout_tick) {
      error = 1;
    }
    // send command
    _LedToggle();
    p = FZ3387_getImage();
    // check response
    switch (p) {
      case FINGERPRINT_OK:
        LOG_StrLn("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        LOG_StrLn(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        LOG_StrLn("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        LOG_StrLn("Imaging error");
        break;
      default:
        LOG_StrLn("Unknown error");
        break;
    }
  }

  if (!error) {
    // set default to error, then set to fix
    error = 1;
    //	put image to buffer 1
    p = FZ3387_image2Tz(1);
    switch (p) {
      case FINGERPRINT_OK:
        LOG_StrLn("Image converted");
        error = 0;
        break;
      case FINGERPRINT_IMAGEMESS:
        LOG_StrLn("Image too messy");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        LOG_StrLn("Communication error");
        break;
      case FINGERPRINT_FEATUREFAIL:
        LOG_StrLn("Could not find finger print features");
        break;
      case FINGERPRINT_INVALIDIMAGE:
        LOG_StrLn("Could not find finger print features");
        break;
      default:
        LOG_StrLn("Unknown error");
        break;
    }
  }

  if (!error) {
    //	 Wait for put your finger up
    _LedWrite(0);
    LOG_StrLn("Remove finger");
    osDelay(2000);

    //	Take Image again
    LOG_StrLn("Place same finger again");
    p = -1;
    // set timeout guard
    tick = osKernelSysTick();
    while (p != FINGERPRINT_OK && !error) {
      // handle timeout
      if ((osKernelSysTick() - tick) > timeout_tick) {
        error = 1;
      }
      // send command
      _LedToggle();
      p = FZ3387_getImage();
      // handle response
      switch (p) {
        case FINGERPRINT_OK:
          LOG_StrLn("Image taken");
          break;
        case FINGERPRINT_NOFINGER:
          LOG_Str(".");
          break;
        case FINGERPRINT_PACKETRECIEVEERR:
          LOG_StrLn("Communication error");
          break;
        case FINGERPRINT_IMAGEFAIL:
          LOG_StrLn("Imaging error");
          break;
        default:
          LOG_StrLn("Unknown error");
          break;
      }
    }
  }

  if (!error) {
    error = 1;
    //	put image to buffer 2
    p = FZ3387_image2Tz(2);
    switch (p) {
      case FINGERPRINT_OK:
        LOG_StrLn("Image converted");
        error = 0;
        break;
      case FINGERPRINT_IMAGEMESS:
        LOG_StrLn("Image too messy");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        LOG_StrLn("Communication error");
        break;
      case FINGERPRINT_FEATUREFAIL:
        LOG_StrLn("Could not find fingerprint features");
        break;
      case FINGERPRINT_INVALIDIMAGE:
        LOG_StrLn("Could not find fingerprint features");
        break;
      default:
        LOG_StrLn("Unknown error");
        break;
    }
  }

  if (!error) {
    //	 Wait for put your finger up
    _LedWrite(0);
    LOG_StrLn("Remove finger");
    osDelay(2000);
    //	Create Register model
    error = 1;
    LOG_Str("\nCreating model for #");
    LOG_Int(id);
    LOG_Enter();

    p = FZ3387_createModel();
    if (p == FINGERPRINT_OK) {
      LOG_StrLn("Prints matched!");
      error = 0;
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      LOG_StrLn("Communication error");
    } else if (p == FINGERPRINT_ENROLLMISMATCH) {
      LOG_StrLn("Fingerprints did not match");
    } else {
      LOG_StrLn("Unknown error");
    }
  }

  if (!error) {
    error = 1;
    //	Store in memory
    LOG_Str("\nID #");
    LOG_Int(id);
    LOG_Enter();

    p = FZ3387_storeModel(id);
    if (p == FINGERPRINT_OK) {
      LOG_StrLn("Stored!");
      error = 0;
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      LOG_StrLn("Communication error");
    } else if (p == FINGERPRINT_BADLOCATION) {
      LOG_StrLn("Could not store in that location");
    } else if (p == FINGERPRINT_FLASHERR) {
      LOG_StrLn("Error writing to flash");
    } else {
      LOG_StrLn("Unknown error");
    }
  }

  Finger_Off();
  return p;
}

uint8_t Finger_DeleteID(uint8_t id) {
  uint8_t p = -1;

  Finger_On();
  p = FZ3387_deleteModel(id);
  Finger_Off();

  if (p == FINGERPRINT_OK) {
    LOG_StrLn("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    LOG_StrLn("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    LOG_StrLn("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    LOG_StrLn("Error writing to flash");
  } else {
    LOG_Str("\nUnknown error: 0x");
    LOG_Hex8(p);
    LOG_Enter();
  }

  return p;
}

uint8_t Finger_EmptyDatabase(void) {
  uint8_t p = -1;

  Finger_On();
  p = FZ3387_emptyDatabase();
  Finger_Off();

  return p;
}

uint8_t Finger_SetPassword(uint32_t password) {
  uint8_t p = -1;

  Finger_On();
  p = FZ3387_setPassword(password);
  Finger_Off();

  if (p == FINGERPRINT_OK) {
    return 1;
  }
  return 0;
}

int8_t Finger_Auth(void) {
  uint8_t p = -1, error = 1;

  Finger_On();

  p = FZ3387_getImage();
  switch (p) {
    case FINGERPRINT_OK:
      LOG_StrLn("Image taken");
      error = 0;
      break;
    case FINGERPRINT_NOFINGER:
      LOG_StrLn("No finger detected");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      LOG_StrLn("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      LOG_StrLn("Imaging error");
      break;
    default:
      LOG_StrLn("Unknown error");
      break;
  }

  if (!error) {
    error = 1;
    // OK success!
    p = FZ3387_image2Tz(1);
    switch (p) {
      case FINGERPRINT_OK:
        LOG_StrLn("Image converted");
        error = 0;
        break;
      case FINGERPRINT_IMAGEMESS:
        LOG_StrLn("Image too messy");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        LOG_StrLn("Communication error");
        break;
      case FINGERPRINT_FEATUREFAIL:
        LOG_StrLn("Could not find finger print features");
        break;
      case FINGERPRINT_INVALIDIMAGE:
        LOG_StrLn("Could not find finger print features");
        break;
      default:
        LOG_StrLn("Unknown error");
        break;
    }
  }

  if (!error) {
    error = 1;
    // OK converted!
    p = FZ3387_fingerFastSearch();
    if (p == FINGERPRINT_OK) {
      LOG_StrLn("Found a print match!");
      error = 0;
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      LOG_StrLn("Communication error");
    } else if (p == FINGERPRINT_NOTFOUND) {
      LOG_StrLn("Did not find a match");
    } else {
      LOG_StrLn("Unknown error");
    }
  }

  Finger_Off();

  if (!error) {
    // found a match!
    LOG_Str("\nFound ID #");
    LOG_Int(fingerID);
    LOG_Str(" with confidence of ");
    LOG_Int(fingerConfidence);
    LOG_Enter();

    if (fingerConfidence > FINGER_CONFIDENCE_MIN) {
      return fingerID;
    }
  }

  return -1;
}

int8_t Finger_AuthFast(void) {
  uint8_t p = -1;

  Finger_On();

  p = FZ3387_getImage();

  if (p == FINGERPRINT_OK) {
    p = FZ3387_image2Tz(1);
  }

  if (p == FINGERPRINT_OK) {
    p = FZ3387_fingerFastSearch();
  }

  Finger_Off();

  if (p == FINGERPRINT_OK) {
    // found a match!
    LOG_Str("\nFound ID #");
    LOG_Int(fingerID);
    LOG_Str(" with confidence of ");
    LOG_Int(fingerConfidence);
    LOG_Enter();

    if (fingerConfidence > FINGER_CONFIDENCE_MIN) {
      return fingerID;
    }
  }

  return -1;
}

