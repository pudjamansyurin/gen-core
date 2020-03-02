/*
 * _finger.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

#include "_finger.h"

/* External variable ---------------------------------------------------------*/
extern uint16_t fingerID;
extern uint16_t fingerConfidence;
extern osMutexId FingerRecMutexHandle;
/* Private variable ---------------------------------------------------------*/
char str[50];

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
		SWV_SendStrLn("Finger_Init");

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
	sprintf(str, "Waiting for valid finger to enroll as # %d", id);
	SWV_SendStrLn(str);

	// set timeout guard
	tick = osKernelSysTick();
	while (p != FINGERPRINT_OK && !error) {
		// handle timeout
		if ((osKernelSysTick() - tick) > timeout_tick) {
			error = 1;
		}
		// send command
		BSP_LedToggle();
		p = FZ3387_getImage();
		// check response
		switch (p) {
			case FINGERPRINT_OK:
				SWV_SendStrLn("Image taken");
				break;
			case FINGERPRINT_NOFINGER:
				SWV_SendStrLn(".");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				SWV_SendStrLn("Communication error");
				break;
			case FINGERPRINT_IMAGEFAIL:
				SWV_SendStrLn("Imaging error");
				break;
			default:
				SWV_SendStrLn("Unknown error");
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
				SWV_SendStrLn("Image converted");
				error = 0;
				break;
			case FINGERPRINT_IMAGEMESS:
				SWV_SendStrLn("Image too messy");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				SWV_SendStrLn("Communication error");
				break;
			case FINGERPRINT_FEATUREFAIL:
				SWV_SendStrLn("Could not find finger print features");
				break;
			case FINGERPRINT_INVALIDIMAGE:
				SWV_SendStrLn("Could not find finger print features");
				break;
			default:
				SWV_SendStrLn("Unknown error");
				break;
		}
	}

	if (!error) {
		//	 Wait for put your finger up
		BSP_LedWrite(0);
		SWV_SendStrLn("Remove finger");
		osDelay(2000);

		//	Take Image again
		SWV_SendStrLn("Place same finger again");
		p = -1;
		// set timeout guard
		tick = osKernelSysTick();
		while (p != FINGERPRINT_OK && !error) {
			// handle timeout
			if ((osKernelSysTick() - tick) > timeout_tick) {
				error = 1;
			}
			// send command
			BSP_LedToggle();
			p = FZ3387_getImage();
			// handle response
			switch (p) {
				case FINGERPRINT_OK:
					SWV_SendStrLn("Image taken");
					break;
				case FINGERPRINT_NOFINGER:
					SWV_SendStr(".");
					break;
				case FINGERPRINT_PACKETRECIEVEERR:
					SWV_SendStrLn("Communication error");
					break;
				case FINGERPRINT_IMAGEFAIL:
					SWV_SendStrLn("Imaging error");
					break;
				default:
					SWV_SendStrLn("Unknown error");
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
				SWV_SendStrLn("Image converted");
				error = 0;
				break;
			case FINGERPRINT_IMAGEMESS:
				SWV_SendStrLn("Image too messy");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				SWV_SendStrLn("Communication error");
				break;
			case FINGERPRINT_FEATUREFAIL:
				SWV_SendStrLn("Could not find fingerprint features");
				break;
			case FINGERPRINT_INVALIDIMAGE:
				SWV_SendStrLn("Could not find fingerprint features");
				break;
			default:
				SWV_SendStrLn("Unknown error");
				break;
		}
	}

	if (!error) {
		//	 Wait for put your finger up
		BSP_LedWrite(0);
		SWV_SendStrLn("Remove finger");
		osDelay(2000);
		//	Create Register model
		error = 1;
		sprintf(str, "Creating model for #%d", id);
		SWV_SendStrLn(str);

		p = FZ3387_createModel();
		if (p == FINGERPRINT_OK) {
			SWV_SendStrLn("Prints matched!");
			error = 0;
		} else if (p == FINGERPRINT_PACKETRECIEVEERR) {
			SWV_SendStrLn("Communication error");
		} else if (p == FINGERPRINT_ENROLLMISMATCH) {
			SWV_SendStrLn("Fingerprints did not match");
		} else {
			SWV_SendStrLn("Unknown error");
		}
	}

	if (!error) {
		error = 1;
		//	Store in memory
		sprintf(str, "ID %d", id);
		SWV_SendStrLn(str);

		p = FZ3387_storeModel(id);
		if (p == FINGERPRINT_OK) {
			SWV_SendStrLn("Stored!");
			error = 0;
		} else if (p == FINGERPRINT_PACKETRECIEVEERR) {
			SWV_SendStrLn("Communication error");
		} else if (p == FINGERPRINT_BADLOCATION) {
			SWV_SendStrLn("Could not store in that location");
		} else if (p == FINGERPRINT_FLASHERR) {
			SWV_SendStrLn("Error writing to flash");
		} else {
			SWV_SendStrLn("Unknown error");
		}
	}

	Finger_Off();
	return p;
}

