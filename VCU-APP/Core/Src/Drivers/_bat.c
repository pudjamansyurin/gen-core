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
static uint16_t BUFFER[AVERAGE_SZ];
static ADC_HandleTypeDef *hadc;

/* Private functions declaration --------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value);

/* Public functions implementation ------------------------------------------*/
void BAT_Init(ADC_HandleTypeDef *adc) {
  hadc = adc;

  MX_ADC1_Init();
  HAL_ADC_Start(hadc);
}

void BAT_DeInit(void) {
  HAL_ADC_Stop(hadc);
  HAL_ADC_DeInit(hadc);
}

void BAT_ReInit(void) {
  BAT_DeInit();
  _DelayMS(500);
  BAT_Init(hadc);
}

void BAT_ScanValue(uint16_t *voltage) {
	uint8_t retry = 2;
  uint16_t value;

  while (retry-- && HAL_ADC_PollForConversion(hadc, 10) != HAL_OK) {
		BAT_ReInit();
  }

  value = HAL_ADC_GetValue(hadc);

  // change to battery value
  value = (value * BAT_MAX_VOLTAGE ) / ADC_MAX_VALUE;

  // calculate the moving average
  value = MovingAverage(BUFFER, AVERAGE_SZ, value);

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
