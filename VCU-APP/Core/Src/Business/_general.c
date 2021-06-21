/*
 * _general.c
 *
 *  Created on: Jun 21, 2021
 *      Author: pudja
 */


/* Includes ------------------------------------------------------------------*/
#include "Business/_general.h"
#include "Nodes/MCU.h"
#include "Nodes/BMS.h"

/* Private functions declaration
 * --------------------------------------------*/
static uint8_t GEN_CalcDistance(uint32_t dms);

/* Public functions implementation
 * --------------------------------------------*/
void GEN_RangePrediction(void) {
	static uint32_t tick = 0;
	uint8_t distance, eff, km;

	if (_TickOut(tick, 1000)) {
		distance = GEN_CalcDistance(_GetTickMS() - tick);
		tick = _GetTickMS();

		BMS_GetPrediction(&eff, &km, distance);
		HBAR_SetReport(eff, km);
		HBAR_AddTripMeter(distance);
	} else if (tick == 0)
		tick = _GetTickMS();
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t GEN_CalcDistance(uint32_t dms) {
	uint8_t meter;
	float mps;

	mps = (float) MCU_RpmToSpeed(MCU.d.rpm) / 3.6;
	meter = (dms * mps) / 1000;

	return meter;
}
