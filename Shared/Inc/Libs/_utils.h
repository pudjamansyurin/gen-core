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
#include "Drivers/_gate.h"
#if RTOS_ENABLE
#include "Libs/_rtos.h"
#endif
#if !BOOTLOADER
#include "Business/_event.h"
#endif

/* Exported union ------------------------------------------------------------*/
typedef union {
    uint8_t u8[8];
    uint16_t u16[4];
    uint32_t u32[2];
    uint64_t u64;
    int8_t s8[8];
    int16_t s16[4];
    int32_t s32[2];
    int64_t s64;
    char CHAR[8];
    float FLOAT[2];
    double DOUBLE;
} UNION64;

/* Exported struct
 * -------------------------------------------------------------*/
typedef struct {
	double sum;
	uint16_t pos;
	uint16_t len;
} averager_float_t;

/* Public functions prototype ------------------------------------------------*/
void _I2C_ClearBusyFlagErratum(void);
void _DelayMS(uint32_t ms);
uint32_t _GetTickMS(void);
uint8_t _TickOut(uint32_t tick, uint32_t ms);
uint8_t _TickIn(uint32_t tick, uint32_t ms);
void _Error(char msg[50]);
uint32_t _ByteSwap32(uint32_t x);
//int8_t _BitPos(uint64_t event_id);
float _MovAvgFloat(averager_float_t *m, float *buf, uint16_t sz, float val);
#endif /* UTILS_H_ */
