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
#define NET_BOOT_MS ((uint16_t)8000)
#define NET_GUARD_MS ((uint16_t)1000)

#define NET_CON_APN "3gprs"
#define NET_CON_USERNAME "3gprs"
#define NET_CON_PASSWORD "3gprs"

//#define NET_CON_APN                             "telkomsel"
//#define NET_CON_USERNAME                        "wap"
//#define NET_CON_PASSWORD                        "wap123"
//#define NET_CON_APN                             "indosatgprs"
//#define NET_CON_USERNAME                        "indosat"
//#define NET_CON_PASSWORD                        "indosat"

#define NET_FTP_SERVER "ftp.genmotorcycles.com"
#define NET_FTP_USERNAME "fota@genmotorcycles.com"
#define NET_FTP_PASSWORD "@Garda313"

//#define NET_TCP_SERVER                          "pujakusumae-30856.portmap.io"
//#define NET_TCP_PORT                 ((uint16_t)46606)
//#define NET_TCP_SERVER                          "mqtt.eclipseprojects.io"
//#define NET_TCP_PORT                 ((uint16_t)1883)
#define NET_TCP_SERVER "test.mosquitto.org"
#define NET_TCP_PORT ((uint16_t)1883)

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

/* Exported enum -------------------------------------------------------------*/
typedef enum {
	SIM_RESTARTED = -3,
	SIM_NORESPONSE = -2,
	SIM_TIMEOUT = -1,
	SIM_ERROR = 0,
	SIM_OK = 1,
} SIM_RESULT;

typedef enum {
	SIM_STATE_DOWN = -1,
	SIM_STATE_READY = 0,
	SIM_STATE_CONFIGURED = 1,
	SIM_STATE_NETWORK_ON = 2,
	SIM_STATE_GPRS_ON = 3,
	SIM_STATE_PDP_ON = 4,
#if (BOOTLOADER)
	SIM_STATE_BEARER_ON = 5,
#else
	SIM_STATE_INTERNET_ON = 5,
	SIM_STATE_SERVER_ON = 6,
	SIM_STATE_MQTT_ON = 7
#endif
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

/* Struct -------------------------------------------------------------------*/
typedef struct {
	uint8_t signal;
	SIMCOM_STATE state;
	AT_CIPSTATUS ipstatus;
	char *response;
} sim_data_t;

typedef struct {
	sim_data_t d;
	UART_HandleTypeDef *puart;
	DMA_HandleTypeDef *pdma;
} sim_t;

/* Exported variables --------------------------------------------------------*/
extern sim_t SIM;

/* Public functions prototype ------------------------------------------------*/
void Simcom_Init(void);
void Simcom_DeInit(void);
void Simcom_Lock(void);
void Simcom_Unlock(void);
uint8_t Simcom_SetState(SIMCOM_STATE state, uint32_t timeout);
char *Simcom_Resp(char *keyword, char *from);
SIM_RESULT Simcom_Cmd(char *command, char *reply, uint32_t ms);
#if (!BOOTLOADER)
void Simcom_CalibrateTime(void);
uint8_t Simcom_SendUSSD(char *ussd, char *buf, uint8_t buflen);
uint8_t Simcom_ReadNewSMS(char *buf, uint8_t buflen);
// uint8_t Simcom_CheckQuota(char *buf, uint8_t buflen);
uint8_t Simcom_Upload(void *payload, uint16_t size);
int Simcom_GetData(unsigned char *buf, int count);
uint8_t Simcom_ReceivedResponse(uint32_t timeout);
#endif

#endif /* SIMCOM_H_ */
