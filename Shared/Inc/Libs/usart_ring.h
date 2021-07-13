/*
 * usart_ring.h
 *
 *  Created on: Oct 22, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__USART_RING_H_
#define INC_LIBS__USART_RING_H_

/* Includes
 * --------------------------------------------*/
#include "App/util.h"

/* Exported types
 * --------------------------------------------*/
typedef struct {
  UART_HandleTypeDef* huart;
  DMA_HandleTypeDef* hdma;
  void (*IdleCallback)(void);
  struct {
    size_t idx;
    char* buf;
    uint16_t sz;
  } usart;
  struct {
    char* buf;
    uint16_t sz;
  } dma;
  struct {
    uint8_t idle;
    size_t old_pos;
  } tmp;
} uring_t;

/* Public functions prototype
 * --------------------------------------------*/
void URING_DMA_Start(uring_t* r);
void URING_DMA_Stop(uring_t* r);
void URING_DMA_IrqHandler(uring_t* r);
void URING_IrqHandler(uring_t* r);
void URING_ResetBuffer(uring_t* r);

#endif /* INC_LIBS__USART_RING_H_ */
