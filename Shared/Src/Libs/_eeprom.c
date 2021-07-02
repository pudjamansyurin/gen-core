/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_eeprom.h"

#include "Drivers/_at24c.h"
#include "i2c.h"

#if (APP)
#include "Drivers/_aes.h"
#include "Libs/_hbar.h"
#else
#include "App/_fota.h"
#endif
#include "Drivers/_sim_con.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t EepromRecMutexHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static I2C_HandleTypeDef* pi2c = &hi2c2;
static uint8_t EE_SZ[VA_MAX];

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
static void InitializeSize(void);
static uint16_t Address(EE_VA va);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t EE_Init(void) {
	uint8_t ok;

	lock();
	printf("EEPROM:Init\n");
	AT24C_Init(pi2c);
	do {
		ok = AT24C_Probe(100);
		if (!ok)
			printf("EEPROM:Error\n");
	} while (!ok);

	printf("EEPROM:OK\n");
	InitializeSize();

//	IAP_TYPE type = ITYPE_HMI;
//	IAP_TypeStore(&type);

#if (APP)
	AES_KeyStore(NULL);
	HBAR_LoadStore();
#else
	if (IEEP_VALUE == IEEP_RESET)
		if (SIMCon_SetDefaultStore())
			IAP_SetBootMeta(IEEP_OFFSET, IEEP_SET);
#endif
	SIMCon_SetDefaultStore();
//	SIMCon_LoadStore();
	IAP_VersionStore(NULL);
	IAP_TypeStore(NULL);
	unlock();

	return ok;
}

uint8_t EE_Cmd(EE_VA va, void* src, void* dst) {
	uint16_t addr = Address(va);
	uint8_t ok;

	lock();
	if (src != NULL)
		ok = AT24C_Write(addr, src, EE_SZ[va]);
	ok = AT24C_Read(addr, dst, EE_SZ[va]);
	unlock();

	return ok;
}

//uint8_t EE_CmdWithClear(EE_VA va, void* src, void* dst) {
//	uint8_t ok;
//
//	lock();
//	if (src != NULL)
//		ok = AT24C_Clear(Address(va), EE_SZ[va]);
//	ok = EE_Cmd(va, src, dst);
//	unlock();
//
//	return ok;
//}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
	osMutexAcquire(EepromRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
	osMutexRelease(EepromRecMutexHandle);
#endif
}

static void InitializeSize(void) {
	EE_SZ[VA_AES_KEY] = 4 * sizeof(uint32_t);
	EE_SZ[VA_IAP_VERSION] = sizeof(uint16_t);
	EE_SZ[VA_IAP_FLAG] = sizeof(uint32_t);
	EE_SZ[VA_IAP_TYPE] = sizeof(uint32_t);
	EE_SZ[VA_TRIP_A] = sizeof(uint16_t);
	EE_SZ[VA_TRIP_B] = sizeof(uint16_t);
	EE_SZ[VA_TRIP_ODO] = sizeof(uint16_t);
	EE_SZ[VA_MODE_DRIVE] = sizeof(uint8_t);
	EE_SZ[VA_MODE_TRIP] = sizeof(uint8_t);
	EE_SZ[VA_MODE_PREDICTION] = sizeof(uint8_t);
	EE_SZ[VA_MODE] = sizeof(uint8_t);
	EE_SZ[VA_APN_NAME] = EE_STR_MAX;
	EE_SZ[VA_APN_USER] = EE_STR_MAX;
	EE_SZ[VA_APN_PASS] = EE_STR_MAX;
	EE_SZ[VA_FTP_HOST] = EE_STR_MAX;
	EE_SZ[VA_FTP_USER] = EE_STR_MAX;
	EE_SZ[VA_FTP_PASS] = EE_STR_MAX;
	EE_SZ[VA_MQTT_HOST] = EE_STR_MAX;
	EE_SZ[VA_MQTT_PORT] = EE_STR_MAX;
	EE_SZ[VA_MQTT_USER] = EE_STR_MAX;
	EE_SZ[VA_MQTT_PASS] = EE_STR_MAX;
}

static uint16_t Address(EE_VA va) {
	uint16_t reg = 0;

	for (uint8_t v = 0; v < VA_MAX; v++) {
		if (v == va) break;
		reg += EE_SZ[v];
	}

	return reg;
}
