/*
 * _finger.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_finger.h"
#include "HMI1.h"

/* External variables ---------------------------------------------------------*/
extern db_t DB;
extern hmi1_t HMI1;
extern finger_t finger;
extern osMutexId_t FingerRecMutexHandle;

/* Private functions ----------------------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
void Finger_Init(void) {
	uint8_t verified = 0;

	do {
		LOG_StrLn("Finger:Init");

		// mosfet control
		HAL_GPIO_WritePin(EXT_FINGER_PWR_GPIO_Port, EXT_FINGER_PWR_Pin, 0);
		osDelay(100);
		HAL_GPIO_WritePin(EXT_FINGER_PWR_GPIO_Port, EXT_FINGER_PWR_Pin, 1);
		osDelay(500);

		// verify password and check hardware
		lock();
		verified = FZ3387_verifyPassword();
		unlock();

		osDelay(500);
	} while (!verified);
}

uint8_t Finger_Enroll(uint8_t id) {
	TickType_t tick;
	const TickType_t timeout_tick = pdMS_TO_TICKS(FINGER_SCAN_TIMEOUT*1000);
	int p = -1, error = 0;

	lock();
	//	Take Image
	LOG_Str("\nWaiting for valid finger to enroll as #");
	LOG_Int(id);
	LOG_Enter();

	// set timeout guard
	tick = osKernelGetTickCount();
	while (p != FINGERPRINT_OK && !error) {
		// handle timeout
		if ((osKernelGetTickCount() - tick) > timeout_tick) {
			error = 1;
		}

		// send command
		HMI1.d.status.finger = !HMI1.d.status.finger;
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
		HMI1.d.status.finger = 0;
		LOG_StrLn("Remove finger");
		osDelay(2000);

		//	Take Image again
		LOG_StrLn("Place same finger again");
		p = -1;
		// set timeout guard
		tick = osKernelGetTickCount();
		while (p != FINGERPRINT_OK && !error) {
			// handle timeout
			if ((osKernelGetTickCount() - tick) > timeout_tick) {
				error = 1;
			}

			// send command
			HMI1.d.status.finger = !HMI1.d.status.finger;
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
		HMI1.d.status.finger = 0;
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

	HMI1.d.status.finger = 0;
	unlock();
	return p;
}

uint8_t Finger_DeleteID(uint8_t id) {
	int8_t p = -1;

	lock();
	p = FZ3387_deleteModel(id);

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

	unlock();
	return p;
}

uint8_t Finger_EmptyDatabase(void) {
	int8_t p = -1;

	lock();
	p = FZ3387_emptyDatabase();
	unlock();

	return p;
}

uint8_t Finger_SetPassword(uint32_t password) {
	int8_t p = -1;

	lock();
	p = FZ3387_setPassword(password);
	unlock();

	return (p == FINGERPRINT_OK);
}

int8_t Finger_Auth(void) {
	int8_t p = -1, id = -1, error = 1;

	lock();

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

	if (!error) {
		// found a match!
		LOG_Str("\nFound ID #");
		LOG_Int(finger.id);
		LOG_Str(" with confidence of ");
		LOG_Int(finger.confidence);
		LOG_Enter();

		if (finger.confidence > FINGER_CONFIDENCE_MIN) {
			id = finger.id;
		}
	}

	unlock();
	return id;
}

int8_t Finger_AuthFast(void) {
	int8_t p = -1, id = -1;

	lock();

	p = FZ3387_getImage();

	if (p == FINGERPRINT_OK) {
		p = FZ3387_image2Tz(1);
	}

	if (p == FINGERPRINT_OK) {
		p = FZ3387_fingerFastSearch();
	}

	if (p == FINGERPRINT_OK) {
		// found a match!
		LOG_Str("\nFound ID #");
		LOG_Int(finger.id);
		LOG_Str(" with confidence of ");
		LOG_Int(finger.confidence);
		LOG_Enter();

		if (finger.confidence > FINGER_CONFIDENCE_MIN) {
			id = finger.id;
		}
	}

	unlock();
	return id;
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
	osMutexAcquire(FingerRecMutexHandle, osWaitForever);
	FZ3387_SET_POWER(0);
}

static void unlock(void) {
	FZ3387_SET_POWER(1);
	osDelay(50);
	osMutexRelease(FingerRecMutexHandle);
}
