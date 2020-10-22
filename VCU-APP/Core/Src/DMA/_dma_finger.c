/*
 * _dma_finger.c
 *
 *  Created on: Aug 27, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "DMA/_dma_finger.h"
#include "Drivers/_usart_ring.h"

/* External variables ---------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_uart4_rx;
extern UART_HandleTypeDef huart4;

/* Public variables -----------------------------------------------------------*/
char FINGER_UART_RX[FINGER_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char FINGER_DMA_RX[FINGER_DMA_RX_SZ];
static usart_ring_t FINGER_RING = {
		.huart = &huart4,
		.hdma = &hdma_uart4_rx,
		.rx_only = 0,
		.usart = {
				.idx = 0,
				.buf = &FINGER_UART_RX,
				.sz = 0
		},
		.dma = {
				.buf = &FINGER_DMA_RX,
				.sz = 0
		},
		.tmp = {
				.idle = 1,
				.old_pos = 0,
		}
};


/* Public functions implementation ---------------------------------------------*/
void FINGER_DMA_Init(void) {
	USART_DMA_Init(&FINGER_RING);
}

void FINGER_DMA_IrqHandler(void) {
	USART_DMA_IrqHandler(&FINGER_RING);
}

void FINGER_USART_IrqHandler(void) {
	USART_IrqHandler(&FINGER_RING);
}


uint8_t FINGER_Transmit8(uint8_t *pData) {
    return (HAL_UART_Transmit(&huart4, pData, 1, HAL_MAX_DELAY) == HAL_OK);
}
