/*
 * _hbar.h
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

#ifndef LIBS__HBAR_H_
#define LIBS__HBAR_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define MODE_SESSION_MS (uint16_t)4000
#define MODE_BLINK_MS (uint16_t) 250
#define MODE_RESET_MS (uint16_t)3000
#define STARTER_LONG_PRESS (uint16_t)2000 // in ms

/* Exported enum
 * ----------------------------------------------------------------*/
typedef enum {
	HBAR_K_SELECT = 0,
	HBAR_K_SET,
	HBAR_K_SEIN_L,
	HBAR_K_SEIN_R,
	HBAR_K_STARTER,
	HBAR_K_REVERSE,
	HBAR_K_LAMP,
	HBAR_K_ABS,
	HBAR_K_MAX = 8
} HBAR_KEY;

typedef enum {
	HBAR_M_DRIVE = 0,
	HBAR_M_TRIP,
	HBAR_M_REPORT,
	HBAR_M_MAX = 3
} HBAR_MODE;

typedef enum {
	HBAR_M_DRIVE_ECONOMY = 0,
	HBAR_M_DRIVE_STANDARD,
	HBAR_M_DRIVE_SPORT,
	HBAR_M_DRIVE_MAX = 3
} HBAR_MODE_DRIVE;

typedef enum {
	HBAR_M_TRIP_A = 0,
	HBAR_M_TRIP_B,
	HBAR_M_TRIP_ODO,
	HBAR_M_TRIP_MAX = 3
} HBAR_MODE_TRIP;

typedef enum {
	HBAR_M_REPORT_RANGE = 0,
	HBAR_M_REPORT_AVERAGE,
	HBAR_M_REPORT_MAX = 2
} HBAR_MODE_REPORT;

typedef enum {
	HBAR_STARTER_UNKNOWN = 0,
	HBAR_STARTER_ON,
	HBAR_STARTER_OFF
} HBAR_STARTER;

/* Exported struct
 * --------------------------------------------------------------*/
typedef struct {
	uint8_t left;
	uint8_t right;
} hbar_sein_t;

typedef struct {
	uint8_t listening;
	uint8_t blink;
	struct {
		uint32_t starter;
		uint32_t session;
		uint32_t blink;
	} tick;
} hbar_control_t;

typedef struct {
	hbar_sein_t sein;
	HBAR_MODE m;
	uint8_t max[HBAR_M_MAX];
	uint8_t mode[HBAR_M_MAX];
	uint8_t report[HBAR_M_REPORT_MAX];
	uint32_t trip[HBAR_M_TRIP_MAX];
} hbar_data_t;

typedef struct {
	uint32_t start;
	uint8_t running;
	uint8_t time;
} hbar_timer_t;

typedef struct {
	hbar_data_t d;
	hbar_control_t ctl;
	uint8_t state[HBAR_K_MAX];
	hbar_timer_t timer[2];
} hbar_t;



/* Exported variables
 * ---------------------------------------------------------*/
extern hbar_t HBAR;

/* Public functions prototype ------------------------------------------------*/
void HBAR_Init(void);
void HBAR_ReadStarter(void);
void HBAR_ReadStates(void);
HBAR_STARTER HBAR_RefreshStarter(vehicle_state_t lastState);
hbar_sein_t HBAR_SeinController(void);
uint32_t HBAR_AccumulateTrip(uint8_t meter);
void HBAR_RefreshSelectSet(void);
void HBAR_TimerSelectSet(void);

#endif /* LIBS__HBAR_H_ */
