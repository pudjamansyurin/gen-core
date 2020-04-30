/*
 * keyless.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_keyless.h"

/* External variables ----------------------------------------------------------*/
extern nrf24l01 nrf;
extern osThreadId_t KeylessTaskHandle;

/* Private variables ----------------------------------------------------------*/
static nrf24l01_config config;
static payload_t payload = {
		.count = 8,
		.rx = { 0 }
};

/* Public functions implementation --------------------------------------------*/
void KEYLESS_Init(void) {
	LOG_StrLn("NRF:Init");

	// set configuration
	nrf_set_config(&config, payload.rx, payload.count);
	// initialization
	nrf_init(&nrf, &config);
}

void KEYLESS_IrqHandler(void) {
	nrf_irq_handler(&nrf);
}

uint8_t KEYLESS_ReadPayload(void) {
	return payload.rx[payload.count - 1];
}

void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data) {
	if (osKernelGetState() == osKernelRunning) {
		osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_RX_IT);
	}
}
