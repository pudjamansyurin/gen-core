/*
 * _user_ring_buffer.h
 *
 *  Created on: Oct 22, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__USART_RING_H_
#define INC_LIBS__USART_RING_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported structs
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
void USART_DMA_Start(usart_ring_t* ring);
void USART_DMA_Stop(usart_ring_t* ring);
void USART_DMA_IrqHandler(usart_ring_t* ring);
void USART_IrqHandler(usart_ring_t* ring);
void USART_Check_Buffer(usart_ring_t* ring);
void USART_Fill_Buffer(usart_ring_t* ring, size_t start, size_t len);
void USART_Reset_Buffer(usart_ring_t* ring);

#endif /* INC_LIBS__USART_RING_H_ */
