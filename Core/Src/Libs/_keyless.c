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
                .address = { 0xAB, 0x00, 0x00, 0x00, 0x00 },
                .payload = { 0 },
        },
        .rx = {
                .address = { 0xCD, 0x00, 0x00, 0x00, 0x00 },
                .payload = { 0 },
        },
};

/* Private functions declaration ---------------------------------------------*/
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
void KLESS_Init(void) {
    // set configuration
    KLESS.config.tx_address = (uint8_t*) KLESS.tx.address;
    KLESS.config.rx_address = (uint8_t*) KLESS.rx.address;
    KLESS.config.rx_buffer = (uint8_t*) KLESS.rx.payload;
    nrf_set_config(&(KLESS.config));

    // initialisation
    nrf_init(&nrf, &(KLESS.config));
}

uint8_t KLESS_ValidateCommand(KLESS_CMD *cmd) {
    uint8_t pos, valid, payload[NRF_DATA_LENGTH] = { 0 };

    lock();
    // Read Payload
    valid = KLESS_Payload(KLESS_R, payload);

    // Get Payload position
    if (valid) {
        pos = (payload[0] & 0xF0) >> 4;
        *cmd = payload[pos] & 0x0F;

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
    const uint8_t tx_address[NRF_ADDR_LENGTH] = { 0xAB, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t rx_address[NRF_ADDR_LENGTH] = { 0xCD, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t payload_length = NRF_DATA_LENGTH + NRF_ADDR_LENGTH;
    uint8_t payload[NRF_DATA_LENGTH + NRF_ADDR_LENGTH] = { 0 };
    NRF_RESULT p;

    // Generate Payload
    // insert AES Key
    KLESS_GenerateAesKey((uint32_t*) payload);
    // insert address
    memcpy(&payload[NRF_DATA_LENGTH], KLESS.tx.address, NRF_ADDR_LENGTH);

    // Default pairing configuration
    ce_reset(&nrf);
    nrf_set_tx_address(&nrf, (uint8_t*) tx_address);
    nrf_set_rx_address_p0(&nrf, (uint8_t*) rx_address);
    nrf_set_rx_payload_width_p0(&nrf, payload_length);
    ce_set(&nrf);
    // Send Payload
    p = nrf_send_packet(&nrf, payload);
    // Retrieve back the configuration
    ce_reset(&nrf);
    nrf_set_tx_address(&nrf, nrf.config.tx_address);
    nrf_set_rx_address_p0(&nrf, nrf.config.rx_address);
    nrf_set_rx_payload_width_p0(&nrf, nrf.config.payload_length);
    ce_set(&nrf);

    // Update the AES key, and save  permanently
    EEPROM_AesKey(EE_CMD_W, (uint8_t*) payload);

    // debug
    LOG_Str("NRF:Send = ");
    LOG_Str(p ? "OK" : "ERROR");
    LOG_Enter();

    return (p == NRF_OK);
}

uint8_t KLESS_SendDummy(void) {
    uint8_t payload[NRF_DATA_LENGTH] = { 0 };
    NRF_RESULT p;

    payload[0] = 1;
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

