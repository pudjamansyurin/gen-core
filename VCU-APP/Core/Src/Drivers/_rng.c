/*
 * _rng.c
 *
 *  Created on: Mar 8, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_rng.h"

#include "rng.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t RngMutexHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static RNG_HandleTypeDef *prng = &hrng;

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t RNG_Generate32(uint32_t *payload, uint8_t size) {
  uint8_t ok;

  lock();
  do {
    ok = HAL_RNG_GenerateRandomNumber(prng, payload++) == HAL_OK;
  } while (ok && --size);
  unlock();

  return ok;
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(RngMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(RngMutexHandle);
#endif
}
