/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "VCU.h"
#include "_utils.h"

/* Public variables -----------------------------------------------------------*/
vcu_t VCU = {
		.d = { 0 },
		VCU_Init,
		VCU_SetEvent,
		VCU_ReadEvent,
		VCU_CheckMainPower,
};

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void) {
	// reset VCU data
	VCU.d.knob = 0;
	VCU.d.independent = 1;
	VCU.d.interval = RPT_INTERVAL_SIMPLE;
	VCU.d.volume = 0;
	VCU.d.bat_voltage = 0;
	VCU.d.signal_percent = 0;
	VCU.d.speed = 0;
	VCU.d.odometer = 0;
	VCU.d.events = 0;
	VCU.d.tick.keyless = 0;
	//  VCU.d.tick.finger = 0;
	VCU.d.seq_id.report = 0;
	VCU.d.seq_id.response = 0;
}

void VCU_SetEvent(uint64_t event_id, uint8_t value) {
	if (value & 1) {
		BV(VCU.d.events, _BitPosition(event_id));
	} else {
		BC(VCU.d.events, _BitPosition(event_id));
	}
}

uint8_t VCU_ReadEvent(uint64_t event_id) {
	return (VCU.d.events & event_id) == event_id;
}

void VCU_CheckMainPower(void) {
	VCU.d.independent = !HAL_GPIO_ReadPin(EXT_BMS_IRQ_GPIO_Port, EXT_BMS_IRQ_Pin);
	VCU.d.interval = VCU.d.independent ? RPT_INTERVAL_INDEPENDENT : RPT_INTERVAL_SIMPLE;
	VCU.SetEvent(EV_VCU_INDEPENDENT, VCU.d.independent);
}
