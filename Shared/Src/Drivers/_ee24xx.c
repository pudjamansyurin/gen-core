/*
 * _eeprom24xx.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Nima Askari
 */

/* Includes
 * -------------------------------------------------------------------*/
#include "Drivers/_ee24xx.h"

/* Private variables
 * ----------------------------------------------------------*/
static I2C_HandleTypeDef *i2c;
static uint16_t DevAddress;

/* Public functions implementation
 * ---------------------------------------------*/
void EEPROM24XX_SetDevice(I2C_HandleTypeDef *hi2c, uint16_t device) {
  i2c = hi2c;
  DevAddress = device;
}

uint8_t EEPROM24XX_IsConnected(uint32_t timeout) {
  return HAL_I2C_IsDeviceReady(i2c, DevAddress, 1, timeout) == HAL_OK;
}

uint8_t EEPROM24XX_Save(uint16_t address, void *data, size_t size) {
  if (size > 32)
    return 0;

  if (HAL_I2C_Mem_Write(i2c, DevAddress, address, I2C_MEMADD_SIZE_16BIT,
                        (uint8_t *)data, size, 100) == HAL_OK) {
    _DelayMS(15);
    return 1;
  }
  return 0;
}

uint8_t EEPROM24XX_Load(uint16_t address, void *data, size_t size) {
  return (HAL_I2C_Mem_Read(i2c, DevAddress, address, I2C_MEMADD_SIZE_16BIT,
                           (uint8_t *)data, size, 100) == HAL_OK);
}
