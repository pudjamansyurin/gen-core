/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_ee24xx.h"
#include "Libs/_eeprom.h"
#if (!BOOTLOADER)
#include "Drivers/_aes.h"
#include "Libs/_reporter.h"
#include "Libs/_remote.h"
#include "Nodes/VCU.h"
#endif

/* External variables -------------------------------------------------------*/
#if (!BOOTLOADER)
extern osMutexId_t EepromMutexHandle;
#endif

/* Exported variables ---------------------------------------------------------*/
uint16_t FOTA_VERSION = 0;
IAP_TYPE FOTA_TYPE = 0;
#if (BOOTLOADER)
uint32_t DFU_FLAG = 0;
#endif

/* Private functions prototype ------------------------------------------------*/
static uint8_t Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size);
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
uint8_t EEPROM_Init(I2C_HandleTypeDef *hi2c) {
	uint8_t retry = 5, valid = 0;

	lock();
	printf("EEPROM:Init\n");
	EEPROM24XX_SetDevice(hi2c, EEPROM_ADDR);
	while (!valid && retry--)
		valid = EEPROM24XX_IsConnected(1000);
	unlock();

	if (valid) {
#if (!BOOTLOADER)
		EEPROM_ResetOrLoad();
		EEPROM_FotaVersion(EE_CMD_R, EE_NULL);
#endif
		EEPROM_FotaType(EE_CMD_R, EE_NULL);
	} else
		printf("EEPROM:Error\n");

	return valid;
}

#if (!BOOTLOADER)
void EEPROM_ResetOrLoad(void) {
	if (!EEPROM_Reset(EE_CMD_R, EEPROM_RESET)) {
		// load from EEPROM
		EEPROM_UnitID(EE_CMD_R, EE_NULL);
		EEPROM_Odometer(EE_CMD_R, EE_NULL);
		EEPROM_AesKey(EE_CMD_R, EE_NULL);
	} else {
		// save to EEPROM, first
		EEPROM_UnitID(EE_CMD_W, VCU_UNITID);
		EEPROM_Odometer(EE_CMD_W, 0);

		// re-write eeprom
		EEPROM_Reset(EE_CMD_W, EEPROM_RESET);
	}
}

uint8_t EEPROM_Reset(EEPROM_COMMAND cmd, uint16_t value) {
	uint8_t ret;
	uint16_t tmp = value, temp;

	ret = Command(VADDR_RESET, cmd, &value, &temp, sizeof(value));

	if (ret)
		if (cmd == EE_CMD_R)
			return tmp != temp;

	return ret;
}

uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t value) {
	return Command(VADDR_ODOMETER, cmd, &value, &(VCU.d.odometer), sizeof(value));
}

uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t value) {
	uint8_t ret;

	ret = Command(VADDR_UNITID, cmd, &value, &(VCU.d.unit_id), sizeof(value));

	return ret;
}

uint8_t EEPROM_AesKey(EEPROM_COMMAND cmd, uint32_t *value) {
	uint8_t ret;
	uint32_t *ptr, tmp[4];

	ptr = (cmd == EE_CMD_W ? value : tmp);
	ret = Command(VADDR_AES_KEY, cmd, ptr, AesKey, 16);

	return ret;
}
#else
uint8_t EEPROM_FlagDFU(EEPROM_COMMAND cmd, uint32_t value) {
	return Command(VADDR_DFU_FLAG, cmd, &value, &DFU_FLAG, sizeof(value));
}
#endif

uint8_t EEPROM_FotaVersion(EEPROM_COMMAND cmd, uint16_t value) {
	return Command(VADDR_FOTA_VERSION, cmd, &value, &FOTA_VERSION, sizeof(value));
}

uint8_t EEPROM_FotaType(EEPROM_COMMAND cmd, IAP_TYPE value) {
	uint32_t result;

	result = Command(VADDR_FOTA_TYPE, cmd, &value, &FOTA_TYPE, sizeof(uint32_t));

	if (cmd == EE_CMD_R)
		if (!(FOTA_TYPE == IAP_VCU || FOTA_TYPE == IAP_HMI))
			FOTA_TYPE = IAP_VCU;

	return result;
}

/* Private functions implementation --------------------------------------------*/
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
#if (!BOOTLOADER)
	osMutexAcquire(EepromMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (!BOOTLOADER)
	osMutexRelease(EepromMutexHandle);
#endif
}
