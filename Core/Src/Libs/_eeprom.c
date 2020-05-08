/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_eeprom.h"
#include "_database.h"
#include "_reporter.h"

/* External variabless -------------------------------------------------------*/
extern osMutexId_t EepromMutexHandle;
extern db_t DB;
extern report_t REPORT;
extern response_t RESPONSE;

/* Private functions prototype ------------------------------------------------*/
static uint8_t EE_16(uint16_t vaddr, EEPROM_COMMAND cmd, uint16_t *value, uint16_t *ptr);
static uint8_t EE_32(uint16_t vaddr, EEPROM_COMMAND cmd, uint32_t *value, uint32_t *ptr);
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
uint8_t EEPROM_Init(void) {
	uint8_t retry = 5;
	uint8_t ret = 0;

	lock();
	LOG_StrLn("EEPROM:Init");
	// check main eeprom
	EEPROM24XX_SetDevice(EEPROM24_MAIN);
	do {
		if (EEPROM24XX_IsConnected()) {
			LOG_StrLn("EEPROM:Main");
			ret = 1;
			break;
		}
		osDelay(50);
	} while (retry--);

	// check backup eeprom
	if (!ret) {
		retry = 5;
		EEPROM24XX_SetDevice(EEPROM24_BACKUP);
		do {
			if (EEPROM24XX_IsConnected()) {
				LOG_StrLn("EEPROM:Backup");
				ret = 1;
				break;
			}
			osDelay(50);
		} while (retry--);
	}

	// all failed
	if (!ret) {
		LOG_StrLn("EEPROM:Error");
	}

	unlock();
	return ret;
}

uint8_t EEPROM_Reset(EEPROM_COMMAND cmd, uint32_t value) {
	uint8_t ret;
	uint32_t tmp = value, temp;

	ret = EE_32(VADDR_RESET, cmd, &value, &temp);

	if (ret) {
		if (cmd == EE_CMD_R) {
			return tmp != temp;
		}
	}

	return ret;
}

void EEPROM_ResetOrLoad(void) {
	if (EEPROM_Init() && !EEPROM_Reset(EE_CMD_R, EEPROM_RESET)) {
		// load from EEPROM
		EEPROM_UnitID(EE_CMD_R, EE_NULL);
		EEPROM_Odometer(EE_CMD_R, EE_NULL);
		for (uint8_t type = 0; type <= PAYLOAD_MAX; type++) {
			EEPROM_SequentialID(EE_CMD_R, EE_NULL, type);
		}
	} else {
		// reporter configuration
		EEPROM_UnitID(EE_CMD_W, RPT_UNITID);
		EEPROM_Odometer(EE_CMD_W, 0);
		for (uint8_t type = 0; type <= PAYLOAD_MAX; type++) {
			EEPROM_SequentialID(EE_CMD_W, 0, type);
		}
		// simcom configuration

		// re-write eeprom
		EEPROM_Reset(EE_CMD_W, EEPROM_RESET);
	}

}

uint8_t EEPROM_SequentialID(EEPROM_COMMAND cmd, uint16_t value, PAYLOAD_TYPE type) {
	uint16_t *pSeqId;
	uint32_t vaddr;

	// decide payload type
	if (type == PAYLOAD_REPORT) {
		pSeqId = &(DB.vcu.seq_id.report);
		vaddr = VADDR_REPORT_SEQ_ID;
	} else {
		pSeqId = &(DB.vcu.seq_id.response);
		vaddr = VADDR_RESPONSE_SEQ_ID;
	}

	return EE_16(vaddr, cmd, &value, pSeqId);
}

uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t value) {
	// reset on overflow
	if (value > VCU_ODOMETER_MAX) {
		value = 0;
	}
	// FIXME: only update eeprom for 1km/hr increment
	return EE_32(VADDR_ODOMETER, cmd, &value, &(DB.vcu.odometer));
}

uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t value) {
	return EE_32(VADDR_UNITID, cmd, &value, &(DB.vcu.unit_id));
}

/* Private functions implementation --------------------------------------------*/
static uint8_t EE_16(uint16_t vaddr, EEPROM_COMMAND cmd, uint16_t *value, uint16_t *ptr) {
	uint8_t ret = 0;

	lock();

	// check if new value is same with old value
	if (cmd == EE_CMD_W) {
		if (*ptr == *value) {
			ret = 1;
		} else {
			*ptr = *value;
			// only update when value is different
			ret = EEPROM24XX_Save(vaddr, value, sizeof(uint16_t));
		}
	} else {
		ret = EEPROM24XX_Load(vaddr, value, sizeof(uint16_t));

		// restore the value
		if (ret) {
			*ptr = *value;
		}
	}

	unlock();
	return ret;
}

static uint8_t EE_32(uint16_t vaddr, EEPROM_COMMAND cmd, uint32_t *value, uint32_t *ptr) {
	uint8_t ret = 0;

	lock();

	// check if new value is same with old value
	if (cmd == EE_CMD_W) {
		if (*ptr == *value) {
			ret = 1;
		} else {
			*ptr = *value;
			// only update when value is different
			ret = EEPROM24XX_Save(vaddr, value, sizeof(uint32_t));
		}
	} else {
		ret = EEPROM24XX_Load(vaddr, value, sizeof(uint32_t));

		// restore the value
		if (ret) {
			*ptr = *value;
		}
	}

	unlock();
	return ret;
}

static void lock(void) {
	osMutexAcquire(EepromMutexHandle, osWaitForever);
}

static void unlock(void) {
	osMutexRelease(EepromMutexHandle);
}
