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
#define NRF_DATA_LENGTH											17										// Max: 32 bytes
#define NRF_ADDR_LENGTH											5
#define NRF_ADDRRESS_WIDTH							NRF_ADDR_WIDTH_5

/* Exported enum -------------------------------------------------------------*/
typedef enum {
  KLESS_CMD_PING = 1,
  KLESS_CMD_ALARM = 2,
  KLESS_CMD_SEAT = 3,
  KLESS_CMD_MAX = 3
} KLESS_CMD;

typedef enum {
  KLESS_R = 0, KLESS_W = 1
} KLESS_MODE;

/* Exported struct -----------------------------------------------------------*/
typedef struct {
  uint8_t tx[NRF_ADDR_LENGTH];
  uint8_t rx[NRF_ADDR_LENGTH];
} kless_address_t;

typedef struct {
  uint8_t tx[NRF_DATA_LENGTH];
  uint8_t rx[NRF_DATA_LENGTH];
} kless_payload_t;

typedef struct {
  kless_address_t address;
  kless_payload_t payload;
} kless_t;

/* Public functions prototype ------------------------------------------------*/
void KLESS_Init(void);
void KLESS_Debugger(void);
uint8_t KLESS_ValidateCommand(KLESS_CMD *cmd);
uint8_t KLESS_UsePayload(KLESS_MODE mode, uint8_t *payload);
uint8_t KLESS_SendDummy(void);
void KLESS_Refresh(void);
void KLESS_IrqHandler(void);

#endif /* KEYLESS_H_ */
