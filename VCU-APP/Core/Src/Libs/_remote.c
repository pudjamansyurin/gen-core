/*
 * .c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "Drivers/_rng.h"
#include "Drivers/_aes.h"
#include "Libs/_remote.h"
#include "Libs/_eeprom.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osThreadId_t RemoteTaskHandle;
extern osMutexId_t RemoteRecMutexHandle;
#endif

/* Private variables -----------------------------------------------------------*/
static remote_t RMT = {
		.tx = {
				.address = { 0x00, 0x00, 0x00, 0x00, 0xAB },
				.payload = { 0 },
		},
		.rx = {
				.address = { 0x00, 0x00, 0x00, 0x00, 0xCD },
				.payload = { 0 },
		},
		.tick = {
				.heartbeat = 0,
				.pairing = 0,
		},
		.pspi = &hspi1
};
static const uint8_t COMMAND[3][8] = {
		{ 0x5e, 0x6c, 0xa7, 0x74, 0xfd, 0xe3, 0xdf, 0xbc },
		{ 0xf7, 0xda, 0x4a, 0x4f, 0x65, 0x2d, 0x6e, 0xf0 },
		{ 0xff, 0xa6, 0xe6, 0x5a, 0x84, 0x82, 0x66, 0x4f },
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static void ChangeMode(RMT_MODE mode, uint32_t *unit_id);
static uint8_t Payload(RMT_ACTION action, uint8_t *payload);
static void RMT_GenerateAesKey(uint32_t *aesSwapped);
static void RawDebugger(void);
static void Debugger(RMT_CMD command);

/* Public functions implementation --------------------------------------------*/
void RMT_Init(uint32_t *unit_id) {
	nrf_param(RMT.pspi, RMT.rx.payload);
	RMT_ReInit(unit_id);
}

void RMT_DeInit(void) {
	GATE_RemoteShutdown();
	HAL_SPI_DeInit(RMT.pspi);
	_DelayMS(1000);
}

void RMT_ReInit(uint32_t *unit_id) {
	MX_SPI1_Init();
	do {
		printf("NRF:Init\n");

		//    HAL_SPI_Init(hspi);
		GATE_RemoteReset();
		_DelayMS(1000);
	} while (nrf_check() == NRF_ERROR);

	nrf_configure();
	ChangeMode(RMT_MODE_NORMAL, unit_id);
}


uint8_t RMT_NeedPing(vehicle_state_t *state, uint8_t *unremote) {
	uint32_t timeout = (*state < VEHICLE_RUN) ? REMOTE_TIMEOUT : REMOTE_TIMEOUT_RUN;

	if (RMT.tick.heartbeat)
		*unremote =  (_GetTickMS() - RMT.tick.heartbeat) > timeout ;

	if (*state >= VEHICLE_RUN)
		return (_GetTickMS() - RMT.tick.heartbeat) > (timeout-3000);
	return 1;
}

uint8_t RMT_Ping(void) {
	RNG_Generate32((uint32_t*) RMT.tx.payload, NRF_DATA_LENGTH / 4);
	return (nrf_send_packet_noack(RMT.tx.payload) == NRF_OK);
}

void RMT_Pairing(uint32_t *unit_id) {
	uint32_t aes[4];

	RMT.tick.pairing = _GetTickMS();
	RMT_GenerateAesKey(aes);
	AES_ChangeKey(RMT.pairingAes);

	// Insert to payload
	memcpy(&RMT.tx.payload[0], aes, NRF_DATA_LENGTH);
	memcpy(&RMT.tx.payload[NRF_DATA_LENGTH ], RMT.tx.address, NRF_ADDR_LENGTH);

	ChangeMode(RMT_MODE_PAIRING, NULL);
	nrf_send_packet_noack(RMT.tx.payload);

	// back to normal
	RMT_ReInit(unit_id);
}

uint8_t RMT_GotPairedResponse(void) {
	uint8_t paired = 0;

	if (RMT.tick.pairing > 0) {
		RMT.tick.pairing = 0;
		paired = 1;

		EEPROM_AesKey(EE_CMD_W, RMT.pairingAes);
		AES_ChangeKey(NULL);
	}

	return paired;
}

void RMT_RefreshPairing(void) {
	if (RMT.tick.pairing > 0) {
		if (_GetTickMS() - RMT.tick.pairing > REMOTE_PAIRING_TIMEOUT) {
			RMT.tick.pairing = 0;
			AES_ChangeKey(NULL);
		}
	}
}

uint8_t RMT_ValidateCommand(RMT_CMD *cmd) {
	uint8_t *payload = RMT.tx.payload;
	uint8_t valid = 0, plain[NRF_DATA_LENGTH ];
	uint8_t rng = NRF_DATA_LENGTH / 2;

	lock();
	// Read payload
	if (Payload(RMT_ACTION_R, plain)) {
		for (uint8_t i = 0; i < (sizeof(COMMAND) / sizeof(COMMAND[0])); i++) {
			if (memcmp(plain, &COMMAND[i], 8) == 0) {
				*cmd = i;
				valid = 1;
				break;
			}
		}
	}
	// Is ping response
	if (valid) {
		if (*cmd == RMT_CMD_PING || *cmd == RMT_CMD_SEAT) {
			RMT.tick.heartbeat = _GetTickMS();
			valid = (memcmp(&plain[rng], &payload[rng], rng) == 0);
		}
	}
	unlock();

	//	if (valid)
	//		Debugger(*cmd);

	return valid;
}

void RMT_IrqHandler(void) {
	nrf_irq_handler();
}

void RMT_PacketReceived(uint8_t *data) {
	// Debugger();
	osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_RX_IT);
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
  #if (RTOS_ENABLE)
	osMutexAcquire(RemoteRecMutexHandle, osWaitForever);
	#endif
}

