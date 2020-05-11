/*
 * BMS.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "BMS.h"
#include "HMI1.h"
#include "_database.h"

/* External variables ---------------------------------------------------------*/
extern hmi1_t HMI1;

/* Public variables -----------------------------------------------------------*/
bms_t BMS = {
		.d = { 0 },
		BMS_Init,
		BMS_ResetIndex,
		BMS_RefreshIndex,
		BMS_GetIndex,
		BMS_SetEvents,
		BMS_CheckRun,
		BMS_CheckState,
		BMS_MergeData,
};

/* Public functions implementation --------------------------------------------*/
void BMS_Init(void) {
	BMS.d.started = 0;
	BMS.d.soc = 0;
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		BMS_ResetIndex(i);
	}
}

void BMS_ResetIndex(uint8_t i) {
	BMS.d.pack[i].id = BMS_ID_NONE;
	BMS.d.pack[i].voltage = 0;
	BMS.d.pack[i].current = 0;
	BMS.d.pack[i].soc = 0;
	BMS.d.pack[i].temperature = 0;
	BMS.d.pack[i].state = BMS_STATE_IDLE;
	BMS.d.pack[i].started = 0;
	BMS.d.pack[i].flag = 0;
	BMS.d.pack[i].tick = 0;
}

void BMS_RefreshIndex(void) {
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		if ((osKernelGetTickCount() - BMS.d.pack[i].tick) > pdMS_TO_TICKS(500)) {
			BMS_ResetIndex(i);
		}
	}
}

uint8_t BMS_GetIndex(uint32_t id) {
	uint8_t i;

	// find index (if already exist)
	for (i = 0; i < BMS_COUNT; i++) {
		if (BMS.d.pack[i].id == id) {
			return i;
		}
	}

	// finx index (if not exist)
	for (i = 0; i < BMS_COUNT; i++) {
		if (BMS.d.pack[i].id == BMS_ID_NONE) {
			return i;
		}
	}

	// force replace first index (if already full)
	return 0;
}

void BMS_SetEvents(uint16_t flag) {
	DB_SetEvent(EV_BMS_SHORT_CIRCUIT, _R1(flag, 0));
	DB_SetEvent(EV_BMS_DISCHARGE_OVER_CURRENT, _R1(flag, 1));
	DB_SetEvent(EV_BMS_CHARGE_OVER_CURRENT, _R1(flag, 2));
	DB_SetEvent(EV_BMS_DISCHARGE_OVER_TEMPERATURE, _R1(flag, 3));
	DB_SetEvent(EV_BMS_DISCHARGE_UNDER_TEMPERATURE, _R1(flag, 4));
	DB_SetEvent(EV_BMS_CHARGE_OVER_TEMPERATURE, _R1(flag, 5));
	DB_SetEvent(EV_BMS_CHARGE_UNDER_TEMPERATURE, _R1(flag, 6));
	DB_SetEvent(EV_BMS_UNBALANCE, _R1(flag, 7));
	DB_SetEvent(EV_BMS_UNDER_VOLTAGE, _R1(flag, 8));
	DB_SetEvent(EV_BMS_OVER_VOLTAGE, _R1(flag, 9));
	DB_SetEvent(EV_BMS_OVER_DISCHARGE_CAPACITY, _R1(flag, 10));
	DB_SetEvent(EV_BMS_SYSTEM_FAILURE, _R1(flag, 11));

	// check event as CAN data
	HMI1.d.status.overheat = DB_ReadEvent(EV_BMS_DISCHARGE_OVER_TEMPERATURE) ||
			DB_ReadEvent(EV_BMS_DISCHARGE_UNDER_TEMPERATURE) ||
			DB_ReadEvent(EV_BMS_CHARGE_OVER_TEMPERATURE) ||
			DB_ReadEvent(EV_BMS_CHARGE_UNDER_TEMPERATURE);
	HMI1.d.status.warning = DB_ReadEvent(EV_BMS_SHORT_CIRCUIT) ||
			DB_ReadEvent(EV_BMS_DISCHARGE_OVER_CURRENT) ||
			DB_ReadEvent(EV_BMS_CHARGE_OVER_CURRENT) ||
			DB_ReadEvent(EV_BMS_UNBALANCE) ||
			DB_ReadEvent(EV_BMS_UNDER_VOLTAGE) ||
			DB_ReadEvent(EV_BMS_OVER_VOLTAGE) ||
			DB_ReadEvent(EV_BMS_OVER_DISCHARGE_CAPACITY) ||
			DB_ReadEvent(EV_BMS_SYSTEM_FAILURE);
}

uint8_t BMS_CheckRun(uint8_t state) {
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		if (BMS.d.pack[i].started != state) {
			return 0;
		}
	}
	return 1;
}

uint8_t BMS_CheckState(BMS_STATE state) {
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		if (BMS.d.pack[i].state != state) {
			return 0;
		}
	}
	return 1;
}

void BMS_MergeData(void) {
	uint16_t flags = 0;
	uint8_t soc = 0, device = 0;

	// Merge flags (OR-ed)
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		flags |= BMS.d.pack[i].flag;
	}
	BMS_SetEvents(flags);

	// Average SOC
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		if (BMS.d.pack[i].started == 1) {
			soc += BMS.d.pack[i].soc;
			device++;
		}
	}
	BMS.d.soc = device ? (soc / device) : soc;
}

