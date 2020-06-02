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
extern nrf24l01 nrf;
extern vcu_t VCU;
extern hmi1_t HMI1;
extern uint32_t AesKey[4];

/* Public variables -----------------------------------------------------------*/
kless_t KLESS = {
        .config = {
                .addr_width = NRF_ADDR_LENGTH - 2,
                .payload_length = NRF_DATA_LENGTH,
                .rx_buffer = NULL,
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
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
void KLESS_Init(void) {
    nrf24l01_config *config = &(KLESS.config);

    // use VCU_ID as address
    memcpy(KLESS.tx.address, &(VCU.d.unit_id), sizeof(VCU.d.unit_id));
    memcpy(KLESS.rx.address, &(VCU.d.unit_id), sizeof(VCU.d.unit_id));

    // set configuration
    config->tx_address = KLESS.tx.address;
    config->rx_address = KLESS.rx.address;
    config->rx_buffer = KLESS.rx.payload;

    config->data_rate = NRF_DATA_RATE_250KBPS;
    config->tx_power = NRF_TX_PWR_0dBm;
    config->crc_width = NRF_CRC_WIDTH_1B;
    config->retransmit_count = 0x0F;   // maximum is 15 times
    config->retransmit_delay = 0x0F; // 4000us, LSB:250us
    config->rf_channel = 110;

    config->spi = &hspi1;
    config->spi_timeout = 100; // milliseconds
    config->csn_port = INT_KEYLESS_CSN_GPIO_Port;
    config->csn_pin = INT_KEYLESS_CSN_Pin;
    config->ce_port = INT_KEYLESS_CE_GPIO_Port;
    config->ce_pin = INT_KEYLESS_CE_Pin;
    config->irq_port = INT_KEYLESS_IRQ_GPIO_Port;
    config->irq_pin = INT_KEYLESS_IRQ_Pin;

    // initialization
    nrf_init(&nrf, config);
}

uint8_t KLESS_ValidateCommand(KLESS_CMD *cmd) {
    uint8_t valid, payload_dec[NRF_DATA_LENGTH];

    lock();
    // Read Payload
    valid = KLESS_Payload(KLESS_R, payload_dec);

    // Get Payload position
    if (valid) {
        valid = 0;
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
    unlock();

    return valid;
}

uint8_t KLESS_Payload(KLESS_MODE mode, uint8_t *payload) {
    uint8_t ret = 0;

    lock();
    // Process Payload
    if (mode == KLESS_R) {
        // Decrypt
        if (AES_Decrypt(payload, KLESS.rx.payload, NRF_DATA_LENGTH)) {
            ret = 1;
        }
    } else {
        // Encrypt
        if (AES_Encrypt(KLESS.tx.payload, payload, NRF_DATA_LENGTH)) {
            ret = 1;
        }
    }
    unlock();

    return ret;
}

void KLESS_GenerateAesKey(uint32_t *payload) {
    for (uint8_t i = 0; i < (NRF_DATA_LENGTH / sizeof(uint32_t)); i++) {
        HAL_RNG_GenerateRandomNumber(&hrng, payload++);
    }
}

uint8_t KLESS_Pairing(void) {
    uint8_t *payload = KLESS.tx.payload;
    uint32_t aes[4], swapped;
    NRF_RESULT p = NRF_ERROR;

    // Insert AES Key
    KLESS_GenerateAesKey(aes);
    // swap byte order
    for (uint8_t i = 0; i < 4; i++) {
        swapped = _ByteSwap32(aes[i]);
        memcpy(&payload[i * 4], &swapped, sizeof(swapped));
    }
    // Insert VCU_ID
    memcpy(&payload[NRF_DATA_LENGTH], KLESS.tx.address, NRF_ADDR_LENGTH);

    // Set Address (pairing mode)
    memset(KLESS.tx.address, 0x00, sizeof(VCU.d.unit_id));
    memset(KLESS.rx.address, 0x00, sizeof(VCU.d.unit_id));

    // Set NRF Config (pairing mode)
    ce_reset(&nrf);
    nrf_set_tx_address(&nrf, KLESS.tx.address);
    nrf_set_rx_address_p0(&nrf, KLESS.rx.address);
    nrf_set_rx_payload_width_p0(&nrf, NRF_DATA_PAIR_LENGTH);
    ce_set(&nrf);

    // Send Payload
    p = nrf_send_packet(&nrf, payload);
    osDelay(100);

    // Set Address (normal mode)
    memcpy(KLESS.tx.address, &(VCU.d.unit_id), sizeof(VCU.d.unit_id));
    memcpy(KLESS.rx.address, &(VCU.d.unit_id), sizeof(VCU.d.unit_id));
    // Set Aes Key (new)
    EEPROM_AesKey(EE_CMD_W, aes);

    // Set NRF Config (normal mode)
    KLESS_Init();

    return (p == NRF_OK);
}

uint8_t KLESS_SendDummy(void) {
    uint8_t *payload = KLESS.tx.payload;
    NRF_RESULT p;

    // set payload
    memset(payload, 0x00, NRF_DATA_LENGTH);
    payload[0] = 1;

    // send payload
    p = nrf_send_packet(&nrf, payload);

    // debug
    LOG_Str("NRF:Send = ");
    LOG_Str(p ? "OK" : "ERROR");
    LOG_Enter();

    return (p == NRF_OK);
}

void KLESS_Debugger(void) {
    lock();
    LOG_Str("NRF:Receive = ");
    LOG_BufHex((char*) KLESS.rx.payload, NRF_DATA_LENGTH);
    LOG_Enter();
    unlock();
}

void KLESS_Refresh(void) {
    if ((osKernelGetTickCount() - VCU.d.tick.keyless) < pdMS_TO_TICKS(KEYLESS_TIMEOUT)) {
        HMI1.d.status.keyless = 1;
    } else {
        HMI1.d.status.keyless = 0;
    }
}

void KLESS_IrqHandler(void) {
    nrf_irq_handler(&nrf);
}

void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data) {
    // used in favor of nrf_receive_packet
    dev->rx_busy = 0;

    osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_RX_IT);
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
    osMutexAcquire(KlessRecMutexHandle, osWaitForever);
}

static void unlock(void) {
    osMutexRelease(KlessRecMutexHandle);
}

