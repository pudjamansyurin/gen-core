/*
 * _errata.h
 *
 *  Created on: Feb 11, 2021
 *      Author: geni
 */

#ifndef INC_DRIVERS__ERRATA_H_
#define INC_DRIVERS__ERRATA_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Public typedef ------------------------------------------------------------*/
typedef struct {
  I2C_HandleTypeDef   instance;
  uint16_t            sdaPin;
  GPIO_TypeDef*       sdaPort;
  uint16_t            sclPin;
  GPIO_TypeDef*       sclPort;
} I2C_module_t;

/* Public functions prototypes ---------------------------------------------*/
void I2C_ClearBusyFlagErratum1(void);
void I2C_ClearBusyFlagErratum2(I2C_module_t* i2c);

#endif /* INC_DRIVERS__ERRATA_H_ */
