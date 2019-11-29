/*
 * simcom.h
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

#ifndef SIMCOM_H_
#define SIMCOM_H_

#include "main.h"
#include "cmsis_os.h"

#include <stdio.h>							// for: sprintf()
#include "_config.h"
#include "_reporter.h"
#include "_DMA_Simcom.h"

#define SIMCOM_STATUS_SEND 					"\r\n>"
#define SIMCOM_STATUS_CIPSEND				"\r\n+CIPSEND:"
#define SIMCOM_STATUS_OK 						"OK\r\n"
#define SIMCOM_STATUS_ERROR 				"ERROR\r\n"
#define SIMCOM_STATUS_READY 				"RDY\r\n"
#define SIMCOM_STATUS_RESTARTED			"START\r\n"

#define SIMCOM_MESSAGE_END					"\x1A"

/* Public typedef -----------------------------------------------------------*/
typedef enum {
	//	SIGNAL_AUTO = 2,
	SIGNAL_2G = 13,
	SIGNAL_3G = 14,
} signal_t;

typedef struct {
	signal_t signal;
	char server_ip[16];
	uint16_t server_port;
	uint16_t local_port;
	char network_apn[20];
	char network_username[20];
	char network_password[20];
	uint8_t boot_timeout;
	uint8_t repeat_delay;
} simcom_t;

typedef struct {
	char var[20];
	char val[20];
	char cmd[40];
} command_t;

/* Public functions ---------------------------------------------------------*/
void Ublox_Init(gps_t *hgps);
void Simcom_Init(uint8_t skipFirstBoot);
uint8_t Simcom_Send_Payload(void);
uint8_t Simcom_Check_Command(void);
uint8_t Simcom_Get_Command(command_t *command);
uint8_t Simcom_To_Server(char *message, uint16_t length);
uint8_t Simcom_Signal_Locked(uint8_t n);
uint8_t Simcom_Set_Signal(signal_t signal);

#endif /* SIMCOM_H_ */
