/*
 * .c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_remote.h"
#include "Drivers/_aes.h"
#include "Drivers/_rng.h"
#include "Libs/_eeprom.h"
#include "Nodes/HMI1.h"
#include "Nodes/VCU.h"
#include "spi.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osThreadId_t RemoteTaskHandle;
extern osMutexId_t RemoteRecMutexHandle;
#endif

/* Public variables
 * -----------------------------------------------------------*/
remote_data_t RMT = { 0 };

/* Private variables
 * -----------------------------------------------------------*/
static remote_t rmt = {
		.tx = {
				.address = {0x00, 0x00, 0x00, 0x00, 0xAB},
				.payload = {0},
		},
		.rx = {
				.address = {0x00, 0x00, 0x00, 0x00, 0xCD},
				.payload = {0},
		},
		.pspi = &hspi1
};
static const uint8_t COMMAND[3][8] = {
		{0x5e, 0x6c, 0xa7, 0x74, 0xfd, 0xe3, 0xdf, 0xbc},
		{0xf7, 0xda, 0x4a, 0x4f, 0x65, 0x2d, 0x6e, 0xf0},
		{0xff, 0xa6, 0xe6, 0x5a, 0x84, 0x82, 0x66, 0x4f},
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static void ChangeMode(RMT_MODE mode);
static uint8_t Payload(RMT_ACTION action, uint8_t *payload);
static void RMT_GenerateAesKey(uint32_t *aesSwapped);
#if REMOTE_DEBUG
static void RawDebugger(void);
static void Debugger(RMT_CMD command);
#endif

/* Public functions implementation
 * --------------------------------------------*/
void RMT_Init(void) {
	nrf_param(rmt.pspi, rmt.rx.payload);
	RMT_ReInit();
}

void RMT_DeInit(void) {
	GATE_RemoteShutdown();
	HAL_SPI_DeInit(rmt.pspi);
	_DelayMS(1000);
}

void RMT_ReInit(void) {
	MX_SPI1_Init();
	do {
		printf("NRF:Init\n");

		//    HAL_SPI_Init(hspi);
		GATE_RemoteReset();
		_DelayMS(1000);
	} while (nrf_check() == NRF_ERROR);

	nrf_configure();
	ChangeMode(RMT_MODE_NORMAL);
}

void RMT_Refresh(void) {
	if (RMT_NeedPing())
		RMT_Ping();

	if (RMT.pairing > 0) {
		if (_GetTickMS() - RMT.pairing > REMOTE_PAIRING_TIMEOUT) {
			RMT.pairing = 0;
			AES_ChangeKey(NULL);
		}
	}
}

uint8_t RMT_NeedPing(void) {
	vehicle_state_t *state = &(VCU.d.state);
	uint32_t timeout = (*state < VEHICLE_RUN) ? REMOTE_TIMEOUT : REMOTE_TIMEOUT_RUN;

	RMT.active = RMT.heartbeat && (_GetTickMS() - RMT.heartbeat) < timeout;
	VCU.SetEvent(EVG_REMOTE_MISSING, RMT.active);
	VCU.d.mod.remote = RMT.active;

	if (*state >= VEHICLE_RUN)
		return (_GetTickMS() - RMT.heartbeat) > (timeout - 3000);
	return 1;
}

uint8_t RMT_Ping(void) {
	RNG_Generate32((uint32_t *)rmt.tx.payload, NRF_DATA_LENGTH / 4);
	return (nrf_send_packet_noack(rmt.tx.payload) == NRF_OK);
}

void RMT_Pairing(void) {
	uint32_t aes[4];

	RMT.pairing = _GetTickMS();
	RMT_GenerateAesKey(aes);
	AES_ChangeKey(rmt.pairingAes);

	// Insert to payload
	memcpy(&rmt.tx.payload[0], aes, NRF_DATA_LENGTH);
	memcpy(&rmt.tx.payload[NRF_DATA_LENGTH], rmt.tx.address, NRF_ADDR_LENGTH);

	ChangeMode(RMT_MODE_PAIRING);
	nrf_send_packet_noack(rmt.tx.payload);

	// back to normal
	RMT_ReInit();
}

uint8_t RMT_GotPairedResponse(void) {
	uint8_t paired = 0;

	if (RMT.pairing > 0) {
		RMT.pairing = 0;
		paired = 1;

		EEPROM_AesKey(EE_CMD_W, rmt.pairingAes);
		AES_ChangeKey(NULL);
	}

	return paired;
}

uint8_t RMT_ValidateCommand(RMT_CMD *cmd) {
	uint8_t *payload = rmt.tx.payload;
	uint8_t valid = 0, plain[NRF_DATA_LENGTH];
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
			RMT.heartbeat = _GetTickMS();
			valid = (memcmp(&plain[rng], &payload[rng], rng) == 0);
		}
	}
	unlock();

#if REMOTE_DEBUG
	if (valid)
		Debugger(*cmd);
#endif

	return valid;
}

void RMT_IrqHandler(void) { nrf_irq_handler(); }

void RMT_PacketReceived(uint8_t *data) {
#if REMOTE_DEBUG
	RawDebugger
	Debugger();
#endif
	osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_RX_IT);
}

/* Private functions implementation
 * --------------------------------------------*/
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

static void ChangeMode(RMT_MODE mode) {
	uint8_t payload_width;
	uint32_t vin = VIN_VALUE;

	if (mode == RMT_MODE_NORMAL) {
		// use VCU_ID as address
		memcpy(rmt.tx.address, &vin, sizeof(uint32_t));
		memcpy(rmt.rx.address, &vin, sizeof(uint32_t));
		payload_width = NRF_DATA_LENGTH;
	} else {
		// Set Address (pairing mode)
		memset(rmt.tx.address, 0x00, sizeof(uint32_t));
		memset(rmt.rx.address, 0x00, sizeof(uint32_t));
		payload_width = NRF_DATA_PAIR_LENGTH;
	}

	// Set NRF Config (pairing mode)
	nrf_change_mode(rmt.tx.address, rmt.rx.address, payload_width);
}

static uint8_t Payload(RMT_ACTION action, uint8_t *payload) {
	uint8_t ret = 0;

	// Process Payload
	lock();
	if (action == RMT_ACTION_R)
		ret = AES_Decrypt(payload, rmt.rx.payload, NRF_DATA_LENGTH);
	else
		ret = AES_Encrypt(rmt.tx.payload, payload, NRF_DATA_LENGTH);

	unlock();

	return ret;
}

static void RMT_GenerateAesKey(uint32_t *aesSwapped) {
	const uint8_t len = sizeof(uint32_t);

	RNG_Generate32(rmt.pairingAes, len);

	// swap byte order
	for (uint8_t i = 0; i < len; i++) {
		*aesSwapped = _ByteSwap32(rmt.pairingAes[i]);
		aesSwapped++;
	}
}

#if REMOTE_DEBUG
static void RawDebugger(void) {
	printf("NRF:Receive = %.*s\n", NRF_DATA_LENGTH, (char *)rmt.rx.payload);
}

static void Debugger(RMT_CMD command) {
	if (command == RMT_CMD_PING)
		printf("NRF:Command = PING\n");
	else if (command == RMT_CMD_SEAT)
		printf("NRF:Command = SEAT\n");
	else if (command == RMT_CMD_ALARM)
		printf("NRF:Command = ALARM\n");
}
#endif
