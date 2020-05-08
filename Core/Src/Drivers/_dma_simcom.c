/*
 * DMA_Simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_dma_simcom.h"

/* External variables ---------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart1_rx;
extern UART_HandleTypeDef huart1;

/* Public variables -----------------------------------------------------------*/
char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static DMA_HandleTypeDef *hdma = &hdma_usart1_rx;
static UART_HandleTypeDef *huart = &huart1;
static char DMA_RX[SIMCOM_DMA_RX_SZ];
static size_t write, len, copy;
static uint8_t *ptr;

/* Public functions implementation ---------------------------------------------*/
void SIMCOM_USART_IrqHandler(void) {
  if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET) { /* if Idle flag is set */
    __HAL_UART_CLEAR_IDLEFLAG(huart); /* Clear idle flag */
    __HAL_DMA_DISABLE(hdma); /* Disabling DMA will force transfer complete interrupt if enabled */
    SIMCOM_DMA_IrqHandler();
  }
}

void SIMCOM_DMA_IrqHandler(void) {
  if (__HAL_DMA_GET_IT_SOURCE(hdma, DMA_IT_TC) != RESET) { // if the source is TC
    /* Clear the transfer complete flag */
    __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma));
    /* Get the length of the data */
    len = SIMCOM_DMA_RX_SZ - __HAL_DMA_GET_COUNTER(hdma);

    /* Only process if DMA is not empty */
    if (len > 0) {
      /* Get number of bytes we can copy to the end of buffer */
      copy = SIMCOM_UART_RX_SZ - write;

      /* Check how many bytes to copy */
      if (copy > len) {
        copy = len;
      }

      /* write received data for UART main buffer for manipulation later */
      ptr = (uint8_t*) DMA_RX;

      /* Copy first part */
      memcpy(&SIMCOM_UART_RX[write], ptr, copy);

      /* Correct values for remaining data */
      write += copy;
      len -= copy;
      ptr += copy;

      /* If still data to write for beginning of buffer */
      if (len) {
        /* Don't care if we override Read pointer now */
        memcpy(&SIMCOM_UART_RX[0], ptr, len);
        write = len;
      }
    }

    /* Start DMA transfer again */
    hdma->Instance->CR |= DMA_SxCR_EN;
  } else {
    /* Start DMA transfer */
    HAL_UART_Receive_DMA(huart, (uint8_t*) DMA_RX, SIMCOM_DMA_RX_SZ);
  }
}

void SIMCOM_DMA_Init(void) {
  __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);      // enable idle line interrupt
  __HAL_DMA_ENABLE_IT(hdma, DMA_IT_TC);  // enable DMA Tx cplt interrupt
  __HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT); // disable half complete interrupt

  /* Start DMA transfer */
  HAL_UART_Receive_DMA(huart, (uint8_t*) DMA_RX, SIMCOM_DMA_RX_SZ);
}

void SIMCOM_Reset_Buffer(void) {
  // clear rx buffer
  memset(SIMCOM_UART_RX, 0x00, SIMCOM_UART_RX_SZ);
  // set index back to first
  write = 0;
}

uint8_t SIMCOM_Transmit(char *pData, uint16_t Size) {
  return (HAL_UART_Transmit(huart, (uint8_t*) pData, Size, HAL_MAX_DELAY) == HAL_OK);
}
