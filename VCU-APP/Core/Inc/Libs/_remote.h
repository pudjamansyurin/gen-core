/*
 * _.h
 *
 *  Created on: Mar 4, 2020
 *      Author: pudja
 */

#ifndef REMOTE_H_
#define REMOTE_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_nrf24l01.h"

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
  struct {
    uint8_t address[NRF_ADDR_LENGTH ];
    uint8_t payload[NRF_DATA_PAIR_LENGTH ];
  } tx;
  struct {
    uint8_t address[NRF_ADDR_LENGTH ];
    uint8_t payload[NRF_DATA_PAIR_LENGTH ];
  } rx;
  struct {
    uint32_t heartbeat;
    uint32_t pairing;
  } tick;
} remote_t;

/* Public functions prototype ------------------------------------------------*/
void RF_Init(uint32_t *unit_id, SPI_HandleTypeDef *hspi);
void RF_DeInit(void);
uint8_t RF_Ping(void);
void RF_Pairing(uint32_t *unit_id);
void RF_Debugger(void);
uint8_t RF_ValidateCommand(RF_CMD *cmd);
void RF_GenerateAesKey(uint32_t *aes);
void RF_Refresh(void);
uint8_t RF_IsTimeout(void);
uint8_t RF_GotPairedResponse(void);
void RF_IrqHandler(void);
void RF_PacketReceived(uint8_t *data);

#endif /* REMOTE_H_ */
