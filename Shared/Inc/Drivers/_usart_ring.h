/*
 * _user_ring_buffer.h
 *
 *  Created on: Oct 22, 2020
 *      Author: pudja
 */

#ifndef INC_DRIVERS__USART_RING_H_
#define INC_DRIVERS__USART_RING_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported struct --------------------------------------------------------------*/
typedef struct {
	UART_HandleTypeDef *huart;
	DMA_HandleTypeDef *hdma;
	uint8_t rx_only;
	struct {
		size_t idx;
		char *buf;
		uint16_t sz;
	} usart;
	struct {
		char *buf;
		uint16_t sz;
	} dma;
	struct {
		uint8_t idle;
		size_t old_pos;
	} tmp;
} usart_ring_t;

/* Public functions implementation --------------------------------------------*/
void USART_DMA_Init(usart_ring_t *ring);
void USART_DMA_IrqHandler(usart_ring_t *ring);
void USART_IrqHandler(usart_ring_t *ring);
void USART_Check_Buffer(usart_ring_t *ring);
void USART_Fill_Buffer(usart_ring_t *ring, size_t start, size_t len);
void USART_Reset_Buffer(usart_ring_t *ring);

#endif /* INC_DRIVERS__USART_RING_H_ */
