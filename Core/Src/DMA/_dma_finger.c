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
static size_t finger_write = 0;

/* Private functions implementation -------------------------------------------*/
static void FINGER_Check_Buffer(void);
static void FINGER_Fill_Buffer(const void *data, size_t len);

/* Public functions implementation ---------------------------------------------*/
void FINGER_USART_IrqHandler(void) {
    /* if Idle flag is set */
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_IDLE)) {
        /* Clear idle flag */
        __HAL_UART_CLEAR_IDLEFLAG(&huart4);

        FINGER_Check_Buffer();
    }
}

void FINGER_DMA_IrqHandler(void) {
    // if the source is HT
    if (__HAL_DMA_GET_IT_SOURCE(&hdma_uart4_rx, DMA_IT_HT)) {
        /* Clear HT flag */
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_HT_FLAG_INDEX(&hdma_uart4_rx));

        FINGER_Check_Buffer();
    }

    // if the source is TC
    else if (__HAL_DMA_GET_IT_SOURCE(&hdma_uart4_rx, DMA_IT_TC)) {
        /* Clear TC flag */
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_uart4_rx));

        FINGER_Check_Buffer();
    }

    // error interrupts
    else {
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_TE_FLAG_INDEX(&hdma_uart4_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_FE_FLAG_INDEX(&hdma_uart4_rx));
        __HAL_DMA_CLEAR_FLAG(&hdma_uart4_rx, __HAL_DMA_GET_DME_FLAG_INDEX(&hdma_uart4_rx));

        /* Start DMA transfer */
        HAL_UART_Receive_DMA(&huart4, (uint8_t*) FINGER_DMA_RX, FINGER_DMA_RX_SZ);
    }
}

void FINGER_DMA_Init(void) {
    __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);        // enable idle line interrupt
    __HAL_DMA_ENABLE_IT(&hdma_uart4_rx, DMA_IT_TC);    // enable DMA Tx cplt interrupt
    __HAL_DMA_ENABLE_IT(&hdma_uart4_rx, DMA_IT_HT);    // enable half complete interrupt

    /* Start DMA transfer */
    HAL_UART_Receive_DMA(&huart4, (uint8_t*) FINGER_DMA_RX, FINGER_DMA_RX_SZ);
}

static void FINGER_Check_Buffer(void) {
    static size_t old_pos;
    size_t pos;

    /* Calculate current position in buffer */
    pos = FINGER_DMA_RX_SZ - __HAL_DMA_GET_COUNTER(&hdma_uart4_rx);
    if (pos != old_pos) { /* Check change in received data */
        if (pos > old_pos) { /* Current position is over previous one */
            /* We are in "linear" mode */
            /* Process data directly by subtracting "pointers" */
            FINGER_Fill_Buffer(&FINGER_DMA_RX[old_pos], pos - old_pos);
        } else {
            /* We are in "overflow" mode */
            /* First process data to the end of buffer */
            FINGER_Fill_Buffer(&FINGER_DMA_RX[old_pos], FINGER_DMA_RX_SZ - old_pos);
            /* Check and continue with beginning of buffer */
            if (pos > 0) {
                FINGER_Fill_Buffer(&FINGER_DMA_RX[0], pos);
            }
        }
    }
    old_pos = pos; /* Save current position as old */

    /* Check and manually update if we reached end of buffer */
    if (old_pos == FINGER_DMA_RX_SZ) {
        old_pos = 0;
    }
}

static void FINGER_Fill_Buffer(const void *data, size_t len) {
    /* Write data to buffer */
    memcpy(&FINGER_UART_RX[finger_write], data, len);
    finger_write += len;
}

void FINGER_Reset_Buffer(void) {
    // clear rx buffer
    memset(FINGER_UART_RX, 0x00, finger_write);
    finger_write = 0;
}

uint8_t FINGER_Transmit8(uint8_t *pData) {
    return (HAL_UART_Transmit(&huart4, pData, 1, HAL_MAX_DELAY) == HAL_OK);
}
