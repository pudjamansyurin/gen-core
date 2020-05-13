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
#define FINGER_UART_RX_SZ               256
#define FINGER_DMA_RX_SZ                (FINGER_UART_RX_SZ)

/* Public functions prototype ------------------------------------------------*/
void FINGER_USART_IrqHandler(void);
void FINGER_DMA_IrqHandler(void);
void FINGER_DMA_Init(void);
void FINGER_Reset_Buffer(void);

#endif /* DMA_FINGER_H_ */
