/*
 * keyless.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_keyless.h"
#include "Nodes/VCU.h"
#include "Nodes/HMI1.h"
#include "Drivers/_aes.h"

/* External variables ----------------------------------------------------------*/
extern osThreadId_t KeylessTaskHandle;
extern osMutexId_t KlessRecMutexHandle;
extern RNG_HandleTypeDef hrng;
extern nrf24l01 nrf;
extern vcu_t VCU;
extern hmi1_t HMI1;

/* Public variables -----------------------------------------------------------*/
kless_t KLESS = {
        .address = {
                .tx = { 0, 0, 0, 0, 1 },
                .rx = { 0, 0, 0, 0, 2 },
        },
        .payload = {
                .tx = { 0 },
                .rx = { 0 },
        }
};

/* Private variables ----------------------------------------------------------*/
static nrf24l01_config config;
static const kless_address_t KLESS_AddressDefault = {
        .tx = { 0xAB, 0x00, 0x00, 0x00, 0x00 },
        .rx = { 0xCD, 0x00, 0x00, 0x00, 0x00 },
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
void KLESS_Init(void) {
	// set configuration
	config.tx_address = KLESS_AddressDefault.tx;
	config.rx_address = KLESS_AddressDefault.rx;
	config.payload_length = NRF_DATA_LENGTH;
	config.addr_width = NRF_ADDRRESS_WIDTH;
	config.rx_buffer = (uint8_t*) KLESS.payload.rx;
	nrf_set_config(&config);

	// initialization
	nrf_init(&nrf, &config);
}

void KLESS_Debugger(void) {
	lock();
	LOG_Str("NRF:Receive = ");
	LOG_BufHex((char*) KLESS.payload.rx, NRF_DATA_LENGTH);
	LOG_Enter();
	unlock();
}

uint8_t KLESS_ValidateCommand(KLESS_CMD *cmd) {
	uint8_t pos, valid, payload[NRF_DATA_LENGTH] = { 0 };

	lock();
	// Read Payload
	valid = KLESS_UsePayload(KLESS_R, payload);

	// Get Payload position
	if (valid) {
		pos = (payload[0] & 0xF0) >> 4;
		*cmd = payload[pos] & 0x0F;

		// Check Payload Command
		if (*cmd > KLESS_CMD_MAX) {
			valid = 0;
		}
	}
	unlock();

	return valid;
}

uint8_t KLESS_UsePayload(KLESS_MODE mode, uint8_t *payload) {
	uint8_t ret = 0;

	lock();
	// Process Payload
	if (mode == KLESS_R) {
		// Decrypt
		if (AES_Decrypt(payload, KLESS.payload.rx, NRF_DATA_LENGTH)) {
			ret = 1;
		}
	} else {
		// Encrypt
		if (AES_Encrypt(KLESS.payload.tx, payload, NRF_DATA_LENGTH)) {
			ret = 1;
		}
	}
	unlock();

	return ret;
}

uint8_t KLESS_SendDummy(void) {
	uint8_t payload[NRF_DATA_LENGTH] = { 0 };
	NRF_RESULT p;

	payload[0] = 1;
	p = nrf_send_packet(&nrf, payload);

	// debug
	LOG_Str("NRF:Send = ");
	LOG_Str(p ? "OK" : "ERROR");
	LOG_Enter();

	return (p == NRF_OK);
}

void KLESS_Refresh(void) {
	if ((osKernelGetTickCount() - VCU.d.tick.keyless)
	        < pdMS_TO_TICKS(KEYLESS_TIMEOUT)) {
		HMI1.d.status.keyless = 1;
	} else {
		HMI1.d.status.keyless = 0;
	}
}

void KLESS_IrqHandler(void) {
	nrf_irq_handler(&nrf);
	osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_RX_IT);
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
	osMutexAcquire(KlessRecMutexHandle, osWaitForever);
}

static void unlock(void) {
	osMutexRelease(KlessRecMutexHandle);
}

