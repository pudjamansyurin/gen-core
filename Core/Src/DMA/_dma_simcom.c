/*
 * DMA_Simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "DMA/_dma_simcom.h"

/* External variables ---------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart1_rx;
extern UART_HandleTypeDef huart1;

/* Public variables -----------------------------------------------------------*/
char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];

/* Private variables ----------------------------------------------------------*/
static char SIMCOM_DMA_RX[SIMCOM_DMA_RX_SZ];
static size_t simcom_write = 0, simcom_len, simcom_copy;
static uint8_t *simcom_ptr;

/* Public functions implementation ---------------------------------------------*/
void SIMCOM_USART_IrqHandler(void) {
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET) { /* if Idle flag is set */
        __HAL_UART_CLEAR_IDLEFLAG(&huart1); /* Clear idle flag */
        __HAL_DMA_DISABLE(&hdma_usart1_rx); /* Disabling DMA will force transfer complete interrupt if enabled */
        SIMCOM_DMA_IrqHandler();
    }
}

void SIMCOM_DMA_IrqHandler(void) {
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_usart1_rx, DMA_IT_TC) != RESET) { // if the source is TC
        /* Get the length of the data */
        simcom_len = SIMCOM_DMA_RX_SZ - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);

        /* Only process if DMA is not empty */
        if (simcom_len > 0) {
            /* Get number of bytes we can copy to the end of buffer */
            simcom_copy = SIMCOM_UART_RX_SZ - simcom_write;

            /* Check how many bytes to copy */
            if (simcom_copy > simcom_len) {
                simcom_copy = simcom_len;
            }

            /* write received data for UART main buffer for manipulation later */
            simcom_ptr = (uint8_t*) SIMCOM_DMA_RX;

            /* Copy first part */
            memcpy(&SIMCOM_UART_RX[simcom_write], simcom_ptr, simcom_copy);

            /* Correct values for remaining data */
            simcom_write += simcom_copy;
            simcom_len -= simcom_copy;
            simcom_ptr += simcom_copy;

            /* If still data to write for beginning of buffer */
            if (simcom_len) {
                /* Don't care if we override Read pointer now */
                memcpy(&SIMCOM_UART_RX[0], simcom_ptr, simcom_len);
                simcom_write = simcom_len;
            }
        }

        /* Start DMA transfer again */
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_usart1_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_HT_FLAG_INDEX(&hdma_usart1_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_TE_FLAG_INDEX(&hdma_usart1_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_FE_FLAG_INDEX(&hdma_usart1_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_DME_FLAG_INDEX(&hdma_usart1_rx));
        __HAL_DMA_SET_COUNTER(&hdma_usart1_rx, SIMCOM_DMA_RX_SZ);
        __HAL_DMA_ENABLE(&hdma_usart1_rx);
    }
    else {
        /* Start DMA transfer */
        HAL_UART_Receive_DMA(&huart1, (uint8_t*) SIMCOM_DMA_RX, SIMCOM_DMA_RX_SZ);
    }
}

void SIMCOM_DMA_Init(void) {
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);      // enable idle line interrupt
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_TC);  // enable DMA Tx cplt interrupt
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT); // disable half complete interrupt

    /* Start DMA transfer */
    HAL_UART_Receive_DMA(&huart1, (uint8_t*) SIMCOM_DMA_RX, SIMCOM_DMA_RX_SZ);
}

void SIMCOM_Reset_Buffer(void) {
    // clear rx buffer
    memset(SIMCOM_UART_RX, 0x00, simcom_write);
    // set index back to first
    simcom_write = 0;
}

uint8_t SIMCOM_Transmit(char *pData, uint16_t Size) {
    return (HAL_UART_Transmit(&huart1, (uint8_t*) pData, Size, HAL_MAX_DELAY) == HAL_OK);
}
