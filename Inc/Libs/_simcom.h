/*
 * simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef SIMCOM_H_
#define SIMCOM_H_

#include "_DMA_Simcom.h"
#include "_reporter.h"
#include "_crc.h"

#define SIMCOM_STATUS_SEND 					">"
#define SIMCOM_STATUS_SENT					"SEND OK\r\n"
#define SIMCOM_STATUS_OK 					"OK\r\n"
#define SIMCOM_STATUS_ERROR 				"ERROR\r\n"
#define SIMCOM_STATUS_READY 				"RDY\r"
#define SIMCOM_BOOT_COMMAND					"AT\r"
#define SIMCOM_RESPONSE_IPD					"+IPD,"

/* Public typedef -----------------------------------------------------------*/
typedef struct {
  char CMD_CIPSTART[100];
  char CMD_CSTT[75];
  char CMD_CNMP[12];
} simcom_t;

typedef struct {
  char name[10];
  int8_t minValue;
  uint8_t linMinValue;
  uint8_t percentage;
} rssi_t;

/* Public functions ---------------------------------------------------------*/
void Simcom_Init(void);
uint8_t Simcom_Command(char *cmd, uint32_t ms, char *res, uint8_t n);
uint8_t Simcom_Upload(char *message, uint16_t length);
uint8_t Simcom_Read_ACK(report_header_t *report_header);
uint8_t Simcom_Read_Command(command_t *command);
uint8_t Simcom_Read_Signal(uint8_t *signal_percentage);
uint8_t Simcom_Read_Carrier_Time(timestamp_t *timestamp);

#endif /* SIMCOM_H_ */
