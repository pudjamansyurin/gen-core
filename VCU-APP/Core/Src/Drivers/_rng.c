/*
 * _rng.c
 *
 *  Created on: Mar 8, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "rng.h"
#include "Drivers/_rng.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t RngMutexHandle;
#endif

/* Private variable ----------------------------------------------------------*/
static RNG_HandleTypeDef *prng = &hrng;

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation -------------------------------------------*/
uint8_t RNG_Generate32(uint32_t *payload, uint8_t size) {
	uint8_t ok = 1;

	lock();
	while(ok && size--){
		ok = HAL_RNG_GenerateRandomNumber(prng, payload++) == HAL_OK;
	}
	unlock();

	return ok;
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
  #if (RTOS_ENABLE)
  osMutexAcquire(RngMutexHandle, osWaitForever);
	#endif
}

static void unlock(void) {
  #if (RTOS_ENABLE)
  osMutexRelease(RngMutexHandle);
	#endif
}