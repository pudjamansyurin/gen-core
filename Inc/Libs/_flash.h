/*
 * _flash.h
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "_ee_emulation.h"

/* Virtual address */
/* Virtual address defined by the user: 0xFFFF value is prohibited */
// FIXME: do i needed? eeprom emulation?
#define VADDR_ODOMETER_L			0x0000
#define VADDR_ODOMETER_H			(VADDR_ODOMETER_L + 1)

void Flash_Save_Odometer(uint32_t odometer);
uint32_t Flash_Get_Odometer(void);

#endif /* FLASH_H_ */
