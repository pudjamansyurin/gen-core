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

/* Constants -----------------------------------------------------------------*/
#define DMA_SZ            (uint8_t) 20
#define AVERAGE_SZ       (uint16_t) 40

#define ADC_MAX_VALUE       (float) 4095   // 12 bit
#define REF_MAX_VOLTAGE     (float) 3300   // mV
#define BAT_MAX_VOLTAGE     (float) 4150   // mV
#define BAT_OFFSET_VOLTAGE	(float) 0		 // mV

/* Public functions prototype ------------------------------------------------*/
void BAT_DMA_Init(void);
void BAT_Debugger(void);
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

#endif /* DRIVERS__DMA_BATTERY_H_ */
