/*
 * _crc.c
 *
 *  Created on: Feb 4, 2020
 *      Author: pudja
 */

#include "_crc.h"
#include "_swv.h"

uint32_t CRC_Calculate8(CRC_HandleTypeDef *hcrc, uint8_t *arr, uint32_t count, uint8_t reset) {
	uint32_t cnt, remaining = 0;

	/* Reset CRC data register if necessary */
	if (reset) {
		/* Reset generator */
		__HAL_CRC_DR_RESET(hcrc);
	}

	/* Calculate number of 32-bit blocks */
	cnt = count >> 2;

//	SWV_SendStrLn("");
	/* Calculate */
	while (cnt--) {
//		SWV_SendHex32(*(uint32_t*) arr);
//		SWV_SendStr(" ");
		/* Set new value */
		hcrc->Instance->DR = *(uint32_t*) arr;

		/* Increase by 4 */
		arr += 4;
	}

	/* Calculate remaining data as 8-bit */
	cnt = count % 4;

	/* Calculate */
	while (cnt--) {
		remaining |= ((*arr) << (cnt * 8));

		// increment pointer
		arr++;
	}

	/* Store remaining as 32bit */
	if (remaining) {
//		SWV_SendHex32(remaining);
//		SWV_SendStr(" ");

		/* Set new value */
		hcrc->Instance->DR = remaining;
	}

//	SWV_SendStrLn("");

	/* Return data */
	return hcrc->Instance->DR;
}

uint32_t CRC_Calculate32(CRC_HandleTypeDef *hcrc, uint32_t *arr, uint32_t count, uint8_t reset) {
	/* Reset CRC data register if necessary */
	if (reset) {
		/* Reset generator */
		__HAL_CRC_DR_RESET(hcrc);
	}

	/* Calculate CRC */
	while (count--) {
		/* Set new value */
		hcrc->Instance->DR = *arr++;
	}

	/* Return data */
	return hcrc->Instance->DR;
}
