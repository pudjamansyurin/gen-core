/*
 * _dma_battery.h
 *
 *  Created on: Apr 6, 2020
 *      Author: pudja
 */

#ifndef DRIVERS__DMA_BATTERY_H_
#define DRIVERS__DMA_BATTERY_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Public functions prototype ------------------------------------------------*/
void BAT_DMA_Init(void);
void BAT_Debugger(void);
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

#endif /* DRIVERS__DMA_BATTERY_H_ */
