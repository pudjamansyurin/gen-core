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
static size_t ublox_write = 0;

/* Private functions implementation -------------------------------------------*/
static void UBLOX_Check_Buffer(uint8_t idle);
static void UBLOX_Fill_Buffer(const void *data, size_t len);

/* Public functions implementation ---------------------------------------------*/
void UBLOX_USART_IrqHandler(void) {
    /* if Idle flag is set */
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE)) {
        /* Clear idle flag */
        __HAL_UART_CLEAR_IDLEFLAG(&huart2);

        UBLOX_Check_Buffer(1);
    }
}

void UBLOX_DMA_IrqHandler(void) {
    // if the source is HT
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_usart2_rx, DMA_IT_HT)) {
        /* Clear HT flag */
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_HT_FLAG_INDEX(&hdma_usart2_rx));

        UBLOX_Check_Buffer(0);
    }

    // if the source is TC
    else if (__HAL_DMA_GET_IT_SOURCE(&hdma_usart2_rx, DMA_IT_TC)) {
        /* Clear TC flag */
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_usart2_rx));

        UBLOX_Check_Buffer(0);
    }

    // error interrupts
    else {
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_TE_FLAG_INDEX(&hdma_usart2_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_FE_FLAG_INDEX(&hdma_usart2_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_usart2_rx, __HAL_DMA_GET_DME_FLAG_INDEX(&hdma_usart2_rx));

        /* Start DMA transfer */
//        HAL_UART_Receive_DMA(&huart2, (uint8_t*) UBLOX_DMA_RX, UBLOX_DMA_RX_SZ);
    }
}

void UBLOX_DMA_Init(void) {
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);        // enable idle line interrupt
    __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TC);    // enable DMA Tx cplt interrupt
    __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_HT);    // enable half complete interrupt

    /* Start DMA transfer */
    HAL_UART_Receive_DMA(&huart2, (uint8_t*) UBLOX_DMA_RX, UBLOX_DMA_RX_SZ);
}

static void UBLOX_Check_Buffer(uint8_t idle) {
    static uint8_t clear = 1;
    static size_t old_pos;
    size_t pos;

    // clearing
    if (clear) {
        UBLOX_Reset_Buffer();
        clear = 0;
    }

    /* Calculate current position in buffer */
    pos = UBLOX_DMA_RX_SZ - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
    if (pos != old_pos) { /* Check change in received data */
        if (pos > old_pos) { /* Current position is over previous one */
            /* We are in "linear" mode */
            /* Process data directly by subtracting "pointers" */
            UBLOX_Fill_Buffer(&UBLOX_DMA_RX[old_pos], pos - old_pos);
        } else {
            /* We are in "overflow" mode */
            /* First process data to the end of buffer */
            UBLOX_Fill_Buffer(&UBLOX_DMA_RX[old_pos], UBLOX_DMA_RX_SZ - old_pos);
            /* Check and continue with beginning of buffer */
            if (pos > 0) {
                UBLOX_Fill_Buffer(&UBLOX_DMA_RX[0], pos);
            }
        }
    }
    old_pos = pos; /* Save current position as old */

    /* Check and manually update if we reached end of buffer */
    if (old_pos == UBLOX_DMA_RX_SZ) {
        old_pos = 0;
    }

    // handle idle
    if (idle) {
        clear = 1;
    }
}

static void UBLOX_Fill_Buffer(const void *data, size_t len) {
    /* Write data to buffer */
    memcpy(&UBLOX_UART_RX[ublox_write], data, len);
    ublox_write += len;
}

void UBLOX_Reset_Buffer(void) {
    // clear rx buffer
    memset(UBLOX_UART_RX, 0x00, ublox_write);
    ublox_write = 0;
}
