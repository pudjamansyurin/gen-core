/*
 * _bat.c
 *
 *  Created on: Dec 11, 2020
 *      Author: pudja
 */

/* Includes -----------------------------------------------------------------*/
#include "Drivers/_bat.h"

/* External variables -------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;

/* Private variables --------------------------------------------------------*/
static uint16_t AVERAGE_BUFFER[AVERAGE_SZ] = { 0 };

/* Private functions declaration --------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value);

/* Public functions implementation ------------------------------------------*/
void BAT_Init(void) {
  HAL_ADC_Start(&hadc1);
}

void BAT_ScanValue(uint16_t *bat) {
  uint16_t value;

  HAL_ADC_PollForConversion(&hadc1, 50);
  value = HAL_ADC_GetValue(&hadc1);

  // change to battery value
  value = (value * BAT_MAX_VOLTAGE ) / ADC_MAX_VALUE;

  // calculate the moving average
  value = MovingAverage(AVERAGE_BUFFER, AVERAGE_SZ, value);

  *bat = value;
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
