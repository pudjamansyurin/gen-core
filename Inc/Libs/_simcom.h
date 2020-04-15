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
#define SIMCOM_RSP_SENT					"SEND OK\r\n"
#define SIMCOM_RSP_OK 					"OK\r\n"
#define SIMCOM_RSP_ERROR 				"ERROR"
#define SIMCOM_RSP_READY 				"RDY"
#define SIMCOM_RSP_IPD                  "+IPD,"
#define SIMCOM_CMD_BOOT 				"AT\r"

/* Exported enum -------------------------------------------------------------*/
typedef enum {
  SIMCOM_RESTART = 0,
  SIMCOM_POWER_UP = 1
} SIMCOM_PWR;

typedef enum {
  SIMCOM_R_NACK = -4,
  SIMCOM_R_RESTARTED = -3,
  SIMCOM_R_NO_RESPONSE = -2,
  SIMCOM_R_TIMEOUT = -1,
  SIMCOM_R_ERROR = 0,
  SIMCOM_R_OK = 1,
  SIMCOM_R_ACK = 2,
} SIMCOM_RESULT;

/* Exported struct -----------------------------------------------------------*/
typedef struct {
  uint8_t ready;
  uint8_t online;
  struct {
    char CIPSTART[200];
    char CSTT[75];
  } cmd;
} simcom_t;

typedef struct {
  char name[10];
  uint8_t min;
  uint8_t percentage;
} rssi_t;

/* Public functions prototype ------------------------------------------------*/
void Simcom_Sleep(uint8_t state);
void Simcom_Init(SIMCOM_PWR state);
SIMCOM_RESULT Simcom_Upload(char *message, uint16_t length);
SIMCOM_RESULT Simcom_ReadCommand(command_t *command);
SIMCOM_RESULT Simcom_ReadACK(report_header_t *report_header);
SIMCOM_RESULT Simcom_ReadSignal(uint8_t *signal_percentage);
SIMCOM_RESULT Simcom_ReadTime(timestamp_t *timestamp);

#endif /* SIMCOM_H_ */
