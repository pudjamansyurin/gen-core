/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_simcom.h"

#include "Libs/_at.h"
#include "usart.h"

#if (APP)
#include "Libs/_mqtt.h"
#include "Nodes/VCU.h"

#else
#include "Drivers/_iwdg.h"
#endif

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t SimcomRecMutexHandle;
#endif

/* Private constants
 * --------------------------------------------*/
#define NET_GUARD_MS ((uint16_t)1000)

/* Private types
 * --------------------------------------------*/
typedef struct {
	char* response;
	UART_HandleTypeDef* puart;
	DMA_HandleTypeDef* pdma;
} sim_t;

/* Private variables
 * --------------------------------------------*/
static sim_t SIM = {
		.response = NULL,
		.puart = &huart1,
		.pdma = &hdma_usart1_rx
};

/* Private functions prototype
 * --------------------------------------------*/
static SIMR CmdRaw(const char* data, uint16_t size, char* reply, uint32_t ms);
static SIMR TransmitCmd(const char* data, uint16_t size, uint32_t ms,
		char* reply);
#if (APP)
static uint8_t ProcessResponse(void);
#endif

/* Public functions implementation
 * --------------------------------------------*/
void SIM_Lock(void) {
#if (APP)
	osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
#endif
}

void SIM_Unlock(void) {
#if (APP)
	osMutexRelease(SimcomRecMutexHandle);
#endif
}

void SIM_Init(void) {
	SIMCon_Init();
	MX_USART1_UART_Init();
	SIM_DMA_Start(SIM.puart, SIM.pdma);
}

void SIM_DeInit(void) {
	GATE_SimcomShutdown();
	SIM_DMA_Stop();
	HAL_UART_DeInit(SIM.puart);
}

uint8_t SIM_SetState(SIM_STATE state, uint32_t timeout) {
	SIM_STATE lastState = SIM_STATE_DOWN;
	SIMR res = SIM_ERROR;
	uint32_t tick = _GetTickMS();
	uint8_t retry = 0;

	SIM_Lock();
	do {
		if (SIMSta_StateTimeout(res, &tick, timeout)) break;
		if (SIMSta_StateLockedLoop(&lastState, &retry)) break;
		if (SIMSta_StatePoorSignal()) break;

		// Set value
		res = SIM_OK;

		// Handle states
		switch (SIMSta_IO_State()) {
			case SIM_STATE_DOWN:
				SIMSta_Down(&res);
				break;

			case SIM_STATE_READY:
				SIMSta_Ready(&res);
				break;

			case SIM_STATE_CONFIGURED:
				SIMSta_Configured(&res, tick, timeout);
				break;

			case SIM_STATE_NETWORK_ON:
				SIMSta_NetworkOn(&res, tick, timeout);
				break;

			case SIM_STATE_GPRS_ON:
				SIMSta_GprsOn(&res, tick, timeout);
				break;

#if (!APP)
			case SIM_STATE_PDP_ON:
				SIMSta_PdpOn(&res);
				break;
			case SIM_STATE_BEARER_ON:
				/*nothing*/
				break;
#else

			case SIM_STATE_PDP_ON:
				SIMSta_PdpOn(&res);
				break;

			case SIM_STATE_INTERNET_ON:
				SIMSta_InternetOn(&res, tick, timeout);
				break;

			case SIM_STATE_SERVER_ON:
				SIMSta_ServerOn();
				break;

			case SIM_STATE_MQTT_ON:
				SIMSta_MqttOn();
				break;
#endif

			default:
				break;
		}
		_DelayMS(500);
	} while (SIMSta_IO_State() < state);
	SIM_Unlock();

	return (SIMSta_IO_State() >= state);
}

SIMR SIM_Cmd(const char* command, char* reply, uint32_t ms) {
	return CmdRaw(command, strlen(command), reply, ms);
}

char* SIM_Resp(const char* keyword, const char* from) {
	const char* start = NULL;
	char* stop = &SIM_UART_RX[SIM_UART_RX_SZ];

	if (from != NULL)
		if (from >= start && from <= stop)
			start = from;

	if (start == NULL)
		start = &SIM_UART_RX[0];

#if (!APP)
	IWDG_Refresh();
#endif

	return strnstr(start, keyword, stop - start);
}

#if (APP)
uint8_t SIM_FetchTime(timestamp_t* ts) {
	uint8_t ok = 0;

	if (SIM_SetState(SIM_STATE_READY, 0))
		if (AT_Clock(ATR, ts) == SIM_OK)
			ok = (ts->date.Year >= VCU_BUILD_YEAR);

	return ok;
}

uint8_t SIM_SendUSSD(const char* ussd, char* buf, uint8_t buflen) {
	SIMR res;

	if (!SIM_SetState(SIM_STATE_NETWORK_ON, 0)) return 0;

	// Set TE Character
	{
		char chset[5] = "GSM";
		res = AT_CharacterSetTE(ATW, chset, sizeof(chset));
	}
	// Delete all message
	{
		at_cmgd_t param = {.index = 0, .delflag = CMGD_ALL};
		res = AT_DeleteMessageSMS(&param);
	}
	// Dial USSD (check quota)
	if (res == SIM_OK) {
		at_cusd_t param = {.n = CUSD_ENABLE, .dcs = 15};
		strncpy(param.str, ussd, sizeof(param.str) - 1);
		res = AT_ServiceDataUSSD(ATW, &param, buf, buflen);
	}

	return res == SIM_OK;
}

