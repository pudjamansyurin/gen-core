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
#if (!BOOTLOADER)
extern osMutexId_t CrcMutexHandle;
#endif

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
uint32_t CRC_Calculate8(uint8_t *arr, uint32_t count, uint8_t swapped) {
    uint32_t cnt, result, value = 0;
    uint8_t index = 0, remaining[4] = { 0 };

    lock();

    /* Reset generator */
    __HAL_CRC_DR_RESET(&hcrc);

    /* Calculate number of 32-bit blocks */
    cnt = count >> 2;

    /* Calculate */
    while (cnt--) {
        value = *(uint32_t*) arr;
        /* Set new value */
        hcrc.Instance->DR = swapped ? _ByteSwap32(value) : value;

        /* Increase by 4 */
        arr += 4;
    }

    /* Calculate remaining data as 8-bit */
    cnt = count % 4;

    if (cnt) {
        /* Calculate */
        while (cnt--) {
            remaining[index++] = *arr++;
        }
        /* Set new value */
        value = *(uint32_t*) remaining;
        hcrc.Instance->DR = swapped ? _ByteSwap32(value) : value;
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
#if (!BOOTLOADER)
    osMutexAcquire(CrcMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (!BOOTLOADER)
    osMutexRelease(CrcMutexHandle);
#endif
}
