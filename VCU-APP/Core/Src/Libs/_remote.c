/*
 * .c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "rng.h"
#include "Drivers/_aes.h"
#include "Libs/_remote.h"
#include "Libs/_eeprom.h"

/* External variables -----------------------------------------------------------*/
//extern osMutexId_t RemoteRecMutexHandle;
extern osThreadId_t RemoteTaskHandle;

/* Private variables -----------------------------------------------------------*/
static remote_t RF = {
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
static void SetAesPayload(uint32_t *aes);
static void ChangeMode(RF_MODE mode, uint32_t *unit_id);
static uint8_t Payload(RF_ACTION action, uint8_t *payload);
static void GenRandomNumber32(uint32_t *payload, uint8_t size);
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
void RF_Init(uint32_t *unit_id, SPI_HandleTypeDef *hspi) {
  nrf_param(hspi, RF.rx.payload);
  nrf_init();
  nrf_configure();
  ChangeMode(RF_MODE_NORMAL, unit_id);
}

void RF_DeInit(void) {
  nrf_deinit();
}

uint8_t RF_Ping(uint8_t *unremote) {
  *unremote =  ((_GetTickMS() - RF.tick.heartbeat) > REMOTE_TIMEOUT );

  GenRandomNumber32((uint32_t*) RF.tx.payload, NRF_DATA_LENGTH / 4);

  return (nrf_send_packet_noack(RF.tx.payload) == NRF_OK);
}

void RF_Pairing(uint32_t *unit_id) {
  uint32_t aes[4];

  RF.tick.pairing = _GetTickMS();

  // Insert AES to payload
  RF_GenerateAesKey(aes);
  EEPROM_AesKey(EE_CMD_W, aes);
  SetAesPayload(aes);

  // Insert VCU_ID to payload
  memcpy(&RF.tx.payload[NRF_DATA_LENGTH ], RF.tx.address, NRF_ADDR_LENGTH);

  ChangeMode(RF_MODE_PAIRING, NULL);
  nrf_send_packet_noack(RF.tx.payload);

  // back to normal
  nrf_init();
  nrf_configure();
  ChangeMode(RF_MODE_NORMAL, unit_id);
}

uint8_t RF_ValidateCommand(RF_CMD *cmd) {
  uint8_t *payload = RF.tx.payload;
  uint8_t valid = 0, plain[NRF_DATA_LENGTH ];
  const uint8_t rng = NRF_DATA_LENGTH / 2;

  lock();
  // Read Payload
  if (Payload(RF_ACTION_R, plain)) {
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
    if (*cmd == RF_CMD_PING || *cmd == RF_CMD_SEAT)
      if (memcmp(&plain[rng], &payload[rng], rng) == 0)
        valid = 1;
  unlock();

  return valid;
}

void RF_GenerateAesKey(uint32_t *aes) {
  GenRandomNumber32(aes, NRF_DATA_LENGTH / 4);
}

void RF_Debugger(void) {
  lock();
  LOG_Str("NRF:Receive = ");
  LOG_BufHex((char*) RF.rx.payload, NRF_DATA_LENGTH);
  LOG_Enter();
  unlock();
}

uint8_t RF_GotPairedResponse(void) {
  uint8_t paired = 0;

  if (RF.tick.pairing > 0) {
    if (_GetTickMS() - RF.tick.pairing < REMOTE_PAIRING_TIMEOUT)
      paired = 1;
    RF.tick.pairing = 0;
  }

  return paired;
}

void RF_IrqHandler(void) {
  nrf_irq_handler();
}

void RF_PacketReceived(uint8_t *data) {
  RF.tick.heartbeat = _GetTickMS();
  osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_RX_IT);
}

uint32_t RF_Heartbeat(void) {
  return RF.tick.heartbeat;
}

/* Private functions implementation --------------------------------------------*/
static void SetAesPayload(uint32_t *aes) {
  uint32_t swapped;

  // insert to payload & swap byte order
  for (uint8_t i = 0; i < (NRF_DATA_LENGTH / 4); i++) {
    swapped = _ByteSwap32(*(aes + i));
    memcpy(&RF.tx.payload[i * 4], &swapped, 4);
  }
}

static void ChangeMode(RF_MODE mode, uint32_t *unit_id) {
  uint8_t payload_width;

  if (mode == RF_MODE_NORMAL) {
    // use VCU_ID as address
    memcpy(RF.tx.address, unit_id, 4);
    memcpy(RF.rx.address, unit_id, 4);
    payload_width = NRF_DATA_LENGTH;
  } else {
    // Set Address (pairing mode)
    memset(RF.tx.address, 0x00, 4);
    memset(RF.rx.address, 0x00, 4);
    payload_width = NRF_DATA_PAIR_LENGTH;
  }

  // Set NRF Config (pairing mode)
  nrf_change_mode(RF.tx.address, RF.rx.address, payload_width);
}

static uint8_t Payload(RF_ACTION action, uint8_t *payload) {
  uint8_t ret = 0;

  // Process Payload
  lock();
  if (action == RF_ACTION_R)
    ret = AES_Decrypt(payload, RF.rx.payload, NRF_DATA_LENGTH);
  else
    ret = AES_Encrypt(RF.tx.payload, payload, NRF_DATA_LENGTH);

  unlock();

  return ret;
}

static void GenRandomNumber32(uint32_t *payload, uint8_t size) {
  for (uint8_t i = 0; i < size; i++)
    HAL_RNG_GenerateRandomNumber(&hrng, payload++);
}

static void lock(void) {
  //  osMutexAcquire(RemoteRecMutexHandle, osWaitForever);
}

static void unlock(void) {
  //  osMutexRelease(RemoteRecMutexHandle);
}
