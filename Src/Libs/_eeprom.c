/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

#include "_eeprom.h"

uint16_t VirtAddVarTab[NB_OF_VAR] = { VADDR_ODOMETER_L, VADDR_ODOMETER_H };

void EEPROM_WriteOdometer(uint32_t odometer) {
  EE_WriteVariable(VADDR_ODOMETER_L, (uint16_t) odometer);
  EE_WriteVariable(VADDR_ODOMETER_H, (uint16_t) BSR(odometer, 16));
}

uint32_t EEPROM_ReadOdometer(void) {
  uint16_t odom_L, odom_H;

  if (EE_ReadVariable(VADDR_ODOMETER_L, &odom_L) == HAL_OK) {
    if (EE_ReadVariable(VADDR_ODOMETER_H, &odom_H) == HAL_OK) {
      return BSL(odom_H, 16) | odom_L;
    }
  }
  return 0;
}
