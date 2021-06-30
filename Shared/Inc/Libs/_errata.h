/*
 * _errata.h
 *
 *  Created on: Jun 30, 2021
 *      Author: pujak
 */

#ifndef INC_LIBS__ERRATA_H_
#define INC_LIBS__ERRATA_H_


/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported structs
 * --------------------------------------------*/
typedef struct {
	GPIO_TypeDef* gpio;
	uint32_t alt;
	uint32_t pin;
} i2c_type_t;

typedef struct {
	I2C_HandleTypeDef *hi2c;
	i2c_type_t sda;
	i2c_type_t scl;
} i2c_errata_t;

/* Public functions prototype
 * --------------------------------------------*/
void ERRATA_I2C_ClearBusy_2_14_7(void);
void ERRATA_I2C_ClearBusyFlag(void);

#endif /* INC_LIBS__ERRATA_H_ */
