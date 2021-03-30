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

	HBAR.d.ctl.blink = 0;
	HBAR.d.ctl.listening = 0;
	HBAR.d.ctl.tick.blink = 0;
	HBAR.d.ctl.tick.session = 0;
	HBAR.d.ctl.tick.starter = 0;

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
		HBAR.d.ctl.tick.starter = tick ? _GetTickMS() - tick : 0;
		tick = 0;
	}
	else {
		tick = _GetTickMS();
	}
}

void HBAR_ReadStates(void) {
	HBAR.state[HBAR_K_SELECT] = GATE_ReadSelect();
	HBAR.state[HBAR_K_SET] = GATE_ReadSet();
	HBAR.state[HBAR_K_SEIN_L] = GATE_ReadSeinL();
	HBAR.state[HBAR_K_SEIN_R] = GATE_ReadSeinR();
	HBAR.state[HBAR_K_REVERSE] = GATE_ReadReverse();
	HBAR.state[HBAR_K_LAMP] = GATE_ReadLamp();
	HBAR.state[HBAR_K_ABS] = GATE_ReadABS();

	if (!HBAR.state[HBAR_K_REVERSE]) {
		if (HBAR.state[HBAR_K_SELECT] || HBAR.state[HBAR_K_SET]) {
			HBAR.d.ctl.tick.session = _GetTickMS();
			HBAR_TimerSelectSet();

			if (HBAR.state[HBAR_K_SELECT]) RunSelect();
			else RunSet();
		}
	}
}

HBAR_STARTER HBAR_RefreshStarter(vehicle_state_t lastState) {
	static uint32_t startTick = 0;
	HBAR_STARTER state = 0;

	if (HBAR.d.ctl.tick.starter && startTick != HBAR.d.ctl.tick.starter) {
		startTick = HBAR.d.ctl.tick.starter;
		if (HBAR.d.ctl.tick.starter > STARTER_LONG_PRESS && lastState > VEHICLE_NORMAL) {
			state = HBAR_STARTER_OFF;
		} else {
			state = HBAR_STARTER_ON;
		}
	}

	return state;
}

hbar_sein_t HBAR_SeinController(void) {
	static hbar_sein_t sein = {0, 0};
	static TickType_t tickSein;

	if ((_GetTickMS() - tickSein) >= 500) {
		if (HBAR.state[HBAR_K_SEIN_L] && HBAR.state[HBAR_K_SEIN_R]) {
			tickSein = _GetTickMS();
			sein.left = !sein.left;
			sein.right = sein.left;
		} else if (HBAR.state[HBAR_K_SEIN_L]) {
			tickSein = _GetTickMS();
			sein.left = !sein.left;
			sein.right = 0;
		} else if (HBAR.state[HBAR_K_SEIN_R]) {
			tickSein = _GetTickMS();
			sein.left = 0;
			sein.right = !sein.right;
		} else {
			sein.left = 0;
			sein.right = 0;
		}
	}

	return sein;
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

void HBAR_RefreshSelectSet(void) {
	if (HBAR.d.ctl.listening) {
		// stop listening
		if ((_GetTickMS() - HBAR.d.ctl.tick.session) >= MODE_SESSION_MS || HBAR.state[HBAR_K_REVERSE]) {
			HBAR.d.ctl.listening = 0;
			HBAR.d.ctl.blink = 0;
		}
		// start listening, blink
		else if ((_GetTickMS() - HBAR.d.ctl.tick.blink) >= MODE_BLINK_MS) {
			HBAR.d.ctl.tick.blink = _GetTickMS();
			HBAR.d.ctl.blink = !HBAR.d.ctl.blink;
		}
	} else {
		HBAR.d.ctl.blink = 0;
		memset(HBAR.timer, 0, sizeof(HBAR.timer));
	}
}

void HBAR_TimerSelectSet(void) {
	uint8_t keys[] = { HBAR_K_SELECT, HBAR_K_SET };
	hbar_timer_t *timer;

	for (uint8_t i = 0; i <= sizeof(keys); i++) {
		uint8_t key = keys[i];

		timer = &(HBAR.timer[key]);
		timer->time = 0;

		if (HBAR.state[key]) {
			if (!timer->running) {
				if (key == HBAR_K_SELECT || (key == HBAR_K_SET && HBAR.d.ctl.listening)) {
					timer->running = 1;
					timer->start = _GetTickMS();
				}
			}
		} else {
			if (timer->running) {
				timer->running = 0;
				timer->time = (_GetTickMS() - timer->start) / 1000;
			}
		}
	}
}

/* Private functions implementation
 * -------------------------------------------*/
static void RunSelect(void) {
	if (HBAR.d.ctl.listening) {
		if (HBAR.d.m == (HBAR_M_MAX - 1)) HBAR.d.m = 0;
		else HBAR.d.m++;
	}

	HBAR.d.ctl.listening = 1;
}

static void RunSet(void) {
	if (HBAR.d.ctl.listening) {
		if (HBAR.d.m == HBAR_M_TRIP &&
				HBAR.d.mode[HBAR.d.m] != HBAR_M_TRIP_ODO &&
				HBAR.timer[HBAR_K_SET].time > MODE_RESET_MS
		)
			HBAR.d.trip[HBAR.d.mode[HBAR.d.m]] = 0;
		else {
			if (HBAR.d.mode[HBAR.d.m] == HBAR.d.max[HBAR.d.m]) HBAR.d.mode[HBAR.d.m] = 0;
			else HBAR.d.mode[HBAR.d.m]++;
		}
	}
}
