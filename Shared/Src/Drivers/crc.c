/*
 * crc.c
 *
 *  Created on: Feb 4, 2020
 *      Author: Pudja Mansyurin
 *      See: crccalc.com (CRC-32/MPEG-2)
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/crc.h"

#include "crc.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t CrcMutexHandle;
#endif

/* Private functions prototype
 * --------------------------------------------*/
static void Lock(void);
static void UnLock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint32_t CRC_Calculate8(uint8_t* arr, uint32_t count, uint8_t swapped) {
  uint32_t cnt, result, value = 0;
  uint8_t index = 0, remaining[4] = {0};

  Lock();
  /* Reset generator */
  __HAL_CRC_DR_RESET(&hcrc);

  /* Calculate number of 32-bit bLocks */
  cnt = count >> 2;

  /* Calculate */
  while (cnt--) {
    value = *(uint32_t*)arr;
    /* Set new value */
    hcrc.Instance->DR = swapped ? swap32(value) : value;

    /* Increase by 4 */
    arr += 4;
  }

  /* Calculate remaining data as 8-bit */
  cnt = count % 4;

  if (cnt) {
    /* Calculate */
    while (cnt--) remaining[index++] = *arr++;
    /* Set new value */
    value = *(uint32_t*)remaining;
    hcrc.Instance->DR = swapped ? swap32(value) : value;
  }
  result = hcrc.Instance->DR;
  UnLock();

  return result;
}

uint32_t CRC_Calculate32(uint32_t* arr, uint32_t count) {
  uint32_t result;

  Lock();
  result = HAL_CRC_Calculate(&hcrc, arr, count);
  UnLock();

  return result;
}

/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(CrcMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(CrcMutexHandle);
#endif
}
