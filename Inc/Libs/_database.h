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
#define SW_K_SELECT 					0
#define SW_K_SET 						1
#define SW_K_SEIN_LEFT 					2
#define SW_K_SEIN_RIGHT 				3
#define SW_K_REVERSE		 			4
#define SW_K_ABS				 		5
#define SW_K_MIRRORING 					6

// enum list
typedef enum {
  SW_M_DRIVE = 0,
  SW_M_TRIP = 1,
  SW_M_REPORT = 2,
  SW_M_MAX = 2
} sw_mode_t;

typedef enum {
  SW_M_DRIVE_E = 0,
  SW_M_DRIVE_S = 1,
  SW_M_DRIVE_P = 2,
  SW_M_DRIVE_R = 3,
  SW_M_DRIVE_MAX = 2
} sw_mode_drive_t;

typedef enum {
  SW_M_TRIP_A = 0,
  SW_M_TRIP_B = 1,
  SW_M_TRIP_MAX = 1
} sw_mode_trip_t;

typedef enum {
  SW_M_REPORT_RANGE = 0,
  SW_M_REPORT_AVERAGE = 1,
  SW_M_REPORT_MAX = 1
} sw_mode_report_t;

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
  struct {
    uint8_t signal;
    uint8_t speed;
    uint32_t odometer;
    timestamp_t timestamp;
    struct {
      uint8_t count;
      struct {
        char event[20];
        uint16_t pin;
        GPIO_TypeDef *port;
        uint8_t state;
      } list[6];
      struct {
        uint32_t start;
        uint8_t running;
        uint8_t time;
      } timer[2];
      struct {
        uint8_t listening;
        struct {
          sw_mode_t val;
          struct {
            uint8_t val[SW_M_MAX + 1];
            uint8_t max[SW_M_MAX + 1];
            uint8_t report[SW_M_REPORT_MAX + 1];
            uint32_t trip[SW_M_TRIP_MAX + 1];
          } sub;
        } mode;
      } runner;
    } sw;
  } vcu;
  struct {
    status_t status;
  } hmi1;
  struct {
    uint8_t shutdown;
  } hmi2;
} db_t;

#endif /* DATABASE_H_ */
