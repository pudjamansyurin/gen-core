/*
 * _iwdg.c
 *
 *  Created on: Mar 14, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "iwdg.h"
#include "Drivers/_iwdg.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t IwdgMutexHandle;
#endif

/* Private variable ----------------------------------------------------------*/
static IWDG_HandleTypeDef *piwdg = &hiwdg;

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation -------------------------------------------*/
void IWDG_Refresh(void) {
  lock();
  HAL_IWDG_Refresh(piwdg);
  unlock();
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
  osMutexAcquire(IwdgMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
  osMutexRelease(IwdgMutexHandle);
#endif
}
