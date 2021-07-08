/*
 * _.h
 *
 *  Created on: Mar 4, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__REMOTE_H_
#define INC_LIBS__REMOTE_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_aes.h"
#include "Drivers/_nrf24l01.h"

/* Exported constants
 * --------------------------------------------*/
#define RMT_RESET_GUARD_MS ((uint16_t)7000)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  RMT_TICK_PING,
  RMT_TICK_HBEAT,
  RMT_TICK_PAIR,
  RMT_TICK_RX,
  RMT_TICK_MAX,
} RMT_TICK;

typedef enum {
  RMT_DUR_TX,
  RMT_DUR_RX,
  RMT_DUR_FULL,
  RMT_DUR_MAX,
} RMT_DURATION;

typedef enum {
  RMT_CMD_PING = 0,
  RMT_CMD_ALARM,
  RMT_CMD_SEAT,
  RMT_CMD_ANTITHIEF,
	RMT_CMD_MAX,
} RMT_CMD;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t RMT_Init(void);
uint8_t RMT_ReInit(void);
void RMT_DeInit(void);
uint8_t RMT_Probe(void);
void RMT_Flush(void);
void RMT_Refresh(vehicle_t vehicle);
uint8_t RMT_Ping(vehicle_t vehicle);
uint8_t RMT_Pairing(void);
uint8_t RMT_GotPairedResponse(void);
uint8_t RMT_ValidateCommand(RMT_CMD *cmd);
void RMT_IrqHandler(void);

uint8_t RMT_IO_GetActive(void);
uint8_t RMT_IO_GetNearby(void);
uint32_t RMT_IO_GetTick(RMT_TICK key);
uint8_t RMT_IO_GetDuration(RMT_DURATION key);
void RMT_IO_SetTick(RMT_TICK key);
void RMT_IO_SetDuration(RMT_DURATION key, uint32_t tick);
#endif /* INC_LIBS__REMOTE_H_ */
