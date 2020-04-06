/*
 * _dma_battery.c
 *
 *  Created on: Apr 6, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_dma_battery.h"

/* External variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern db_t DB;

/* Local constants -----------------------------------------------------------*/
#define ADC_MAX_VALUE               4095    // 12 bit
#define REFFERENCE_MAX_VOLTAGE      3300    // mV
#define RATIO                       1460
#define RATIO_MULTIPLIER            1000

/* Private variables ----------------------------------------------------------*/
static uint16_t DMA_BUFFER[BATTERY_DMA_SZ];

/* Public functions implementation ---------------------------------------------*/
void Battery_DMA_Init(void) {
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) DMA_BUFFER, BATTERY_DMA_SZ);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  uint8_t i;
  uint16_t ADC_AverageValue, ADC_RefferenceVoltage;
  uint32_t tempValue = 0;

  // sum all buffer sample
  for (i = 0; i < BATTERY_DMA_SZ; i++) {
    tempValue += DMA_BUFFER[i];
  }

  // calculate the average
  ADC_AverageValue = (tempValue / BATTERY_DMA_SZ);

  // change to refference value
  ADC_RefferenceVoltage = ADC_AverageValue * REFFERENCE_MAX_VOLTAGE / ADC_MAX_VALUE;

  // change to battery value
  DB.vcu.battery = ADC_RefferenceVoltage * RATIO / RATIO_MULTIPLIER;
}
