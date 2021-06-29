/*
 * _usart_ring_buffer.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_usart_ring.h"

/* Public functions implementation
 * --------------------------------------------*/
void USART_DMA_Start(usart_ring_t* ring) {
  __HAL_UART_ENABLE_IT(ring->huart, UART_IT_IDLE);
  __HAL_DMA_ENABLE_IT(ring->hdma, DMA_IT_TC);
  __HAL_DMA_ENABLE_IT(ring->hdma, DMA_IT_HT);

  HAL_UART_Receive_DMA(ring->huart, (uint8_t*)(ring->dma.buf), ring->dma.sz);
}

void USART_DMA_Stop(usart_ring_t* ring) { HAL_UART_DMAStop(ring->huart); }

void USART_DMA_IrqHandler(usart_ring_t* ring) {
  // if the source is HT
  if (__HAL_DMA_GET_IT_SOURCE(ring->hdma, DMA_IT_HT)) {
    __HAL_DMA_CLEAR_FLAG(ring->hdma, __HAL_DMA_GET_HT_FLAG_INDEX(ring->hdma));

    USART_Check_Buffer(ring);
  }
  // if the source is TC
  else if (__HAL_DMA_GET_IT_SOURCE(ring->hdma, DMA_IT_TC)) {
    __HAL_DMA_CLEAR_FLAG(ring->hdma, __HAL_DMA_GET_TC_FLAG_INDEX(ring->hdma));

    USART_Check_Buffer(ring);
  }
  // error interrupts
  else {
    __HAL_DMA_CLEAR_FLAG(ring->hdma, __HAL_DMA_GET_TE_FLAG_INDEX(ring->hdma));
    __HAL_DMA_CLEAR_FLAG(ring->hdma, __HAL_DMA_GET_FE_FLAG_INDEX(ring->hdma));
    __HAL_DMA_CLEAR_FLAG(ring->hdma, __HAL_DMA_GET_DME_FLAG_INDEX(ring->hdma));

    HAL_UART_Receive_DMA(ring->huart, (uint8_t*)(ring->dma.buf), ring->dma.sz);
  }
}

void USART_IrqHandler(usart_ring_t* ring) {
  if (__HAL_UART_GET_FLAG(ring->huart, UART_FLAG_IDLE)) {
    __HAL_UART_CLEAR_IDLEFLAG(ring->huart);

    ring->tmp.idle = 1;
    USART_Check_Buffer(ring);
  }
}

void USART_Check_Buffer(usart_ring_t* ring) {
  size_t pos;

  /* Calculate current position in buffer */
  pos = ring->dma.sz - __HAL_DMA_GET_COUNTER(ring->hdma);
  if (pos != ring->tmp.old_pos) {  /* Check change in received data */
    if (pos > ring->tmp.old_pos) { /* Current position is over previous one */
      /* We are in "linear" mode */
      /* Process data directly by subtracting "pointers" */
      USART_Fill_Buffer(ring, ring->tmp.old_pos, pos - ring->tmp.old_pos);
    } else {
      /* We are in "overflow" mode */
      /* First process data to the end of buffer */
      USART_Fill_Buffer(ring, ring->tmp.old_pos,
                        ring->dma.sz - ring->tmp.old_pos);
      /* Check and continue with beginning of buffer */
      if (pos > 0) {
        USART_Fill_Buffer(ring, 0, pos);
      }
    }
  }

  /* Check and manually update if we reached end of buffer */
  if (pos == ring->dma.sz) {
    /* Set index back to first */
    ring->tmp.old_pos = 0;
  } else {
    /* Save current position as old */
    ring->tmp.old_pos = pos;
  }

  // auto reset buffer after idle
  if (ring->IdleCallback != NULL) {
    if (ring->tmp.idle) {
      ring->tmp.idle = 0;
      ring->IdleCallback();
    }
  }
}

/**
 * Write data to buffer
 */
void USART_Fill_Buffer(usart_ring_t* ring, size_t start, size_t len) {
  void* dest = &(ring->usart.buf[ring->usart.idx]);
  void* src = &(ring->dma.buf[start]);

  // check
  if ((ring->usart.idx + 1 + len) > ring->usart.sz)
    len = ring->usart.sz - (ring->usart.idx + 1);

  memcpy(dest, src, len);
  ring->usart.idx += len;
}

/**
 * Clear rx buffer
 */
void USART_Reset_Buffer(usart_ring_t* ring) {
  void* dest = &(ring->usart.buf[0]);

  // check
  //	if ((ring->usart.idx + 1) > ring->usart.sz)
  //		memset(dest, 0x00, ring->usart.sz);
  //	else
  memset(dest, 0x00, ring->usart.idx);
  ring->usart.idx = 0;
}
