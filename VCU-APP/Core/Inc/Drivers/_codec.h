/*
 * _codec.h
 *
 *  Created on: Mar 8, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__CODEC_H_
#define INC_DRIVERS__CODEC_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported constants
 * --------------------------------------------*/
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
 // on accurate values, they just guarantee that the application will not remain
 // stuck if the SPI communication is corrupted.
 // You may modify these timeout values depending on CPU frequency and
 // application conditions (interrupts routines ...). */
#define I2Cx_TIMEOUT_MAX 0x1000
#define VERIFY_WRITTENDATA 0

/* Public functions prototype
 * --------------------------------------------*/
uint8_t CODEC_Init(uint8_t Addr);
void CODEC_DeInit(void);
uint8_t CODEC_Write(uint8_t Reg, uint8_t Value);
uint8_t CODEC_Read(uint8_t Reg);

#endif /* INC_DRIVERS__CODEC_H_ */
