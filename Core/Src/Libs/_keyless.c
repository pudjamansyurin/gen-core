/*
 * keyless.c
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_keyless.h"
#include "Libs/_eeprom.h"
#include "Nodes/VCU.h"
#include "Nodes/HMI1.h"
#include "Drivers/_aes.h"

/* External variables ----------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1;
extern osThreadId_t KeylessTaskHandle;
extern osMutexId_t KlessRecMutexHandle;
extern RNG_HandleTypeDef hrng;
extern nrf24l01 nrf;
extern vcu_t VCU;
extern hmi1_t HMI1;

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

    // initialisation
    nrf_init(&nrf, config);
}

uint8_t KLESS_ValidateCommand(KLESS_CMD *cmd) {
    uint8_t pos, valid, payload_dec[NRF_DATA_LENGTH];

    lock();
    // Read Payload
    valid = KLESS_Payload(KLESS_R, payload_dec);

    // Get Payload position
    if (valid) {
        pos = (payload_dec[0] & 0xF0) >> 4;
        *cmd = payload_dec[pos] & 0x0F;

        // Check Payload Command
        if (*cmd > KLESS_CMD_MAX) {
            valid = 0;
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
            // FIXME: use AES
//            memcpy(payload, KLESS.rx.payload, NRF_DATA_LENGTH);
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
    NRF_RESULT p;
    uint8_t transmit250ms = 2;
    uint8_t *payload = KLESS.tx.payload;

    // Generate Payload

    // insert AES Key
    KLESS_GenerateAesKey((uint32_t*) payload);
    // insert VCU_ID
    memcpy(&payload[NRF_DATA_LENGTH], KLESS.tx.address, NRF_ADDR_LENGTH);

    // Reset Address
    memset(KLESS.tx.address, 0x00, sizeof(VCU.d.unit_id));
    memset(KLESS.rx.address, 0x00, sizeof(VCU.d.unit_id));

//    LOG_StrLn("NRF:Payload");
//    LOG_BufHex((char*) payload, NRF_DATA_PAIR_LENGTH);
//    LOG_Enter();
//    LOG_Str("NRF:TX = ");
//    LOG_BufHex((char*) KLESS.tx.address, sizeof(KLESS.tx.address));
//    LOG_Enter();
//    LOG_Str("NRF:RX = ");
//    LOG_BufHex((char*) KLESS.rx.address, sizeof(KLESS.rx.address));
//    LOG_Enter();

    // Default pairing configuration
    ce_reset(&nrf);
    nrf_set_tx_address(&nrf, KLESS.tx.address);
    nrf_set_rx_address_p0(&nrf, KLESS.rx.address);
    nrf_set_rx_payload_width_p0(&nrf, NRF_DATA_PAIR_LENGTH);
    ce_set(&nrf);

    // Send Payload
    while (transmit250ms--) {
        p = nrf_send_packet(&nrf, payload);
        _LedToggle();

        osDelay(250);
    }

    // Set Address (back)
    memcpy(KLESS.tx.address, &(VCU.d.unit_id), sizeof(VCU.d.unit_id));
    memcpy(KLESS.rx.address, &(VCU.d.unit_id), sizeof(VCU.d.unit_id));

    // Retrieve back the configuration
    ce_reset(&nrf);
    nrf_set_tx_address(&nrf, nrf.config.tx_address);
    nrf_set_rx_address_p0(&nrf, nrf.config.rx_address);
    nrf_set_rx_payload_width_p0(&nrf, nrf.config.payload_length);
    ce_set(&nrf);

    // Update the AES key, and save  permanently
    EEPROM_AesKey(EE_CMD_W, (uint8_t*) payload);
    AES_Init();

    return (p == NRF_OK);
}

uint8_t KLESS_SendDummy(void) {
    NRF_RESULT p;
    uint8_t *payload = KLESS.tx.payload;

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
    if ((osKernelGetTickCount() - VCU.d.tick.keyless)
            < pdMS_TO_TICKS(KEYLESS_TIMEOUT)) {
        HMI1.d.status.keyless = 1;
    } else {
        HMI1.d.status.keyless = 0;
    }
}

void KLESS_IrqHandler(void) {
    nrf_irq_handler(&nrf);
    osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_RX_IT);
}

/* Private functions implementation --------------------------------------------*/
static void lock(void) {
    osMutexAcquire(KlessRecMutexHandle, osWaitForever);
}

static void unlock(void) {
    osMutexRelease(KlessRecMutexHandle);
}

