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

/* Public functions prototypes ------------------------------------------*/
void BAT_Init(void);
void BAT_DeInit(void);
// void BAT_ReInit(void);
uint16_t BAT_ScanValue(void);

#endif /* INC_LIBS__BAT_H_ */
