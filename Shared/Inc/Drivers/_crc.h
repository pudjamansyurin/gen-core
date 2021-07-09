/*
 * _crc.h
 *
 *  Created on: Feb 4, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__CRC_H_
#define INC_DRIVERS__CRC_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Public functions prototype
 * --------------------------------------------*/
/**
 * @brief  Calculates 32-bit CRC value from 8-bit input array
 * @param  *arr: Pointer to 8-bit data array for CRC calculation
 * @param  count: Number of elements in array for calculation
 * @retval 32-bit CRC calculated number
 */
uint32_t CRC_Calculate8(uint8_t* arr, uint32_t count, uint8_t swapped);

/**
 * @brief  Calculates 32-bit CRC value from 32-bit input array
 * @param  *arr: Pointer to 32-bit data array for CRC calculation
 * @param  count: Number of elements in array for calculation
 * @retval 32-bit CRC calculated number
 */
uint32_t CRC_Calculate32(uint32_t* arr, uint32_t count);

#endif /* INC_DRIVERS__CRC_H_ */
