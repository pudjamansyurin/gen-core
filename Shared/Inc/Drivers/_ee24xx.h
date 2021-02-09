/*
 * _eeprom24xx.h
 *
 *      Author: Nima Askari
 */

#ifndef _EEPROM24XX_H
#define _EEPROM24XX_H

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Public typedef ------------------------------------------------------------*/


/* Public functions prototype ------------------------------------------------*/
void EEPROM24XX_SetDevice(I2C_HandleTypeDef *hi2c, uint16_t device);
uint8_t EEPROM24XX_IsConnected(uint32_t timeout);
uint8_t EEPROM24XX_Save(uint16_t address, void *data, size_t size);
uint8_t EEPROM24XX_Load(uint16_t address, void *data, size_t size);

#endif
