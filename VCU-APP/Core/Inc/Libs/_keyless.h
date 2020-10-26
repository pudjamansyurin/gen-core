/*
 * _keyless.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef KEYLESS_H_
#define KEYLESS_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_nrf24l01.h"

/* Exported define -----------------------------------------------------------*/
#define NRF_DATA_LENGTH		    (uint8_t) 16									// Max: 32 bytes
#define NRF_ADDR_LENGTH         (uint8_t) 5                                    // Range 3:5
#define NRF_DATA_PAIR_LENGTH    (uint8_t) (NRF_DATA_LENGTH + NRF_ADDR_LENGTH)

/* Exported enum -------------------------------------------------------------*/
typedef enum {
    KLESS_CMD_PING = 0,
    KLESS_CMD_ALARM = 1,
    KLESS_CMD_SEAT = 2
} KLESS_CMD;

typedef enum {
    KLESS_R = 0, KLESS_W = 1
} KLESS_MODE;

/* Exported struct -----------------------------------------------------------*/
typedef struct {
    nrf24l01_config config;
    struct {
        uint8_t address[NRF_ADDR_LENGTH ];
        uint8_t payload[NRF_DATA_PAIR_LENGTH ];
    } tx;
    struct {
        uint8_t address[NRF_ADDR_LENGTH ];
        uint8_t payload[NRF_DATA_PAIR_LENGTH ];
    } rx;
} kless_t;

/* Public functions prototype ------------------------------------------------*/
void KLESS_Init(void);
void KLESS_Debugger(void);
uint8_t KLESS_ValidateCommand(KLESS_CMD *cmd);
void KLESS_GenerateAesKey(uint32_t *payload);
uint8_t KLESS_Pairing(void);
uint8_t KLESS_SendPing(void);
void KLESS_Refresh(void);
void KLESS_IrqHandler(void);
void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data);

#endif /* KEYLESS_H_ */
