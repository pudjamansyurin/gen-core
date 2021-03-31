/*
 * _dma_finger.c
 *
 *  Created on: Aug 27, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "DMA/_dma_finger.h"

/* Public variables
 * -----------------------------------------------------------*/
char FINGER_UART_RX[FINGER_UART_RX_SZ];

/* Private variables
 * ----------------------------------------------------------*/
static char FINGER_DMA_RX[FINGER_DMA_RX_SZ];
static usart_ring_t FINGER_RING = {
    .IdleCallback = NULL,
    .usart = {.idx = 0, .buf = FINGER_UART_RX, .sz = FINGER_UART_RX_SZ},
    .dma = {.buf = FINGER_DMA_RX, .sz = FINGER_DMA_RX_SZ},
    .tmp = {
        .idle = 1,
        .old_pos = 0,
    }};

/* Public functions implementation
 * ---------------------------------------------*/
void FINGER_DMA_Start(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
  FINGER_RING.huart = huart;
  FINGER_RING.hdma = hdma;
  USART_DMA_Start(&FINGER_RING);
}

void FINGER_DMA_Stop(void) { USART_DMA_Stop(&FINGER_RING); }

void FINGER_DMA_IrqHandler(void) { USART_DMA_IrqHandler(&FINGER_RING); }

void FINGER_USART_IrqHandler(void) { USART_IrqHandler(&FINGER_RING); }

void FINGER_Reset_Buffer(void) { USART_Reset_Buffer(&FINGER_RING); }

uint8_t FINGER_Transmit(uint8_t *data, uint8_t len) {
  return (HAL_UART_Transmit(FINGER_RING.huart, data, len, HAL_MAX_DELAY) == HAL_OK);
}
