/*
 * simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef SIMCOM_H_
#define SIMCOM_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define SIMCOM_RSP_NONE                 "\r\n"
#define SIMCOM_RSP_SEND 				">"
#define SIMCOM_RSP_SENT					"SEND OK\r"
#define SIMCOM_RSP_OK 					"OK\r"
#define SIMCOM_RSP_ERROR 				"ERROR"
#define SIMCOM_RSP_READY 				"RDY"
#define SIMCOM_RSP_IPD                  "+IPD,"
#define SIMCOM_CMD_BOOT 				"AT\r"
#define SIMCOM_DEBUG                    1
#define SIMCOM_MAX_UPLOAD_RETRY         3

#define MAX_ENUM_SIZE                   0xFFFFFFFF

/* Exported enum -------------------------------------------------------------*/
typedef enum {
    SIM_RESULT_NACK = -4,
    SIM_RESULT_RESTARTED = -3,
    SIM_RESULT_NO_RESPONSE = -2,
    SIM_RESULT_TIMEOUT = -1,
    SIM_RESULT_ERROR = 0,
    SIM_RESULT_OK = 1,
    SIM_RESULT_ACK = 2,
} SIMCOM_RESULT;

typedef enum {
    SIM_STATE_DOWN = -1,
    SIM_STATE_READY = 0,
    SIM_STATE_CONFIGURED = 1,
    SIM_STATE_NETWORK_ON = 2,
    SIM_STATE_GPRS_ON = 3,
    SIM_STATE_PDP_ON = 4,
    SIM_STATE_INTERNET_ON = 5,
    SIM_STATE_SERVER_ON = 6
} SIMCOM_STATE;

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

/* Exported struct -----------------------------------------------------------*/
typedef struct {
    SIMCOM_STATE state;
    AT_CIPSTATUS ip_status;
    uint8_t signal;
} sim_t;

/* Public functions prototype ------------------------------------------------*/
uint8_t Simcom_SetState(SIMCOM_STATE state);
void Simcom_Init(void);
char* Simcom_Response(char *str);
SIMCOM_RESULT Simcom_Upload(void *payload, uint16_t size);
SIMCOM_RESULT Simcom_FOTA(void);
SIMCOM_RESULT Simcom_Command(char *data, char *res, uint32_t ms, uint16_t size);
SIMCOM_RESULT Simcom_IdleJob(uint8_t *iteration);
void Simcom_Lock(void);
void Simcom_Unlock(void);

#endif /* SIMCOM_H_ */
