/*
 * sim_state.h
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */

#ifndef INC_DRIVERS__SIM_STATE_H_
#define INC_DRIVERS__SIM_STATE_H_

/* Includes
 * --------------------------------------------*/
#include "App/common.h"

/* Exported constants
 * --------------------------------------------*/
#define SIM_CMD_BOOT "AT\r"
#define SIM_CMD_FTPGET "AT+FTPGET=2"

#define SIM_RSP_NONE "\r\n"
#define SIM_RSP_SEND ">"
#define SIM_RSP_ACCEPT "DATA ACCEPT:"
#define SIM_RSP_OK "OK\r"
#define SIM_RSP_ERROR "ERROR"
#define SIM_RSP_READY "RDY"
#define SIM_RSP_IPD "+IPD,"

#define MAX_ENUM_SIZE ((uint32_t)0xFFFFFFFF)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  SIM_RESTARTED = -3,
  SIM_NORESPONSE,
  SIM_TIMEOUT,
  SIM_ERROR = 0,
  SIM_OK,
} SIMR;

typedef enum {
  SIM_STATE_DOWN = -1,
  SIM_STATE_READY = 0,
  SIM_STATE_CONFIGURED,
  SIM_STATE_NETWORK_ON,
  SIM_STATE_GPRS_ON,
  SIM_STATE_PDP_ON,
#if (!APP)
  SIM_STATE_BEARER_ON,
#else
  SIM_STATE_INTERNET_ON,
  SIM_STATE_SERVER_ON,
  SIM_STATE_MQTT_ON
#endif
} SIM_STATE;

typedef enum {
  SIM_IP_UNKNOWN = -1,
  SIM_IP_INITIAL = 0,
  SIM_IP_START,
  SIM_IP_CONFIG,
  SIM_IP_GPRSACT,
  SIM_IP_STATUS,
  SIM_IP_CONNECTING,
  SIM_IP_CONNECT_OK,
  SIM_IP_CLOSING,
  SIM_IP_CLOSED,
  SIM_IP_PDP_DEACT,
  SIM_IP_ForceEnumSize = MAX_ENUM_SIZE
} SIM_IP;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t SIMSta_SetState(SIM_STATE state, uint32_t timeout);

SIM_IP SIMSta_IO_Ip(void);
uint8_t SIMSta_IO_Signal(void);
SIM_STATE SIMSta_IO_State(void);
void SIMSta_IO_SetState(SIM_STATE value);
#endif /* INC_DRIVERS__SIM_STATE_H_ */
