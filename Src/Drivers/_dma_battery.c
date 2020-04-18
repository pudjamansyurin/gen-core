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
#define DMA_SZ                      10
#define AVERAGE_SZ                  500U
#define ADC_MAX_VALUE               4095    // 12 bit
#define REF_MAX_VOLTAGE             3300    // mV
#define BAT_MAX_VOLTAGE             4250    // mV
#define RATIO                       (BAT_MAX_VOLTAGE / ADC_MAX_VALUE)

/* Private variables ----------------------------------------------------------*/
static uint16_t DMA_BUFFER[DMA_SZ];
static uint16_t AVERAGE_BUFFER[AVERAGE_SZ] = { 0 };

/* Public functions declaration ------------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value);

/* Public functions implementation ---------------------------------------------*/
void BAT_DMA_Init(void) {
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) DMA_BUFFER, DMA_SZ);
  HAL_Delay(1000);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  uint8_t i;
  uint32_t temp = 0;

  // sum all buffer sample
  for (i = 0; i < (DMA_SZ / 2); i++) {
    temp += DMA_BUFFER[i];
  }
  // calculate the average
  temp = temp / (DMA_SZ / 2);
  // calculate the moving average
  MovingAverage(AVERAGE_BUFFER, AVERAGE_SZ, temp);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  uint8_t i;
  uint32_t temp = 0;

  // sum all buffer sample
  for (i = ((DMA_SZ / 2) - 1); i < DMA_SZ; i++) {
    temp += DMA_BUFFER[i];
  }
  // calculate the average
  temp = temp / (DMA_SZ / 2);
  // calculate the moving average
  temp = MovingAverage(AVERAGE_BUFFER, AVERAGE_SZ, temp);
  // change to battery value
  DB.vcu.battery = temp * RATIO;
}

/* Private functions implementation ---------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value) {
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
