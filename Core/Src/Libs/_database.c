/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_database.h"
#include "_reporter.h"
#include "BMS.h"
#include "HMI1.h"
#include "HMI2.h"

/* External variables ---------------------------------------------------------*/
extern bms_t BMS;
extern hmi1_t HMI1;
extern hmi2_t HMI2;

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

	// Others
	HMI1.Init();
	HMI2.Init();
	BMS.Init();
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

void DB_VCU_CheckMainPower(void) {
	DB.vcu.independent = !HAL_GPIO_ReadPin(EXT_BMS_IRQ_GPIO_Port, EXT_BMS_IRQ_Pin);
	DB.vcu.interval = DB.vcu.independent ? RPT_INTERVAL_INDEPENDENT : RPT_INTERVAL_SIMPLE;
	DB_SetEvent(EV_VCU_INDEPENDENT, DB.vcu.independent);
}

