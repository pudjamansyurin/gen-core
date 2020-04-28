/*
 * _crc.c
 *
 *  Created on: Feb 4, 2020
 *      Author: pudja
 *      See: crccalc.com (CRC-32/MPEG-2)
 */

/* Includes ------------------------------------------------------------------*/
#include "_crc.h"

/* External variables ----------------------------------------------------------*/
extern CRC_HandleTypeDef hcrc;
extern osMutexId_t CRCMutexHandle;

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
uint32_t CRC_Calculate8(uint8_t *arr, uint32_t count, uint8_t reset) {
  uint32_t cnt, remaining = 0;

  lock();

  /* Reset CRC data register if necessary */
  if (reset) {
    /* Reset generator */
    __HAL_CRC_DR_RESET(&hcrc);
  }

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
      /* Combine 8bit to 32bit using litle endian */
      remaining |= _L((*arr), (24 - (cnt * 8)));

      // increment pointer
      arr++;
    }

    /* Set new value */
    hcrc.Instance->DR = remaining;
  }

  unlock();

  /* Return data */
  return hcrc.Instance->DR;
}

uint32_t CRC_Calculate32(uint32_t *arr, uint32_t count, uint8_t reset) {
  lock();

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

  unlock();
  /* Return data */
  return hcrc.Instance->DR;
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
  osMutexAcquire(CRCMutexHandle, osWaitForever);
}

static void unlock(void) {
  osMutexRelease(CRCMutexHandle);
}
