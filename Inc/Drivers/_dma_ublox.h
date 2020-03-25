/*
 * DMA_Ublox.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef DMA_UBLOX_H_
#define DMA_UBLOX_H_

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"

/* Exported constants --------------------------------------------------------*/
#define UBLOX_UART_RX_SZ                512
#define UBLOX_DMA_RX_SZ                 UBLOX_UART_RX_SZ

/* Public functions prototype ------------------------------------------------*/
void UBLOX_USART_IrqHandler(void);
void UBLOX_DMA_IrqHandler(void);
void UBLOX_DMA_Init(void);
void UBLOX_Reset_Buffer(void);

#endif /* DMA_UBLOX_H_ */
