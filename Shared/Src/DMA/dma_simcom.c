/*
 * dma_simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Pudja Mansyurin
 *  See:
 * https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx/blob/master/projects/usart_rx_idle_line_irq_ringbuff_G0/Src/main.c
 */

/* Includes
 * --------------------------------------------*/
#include "DMA/dma_simcom.h"

#include "Libs/usart_ring.h"

/* Private constants
 * --------------------------------------------*/
#define SIM_DMA_RX_SZ ((uint16_t)128)

/* Public variables
 * --------------------------------------------*/
char SIM_UART_RX[SIM_UART_RX_SZ];

/* Private variables
 * --------------------------------------------*/
static char SIM_DMA_RX[SIM_DMA_RX_SZ];
static uring_t SIM_RING = {
    .IdleCallback = NULL,
    .usart = {.idx = 0, .buf = SIM_UART_RX, .sz = SIM_UART_RX_SZ},
    .dma = {.buf = SIM_DMA_RX, .sz = SIM_DMA_RX_SZ},
    .tmp = {
        .idle = 1,
        .old_pos = 0,
    }};

/* Public functions implementation
 * --------------------------------------------*/
void SIM_DMA_Start(UART_HandleTypeDef* huart, DMA_HandleTypeDef* hdma) {
  SIM_RING.huart = huart;
  SIM_RING.hdma = hdma;
  URING_DMA_Start(&SIM_RING);
}

void SIM_DMA_Stop(void) { URING_DMA_Stop(&SIM_RING); }

void SIM_DMA_IrqHandler(void) { URING_DMA_IrqHandler(&SIM_RING); }

void SIM_USART_IrqHandler(void) { URING_IrqHandler(&SIM_RING); }

void SIM_Reset_Buffer(void) { URING_ResetBuffer(&SIM_RING); }

uint8_t SIM_Transmit(const char* data, uint16_t Size) {
  SIM_Reset_Buffer();

  return (HAL_UART_Transmit(SIM_RING.huart, (uint8_t*)data, Size,
                            HAL_MAX_DELAY) == HAL_OK);
}
