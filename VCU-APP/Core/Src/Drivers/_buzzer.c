/*
 * _buzzer.c
 *
 *  Created on: Mar 17, 2021
 *      Author: pudja
 *      Source:
 * https://stm32f4-discovery.net/2014/05/stm32f4-stm32f429-discovery-pwm-tutorial/
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_buzzer.h"
#include "tim.h"

/* External variables
 * ---------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t BuzzerMutexHandle;
#endif

/* Private functions declaration
 * ----------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation
 * --------------------------------------------*/
void BUZZER_Write(uint8_t state) {
  lock();
  if (state)
    HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
  else
    HAL_TIM_PWM_Stop(&htim10, TIM_CHANNEL_1);
  unlock();
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
  osMutexAcquire(BuzzerMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
  osMutexRelease(BuzzerMutexHandle);
#endif
}
