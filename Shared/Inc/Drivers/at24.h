/*
 * at24.h
 *
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__AT24_H
#define INC_DRIVERS__AT24_H

/* Includes
 * --------------------------------------------*/
#include "App/util.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t AT24_Probe(void);
uint8_t AT24_Read(uint16_t addr, uint8_t *data, uint16_t n);
uint8_t AT24_Write(uint16_t addr, const uint8_t *data, uint16_t n);
uint8_t AT24_Clear(uint16_t addr, uint16_t n);
#endif
