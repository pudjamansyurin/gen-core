/*
 * .c
 *
 *  Created on: Mar 4, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/remote.h"

#include "App/iap.h"
#include "App/task.h"
#include "Drivers/aes.h"
#include "Drivers/rng.h"
#include "Libs/eeprom.h"
#include "Nodes/VCU.h"
#include "spi.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osThreadId_t RemoteTaskHandle;
extern osMutexId_t RemoteRecMutexHandle;
#endif

/* Exported constants
 * --------------------------------------------*/
#define RMT_TIMEOUT_MS ((uint16_t)3000)
#define RMT_BEAT_MS ((uint16_t)5000)
#define RMT_BEAT_RUN_MS ((uint16_t)15000)
#define RMT_PAIRING_MS ((uint16_t)5000)
#define RMT_GUARD_RUN_MS (RMT_BEAT_RUN_MS - RMT_TIMEOUT_MS)
#define RMT_CMD_LENGTH 8

/* Exported enums
 * --------------------------------------------*/
typedef enum { RMT_MODE_NORMAL = 0, RMT_MODE_PAIRING } RMT_MODE;

/* Exported types
 * --------------------------------------------*/
typedef struct {
  uint8_t active;
  uint8_t nearby;
  uint32_t tick[RMT_TICK_MAX];
  uint8_t duration[RMT_DUR_MAX];
  aes_key_t pairing_key;
} remote_data_t;

typedef struct {
  uint8_t address[NRF_ADDR_LENGTH];
  uint8_t payload[NRF_PAIR_LENGTH];
} remote_packet_t;

typedef struct {
  remote_data_t d;
  remote_packet_t tx;
  remote_packet_t rx;
  SPI_HandleTypeDef *pspi;
} remote_t;

/* Public variables
 * --------------------------------------------*/
static remote_t RMT = {.d = {0},
                       .tx =
                           {
                               .address = {0x00, 0x00, 0x00, 0x00, 0xAB},
                               .payload = {0},
                           },
                       .rx =
                           {
                               .address = {0x00, 0x00, 0x00, 0x00, 0xCD},
                               .payload = {0},
                           },
                       .pspi = &hspi1};

/* Private variables
 * --------------------------------------------*/
static const uint8_t COMMAND[RMT_CMD_MAX][RMT_CMD_LENGTH] = {
    {0x5e, 0x6c, 0xa7, 0x74, 0xfd, 0xe3, 0xdf, 0xbc},
    {0xf7, 0xda, 0x4a, 0x4f, 0x65, 0x2d, 0x6e, 0xf0},
    {0xff, 0xa6, 0xe6, 0x5a, 0x84, 0x82, 0x66, 0x4f},
    {0xab, 0xf5, 0x83, 0xc4, 0xe9, 0x27, 0x0a, 0xb2},
};

/* Private functions declaration
 * --------------------------------------------*/
static void Lock(void);
static void UnLock(void);
static void ChangeMode(RMT_MODE mode);
static uint8_t ReadPayload(uint8_t *payload);
static uint8_t Transmit(const uint8_t *data);
#if REMOTE_DEBUG
static void RawDebugger(void);
static void Debugger(RMT_CMD command);
#endif

/* Public functions implementation
 * --------------------------------------------*/
uint8_t RMT_Init(void) {
  nrf_param(RMT.pspi, RMT.rx.payload);
  return RMT_ReInit();
}

uint8_t RMT_ReInit(void) {
  uint8_t ok;
  uint32_t tick;

  Lock();
  printf("NRF:Init\n");

  tick = tickMs();
  do {
    MX_SPI1_Init();
    GATE_RemoteReset();

    ok = RMT_Probe();
    if (!ok) delayMs(500);
  } while (!ok && tickIn(tick, RMT_TIMEOUT_MS));

  if (ok) {
    nrf_configure();
    ChangeMode(RMT_MODE_NORMAL);
  }
  UnLock();

  printf("NRF:%s\n", ok ? "OK" : "Error");
  return ok;
}

void RMT_DeInit(void) {
  Lock();
  RMT_Flush();
  GATE_RemoteShutdown();
  HAL_SPI_DeInit(RMT.pspi);
  UnLock();
}

uint8_t RMT_Probe(void) {
  uint8_t ok;

  Lock();
  ok = (nrf_check() != NRF_ERROR);
  UnLock();

  return ok;
}

void RMT_Flush(void) {
  Lock();
  memset(&(RMT.d), 0, sizeof(remote_data_t));
  UnLock();
}

void RMT_Refresh(vehicle_t vehicle) {
  uint32_t *tick = RMT.d.tick;
  uint32_t timeout;

  Lock();
  timeout = vehicle == VEHICLE_RUN ? RMT_BEAT_RUN_MS : RMT_BEAT_MS;
  RMT.d.nearby = tickIn(tick[RMT_TICK_HBEAT], timeout);
  EVT_Write(EVG_REMOTE_MISSING, !RMT.d.nearby);

  RMT.d.active = tickIn(tick[RMT_TICK_PING], RMT_TIMEOUT_MS) || RMT.d.nearby;
  if (!RMT.d.active) {
    RMT_DeInit();
    delayMs(500);
    RMT_Init();
  }

  if (tickOut(tick[RMT_TICK_PAIR], RMT_PAIRING_MS)) {
    tick[RMT_TICK_PAIR] = 0;
    AES_SetKey(NULL);
  }

  UnLock();
}

