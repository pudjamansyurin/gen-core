/*
 * DMA_Simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#include "_DMA_Simcom.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
UART_HandleTypeDef *huart_simcom = &huart1;
DMA_HandleTypeDef *hdma_simcom = &hdma_usart1_rx;

char SIMCOM_DMA_RX_Buffer[SIMCOM_DMA_RX_BUFFER_SIZE];
char SIMCOM_UART_RX_Buffer[SIMCOM_UART_RX_BUFFER_SIZE];
size_t simcom_write, simcom_len, simcom_tocopy;
uint8_t *simcom_ptr;

void SIMCOM_USART_IrqHandler(void) {
	if (huart_simcom->Instance->SR & UART_FLAG_IDLE) /* if Idle flag is set */
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart_simcom); /* Clear idle flag */
		__HAL_DMA_DISABLE(hdma_simcom); /* Disabling DMA will force transfer complete interrupt if enabled */
		SIMCOM_DMA_IrqHandler();
	}
}

void SIMCOM_DMA_IrqHandler(void) {
	if (__HAL_DMA_GET_IT_SOURCE(hdma_simcom, DMA_IT_TC) != RESET) { // if the source is TC
		/* Clear the transfer complete flag */
		__HAL_DMA_CLEAR_FLAG(hdma_simcom, __HAL_DMA_GET_TC_FLAG_INDEX(hdma_simcom));
		/* Get the simcom_length of the data */
		simcom_len = SIMCOM_DMA_RX_BUFFER_SIZE - hdma_simcom->Instance->NDTR;

		/* Only process if DMA is not empty */
		if (simcom_len > 0) {
			/* Get number of bytes we can copy to the end of buffer */
			simcom_tocopy = SIMCOM_UART_RX_BUFFER_SIZE - simcom_write;
			/* simcom_write received data for UART main buffer for manipulation later */
			simcom_ptr = (uint8_t*) SIMCOM_DMA_RX_Buffer;
			/* Check how many bytes to copy */
			if (simcom_tocopy > simcom_len) {
				simcom_tocopy = simcom_len;
			}
			/* Copy first part */
			memcpy(&SIMCOM_UART_RX_Buffer[simcom_write], simcom_ptr, simcom_tocopy);
			/* Correct values for remaining data */
			simcom_write += simcom_tocopy;
			simcom_len -= simcom_tocopy;
			simcom_ptr += simcom_tocopy;
			/* If still data to simcom_write for beginning of buffer */
			if (simcom_len) {
				/* Don't care if we override Read pointer now */
				memcpy(&SIMCOM_UART_RX_Buffer[0], simcom_ptr, simcom_len);
				simcom_write = simcom_len;
			}
			// set null at the end
			SIMCOM_UART_RX_Buffer[simcom_write] = '\0';
		}

		/* Start DMA transfer again */
		hdma_simcom->Instance->CR |= DMA_SxCR_EN;
	}
}

void SIMCOM_DMA_Init(void) {
	__HAL_UART_ENABLE_IT(huart_simcom, UART_IT_IDLE);      // enable idle line interrupt
	__HAL_DMA_ENABLE_IT(hdma_simcom, DMA_IT_TC);  // enable DMA Tx cplt interrupt
	__HAL_DMA_DISABLE_IT(hdma_simcom, DMA_IT_HT); // disable half complete interrupt
	HAL_UART_Receive_DMA(huart_simcom, (uint8_t*) SIMCOM_DMA_RX_Buffer, SIMCOM_DMA_RX_BUFFER_SIZE);
}

void SIMCOM_Reset_Buffer(void) {
	// FIXME: just add check command here, before clearing the buffer
	// show before clear
	//	SWV_SendStr("\n----------------------------------------\n");
	//	SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
	//	SWV_SendStr("\n========================================\n");
	// clear rx buffer
	memset(SIMCOM_UART_RX_Buffer, 0, strlen(SIMCOM_UART_RX_Buffer));
	// wail until clear is done
	osDelay(50);
	// set index back to first
	simcom_write = 0;
	// set null at the end
	//	SIMCOM_UART_RX_Buffer[simcom_write] = '\0';
}

void SIMCOM_Transmit(char *pData, uint16_t Size) {
	HAL_UART_Transmit(huart_simcom, (uint8_t*) pData, Size, HAL_MAX_DELAY);
}