uint8_t SIM_ReadLastSMS(char* buf, uint8_t buflen) {
	SIMR res;

	if (!SIM_SetState(SIM_STATE_NETWORK_ON, 0)) return 0;

	// Set SMS format
	{
		AT_CMGF param = CMGF_TEXT;
		res = AT_MessageFormatSMS(ATW, &param);
	}
	// Read SMS response
	if (res == SIM_OK) {
		at_cmgr_t par = {.index = 1, .mode = CMG_MODE_NORMAL};
		res = AT_ReadMessageSMS(&par, buf, buflen);
	}

	return res == SIM_OK;
}

uint8_t SIM_Upload(const void* payload, uint16_t size) {
	SIMR res;
	char cmd[20];

	if (SIMSta_IO_Ip() != SIM_IP_CONNECT_OK && SIMSta_IO_State() < SIM_STATE_SERVER_ON)
		return 0;

	SIM_Lock();
	sprintf(cmd, "AT+CIPSEND=%d\r", size);
	res = SIM_Cmd(cmd, SIM_RSP_SEND, 500);

	if (res == SIM_OK)
		res = CmdRaw((char*)payload, size, SIM_RSP_ACCEPT, 30000);
	SIM_Unlock();

	return res == SIM_OK;
}

int SIM_GetData(unsigned char* buf, int count) {
	if (SIM.response == NULL) return -1;
	if (SIM.response < &SIM_UART_RX[0]) return -1;
	if ((SIM.response + count) > &SIM_UART_RX[SIM_UART_RX_SZ]) return -1;

	memcpy(buf, SIM.response, count);
	SIM.response += count;

	return count;
}

uint8_t SIM_GotResponse(uint32_t timeout) {
	TickType_t tick;
	char* ptr = NULL;

	tick = _GetTickMS();
	do {
		ptr = SIM_Resp(SIM_RSP_IPD, NULL);
		_DelayMS(10);
	} while (ptr == NULL && _TickIn(tick, timeout));

	if (ptr != NULL)
		if ((ptr = SIM_Resp(":", ptr)) != NULL) SIM.response = ptr + 1;

	return (ptr != NULL);
}
#endif

/* Private functions implementation
 * --------------------------------------------*/
static SIMR CmdRaw(const char* data, uint16_t size, char* reply, uint32_t ms) {
	SIMR res = SIM_ERROR;

	// only handle command if SIM_STATE_READY or BOOT_CMD
	if (!(SIMSta_IO_State() >= SIM_STATE_READY ||
			(strncmp(data, SIM_CMD_BOOT, size) == 0)))
		return res;

	// Debug: print payload
	if (SIM_DEBUG) {
		if (strcmp(reply, SIM_RSP_ACCEPT) == 0)
			printf_hex(data, size);
		else {
			printf("\n=> ");
			printf("%.*s", size, data);
		}
		printf("\n");
	}

	// execute payload
	SIM_Lock();
	GATE_SimcomSleep(0);
	res = TransmitCmd(data, size, ms, reply);
	GATE_SimcomSleep(1);
	SIM_Unlock();

	// Debug: print response (ignore FTPGET command)
	if (SIM_DEBUG)
		if (strnstr(data, SIM_CMD_FTPGET, size) == NULL)
			printf("%.*s\n", sizeof(SIM_UART_RX), SIM_UART_RX);

	return res;
}

static SIMR TransmitCmd(const char* data, uint16_t size, uint32_t ms,
		char* reply) {
	SIMR res = SIM_ERROR;
	uint32_t tick;

#if (APP)
	if (SIM_GotResponse(0)) ProcessResponse();
#endif
	SIM_Transmit(data, size);

	// wait response from SIMCOM
	ms += NET_GUARD_MS;
	tick = _GetTickMS();
	while (1) {
		if (SIM_Resp(reply, NULL) || SIM_Resp(SIM_RSP_ERROR, NULL) ||
				SIM_Resp(SIM_RSP_READY, NULL)
#if (APP)
				|| SIM_Resp(SIM_RSP_IPD, NULL)
#endif
				|| _TickOut(tick, ms)) {

			// check response
			if (SIM_Resp(reply, NULL)) res = SIM_OK;

			// Handle failure
			else {
				// exception for accidentally reboot
				if (SIM_Resp(SIM_RSP_READY, NULL) && (SIMSta_IO_State() >= SIM_STATE_READY)) {
					printf("Simcom:Restarted\n");
					res = SIM_RESTARTED;
					SIMSta_IO_SetState(SIM_STATE_READY);
#if (APP)
					EVT_Set(EVG_NET_SOFT_RESET);
#endif
				}
#if (APP)
				// exception for server command collision
				else if (strcmp(reply, SIM_RSP_ACCEPT) != 0) {
					if (SIM_GotResponse(0)) {
						if (ProcessResponse()) {
							printf("Simcom:CommandCollision\n");
							res = SIM_TIMEOUT;
						}
					}
				}
#endif
				// exception for no response
				else if (strnlen(SIM_UART_RX, sizeof(SIM_UART_RX)) == 0) {
					printf("Simcom:NoResponse\n");
					res = SIM_NORESPONSE;
					// SIMSta_IO_SetState(SIM_STATE_DOWN);
				}

				// exception for timeout
				else if (_TickOut(tick, ms)) {
					printf("Simcom:Timeout\n");
					res = SIM_TIMEOUT;
				}

				else
					printf("Simcom:UnknownError\n");
			}

			// exit loop
			break;
		}

		_DelayMS(10);
	}

	return res;
}

#if (APP)
static uint8_t ProcessResponse(void) {
	char* ptr = SIM.response;

	_DelayMS(1000);
	if (!MQTT_GotPublish()) {
		SIM.response = ptr;
		return 0;
	}
	return 1;
}
#endif

