/*
 * _event.h
 *
 *  Created on: Jun 21, 2021
 *      Author: pudja
 */

#ifndef INC_BUSINESS__EVENT_H_
#define INC_BUSINESS__EVENT_H_

/* Includes
 * --------------------------------------------*/
#include "Libs/_utils.h"

/* Public functions prototype
 * --------------------------------------------*/
uint16_t EVT_Val(void);
uint8_t EVT_Get(uint8_t bit);
void EVT_Set(uint8_t bit);
void EVT_Clr(uint8_t bit);
void EVT_SetVal(uint8_t bit, uint8_t value);

#endif /* INC_BUSINESS__EVENT_H_ */
