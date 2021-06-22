/*
 * _.h
 *
 *  Created on: Mar 4, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef REMOTE_H_
#define REMOTE_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_nrf24l01.h"

/* Exported constants
 * --------------------------------------------*/
#define RMT_TIMEOUT_MS ((uint16_t)3000)
#define RMT_BEAT_MS ((uint16_t)5000)
#define RMT_BEAT_RUN_MS ((uint16_t)15000)
#define RMT_PAIRING_MS ((uint16_t)5000)
#define RMT_RESET_GUARD_MS ((uint16_t)7000)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  RMT_CMD_PING = 0,
  RMT_CMD_ALARM,
  RMT_CMD_SEAT,
  RMT_CMD_ANTITHIEF
} RMT_CMD;

typedef enum { RMT_ACTION_R = 0, RMT_ACTION_W } RMT_ACTION;

typedef enum { RMT_MODE_NORMAL = 0, RMT_MODE_PAIRING } RMT_MODE;

/* Exported structs
 * --------------------------------------------*/
typedef struct {
  uint32_t ping;
  uint32_t heartbeat;
  uint32_t pairing;
  uint32_t rx;
} remote_tick_t;

typedef struct {
  uint8_t tx;
  uint8_t rx;
  uint8_t full;
} remote_duration_t;

typedef struct {
  uint8_t active;
  uint8_t nearby;
  uint32_t pairing_aes[4];
  remote_tick_t tick;
  remote_duration_t duration;
} remote_data_t;

typedef struct {
  uint8_t address[NRF_ADDR_LENGTH];
  uint8_t payload[NRF_DATA_PAIR_LENGTH];
} remote_packet_t;

typedef struct {
  remote_data_t d;
  remote_packet_t t;
  remote_packet_t r;
  SPI_HandleTypeDef *pspi;
} remote_t;

/* Exported variables
 * --------------------------------------------*/
extern remote_t RMT;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t RMT_Init(void);
uint8_t RMT_ReInit(void);
void RMT_DeInit(void);
uint8_t RMT_Probe(void);
void RMT_Flush(void);
void RMT_Refresh(vehicle_state_t state);
uint8_t RMT_Ping(vehicle_state_t state);
uint8_t RMT_Pairing(void);
uint8_t RMT_GotPairedResponse(void);
uint8_t RMT_ValidateCommand(RMT_CMD *cmd);
void RMT_IrqHandler(void);

#endif /* REMOTE_H_ */
