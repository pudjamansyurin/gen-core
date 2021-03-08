/*
 * _codec.h
 *
 *  Created on: Mar 8, 2021
 *      Author: pudja
 */

#ifndef INC_DRIVERS__CODEC_H_
#define INC_DRIVERS__CODEC_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
 // on accurate values, they just guarantee that the application will not remain
 // stuck if the SPI communication is corrupted.
 // You may modify these timeout values depending on CPU frequency and application
 // conditions (interrupts routines ...). */
#define I2Cx_TIMEOUT_MAX                0x1000 /*<! The value of the maximal timeout for BUS waiting loops */

// #define VERIFY_WRITTENDATA

/* Public functions prototype ------------------------------------------------*/
void CODEC_Init(void);
void CODEC_DeInit(void);
void CODEC_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
uint8_t CODEC_Read(uint8_t Addr, uint8_t Reg);
void I2Cx_Error(uint8_t Addr);
uint8_t CODEC_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);

#endif /* INC_DRIVERS__CODEC_H_ */
