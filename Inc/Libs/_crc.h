/*
 * _crc.h
 *
 *  Created on: Feb 4, 2020
 *      Author: pudja
 */

#ifndef LIBS__CRC_H_
#define LIBS__CRC_H_

#include "_config.h"
/**
 * @brief  Calculates 32-bit CRC value from 8-bit input array
 * @param  *arr: Pointer to 8-bit data array for CRC calculation
 * @param  count: Number of elements in array for calculation
 * @param  reset: Reset CRC peripheral to 0 state before starting new calculations
 *            - 0: CRC unit will not be reset before new calculations will happen and will use previous data to continue
 *            - > 0: CRC unit is set to 0 before first calculation
 * @retval 32-bit CRC calculated number
 */
uint32_t CRC_Calculate8(uint8_t *arr, uint32_t count, uint8_t reset);

/**
 * @brief  Calculates 32-bit CRC value from 32-bit input array
 * @param  *arr: Pointer to 32-bit data array for CRC calculation
 * @param  count: Number of elements in array for calculation
 * @param  reset: Reset CRC peripheral to 0 state before starting new calculations
 *            - 0: CRC unit will not be reset before new calculations will happen and will use previous data to continue
 *            - > 0: CRC unit is set to 0 before first calculation
 * @retval 32-bit CRC calculated number
 */
uint32_t CRC_Calculate32(uint32_t *arr, uint32_t count, uint8_t reset);

#endif /* LIBS__CRC_H_ */
