/*
 * DMA_Simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 *  See: https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx/blob/master/projects/usart_rx_idle_line_irq_ringbuff_G0/Src/main.c
 */

/* Includes ------------------------------------------------------------------*/
#include <Libs/_usart_ring.h>
#include "DMA/_dma_simcom.h"

/* Public variables -----------------------------------------------------------*/
char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char SIMCOM_DMA_RX[SIMCOM_DMA_RX_SZ];
static usart_ring_t SIMCOM_RING = {
	.rx_only = 0,
	.usart = {
			.idx = 0,
			.buf = SIMCOM_UART_RX,
			.sz = SIMCOM_UART_RX_SZ
	},
	.dma = {
			.buf = SIMCOM_DMA_RX,
			.sz = SIMCOM_DMA_RX_SZ
	},
	.tmp = {
			.idle = 1,
			.old_pos = 0,
	}
};


/* Public functions implementation ---------------------------------------------*/
void SIMCOM_DMA_Start(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
	SIMCOM_RING.huart = huart;
	SIMCOM_RING.hdma = hdma;
	USART_DMA_Start(&SIMCOM_RING);
}

void SIMCOM_DMA_Stop(void) {
  USART_DMA_Stop(&SIMCOM_RING);
}

void SIMCOM_DMA_IrqHandler(void) {
	USART_DMA_IrqHandler(&SIMCOM_RING);
}

void SIMCOM_USART_IrqHandler(void) {
	USART_IrqHandler(&SIMCOM_RING);
}

void SIMCOM_Reset_Buffer(void) {
	USART_Reset_Buffer(&SIMCOM_RING);
}

uint8_t SIMCOM_Transmit(char *data, uint16_t Size) {
	SIMCOM_Reset_Buffer();

	return (HAL_UART_Transmit(SIMCOM_RING.huart, (uint8_t*) data, Size, HAL_MAX_DELAY) == HAL_OK);
}
