/*
 * event.h
 *
 *  Created on: Jun 21, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__EVENT_H_
#define INC_APP__EVENT_H_

/* Includes
 * --------------------------------------------*/
#include "defs.h"

/* Public functions prototype
 * --------------------------------------------*/
uint16_t EVT_Val(void);
uint8_t EVT_Get(uint8_t bit);
void EVT_Set(uint8_t bit);
void EVT_Clr(uint8_t bit);
void EVT_Write(uint8_t bit, uint8_t value);

#endif /* INC_APP__EVENT_H_ */
