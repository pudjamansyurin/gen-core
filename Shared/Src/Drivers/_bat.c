/*
 * _bat.c
 *
 *  Created on: Dec 11, 2020
 *      Author: pudja
 */

/* Includes -----------------------------------------------------------------*/
#include "Drivers/_bat.h"
#include "adc.h"

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

uint8_t BAT_ScanValue(uint16_t *voltage) {
	uint8_t res;
	uint16_t value;

	lock();

	HAL_ADC_Start(padc);
	res = HAL_ADC_PollForConversion(padc, 100) == HAL_OK;
	if (res)
		value = HAL_ADC_GetValue(padc);
	HAL_ADC_Stop(padc);

	if (res) {
		// change to battery value
		value = (value * BAT_MAX_VOLTAGE ) / ADC_MAX_VALUE;
		// calculate the moving average
		value = MovingAverage(BUFFER, AVERAGE_SZ, value);
		*voltage = value;
	}

	unlock();
	return res;
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
