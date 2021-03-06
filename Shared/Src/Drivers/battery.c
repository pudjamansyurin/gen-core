/*
 * battery.c
 *
 *  Created on: Dec 11, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/battery.h"

#include "adc.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t BatMutexHandle;
#endif

/* Private constants
 * --------------------------------------------*/
#define BAT_SAMPLE_SZ ((uint8_t)100)
#define ADC_MAX_VALUE ((float)4095.0)  // 12 bit
#define REF_MAX_MV ((float)3300.0)
#define BAT_MAX_MV ((float)4150.0)
#define BAT_OFFSET_MV ((float)0.0)

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint16_t voltage;
  uint16_t buf[BAT_SAMPLE_SZ];
  ADC_HandleTypeDef* padc;
} bat_t;

/* Private variables
 * --------------------------------------------*/
static bat_t BAT = {
    .voltage = 0,
    .buf = {0},
    .padc = &hadc1,
};

/* Private functions prototype
 * --------------------------------------------*/
static void Lock(void);
static void UnLock(void);
static uint16_t Sampling(uint16_t val);

/* Public functions implementation
 * --------------------------------------------*/
void BAT_Init(void) {
  Lock();
  MX_ADC1_Init();
  UnLock();
}

void BAT_DeInit(void) {
  Lock();
  BAT.voltage = 0;
  HAL_ADC_DeInit(BAT.padc);
  UnLock();
}

uint16_t BAT_ScanVoltage(void) {
  uint16_t value = 0;
  uint8_t ok;

  Lock();
  HAL_ADC_Start(BAT.padc);
  ok = HAL_ADC_PollForConversion(BAT.padc, 5) == HAL_OK;
  if (ok) value = HAL_ADC_GetValue(BAT.padc);
  HAL_ADC_Stop(BAT.padc);

  if (ok) {
    value = (value * BAT_MAX_MV) / ADC_MAX_VALUE;
    value = Sampling(value);
  }

  BAT.voltage = value;
  UnLock();

  return value;
}

uint16_t BAT_IO_Voltage(void) { return BAT.voltage; }

/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(BatMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(BatMutexHandle);
#endif
}

static uint16_t Sampling(uint16_t val) {
  static uint32_t sum = 0;
  static uint16_t pos = 0, len = 0;
  uint16_t* buf = BAT.buf;
  uint16_t sz = BAT_SAMPLE_SZ;

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
