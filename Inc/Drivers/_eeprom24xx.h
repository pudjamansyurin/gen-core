/*
 * _eeprom24xx.h
 *
 *      Author: Nima Askari
 */

#ifndef _EEPROM24XX_H
#define _EEPROM24XX_H

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"

/* Exported constants --------------------------------------------------------*/
#define		_EEPROM_SIZE_KBIT										32
#define		_EEPROM_FREERTOS_IS_ENABLE					            1
#define		_EEPROM_USE_WP_PIN									    0
#if ( _EEPROM_USE_WP_PIN == 1 )
#define		_EEPROM_WP_GPIO											EE_WP_GPIO_Port
#define		_EEPROM_WP_PIN											EE_WP_Pin
#endif

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
