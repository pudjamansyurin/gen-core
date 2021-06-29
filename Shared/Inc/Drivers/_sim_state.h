/*
 * _sim_state.h
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */

#ifndef INC_DRIVERS__SIM_STATE_H_
#define INC_DRIVERS__SIM_STATE_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported constants
 * --------------------------------------------*/
#define SIM_CMD_BOOT "AT\r"
#define SIM_CMD_FTPGET "AT+FTPGET=2"

#define SIM_RSP_NONE "\r\n"
#define SIM_RSP_SEND ">"
//#define SIM_RSP_SENT                 "SEND OK\r"
#define SIM_RSP_ACCEPT "DATA ACCEPT:"
#define SIM_RSP_OK "OK\r"
#define SIM_RSP_ERROR "ERROR"
#define SIM_RSP_READY "RDY"
#define SIM_RSP_IPD "+IPD,"

#define MAX_ENUM_SIZE (uint32_t)0xFFFFFFFF

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  SIM_RESTARTED = -3,
  SIM_NORESPONSE = -2,
  SIM_TIMEOUT = -1,
  SIM_ERROR = 0,
  SIM_OK = 1,
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
  CIPSTAT_UNKNOWN = -1,
  CIPSTAT_IP_INITIAL = 0,
  CIPSTAT_IP_START,
  CIPSTAT_IP_CONFIG,
  CIPSTAT_IP_GPRSACT,
  CIPSTAT_IP_STATUS,
  CIPSTAT_CONNECTING,
  CIPSTAT_CONNECT_OK,
  CIPSTAT_CLOSING,
  CIPSTAT_CLOSED,
  CIPSTAT_PDP_DEACT,
  CIPSTAT_ForceEnumSize = MAX_ENUM_SIZE
} AT_CIPSTATUS;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t SIMSta_StateTimeout(uint32_t* tick, uint32_t timeout, SIMR res);
uint8_t SIMSta_StateLockedLoop(SIM_STATE* lastState, uint8_t* retry);
uint8_t SIMSta_StatePoorSignal(void);

void SIMSta_Down(SIMR* res);
void SIMSta_Ready(SIMR* res);
void SIMSta_Configured(SIMR* res, uint32_t tick, uint32_t timeout);
void SIMSta_NetworkOn(SIMR* res, uint32_t tick, uint32_t timeout);
void SIMSta_NetworkOn(SIMR* res, uint32_t tick, uint32_t timeout);
void SIMSta_GprsOn(SIMR* res, uint32_t tick, uint32_t timeout);
#if (!APP)
void SIMSta_PdpOn(SIMR* res);
#else
void SIMSta_PdpOn(SIMR* res);
void SIMSta_InternetOn(SIMR* res, uint32_t tick, uint32_t timeout);
void SIMSta_ServerOn(void);
void SIMSta_MqttOn(void);
#endif

#endif /* INC_DRIVERS__SIM_STATE_H_ */
