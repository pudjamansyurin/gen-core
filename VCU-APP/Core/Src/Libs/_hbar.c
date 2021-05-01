/*
 * _hbar.c
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_eeprom.h"
#include "Libs/_hbar.h"
#include "Nodes/MCU.h"
#include "Nodes/BMS.h"

/* Public variables
 * -----------------------------------------------------------*/
hbar_t HBAR = {
		.state = {0},
};

/* Private functions prototype -----------------------------------------------*/
static uint8_t Reversed(void);
static void RunSelect(void);
static void RunSet(void);

/* Public functions implementation
 * --------------------------------------------*/
void HBAR_Init(void) {
	memset(&(HBAR.timer), 0, sizeof(HBAR.timer));

	HBAR.ctl.starter = HBAR_STARTER_UNKNOWN;
	HBAR.ctl.session = 0;
	HBAR.ctl.tick.starter = 0;
	HBAR.ctl.tick.session = 0;

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

void HBAR_ReadStarter(uint8_t normalState) {
	HBAR.state[HBAR_K_STARTER] = GATE_ReadStarter();

	if (HBAR.state[HBAR_K_STARTER] && !HBAR.ctl.tick.starter)
		HBAR.ctl.tick.starter = _GetTickMS();
	else if (!HBAR.state[HBAR_K_STARTER] && HBAR.ctl.tick.starter) {
		HBAR_STARTER starterState;
		uint8_t short_press;

		short_press = (_GetTickMS() - HBAR.ctl.tick.starter) < STARTER_LONG_PRESS_MS;
		if (short_press || normalState)
			starterState = HBAR_STARTER_ON;
		else
			starterState = HBAR_STARTER_OFF;

		HBAR.ctl.starter = starterState;
		HBAR.ctl.tick.starter = 0;
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

	if (!Reversed()) {
		HBAR_TimerSelectSet();
		HBAR_HandleSelectSet();
	}
}

void HBAR_HandleSelectSet(void) {
	if (HBAR.timer[HBAR_K_SELECT].time)
		HBAR.ctl.session++;

	if (HBAR.ctl.session) {
		if (HBAR.timer[HBAR_K_SELECT].time || HBAR.timer[HBAR_K_SET].time) {
			HBAR.ctl.tick.session = _GetTickMS();
			if (HBAR.timer[HBAR_K_SELECT].time && HBAR.ctl.session > 1)
				RunSelect();
			if (HBAR.timer[HBAR_K_SET].time)
				RunSet();
		}
	}
}

void HBAR_TimerSelectSet(void) {
	uint8_t keys[] = { HBAR_K_SELECT, HBAR_K_SET };

	for (uint8_t i = 0; i < sizeof(keys); i++) {
		uint8_t key = keys[i];
		hbar_timer_t *t = &(HBAR.timer[key]);

		t->time = 0;
		if (HBAR.state[key] && !t->start) {
			if (key == HBAR_K_SELECT || (key == HBAR_K_SET && HBAR.ctl.session)) {
				t->start = _GetTickMS();
			}
		}
		else if (!HBAR.state[key] && t->start) {
			t->time = _GetTickMS() - t->start;
			t->start = 0;
		}
	}
}

void HBAR_RefreshSelectSet(void) {
	if (HBAR.ctl.session) {
		// stop session
		if ((_GetTickMS() - HBAR.ctl.tick.session) >= MODE_SESSION_MS || Reversed()) {
			HBAR.ctl.session = 0;
			memset(HBAR.timer, 0, sizeof(HBAR.timer));
		}
	}
}

hbar_sein_t HBAR_SeinController(void) {
	static hbar_sein_t sein = {0, 0};
	static TickType_t tickSein;

	if ((_GetTickMS() - tickSein) >= 250) {
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

uint16_t HBAR_AccumulateTrip(uint8_t km) {
	HBAR_MODE_TRIP mTrip = HBAR.d.mode[HBAR_M_TRIP];
	uint16_t *trip = &(HBAR.d.trip[mTrip]);

	*trip += km;

	if (mTrip != HBAR_M_TRIP_ODO)
		HBAR.d.trip[HBAR_M_TRIP_ODO] += km;

	return HBAR.d.trip[HBAR_M_TRIP_ODO];
}

void HBAR_SetOdometer(uint8_t m) {
	static uint8_t init = 1;
	static uint32_t odo_m = 0;
	uint16_t *odo_km = &(HBAR.d.trip[HBAR_M_TRIP_ODO]);

	if (init) {
		init = 0;
		odo_m = ((*odo_km) * 1000);
	} else {
		odo_m += m;

		uint8_t km = odo_m / 1000;
		if (km > (*odo_km)) {
			uint8_t odom = HBAR_AccumulateTrip(km - (*odo_km));
			EEPROM_Odometer(EE_CMD_W, odom);
		}

		{
			float km_kwh = BMS.GetKmPerKwh(odo_m);
			HBAR.d.report[HBAR_M_REPORT_AVERAGE] = km_kwh;
			HBAR.d.report[HBAR_M_REPORT_RANGE] = BMS.GetMinKWH() * km_kwh;
		}
	}
}

/* Private functions implementation
 * -------------------------------------------*/
static uint8_t Reversed(void) {
	//	return HBAR.state[HBAR_K_REVERSE];
	return MCU.Reversed();
}

static void RunSelect(void) {
	if (HBAR.d.m == (HBAR_M_MAX - 1)) HBAR.d.m = 0;
	else HBAR.d.m++;
}

static void RunSet(void) {
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
