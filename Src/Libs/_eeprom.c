/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

#include "_eeprom.h"

uint16_t VirtAddVarTab[NB_OF_VAR] = { VADDR_ODOMETER_L, VADDR_ODOMETER_H };

void Flash_Save_Odometer(uint32_t odometer) {
	EE_WriteVariable(VADDR_ODOMETER_L, (uint16_t) odometer);
	EE_WriteVariable(VADDR_ODOMETER_H, (uint16_t) odometer >> 16);
}

uint32_t Flash_Get_Odometer(void) {
	uint16_t odom_L, odom_H;

	if (EE_ReadVariable(VADDR_ODOMETER_L, &odom_L) == HAL_OK) {
		if (EE_ReadVariable(VADDR_ODOMETER_H, &odom_H) == HAL_OK) {
			return (odom_H << 16) | odom_L;
		}
	}
	return 0;
}
