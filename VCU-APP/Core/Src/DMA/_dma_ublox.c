/*
 * DMA_Ublox.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "DMA/_dma_ublox.h"

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
static void (*BufferCallback)(void *ptr, size_t len);
static void IdleHandler(void);

/* Public functions implementation
 * --------------------------------------------*/
void UBLOX_DMA_Start(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma,
                     void (*BufferHandler)(void *ptr, size_t len)) {
  GPS_RING.huart = huart;
  GPS_RING.hdma = hdma;
  BufferCallback = BufferHandler;
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
  USART_Reset_Buffer(&GPS_RING);
}
