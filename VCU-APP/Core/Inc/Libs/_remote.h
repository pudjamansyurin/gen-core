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
typedef enum { RMT_CMD_PING = 0, RMT_CMD_ALARM, RMT_CMD_SEAT } RMT_CMD;

typedef enum { RMT_ACTION_R = 0, RMT_ACTION_W } RMT_ACTION;

typedef enum { RMT_MODE_NORMAL = 0, RMT_MODE_PAIRING } RMT_MODE;

/* Exported struct -----------------------------------------------------------*/
typedef struct {
  struct {
    uint8_t address[NRF_ADDR_LENGTH];
    uint8_t payload[NRF_DATA_PAIR_LENGTH];
  } tx;
  struct {
    uint8_t address[NRF_ADDR_LENGTH];
    uint8_t payload[NRF_DATA_PAIR_LENGTH];
  } rx;
  struct {
    uint32_t heartbeat;
    uint32_t pairing;
  } tick;
  uint32_t pairingAes[4];
  SPI_HandleTypeDef *pspi;
} remote_t;

/* Public functions prototype ------------------------------------------------*/
void RMT_Init(void);
void RMT_DeInit(void);
void RMT_ReInit(void);
uint8_t RMT_NeedPing(void);
uint8_t RMT_Ping(void);
void RMT_Pairing(void);
uint8_t RMT_GotPairedResponse(void);
void RMT_RefreshPairing(void);
uint8_t RMT_ValidateCommand(RMT_CMD *cmd);
void RMT_IrqHandler(void);
void RMT_PacketReceived(uint8_t *data);

#endif /* REMOTE_H_ */
