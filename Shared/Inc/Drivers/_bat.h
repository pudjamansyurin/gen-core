/*
 * _bat.h
 *
 *  Created on: Dec 11, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__BAT_H_
#define INC_DRIVERS__BAT_H_

/* Includes
 * ---------------------------------- ----------*/
#include "App/_common.h"

/* Public functions prototype
 * --------------------------------------------*/
void BAT_Init(void);
void BAT_DeInit(void);
uint16_t BAT_ScanValue(void);

#endif /* INC_DRIVERS__BAT_H_ */
