/*
 * _utils.h
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

#ifndef UTILS_H_
#define UTILS_H_

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include "main.h"
#include "cmsis_os.h"
#include "_log.h"
#include "_database.h"

/* External variables ---------------------------------------------------------*/
extern const TickType_t tick5ms,
    tick100ms,
    tick250ms,
    tick500ms,
    tick1000ms,
    tick5000ms,
    tickDelayFull,
    tickDelaySimple;

/* Exported constants --------------------------------------------------------*/
#define CHARISNUM(x)                            ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)                            ((x) - '0')

/* Public functions prototype ------------------------------------------------*/
void _LedWrite(uint8_t state);
void _LedToggle(void);
void _LedDisco(uint16_t ms);
void _DummyGenerator(db_t *db);
uint8_t _TimeNeedCalibration(rtc_t rtc);
uint8_t _TimeCheckDaylight(timestamp_t timestamp);
int8_t _BitPosition(uint64_t event_id);
int32_t _ParseNumber(const char *ptr, uint8_t *cnt);
float _ParseFloatNumber(const char *ptr, uint8_t *cnt);

#endif /* UTILS_H_ */
