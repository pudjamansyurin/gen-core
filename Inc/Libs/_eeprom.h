/*
 * _flash.h
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

#ifndef EEPROM_H_
#define EEPROM_H_

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"
#include "_eeprom24xx.h"

/* Exported constants --------------------------------------------------------*/
#define VADDR_ODOMETER              0x000
#define VADDR_UNITID                VADDR_ODOMETER + sizeof(uint32_t)

/* Exported enum -------------------------------------------------------------*/
typedef enum {
  EE_CMD_R = 0,
  EE_CMD_W = 1
} EEPROM_COMMAND;

/* Public functions prototype ------------------------------------------------*/
uint8_t EEPROM_Init(void);
uint8_t EEPROM_Odometer(EEPROM_COMMAND cmd, uint32_t *value);
uint8_t EEPROM_UnitID(EEPROM_COMMAND cmd, uint32_t *value);

#endif /* EEPROM_H_ */
