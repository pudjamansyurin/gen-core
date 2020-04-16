/*
 * _dma_battery.h
 *
 *  Created on: Apr 6, 2020
 *      Author: pudja
 */

#ifndef DRIVERS__DMA_BATTERY_H_
#define DRIVERS__DMA_BATTERY_H_

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"

/* Public functions prototype ------------------------------------------------*/
void Battery_DMA_Init(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

#endif /* DRIVERS__DMA_BATTERY_H_ */
