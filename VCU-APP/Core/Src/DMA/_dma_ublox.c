/*
 * DMA_Ublox.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "DMA/_dma_ublox.h"
#include "Libs/_usart_ring.h"

/* Public variables -----------------------------------------------------------*/
char UBLOX_UART_RX[UBLOX_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char UBLOX_DMA_RX[UBLOX_DMA_RX_SZ];
static usart_ring_t GPS_RING = {
		.rx_only = 1,
		.usart = {
				.idx = 0,
				.buf = UBLOX_UART_RX,
				.sz = UBLOX_UART_RX_SZ
		},
		.dma = {
				.buf = UBLOX_DMA_RX,
				.sz = UBLOX_DMA_RX_SZ
		},
		.tmp = {
				.idle = 1,
				.old_pos = 0,
		}
};

/* Public functions implementation ---------------------------------------------*/
void UBLOX_DMA_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
	GPS_RING.huart = huart;
	GPS_RING.hdma = hdma;
	USART_DMA_Init(&GPS_RING);
}

void UBLOX_DMA_IrqHandler(void) {
	USART_DMA_IrqHandler(&GPS_RING);
}

void UBLOX_USART_IrqHandler(void) {
	USART_IrqHandler(&GPS_RING);
}

