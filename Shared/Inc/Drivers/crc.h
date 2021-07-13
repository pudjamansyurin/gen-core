/*
 * crc.h
 *
 *  Created on: Feb 4, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__CRC_H_
#define INC_DRIVERS__CRC_H_

/* Includes
 * --------------------------------------------*/
#include "App/util.h"

/* Public functions prototype
 * --------------------------------------------*/
uint32_t CRC_Calculate8(uint8_t* arr, uint32_t count, uint8_t swapped);
uint32_t CRC_Calculate32(uint32_t* arr, uint32_t count);

#endif /* INC_DRIVERS__CRC_H_ */
