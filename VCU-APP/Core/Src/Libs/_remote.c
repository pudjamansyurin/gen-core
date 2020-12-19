/*
 * .c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_remote.h"
#include "Libs/_eeprom.h"
#include "Drivers/_aes.h"
#include "rng.h"

/* Private variables -----------------------------------------------------------*/
//extern osMutexId_t RemoteRecMutexHandle;
extern osThreadId_t RemoteTaskHandle;

/* Private variables -----------------------------------------------------------*/
static nrf24l01 NRF;
static rf_t RF = {
    .tx = {
        .address = { 0x00, 0x00, 0x00, 0x00, 0xAB },
        .payload = { 0 },
    },
    .rx = {
        .address = { 0x00, 0x00, 0x00, 0x00, 0xCD },
        .payload = { 0 },
    },
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
  NRF.config.addr_width = NRF_ADDR_LENGTH - 2;
  NRF.config.payload_length = NRF_DATA_LENGTH;
  NRF.config.rx_buffer = NULL;

  NRF.config.data_rate = NRF_DATA_RATE_250KBPS;
  NRF.config.tx_power = NRF_TX_PWR_M18dBm;
  NRF.config.crc_width = NRF_CRC_WIDTH_1B;
  NRF.config.retransmit_count = 0x0F;   // maximum is 15 times
  NRF.config.retransmit_delay = 0x0F; // 4000us, LSB:250us
  NRF.config.rf_channel = 110;
  NRF.config.spi = hspi;
  NRF.config.spi_timeout = 3; // milliseconds

  nrf_init(&NRF);
  nrf_configure(&NRF);
  ChangeMode(RF_MODE_NORMAL, unit_id);
}

uint8_t RF_Ping(void) {
  NRF_RESULT p;

  GenRandomNumber32((uint32_t*) RF.tx.payload, NRF_DATA_LENGTH / 4);

  p = nrf_send_packet_noack(&NRF, RF.tx.payload);

  return (p == NRF_OK);
}

uint8_t RF_Pairing(uint32_t *unit_id) {
  NRF_RESULT p = NRF_ERROR;
  uint32_t aes[4];

  // Insert AES to payload
  RF_GenerateAesKey(aes);
  EEPROM_AesKey(EE_CMD_W, aes);
  SetAesPayload(aes);

  // Insert VCU_ID to payload
  memcpy(&RF.tx.payload[NRF_DATA_LENGTH ], RF.tx.address, NRF_ADDR_LENGTH);

  ChangeMode(RF_MODE_PAIRING, NULL);
  p = nrf_send_packet_noack(&NRF, RF.tx.payload);

  // back to normal
  nrf_init(&NRF);
  nrf_configure(&NRF);
  ChangeMode(RF_MODE_NORMAL, unit_id);

  return (p == NRF_OK);
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

uint8_t RF_Refresh(uint32_t tick_) {
  return ((_GetTickMS() - tick_) > REMOTE_TIMEOUT );
}

void RF_IrqHandler(void) {
  nrf_irq_handler(&NRF);
}

void RF_PacketReceived(uint8_t *data) {
  osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_RX_IT);
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
  if (mode == RF_MODE_NORMAL) {
    // use VCU_ID as address
    memcpy(RF.tx.address, unit_id, 4);
    memcpy(RF.rx.address, unit_id, 4);
    NRF.config.payload_length = NRF_DATA_LENGTH;
  } else {
    // Set Address (pairing mode)
    memset(RF.tx.address, 0x00, 4);
    memset(RF.rx.address, 0x00, 4);
    NRF.config.payload_length = NRF_DATA_PAIR_LENGTH;
  }

  // set configuration
  NRF.config.tx_address = RF.tx.address;
  NRF.config.rx_address = RF.rx.address;
  NRF.config.rx_buffer = RF.rx.payload;

  // Set NRF Config (pairing mode)
  nrf_change_mode(&NRF);
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
