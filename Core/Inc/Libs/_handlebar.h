/*
 * _handlebar.h
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

#ifndef LIBS__HANDLEBAR_H_
#define LIBS__HANDLEBAR_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported constants --------------------------------------------------------*/
// EXTI list
#define SW_TOTAL_LIST                           7
#define SW_K_SELECT                             0
#define SW_K_SET                                1
#define SW_K_SEIN_LEFT                          2
#define SW_K_SEIN_RIGHT                         3
#define SW_K_REVERSE                            4
#define SW_K_ABS                                5
#define SW_K_LAMP                               6

/* Exported enum ----------------------------------------------------------------*/
typedef enum {
	SW_M_DRIVE = 0,
	SW_M_TRIP = 1,
	SW_M_REPORT = 2,
	SW_M_MAX = 2
} SW_MODE;

typedef enum {
	SW_M_DRIVE_E = 0,
	SW_M_DRIVE_S = 1,
	SW_M_DRIVE_P = 2,
	SW_M_DRIVE_R = 3,
	SW_M_DRIVE_MAX = 2
} SW_MODE_DRIVE;

typedef enum {
	SW_M_TRIP_A = 0,
	SW_M_TRIP_B = 1,
	SW_M_TRIP_MAX = 1
} SW_MODE_TRIP;

typedef enum {
	SW_M_REPORT_RANGE = 0,
	SW_M_REPORT_EFFICIENCY = 1,
	SW_M_REPORT_MAX = 1
} SW_MODE_REPORT;

/* Exported struct --------------------------------------------------------------*/
typedef struct {
	uint8_t listening;
	struct {
		SW_MODE val;
		struct {
			uint8_t val[SW_M_MAX + 1];
			uint8_t max[SW_M_MAX + 1];
			uint8_t report[SW_M_REPORT_MAX + 1];
			uint32_t trip[SW_M_TRIP_MAX + 1];
		} sub;
	} mode;
} sw_runner_t;

typedef struct {
	struct {
		char event[20];
		uint16_t pin;
		GPIO_TypeDef *port;
		uint8_t state;
	} list[SW_TOTAL_LIST];
	struct {
		uint32_t start;
		uint8_t running;
		uint8_t time;
	} timer[2];
	sw_runner_t runner;
} sw_t;

/* Public functions prototype ------------------------------------------------*/
void HBAR_ReadStates(void);
void HBAR_RestoreMode(void);
void HBAR_CheckReverse(void);
void HBAR_CheckSelectSet(void);
void HBAR_RunSelect(void);
void HBAR_RunSet(void);
void HBAR_AccumulateSubTrip(void);

#endif /* LIBS__HANDLEBAR_H_ */
