/*
 * _flash.h
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

#ifndef EEPROM_H_
#define EEPROM_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#if (!BOOTLOADER)
#include "Libs/_hbar.h"
#endif

/* Exported macro function ---------------------------------------------------*/
#define EE_ADDR  			   		((uint16_t)0xA0)
#define EE_NULL                     ((uint8_t)0)
#define EE_WORD(ad)             	((uint16_t)(ad*32))

/* Exported constants --------------------------------------------------------*/
#define VADDR_RESET                 EE_WORD(0)
#define VADDR_UNUSED                EE_WORD(1)
#define VADDR_AES_KEY               EE_WORD(2)
#define VADDR_FOTA_VERSION          EE_WORD(3)
#define VADDR_FOTA_FLAG             EE_WORD(4)
#define VADDR_FOTA_TYPE             EE_WORD(5)
#define VADDR_TRIP_BASE             EE_WORD(6)
#define VADDR_USED_TRIP1            EE_WORD(7)
#define VADDR_USED_TRIP2            EE_WORD(8)

/* Exported struct -----------------------------------------------------------*/
typedef struct {
    uint32_t FLAG;
    uint16_t VERSION;
    IAP_TYPE TYPE;
} fota_t;

/* Exported enum -------------------------------------------------------------*/
typedef enum {
    EE_CMD_R = 0,
    EE_CMD_W = 1
} EE_CMD;

/* Exported variables ---------------------------------------------------------*/
extern fota_t FOTA;

/* Public functions prototype ------------------------------------------------*/
uint8_t EE_Init(void);
#if (!BOOTLOADER)
void EE_ResetOrLoad(void);
uint8_t EE_Reset(EE_CMD cmd, uint16_t value);
uint8_t EE_AesKey(EE_CMD cmd, uint32_t *value);
uint8_t EE_TripMeter(EE_CMD cmd, HBAR_MODE_TRIP type, uint16_t value);
#endif
uint8_t EE_FotaType(EE_CMD cmd, IAP_TYPE value);
uint8_t EE_FotaFlag(EE_CMD cmd, uint32_t value);
uint8_t EE_FotaVersion(EE_CMD cmd, uint16_t value);
#endif /* EEPROM_H_ */
