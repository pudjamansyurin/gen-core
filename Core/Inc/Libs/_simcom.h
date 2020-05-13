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
#include "_reporter.h"

/* Exported constants --------------------------------------------------------*/
#define SIMCOM_RSP_NONE                 "\r\n"
#define SIMCOM_RSP_SEND 								">"
#define SIMCOM_RSP_SENT									"SEND OK\r"
#define SIMCOM_RSP_OK 									"OK\r"
#define SIMCOM_RSP_ERROR 								"ERROR"
#define SIMCOM_RSP_READY 								"RDY"
#define SIMCOM_RSP_IPD                  "+IPD,"
#define SIMCOM_CMD_BOOT 								"AT\r"

#define SIMCOM_MAX_UPLOAD_RETRY         3

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

/* Exported struct -----------------------------------------------------------*/
typedef struct {
	SIMCOM_STATE state;
	uint8_t uploading;
} sim_t;

/* Public functions prototype ------------------------------------------------*/
void Simcom_Sleep(uint8_t state);
void Simcom_SetState(SIMCOM_STATE state);
SIMCOM_RESULT Simcom_Response(char *str);
SIMCOM_RESULT Simcom_Upload(void *payload, uint16_t size, uint8_t *retry);
SIMCOM_RESULT Simcom_Cmd(char *cmd, uint32_t ms, uint8_t n);
SIMCOM_RESULT Simcom_Command(char *cmd, uint32_t ms, uint8_t n, char *res);
SIMCOM_RESULT Simcom_ProcessCommand(command_t *command);
SIMCOM_RESULT Simcom_ProcessACK(header_t *header);
void Simcom_Lock(void);
void Simcom_Unlock(void);

#endif /* SIMCOM_H_ */
