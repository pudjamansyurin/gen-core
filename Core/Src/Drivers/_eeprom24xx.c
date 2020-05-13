/*
 * _eeprom24xx.c
 *
 *  Created on: Oct 7, 2019
 *      Author: Nima Askari
 */

/* Includes -------------------------------------------------------------------*/
#include "Drivers/_eeprom24xx.h"

/* External variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c2;

/* Private variables ----------------------------------------------------------*/
static I2C_HandleTypeDef *hi2c = &hi2c2;
static uint16_t DevAddress = EEPROM24_MAIN;

/* Public functions implementation ---------------------------------------------*/
void EEPROM24XX_SetDevice(EEPROM24_DEVICE Device) {
	DevAddress = Device;
}

//##########################################################################
uint8_t EEPROM24XX_IsConnected(void) {
	if (HAL_I2C_IsDeviceReady(hi2c, DevAddress, 2, 1000) == HAL_OK) {
		return 1;
	}
	return 0;
}

//##########################################################################
uint8_t EEPROM24XX_Save(uint16_t Address, void *data, size_t size_of_data) {
#if ((EEPROM_SIZE_KBIT==1) || (EEPROM_SIZE_KBIT==2))
  if(size_of_data > 8){
    return 0;
  }
#elif ((EEPROM_SIZE_KBIT==4) || (EEPROM_SIZE_KBIT==8) || (EEPROM_SIZE_KBIT==16))
  if(size_of_data > 16){
    return 0;
  }
#else
	if (size_of_data > 32) {
		return 0;
	}
#endif

#if ((EEPROM_SIZE_KBIT==1) || (EEPROM_SIZE_KBIT==2))
  if(HAL_I2C_Mem_Write(hi2c,DevAddress,Address,I2C_MEMADD_SIZE_8BIT,(uint8_t*)data,size_of_data,100) == HAL_OK) {
#elif   (EEPROM_SIZE_KBIT==4)
  if(HAL_I2C_Mem_Write(hi2c,DevAddress|((Address&0x0100>>7)),(Address&0xff),I2C_MEMADD_SIZE_8BIT,(uint8_t*)data,size_of_data,100) == HAL_OK) {
#elif   (EEPROM_SIZE_KBIT==8)
  if(HAL_I2C_Mem_Write(hi2c,DevAddress|((Address&0x0300>>7)),(Address&0xff),I2C_MEMADD_SIZE_8BIT,(uint8_t*)data,size_of_data,100) == HAL_OK) {
#elif   (EEPROM_SIZE_KBIT==16)
  if(HAL_I2C_Mem_Write(hi2c,DevAddress|((Address&0x0700>>7)),(Address&0xff),I2C_MEMADD_SIZE_8BIT,(uint8_t*)data,size_of_data,100) == HAL_OK) {
#else
	if (HAL_I2C_Mem_Write(hi2c, DevAddress, Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*) data, size_of_data, 100) == HAL_OK) {
#endif
		osDelay(7);
		return 1;
	}
	return 0;

}

//##########################################################################
uint8_t EEPROM24XX_Load(uint16_t Address, void *data, size_t size_of_data) {
#if ((EEPROM_SIZE_KBIT==1) || (EEPROM_SIZE_KBIT==2))
  if(HAL_I2C_Mem_Read(hi2c,DevAddress,Address,I2C_MEMADD_SIZE_8BIT,(uint8_t*)data,size_of_data,100) == HAL_OK) {
#elif (EEPROM_SIZE_KBIT==4)
  if(HAL_I2C_Mem_Read(hi2c,DevAddress|((Address&0x0100>>7)),(Address&0xff),I2C_MEMADD_SIZE_8BIT,(uint8_t*)data,size_of_data,100) == HAL_OK) {
#elif (EEPROM_SIZE_KBIT==8)
  if(HAL_I2C_Mem_Read(hi2c,DevAddress|((Address&0x0300>>7)),(Address&0xff),I2C_MEMADD_SIZE_8BIT,(uint8_t*)data,size_of_data,100) == HAL_OK) {
#elif (EEPROM_SIZE_KBIT==16)
  if(HAL_I2C_Mem_Read(hi2c,DevAddress|((Address&0x0700>>7)),(Address&0xff),I2C_MEMADD_SIZE_8BIT,(uint8_t*)data,size_of_data,100) == HAL_OK) {
#else
	if (HAL_I2C_Mem_Read(hi2c, DevAddress, Address, I2C_MEMADD_SIZE_16BIT, (uint8_t*) data, size_of_data, 100) == HAL_OK) {
#endif
		return 1;
	}
	return 0;
}
