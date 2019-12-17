/*
 * _DMA_Finger.h
 *
 *  Created on: Aug 27, 2019
 *      Author: Puja
 */

#ifndef DMA_FINGER_H_
#define DMA_FINGER_H_

#include "stm32f4xx_hal.h"
#include <string.h>

#define FINGER_UART_RX_BUFFER_SIZE 50
#define FINGER_DMA_RX_BUFFER_SIZE FINGER_UART_RX_BUFFER_SIZE

void FINGER_USART_IrqHandler(void);
void FINGER_DMA_IrqHandler(void);
void FINGER_DMA_Init(void);
void FINGER_Reset_Buffer(void);

#endif /* DMA_FINGER_H_ */
