/*
 * DMA_Ublox.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef UBLOX_DMA_H_
#define UBLOX_DMA_H_

#include "stm32f4xx_hal.h"
#include <string.h>

#define UBLOX_UART_RX_BUFFER_SIZE 1024
#define UBLOX_DMA_RX_BUFFER_SIZE UBLOX_UART_RX_BUFFER_SIZE

void UBLOX_USART_IrqHandler(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
void UBLOX_DMA_IrqHandler(DMA_HandleTypeDef *hdma, UART_HandleTypeDef *huart);
void UBLOX_DMA_Init(void);
void UBLOX_Reset_Buffer(void);


#endif /* UBLOX_DMA_H_ */
