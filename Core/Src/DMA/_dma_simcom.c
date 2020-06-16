/*
 * DMA_Simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 *  See: https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx/blob/master/projects/usart_rx_idle_line_irq_ringbuff_G0/Src/main.c
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
static size_t simcom_write = 0;

/* Private functions implementation -------------------------------------------*/
static void SIMCOM_Check_Buffer(void);
static void SIMCOM_Fill_Buffer(const void *data, size_t len);

/* Public functions implementation ---------------------------------------------*/
void SIMCOM_USART_IrqHandler(void) {
    /* if Idle flag is set */
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE)) {
        /* Clear idle flag */
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);

        SIMCOM_Check_Buffer();
    }
}

void SIMCOM_DMA_IrqHandler(void) {
    // if the source is HT
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_usart1_rx, DMA_IT_HT)) {
        /* Clear HT flag */
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_HT_FLAG_INDEX(&hdma_usart1_rx));

        SIMCOM_Check_Buffer();
    }

    // if the source is TC
    else if (__HAL_DMA_GET_IT_SOURCE(&hdma_usart1_rx, DMA_IT_TC)) {
        /* Clear TC flag */
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_usart1_rx));

        SIMCOM_Check_Buffer();
    }

    // error interrupts
    else {
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_TE_FLAG_INDEX(&hdma_usart1_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_FE_FLAG_INDEX(&hdma_usart1_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart1_rx, __HAL_DMA_GET_DME_FLAG_INDEX(&hdma_usart1_rx));

        /* Start DMA transfer */
//        HAL_UART_Receive_DMA(&huart1, (uint8_t*) SIMCOM_DMA_RX, SIMCOM_DMA_RX_SZ);
    }
}

void SIMCOM_DMA_Init(void) {
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);        // enable idle line interrupt
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_TC);    // enable DMA Tx cplt interrupt
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_HT);    // enable half complete interrupt

    /* Start DMA transfer */
    HAL_UART_Receive_DMA(&huart1, (uint8_t*) SIMCOM_DMA_RX, SIMCOM_DMA_RX_SZ);
}

static void SIMCOM_Check_Buffer(void) {
    static size_t old_pos;
    size_t pos;

    /* Calculate current position in buffer */
    pos = SIMCOM_DMA_RX_SZ - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
    if (pos != old_pos) { /* Check change in received data */
        if (pos > old_pos) { /* Current position is over previous one */
            /* We are in "linear" mode */
            /* Process data directly by subtracting "pointers" */
            SIMCOM_Fill_Buffer(&SIMCOM_DMA_RX[old_pos], pos - old_pos);
        } else {
            /* We are in "overflow" mode */
            /* First process data to the end of buffer */
            SIMCOM_Fill_Buffer(&SIMCOM_DMA_RX[old_pos], SIMCOM_DMA_RX_SZ - old_pos);
            /* Check and continue with beginning of buffer */
            if (pos > 0) {
                SIMCOM_Fill_Buffer(&SIMCOM_DMA_RX[0], pos);
            }
        }
    }
    old_pos = pos; /* Save current position as old */

    /* Check and manually update if we reached end of buffer */
    if (old_pos == SIMCOM_DMA_RX_SZ) {
        old_pos = 0;
    }
}

static void SIMCOM_Fill_Buffer(const void *data, size_t len) {
    /* Write data to buffer */
    memcpy(&SIMCOM_UART_RX[simcom_write], data, len);
    simcom_write += len;
}

void SIMCOM_Reset_Buffer(void) {
    // clear rx buffer
    memset(SIMCOM_UART_RX, 0x00, simcom_write);
    simcom_write = 0;
}

uint8_t SIMCOM_Transmit(char *pData, uint16_t Size) {
    return (HAL_UART_Transmit(&huart1, (uint8_t*) pData, Size, HAL_MAX_DELAY) == HAL_OK);
}
