/*
 * _crc.c
 *
 *  Created on: Feb 4, 2020
 *      Author: pudja
 *      See: crccalc.com (CRC-32/MPEG-2)
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_crc.h"

/* External variables ----------------------------------------------------------*/
extern CRC_HandleTypeDef hcrc;
extern osMutexId_t CrcMutexHandle;

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
uint32_t CRC_Calculate8(uint8_t *arr, uint32_t count) {
	uint32_t cnt, result, remaining = 0, swap = 0;

	lock();

	/* Reset generator */
	__HAL_CRC_DR_RESET(&hcrc);

	/* Calculate number of 32-bit blocks */
	cnt = _R8(count, 2);

	/* Calculate */
	while (cnt--) {

		/* Set new value */
		hcrc.Instance->DR = *(uint32_t*) arr;

		/* Increase by 4 */
		arr += 4;
	}

	/* Calculate remaining data as 8-bit */
	cnt = count % 4;

	if (cnt) {
		/* Calculate */
		while (cnt--) {
			/* Combine 8bit to 32bit using little endian */
			/* Rest of bit is filled with zeros */
			remaining |= _L((*arr), swap);

			// increment pointer
			arr++;
			swap += 8;
		}

		/* Set new value */
		hcrc.Instance->DR = remaining;
	}
	result = hcrc.Instance->DR;

	unlock();
	/* Return data */
	return result;
}

uint32_t CRC_Calculate32(uint32_t *arr, uint32_t count) {
	uint32_t result;

	lock();
	result = HAL_CRC_Calculate(&hcrc, arr, count);
	unlock();

	return result;
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
	osMutexAcquire(CrcMutexHandle, osWaitForever);
}

static void unlock(void) {
	osMutexRelease(CrcMutexHandle);
}
