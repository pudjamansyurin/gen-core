/*
 * _user_ring.h
 *
 *  Created on: Oct 22, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__USART_RING_H_
#define INC_LIBS__USART_RING_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

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
} usart_ring_t;

/* Public functions prototype
 * --------------------------------------------*/
void USART_DMA_Start(usart_ring_t* r);
void USART_DMA_Stop(usart_ring_t* r);
void USART_DMA_IrqHandler(usart_ring_t* r);
void USART_IrqHandler(usart_ring_t* r);
void USART_CheckBuffer(usart_ring_t* r);
void USART_FillBuffer(usart_ring_t* r, size_t start, size_t len);
void USART_ResetBuffer(usart_ring_t* r);

#endif /* INC_LIBS__USART_RING_H_ */
