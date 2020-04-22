/*
 * simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef SIMCOM_H_
#define SIMCOM_H_

/* Includes ------------------------------------------------------------------*/
#include "_dma_simcom.h"
#include "_reporter.h"
#include "_crc.h"

/* Exported constants --------------------------------------------------------*/
#define SIMCOM_RSP_NONE                 "\r\n"
#define SIMCOM_RSP_SEND 				">"
#define SIMCOM_RSP_SENT					"SEND OK\r"
#define SIMCOM_RSP_OK 					"OK\r"
#define SIMCOM_RSP_ERROR 				"ERROR"
#define SIMCOM_RSP_READY 				"RDY"
#define SIMCOM_RSP_IPD                  "+IPD,"
#define SIMCOM_CMD_BOOT 				"AT\r"

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
  uint8_t sleep;
  struct {
    char CIPSTART[200];
    char CSTT[75];
  } cmd;
} sim_t;

typedef struct {
  char name[10];
  uint8_t min;
  uint8_t percent;
} rssi_t;

typedef struct {
  struct {
    rssi_t list;
    uint8_t value;
  } rssi;
  uint8_t ber;
} signal_t;

/* Public functions prototype ------------------------------------------------*/
void Simcom_Sleep(uint8_t state);
void Simcom_SetState(SIMCOM_STATE state);
SIMCOM_RESULT Simcom_Upload(char *message, uint16_t length);
SIMCOM_RESULT Simcom_Cmd(char *cmd, uint32_t ms, uint8_t n);
SIMCOM_RESULT Simcom_ProcessCommand(command_t *command);
SIMCOM_RESULT Simcom_ProcessACK(report_header_t *report_header);

SIMCOM_RESULT SIM_SignalQuality(uint8_t *percent);
SIMCOM_RESULT SIM_Clock(timestamp_t *timestamp);

#endif /* SIMCOM_H_ */