uint8_t Finger_Delete_ID(uint8_t id) {
	uint8_t p = -1;

	Finger_On();
	p = FZ3387_deleteModel(id);
	Finger_Off();

	if (p == FINGERPRINT_OK) {
		SWV_SendStrLn("Deleted!");
	} else if (p == FINGERPRINT_PACKETRECIEVEERR) {
		SWV_SendStrLn("Communication error");
	} else if (p == FINGERPRINT_BADLOCATION) {
		SWV_SendStrLn("Could not delete in that location");
	} else if (p == FINGERPRINT_FLASHERR) {
		SWV_SendStrLn("Error writing to flash");
	} else {
		sprintf(str, "Unknown error: 0x%02x", p);
		SWV_SendStrLn(str);
	}

	return p;
}

uint8_t Finger_Empty_Database(void) {
	uint8_t p = -1;

	Finger_On();
	p = FZ3387_emptyDatabase();
	Finger_Off();

	return p;
}

uint8_t Finger_Set_Password(uint32_t password) {
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
			SWV_SendStrLn("Image taken");
			error = 0;
			break;
		case FINGERPRINT_NOFINGER:
			SWV_SendStrLn("No finger detected");
			break;
		case FINGERPRINT_PACKETRECIEVEERR:
			SWV_SendStrLn("Communication error");
			break;
		case FINGERPRINT_IMAGEFAIL:
			SWV_SendStrLn("Imaging error");
			break;
		default:
			SWV_SendStrLn("Unknown error");
			break;
	}

	if (!error) {
		error = 1;
		// OK success!
		p = FZ3387_image2Tz(1);
		switch (p) {
			case FINGERPRINT_OK:
				SWV_SendStrLn("Image converted");
				error = 0;
				break;
			case FINGERPRINT_IMAGEMESS:
				SWV_SendStrLn("Image too messy");
				break;
			case FINGERPRINT_PACKETRECIEVEERR:
				SWV_SendStrLn("Communication error");
				break;
			case FINGERPRINT_FEATUREFAIL:
				SWV_SendStrLn("Could not find finger print features");
				break;
			case FINGERPRINT_INVALIDIMAGE:
				SWV_SendStrLn("Could not find finger print features");
				break;
			default:
				SWV_SendStrLn("Unknown error");
				break;
		}
	}

	if (!error) {
		error = 1;
		// OK converted!
		p = FZ3387_fingerFastSearch();
		if (p == FINGERPRINT_OK) {
			SWV_SendStrLn("Found a print match!");
			error = 0;
		} else if (p == FINGERPRINT_PACKETRECIEVEERR) {
			SWV_SendStrLn("Communication error");
		} else if (p == FINGERPRINT_NOTFOUND) {
			SWV_SendStrLn("Did not find a match");
		} else {
			SWV_SendStrLn("Unknown error");
		}
	}

	Finger_Off();

	if (!error) {
		// found a match!
		sprintf(str, "Found ID #%d  with confidence of %d", fingerID, fingerConfidence);
		SWV_SendStrLn(str);

		if (fingerConfidence > FINGER_CONFIDENCE_MIN) {
			return fingerID;
		}
	}

	return -1;
}

int8_t Finger_Auth_Fast(void) {
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
		sprintf(str, "Found ID #%d  with confidence of %d", fingerID, fingerConfidence);
		SWV_SendStrLn(str);

		if (fingerConfidence > FINGER_CONFIDENCE_MIN) {
			return fingerID;
		}
	}

	return -1;
}

