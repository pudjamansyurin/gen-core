/*
 * _utils.h
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

#ifndef UTILS_H_
#define UTILS_H_

/* Includes ------------------------------------------------------------------*/
#include "_defines.h"
#include "Drivers/_log.h"

/* Exported constants --------------------------------------------------------*/
#define CHARISNUM(x)                            ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)                            ((x) - '0')

/* Public functions prototype ------------------------------------------------*/
uint8_t _LedRead(void);
void _LedWrite(uint8_t state);
void _LedToggle(void);
void _BuzzerWrite(uint8_t state);

void _Error(char msg[50]);

void _RTOS_Debugger(uint32_t ms);
uint8_t _RTOS_ValidThreadFlag(uint32_t flag);
uint8_t _RTOS_ValidEventFlag(uint32_t flag);

void _DummyGenerator(void);

int8_t _BitPosition(uint64_t event_id);
uint32_t _ByteSwap32(uint32_t x);

#endif /* UTILS_H_ */
