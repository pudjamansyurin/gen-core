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

/* Public functions prototype ------------------------------------------------*/
#if RTOS_ENABLE
uint8_t _osThreadFlagsWait(uint32_t *notif, uint32_t flags, uint32_t options, uint32_t timeout);
#endif
void _DelayMS(uint32_t ms);
uint32_t _GetTickMS(void);
void _Error(char msg[50]);
uint32_t _ByteSwap32(uint32_t x);
int8_t _BitPos(uint64_t event_id);
#endif /* UTILS_H_ */
