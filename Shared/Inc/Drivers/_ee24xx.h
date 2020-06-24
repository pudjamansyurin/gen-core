/*
 * _eeprom24xx.h
 *
 *      Author: Nima Askari
 */

#ifndef _EEPROM24XX_H
#define _EEPROM24XX_H

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define		EEPROM_SIZE_KBIT			            32

/* Exported enum -------------------------------------------------------------*/
typedef enum {
    EEPROM24_MAIN = 0xA0,
    EEPROM24_BACKUP = 0xA1
} EEPROM24_DEVICE;

/* Public functions prototype ------------------------------------------------*/
void EEPROM24XX_SetDevice(EEPROM24_DEVICE Device);
uint8_t EEPROM24XX_IsConnected(void);
uint8_t EEPROM24XX_Save(uint16_t Address, void *data, size_t size_of_data);
uint8_t EEPROM24XX_Load(uint16_t Address, void *data, size_t size_of_data);

#endif
