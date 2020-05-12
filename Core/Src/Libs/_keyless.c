/*
 * keyless.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_keyless.h"
#include "VCU.h"
#include "HMI1.h"

/* External variables ----------------------------------------------------------*/
extern nrf24l01 nrf;
extern osThreadId_t KeylessTaskHandle;
extern vcu_t VCU;
extern hmi1_t HMI1;

/* Private variables ----------------------------------------------------------*/
static nrf24l01_config config;
static payload_t payload = { 0 };

/* Public functions implementation --------------------------------------------*/
void KEYLESS_Init(void) {
	LOG_StrLn("NRF:Init");

	// set configuration
	nrf_set_config(&config, payload.rx, NRF_DATA_LENGTH);
	// initialization
	nrf_init(&nrf, &config);
}

uint8_t KEYLESS_ReadPayload(void) {
	return payload.rx[NRF_DATA_LENGTH - 1];
}

void KEYLESS_Debugger(void) {
	uint8_t msg = KEYLESS_ReadPayload();

	LOG_Str("NRF received packet, msg = ");
	LOG_Hex8(msg);
	LOG_Enter();
}

void KEYLESS_Refresh(void) {
	if ((osKernelGetTickCount() - VCU.d.tick.keyless) < pdMS_TO_TICKS(5000)) {
		HMI1.d.status.keyless = 1;
	} else {
		HMI1.d.status.keyless = 0;
	}
}

void KEYLESS_IrqHandler(void) {
	nrf_irq_handler(&nrf);
}

void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data) {
	osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_RX_IT);
}
