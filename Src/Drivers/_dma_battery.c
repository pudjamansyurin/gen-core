/*
 * _dma_battery.c
 *
 *  Created on: Apr 6, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_dma_battery.h"
#include "_simcom.h"

/* External variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern db_t DB;
extern sim_t SIM;

/* Local constants -----------------------------------------------------------*/
#define DMA_SZ                      25
#define AVERAGE_SZ                  255

#define ADC_MAX_VALUE               4095    // 12 bit
#define REFFERENCE_MAX_VOLTAGE      3300    // mV
#define RATIO                       1260
#define RATIO_MULTIPLIER            1000

/* Private variables ----------------------------------------------------------*/
static uint16_t DMA_BUFFER[DMA_SZ];
static uint16_t AVERAGE_BUFFER[AVERAGE_SZ] = { 0 };

/* Public functions declaration ------------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint8_t len, uint16_t value);

/* Public functions implementation ---------------------------------------------*/
void Battery_DMA_Init(void) {
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) DMA_BUFFER, DMA_SZ);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  uint8_t i;
  uint16_t ADC_AverageValue, ADC_RefferenceVoltage;
  uint32_t tempValue = 0;

  // only capture when simcom is sleeping
  if ((SIM.sleep && !SIM.uploading) || SIM.state == SIM_STATE_DOWN) {
    // sum all buffer sample
    for (i = 0; i < DMA_SZ; i++) {
      tempValue += DMA_BUFFER[i];
    }

    // calculate the average
    ADC_AverageValue = MovingAverage(AVERAGE_BUFFER, AVERAGE_SZ, (tempValue / DMA_SZ));

    // change to refference value
    ADC_RefferenceVoltage = ADC_AverageValue * REFFERENCE_MAX_VOLTAGE / ADC_MAX_VALUE;

    // change to battery value
    DB.vcu.battery = ADC_RefferenceVoltage * RATIO / RATIO_MULTIPLIER;
  }
}

/* Private functions implementation ---------------------------------------------*/
uint16_t MovingAverage(uint16_t *pBuffer, uint8_t len, uint16_t value) {
  static uint32_t Sum = 0, pos = 0;

  //Subtract the oldest number from the prev sum, add the new number
  Sum = Sum - pBuffer[pos] + value;
  //Assign the nextNum to the position in the array
  pBuffer[pos] = value;
  //Increment position
  pos++;
  if (pos >= len) {
    pos = 0;
  }
  //return the average
  return Sum / len;
}
