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
		.d = {0},
		.ctl = {0}
};

/* Private functions prototype -----------------------------------------------*/
static uint32_t Timer(uint8_t key);
static uint8_t Reversed(void);
static void RunSelect(void);
static void RunSet(void);

/* Public functions implementation
 * --------------------------------------------*/
void HBAR_Init(void) {
	memset(&(HBAR.tim), 0, sizeof(HBAR.tim));

	HBAR.ctl.starter = HBAR_STARTER_UNKNOWN;
	HBAR.ctl.session = 0;
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
	HBAR.d.pin[HBAR_K_STARTER] = GATE_ReadStarter();

	if (Timer(HBAR_K_STARTER)) {
		uint8_t on = (HBAR.tim[HBAR_K_STARTER].time < STARTER_LONG_PRESS_MS) || normalState;

		HBAR.ctl.starter = on ? HBAR_STARTER_ON : HBAR_STARTER_OFF;
	}
}

void HBAR_ReadStates(void) {
	HBAR.d.pin[HBAR_K_SELECT] = GATE_ReadSelect();
	HBAR.d.pin[HBAR_K_SET] = GATE_ReadSet();
	HBAR.d.pin[HBAR_K_SEIN_L] = GATE_ReadSeinL();
	HBAR.d.pin[HBAR_K_SEIN_R] = GATE_ReadSeinR();
	HBAR.d.pin[HBAR_K_REVERSE] = GATE_ReadReverse();
	HBAR.d.pin[HBAR_K_LAMP] = GATE_ReadLamp();
	HBAR.d.pin[HBAR_K_ABS] = GATE_ReadABS();

	if (!Reversed()) {
		if (Timer(HBAR_K_SELECT))
			HBAR.ctl.session++;

		if (HBAR.ctl.session) {
			Timer(HBAR_K_SET);

			if (HBAR.tim[HBAR_K_SELECT].time || HBAR.tim[HBAR_K_SET].time) {
				HBAR.ctl.tick.session = _GetTickMS();
				if (HBAR.tim[HBAR_K_SELECT].time && HBAR.ctl.session > 1)
					RunSelect();
				if (HBAR.tim[HBAR_K_SET].time)
					RunSet();
			}

		}
	}
}

void HBAR_RefreshSelectSet(void) {
	if (HBAR.ctl.session) {
		if ((_GetTickMS() - HBAR.ctl.tick.session) >= MODE_SESSION_MS || Reversed()) {
			HBAR.ctl.session = 0;
			memset(&(HBAR.tim[HBAR_K_SELECT]), 0, sizeof(hbar_timer_t));
			memset(&(HBAR.tim[HBAR_K_SET]), 0, sizeof(hbar_timer_t));
		}
	}
}

void HBAR_RefreshSein(void) {
	hbar_sein_t *sein = &(HBAR.ctl.sein);

	if ((_GetTickMS() - HBAR.ctl.tick.sein) >= 250) {
		if (HBAR.d.pin[HBAR_K_SEIN_L] || HBAR.d.pin[HBAR_K_SEIN_R])
			HBAR.ctl.tick.sein = _GetTickMS();

		if (HBAR.d.pin[HBAR_K_SEIN_L] && HBAR.d.pin[HBAR_K_SEIN_R]) {
			sein->left = !sein->left;
			sein->right = sein->left;
		} else if (HBAR.d.pin[HBAR_K_SEIN_L]) {
			sein->left = !sein->left;
			sein->right = 0;
		} else if (HBAR.d.pin[HBAR_K_SEIN_R]) {
			sein->left = 0;
			sein->right = !sein->right;
		} else {
			sein->left = 0;
			sein->right = 0;
		}
	}
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
	uint16_t *odo_km = &(HBAR.d.trip[HBAR_M_TRIP_ODO]);

	if (init) {
		init = 0;
		HBAR.d.odometer = ((*odo_km) * 1000);
	} else {
		HBAR.d.odometer += m;

		{
			float m_wh = BMS.GetMPerWH(HBAR.d.odometer);
			float km = m_wh * BMS.GetMinWH();

			HBAR.d.report[HBAR_M_REPORT_AVERAGE] = m_wh;
			HBAR.d.report[HBAR_M_REPORT_RANGE] = km;
		}

		if ((HBAR.d.odometer / 1000) > (*odo_km)) {
			uint8_t d_km = (HBAR.d.odometer / 1000) - (*odo_km);
			uint16_t odom = HBAR_AccumulateTrip(d_km);
			EEPROM_Odometer(EE_CMD_W, odom);
		}
	}
}

/* Private functions implementation
 * -------------------------------------------*/
static uint32_t Timer(uint8_t key) {
	hbar_timer_t *tim = &(HBAR.tim[key]);
	uint8_t *pin = &(HBAR.d.pin[key]);

	tim->time = 0;
	if (*pin && !tim->start) {
		tim->start = _GetTickMS();
	} else if (!(*pin) && tim->start) {
		tim->time = _GetTickMS() - tim->start;
		tim->start = 0;
	}

	return tim->time;
}

static uint8_t Reversed(void) {
	//	return HBAR.d.pin[HBAR_K_REVERSE];
	return MCU.Reversed();
}

static void RunSelect(void) {
	if (HBAR.d.m == (HBAR_M_MAX - 1))
		HBAR.d.m = 0;
	else HBAR.d.m++;
}

static void RunSet(void) {
	if (HBAR.d.m == HBAR_M_TRIP && HBAR.d.mode[HBAR.d.m] != HBAR_M_TRIP_ODO && HBAR.tim[HBAR_K_SET].time > MODE_RESET_MS )
		HBAR.d.trip[HBAR.d.mode[HBAR.d.m]] = 0;
	else {
		if (HBAR.d.mode[HBAR.d.m] == HBAR.d.max[HBAR.d.m])
			HBAR.d.mode[HBAR.d.m] = 0;
		else HBAR.d.mode[HBAR.d.m]++;
	}
}
