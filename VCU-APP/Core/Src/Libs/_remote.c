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
remote_t RMT = {
		.d = {0},
		.t = {
				.address = {0x00, 0x00, 0x00, 0x00, 0xAB},
				.payload = {0},
		},
		.r = {
				.address = {0x00, 0x00, 0x00, 0x00, 0xCD},
				.payload = {0},
		},
		.pspi = &hspi1
};

/* Private variables
 * -----------------------------------------------------------*/
static const uint8_t COMMAND[3][8] = {
		{0x5e, 0x6c, 0xa7, 0x74, 0xfd, 0xe3, 0xdf, 0xbc},
		{0xf7, 0xda, 0x4a, 0x4f, 0x65, 0x2d, 0x6e, 0xf0},
		{0xff, 0xa6, 0xe6, 0x5a, 0x84, 0x82, 0x66, 0x4f},
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static uint32_t TimeoutDecider(vehicle_state_t state);
static void ChangeMode(RMT_MODE mode);
static uint8_t Payload(RMT_ACTION action, uint8_t *payload);
static void GenerateAesKey(uint32_t *aesSwapped);
#if REMOTE_DEBUG
static void RawDebugger(void);
static void Debugger(RMT_CMD command);
#endif

/* Public functions implementation
 * --------------------------------------------*/
uint8_t RMT_Init(void) {
	uint8_t ok;

	lock();
	nrf_param(RMT.pspi, RMT.r.payload);
	uint32_t tick = _GetTickMS();
	do {
		printf("NRF:Init\n");
		MX_SPI1_Init();
		GATE_RemoteReset();

		ok = nrf_check() != NRF_ERROR;
		_DelayMS(500);
	} while (!ok && _GetTickMS() - tick < RMT_TIMEOUT);

	if (ok) {
		nrf_configure();
		ChangeMode(RMT_MODE_NORMAL);
	}
	unlock();

	printf("NRF:Init = %s\n", ok ? "OK" : "ERROR");
	RMT.d.active = ok;
	return ok;
}

void RMT_DeInit(void) {
	lock();
	RMT_Flush();
	GATE_RemoteShutdown();
	HAL_SPI_DeInit(RMT.pspi);
	unlock();
}

uint8_t RMT_Verify(void) {
	lock();
	RMT.d.active = (nrf_check() != NRF_ERROR);
	if (!RMT.d.active) {
		RMT_DeInit();
		_DelayMS(500);
		RMT_Init();
	}
	unlock();

	return RMT.d.active;
}


void RMT_Refresh(vehicle_state_t state) {
	uint32_t timeout;

	lock();
	timeout = TimeoutDecider(state);
	RMT.d.nearby = RMT.d.heartbeat && (_GetTickMS() - RMT.d.heartbeat) < timeout;
	VCU.SetEvent(EVG_REMOTE_MISSING, !RMT.d.nearby);

//	if (state < VEHICLE_RUN || (state >= VEHICLE_RUN && !RMT.d.nearby))
//		if (!RMT_Ping())
//			RMT_Verify();
	RMT_Ping();

	if (RMT.d.pairing && (_GetTickMS() - RMT.d.pairing) > RMT_PAIRING_TIMEOUT) {
		RMT.d.pairing = 0;
		AES_ChangeKey(NULL);
	}
	unlock();
}

void RMT_Flush(void) {
	lock();
	memset(&(RMT.d), 0, sizeof(remote_data_t));
	unlock();
}

uint8_t RMT_Ping(void) {
	uint8_t ok;

	lock();
	RNG_Generate32((uint32_t *)RMT.t.payload, NRF_DATA_LENGTH / 4);
	ok = (nrf_send_packet_noack(RMT.t.payload) == NRF_OK);
	unlock();

	return ok;
}

void RMT_Pairing(void) {
	uint32_t aes[4];

	lock();
	RMT.d.pairing = _GetTickMS();
	GenerateAesKey(aes);
	AES_ChangeKey(RMT.pairing_aes);

	// Insert to payload
	memcpy(&RMT.t.payload[0], aes, NRF_DATA_LENGTH);
	memcpy(&RMT.t.payload[NRF_DATA_LENGTH], RMT.t.address, NRF_ADDR_LENGTH);

	ChangeMode(RMT_MODE_PAIRING);
	nrf_send_packet_noack(RMT.t.payload);

	// back to normal
	RMT_Init();
	unlock();
}

uint8_t RMT_GotPairedResponse(void) {
	uint8_t paired = 0;

	lock();
	if (RMT.d.pairing > 0) {
		RMT.d.pairing = 0;
		paired = 1;

		EEPROM_AesKey(EE_CMD_W, RMT.pairing_aes);
		AES_ChangeKey(NULL);
	}
	unlock();

	return paired;
}

uint8_t RMT_ValidateCommand(RMT_CMD *cmd) {
	uint8_t *payload = RMT.t.payload;
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
			RMT.d.heartbeat = _GetTickMS();
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

void RMT_OpenSeat(void) {
	lock();
	GATE_SeatToggle();
	unlock();
}

void RMT_BeepAlarm(void) {
	lock();
	GATE_HornToggle(200);
	if (VCU.ReadEvent(EVG_BIKE_MOVED))
		osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_DETECTOR_RESET);
	unlock();
}

void RMT_IrqHandler(void) { nrf_irq_handler(); }

void RMT_PacketReceived(uint8_t *data) {
#if REMOTE_DEBUG
	RawDebugger();
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

static uint32_t TimeoutDecider(vehicle_state_t state) {
	uint32_t timeout = RMT_BEAT_TIMEOUT;
	uint16_t guard = 3000;

	if (state >= VEHICLE_RUN) {
		timeout = RMT_BEAT_TIMEOUT_RUN;
		if (timeout > guard)
			timeout -= guard;
	}

	return timeout;
}

static void ChangeMode(RMT_MODE mode) {
	uint8_t payload_width;
	uint32_t vin = VIN_VALUE;

	if (mode == RMT_MODE_NORMAL) {
		// use VCU_ID as address
		memcpy(RMT.t.address, &vin, sizeof(uint32_t));
		memcpy(RMT.r.address, &vin, sizeof(uint32_t));
		payload_width = NRF_DATA_LENGTH;
	} else {
		// Set Address (pairing mode)
		memset(RMT.t.address, 0x00, sizeof(uint32_t));
		memset(RMT.r.address, 0x00, sizeof(uint32_t));
		payload_width = NRF_DATA_PAIR_LENGTH;
	}

	// Set NRF Config (pairing mode)
	nrf_change_mode(RMT.t.address, RMT.r.address, payload_width);
}

static uint8_t Payload(RMT_ACTION action, uint8_t *payload) {
	uint8_t ret = 0;

	// Process Payload
	if (action == RMT_ACTION_R)
		ret = AES_Decrypt(payload, RMT.r.payload, NRF_DATA_LENGTH);
	else
		ret = AES_Encrypt(RMT.t.payload, payload, NRF_DATA_LENGTH);

	return ret;
}

static void GenerateAesKey(uint32_t *aesSwapped) {
	const uint8_t len = sizeof(uint32_t);

	RNG_Generate32(RMT.pairing_aes, len);
	// swap byte order
	for (uint8_t i = 0; i < len; i++) {
		*aesSwapped = _ByteSwap32(RMT.pairing_aes[i]);
		aesSwapped++;
	}
}

#if REMOTE_DEBUG
static void RawDebugger(void) {
	printf("NRF:Receive = %.*s\n", NRF_DATA_LENGTH, (char *)RMT.r.payload);
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
