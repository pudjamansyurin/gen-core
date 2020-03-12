/*
 * _dma_finger.c
 *
 *  Created on: Aug 27, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_dma_finger.h"

/* External variables ---------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_uart4_rx;
extern UART_HandleTypeDef huart4;

/* Public variables -----------------------------------------------------------*/
char FINGER_UART_RX[FINGER_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char DMA_RX[FINGER_DMA_RX_SZ];
static size_t write, len, copy;
static uint8_t *ptr;

/* Public functions implementation ---------------------------------------------*/
void FINGER_USART_IrqHandler(void) {
  if (huart4.Instance->SR & UART_FLAG_IDLE) /* if Idle flag is set */
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart4); /* Clear idle flag */
    __HAL_DMA_DISABLE(&hdma_uart4_rx); /* Disabling DMA will force transfer complete interrupt if enabled */
    FINGER_DMA_IrqHandler();
  }
}

void FINGER_DMA_IrqHandler(void) {
  if (__HAL_DMA_GET_IT_SOURCE(&hdma_uart4_rx, DMA_IT_TC) != RESET) // if the source is TC
      {
    /* Clear the transfer complete flag */
    __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_uart4_rx));

    /* Get the length of the data */
    len = FINGER_DMA_RX_SZ - hdma_uart4_rx.Instance->NDTR;
    /* Only process if DMA is not empty */
    if (len > 0) {
      /* Reset the buffer */
      FINGER_Reset_Buffer();
      /* Get number of bytes we can copy to the end of buffer */
      copy = FINGER_UART_RX_SZ - write;
      /* write received data for UART main buffer for manipulation later */
      ptr = (uint8_t*) DMA_RX;
      /* Check how many bytes to copy */
      if (copy > len) {
        copy = len;
      }
      /* Copy first part */
      memcpy(&FINGER_UART_RX[write], ptr, copy);
      /* Correct values for remaining data */
      write += copy;
      len -= copy;
      ptr += copy;

      /* If still data to write for beginning of buffer */
      if (len) {
        /* Don't care if we override Read pointer now */
        memcpy(&FINGER_UART_RX[0], ptr, len);
        write = len;
      }
      // set null at the end
      //			FINGER_UART_RX[write] = '\0';
    }

    /* Start DMA transfer again */
    hdma_uart4_rx.Instance->CR |= DMA_SxCR_EN;
  }
}

void FINGER_DMA_Init(void) {
  __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);      // enable idle line interrupt
  __HAL_DMA_ENABLE_IT(&hdma_uart4_rx, DMA_IT_TC);  // enable DMA Tx cplt interrupt
  __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT); // disable half complete interrupt
  HAL_UART_Receive_DMA(&huart4, (uint8_t*) DMA_RX, FINGER_DMA_RX_SZ);
}

void FINGER_Reset_Buffer(void) {
  // clear rx buffer
  memset(FINGER_UART_RX, 0, sizeof(FINGER_UART_RX));
  // set index back to first
  write = 0;
}
