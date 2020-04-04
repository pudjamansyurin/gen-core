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
#define SIMCOM_STATUS_SEND 					">"
#define SIMCOM_STATUS_SENT					"SEND OK\r\n"
#define SIMCOM_STATUS_OK 					"OK\r\n"
#define SIMCOM_STATUS_ERROR 				"ERROR"
#define SIMCOM_STATUS_READY 				"RDY"
#define SIMCOM_BOOT_COMMAND					"AT\r"
#define SIMCOM_RESPONSE_IPD					"+IPD,"

/* Exported enum -------------------------------------------------------------*/
typedef enum {
  SIMCOM_RESTART = 0,
  SIMCOM_POWER_UP = 1
} SIMCOM_PWR;

/* Exported struct -----------------------------------------------------------*/
typedef struct {
  uint8_t ready;
  uint8_t online;
  struct {
    char CIPSTART[100];
    char CSTT[75];
    char CNMP[12];
  } cmd;
} simcom_t;

typedef struct {
  char name[10];
  int8_t minValue;
  uint8_t linMinValue;
  uint8_t percentage;
} rssi_t;

/* Public functions prototype ------------------------------------------------*/
void Simcom_Init(SIMCOM_PWR state);
uint8_t Simcom_Command(char *cmd, uint32_t ms, char *res, uint8_t n);
uint8_t Simcom_Upload(char *message, uint16_t length);
uint8_t Simcom_ReadACK(report_header_t *report_header);
uint8_t Simcom_ReadCommand(command_t *command);
uint8_t Simcom_ReadSignal(uint8_t *signal_percentage);
uint8_t Simcom_ReadTime(timestamp_t *timestamp);

#endif /* SIMCOM_H_ */
