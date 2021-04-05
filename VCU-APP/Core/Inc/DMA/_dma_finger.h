/*
 * _dma_finger.h
 *
 *  Created on: Aug 27, 2019
 *      Author: Puja
 */

#ifndef DMA_FINGER_H_
#define DMA_FINGER_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_usart_ring.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define FINGER_UART_RX_SZ (uint16_t)(256)
#define FINGER_DMA_RX_SZ (uint16_t)(64)

/* Exported variables --------------------------------------------------------*/
extern char FINGER_UART_RX[FINGER_UART_RX_SZ];

/* Public functions prototype ------------------------------------------------*/
void FINGER_DMA_Start(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
void FINGER_DMA_Stop(void);
void FINGER_DMA_IrqHandler(void);
void FINGER_USART_IrqHandler(void);
void FINGER_Reset_Buffer(void);
uint8_t FINGER_Transmit(uint8_t *data, uint8_t len);
uint8_t FINGER_Received(void);

#endif /* DMA_FINGER_H_ */
