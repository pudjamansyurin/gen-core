/*
 * DMA_Ublox.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef DMA_UBLOX_H_
#define DMA_UBLOX_H_

/* Includes
 * --------------------------------------------*/
#include "Libs/usart_ring.h"

/* Exported constants
 * --------------------------------------------*/
#define UBLOX_UART_RX_SZ ((uint16_t)1024)

/* Exported types
 * --------------------------------------------*/
typedef void (*buffer_func)(const void *ptr, size_t len);

/* Exported variables
 * --------------------------------------------*/
extern char UBLOX_UART_RX[UBLOX_UART_RX_SZ];

/* Public functions prototype
 * --------------------------------------------*/
void UBLOX_DMA_Start(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma,
                     buffer_func callback);
void UBLOX_DMA_Stop(void);
void UBLOX_DMA_IrqHandler(void);
void UBLOX_USART_IrqHandler(void);

#endif /* DMA_UBLOX_H_ */
