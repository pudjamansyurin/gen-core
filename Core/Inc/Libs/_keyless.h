/*
 * _keyless.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef KEYLESS_H_
#define KEYLESS_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_nrf24l01.h"

/* Exported define -----------------------------------------------------------*/
#define NRF_DATA_LENGTH														8

/* Exported enum -------------------------------------------------------------*/
typedef enum {
	MSG_KEYLESS_BROADCAST = 1,
	MSG_KEYLESS_FINDER = 2,
	MSG_KEYLESS_SEAT = 4,
	MSG_KEYLESS_MAX = 4,
} MSG_KEYLESS;

/* Exported struct -----------------------------------------------------------*/
typedef struct {
	uint8_t rx[NRF_DATA_LENGTH];
} payload_t;

/* Public functions prototype ------------------------------------------------*/
void KEYLESS_Init(void);
void KEYLESS_Debugger(void);
void KEYLESS_Refresh(void);
uint8_t KEYLESS_ReadPayload(void);
void KEYLESS_IrqHandler(void);
void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data);

#endif /* KEYLESS_H_ */
