/*
 * _utils.h
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

#ifndef UTILS_H_
#define UTILS_H_

/* Includes ------------------------------------------------------------------*/
#include "_database.h"
#include "_log.h"
#include "_handlebar.h"

/* Exported constants --------------------------------------------------------*/
#define CHARISNUM(x)                            ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)                            ((x) - '0')

/* Public functions prototype ------------------------------------------------*/
uint8_t _LedRead(void);
void _LedWrite(uint8_t state);
void _LedToggle(void);
void _LedDisco(uint16_t ms);
void _Error(char msg[50]);
void _DebugTask(char name[20]);
void _DebugStackSpace(osThreadId_t *threads, uint8_t count);
void _DummyGenerator(db_t *db, sw_t *sw);
uint8_t _TimeNeedCalibration(db_t *db);
uint8_t _TimeCheckDaylight(timestamp_t timestamp);
int8_t _BitPosition(uint64_t event_id);
int32_t _ParseNumber(const char *ptr, uint8_t *cnt);
float _ParseFloatNumber(const char *ptr, uint8_t *cnt);
uint32_t _ByteSwap32(uint32_t x);

#endif /* UTILS_H_ */
