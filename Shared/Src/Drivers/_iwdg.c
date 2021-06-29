/*
 * _iwdg.c
 *
 *  Created on: Mar 14, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_iwdg.h"

#include "iwdg.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t IwdgMutexHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static IWDG_HandleTypeDef* piwdg = &hiwdg;

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
void IWDG_Refresh(void) {
  lock();
  HAL_IWDG_Refresh(piwdg);
  unlock();
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (APP)
  osMutexAcquire(IwdgMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (APP)
  osMutexRelease(IwdgMutexHandle);
#endif
}