static void unlock(void) {
  #if (RTOS_ENABLE)
	osMutexRelease(RemoteRecMutexHandle);
	#endif
}

static void ChangeMode(RMT_MODE mode, uint32_t *unit_id) {
	uint8_t payload_width;

	if (mode == RMT_MODE_NORMAL) {
		// use VCU_ID as address
		memcpy(RMT.tx.address, unit_id, sizeof(uint32_t));
		memcpy(RMT.rx.address, unit_id, sizeof(uint32_t));
		payload_width = NRF_DATA_LENGTH;
	} else {
		// Set Address (pairing mode)
		memset(RMT.tx.address, 0x00, sizeof(uint32_t));
		memset(RMT.rx.address, 0x00, sizeof(uint32_t));
		payload_width = NRF_DATA_PAIR_LENGTH;
	}

	// Set NRF Config (pairing mode)
	nrf_change_mode(RMT.tx.address, RMT.rx.address, payload_width);
}

static uint8_t Payload(RMT_ACTION action, uint8_t *payload) {
	uint8_t ret = 0;

	// Process Payload
	lock();
	if (action == RMT_ACTION_R)
		ret = AES_Decrypt(payload, RMT.rx.payload, NRF_DATA_LENGTH);
	else
		ret = AES_Encrypt(RMT.tx.payload, payload, NRF_DATA_LENGTH);

	unlock();

	return ret;
}


static void RMT_GenerateAesKey(uint32_t *aesSwapped) {
	const uint8_t len = sizeof(uint32_t);

	RNG_Generate32(RMT.pairingAes, len);

	// swap byte order
	for (uint8_t i = 0; i < len; i++) {
		*aesSwapped = _ByteSwap32(RMT.pairingAes[i]);
		aesSwapped++;
	}
}

static void RawDebugger(void) {
	printf("NRF:Receive = %.*s\n", NRF_DATA_LENGTH, (char*) RMT.rx.payload);
}

static void Debugger(RMT_CMD command) {
	if (command == RMT_CMD_PING)
		printf("NRF:Command = PING\n");
	else if (command == RMT_CMD_SEAT)
		printf("NRF:Command = SEAT\n");
	else if (command == RMT_CMD_ALARM)
		printf("NRF:Command = ALARM\n");
}
