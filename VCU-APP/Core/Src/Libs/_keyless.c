/*
 * keyless.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_keyless.h"
#include "Libs/_eeprom.h"
#include "Drivers/_aes.h"
#include "Nodes/VCU.h"
#include "Nodes/HMI1.h"

/* External variables ----------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1;
extern osThreadId_t KeylessTaskHandle;
extern osMutexId_t KlessRecMutexHandle;
extern RNG_HandleTypeDef hrng;

extern vcu_t VCU;
extern hmi1_t HMI1;
extern uint32_t AesKey[4];

/* Public variables -----------------------------------------------------------*/
nrf24l01 NRF;
kless_t RF = {
		.config = {
				.addr_width = NRF_ADDR_LENGTH - 2,
		},
		.tx = {
				.address = { 0x00, 0x00, 0x00, 0x00, 0xAB },
				.payload = { 0 },
		},
		.rx = {
				.address = { 0x00, 0x00, 0x00, 0x00, 0xCD },
				.payload = { 0 },
		},
};

/* Private variables -----------------------------------------------------------*/
static const uint8_t commands[3][8] = {
		{ 0x5e, 0x6c, 0xa7, 0x74, 0xfd, 0xe3, 0xdf, 0xbc },
		{ 0xf7, 0xda, 0x4a, 0x4f, 0x65, 0x2d, 0x6e, 0xf0 },
		{ 0xff, 0xa6, 0xe6, 0x5a, 0x84, 0x82, 0x66, 0x4f },
};

/* Private functions declaration ---------------------------------------------*/
static void RF_SetPairingPayload(uint32_t *payload);
static void RF_SetAesPayload(uint32_t *payload, uint32_t *aes);
static void RF_ChangeMode(RF_MODE mode);
static uint8_t RF_Payload(RF_ACTION action, uint8_t *payload);
static void GenRandomNumber32(uint32_t *payload, uint8_t size);
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
void RF_Init(void) {
	// set more config
	nrf_set_config(&NRF, &(RF.config));
	// initialization
	nrf_init(&NRF);
	// apply config
	RF_ChangeMode(RF_MODE_NORMAL);
	nrf_configure(&NRF);
}

uint8_t RF_SendPing(uint8_t retry) {
	uint8_t *payload = RF.tx.payload;
	RF_CMD cmd = RF_CMD_PING;
	NRF_RESULT p;

	GenRandomNumber32((uint32_t *) payload, NRF_DATA_LENGTH/4);

	RF_Payload(RF_ACTION_W, (uint8_t *) commands[cmd]);

	while(retry--) {
		p = nrf_send_packet_noack(&NRF, payload);
	}

	return (p == NRF_OK);
}

uint8_t RF_ValidateCommand(RF_CMD *cmd) {
	uint8_t *payload = RF.tx.payload;
	uint8_t valid = 0, payload_dec[NRF_DATA_LENGTH];
	const uint8_t rng = NRF_DATA_LENGTH/2;

	lock();
	// Read Payload
	if (RF_Payload(RF_ACTION_R, payload_dec)) {
		// Check Payload Command
		for (uint8_t i = 0; i < 3; i++) {
			// check command
			if (memcmp(payload_dec, &commands[i], 8) == 0) {
				*cmd = i;
				valid = 1;
				break;
			}
		}
	}
	// Check is valid ping response
	if (valid) {
		if (*cmd == RF_CMD_PING) {
			// compare upper RNG
			if (memcmp(&payload_dec[rng], &payload[rng], rng) == 0) {
				valid = 1;
			}
		}
	}
	unlock();

	return valid;
}

uint8_t RF_Pairing(void) {
	uint8_t *payload = RF.tx.payload;
	NRF_RESULT p = NRF_ERROR;

	RF_SetPairingPayload((uint32_t *) payload);

	RF_ChangeMode(RF_MODE_PAIRING);

	p = nrf_send_packet_noack(&NRF, payload);

	RF_ChangeMode(RF_MODE_NORMAL);

	return (p == NRF_OK);
}

void RF_GenerateAesKey(uint32_t *aes) {
	GenRandomNumber32(aes, NRF_DATA_LENGTH/4);
	EEPROM_AesKey(EE_CMD_W, aes);
}

void RF_Debugger(void) {
	lock();
	LOG_Str("NRF:Receive = ");
	LOG_BufHex((char*) RF.rx.payload, NRF_DATA_LENGTH);
	LOG_Enter();
	unlock();
}

void RF_Refresh(void) {
	uint8_t timeout;

	timeout = ((_GetTickMS() - VCU.d.tick.keyless) < KEYLESS_TIMEOUT);
	HMI1.d.status.keyless = timeout;
}

void RF_IrqHandler(void) {
	nrf_irq_handler(&NRF);
}

void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data) {
	// used in favor of nrf_receive_packet
	dev->rx_busy = 0;

	osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_RX_IT);
}

/* Private functions implementation --------------------------------------------*/
static void RF_SetPairingPayload(uint32_t *payload) {
	uint32_t aes[4];

	// Generate new AES Key
	RF_GenerateAesKey(aes);
	// Inject AES to payload
	RF_SetAesPayload(payload, aes);
	// Insert VCU_ID to payload
	memcpy(&payload[NRF_DATA_LENGTH], &(VCU.d.unit_id), NRF_ADDR_LENGTH);
}

static void RF_SetAesPayload(uint32_t *payload, uint32_t *aes){
	uint32_t swapped;

	// insert to payload & swap byte order
	for (uint8_t i = 0; i < (NRF_DATA_LENGTH/4); i++) {
		swapped = _ByteSwap32(*aes++);
		memcpy(payload++, &swapped, 4);
	}
}

static void RF_ChangeMode(RF_MODE mode) {
	if(mode == RF_MODE_NORMAL) {
		// use VCU_ID as address
		memcpy(RF.tx.address, &(VCU.d.unit_id), 4);
		memcpy(RF.rx.address, &(VCU.d.unit_id), 4);
		RF.config.payload_length = NRF_DATA_LENGTH;
	} else {
		// Set Address (pairing mode)
		memset(RF.tx.address, 0x00, 4);
		memset(RF.rx.address, 0x00, 4);
		RF.config.payload_length = NRF_DATA_PAIR_LENGTH;
	}

	// set configuration
	RF.config.tx_address = &(RF.tx.address[0]);
	RF.config.rx_address = &(RF.rx.address[0]);
	RF.config.rx_buffer = &(RF.rx.payload[0]);

	// Set NRF Config (pairing mode)
	nrf_change_mode(&NRF, &(RF.config));
}

static uint8_t RF_Payload(RF_ACTION action, uint8_t *payload) {
	uint8_t ret = 0;

	// Process Payload
	lock();
	if (action == RF_ACTION_R) {
		ret = AES_Decrypt(payload, RF.rx.payload, NRF_DATA_LENGTH);
	} else {
		ret = AES_Encrypt(RF.tx.payload, payload, NRF_DATA_LENGTH);
	}
	unlock();

	return ret;
}

static void GenRandomNumber32(uint32_t *payload, uint8_t size) {
	for (uint8_t i = 0; i < size; i++) {
		HAL_RNG_GenerateRandomNumber(&hrng, payload++);
	}
}

static void lock(void) {
	osMutexAcquire(KlessRecMutexHandle, osWaitForever);
}

static void unlock(void) {
	osMutexRelease(KlessRecMutexHandle);
}

