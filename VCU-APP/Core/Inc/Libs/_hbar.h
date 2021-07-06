/*
 * _hbar.h
 *
 *  Created on: Apr 16, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef LIBS__HBAR_H_
#define LIBS__HBAR_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported constants
 * --------------------------------------------*/
#define MODE_SESSION_MS ((uint16_t)4000)
#define MODE_RESET_MS ((uint16_t)1500)
#define STARTER_LONG_PRESS_MS ((uint16_t)1000)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  HBAR_K_SELECT = 0,
  HBAR_K_SET,
  HBAR_K_STARTER,
  HBAR_K_SEIN_L,
  HBAR_K_SEIN_R,
  HBAR_K_REVERSE,
  HBAR_K_LAMP,
  HBAR_K_ABS,
  HBAR_K_MAX = 8
} HBAR_KEY;

typedef enum {
  HBAR_M_DRIVE = 0,
  HBAR_M_TRIP,
  HBAR_M_PREDICTION,
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
  HBAR_M_PREDICTION_RANGE = 0,
  HBAR_M_PREDICTION_EFFICIENCY,
  HBAR_M_PREDICTION_MAX = 2
} HBAR_MODE_PREDICTION;

typedef enum {
  HBAR_STARTER_UNKNOWN = 0,
  HBAR_STARTER_ON,
  HBAR_STARTER_OFF
} HBAR_STARTER;

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint8_t left;
  uint8_t right;
} hbar_sein_t;

typedef struct {
  HBAR_STARTER starter;
  hbar_sein_t sein;
  uint8_t session;
  struct {
    uint32_t sein;
    uint32_t session;
  } tick;
} hbar_ctl_t;

typedef struct {
  uint32_t meter;
  uint8_t pin[HBAR_K_MAX];
  HBAR_MODE m;
  uint8_t mode[HBAR_M_MAX];
  uint8_t prediction[HBAR_M_PREDICTION_MAX];
  uint16_t trip[HBAR_M_TRIP_MAX];
} hbar_data_t;

typedef struct {
  uint32_t start;
  uint32_t time;
} hbar_timer_t;

typedef struct {
  hbar_data_t d;
  hbar_ctl_t ctl;
  hbar_timer_t tim[3];
} hbar_t;

/* Exported variables
 * --------------------------------------------*/
extern hbar_t HBAR;

/* Public functions prototype
 * --------------------------------------------*/
void HBAR_Init(void);
uint8_t HBAR_SubModeMax(HBAR_MODE m);
void HBAR_ReadStarter(uint8_t normalState);
void HBAR_CheckStarter(uint8_t *start, uint8_t *shutdown);
void HBAR_ReadStates(void);
void HBAR_RefreshSelectSet(void);
void HBAR_RefreshSein(void);
void HBAR_AddTripMeter(uint8_t m);
void HBAR_SetReport(uint8_t eff, uint8_t km);

void HBAR_ReadStore(void);
void HBAR_WriteDefferedStore(void);
uint8_t HBAR_ModeStore(uint8_t *src);
uint8_t HBAR_SubModeStore(HBAR_MODE m, uint8_t *src);
uint8_t HBAR_TripMeterStore(HBAR_MODE_TRIP mTrip, uint16_t *src);
#endif /* LIBS__HBAR_H_ */
