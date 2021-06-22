/*
 * DMA_Simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef DMA_SIMCOM_H_
#define DMA_SIMCOM_H_

/* Includes
 * --------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants
 * --------------------------------------------*/
#define SIMCOM_UART_RX_SZ ((uint16_t)(1024 + 512 + 128))
#define SIMCOM_DMA_RX_SZ ((uint16_t)128)

/* Exported variables
 * --------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];

/* Public functions prototype
 * --------------------------------------------*/
void SIMCOM_DMA_Start(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
void SIMCOM_DMA_Stop(void);
void SIMCOM_USART_IrqHandler(void);
void SIMCOM_DMA_IrqHandler(void);
void SIMCOM_Reset_Buffer(void);
uint8_t SIMCOM_Transmit(char *data, uint16_t Size);

#endif /* DMA_SIMCOM_H_ */
