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

// EXTI list
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
} sw_mode_t;

typedef enum {
	SWITCH_MODE_DRIVE_E = 0,
	SWITCH_MODE_DRIVE_S = 1,
	SWITCH_MODE_DRIVE_P = 2,
	SWITCH_MODE_DRIVE_R = 3,
	SWITCH_MODE_DRIVE_MAX = 2
} sw_mode_drive_t;

typedef enum {
	SWITCH_MODE_TRIP_A = 0,
	SWITCH_MODE_TRIP_B = 1,
	SWITCH_MODE_TRIP_MAX = 1
} sw_mode_trip_t;

typedef enum {
	SWITCH_MODE_REPORT_RANGE = 0,
	SWITCH_MODE_REPORT_AVERAGE = 1,
	SWITCH_MODE_REPORT_MAX = 1
} sw_mode_report_t;

// object list
typedef struct {
	uint8_t val[SWITCH_MODE_MAX + 1];
	uint8_t max[SWITCH_MODE_MAX + 1];
	uint8_t report[SWITCH_MODE_REPORT_MAX + 1];
	uint32_t trip[SWITCH_MODE_TRIP_MAX + 1];
} sw_mode_sub_t;

typedef struct {
	sw_mode_t val;
	sw_mode_sub_t sub;
} switch_mode_t;

typedef struct {
	switch_mode_t mode;
	uint8_t listening;
} switch_runner_t;

typedef struct {
	char event[20];
	uint16_t pin;
	GPIO_TypeDef *port;
	uint8_t state;
} switch_list_t;

typedef struct {
	uint32_t start;
	uint8_t running;
	uint8_t time;
} switch_timer_t;

typedef struct {
	switch_list_t list[7];
	switch_timer_t timer[2];
	switch_runner_t runner;
} switch_t;

//FIXME active disabled GPIO input
typedef struct {
	//	uint8_t abs;
	//	uint8_t mirror;
	uint8_t lamp;
	uint8_t warning;
	uint8_t temperature;
	uint8_t finger;
	uint8_t keyless;
//	uint8_t daylight;
//	uint8_t sein_left;
//	uint8_t sein_right;
} status_t;

// Node struct
typedef struct {
	uint8_t signal;
	uint8_t speed;
	uint32_t odometer;
	timestamp_t timestamp;
	switch_t sw;
} vcu_t;

typedef struct {
	status_t status;
} hmi1_t;

typedef struct {
	uint8_t shutdown;
} hmi2_t;

#endif /* DATABASE_H_ */
