/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_database.h"
#include "_reporter.h"

/* Public variables -----------------------------------------------------------*/
db_t DB;

/* Public functions implementation --------------------------------------------*/
void DB_Init(void) {
	// reset VCU data
	DB.vcu.knob = 0;
	DB.vcu.independent = 1;
	DB.vcu.interval = RPT_INTERVAL_SIMPLE;
	DB.vcu.volume = 0;
	DB.vcu.bat_voltage = 0;
	DB.vcu.signal_percent = 0;
	DB.vcu.speed = 0;
	DB.vcu.odometer = 0;
	DB.vcu.events = 0;
	DB.vcu.tick.keyless = 0;
	//  DB.vcu.tick.finger = 0;
	DB.vcu.seq_id.report = 0;
	DB.vcu.seq_id.response = 0;

	// reset HMI1 data
	DB.hmi1.started = 0;
	DB.hmi1.status.mirroring = 0;
	DB.hmi1.status.warning = 0;
	DB.hmi1.status.overheat = 0;
	DB.hmi1.status.finger = 0;
	DB.hmi1.status.keyless = 0;
	DB.hmi1.status.daylight = 0;
	for (uint8_t i = 0; i < HMI1_DEV_MAX; i++) {
		DB.hmi1.device[i].started = 0;
		DB.hmi1.device[i].tick = 0;
	}

	// reset HMI2 data
	DB.hmi2.started = 0;
	DB.hmi2.tick = 0;

	// reset BMS data
	BMS_Init();
}

void DB_SetEvent(uint64_t event_id, uint8_t value) {
	if (value & 1) {
		BV(DB.vcu.events, _BitPosition(event_id));
	} else {
		BC(DB.vcu.events, _BitPosition(event_id));
	}
}

uint8_t DB_ReadEvent(uint64_t event_id) {
	return (DB.vcu.events & event_id) == event_id;
}

void DB_HMI1_RefreshIndex(void) {
	for (uint8_t i = 0; i < HMI1_DEV_MAX; i++) {
		if ((osKernelGetTickCount() - DB.hmi1.device[i].tick) > pdMS_TO_TICKS(1000)) {
			DB.hmi1.device[i].started = 0;
		}
	}
}

void DB_VCU_CheckBMSPresence(void) {
	DB.vcu.independent = !HAL_GPIO_ReadPin(EXT_BMS_IRQ_GPIO_Port, EXT_BMS_IRQ_Pin);
	DB.vcu.interval = DB.vcu.independent ? RPT_INTERVAL_INDEPENDENT : RPT_INTERVAL_SIMPLE;
	DB_SetEvent(EV_VCU_INDEPENDENT, DB.vcu.independent);
}

uint8_t DB_ValidThreadFlag(uint32_t flag) {
	uint8_t ret = 1;

	// check is empty
	if (!flag) {
		ret = 0;
	} else if (flag & (~EVT_MASK)) {
		// error
		ret = 0;
	}

	return ret;
}

uint8_t DB_ValidEventFlag(uint32_t flag) {
	uint8_t ret = 1;

	// check is empty
	if (!flag) {
		ret = 0;
	} else if (flag & (~EVENT_MASK)) {
		// error
		ret = 0;
	}

	return ret;
}

//void DB_SetEvents(uint64_t value) {
//  DB.vcu.events = value;
//}
//
//uint8_t DB_ReadEvent(uint64_t event_id) {
//  return _R8((DB.vcu.events & event_id), _BitPosition(event_id));
//}

