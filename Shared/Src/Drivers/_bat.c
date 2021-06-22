/*
 * _bat.c
 *
 *  Created on: Dec 11, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_bat.h"

#include "adc.h"

/* External variables
 * --------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t BatMutexHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static bat_t BAT = {
    .voltage = 0,
    .buf = {0},
    .padc = &hadc1,
};

/* Private functions prototype
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint16_t MovAvg(uint16_t* buf, uint16_t sz, uint16_t val);

/* Public functions implementation
 * --------------------------------------------*/
void BAT_Init(void) {
  lock();
  MX_ADC1_Init();
  unlock();
}

void BAT_DeInit(void) {
  lock();
  BAT.voltage = 0;
  HAL_ADC_DeInit(BAT.padc);
  unlock();
}

uint16_t BAT_ScanValue(void) {
  uint16_t value = 0;
  uint8_t ok;

  lock();
  HAL_ADC_Start(BAT.padc);
  ok = HAL_ADC_PollForConversion(BAT.padc, 5) == HAL_OK;
  if (ok) value = HAL_ADC_GetValue(BAT.padc);
  HAL_ADC_Stop(BAT.padc);

  if (ok) {
    value = (value * BAT_MAX_MV) / ADC_MAX_VALUE;
    value = MovAvg(BAT.buf, BAT_AVG_SZ, value);
  }

  BAT.voltage = value;
  unlock();

  return value;
}

/* Private functions implementation
 * --------------------------------------------*/
static void lock(void) {
#if (RTOS_ENABLE)
  osMutexAcquire(BatMutexHandle, osWaitForever);
#endif
}

static void unlock(void) {
#if (RTOS_ENABLE)
  osMutexRelease(BatMutexHandle);
#endif
}

static uint16_t MovAvg(uint16_t* buf, uint16_t sz, uint16_t val) {
  static uint32_t sum = 0;
  static uint16_t pos = 0, len = 0;

  // Subtract the oldest number from the prev sum, add the new number
  sum = sum - buf[pos] + val;
  // Assign the nextNum to the position in the array
  buf[pos] = val;
  // Increment position
  if (++pos >= sz) pos = 0;
  // calculate filled array
  if (len < sz) len++;
  // return the average
  return sum / len;
}
