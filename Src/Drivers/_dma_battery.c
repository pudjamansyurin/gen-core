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
#define DMA_SZ                      500U
#define AVERAGE_SZ                  1000U
#define ADC_MAX_VALUE               4095U    // 12 bit
#define REF_MAX_VOLTAGE             3300U    // mV
#define BAT_MAX_VOLTAGE             4250U    // mV

/* Private variables ----------------------------------------------------------*/
static uint16_t DMA_BUFFER[DMA_SZ];
static uint16_t AVERAGE_BUFFER[AVERAGE_SZ] = { 0 };

/* Public functions declaration ------------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value);

/* Public functions implementation ---------------------------------------------*/
void BAT_DMA_Init(void) {
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) DMA_BUFFER, DMA_SZ);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
  uint16_t i;
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
  uint16_t i;
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
  DB.vcu.bat_voltage = (temp * BAT_MAX_VOLTAGE) / ADC_MAX_VALUE;
}

/* Private functions implementation ---------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value) {
  static uint32_t sum = 0, pos = 0;
  static uint16_t length = 0;

  //Subtract the oldest number from the prev sum, add the new number
  sum = sum - pBuffer[pos] + value;
  //Assign the nextNum to the position in the array
  pBuffer[pos] = value;
  //Increment position
  pos++;
  if (pos >= len) {
    pos = 0;
  }
  // calculate filled array
  if (length < len) {
    length++;
  }
  //return the average
  return sum / length;
}
