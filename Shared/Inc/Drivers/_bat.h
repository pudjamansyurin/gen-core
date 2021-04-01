/*
 * _bat.h
 *
 *  Created on: Dec 11, 2020
 *      Author: pudja
 */

#ifndef INC_LIBS__BAT_H_
#define INC_LIBS__BAT_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Constants -----------------------------------------------------------------*/
#define AVERAGE_SZ (uint8_t)100
#define ADC_MAX_VALUE (float)4095   // 12 bit
#define REF_MAX_VOLTAGE (float)3300 // mV
#define BAT_MAX_VOLTAGE (float)4150 // mV
#define BAT_OFFSET_VOLTAGE (float)0 // mV

/* Exported struct
 * --------------------------------------------------------------*/
typedef struct {
	uint16_t voltage;
	uint16_t buf[AVERAGE_SZ];
	ADC_HandleTypeDef *padc;
} bat_t;

/* Public functions prototypes ------------------------------------------*/
void BAT_Init(void);
void BAT_DeInit(void);
uint16_t BAT_ScanValue(void);

#endif /* INC_LIBS__BAT_H_ */
