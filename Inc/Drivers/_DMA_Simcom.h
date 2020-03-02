/*
 * DMA_Simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef SIMCOM_DMA_H_
#define SIMCOM_DMA_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include <string.h>

#define SIMCOM_UART_RX_BUFFER_SIZE 512
#define SIMCOM_DMA_RX_BUFFER_SIZE 256

void SIMCOM_USART_IrqHandler(void);
void SIMCOM_DMA_IrqHandler(void);
void SIMCOM_DMA_Init(void);
void SIMCOM_Reset_Buffer(void);
void SIMCOM_Transmit(char *pData, uint16_t Size);

#endif /* SIMCOM_DMA_H_ */
