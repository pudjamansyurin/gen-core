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
static char UBLOX_DMA_RX[UBLOX_DMA_RX_SZ];
static size_t ublox_write = 0, ublox_len, ublox_copy;
static uint8_t *ublox_ptr;

/* Public functions implementation ---------------------------------------------*/
void UBLOX_USART_IrqHandler(void) {
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE) != RESET) { /* if Idle flag is set */
        __HAL_UART_CLEAR_IDLEFLAG(&huart2); /* Clear idle flag */
        __HAL_DMA_DISABLE(&hdma_usart2_rx); /* Disabling DMA will force transfer complete interrupt if enabled */
        UBLOX_DMA_IrqHandler();
    }
}

void UBLOX_DMA_IrqHandler(void) {
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_usart2_rx, DMA_IT_TC) != RESET) { // if the source is TC
        /* Get the length of the data */
        ublox_len = UBLOX_DMA_RX_SZ - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);

        /* Only process if DMA is not empty */
        if (ublox_len > 0) {
            /* Reset the buffer */
            UBLOX_Reset_Buffer();

            /* Get number of bytes we can copy to the end of buffer */
            ublox_copy = UBLOX_UART_RX_SZ - ublox_write;

            /* Check how many bytes to copy */
            if (ublox_copy > ublox_len) {
                ublox_copy = ublox_len;
            }

            /* write received data for UART main buffer for manipulation later */
            ublox_ptr = (uint8_t*) UBLOX_DMA_RX;

            /* Copy first part */
            memcpy(&UBLOX_UART_RX[ublox_write], ublox_ptr, ublox_copy);

            /* Correct values for remaining data */
            ublox_write += ublox_copy;
            ublox_ptr += ublox_copy;
            ublox_len -= ublox_copy;

            /* If still data to write for beginning of buffer */
            if (ublox_len) {
                /* Don't care if we override Read pointer now */
                memcpy(&UBLOX_UART_RX[0], ublox_ptr, ublox_len);
                ublox_write = ublox_len;
            }
        }

        /* Start DMA transfer again */
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_usart2_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_HT_FLAG_INDEX(&hdma_usart2_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_TE_FLAG_INDEX(&hdma_usart2_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_FE_FLAG_INDEX(&hdma_usart2_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_DME_FLAG_INDEX(&hdma_usart2_rx));
        __HAL_DMA_SET_COUNTER(&hdma_usart2_rx, UBLOX_DMA_RX_SZ);
        __HAL_DMA_ENABLE(&hdma_usart2_rx);
    }
    else {
        /* Start DMA transfer */
        HAL_UART_Receive_DMA(&huart2, (uint8_t*) UBLOX_DMA_RX, UBLOX_DMA_RX_SZ);
    }
}

void UBLOX_DMA_Init(void) {
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);      // enable idle line interrupt
    __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TC);  // enable DMA Tx cplt interrupt
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT); // disable half complete interrupt

    /* Start DMA transfer */
    HAL_UART_Receive_DMA(&huart2, (uint8_t*) UBLOX_DMA_RX, UBLOX_DMA_RX_SZ);
}

void UBLOX_Reset_Buffer(void) {
    // clear rx buffer
    memset(UBLOX_UART_RX, 0x00, ublox_write);
    // set index back to first
    ublox_write = 0;
}
