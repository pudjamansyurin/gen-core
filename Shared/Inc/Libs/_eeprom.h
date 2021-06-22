/*
 * _flash.h
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

#ifndef EEPROM_H_
#define EEPROM_H_

/* Includes
 * --------------------------------------------*/
#include "Libs/_utils.h"
#if (!BOOTLOADER)
#include "Libs/_hbar.h"
#endif

/* Exported macros
 * --------------------------------------------*/
#define EE_ADDR ((uint16_t)0xA0)
#define EE_NULL ((uint8_t)0)
#define EE_WORD(X) ((uint16_t)(X * 32))

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint32_t FLAG;
  uint16_t VERSION;
  IAP_TYPE TYPE;
} fota_t;

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  VADDR_RESET,
  VADDR_UNUSED,
  VADDR_AES_KEY,
  VADDR_FOTA_VERSION,
  VADDR_FOTA_FLAG,
  VADDR_FOTA_TYPE,
  VADDR_TRIP_A,
  VADDR_TRIP_B,
  VADDR_TRIP_ODO,
  VADDR_MODE_DRIVE,
  VADDR_MODE_TRIP,
  VADDR_MODE_REPORT,
  VADDR_MODE,
  VADDR_MAX,
} EE_VADDR;

typedef enum { EE_CMD_R = 0, EE_CMD_W = 1 } EE_CMD;

/* Exported variables
 * --------------------------------------------*/
extern fota_t FOTA;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t EE_Init(void);
#if (!BOOTLOADER)
void EE_Load(void);
// uint8_t EE_Reset(EE_CMD cmd, uint16_t value);
uint8_t EE_AesKey(EE_CMD cmd, uint32_t* value);
uint8_t EE_TripMeter(EE_CMD cmd, HBAR_MODE_TRIP mTrip, uint16_t value);
uint8_t EE_SubMode(EE_CMD cmd, HBAR_MODE m, uint8_t value);
uint8_t EE_Mode(EE_CMD cmd, uint8_t value);
#endif
uint8_t EE_FotaType(EE_CMD cmd, IAP_TYPE value);
uint8_t EE_FotaFlag(EE_CMD cmd, uint32_t value);
uint8_t EE_FotaVersion(EE_CMD cmd, uint16_t value);
#endif /* EEPROM_H_ */
