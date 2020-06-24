/*
 * _eeprom24xx.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Nima Askari
 */

/* Includes -------------------------------------------------------------------*/
#include "Drivers/_ee24xx.h"

/* External variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c2;

/* Private variables ----------------------------------------------------------*/
static uint16_t DevAddress = EEPROM24_MAIN;

/* Public functions implementation ---------------------------------------------*/
void EEPROM24XX_SetDevice(EEPROM24_DEVICE Device) {
    DevAddress = Device;
}

uint8_t EEPROM24XX_IsConnected(void) {
    return (HAL_I2C_IsDeviceReady(&hi2c2, DevAddress, 2, 1000) == HAL_OK);
}

uint8_t EEPROM24XX_Save(uint16_t Address, void *data, size_t size_of_data) {
    if (size_of_data > 32) {
        return 0;
    }

    if (HAL_I2C_Mem_Write(&hi2c2, DevAddress, Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*) data, size_of_data, 100) == HAL_OK) {
        _DelayMS(15);
        return 1;
    }
    return 0;

}

uint8_t EEPROM24XX_Load(uint16_t Address, void *data, size_t size_of_data) {
    return (HAL_I2C_Mem_Read(&hi2c2, DevAddress, Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*) data, size_of_data, 100) == HAL_OK);
}
