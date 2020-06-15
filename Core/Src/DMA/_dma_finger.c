/*
 * _dma_finger.c
 *
 *  Created on: Aug 27, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "DMA/_dma_finger.h"

/* External variables ---------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_uart4_rx;
extern UART_HandleTypeDef huart4;

/* Public variables -----------------------------------------------------------*/
char FINGER_UART_RX[FINGER_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char FINGER_DMA_RX[FINGER_DMA_RX_SZ];
static size_t finger_len, finger_copy, finger_write = 0;
static uint8_t *finger_ptr;

/* Public functions implementation ---------------------------------------------*/
void FINGER_USART_IrqHandler(void) {
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_IDLE) != RESET) { /* if Idle flag is set */
        __HAL_UART_CLEAR_IDLEFLAG(&huart4); /* Clear idle flag */
        __HAL_DMA_DISABLE(&hdma_uart4_rx); /* Disabling DMA will force transfer complete interrupt if enabled */
        FINGER_DMA_IrqHandler();
    }
}

void FINGER_DMA_IrqHandler(void) {
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_uart4_rx, DMA_IT_TC) != RESET) { // if the source is TC
        /* Get the length of the data */
        finger_len = FINGER_DMA_RX_SZ - __HAL_DMA_GET_COUNTER(&hdma_uart4_rx);

        /* Only process if DMA is not empty */
        if (finger_len > 0) {
            /* Reset the buffer */
            FINGER_Reset_Buffer();

            /* Get number of bytes we can copy to the end of buffer */
            finger_copy = FINGER_UART_RX_SZ - finger_write;

            /* Check how many bytes to copy */
            if (finger_copy > finger_len) {
                finger_copy = finger_len;
            }

            /* write received data for UART main buffer for manipulation later */
            finger_ptr = (uint8_t*) FINGER_DMA_RX;

            /* Copy first part */
            memcpy(&FINGER_UART_RX[finger_write], finger_ptr, finger_copy);

            /* Correct values for remaining data */
            finger_write += finger_copy;
            finger_ptr += finger_copy;
            finger_len -= finger_copy;

            /* If still data to write for beginning of buffer */
            if (finger_len) {
                /* Don't care if we override Read pointer now */
                memcpy(&FINGER_UART_RX[0], finger_ptr, finger_len);
                finger_write = finger_len;
            }
        }

        /* Start DMA transfer again */
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_uart4_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_HT_FLAG_INDEX(&hdma_uart4_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_TE_FLAG_INDEX(&hdma_uart4_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_FE_FLAG_INDEX(&hdma_uart4_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_DME_FLAG_INDEX(&hdma_uart4_rx));
        __HAL_DMA_SET_COUNTER(&hdma_uart4_rx, FINGER_DMA_RX_SZ);
        __HAL_DMA_ENABLE(&hdma_uart4_rx);
    } else {
        /* Start DMA transfer */
        HAL_UART_Receive_DMA(&huart4, (uint8_t*) FINGER_DMA_RX, FINGER_DMA_RX_SZ);
    }
}

void FINGER_DMA_Init(void) {
    __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);      // enable idle line interrupt
    __HAL_DMA_ENABLE_IT(&hdma_uart4_rx, DMA_IT_TC);  // enable DMA Tx cplt interrupt
    __HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT); // disable half complete interrupt

    /* Start DMA transfer */
    HAL_UART_Receive_DMA(&huart4, (uint8_t*) FINGER_DMA_RX, FINGER_DMA_RX_SZ);
}

void FINGER_Reset_Buffer(void) {
    // clear rx buffer
    memset(FINGER_UART_RX, 0x00, finger_write);
    // set index back to first
    finger_write = 0;
}
