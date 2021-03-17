/*
 * _bat.c
 *
 *  Created on: Dec 11, 2020
 *      Author: pudja
 */

/* Includes -----------------------------------------------------------------*/
#include "adc.h"
#include "Drivers/_bat.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t BatMutexHandle;
#endif

/* Private variables --------------------------------------------------------*/
static uint16_t BUFFER[AVERAGE_SZ];
static ADC_HandleTypeDef *padc = &hadc1;

/* Private functions declaration ----------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint16_t MovingAverage(uint16_t *pBuffer, uint16_t len, uint16_t value);

/* Public functions implementation ------------------------------------------*/
void BAT_Init(void) {
	lock();
	MX_ADC1_Init();
	unlock();
}

void BAT_DeInit(void) {
	lock();
	HAL_ADC_DeInit(padc);
	unlock();
}

//void BAT_ReInit(void) {
//	lock();
//	BAT_DeInit();
//	_DelayMS(500);
//	BAT_Init();
//	unlock();
//}

uint16_t BAT_ScanValue(void) {
	uint16_t value;
	uint8_t res;

	lock();
	HAL_ADC_Start(padc);
	res = HAL_ADC_PollForConversion(padc, 100) == HAL_OK;

	if (res)
		value = HAL_ADC_GetValue(padc);
	HAL_ADC_Stop(padc);

	if (res) {
		value = (value * BAT_MAX_VOLTAGE ) / ADC_MAX_VALUE;
		value = MovingAverage(BUFFER, AVERAGE_SZ, value);
	}
	unlock();

	return value;
}

/* Private functions implementation --------------------------------------------*/
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
