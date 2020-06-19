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

/* Exported constants --------------------------------------------------------*/
#define VADDR_RESET                 0x00000000
#define VADDR_ODOMETER              VADDR_RESET + sizeof(uint16_t)
#define VADDR_UNITID                VADDR_ODOMETER + sizeof(uint32_t)
#define VADDR_REPORT_SEQ_ID         VADDR_UNITID + sizeof(uint32_t)
#define VADDR_RESPONSE_SEQ_ID       VADDR_REPORT_SEQ_ID + sizeof(uint16_t)
#define VADDR_AES_KEY               VADDR_RESPONSE_SEQ_ID + sizeof(uint16_t)
#define VADDR_FOTA                  VADDR_AES_KEY + 128

#define EE_NULL                     0

/* Exported enum -------------------------------------------------------------*/
typedef enum {
    EE_CMD_R = 0,
    EE_CMD_W = 1
} EEPROM_COMMAND;

/* Public functions prototype ------------------------------------------------*/
uint8_t EEPROM_Init(void);
void EEPROM_ResetOrLoad(void);
uint8_t EEPROM_Reset(EEPROM_COMMAND cmd, uint16_t value);
uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t value);
uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t value);
uint8_t EEPROM_SequentialID(EEPROM_COMMAND cmd, uint16_t value, PAYLOAD_TYPE type);
uint8_t EEPROM_AesKey(EEPROM_COMMAND cmd, uint32_t *value);
uint8_t EEPROM_Fota(EEPROM_COMMAND cmd, uint32_t value);

#endif /* EEPROM_H_ */
