/*
 * DMA_Simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef DMA_SIMCOM_H_
#define DMA_SIMCOM_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define SIMCOM_UART_RX_SZ    (uint16_t) (1024+512)
#define SIMCOM_DMA_RX_SZ     (uint16_t) (SIMCOM_UART_RX_SZ/(SIMCOM_UART_RX_SZ/128))

/* Public functions prototype ------------------------------------------------*/
void SIMCOM_USART_IrqHandler(void);
void SIMCOM_DMA_IrqHandler(void);
void SIMCOM_DMA_Init(void);
void SIMCOM_Reset_Buffer(void);
uint8_t SIMCOM_Transmit(char *pData, uint16_t Size);

#endif /* DMA_SIMCOM_H_ */