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
	LOG_StrLn("NRF:Init");

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
	LOG_Str("NRF:Receive [");
	LOG_Hex8(KLESS.payload.rx[NRF_DATA_LENGTH]);
	LOG_Str("] = ");
	LOG_BufHex((char*) KLESS.payload.rx, NRF_DATA_LENGTH - 1);
	LOG_Enter();
	unlock();
}

uint8_t KLESS_ValidateCommand(KLESS_CMD *cmd) {
	uint8_t valid = 0;

	lock();
	// Check Command
	if (KLESS_CheckCommand(cmd)) {
		if (*cmd != KLESS_CMD_ALARM) {
			// Check AES
			if (KLESS_CheckAES()) {
				valid = 1;
			}
		} else {
			valid = 1;
		}
	}
	unlock();

	return valid;
}

uint8_t KLESS_CheckCommand(KLESS_CMD *cmd) {
	uint8_t p, valid = 0, pBuf[NRF_DATA_LENGTH - 1];
	KLESS_CODE code;

	lock();
	// Read Payload
	p = KLESS_UsePayload(KLESS_R, &code, pBuf, 1);

	// Check Code
	if (p) {
		p = (code == KLESS_CODE_COMMAND);
	}

	// Validate-1, Check Payload:Command
	if (p) {
		*cmd = pBuf[0];

		// Check Payload:Command
		if (*cmd == KLESS_CMD_PING ||
				*cmd == KLESS_CMD_ALARM ||
				*cmd == KLESS_CMD_SEAT) {
			valid = 1;
		}
	}
	unlock();

	return valid;
}

uint8_t KLESS_CheckAES(void) {
	uint8_t p, valid = 0, pBuf[NRF_DATA_LENGTH - 1];
	KLESS_CODE code;

	lock();
	// Generate Payload
	p = (HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*) pBuf) == HAL_OK);

	// Write Payload
	if (p) {
		code = KLESS_CODE_RANDOM;
		p = (KLESS_UsePayload(KLESS_W, &code, pBuf, sizeof(uint32_t)));
	}

	// Send Payload
	if (p) {
		p = (nrf_send_packet_noack(&nrf, KLESS.payload.tx) == NRF_OK);
	}

	// Receive Payload
	if (p) {
		p = (nrf_receive_packet(&nrf, KLESS.payload.rx, 500) == NRF_OK);
	}

	// Read Payload
	if (p) {
		p = KLESS_UsePayload(KLESS_R, &code, pBuf, NRF_DATA_LENGTH - 1);
	}

	// Check Code
	if (p) {
		p = (code == KLESS_CODE_RANDOM);
	}

	// Validate, Check Payload
	if (p) {
		if (memcmp(pBuf, KLESS.payload.tx, NRF_DATA_LENGTH - 1) == 0) {
			valid = 1;
		}
	}
	unlock();

	return valid;
}

void KLESS_UseCode(KLESS_MODE mode, KLESS_CODE *code) {
	lock();
	if (mode == KLESS_R) {
		*code = KLESS.payload.rx[NRF_DATA_LENGTH];
	} else {
		KLESS.payload.tx[NRF_DATA_LENGTH] = *code;
	}
	unlock();
}

uint8_t KLESS_UsePayload(KLESS_MODE mode, KLESS_CODE *code, uint8_t *payload, uint8_t size) {
	uint8_t ret = 0;
	uint8_t pBuf[NRF_DATA_LENGTH - 1];

	lock();
	// Process Payload
	if (mode == KLESS_R) {
		// Decrypt
		if (AES_Decrypt(KLESS.payload.rx, pBuf, NRF_DATA_LENGTH - 1)) {
			// Read
			memcpy(payload, pBuf, size);
			ret = 1;
		}
	} else {
		// Write
		memcpy(pBuf, payload, size);
		// Encrypt
		if (AES_Encrypt(pBuf, KLESS.payload.tx, NRF_DATA_LENGTH - 1)) {
			ret = 1;
		}
	}

	// Process Code
	if (ret) {
		KLESS_UseCode(mode, code);
	}
	unlock();

	return ret;
}

void KLESS_Refresh(void) {
	if ((osKernelGetTickCount() - VCU.d.tick.keyless) < pdMS_TO_TICKS(5000)) {
		HMI1.d.status.keyless = 1;
	} else {
		HMI1.d.status.keyless = 0;
	}
}

void KLESS_IrqHandler(void) {
	nrf_irq_handler(&nrf);
}

void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data) {
	osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_RX_IT);
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
	osMutexAcquire(KlessRecMutexHandle, osWaitForever);
}

static void unlock(void) {
	osMutexRelease(KlessRecMutexHandle);
}

