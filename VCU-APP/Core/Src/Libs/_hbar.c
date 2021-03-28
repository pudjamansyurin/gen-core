/*
 * _hbar.c
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_hbar.h"
#include "Nodes/VCU.h"
#include "_defines.h"

/* Public variables
 * -----------------------------------------------------------*/
hbar_t HBAR = {
		.state = {0},
};

/* Private functions prototype -----------------------------------------------*/
static void RunSelect(void);
static void RunSet(void);

/* Public functions implementation
 * --------------------------------------------*/
void HBAR_Init(void) {
	for (uint8_t i = 0; i < sizeof(HBAR.timer) / sizeof(HBAR.timer[0]); i++) {
		HBAR.timer[i].start = 0;
		HBAR.timer[i].running = 0;
		HBAR.timer[i].time = 0;
	}

	HBAR.d.starter_tick = 0;
	HBAR.d.listening = 0;

	HBAR.d.m = HBAR_M_DRIVE;
	HBAR.d.mode[HBAR_M_TRIP] = HBAR_M_TRIP_ODO;
	HBAR.d.mode[HBAR_M_DRIVE] = HBAR_M_DRIVE_STANDARD;
	HBAR.d.mode[HBAR_M_REPORT] = HBAR_M_REPORT_RANGE;

	HBAR.d.max[HBAR_M_DRIVE] = HBAR_M_DRIVE_MAX - 1;
	HBAR.d.max[HBAR_M_TRIP] = HBAR_M_TRIP_MAX - 1;
	HBAR.d.max[HBAR_M_REPORT] = HBAR_M_REPORT_MAX - 1;

	HBAR.d.report[HBAR_M_REPORT_RANGE] = 0;
	HBAR.d.report[HBAR_M_REPORT_AVERAGE] = 0;

	HBAR.d.trip[HBAR_M_TRIP_A] = 0;
	HBAR.d.trip[HBAR_M_TRIP_B] = 0;
	//	HBAR.d.trip[HBAR_M_TRIP_ODO] = 0;
}

void HBAR_ReadStarter(void) {
	static uint32_t tick = 0;

	HBAR.state[HBAR_K_STARTER] = GATE_ReadStarter();
	if (!HBAR.state[HBAR_K_STARTER]) {
		HBAR.d.starter_tick = tick ? _GetTickMS() - tick : 0;
		tick = 0;
	}
	else tick = _GetTickMS();
}

void HBAR_ReadStates(void) {
	HBAR.state[HBAR_K_SELECT] = GATE_ReadSelect();
	HBAR.state[HBAR_K_SET] = GATE_ReadSet();
	HBAR.state[HBAR_K_SEIN_L] = GATE_ReadSeinL();
	HBAR.state[HBAR_K_SEIN_R] = GATE_ReadSeinR();
	HBAR.state[HBAR_K_REVERSE] = GATE_ReadReverse();
	HBAR.state[HBAR_K_LAMP] = GATE_ReadLamp();
	HBAR.state[HBAR_K_ABS] = GATE_ReadABS();

	HBAR_TimerSelectSet();
	HBAR_RunSelectOrSet();
}

uint32_t HBAR_AccumulateTrip(uint8_t meter) {
	HBAR_MODE_TRIP mTrip = HBAR.d.mode[HBAR_M_TRIP];
	uint32_t *trip = &(HBAR.d.trip[mTrip]);
	uint32_t limit;

	limit = (mTrip == HBAR_M_TRIP_ODO) ? ODOMETER_MAX : UINT16_MAX;

	if ((*trip / 1000) > limit) *trip = meter;
	else *trip += meter;

	HBAR.d.trip[HBAR_M_TRIP_ODO] += meter;

	return HBAR.d.trip[HBAR_M_TRIP_ODO];
}

uint8_t HBAR_ModeController(void) {
	static int8_t iName = -1, iValue = -1;
	static TickType_t tick, tickPeriod;
	static uint8_t iHide = 0;

	// MODE Show/Hide Manipulator
	if (HBAR.d.listening) {
		// mode changed
		if (iName != HBAR.d.m) {
			iName = HBAR.d.m;
			tickPeriod = _GetTickMS();
		}
		// value changed
		else if (iValue != HBAR.d.mode[HBAR.d.m]) {
			iValue = HBAR.d.mode[HBAR.d.m];
			tickPeriod = _GetTickMS();
		}

		// stop listening
		if ((_GetTickMS() - tickPeriod) >= MODE_TIME_GUARD || HBAR.state[HBAR_K_REVERSE]) {
			HBAR.d.listening = 0;
			iHide = 0;
			iName = -1;
			iValue = -1;
		}
		// start listening, blink
		else if ((_GetTickMS() - tick) >= 250) {
			tick = _GetTickMS();
			iHide = !iHide;
		}
	} else
		iHide = 0;

	return iHide;
}

void HBAR_TimerSelectSet(void) {
	uint8_t keys[] = { HBAR_K_SELECT, HBAR_K_SET };
	hbar_timer_t *timer;

	for (uint8_t i = 0; i <= sizeof(keys); i++) {
		uint8_t key = keys[i];

		timer = &(HBAR.timer[key]);
		timer->time = 0;

		// next job
		if (HBAR.state[key]) {
			if (!timer->running) {
				if (key == HBAR_K_SELECT || (key == HBAR_K_SET && HBAR.d.listening)) {
					timer->running = 1;
					timer->start = _GetTickMS();
				}
			}
			// reverse it
			// HBAR.state[key] = 0;

		} else if (timer->running) {
			timer->running = 0;
			timer->time = (_GetTickMS() - timer->start) / 1000;
			// reverse it
			// HBAR.state[key] = 1;
		}
	}
}

void HBAR_RunSelectOrSet(void) {
	if (HBAR.state[HBAR_K_REVERSE]) return;

	if (HBAR.state[HBAR_K_SELECT]) RunSelect();
	else if (HBAR.state[HBAR_K_SET]) RunSet();
}

/* Private functions implementation
 * -------------------------------------------*/
static void RunSelect(void) {
	if (HBAR.d.listening) {
		if (HBAR.d.m == (HBAR_M_MAX - 1)) HBAR.d.m = 0;
		else HBAR.d.m++;
	}
	HBAR.d.listening = 1;
}

static void RunSet(void) {
	if (HBAR.d.listening) {
		if (HBAR.d.mode[HBAR.d.m] == HBAR.d.max[HBAR.d.m])
			HBAR.d.mode[HBAR.d.m] = 0;
		else
			HBAR.d.mode[HBAR.d.m]++;
	}

	else {
		if (HBAR.timer[HBAR_K_SET].time >= 3)
			if (HBAR.d.m == HBAR_M_TRIP && HBAR.d.mode[HBAR.d.m] != HBAR_M_TRIP_ODO)
				HBAR.d.trip[HBAR.d.mode[HBAR.d.m]] = 0;
	}
}
