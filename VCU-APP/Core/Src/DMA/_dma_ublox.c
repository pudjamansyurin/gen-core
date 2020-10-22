/*
 * DMA_Ublox.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "DMA/_dma_ublox.h"
#include "Drivers/_usart_ring.h"

/* External variables ---------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart2;

/* Public variables -----------------------------------------------------------*/
char UBLOX_UART_RX[UBLOX_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char UBLOX_DMA_RX[UBLOX_DMA_RX_SZ];
static usart_ring_t GPS_RING = {
		.huart = &huart2,
		.hdma = &hdma_usart2_rx,
		.rx_only = 1,
		.usart = {
				.idx = 0,
				.buf = &UBLOX_UART_RX,
				.sz = 0
		},
		.dma = {
				.buf = &UBLOX_DMA_RX,
				.sz = 0
		},
		.tmp = {
				.idle = 1,
				.old_pos = 0,
		}
};

/* Public functions implementation ---------------------------------------------*/
void UBLOX_DMA_Init(void) {
	USART_DMA_Init(&GPS_RING);
}

void UBLOX_DMA_IrqHandler(void) {
	USART_DMA_IrqHandler(&GPS_RING);
}

void UBLOX_USART_IrqHandler(void) {
	USART_IrqHandler(&GPS_RING);
}

