/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_eeprom.h"
#include "Drivers/_ee24xx.h"
#include "i2c.h"

#if (!BOOTLOADER)
#include "Drivers/_aes.h"
#include "Libs/_hbar.h"
#include "Libs/_remote.h"
#include "Libs/_reporter.h"
#include "Nodes/VCU.h"

#endif

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t EepromRecMutexHandle;
#endif

/* Public variables
 * -----------------------------------------------------------*/
fota_t FOTA = {0};

/* Private variables
 * ----------------------------------------------------------*/
static I2C_HandleTypeDef *pi2c = &hi2c2;

/* Private functions prototype
 * ------------------------------------------------*/
static uint8_t Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size);
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t EEPROM_Init(void) {
	uint8_t retry = 2, ok;

	lock();
	printf("EEPROM:Init\n");
	EEPROM24XX_SetDevice(pi2c, EEPROM_ADDR);
	do {
		ok = EEPROM24XX_IsConnected(100);
	} while (!ok && retry--);

	if (ok) {
#if (!BOOTLOADER)
		EEPROM_ResetOrLoad();
		EEPROM_FotaVersion(EE_CMD_R, EE_NULL);
#endif
		EEPROM_FotaType(EE_CMD_R, EE_NULL);
	} else
		printf("EEPROM:Error\n");
	unlock();

	return ok;
}

#if (!BOOTLOADER)
void EEPROM_ResetOrLoad(void) {
	lock();
	if (!EEPROM_Reset(EE_CMD_R, EEPROM_RESET)) {
		EEPROM_Odometer(EE_CMD_R, EE_NULL);
		EEPROM_AesKey(EE_CMD_R, EE_NULL);
	} else {
		EEPROM_Odometer(EE_CMD_W, 0);
		EEPROM_Reset(EE_CMD_W, EEPROM_RESET);
	}
	unlock();
}

uint8_t EEPROM_Reset(EEPROM_COMMAND cmd, uint16_t value) {
	uint16_t tmp = value, temp;
	uint8_t ret;

	ret = Command(VADDR_RESET, cmd, &value, &temp, sizeof(value));

	if (ret)
		if (cmd == EE_CMD_R)
			return tmp != temp;

	return ret;
}

uint8_t EEPROM_AesKey(EEPROM_COMMAND cmd, uint32_t *value) {
	uint32_t *ptr, tmp[4];
	uint8_t ret;

	ptr = (cmd == EE_CMD_W ? value : tmp);
	ret = Command(VADDR_AES_KEY, cmd, ptr, AES_KEY, 16);

	return ret;
}

uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint16_t value) {
	return Command(VADDR_ODOMETER, cmd, &value, &(HBAR.d.trip[HBAR_M_TRIP_ODO]), sizeof(value));
}

#endif

uint8_t EEPROM_FotaType(EEPROM_COMMAND cmd, IAP_TYPE value) {
	uint32_t ret;

	ret = Command(VADDR_FOTA_TYPE, cmd, &value, &(FOTA.TYPE), sizeof(uint32_t));

	if (cmd == EE_CMD_R)
		if (!(FOTA.TYPE == IAP_VCU || FOTA.TYPE == IAP_HMI))
			FOTA.TYPE = IAP_VCU;

	return ret;
}

uint8_t EEPROM_FotaFlag(EEPROM_COMMAND cmd, uint32_t value) {
	return Command(VADDR_FOTA_FLAG, cmd, &value, &(FOTA.FLAG), sizeof(value));
}

uint8_t EEPROM_FotaVersion(EEPROM_COMMAND cmd, uint16_t value) {
	return Command(VADDR_FOTA_VERSION, cmd, &value, &(FOTA.VERSION), sizeof(value));
}
/* Private functions implementation
 * --------------------------------------------*/
static uint8_t Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size) {
	uint8_t ret = 0;

	lock();
	// check if new value is same with old value
	if (cmd == EE_CMD_W) {
		// apply the value
		memcpy(ptr, value, size);
		// save the value
		ret = EEPROM24XX_Save(vaddr, value, size);
	} else {
		// load the value
		ret = EEPROM24XX_Load(vaddr, value, size);
		// apply the value
		if (ret)
			memcpy(ptr, value, size);
	}
	unlock();

	return ret;
}

static void lock(void) {
#if (RTOS_ENABLE)
	osMutexAcquire(EepromRecMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
	osMutexRelease(EepromRecMutexHandle);
#endif
}
