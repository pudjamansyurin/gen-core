/*
 * DMA_Simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef DMA_SIMCOM_H_
#define DMA_SIMCOM_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported constants
 * --------------------------------------------*/
#define SIM_UART_RX_SZ ((uint16_t)(1024 + 512 + 128))

/* Exported variables
 * --------------------------------------------*/
extern char SIM_UART_RX[SIM_UART_RX_SZ];

/* Public functions prototype
 * --------------------------------------------*/
void SIM_DMA_Start(UART_HandleTypeDef* huart, DMA_HandleTypeDef* hdma);
void SIM_DMA_Stop(void);
void SIM_USART_IrqHandler(void);
void SIM_DMA_IrqHandler(void);
void SIM_Reset_Buffer(void);
uint8_t SIM_Transmit(char* data, uint16_t Size);

#endif /* DMA_SIMCOM_H_ */
