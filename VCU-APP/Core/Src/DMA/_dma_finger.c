/*
 * _dma_finger.c
 *
 *  Created on: Aug 27, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "DMA/_dma_finger.h"

/* Private constants
 * --------------------------------------------*/
#define FINGER_DMA_RX_SZ ((uint16_t)64)

/* Public variables
 * --------------------------------------------*/
char FINGER_UART_RX[FINGER_UART_RX_SZ];

/* Private variables
 * --------------------------------------------*/
static char FINGER_DMA_RX[FINGER_DMA_RX_SZ];
static usart_ring_t FGR_RING = {
    .IdleCallback = NULL,
    .usart = {.idx = 0, .buf = FINGER_UART_RX, .sz = FINGER_UART_RX_SZ},
    .dma = {.buf = FINGER_DMA_RX, .sz = FINGER_DMA_RX_SZ},
    .tmp = {
        .idle = 1,
        .old_pos = 0,
    }};

/* Public functions implementation
 * --------------------------------------------*/
void FINGER_DMA_Start(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
  FGR_RING.huart = huart;
  FGR_RING.hdma = hdma;
  USART_DMA_Start(&FGR_RING);
}

void FINGER_DMA_Stop(void) { USART_DMA_Stop(&FGR_RING); }

void FINGER_DMA_IrqHandler(void) { USART_DMA_IrqHandler(&FGR_RING); }

void FINGER_USART_IrqHandler(void) { USART_IrqHandler(&FGR_RING); }

void FINGER_Reset_Buffer(void) {
  FGR_RING.tmp.idle = 0;
  USART_ResetBuffer(&FGR_RING);
}

uint8_t FINGER_Transmit(uint8_t *data, uint8_t len) {
  FGR_RING.tmp.idle = 0;
  return (HAL_UART_Transmit(FGR_RING.huart, data, len, HAL_MAX_DELAY) ==
          HAL_OK);
}

uint8_t FINGER_Received(void) { return FGR_RING.tmp.idle; }
