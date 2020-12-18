/*
 * _dma_finger.c
 *
 *  Created on: Aug 27, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include <Libs/_usart_ring.h>
#include "DMA/_dma_finger.h"

/* Public variables -----------------------------------------------------------*/
char FINGER_UART_RX[FINGER_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char FINGER_DMA_RX[FINGER_DMA_RX_SZ];
static usart_ring_t FINGER_RING = {
		.rx_only = 0,
		.usart = {
				.idx = 0,
				.buf = FINGER_UART_RX,
				.sz = FINGER_UART_RX_SZ
		},
		.dma = {
				.buf = FINGER_DMA_RX,
				.sz = FINGER_DMA_RX_SZ
		},
		.tmp = {
				.idle = 1,
				.old_pos = 0,
		}
};

/* Public functions implementation ---------------------------------------------*/
void FINGER_DMA_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
	FINGER_RING.huart = huart;
	FINGER_RING.hdma = hdma;
	USART_DMA_Init(&FINGER_RING);
}

void FINGER_DMA_IrqHandler(void) {
	USART_DMA_IrqHandler(&FINGER_RING);
}

void FINGER_USART_IrqHandler(void) {
	USART_IrqHandler(&FINGER_RING);
}

void FINGER_Reset_Buffer(void) {
	USART_Reset_Buffer(&FINGER_RING);
}

uint8_t FINGER_Transmit8(uint8_t *data) {
	return (HAL_UART_Transmit(FINGER_RING.huart, data, 1, HAL_MAX_DELAY) == HAL_OK);
}
