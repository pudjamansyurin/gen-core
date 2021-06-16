/*
 * _finger.c
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_finger.h"
#include "Drivers/_fz3387.h"
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
static void IndicatorShow(uint8_t ok);
static void IndicatorHide(void);
static uint8_t AuthFast(void);
static uint8_t GenerateID(uint8_t *theId);
static uint8_t ConvertImage(uint8_t slot);
static uint8_t GetImage(uint32_t timeout);
static void DebugResponse(uint8_t res, char *msg);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t FGR_Init(void) {
	uint8_t ok;
	uint32_t tick;

	lock();
	printf("FGR:Init\n");

	tick = _GetTickMS();
	do {

		MX_UART4_Init();
		FINGER_DMA_Start(FGR.puart, FGR.pdma);
		GATE_FingerReset();

		ok = FGR_Probe();
		if (!ok) _DelayMS(500);
	} while (!ok && _TickIn(tick, FINGER_TIMEOUT_MS));

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
	ok = fz3387_checkPassword() == FINGERPRINT_OK;
	GATE_FingerChipPower(0);
	unlock();

	return ok;
}

void FGR_Verify(void) {
	lock();
	FGR.d.verified = FGR_Probe();
	if (!FGR.d.verified) {
		FGR_DeInit();
		_DelayMS(500);
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
	res = fz3387_getTemplateCount(&templateCount);
	for (uint8_t id = 1; id <= FINGER_USER_MAX; id++)
		FGR.d.db[id-1] = (fz3387_loadModel(id) == FINGERPRINT_OK);
	unlock();

	return res == FINGERPRINT_OK;
}

uint8_t FGR_Enroll(uint8_t *id, uint8_t *ok) {
	uint8_t res;

	lock();
	res = GenerateID(id);

	*ok = (res == FINGERPRINT_OK && *id > 0);

	if (*ok) {
		IndicatorShow(1);
		printf("FGR:Waiting for valid finger to enroll as #%u\n", *id);
		*ok = GetImage(FINGER_SCAN_MS);
	}
	if (*ok)
		*ok = ConvertImage(1);

	if (*ok) {
		IndicatorHide();
		while (fz3387_getImage() != FINGERPRINT_NOFINGER) {
			_DelayMS(50);
		}

		IndicatorShow(1);
		printf("FGR:Waiting for valid finger to enroll as #%u\n", *id);
		*ok = GetImage(FINGER_SCAN_MS);
	}
	if (*ok)
		*ok = ConvertImage(2);

	IndicatorHide();
	if (*ok) {
		while (fz3387_getImage() != FINGERPRINT_NOFINGER) {
			_DelayMS(50);
		}

		printf("FGR:Creating model for #%u\n", *id);
		res = fz3387_createModel();
		DebugResponse(res, "Prints matched!");
		*ok = (res == FINGERPRINT_OK);
	}

	if (*ok) {
		printf("FGR:ID #%u\n", *id);
		res = fz3387_storeModel(*id);
		DebugResponse(res, "Stored!");
		*ok = (res == FINGERPRINT_OK);
	}
	unlock();

	IndicatorShow(*ok); _DelayMS(250);
	IndicatorHide();

	return (res == FINGERPRINT_OK);
}

uint8_t FGR_DeleteID(uint8_t id) {
	uint8_t res;

	lock();
	res = fz3387_deleteModel(id);
	DebugResponse(res, "Deleted!");
	unlock();

	return (res == FINGERPRINT_OK);
}

uint8_t FGR_ResetDB(void) {
	uint8_t res;

	lock();
	res = fz3387_emptyDatabase();
	DebugResponse(res, "Reseted!");
	unlock();

	return (res == FINGERPRINT_OK);
}

uint8_t FGR_SetPassword(uint32_t password) {
	uint8_t res;

	lock();
	res = fz3387_setPassword(password);
	DebugResponse(res, "Password applied!");
	unlock();

	return (res == FINGERPRINT_OK);
}

void FGR_Authenticate(void) {
	uint8_t id, ok;

	lock();
	if (GetImage(50)) {
		id = AuthFast();
		ok = id > 0;
		if (ok)
			FGR.d.id = FGR.d.id ? 0: id;

		IndicatorShow(ok); _DelayMS(250);
		IndicatorHide();
	}
	unlock();
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
	osMutexAcquire(FingerRecMutexHandle, osWaitForever);
#endif
	//	GATE_FingerChipPower(1);
}

static void unlock(void) {
	//	GATE_FingerChipPower(0);
#if (RTOS_ENABLE)
	osMutexRelease(FingerRecMutexHandle);
#endif
}

static uint8_t AuthFast(void) {
	uint16_t id = 0, confidence;

	lock();
	if (fz3387_image2Tz(1) == FINGERPRINT_OK) {
		if (fz3387_fingerFastSearch(&id, &confidence) == FINGERPRINT_OK) {
			if (confidence < FINGER_CONFIDENCE_MIN_PERCENT)
				id = 0;
		} else
			id = 0;
	}
	unlock();

	return id;
}

static void IndicatorShow(uint8_t ok) {
	FGR.d.registering = (FGR_REG_SHOW & 0x01) | ((!ok & 0x01) << 1);
}

static void IndicatorHide(void) {
	FGR.d.registering = (FGR_REG_HIDE & 0x01);
}

static uint8_t GetImage(uint32_t timeout) {
	uint8_t ok, res;
	uint32_t tick;

	tick = _GetTickMS();
	do {
		res = fz3387_getImage();
		ok = res == FINGERPRINT_OK;

		if (res == FINGERPRINT_NOFINGER) printf(".\n");
		else DebugResponse(res, "Image taken");

		_DelayMS(50);
	} while (!ok && _TickIn(tick, timeout));

	return ok;
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
		FGR_Verify();
		break;
	}
#endif
}
