/*
 * _dma_battery.c
 *
 *  Created on: Apr 6, 2020
 *      Author: pudja
 */

/* Includes -----------------------------------------------------------------*/
#include "DMA/_dma_battery.h"
#include "Nodes/VCU.h"

/* Private macro ------------------------------------------------------------*/
#define VOTLAGE_OFFSET_MV					(float) 400

/* External variables -------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;

/* Public variables ---------------------------------------------------------*/
uint16_t BACKUP_VOLTAGE = 0;

/* Private variables --------------------------------------------------------*/
static uint16_t DMA_BUFFER[DMA_SZ ];
static uint16_t AVERAGE_BUFFER[AVERAGE_SZ ] = { 0 };

/* Private functions declaration --------------------------------------------*/
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value);
static uint16_t AverageBuffer(uint16_t start, uint16_t stop);

/* Public functions implementation ------------------------------------------*/
void BAT_DMA_Init(void) {
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) DMA_BUFFER, DMA_SZ);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
	BACKUP_VOLTAGE = AverageBuffer(0, (DMA_SZ / 2)) + VOTLAGE_OFFSET_MV;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	BACKUP_VOLTAGE = AverageBuffer(((DMA_SZ / 2) - 1), DMA_SZ) + VOTLAGE_OFFSET_MV;
}

void BAT_Debugger(void) {
	LOG_Str("Battery:Voltage = ");
	LOG_Int(BACKUP_VOLTAGE);
	LOG_StrLn(" mV");
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

static uint16_t AverageBuffer(uint16_t start, uint16_t stop) {
	uint32_t temp = 0;

	// sum all buffer sample
	for (uint16_t i = start; i < stop; i++)
		temp += DMA_BUFFER[i];

	// calculate the average
	temp /= (stop - start);

	// calculate the moving average
	temp = MovingAverage(AVERAGE_BUFFER, AVERAGE_SZ, temp);

	// change to battery value
	return (temp * BAT_MAX_VOLTAGE ) / ADC_MAX_VALUE ;
}

