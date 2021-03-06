/*
 * usart_ring.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/usart_ring.h"

/* Private functions prototype
 * --------------------------------------------*/
static void CheckBuffer(uring_t* r);
static void FillBuffer(uring_t* r, size_t start, size_t len);

/* Public functions implementation
 * --------------------------------------------*/
void URING_DMA_Start(uring_t* r) {
  __HAL_UART_ENABLE_IT(r->huart, UART_IT_IDLE);
  __HAL_DMA_ENABLE_IT(r->hdma, DMA_IT_TC);
  __HAL_DMA_ENABLE_IT(r->hdma, DMA_IT_HT);

  HAL_UART_Receive_DMA(r->huart, (uint8_t*)(r->dma.buf), r->dma.sz);
}

void URING_DMA_Stop(uring_t* r) { HAL_UART_DMAStop(r->huart); }

void URING_DMA_IrqHandler(uring_t* r) {
  // if the source is HT
  if (__HAL_DMA_GET_IT_SOURCE(r->hdma, DMA_IT_HT)) {
    __HAL_DMA_CLEAR_FLAG(r->hdma, __HAL_DMA_GET_HT_FLAG_INDEX(r->hdma));

    CheckBuffer(r);
  }
  // if the source is TC
  else if (__HAL_DMA_GET_IT_SOURCE(r->hdma, DMA_IT_TC)) {
    __HAL_DMA_CLEAR_FLAG(r->hdma, __HAL_DMA_GET_TC_FLAG_INDEX(r->hdma));

    CheckBuffer(r);
  }
  // error interrupts
  else {
    __HAL_DMA_CLEAR_FLAG(r->hdma, __HAL_DMA_GET_TE_FLAG_INDEX(r->hdma));
    __HAL_DMA_CLEAR_FLAG(r->hdma, __HAL_DMA_GET_FE_FLAG_INDEX(r->hdma));
    __HAL_DMA_CLEAR_FLAG(r->hdma, __HAL_DMA_GET_DME_FLAG_INDEX(r->hdma));

    HAL_UART_Receive_DMA(r->huart, (uint8_t*)(r->dma.buf), r->dma.sz);
  }
}

void URING_IrqHandler(uring_t* r) {
  if (__HAL_UART_GET_FLAG(r->huart, UART_FLAG_IDLE)) {
    __HAL_UART_CLEAR_IDLEFLAG(r->huart);

    r->tmp.idle = 1;
    CheckBuffer(r);
  }
}

/**
 * Clear rx buffer
 */
void URING_ResetBuffer(uring_t* r) {
  void* dest = &(r->usart.buf[0]);

  memset(dest, 0x00, r->usart.idx);
  r->usart.idx = 0;
}

/* Private functions implementation
 * --------------------------------------------*/
static void CheckBuffer(uring_t* r) {
  size_t pos;

  /* Calculate current position in buffer */
  pos = r->dma.sz - __HAL_DMA_GET_COUNTER(r->hdma);
  if (pos != r->tmp.old_pos) {  /* Check change in received data */
    if (pos > r->tmp.old_pos) { /* Current position is over previous one */
      /* We are in "linear" mode */
      /* Process data directly by subtracting "pointers" */
      FillBuffer(r, r->tmp.old_pos, pos - r->tmp.old_pos);
    } else {
      /* We are in "overflow" mode */
      /* First process data to the end of buffer */
      FillBuffer(r, r->tmp.old_pos, r->dma.sz - r->tmp.old_pos);
      /* Check and continue with beginning of buffer */
      if (pos > 0) {
        FillBuffer(r, 0, pos);
      }
    }
  }

  /* Check and manually update if we reached end of buffer */
  if (pos == r->dma.sz) {
    /* Set index back to first */
    r->tmp.old_pos = 0;
  } else {
    /* Save current position as old */
    r->tmp.old_pos = pos;
  }

  // auto reset buffer after idle
  if (r->IdleCallback != NULL) {
    if (r->tmp.idle) {
      r->tmp.idle = 0;
      r->IdleCallback();
    }
  }
}

/**
 * Write data to buffer
 */
static void FillBuffer(uring_t* r, size_t start, size_t len) {
  void* dest = &(r->usart.buf[r->usart.idx]);
  void* src = &(r->dma.buf[start]);

  // check
  if ((r->usart.idx + 1 + len) > r->usart.sz)
    len = r->usart.sz - (r->usart.idx + 1);

  memcpy(dest, src, len);
  r->usart.idx += len;
}

