/*
 * DMA_Ublox.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "DMA/_dma_ublox.h"

/* Private constants
 * --------------------------------------------*/
#define UBLOX_DMA_RX_SZ ((uint16_t)128)

/* Public variables
 * --------------------------------------------*/
char UBLOX_UART_RX[UBLOX_UART_RX_SZ];

/* Private variables
 * --------------------------------------------*/
static char UBLOX_DMA_RX[UBLOX_DMA_RX_SZ];
static usart_ring_t GPS_RING = {
    .IdleCallback = NULL,
    .usart = {.idx = 0, .buf = UBLOX_UART_RX, .sz = UBLOX_UART_RX_SZ},
    .dma = {.buf = UBLOX_DMA_RX, .sz = UBLOX_DMA_RX_SZ},
    .tmp =
        {
            .idle = 1,
            .old_pos = 0,
        },
};

/* Private functions prototype
 * --------------------------------------------*/
static buffer_func BufferCallback;
static void IdleHandler(void);

/* Public functions implementation
 * --------------------------------------------*/
void UBLOX_DMA_Start(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma,
                     buffer_func callback) {
  GPS_RING.huart = huart;
  GPS_RING.hdma = hdma;
  BufferCallback = callback;
  GPS_RING.IdleCallback = IdleHandler;

  USART_DMA_Start(&GPS_RING);
}

void UBLOX_DMA_Stop(void) { USART_DMA_Stop(&GPS_RING); }

void UBLOX_DMA_IrqHandler(void) { USART_DMA_IrqHandler(&GPS_RING); }

void UBLOX_USART_IrqHandler(void) { USART_IrqHandler(&GPS_RING); }

/* Private functions implementation
 * --------------------------------------------*/
static void IdleHandler(void) {
  BufferCallback(GPS_RING.usart.buf, GPS_RING.usart.idx);
  USART_ResetBuffer(&GPS_RING);
}
