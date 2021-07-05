/*
 * _at24c.h
 *
 *      Author: Pudja Mansyurin
 */

#ifndef _AT24C_H
#define _AT24C_H

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t AT24C_Probe(void);
uint8_t AT24C_Read(uint16_t addr, uint8_t *data, uint16_t n);
uint8_t AT24C_Write(uint16_t addr, uint8_t *data, uint16_t n);
uint8_t AT24C_Clear(uint16_t addr, uint16_t n);
#endif
