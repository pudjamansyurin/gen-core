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
#define NRF_DATA_LENGTH		      (uint8_t) 16									// Max: 32 bytes
#define NRF_ADDR_LENGTH         (uint8_t) 5                                    // Range 3:5
#define NRF_DATA_PAIR_LENGTH    (uint8_t) (NRF_DATA_LENGTH + NRF_ADDR_LENGTH)

/* Exported enum -------------------------------------------------------------*/
typedef enum {
    RF_CMD_PING = 0,
    RF_CMD_ALARM,
    RF_CMD_SEAT
} RF_CMD;

typedef enum {
    RF_ACTION_R = 0, RF_ACTION_W
} RF_ACTION;

typedef enum {
	RF_MODE_NORMAL = 0,
	RF_MODE_PAIRING
} RF_MODE;

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
void RF_Init(void);
void RF_Debugger(void);
uint8_t RF_ValidateCommand(RF_CMD *cmd);
void RF_GenerateAesKey(uint32_t *aes);
uint8_t RF_Pairing(void);
uint8_t RF_SendPing(uint8_t retry);
void RF_Refresh(void);
void RF_IrqHandler(void);
void nrf_packet_received_callback(nrf24l01 *dev, uint8_t *data);

#endif /* KEYLESS_H_ */
