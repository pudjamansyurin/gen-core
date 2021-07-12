/*
 * _iap.h
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */

#ifndef INC_APP__IAP_H_
#define INC_APP__IAP_H_

/* Includes
 * --------------------------------------------*/
#include "_defs.h"

#if (APP)
#include "App/_command.h"
#endif

/* Exported constants
 * --------------------------------------------*/
/* SRAM related */
#define SRAM_SIZE ((uint32_t)0x50000)
#define SRAM_BASE_ADDR ((uint32_t)0x20000000)
#define SRAM_END_ADDR (SRAM_BASE_ADDR + SRAM_SIZE)

#define IAP_FLAG_ADDR (SRAM_END_ADDR - 4)
#define IAP_RESP_ADDR (SRAM_END_ADDR - 8)

/* FLASH related */
/* APP Firmware */
#define APP_MAX_SIZE ((uint32_t)0xA0000)
#define APP_START_ADDR ((uint32_t)0x08020000)
#define APP_END_ADDR (APP_START_ADDR + APP_MAX_SIZE - 1)
#define BKP_START_ADDR ((uint32_t)0x080C0000)
#define BKP_END_ADDR (BKP_START_ADDR + APP_MAX_SIZE - 1)

#define SIZE_OFFSET (APP_MAX_SIZE - 4)
#define CRC_OFFSET (APP_MAX_SIZE - 8)

/* Bootloader */
#define BL_MAX_SIZE ((uint32_t)0x10000)
#define BL_START_ADDR ((uint32_t)0x08000000)
#define BL_END_ADDR (BL_START_ADDR + BL_MAX_SIZE - 1)

#define VIN_OFFSET (BL_MAX_SIZE - 4)
#define IEEP_OFFSET (BL_MAX_SIZE - 8)

#define VIN_VALUE (*(__IO uint32_t *)(BL_START_ADDR + VIN_OFFSET))
#define IEEP_VALUE (*(__IO uint32_t *)(BL_START_ADDR + IEEP_OFFSET))

/* Exported macros
 * --------------------------------------------*/
/* SRAM related */
#define SP_MASK ((uint32_t)0x2FFFFFFF)
#define SP_RANGE (SP_MASK - (SRAM_SIZE - 1))

#define IS_VALID_SP(X) ((*(__IO uint32_t *)X & SP_RANGE) == SRAM_BASE_ADDR)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
	IEEP_RESET = 0xFFFFFFFF,
	IEEP_SET = 0xFEDCBA98,
} IAP_EEPROM;

typedef enum {
	IFLAG_RESET = 0,
	IFLAG_EEPROM = 0x89ABCDEF,
	IFLAG_SRAM = 0xAABBCCDD,
} IAP_FLAG;

typedef enum {
	ITYPE_RESET = 0,
	ITYPE_VCU = 0xA1B2C3D4,
	ITYPE_HMI = 0X1A2B3C4D
} IAP_TYPE;

typedef enum {
	IRESP_BATTERY_LOW = 0x76543210,
	IRESP_SIM_TIMEOUT = 0x035F67B8,
	IRESP_DOWNLOAD_ERROR = 0x6AA55122,
	IRESP_FIRMWARE_SAME = 0xA5BE5FF3,
	IRESP_CRC_INVALID = 0xD7E9EF0D,
	IRESP_CAN_FAILED = 0x977FC0A3,
	IRESP_FOTA_ERROR = 0xFA359EA1,
	IRESP_FOTA_SUCCESS = 0x41B0FC9E,
	IRESP_RESPONSE_COUNT = 8
} IAP_RESPONSE;

/* Public functions prototype
 * --------------------------------------------*/
#if (APP)
bool IAP_ValidResponse(void);
bool IAP_EnterMode(IAP_TYPE type);
void IAP_CaptureResponse(response_t *r);
#endif

void IAP_Init(void);

#if (!APP)
void IAP_SetAppMeta(uint32_t offset, uint32_t data);
void IAP_SetBootMeta(uint32_t offset, uint32_t data);
void IAP_SetFlag(void);
void IAP_ResetFlag(void);
uint8_t IAP_InProgress(void);
#endif

uint8_t IAP_EE_Type(IAP_TYPE *src);
uint8_t IAP_EE_Flag(IAP_FLAG *src);
uint8_t IAP_EE_Version(uint16_t *src);

IAP_TYPE IAP_IO_Type(void);
uint16_t IAP_IO_Version(void);
#endif /* INC_APP__IAP_H_ */
