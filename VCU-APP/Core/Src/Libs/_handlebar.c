/*
 * _handlebar.c
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_handlebar.h"
#include "Nodes/VCU.h"
#include "_defines.h"

/* Public variables
 * -----------------------------------------------------------*/
hbar_t HBAR = {
		.list =
		{{.pin = EXT_HBAR_SELECT_Pin,
				.port = EXT_HBAR_SELECT_GPIO_Port,
				.state = 0},
				{.pin = EXT_HBAR_SET_Pin, .port = EXT_HBAR_SET_GPIO_Port, .state = 0},
				{.pin = EXT_HBAR_SEIN_L_Pin,
						.port = EXT_HBAR_SEIN_L_GPIO_Port,
						.state = 0},
						{.pin = EXT_HBAR_SEIN_R_Pin,
								.port = EXT_HBAR_SEIN_R_GPIO_Port,
								.state = 0},
								{.pin = EXT_HBAR_REVERSE_Pin,
										.port = EXT_HBAR_REVERSE_GPIO_Port,
										.state = 0},
										{.pin = EXT_ABS_IRQ_Pin, .port = EXT_ABS_IRQ_GPIO_Port, .state = 0},
										{.pin = EXT_HBAR_LAMP_Pin,
												.port = EXT_HBAR_LAMP_GPIO_Port,
												.state = 0}},
};

/* Private functions prototype -----------------------------------------------*/
static void RunSelect(void);
static void RunSet(void);

/* Public functions implementation
 * --------------------------------------------*/
void HBAR_Init(void) {
	uint8_t i;

	for (i = 0; i < HBAR_K_MAX; i++)
		HBAR.list[i].state = 0;

	for (i = 0; i < sizeof(HBAR.timer) / sizeof(HBAR.timer[0]); i++) {
		HBAR.timer[i].start = 0;
		HBAR.timer[i].running = 0;
		HBAR.timer[i].time = 0;
	}

	HBAR.d.hazard = 0;
	HBAR.d.reverse = 0;
	HBAR.d.listening = 0;

	HBAR.m = HBAR_M_DRIVE;
	HBAR.d.mode[HBAR_M_TRIP] = HBAR_M_TRIP_ODO;
	HBAR.d.mode[HBAR_M_DRIVE] = HBAR_M_DRIVE_STANDARD;
	HBAR.d.mode[HBAR_M_REPORT] = HBAR_M_REPORT_RANGE;

	HBAR.d.max[HBAR_M_DRIVE] = HBAR_M_DRIVE_MAX - 1;
	HBAR.d.max[HBAR_M_TRIP] = HBAR_M_TRIP_MAX - 1;
	HBAR.d.max[HBAR_M_REPORT] = HBAR_M_REPORT_MAX - 1;

	HBAR.d.report[HBAR_M_REPORT_RANGE] = 0;
	HBAR.d.report[HBAR_M_REPORT_AVERAGE] = 0;

	//	HBAR.d.trip[HBAR_M_TRIP_ODO] = 0;
	HBAR.d.trip[HBAR_M_TRIP_A] = 0;
	HBAR.d.trip[HBAR_M_TRIP_B] = 0;
}

void HBAR_ReadStates(void) {
	hbar_list_t *list;

	for (uint8_t i = 0; i < HBAR_K_MAX; i++) {
		list = &(HBAR.list[i]);
		list->state = HAL_GPIO_ReadPin(list->port, list->pin);
	}
	HBAR_SetReverse(HBAR.list[HBAR_K_REVERSE].state);
}

void HBAR_SetReverse(uint8_t state) {
	HBAR.d.reverse = state;
	HBAR.d.hazard = HBAR.d.reverse;
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

sein_t HBAR_SeinController(void) {
	static sein_t sein = {0, 0};
	static TickType_t tickSein;

	if ((_GetTickMS() - tickSein) >= 500) {
		if (HBAR.d.hazard) {
			tickSein = _GetTickMS();
			sein.left = !sein.left;
			sein.right = !sein.right;
		} else if (HBAR.list[HBAR_K_SEIN_LEFT].state) {
			tickSein = _GetTickMS();
			sein.left = !sein.left;
			sein.right = 0;
		} else if (HBAR.list[HBAR_K_SEIN_RIGHT].state) {
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

void HBAR_TimerSelectSet(void) {
	hbar_list_t *list;
	hbar_timer_t *timer;

	for (uint8_t i = HBAR_K_SELECT; i <= HBAR_K_SET; i++) {
		list = &(HBAR.list[i]);
		timer = &(HBAR.timer[i]);
		timer->time = 0;

		// next job
		if (list->state) {
			if (!timer->running) {
				if (i == HBAR_K_SELECT || (i == HBAR_K_SET && HBAR.d.listening)) {
					timer->running = 1;
					timer->start = _GetTickMS();
				}
			}
			// reverse it
			list->state = 0;

		} else {
			if (timer->running) {
				timer->running = 0;
				timer->time = (_GetTickMS() - timer->start) / 1000;
				// reverse it
				list->state = 1;
			}
		}
	}
}

uint8_t HBAR_ModeController(void) {
	static int8_t iName = -1, iValue = -1;
	static TickType_t tick, tickPeriod;
	static uint8_t iHide = 0;

	// MODE Show/Hide Manipulator
	if (HBAR.d.listening) {
		// mode changed
		if (iName != HBAR.m) {
			iName = HBAR.m;
			tickPeriod = _GetTickMS();
		}
		// value changed
		else if (iValue != HBAR.d.mode[HBAR.m]) {
			iValue = HBAR.d.mode[HBAR.m];
			tickPeriod = _GetTickMS();
		}

		// stop listening
		if ((_GetTickMS() - tickPeriod) >= MODE_TIME_GUARD || HBAR.d.reverse) {
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

void HBAR_RunSelectOrSet(void) {
	if (!HBAR.list[HBAR_K_REVERSE].state) {
		if (HBAR.list[HBAR_K_SELECT].state)
			RunSelect();
		else if (HBAR.list[HBAR_K_SET].state)
			RunSet();
	}
}

/* Private functions implementation
 * -------------------------------------------*/
static void RunSelect(void) {
	if (HBAR.d.listening) {
		if (HBAR.m == (HBAR_M_MAX - 1)) HBAR.m = 0;
		else HBAR.m++;
	}
	HBAR.d.listening = 1;
}

static void RunSet(void) {
	if (HBAR.d.listening) {
		if (HBAR.d.mode[HBAR.m] == HBAR.d.max[HBAR.m])
			HBAR.d.mode[HBAR.m] = 0;
		else
			HBAR.d.mode[HBAR.m]++;
	}

	else {
		if (HBAR.timer[HBAR_K_SET].time >= 3)
			if (HBAR.m == HBAR_M_TRIP)
				if (HBAR.d.mode[HBAR.m] != HBAR_M_TRIP_ODO)
					HBAR.d.trip[HBAR.d.mode[HBAR.m]] = 0;
	}
}
