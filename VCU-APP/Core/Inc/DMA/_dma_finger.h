/*
 * _dma_finger.h
 *
 *  Created on: Aug 27, 2019
 *      Author: Puja
 */

#ifndef DMA_FINGER_H_
#define DMA_FINGER_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define FINGER_UART_RX_SZ    (uint16_t) 256
#define FINGER_DMA_RX_SZ     (uint16_t) (FINGER_UART_RX_SZ/(FINGER_UART_RX_SZ/128))

/* Public functions prototype ------------------------------------------------*/
void FINGER_DMA_Init(void);
void FINGER_DMA_IrqHandler(void);
void FINGER_USART_IrqHandler(void);
void FINGER_Reset_Buffer(void);
uint8_t FINGER_Transmit8(uint8_t *data);

#endif /* DMA_FINGER_H_ */
