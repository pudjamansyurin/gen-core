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

/* Exported macro ------------------------------------------------------------*/
#define SRAM_SIZE               (uint32_t) 0x50000
#define SRAM_BASE_ADDR          (uint32_t) 0x20000000
#define SRAM_END_ADDR                      (SRAM_BASE_ADDR + SRAM_SIZE)

#define SP_MASK                 (uint32_t) 0x2FFFFFFF
#define SP_RANGE                           (SP_MASK - (SRAM_SIZE - 1))

#define DFU_IN_PROGRESS         (uint32_t) 0x89ABCDEF
#define IAP_FLAG                (uint32_t) 0xAABBCCDD
#define IAP_RETRY                          1

#define IAP_FLAG_ADDR                      (SRAM_END_ADDR - sizeof(uint32_t))
#define IAP_RETRY_ADDR                     (IAP_FLAG_ADDR - sizeof(uint32_t))

#define IS_VALID_SP(a)          ((*(__IO uint32_t*)a & SP_RANGE) == SRAM_BASE_ADDR)
#define IS_DFU_IN_PROGRESS(v)   ((uint32_t)v == DFU_IN_PROGRESS)

/* Public functions prototype ------------------------------------------------*/
void _DelayMS(uint32_t ms);
uint32_t _GetTickMS(void);
uint8_t _LedRead(void);
void _LedWrite(uint8_t state);
void _LedToggle(void);
void _Error(char msg[50]);
uint32_t _ByteSwap32(uint32_t x);

#if (!BOOTLOADER)
void _BuzzerWrite(uint8_t state);
void _RTOS_Debugger(uint32_t ms);
uint8_t _RTOS_ValidThreadFlag(uint32_t flag);
uint8_t _RTOS_ValidEventFlag(uint32_t flag);
void _DummyGenerator(void);
int8_t _BitPosition(uint64_t event_id);
#endif
#endif /* UTILS_H_ */
