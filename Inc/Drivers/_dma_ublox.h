/*
 * DMA_Ublox.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef DMA_UBLOX_H_
#define DMA_UBLOX_H_

#include "_utils.h"

#define UBLOX_UART_RX_BUFFER_SIZE 1024
#define UBLOX_DMA_RX_BUFFER_SIZE UBLOX_UART_RX_BUFFER_SIZE

void UBLOX_USART_IrqHandler(void);
void UBLOX_DMA_IrqHandler(void);
void UBLOX_DMA_Init(void);
void UBLOX_Reset_Buffer(void);

#endif /* DMA_UBLOX_H_ */
