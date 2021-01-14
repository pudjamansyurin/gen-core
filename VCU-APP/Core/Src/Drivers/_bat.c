/*
 * _bat.c
 *
 *  Created on: Dec 11, 2020
 *      Author: pudja
 */

/* Includes -----------------------------------------------------------------*/
#include "adc.h"
#include "Drivers/_bat.h"

/* Private variables --------------------------------------------------------*/
static bat_t bat = {0};

/* Private functions declaration --------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value);

/* Public functions implementation ------------------------------------------*/
void BAT_Init(ADC_HandleTypeDef *hadc) {
  bat.h.adc = hadc;

  MX_ADC1_Init();
  HAL_ADC_Start(bat.h.adc);
}

void BAT_DeInit(void) {
  HAL_ADC_Stop(bat.h.adc);
  HAL_ADC_DeInit(bat.h.adc);
}

void BAT_ReInit(void) {
  BAT_DeInit();
  _DelayMS(500);
  BAT_Init(bat.h.adc);
}

void BAT_ScanValue(uint16_t *voltage) {
  uint16_t value;

  HAL_ADC_PollForConversion(bat.h.adc, 10);
  value = HAL_ADC_GetValue(bat.h.adc);

  // change to battery value
  value = (value * BAT_MAX_VOLTAGE ) / ADC_MAX_VALUE;

  // calculate the moving average
  value = MovingAverage(bat.buffer, AVERAGE_SZ, value);

  *voltage = value;
}

/* Private functions implementation ------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value) {
  static uint32_t sum = 0, pos = 0;
  static uint16_t length = 0;

  //Subtract the oldest number from the prev sum, add the new number
  sum = sum - pBuffer[pos] + value;
  //Assign the nextNum to the position in the array
  pBuffer[pos] = value;
  //Increment position
  pos++;
  if (pos >= len)
    pos = 0;

  // calculate filled array
  if (length < len)
    length++;

  //return the average
  return sum / length;
}
