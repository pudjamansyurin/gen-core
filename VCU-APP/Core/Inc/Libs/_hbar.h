/*
 * _hbar.h
 *
 *  Created on: Apr 16, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__HB_H_
#define INC_LIBS__HB_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  HB_SEIN_LEFT,
  HB_SEIN_RIGHT,
  HB_SEIN_MAX,
} HB_SEIN;

typedef enum {
  HBP_SELECT = 0,
  HBP_SET,
  HBP_STARTER,
  HBP_SEIN_L,
  HBP_SEIN_R,
  HBP_REVERSE,
  HBP_LAMP,
  HBP_ABS,
  HBP_MAX = 8
} HBP;

typedef enum { HBM_DRIVE = 0, HBM_TRIP, HBM_AVG, HBM_MAX = 3 } HBM;

typedef enum {
  HBMS_DRIVE_ECONOMY = 0,
  HBMS_DRIVE_STANDARD,
  HBMS_DRIVE_SPORT,
  HBMS_DRIVE_MAX = 3
} HBMS_DRIVE;

typedef enum {
  HBMS_TRIP_A = 0,
  HBMS_TRIP_B,
  HBMS_TRIP_ODO,
  HBMS_TRIP_MAX = 3
} HBMS_TRIP;

typedef enum {
  HBMS_AVG_RANGE = 0,
  HBMS_AVG_EFFICIENCY,
  HBMS_AVG_MAX = 2
} HBMS_AVG;

/* Public functions prototype
 * --------------------------------------------*/
void HB_Init(void);
uint8_t HB_SubMax(HBM m);
void HB_ReadStarter(uint8_t normalState);
void HB_CheckStarter(uint8_t *start, uint8_t *shutdown);
void HB_ReadStates(void);
void HB_RefreshSelectSet(void);
void HB_RefreshSein(void);
void HB_AddTrip(uint8_t m);
uint8_t HB_HasSession(void);

void HB_EE_Read(void);
void HB_EE_WriteDeffered(void);
uint8_t HB_EE_Mode(uint8_t *src);
uint8_t HB_EE_Sub(HBM m, uint8_t *src);
uint8_t HB_EE_Trip(HBMS_TRIP mTrip, uint16_t *src);

uint32_t HB_IO_Meter(void);
uint8_t HB_IO_Pin(HBP key);
uint8_t HB_IO_Mode(void);
uint8_t HB_IO_Sub(HBM mode);
uint8_t HB_IO_Sein(HB_SEIN side);
uint16_t HB_IO_Trip(HBMS_TRIP key);
uint8_t HB_IO_Average(HBMS_AVG key);
void HB_IO_SetPin(HBP key, uint8_t value);
void HB_IO_SetSub(HBM mode, uint8_t value);
void HB_IO_SetAverage(HBMS_AVG key, uint8_t value);
#endif /* INC_LIBS__HB_H_ */
