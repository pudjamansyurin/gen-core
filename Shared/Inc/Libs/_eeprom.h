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
#define EEPROM_ADDR  			   (uint16_t) 0xA0
#define EE_NULL                     (uint8_t) 0

/* NOTE: EEPROM is 32 bytes aligned, do not store variable in intersection */
//#define EE_AREA(ad, sz)             (((ad + sz) % 32) >= sz ? (ad) : (ad + (sz - ((ad + sz) % 32))))
#define EE_AREA(ad, sz)             ((ad*32) + sz)

/* Exported constants --------------------------------------------------------*/
#define VADDR_RESET                 (uint16_t) EE_AREA(0, 2)
#define VADDR_ODOMETER              (uint16_t) EE_AREA(1, 2)
#define VADDR_AES_KEY               (uint16_t) EE_AREA(2, 16)
#define VADDR_FOTA_VERSION          (uint16_t) EE_AREA(3, 2)
#define VADDR_FOTA_FLAG             (uint16_t) EE_AREA(4, 4)
#define VADDR_FOTA_TYPE             (uint16_t) EE_AREA(5, 4)

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
} EEPROM_COMMAND;

/* Exported variables ---------------------------------------------------------*/
extern fota_t FOTA;

/* Public functions prototype ------------------------------------------------*/
uint8_t EEPROM_Init(void);
#if (!BOOTLOADER)
void EEPROM_ResetOrLoad(void);
uint8_t EEPROM_Reset(EEPROM_COMMAND cmd, uint16_t value);
uint8_t EEPROM_AesKey(EEPROM_COMMAND cmd, uint32_t *value);
uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint16_t value);
#endif
uint8_t EEPROM_FotaType(EEPROM_COMMAND cmd, IAP_TYPE value);
uint8_t EEPROM_FotaFlag(EEPROM_COMMAND cmd, uint32_t value);
uint8_t EEPROM_FotaVersion(EEPROM_COMMAND cmd, uint16_t value);
#endif /* EEPROM_H_ */
