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

/* Public variables
 * ----------------------------------------------------------*/
finger_t FGR = {
		.d = {0},
		.puart = &huart4,
		.pdma = &hdma_uart4_rx
};

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
uint8_t FINGER_Init(void) {
	uint8_t ok;

	lock();
	uint32_t tick = _GetTickMS();
	do {
		printf("FGR:Init\n");

		MX_UART4_Init();
		FINGER_DMA_Start(FGR.puart, FGR.pdma);
		GATE_FingerReset();

		ok = fz3387_verifyPassword();
		_DelayMS(500);
	} while (!ok && _GetTickMS() - tick < FINGER_TIMEOUT);
	unlock();

	printf("FGR:%s\n", ok ? "OK" : "Error");
	FGR.d.verified = ok;
	return ok;
}

void FINGER_DeInit(void) {
	lock();
	FINGER_Flush();
	GATE_FingerShutdown();
	FINGER_DMA_Stop();
	HAL_UART_DeInit(FGR.puart);
	unlock();
}

uint8_t FINGER_Verify(void) {
	lock();
	FGR.d.verified = fz3387_verifyPassword();
	if (!FGR.d.verified) {
		FINGER_DeInit();
		_DelayMS(500);
		FINGER_Init();
	}
	unlock();

	return FGR.d.verified;
}

void FINGER_Flush(void) {
	lock();
	memset(&(FGR.d), 0, sizeof(finger_data_t));
	unlock();
}

uint8_t FINGER_Fetch(void) {
	uint16_t templateCount;
	uint8_t res;

	lock();
	res = fz3387_getTemplateCount(&templateCount);
	for (uint8_t id = 1; id <= FINGER_USER_MAX; id++)
		FGR.d.db[id-1] = (fz3387_loadModel(id) == FINGERPRINT_OK);
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
		*valid = Scan(*id, 1, FINGER_SCAN_TIMEOUT);
	}

	if (*valid) {
		GATE_LedWrite(0);
		while (fz3387_getImage() != FINGERPRINT_NOFINGER) {
			_DelayMS(50);
		}

		GATE_LedWrite(1);
		*valid = Scan(*id, 2, FINGER_SCAN_TIMEOUT);
	}

	GATE_LedWrite(0);

	if (*valid) {
		while (fz3387_getImage() != FINGERPRINT_NOFINGER) {
			_DelayMS(50);
		}

		printf("FGR:Creating model for #%u\n", *id);
		res = fz3387_createModel();
		DebugResponse(res, "Prints matched!");
		*valid = (res == FINGERPRINT_OK);
	}

	if (*valid) {
		printf("FGR:ID #%u\n", *id);
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

uint8_t FINGER_ResetDB(void) {
	uint8_t res;

	lock();
	res = fz3387_emptyDatabase();
	DebugResponse(res, "Reseted!");
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
	uint8_t valid, res, theId = 0;

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
		printf("FGR:Found ID #%u with confidence of %u\n", id, confidence);
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
	TickType_t tick;
	uint8_t valid = 0;

	printf("FGR:Waiting for valid finger to enroll as #%u\n", id);

	tick = _GetTickMS();
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

	if (res == FINGERPRINT_NOFINGER) printf(".\n");
	else DebugResponse(res, "Image taken");

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
		printf("FGR:TemplateCount = %u\n", templateCount);

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
#if FINGER_DEBUG
	switch (res) {
	case FINGERPRINT_OK:
		printf("FGR:%s\n", msg);
		break;
	case FINGERPRINT_NOFINGER:
		printf("FGR:No finger detected\n");
		break;
	case FINGERPRINT_NOTFOUND:
		printf("FGR:Did not find a match\n");
		break;
	case FINGERPRINT_PACKETRECIEVEERR:
		printf("FGR:Communication error\n");
		break;
	case FINGERPRINT_ENROLLMISMATCH:
		printf("FGR:Fingerprints did not match\n");
		break;
	case FINGERPRINT_IMAGEMESS:
		printf("FGR:Image too messy\n");
		break;
	case FINGERPRINT_IMAGEFAIL:
		printf("FGR:Imaging error\n");
		break;
	case FINGERPRINT_FEATUREFAIL:
		printf("FGR:Could not find finger print features fail\n");
		break;
	case FINGERPRINT_INVALIDIMAGE:
		printf("FGR:Could not find finger print features invalid\n");
		break;
	case FINGERPRINT_BADLOCATION:
		printf("FGR:Could not execute in that location\n");
		break;
	case FINGERPRINT_FLASHERR:
		printf("FGR:Error writing to flash\n");
		break;
	default:
		printf("FGR:Unknown error: 0x%02X\n", res);
		FINGER_Verify();
		break;
	}
#endif
}
