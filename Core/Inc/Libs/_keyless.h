/*
 * _keyless.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef KEYLESS_H_
#define KEYLESS_H_

/* Includes ------------------------------------------------------------------*/
#include "_nrf24l01.h"

/* Exported define -----------------------------------------------------------*/
#define NRF_DATA_LENGTH														8

/* Exported struct -----------------------------------------------------------*/
typedef struct {
	uint8_t rx[NRF_DATA_LENGTH];
} payload_t;

/* Public functions prototype ------------------------------------------------*/
void KEYLESS_Init(void);
void KEYLESS_Debugger(void);
uint8_t KEYLESS_ReadPayload(void);
void KEYLESS_IrqHandler(void);
void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data);

#endif /* KEYLESS_H_ */
