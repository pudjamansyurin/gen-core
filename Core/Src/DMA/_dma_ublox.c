/*
 * DMA_Ublox.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "DMA/_dma_ublox.h"

/* External variables ---------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart2;

/* Public variables -----------------------------------------------------------*/
char UBLOX_UART_RX[UBLOX_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static DMA_HandleTypeDef *hdma = &hdma_usart2_rx;
static UART_HandleTypeDef *huart = &huart2;
static char DMA_RX[UBLOX_DMA_RX_SZ];
static size_t write, len, copy;
static uint8_t *ptr;

/* Public functions implementation ---------------------------------------------*/
void UBLOX_USART_IrqHandler(void) {
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET) { /* if Idle flag is set */
		__HAL_UART_CLEAR_IDLEFLAG(huart); /* Clear idle flag */
		__HAL_DMA_DISABLE(hdma); /* Disabling DMA will force transfer complete interrupt if enabled */
		UBLOX_DMA_IrqHandler();
	}
}

void UBLOX_DMA_IrqHandler(void) {
	if (__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TC) != RESET) { // if the source is TC
		/* Clear the transfer complete flag */
		__HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma));
		/* Get the length of the data */
		len = UBLOX_DMA_RX_SZ - __HAL_DMA_GET_COUNTER(hdma);

		/* Only process if DMA is not empty */
		if (len > 0) {
			/* Reset the buffer */
			UBLOX_Reset_Buffer();

			/* Get number of bytes we can copy to the end of buffer */
			copy = UBLOX_UART_RX_SZ - write;

			/* Check how many bytes to copy */
			if (copy > len) {
				copy = len;
			}

			/* write received data for UART main buffer for manipulation later */
			ptr = (uint8_t*) DMA_RX;

			/* Copy first part */
			memcpy(&UBLOX_UART_RX[write], ptr, copy);

			/* Correct values for remaining data */
			write += copy;
			ptr += copy;
			len -= copy;

			/* If still data to write for beginning of buffer */
			if (len) {
				/* Don't care if we override Read pointer now */
				memcpy(&UBLOX_UART_RX[0], ptr, len);
				write = len;
			}
		}

		/* Start DMA transfer again */
		hdma->Instance->CR |= DMA_SxCR_EN;
	} else {
		/* Start DMA transfer */
		HAL_UART_Receive_DMA(huart, (uint8_t*) DMA_RX, UBLOX_DMA_RX_SZ);
	}
}

void UBLOX_DMA_Init(void) {
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);      // enable idle line interrupt
	__HAL_DMA_ENABLE_IT(hdma, DMA_IT_TC);  // enable DMA Tx cplt interrupt
	__HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT); // disable half complete interrupt

	/* Start DMA transfer */
	HAL_UART_Receive_DMA(huart, (uint8_t*) DMA_RX, UBLOX_DMA_RX_SZ);
}

void UBLOX_Reset_Buffer(void) {
	// clear rx buffer
	memset(UBLOX_UART_RX, 0x00, UBLOX_UART_RX_SZ);
	// set index back to first
	write = 0;
}
