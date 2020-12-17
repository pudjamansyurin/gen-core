/*
 * _handlebar.h
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

#ifndef LIBS__HANDLEBAR_H_
#define LIBS__HANDLEBAR_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define MODE_TIME_GUARD              (uint16_t) 3000

/* Exported enum ----------------------------------------------------------------*/
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
  HBAR_M_MAX = 2
} HBAR_MODE;

typedef enum {
  HBAR_M_DRIVE_ECONOMY = 0,
  HBAR_M_DRIVE_STANDARD,
  HBAR_M_DRIVE_SPORT,
  HBAR_M_DRIVE_PERFORMANCE,
  HBAR_M_DRIVE_MAX = 3
} HBAR_MODE_DRIVE;

typedef enum {
  HBAR_M_TRIP_ODO = 0,
  HBAR_M_TRIP_A,
  HBAR_M_TRIP_B,
  HBAR_M_TRIP_MAX = 2
} HBAR_MODE_TRIP;

typedef enum {
  HBAR_M_REPORT_RANGE = 0,
  HBAR_M_REPORT_AVERAGE,
  HBAR_M_REPORT_MAX = 1
} HBAR_MODE_REPORT;

/* Exported struct --------------------------------------------------------------*/
typedef struct {
  uint8_t max[HBAR_M_MAX + 1];
  uint8_t val[HBAR_M_MAX + 1];
  uint8_t report[HBAR_M_REPORT_MAX + 1];
  uint32_t trip[HBAR_M_TRIP_MAX + 1];
} hbar_data_t;

typedef struct {
  uint8_t listening;
  uint8_t hazard;
  uint8_t reverse;
  struct {
    HBAR_MODE m;
    hbar_data_t d;
  } mode;
} hbar_runner_t;

typedef struct {
  GPIO_TypeDef *port;
  char event[20];
  uint16_t pin;
  uint8_t state;
} hbar_list_t;

typedef struct {
  uint32_t start;
  uint8_t running;
  uint8_t time;
} hbar_timer_t;

typedef struct {
  hbar_list_t list[HBAR_K_MAX];
  hbar_timer_t timer[2];
  hbar_runner_t runner;
} hbar_t;

typedef struct {
  uint8_t left;
  uint8_t right;
} sein_t;

/* Public functions prototype ------------------------------------------------*/
void HBAR_ReadStates(void);
void HBAR_CheckReverse(void);
void HBAR_TimerSelectSet(void);
void HBAR_RunSelectOrSet(void);
void HBAR_AccumulateSubTrip(uint8_t increment);
sein_t HBAR_SeinController(hbar_t *hbar);
uint8_t HBAR_ModeController(hbar_runner_t *runner);

#endif /* LIBS__HANDLEBAR_H_ */
