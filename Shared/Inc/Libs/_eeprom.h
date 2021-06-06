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

/* Exported macro function ---------------------------------------------------*/
#define EE_ADDR  			   		((uint16_t)0xA0)
#define EE_NULL                     ((uint8_t)0)
#define EE_AREA(ad, sz)             ((uint16_t)((ad*32) + sz))

/* Exported constants --------------------------------------------------------*/
#define VADDR_RESET                 EE_AREA(0, 2)
#define VADDR_ODOMETER              EE_AREA(1, 2)
#define VADDR_AES_KEY               EE_AREA(2, 16)
#define VADDR_FOTA_VERSION          EE_AREA(3, 2)
#define VADDR_FOTA_FLAG             EE_AREA(4, 4)
#define VADDR_FOTA_TYPE             EE_AREA(5, 4)

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
uint8_t EE_Odometer(EE_CMD cmd, uint16_t value);
#endif
uint8_t EE_FotaType(EE_CMD cmd, IAP_TYPE value);
uint8_t EE_FotaFlag(EE_CMD cmd, uint32_t value);
uint8_t EE_FotaVersion(EE_CMD cmd, uint16_t value);
#endif /* EEPROM_H_ */
