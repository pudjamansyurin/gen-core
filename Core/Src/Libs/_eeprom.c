/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_eeprom.h"
#include "Drivers/_eeprom24xx.h"
#include "Libs/_reporter.h"
#include "Nodes/VCU.h"

/* External variabless -------------------------------------------------------*/
extern osMutexId_t EepromMutexHandle;
extern response_t RESPONSE;
extern report_t REPORT;
extern vcu_t VCU;

/* Private functions prototype ------------------------------------------------*/
static uint8_t EE_Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size);
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
uint8_t EEPROM_Init(void) {
	const uint8_t MAX_RETRY = 5;
	uint8_t retry, ret = 0;

	lock();
	LOG_StrLn("EEPROM:Init");
	// check main eeprom
	retry = MAX_RETRY;
	EEPROM24XX_SetDevice(EEPROM24_MAIN);
	do {
		if (EEPROM24XX_IsConnected()) {
			LOG_StrLn("EEPROM:Main");
			ret = 1;
			break;
		}
		osDelay(500);
	} while (retry--);

	// check backup eeprom
	if (!ret) {
		retry = MAX_RETRY;
		EEPROM24XX_SetDevice(EEPROM24_BACKUP);
		do {
			if (EEPROM24XX_IsConnected()) {
				LOG_StrLn("EEPROM:Backup");
				ret = 1;
				break;
			}
			osDelay(500);
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

	ret = EE_Command(VADDR_RESET, cmd, &value, &temp, sizeof(value));

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

		// re-write eeprom
		EEPROM_Reset(EE_CMD_W, EEPROM_RESET);
	}

}

uint8_t EEPROM_SequentialID(EEPROM_COMMAND cmd, uint16_t value, PAYLOAD_TYPE type) {
	uint16_t *pSeqId;
	uint32_t vaddr;

	// decide payload type
	if (type == PAYLOAD_REPORT) {
		pSeqId = &(VCU.d.seq_id.report);
		vaddr = VADDR_REPORT_SEQ_ID;
	} else {
		pSeqId = &(VCU.d.seq_id.response);
		vaddr = VADDR_RESPONSE_SEQ_ID;
	}

	return EE_Command(vaddr, cmd, &value, pSeqId, sizeof(value));
}

uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t value) {
	// reset on overflow
	if (value > VCU_ODOMETER_MAX) {
		value = 0;
	}
	return EE_Command(VADDR_ODOMETER, cmd, &value, &(VCU.d.odometer), sizeof(value));
}

uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t value) {
	return EE_Command(VADDR_UNITID, cmd, &value, &(VCU.d.unit_id), sizeof(value));
}

/* Private functions implementation --------------------------------------------*/
static uint8_t EE_Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size) {
	uint8_t ret = 0;

	lock();

	// check if new value is same with old value
	if (cmd == EE_CMD_W) {
		memcpy(ptr, value, size);
		// save the value
		ret = EEPROM24XX_Save(vaddr, value, size);
	} else {
		// load the value
		ret = EEPROM24XX_Load(vaddr, value, size);
		// apply the value
		if (ret) {
			memcpy(ptr, value, size);
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
