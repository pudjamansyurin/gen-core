/*
 * buzzer.c
 *
 *  Created on: Mar 17, 2021
 *      Author: Pudja Mansyurin
 *      Source:
 * https://stm32f4-discovery.net/2014/05/stm32f4-stm32f429-discovery-pwm-tutorial/
 *
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/buzzer.h"

#include "tim.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t BuzzerMutexHandle;
#endif

/* Private functions prototype
 * --------------------------------------------*/
static void Lock(void);
static void UnLock(void);

/* Public functions implementation
 * --------------------------------------------*/
void BUZZER_Write(uint8_t state) {
  Lock();
  if (state)
    HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
  else
    HAL_TIM_PWM_Stop(&htim10, TIM_CHANNEL_1);
  UnLock();
}

/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(BuzzerMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(BuzzerMutexHandle);
#endif
}
