/*
 * DMA_Simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef DMA_SIMCOM_H_
#define DMA_SIMCOM_H_

#include "_utils.h"

#define SIMCOM_UART_RX_BUFFER_SIZE 512
#define SIMCOM_DMA_RX_BUFFER_SIZE 256

void SIMCOM_USART_IrqHandler(void);
void SIMCOM_DMA_IrqHandler(void);
void SIMCOM_DMA_Init(void);
void SIMCOM_Reset_Buffer(void);
void SIMCOM_Transmit(char *pData, uint16_t Size);

#endif /* DMA_SIMCOM_H_ */
