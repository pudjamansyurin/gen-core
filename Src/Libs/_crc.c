/*
 * _crc.c
 *
 *  Created on: Feb 4, 2020
 *      Author: pudja
 *      See: crccalc.com (CRC-32/MPEG-2)
 */

#include "_crc.h"

extern CRC_HandleTypeDef hcrc;

uint32_t CRC_Calculate8(uint8_t *arr, uint32_t count, uint8_t reset) {
  uint32_t cnt, remaining = 0;

  /* Reset CRC data register if necessary */
  if (reset) {
    /* Reset generator */
    __HAL_CRC_DR_RESET(&hcrc);
  }

  /* Calculate number of 32-bit blocks */
  cnt = BSR(count, 2);

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
      /* Combine 8bit to 32bit using litle endian */
      remaining |= BSL((*arr), (24 - (cnt * 8)));

      // increment pointer
      arr++;
    }

    /* Set new value */
    hcrc.Instance->DR = remaining;
  }

  /* Return data */
  return hcrc.Instance->DR;
}

uint32_t CRC_Calculate32(uint32_t *arr, uint32_t count, uint8_t reset) {
  /* Reset CRC data register if necessary */
  if (reset) {
    /* Reset generator */
    __HAL_CRC_DR_RESET(&hcrc);
  }

  /* Calculate CRC */
  while (count--) {
    /* Set new value */
    hcrc.Instance->DR = *arr++;
  }

  /* Return data */
  return hcrc.Instance->DR;
}
