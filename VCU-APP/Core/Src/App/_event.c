/*
 * _event.c
 *
 *  Created on: Jun 21, 2021
 *      Author: pudja
 */

/* Includes
 * --------------------------------------------*/
#include "App/_event.h"

/* Private variables
 * --------------------------------------------*/
static uint16_t EVENTS;

/* Public functions implementation
 * --------------------------------------------*/
uint16_t EVT_Val(void) { return EVENTS; }

uint8_t EVT_Get(uint8_t bit) { return (EVENTS & BIT(bit)) == BIT(bit); }

void EVT_Set(uint8_t bit) { BV(EVENTS, bit); }

void EVT_Clr(uint8_t bit) { BC(EVENTS, bit); }

void EVT_SetVal(uint8_t bit, uint8_t value) {
  if (value & 1)
    BV(EVENTS, bit);
  else
    BC(EVENTS, bit);
}
