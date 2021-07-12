/*
 * event.c
 *
 *  Created on: Jun 21, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/event.h"

#include "App/common.h"

/* Private variables
 * --------------------------------------------*/
static uint16_t EVENTS;

/* Public functions implementation
 * --------------------------------------------*/
uint16_t EVT_Val(void) { return EVENTS; }

uint8_t EVT_Get(uint8_t bit) { return (EVENTS & BIT(bit)) == BIT(bit); }

void EVT_Set(uint8_t bit) { BV(EVENTS, bit); }

void EVT_Clr(uint8_t bit) { BC(EVENTS, bit); }

void EVT_Write(uint8_t bit, uint8_t value) {
  if (value & 1)
    BV(EVENTS, bit);
  else
    BC(EVENTS, bit);
}
