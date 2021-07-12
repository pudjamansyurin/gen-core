/*
 * rng.c
 *
 *  Created on: Mar 8, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/rng.h"

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
static void Lock(void);
static void UnLock(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t RNG_Generate32(uint32_t *payload, uint8_t size) {
  uint8_t ok;

  Lock();
  do {
    ok = HAL_RNG_GenerateRandomNumber(prng, payload++) == HAL_OK;
  } while (ok && --size);
  UnLock();

  return ok;
}

/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(RngMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(RngMutexHandle);
#endif
}
