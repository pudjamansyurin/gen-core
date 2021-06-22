/*
 * .c
 *
 *  Created on: Mar 4, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_remote.h"

#include "App/_task.h"
#include "Drivers/_aes.h"
#include "Drivers/_rng.h"
#include "Libs/_eeprom.h"
#include "Nodes/VCU.h"
#include "spi.h"

/* External variables
 * --------------------------------------------*/
#if (RTOS_ENABLE)
extern osThreadId_t RemoteTaskHandle;
extern osMutexId_t RemoteRecMutexHandle;
#endif

/* Public variables
 * --------------------------------------------*/
remote_t RMT = {.d = {0},
                .t =
                    {
                        .address = {0x00, 0x00, 0x00, 0x00, 0xAB},
                        .payload = {0},
                    },
                .r =
                    {
                        .address = {0x00, 0x00, 0x00, 0x00, 0xCD},
                        .payload = {0},
                    },
                .pspi = &hspi1};

/* Private variables
 * --------------------------------------------*/
static const uint8_t COMMAND[4][8] = {
    {0x5e, 0x6c, 0xa7, 0x74, 0xfd, 0xe3, 0xdf, 0xbc},
    {0xf7, 0xda, 0x4a, 0x4f, 0x65, 0x2d, 0x6e, 0xf0},
    {0xff, 0xa6, 0xe6, 0x5a, 0x84, 0x82, 0x66, 0x4f},
    {0xab, 0xf5, 0x83, 0xc4, 0xe9, 0x27, 0x0a, 0xb2},
};

/* Private functions declaration
 * --------------------------------------------*/
static void lock(void);
static void unlock(void);
static void ChangeMode(RMT_MODE mode);
static uint8_t Payload(RMT_ACTION action, uint8_t *payload);
static uint8_t Transmit(const uint8_t *data);
#if REMOTE_DEBUG
static void RawDebugger(void);
static void Debugger(RMT_CMD command);
#endif

/* Public functions implementation
 * --------------------------------------------*/
uint8_t RMT_Init(void) {
  nrf_param(RMT.pspi, RMT.r.payload);
  return RMT_ReInit();
}

uint8_t RMT_ReInit(void) {
  uint8_t ok;
  uint32_t tick;

  lock();
  printf("NRF:Init\n");

  tick = _GetTickMS();
  do {
    MX_SPI1_Init();
    GATE_RemoteReset();

    ok = RMT_Probe();
    if (!ok) _DelayMS(500);
  } while (!ok && _GetTickMS() - tick < RMT_TIMEOUT_MS);

  if (ok) {
    nrf_configure();
    ChangeMode(RMT_MODE_NORMAL);
  }
  unlock();

  printf("NRF:%s\n", ok ? "OK" : "Error");
  return ok;
}

void RMT_DeInit(void) {
  lock();
  RMT_Flush();
  GATE_RemoteShutdown();
  HAL_SPI_DeInit(RMT.pspi);
  unlock();
}

uint8_t RMT_Probe(void) {
  uint8_t ok;

  lock();
  ok = (nrf_check() != NRF_ERROR);
  unlock();

  return ok;
}

void RMT_Flush(void) {
  lock();
  memset(&(RMT.d), 0, sizeof(remote_data_t));
  unlock();
}

void RMT_Refresh(vehicle_state_t state) {
  remote_tick_t *tick = &(RMT.d.tick);
  uint32_t timeout;

  lock();
  timeout = state == VEHICLE_RUN ? RMT_BEAT_RUN_MS : RMT_BEAT_MS;
  RMT.d.nearby = _TickIn(tick->heartbeat, timeout);
  EVT_SetVal(EVG_REMOTE_MISSING, !RMT.d.nearby);

  RMT.d.active = _TickIn(tick->ping, RMT_TIMEOUT_MS) || RMT.d.nearby;
  if (!RMT.d.active) {
    RMT_DeInit();
    _DelayMS(500);
    RMT_Init();
  }

  if (_TickOut(tick->pairing, RMT_PAIRING_MS)) {
    tick->pairing = 0;
    AES_ChangeKey(NULL);
  }

  unlock();
}

uint8_t RMT_Ping(vehicle_state_t state) {
  uint8_t ok;

  //	if (state == VEHICLE_RUN && RMT.d.nearby)
  //		if ((_GetTickMS() - RMT.d.tick.heartbeat) < (RMT_BEAT_RUN_MS -
  // RMT_TIMEOUT_MS)) 			return 1;

  lock();
  RNG_Generate32((uint32_t *)RMT.t.payload, NRF_DATA_LENGTH / 4);
  ok = Transmit(RMT.t.payload);
  unlock();

  return ok;
}

uint8_t RMT_Pairing(void) {
  uint32_t aes[4];
  uint8_t ok;

  lock();
  RMT.d.tick.pairing = _GetTickMS();
  RNG_Generate32(RMT.d.pairing_aes, 4);
  AES_ChangeKey(RMT.d.pairing_aes);

  // Insert to payload
  for (uint8_t i = 0; i < 4; i++) aes[i] = _ByteSwap32(RMT.d.pairing_aes[i]);
  memcpy(&RMT.t.payload[0], aes, NRF_DATA_LENGTH);
  memcpy(&RMT.t.payload[NRF_DATA_LENGTH], RMT.t.address, NRF_ADDR_LENGTH);

  ChangeMode(RMT_MODE_PAIRING);
  ok = Transmit(RMT.t.payload);
  ChangeMode(RMT_MODE_NORMAL);
  unlock();

  return ok;
}

uint8_t RMT_GotPairedResponse(void) {
  uint8_t paired = 0;

  lock();
  if (RMT.d.tick.pairing > 0) {
    RMT.d.tick.pairing = 0;
    paired = 1;

    EE_AesKey(EE_CMD_W, RMT.d.pairing_aes);
    AES_ChangeKey(NULL);
  }
  unlock();

  return paired;
}

uint8_t RMT_ValidateCommand(RMT_CMD *cmd) {
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
      valid = (memcmp(&plain[rng], &RMT.t.payload[rng], rng) == 0);
    }
  }

  if (valid) RMT.d.tick.heartbeat = _GetTickMS();
  unlock();

#if REMOTE_DEBUG
  if (valid) Debugger(*cmd);
#endif

  return valid;
}

void RMT_IrqHandler(void) {
  if (nrf_irq_handler()) {
    osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_RX_IT);
#if REMOTE_DEBUG
    RawDebugger();
    Debugger();
#endif
  }
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

static uint8_t Transmit(const uint8_t *data) {
  uint32_t tick;

  nrf_send_packet_noack(data);
  tick = _GetTickMS();
  while (nrf_tx_busy() && _TickIn(tick, 3)) {
  };

  return !nrf_tx_busy();
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
