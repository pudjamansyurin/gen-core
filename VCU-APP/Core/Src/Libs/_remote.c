/*
 * .c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "rng.h"
#include "spi.h"
#include "Drivers/_aes.h"
#include "Libs/_remote.h"
#include "Libs/_eeprom.h"

/* External variables -----------------------------------------------------------*/
//extern osMutexId_t RemoteRecMutexHandle;
extern osThreadId_t RemoteTaskHandle;

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
    }
};
static const uint8_t COMMAND[3][8] = {
    { 0x5e, 0x6c, 0xa7, 0x74, 0xfd, 0xe3, 0xdf, 0xbc },
    { 0xf7, 0xda, 0x4a, 0x4f, 0x65, 0x2d, 0x6e, 0xf0 },
    { 0xff, 0xa6, 0xe6, 0x5a, 0x84, 0x82, 0x66, 0x4f },
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);
static void ReInit(void);
static void SetAesPayload(uint32_t *aes);
static void ChangeMode(RMT_MODE mode, uint32_t *unit_id);
static uint8_t Payload(RMT_ACTION action, uint8_t *payload);
static void GenRandomNumber32(uint32_t *payload, uint8_t size);
static void Debugger(void);

/* Public functions implementation --------------------------------------------*/
void RMT_Init(uint32_t *unit_id, SPI_HandleTypeDef *hspi) {
  RMT.h.spi = hspi;

  nrf_param(hspi, RMT.rx.payload);

  ReInit();

  nrf_configure();
  ChangeMode(RMT_MODE_NORMAL, unit_id);
}

void RMT_DeInit(void) {
  GATE_RemoteShutdown();
  HAL_SPI_DeInit(RMT.h.spi);
  _DelayMS(1000);
}

uint8_t RMT_Ping(uint8_t *unremote) {
  if (RMT.tick.heartbeat)
    *unremote =  ((_GetTickMS() - RMT.tick.heartbeat) > REMOTE_TIMEOUT );

  GenRandomNumber32((uint32_t*) RMT.tx.payload, NRF_DATA_LENGTH / 4);

  return (nrf_send_packet_noack(RMT.tx.payload) == NRF_OK);
}

void RMT_Pairing(uint32_t *unit_id) {
  uint32_t aes[4];

  RMT.tick.pairing = _GetTickMS();

  // Insert AES to payload
  RMT_GenerateAesKey(aes);
  EEPROM_AesKey(EE_CMD_W, aes);
  SetAesPayload(aes);

  // Insert VCU_ID to payload
  memcpy(&RMT.tx.payload[NRF_DATA_LENGTH ], RMT.tx.address, NRF_ADDR_LENGTH);

  ChangeMode(RMT_MODE_PAIRING, NULL);
  nrf_send_packet_noack(RMT.tx.payload);

  // back to normal
  ReInit();
  nrf_configure();
  ChangeMode(RMT_MODE_NORMAL, unit_id);
}

uint8_t RMT_ValidateCommand(RMT_CMD *cmd) {
  uint8_t *payload = RMT.tx.payload;
  uint8_t valid = 0, plain[NRF_DATA_LENGTH ];
  const uint8_t rng = NRF_DATA_LENGTH / 2;

  lock();
  // Read Payload
  if (Payload(RMT_ACTION_R, plain)) {
    // Check Payload Command
    for (uint8_t i = 0; i < (sizeof(COMMAND) / sizeof(COMMAND[0])); i++) {
      // check command
      if (memcmp(plain, &COMMAND[i], 8) == 0) {
        *cmd = i;
        valid = 1;
        break;
      }
    }
  }
  // Check is valid ping response
  if (valid)
    if (*cmd == RMT_CMD_PING || *cmd == RMT_CMD_SEAT)
      if (memcmp(&plain[rng], &payload[rng], rng) == 0)
        valid = 1;
  unlock();

  return valid;
}

void RMT_GenerateAesKey(uint32_t *aes) {
  GenRandomNumber32(aes, NRF_DATA_LENGTH / 4);
}

uint8_t RMT_GotPairedResponse(void) {
  uint8_t paired = 0;

  if (RMT.tick.pairing > 0) {
    if (_GetTickMS() - RMT.tick.pairing < REMOTE_PAIRING_TIMEOUT)
      paired = 1;
    RMT.tick.pairing = 0;
  }

  return paired;
}

void RMT_IrqHandler(void) {
  nrf_irq_handler();
}

void RMT_PacketReceived(uint8_t *data) {
  RMT.tick.heartbeat = _GetTickMS();
  // Debugger();
  osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_RX_IT);
}

uint8_t RMT_NeedReset(void) {
  return (_GetTickMS() - RMT.tick.heartbeat) > REMOTE_NEED_RESET;
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
  //  osMutexAcquire(RemoteRecMutexHandle, osWaitForever);
}

static void unlock(void) {
  //  osMutexRelease(RemoteRecMutexHandle);
}

static void ReInit(void) {
  MX_SPI1_Init();
  do {
    Log("NRF:Init\n");

    //    HAL_SPI_Init(hspi);
    GATE_RemoteReset();
    _DelayMS(1000);
  } while (nrf_check() == NRF_ERROR);
}

static void SetAesPayload(uint32_t *aes) {
  uint32_t swapped;

  // insert to payload & swap byte order
  for (uint8_t i = 0; i < (NRF_DATA_LENGTH / 4); i++) {
    swapped = _ByteSwap32(*(aes + i));
    memcpy(&RMT.tx.payload[i * 4], &swapped, 4);
  }
}

static void ChangeMode(RMT_MODE mode, uint32_t *unit_id) {
  uint8_t payload_width;

  if (mode == RMT_MODE_NORMAL) {
    // use VCU_ID as address
    memcpy(RMT.tx.address, unit_id, 4);
    memcpy(RMT.rx.address, unit_id, 4);
    payload_width = NRF_DATA_LENGTH;
  } else {
    // Set Address (pairing mode)
    memset(RMT.tx.address, 0x00, 4);
    memset(RMT.rx.address, 0x00, 4);
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

static void GenRandomNumber32(uint32_t *payload, uint8_t size) {
  for (uint8_t i = 0; i < size; i++)
    HAL_RNG_GenerateRandomNumber(&hrng, payload++);
}

static void Debugger(void) {
  Log("NRF:Receive = %.s\n", NRF_DATA_LENGTH, (char*) RMT.rx.payload);
}
