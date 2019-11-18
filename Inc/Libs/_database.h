/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include "main.h"
#include "_config.h"
#include "_rtc.h"

// exti list
#define IDX_KEY_SELECT 							0
#define IDX_KEY_SET 								1
#define IDX_KEY_SEIN_LEFT 					2
#define IDX_KEY_SEIN_RIGHT 					3
#define IDX_KEY_REVERSE		 					4
#define IDX_KEY_ABS				 					5
#define IDX_KEY_MIRRORING 					6

// enum list
typedef enum {
	SWITCH_MODE_DRIVE = 0,
	SWITCH_MODE_TRIP = 1,
	SWITCH_MODE_REPORT = 2,
	SWITCH_MODE_MAX = 2
} switch_mode_t;

typedef enum {
	SWITCH_MODE_DRIVE_E = 0,
	SWITCH_MODE_DRIVE_S = 1,
	SWITCH_MODE_DRIVE_P = 2,
	SWITCH_MODE_DRIVE_MAX = 2
} switch_mode_drive_t;

typedef enum {
	SWITCH_MODE_TRIP_A = 0, SWITCH_MODE_TRIP_B = 1, SWITCH_MODE_TRIP_MAX = 1
} switch_mode_trip_t;

typedef enum {
	SWITCH_MODE_REPORT_RANGE = 0,
	SWITCH_MODE_REPORT_EFFICIENCY = 1,
	SWITCH_MODE_REPORT_MAX = 1
} switch_mode_report_t;

// object list
typedef struct {
	switch_mode_t mode;
	uint8_t mode_sub[SWITCH_MODE_MAX + 1];
	uint8_t mode_sub_max[SWITCH_MODE_MAX + 1];
	uint8_t mode_sub_report[SWITCH_MODE_REPORT_MAX + 1];
	uint32_t mode_sub_trip[SWITCH_MODE_TRIP_MAX + 1];
} switcher_t;

typedef struct {
	char *event;
	uint16_t pin;
	GPIO_TypeDef *port;
	uint8_t state;
} switch_t;

typedef struct {
	uint32_t start;
	uint8_t running;
	uint8_t time;
} switch_timer_t;

#endif /* DATABASE_H_ */
