/*
 * _database.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef DEFINES_SHARED_H_
#define DEFINES_SHARED_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#if (!BOOTLOADER)
#include "cmsis_os.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define RTOS_ENABLE                             !BOOTLOADER

/* Exported macro functions --------------------------------------------------*/
#define BIT(x)                                  (1ULL << x)
#define BV(var, x)                              (var |= (1ULL << x))
#define BC(var, x)                              (var &= ~(1ULL << x))
#define BT(var, x)                              (var ^= (1ULL << x))

/* SRAM related */
#define SRAM_SIZE                    (uint32_t) 0x50000
#define SRAM_BASE_ADDR               (uint32_t) 0x20000000
#define SRAM_END_ADDR                           (SRAM_BASE_ADDR + SRAM_SIZE)
#define SP_MASK                      (uint32_t) 0x2FFFFFFF
#define SP_RANGE                                (SP_MASK - (SRAM_SIZE - 1))

#define FOTA_PROGRESS_FLAG           (uint32_t) 0x89ABCDEF
#define IAP_FLAG                     (uint32_t) 0xAABBCCDD
#define IAP_FLAG_ADDR                           (SRAM_END_ADDR - 4)
#define IAP_RESPONSE_ADDR                       (SRAM_END_ADDR - 8)
#define IS_VALID_SP(a)               ((*(__IO uint32_t*)a & SP_RANGE) == SRAM_BASE_ADDR)

/* FLASH related */
#define APP_MAX_SIZE                 (uint32_t) 0xA0000
#define APP_START_ADDR               (uint32_t) 0x08020000
#define APP_END_ADDR                            (APP_START_ADDR + APP_MAX_SIZE - 1)
#define BKP_START_ADDR               (uint32_t) 0x080C0000
#define BKP_END_ADDR                            (BKP_START_ADDR + APP_MAX_SIZE - 1)
#define SIZE_OFFSET                             (APP_MAX_SIZE - 4)
#define CHECKSUM_OFFSET                         (APP_MAX_SIZE - 8)
#define VIN_VALUE                    (*(__IO uint32_t*) (APP_START_ADDR - 4))

/* Exported typedef ----------------------------------------------------------*/
typedef enum {
  FOCAN_ERROR = 0x00,
  FOCAN_ACK = 0x79,
  FOCAN_NACK = 0x1F
} FOCAN;

typedef enum {
  IAP_VCU = 0xA1B2C3D4,
  IAP_HMI = 0X1A2B3C4D
} IAP_TYPE;

typedef enum {
  IAP_BATTERY_LOW = 0x76543210,
  IAP_SIMCOM_TIMEOUT = 0x035F67B8,
  IAP_DOWNLOAD_ERROR = 0x6AA55122,
  IAP_FIRMWARE_SAME = 0xA5BE5FF3,
  IAP_CHECKSUM_INVALID = 0xD7E9EF0D,
  IAP_CANBUS_FAILED = 0x977FC0A3,
  IAP_FOTA_ERROR = 0xFA359EA1,
  IAP_FOTA_SUCCESS = 0x41B0FC9E,
  IAP_RESPONSE_COUNT = 8
} IAP_RESPONSE;

#endif /* DEFINES_SHARED_H_ */
