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
static void Lock(void);
static void UnLock(void);

/* Public functions implementation
 * --------------------------------------------*/
void IWDG_Refresh(void) {
  Lock();
  HAL_IWDG_Refresh(piwdg);
  UnLock();
}

/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(IwdgMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(IwdgMutexHandle);
#endif
}