uint8_t RMT_Ping(vehicle_t vehicle) {
  uint8_t ok;

  //	if (vehicle == VEHICLE_RUN && RMT.d.nearby)
  //		if (!tickOut(RMT.d.tick.heartbeat, RMT_GUARD_RUN_MS))
  //        return 1;

  Lock();
  RNG_Generate32((uint32_t *)RMT.tx.payload, NRF_DATA_LENGTH / 4);
  ok = Transmit(RMT.tx.payload);
  UnLock();

  return ok;
}

uint8_t RMT_Pairing(void) {
  aes_key_t aes;
  uint8_t ok;

  Lock();
  RMT.d.tick[RMT_TICK_PAIR] = tickMs();
  RNG_Generate32(RMT.d.pairing_key, 4);
  AES_SetKey(RMT.d.pairing_key);

  // Insert to payload
  for (uint8_t i = 0; i < 4; i++) aes[i] = swap32(RMT.d.pairing_key[i]);
  memcpy(&RMT.tx.payload[0], aes, NRF_DATA_LENGTH);
  memcpy(&RMT.tx.payload[NRF_DATA_LENGTH], RMT.tx.address, NRF_ADDR_LENGTH);

  ChangeMode(RMT_MODE_PAIRING);
  ok = Transmit(RMT.tx.payload);
  ChangeMode(RMT_MODE_NORMAL);
  UnLock();

  return ok;
}

uint8_t RMT_GotPairedResponse(void) {
  uint8_t paired = 0;

  Lock();
  if (RMT.d.tick[RMT_TICK_PAIR] > 0) {
    RMT.d.tick[RMT_TICK_PAIR] = 0;
    paired = 1;

    AES_EE_Key(RMT.d.pairing_key);
    AES_SetKey(NULL);
  }
  UnLock();

  return paired;
}

uint8_t RMT_ValidateCommand(RMT_CMD *cmd) {
  uint8_t ok = 0, plain[NRF_DATA_LENGTH];
  uint8_t rng = NRF_DATA_LENGTH / 2;

  Lock();
  if (ReadPayload(plain)) {
    for (uint8_t i = 0; i < RMT_CMD_MAX; i++) {
      if ((ok = (memcmp(plain, &COMMAND[i], RMT_CMD_LENGTH) == 0))) {
        *cmd = i;
        break;
      }
    }
  }

  // Is ping response
  if (ok)
    if (*cmd == RMT_CMD_PING || *cmd == RMT_CMD_SEAT)
      ok = (memcmp(&plain[rng], &RMT.tx.payload[rng], rng) == 0);

  if (ok) RMT.d.tick[RMT_TICK_HBEAT] = tickMs();
  UnLock();

#if REMOTE_DEBUG
  if (ok) Debugger(*cmd);
#endif

  return ok;
}

void RMT_IrqHandler(void) {
  if (nrf_irq_handler()) {
    osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_RX_IT);
#if REMOTE_DEBUG
    RawDebugger();
#endif
  }
}

uint8_t RMT_IO_Active(void) { return RMT.d.active; }

uint8_t RMT_IO_Nearby(void) { return RMT.d.nearby; }

uint32_t RMT_IO_Tick(RMT_TICK key) { return RMT.d.tick[key]; }

uint8_t RMT_IO_Duration(RMT_DURATION key) { return RMT.d.duration[key]; }

void RMT_IO_SetTick(RMT_TICK key) { RMT.d.tick[key] = tickMs(); }

void RMT_IO_SetDuration(RMT_DURATION key, uint32_t tick) {
  RMT.d.duration[key] = tickMs() - tick;
}
/* Private functions implementation
 * --------------------------------------------*/
static void Lock(void) {
#if (APP)
  osMutexAcquire(RemoteRecMutexHandle, osWaitForever);
#endif
}

static void UnLock(void) {
#if (APP)
  osMutexRelease(RemoteRecMutexHandle);
#endif
}

static void ChangeMode(RMT_MODE mode) {
  uint8_t payload_width = NRF_DATA_LENGTH;
  uint32_t ID = IAP_GetBootMeta(VIN_OFFSET);

  if (mode == RMT_MODE_PAIRING) {
    payload_width = NRF_PAIR_LENGTH;
    ID = 0x00;
  }

  // Set NRF Config (pairing mode)
  memcpy(RMT.rx.address, &ID, sizeof(uint32_t));
  nrf_change_mode(RMT.tx.address, RMT.rx.address, payload_width);
}

static uint8_t ReadPayload(uint8_t *payload) {
  return AES_Decrypt(payload, RMT.rx.payload, NRF_DATA_LENGTH);
}

static uint8_t Transmit(const uint8_t *data) {
  uint32_t tick;

  nrf_send_packet_noack(data);
  tick = tickMs();
  while (nrf_tx_busy() && tickIn(tick, 3)) {
  };

  return !nrf_tx_busy();
}

#if REMOTE_DEBUG
static void RawDebugger(void) {
  printf("NRF:Receive = %.*s\n", NRF_DATA_LENGTH, (char *)RMT.rx.payload);
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
