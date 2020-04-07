/*
 * DMA_Ublox.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_dma_ublox.h"

/* External variables ---------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart2;

/* Public variables -----------------------------------------------------------*/
char UBLOX_UART_RX[UBLOX_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char DMA_RX[UBLOX_DMA_RX_SZ];
static size_t write, len, copy;
static uint8_t *ptr;

/* Public functions implementation ---------------------------------------------*/
void UBLOX_USART_IrqHandler(void) {
  if (huart2.Instance->SR & UART_FLAG_IDLE) /* if Idle flag is set */
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart2); /* Clear idle flag */
    __HAL_DMA_DISABLE(&hdma_usart2_rx); /* Disabling DMA will force transfer complete interrupt if enabled */
    UBLOX_DMA_IrqHandler();
  }
}

void UBLOX_DMA_IrqHandler(void) {
  if (__HAL_DMA_GET_IT_SOURCE(&hdma_usart2_rx, DMA_IT_TC) != RESET) { // if the source is TC

    /* Clear the transfer complete flag */
    __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_usart2_rx));

    /* Get the length of the data */
    len = UBLOX_DMA_RX_SZ - hdma_usart2_rx.Instance->NDTR;
    /* Only process if DMA is not empty */
    if (len > 0) {
      /* Reset the buffer */
      UBLOX_Reset_Buffer();
      /* Get number of bytes we can copy to the end of buffer */
      copy = UBLOX_UART_RX_SZ - write;
      /* write received data for UART main buffer for manipulation later */
      ptr = (uint8_t*) DMA_RX;
      /* Check how many bytes to copy */
      if (copy > len) {
        copy = len;
      }
      /* Copy first part */
      memcpy(&UBLOX_UART_RX[write], ptr, copy);
      /* Correct values for remaining data */
      write += copy;
      len -= copy;
      ptr += copy;

      /* If still data to write for beginning of buffer */
      if (len) {
        /* Don't care if we override Read pointer now */
        memcpy(&UBLOX_UART_RX[0], ptr, len);
        write = len;
      }
      // set null at the end
      UBLOX_UART_RX[write] = '\0';
    }

    /* Start DMA transfer again */
    hdma_usart2_rx.Instance->CR |= DMA_SxCR_EN;
  }
}

void UBLOX_DMA_Init(void) {
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);      // enable idle line interrupt
  __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TC);  // enable DMA Tx cplt interrupt
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT); // disable half complete interrupt
  HAL_UART_Receive_DMA(&huart2, (uint8_t*) DMA_RX, UBLOX_DMA_RX_SZ);
}

void UBLOX_Reset_Buffer(void) {
  // clear rx buffer
  //	memset(UBLOX_UART_RX, 0, sizeof(UBLOX_UART_RX));
  // set index back to first
  write = 0;
  //	 set null at the end
  UBLOX_UART_RX[write] = '\0';
}
