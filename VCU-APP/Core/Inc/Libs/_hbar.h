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
#define MODE_TIME_GUARD (uint16_t)3000

/* Exported enum
 * ----------------------------------------------------------------*/
typedef enum {
	HBAR_K_SELECT = 0,
	HBAR_K_SET,
	HBAR_K_SEIN_LEFT,
	HBAR_K_SEIN_RIGHT,
	HBAR_K_REVERSE,
	HBAR_K_ABS,
	HBAR_K_LAMP,
	HBAR_K_MAX = 7
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

/* Exported struct
 * --------------------------------------------------------------*/
typedef struct {
	uint8_t hazard;
	uint8_t reverse;
	uint8_t listening;
	uint8_t max[HBAR_M_MAX];
	uint8_t mode[HBAR_M_MAX];
	uint8_t report[HBAR_M_REPORT_MAX];
	uint32_t trip[HBAR_M_TRIP_MAX];
} hbar_data_t;

typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
	uint8_t state;
} hbar_list_t;

typedef struct {
	uint32_t start;
	uint8_t running;
	uint8_t time;
} hbar_timer_t;

typedef struct {
	HBAR_MODE m;
	hbar_data_t d;
	hbar_list_t list[HBAR_K_MAX];
	hbar_timer_t timer[2];
} hbar_t;

typedef struct {
	uint8_t left;
	uint8_t right;
} sein_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern hbar_t HBAR;

/* Public functions prototype ------------------------------------------------*/
void HBAR_Init(void);
void HBAR_ReadStates(void);
void HBAR_SetReverse(uint8_t state);
uint32_t HBAR_AccumulateTrip(uint8_t meter);
sein_t HBAR_SeinController(void);
void HBAR_TimerSelectSet(void);
void HBAR_RunSelectOrSet(void);
uint8_t HBAR_ModeController(void);

#endif /* LIBS__HBAR_H_ */
