/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "DMA/_dma_simcom.h"
#include "Drivers/_simcom.h"
#include "Libs/_at.h"
#if (!BOOTLOADER)
#include "Libs/_reporter.h"
#include "Libs/_mqtt.h"
#include "Nodes/VCU.h"
#else
#include "iwdg.h"
#include "Drivers/_flasher.h"
#include "Libs/_eeprom.h"
#include "Libs/_fota.h"
#include "Libs/_focan.h"
#endif

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMutexId_t SimcomRecMutexHandle;
#endif

/* Public variables ---------------------------------------------------------*/
sim_t SIM = {
		.state = SIM_STATE_DOWN,
		.ipstatus = CIPSTAT_UNKNOWN,
		.signal = 0,
		.downloading = 0,
		.response = NULL,
		.puart = &huart1,
		.pdma = &hdma_usart1_rx
};

/* Private functions prototype -----------------------------------------------*/
static SIM_RESULT PowerUp(void);
static SIM_RESULT Reset(uint8_t hard);
static void Simcom_IdleJob(void);
static SIM_RESULT Simcom_CmdRaw(char *data, uint16_t size, char *reply, uint32_t ms);
static SIM_RESULT TransmitCmd(char *data, uint16_t size, uint32_t ms, char *reply);
#if (!BOOTLOADER)
static uint8_t Simcom_ProcessResponse(void);
#endif

static uint8_t TimeoutReached(uint32_t tick, uint32_t timeout, uint32_t delay);
static uint8_t StateTimeout(uint32_t *tick, uint32_t timeout, SIM_RESULT res);
static uint8_t StateLockedLoop(SIMCOM_STATE *lastState, uint8_t *retry);
static uint8_t StatePoorSignal(void);
static void NetworkRegistration(char *type, SIM_RESULT *res, uint32_t tick, uint32_t timeout);
static void SetStateDown(SIM_RESULT *res);
static void SetStateReady(SIM_RESULT *res);
static void SetStateConfigured(SIM_RESULT *res, uint32_t tick, uint32_t timeout);
static void SetStateNetworkOn(SIM_RESULT *res, uint32_t tick, uint32_t timeout);
static void SetStateNetworkOn(SIM_RESULT *res, uint32_t tick, uint32_t timeout);
static void SetStateGprsOn(SIM_RESULT *res, uint32_t tick, uint32_t timeout);
#if (BOOTLOADER)
static void SetStatePdpOn(SIM_RESULT *res);
#else
static void SetStatePdpOn(SIM_RESULT *res);
static void SetStateInternetOn(SIM_RESULT *res, uint32_t tick, uint32_t timeout);
static void SetStateServerOn(void);
static void SetStateMqttOn(void);
#endif

/* Public functions implementation --------------------------------------------*/
void Simcom_Lock(void) {
#if (RTOS_ENABLE)
	osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
#endif
}

void Simcom_Unlock(void) {
#if (RTOS_ENABLE)
	osMutexRelease(SimcomRecMutexHandle);
#endif
}

void Simcom_Init(void) {
	MX_USART1_UART_Init();
	SIMCOM_DMA_Start(SIM.puart, SIM.pdma);
}

void Simcom_DeInit(void) {
	GATE_SimcomShutdown();
	SIMCOM_DMA_Stop();
	HAL_UART_DeInit(SIM.puart);
}

uint8_t Simcom_SetState(SIMCOM_STATE state, uint32_t timeout) {
	SIMCOM_STATE lastState = SIM_STATE_DOWN;
	SIM_RESULT res = SIM_ERROR;
	uint32_t tick = _GetTickMS();
	uint8_t retry = 0;

	Simcom_Lock();
	do {
		if (StateTimeout(&tick, timeout, res))
			break;
		if (StateLockedLoop(&lastState, &retry))
			break;
		if (StatePoorSignal())
			break;

		// Set value
		res = SIM_OK;

		// Handle states
		switch (SIM.state) {
			case SIM_STATE_DOWN:
				SetStateDown(&res);
				break;

			case SIM_STATE_READY:
				SetStateReady(&res);
				break;

			case SIM_STATE_CONFIGURED:
				SetStateConfigured(&res, tick, timeout);
				break;

			case SIM_STATE_NETWORK_ON:
				SetStateNetworkOn(&res, tick, timeout);
				break;

			case SIM_STATE_GPRS_ON:
				SetStateGprsOn(&res, tick, timeout);
				break;

#if (BOOTLOADER)
			case SIM_STATE_PDP_ON:
				SetStatePdpOn(&res);
				break;
			case SIM_STATE_BEARER_ON:
				/*nothing*/
				break;
#else

			case SIM_STATE_PDP_ON:
				SetStatePdpOn(&res);
				break;

			case SIM_STATE_INTERNET_ON:
				SetStateInternetOn(&res, tick, timeout);
				break;

			case SIM_STATE_SERVER_ON:
				SetStateServerOn();
				break;

			case SIM_STATE_MQTT_ON:
				SetStateMqttOn();
				break;
#endif

			default:
				break;
		}
		_DelayMS(500);
	} while (SIM.state < state);
	Simcom_Unlock();

	return (SIM.state >= state);
}

SIM_RESULT Simcom_Cmd(char *command, char *reply, uint32_t ms) {
	return Simcom_CmdRaw(command, strlen(command), reply, ms);
}

char* Simcom_Resp(char *keyword, char *from) {
	char *start = &SIMCOM_UART_RX[0];
	char *stop = &SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];

	if (from != NULL)
		if (from >= start && from <= stop)
			start = from;

	return strnstr(start, keyword, stop - start);
}

#if (!BOOTLOADER)
void Simcom_CalibrateTime(void) {
	timestamp_t ts;

	if (!Simcom_SetState(SIM_STATE_READY, 0))
		return;

	if (AT_Clock(ATR, &ts) == SIM_OK)
		RTC_Calibrate(&ts);
}

uint8_t Simcom_SendUSSD(char *ussd, char *buf, uint8_t buflen) {
	SIM_RESULT res;

	if (!Simcom_SetState(SIM_STATE_NETWORK_ON, 0))
		return 0;

	// Set TE Character
	{
		char chset[5] = "GSM";
		res = AT_CharacterSetTE(ATW, chset, sizeof(chset));
	}
	// Delete all message
	{
		at_cmgd_t param = {
				.index = 0,
				.delflag = CMGD_ALL
		};
		res = AT_DeleteMessageSMS(&param);
	}
	// Dial USSD (check quota)
	if (res == SIM_OK) {
		at_cusd_t param = {
				.n = CUSD_ENABLE,
				.dcs = 15
		};
		strncpy(param.str, ussd, sizeof(param.str));
		res = AT_ServiceDataUSSD(ATW, &param, buf, buflen);
	}

	return res == SIM_OK;
}

uint8_t Simcom_ReadNewSMS(char *buf, uint8_t buflen) {
	SIM_RESULT res;

	if (!Simcom_SetState(SIM_STATE_NETWORK_ON, 0))
		return 0;

	// Set SMS format
	{
		AT_CMGF param = CMGF_TEXT;
		res = AT_MessageFormatSMS(ATW, &param);
	}
	//	// Set preferred storage
	//	if (res == SIM_OK) {
	//		at_cpms_t param;
	//
	//		for(uint8_t i = 0; i < sizeof(at_cpms_t) / sizeof(at_cpms_mem_t); i++)
	//			strcpy(param.mem[i].storage, "SM");
	//		res = AT_StorageMessageSMS(ATW, &param);
	//	}
	//	// List SMS
	//	if (res == SIM_OK) {
	//		at_cmgl_t param = {
	//				.stat = CMGL_STAT_UNREAD,
	//				.mode = CMG_MODE_NORMAL
	//		};
	//		res = AT_ListMessageSMS(&param);
	//	}
	//	// Set new SMS indications
	//	if (res == SIM_OK) {
	//		res = AT_MessageIndicationSMS(2, 1);
	//	}
	//	// Wait SMS message
	//	if (res == SIM_OK) {
	//		at_cmti_t param;
	//		res = AT_WaitMessageSMS(&param, 20000);
	//	}
	//
	// Read SMS response
	if (res == SIM_OK) {
		at_cmgr_t par = {
				.index = 1,
				.mode = CMG_MODE_NORMAL
		};
		res = AT_ReadMessageSMS(&par, buf, buflen);
	}

	return res == SIM_OK;
}

uint8_t Simcom_Upload(void *payload, uint16_t size) {
	SIM_RESULT res;
	char cmd[20];

	if (SIM.ipstatus != CIPSTAT_CONNECT_OK && SIM.state < SIM_STATE_SERVER_ON)
		return 0;

	Simcom_Lock();
	sprintf(cmd, "AT+CIPSEND=%d\r", size);
	res = Simcom_Cmd(cmd, SIM_RSP_SEND, 500);

	if (res == SIM_OK)
		res = Simcom_CmdRaw((char*) payload, size, SIM_RSP_SENT, 30000);
	Simcom_Unlock();

	return res == SIM_OK;
}

int Simcom_GetData(unsigned char *buf, int count) {
	if (SIM.response == NULL)
		return -1;
	if (SIM.response < &SIMCOM_UART_RX[0])
		return -1;
	if ((SIM.response + count) > &SIMCOM_UART_RX[SIMCOM_UART_RX_SZ])
		return -1;

	memcpy(buf, SIM.response, count);
	SIM.response += count;

	return count;
}

uint8_t Simcom_ReceivedResponse(uint32_t timeout) {
	TickType_t tick = _GetTickMS();
	char *ptr = NULL;

	do {
		ptr = Simcom_Resp(SIM_RSP_IPD, NULL);
		_DelayMS(10);
	} while (ptr == NULL && (_GetTickMS() - tick) < timeout);

	if (ptr != NULL)
		if ((ptr = Simcom_Resp(":", ptr)) != NULL)
			SIM.response = ptr+1;

	return (ptr != NULL);
}

#endif

/* Private functions implementation --------------------------------------------*/
static SIM_RESULT PowerUp(void) {
	SIM_RESULT res;

	res = Reset(0);
	if (res != SIM_OK)
#if (!BOOTLOADER)
		if (VCU.d.bat > SIMCOM_MIN_VOLTAGE)
#endif
			return Reset(1);

	return res;
}

static SIM_RESULT Reset(uint8_t hard) {
	uint32_t tick;

	SIMCOM_Reset_Buffer();
	printf("Simcom:%s\n", hard ? "HardReset" : "SoftReset");

	if (hard) GATE_SimcomReset();
	else GATE_SimcomSoftReset();

#if (!BOOTLOADER)
	VCU.SetEvent(hard ? EV_VCU_NET_HARD_RESET : EV_VCU_NET_SOFT_RESET, 1);
#endif

	// Wait until ready
	tick = _GetTickMS();
	do {
		if (Simcom_Resp(SIM_RSP_READY, NULL) || Simcom_Resp(SIM_RSP_OK, NULL))
			break;
		_DelayMS(100);
	} while (SIM.state == SIM_STATE_DOWN && (_GetTickMS() - tick) < NET_BOOT_TIMEOUT);

	return Simcom_Cmd(SIM_CMD_BOOT, SIM_RSP_READY, 1000);
}

static void Simcom_IdleJob(void) {
	at_csq_t signal;
	if (AT_SignalQualityReport(&signal) == SIM_OK)
		SIM.signal = signal.percent;

#if (!BOOTLOADER)
	AT_ConnectionStatus(&(SIM.ipstatus));
#endif
}

static SIM_RESULT Simcom_CmdRaw(char *data, uint16_t size, char *reply, uint32_t ms) {
	SIM_RESULT res = SIM_ERROR;

	// only handle command if SIM_STATE_READY or BOOT_CMD
	if (!(SIM.state >= SIM_STATE_READY || (strncmp(data, SIM_CMD_BOOT, size) == 0)))
		return res;

	// Debug: print payload
	if (SIMCOM_DEBUG) {
		if (strcmp(reply, SIM_RSP_SENT) == 0)
			printf_hex(data, size);
		else {
			printf("\n=> ");
			printf("%.*s", size, data);
		}
		printf("\n");
	}

	// execute payload
	Simcom_Lock();
	GATE_SimcomSleep(0);
	res = TransmitCmd(data, size, ms, reply);
	GATE_SimcomSleep(1);
	Simcom_Unlock();

	// Debug: print response (ignore FTPGET command)
	if (SIMCOM_DEBUG)
		if (strnstr(data, SIM_CMD_FTPGET, size) == NULL)
			printf("%.*s\n", sizeof(SIMCOM_UART_RX), SIMCOM_UART_RX);

	return res;
}

static SIM_RESULT TransmitCmd(char *data, uint16_t size, uint32_t ms, char *reply) {
	SIM_RESULT res = SIM_ERROR;
	uint32_t tick;

#if (!BOOTLOADER)
	if (Simcom_ReceivedResponse(0))
		Simcom_ProcessResponse();
#endif
	SIMCOM_Transmit(data, size);

	// wait response from SIMCOM
	ms += NET_EXTRA_TIME;
	tick = _GetTickMS();
	while (1) {
		if (Simcom_Resp(reply, NULL)
				|| Simcom_Resp(SIM_RSP_ERROR, NULL)
				|| Simcom_Resp(SIM_RSP_READY, NULL)
#if (!BOOTLOADER)
				|| Simcom_Resp(SIM_RSP_IPD, NULL)
#endif
				|| (_GetTickMS() - tick) > ms
		) {

			// check response
			if (Simcom_Resp(reply, NULL))
				res = SIM_OK;

			// Handle failure
			else {
				// exception for accidentally reboot
				if (Simcom_Resp(SIM_RSP_READY, NULL) && (SIM.state >= SIM_STATE_READY)) {
					printf("Simcom:Restarted\n");
					res = SIM_RESTARTED;
					SIM.state = SIM_STATE_READY;
#if (!BOOTLOADER)
					VCU.SetEvent(EV_VCU_NET_SOFT_RESET, 1);
#endif
				}

#if (!BOOTLOADER)
				// exception for server command collision
				else if (Simcom_ReceivedResponse(0)) {
					if (Simcom_ProcessResponse()) {
						printf("Simcom:CommandCollision\n");
						res = SIM_TIMEOUT;
					}
				}
#endif
				// exception for no response
				else if ( strnlen(SIMCOM_UART_RX, sizeof(SIMCOM_UART_RX) ) == 0) {
					printf("Simcom:NoResponse\n");
					res = SIM_NORESPONSE;
					SIM.state = SIM_STATE_DOWN;
				}

				// exception for timeout
				else if ((_GetTickMS() - tick) > ms) {
					printf("Simcom:Timeout\n");
					res = SIM_TIMEOUT;
				}

				else
					printf("Simcom:UnknownError\n");
			}

			// exit loop
			break;
		}

#if (BOOTLOADER)
		HAL_IWDG_Refresh(&hiwdg);
#endif
		_DelayMS(10);
	}

	return res;
}

#if (!BOOTLOADER)
static uint8_t Simcom_ProcessResponse(void) {
	char *ptr = SIM.response;

	_DelayMS(1000);

	if (!MQTT_GotPublish()) {
		SIM.response = ptr;
		return 0;
	}
	return 1;
}
#endif

static uint8_t TimeoutReached(uint32_t tick, uint32_t timeout, uint32_t delay) {
	if (timeout && (_GetTickMS() - tick) > timeout) {
		printf("Simcom:StateTimeout\n");
		return 1;
	}

	_DelayMS(delay);
	return 0;
}

static uint8_t StateTimeout(uint32_t *tick, uint32_t timeout, SIM_RESULT res) {
	if (timeout) {
		if (res == SIM_OK)
			*tick = _GetTickMS();

		if (TimeoutReached(*tick, timeout, 0))
			return 1;
	}
	return 0;
}

static uint8_t StateLockedLoop(SIMCOM_STATE *lastState, uint8_t *retry) {
	// Handle locked-loop
	if (SIM.state < *lastState) {
		if (*retry == 0) {
			if (SIM.state > SIM_STATE_DOWN)
				SIM.state--;
			return 1;
		}
		(*retry)--;
		printf("Simcom:LockedLoop = %u\n", *retry);
	}
	*lastState = SIM.state;
	return 0;
}

static uint8_t StatePoorSignal(void) {
	// Handle signal strength
	if (SIM.state == SIM_STATE_DOWN)
		SIM.signal = 0;
	else {
		Simcom_IdleJob();
		if (SIM.state >= SIM_STATE_NETWORK_ON) {
			if (SIM.signal < 15) {
				printf("Simcom:PoorSignal\n");
				return 1;
			}
		}
	}
	return 0;
}

static void NetworkRegistration(char *type, SIM_RESULT *res, uint32_t tick, uint32_t timeout) {
	at_c_greg_t read, param = {
			.mode = CREG_MODE_DISABLE,
			.stat = CREG_STAT_REG_HOME
	};

	// wait until attached
	do {
		*res = AT_NetworkRegistration(type, ATW, &param);
		if (*res == SIM_OK)
			*res = AT_NetworkRegistration(type, ATR, &read);

		if (TimeoutReached(tick, timeout, 1000))
			break;

	} while (*res && read.stat != param.stat);
}

static void SetStateDown(SIM_RESULT *res) {
	printf("Simcom:Init\n");

	// power up the module
	*res = PowerUp();

	// upgrade simcom state
	if (*res == SIM_OK) {
		printf("Simcom:ON\n");
		SIM.state = SIM_STATE_READY;
	} else
		printf("Simcom:Error\n");
}

static void SetStateReady(SIM_RESULT *res) {
	// =========== BASIC CONFIGURATION
	// disable command echo
	if (*res == SIM_OK)
		*res = AT_CommandEchoMode(0);
	// Set serial baud-rate
	if (*res == SIM_OK) {
		uint32_t rate = 0;
		*res = AT_FixedLocalRate(ATW, &rate);
	}
	// Error report format: 0, 1(Numeric), 2(verbose)
	if (*res == SIM_OK) {
		AT_CMEE param = CMEE_VERBOSE;
		*res = AT_ReportMobileEquipmentError(ATW, &param);
	}
	// Use pin DTR as sleep control
	if (*res == SIM_OK) {
		AT_CSCLK param = CSCLK_EN_DTR;
		*res = AT_ConfigureSlowClock(ATW, &param);
	}
#if (!BOOTLOADER)
	// Enable time reporting
	if (*res == SIM_OK) {
		AT_BOOL param = AT_ENABLE;
		*res = AT_EnableLocalTimestamp(ATW, &param);
	}
	// Enable “+IPD” header
	if (*res == SIM_OK) {
		AT_BOOL param = AT_ENABLE;
		*res = AT_IpPackageHeader(ATW, &param);
	}
	// Disable “RECV FROM” header
	if (*res == SIM_OK) {
		AT_BOOL param = AT_DISABLE;
		*res = AT_ShowRemoteIp(ATW, &param);
	}
#endif
	// =========== NETWORK CONFIGURATION
	// Check SIM Card
	if (*res == SIM_OK)
		*res = Simcom_Cmd("AT+CPIN?\r", "READY", 500);

	// Disable presentation of <AcT>&<rac> at CREG and CGREG
	if (*res == SIM_OK) {
		at_csact_t param = {
				.creg = 0,
				.cgreg = 0,
		};
		*res = AT_NetworkAttachedStatus(ATW, &param);
	}

	// upgrade simcom state
	if (*res == SIM_OK)
		SIM.state = SIM_STATE_CONFIGURED;
	else if (SIM.state == SIM_STATE_READY)
		SIM.state = SIM_STATE_DOWN;

}

static void SetStateConfigured(SIM_RESULT *res, uint32_t tick, uint32_t timeout) {
	// =========== NETWORK ATTACH
	// Set signal Generation 2G(13)/3G(14)/AUTO(2)
	if (*res == SIM_OK) {
		at_cnmp_t param = {
				.mode = CNMP_ACT_AUTO,
				.preferred = CNMP_ACT_P_UMTS
		};
		*res = AT_RadioAccessTechnology(ATW, &param);
	}

	// Network Registration Status
	if (*res == SIM_OK)
		NetworkRegistration("CREG", res, tick, timeout);

	// upgrade simcom state
	if (*res == SIM_OK)
		SIM.state = SIM_STATE_NETWORK_ON;
	else if (SIM.state == SIM_STATE_CONFIGURED)
		SIM.state = SIM_STATE_DOWN; // -2 state
}

static void SetStateNetworkOn(SIM_RESULT *res, uint32_t tick, uint32_t timeout) {
	// =========== GPRS ATTACH
	// GPRS Registration Status
	if (*res == SIM_OK)
		NetworkRegistration("CGREG", res, tick, timeout);

	// upgrade simcom state
	if (*res == SIM_OK)
		SIM.state = SIM_STATE_GPRS_ON;
	else if (SIM.state == SIM_STATE_NETWORK_ON)
		SIM.state = SIM_STATE_CONFIGURED;
}

static void SetStateGprsOn(SIM_RESULT *res, uint32_t tick, uint32_t timeout) {
	// Attach to GPRS service
	if (*res == SIM_OK) {
		AT_CGATT param;
		// wait until attached
		do {
			*res = AT_GprsAttachment(ATR, &param);

			if (TimeoutReached(tick, timeout, 1000))
				break;

		} while (*res && !param);
	}

	// upgrade simcom state
	if (*res == SIM_OK) {
		AT_ConnectionStatus(&(SIM.ipstatus));
		if (SIM.ipstatus == CIPSTAT_PDP_DEACT)
			SIM.state = SIM_STATE_DOWN;
		else
			SIM.state = SIM_STATE_PDP_ON;
	} else if (SIM.state == SIM_STATE_GPRS_ON)
		SIM.state = SIM_STATE_NETWORK_ON;
}

#if (BOOTLOADER)
static void SetStatePdpOn(SIM_RESULT *res) {
	// =========== FTP CONFIGURATION
	// Initialise bearer for TCP based applications.
	*res = AT_BearerInitialize();

	// upgrade simcom state
	if (*res == SIM_OK)
		SIM.state = SIM_STATE_BEARER_ON;
	else
		if (SIM.state == SIM_STATE_PDP_ON)
			SIM.state = SIM_STATE_GPRS_ON;
}

#else
static void SetStatePdpOn(SIM_RESULT *res) {
	// =========== PDP ATTACH
	// Set type of authentication for PDP connections of socket
	if (*res == SIM_OK) {
		at_cstt_t param = {
				.apn = NET_CON_APN,
				.username = NET_CON_USERNAME,
				.password = NET_CON_PASSWORD,
		};
		*res = AT_ConfigureAPN(ATW, &param);
	}
	// Select TCPIP application mode:
	// (0: Non Transparent (command mode), 1: Transparent (data mode))
	if (*res == SIM_OK) {
		AT_CIPMODE param = CIPMODE_NORMAL;
		*res = AT_TcpApllicationMode(ATW, &param);
	}
	// Set to Single IP Connection (Backend)
	if (*res == SIM_OK) {
		AT_CIPMUX param = CIPMUX_SINGLE_IP;
		*res = AT_MultiIpConnection(ATW, &param);
	}
	// Get data from network automatically
	if (*res == SIM_OK) {
		AT_CIPRXGET param = CIPRXGET_DISABLE;
		*res = AT_ManuallyReceiveData(ATW, &param);
	}

	// =========== IP ATTACH
	// Check PDP status
	if (*res == SIM_OK) {
		AT_ConnectionStatus(&(SIM.ipstatus));
		if (SIM.ipstatus == CIPSTAT_IP_INITIAL || SIM.ipstatus == CIPSTAT_PDP_DEACT)
			*res = SIM_ERROR;
	}
	// Bring Up IP Connection
	if (*res == SIM_OK) {
		AT_ConnectionStatus(&(SIM.ipstatus));
		if (SIM.ipstatus == CIPSTAT_IP_START)
			*res = Simcom_Cmd("AT+CIICR\r", SIM_RSP_OK, 15000);
	}

	// Check IP Address
	if (*res == SIM_OK) {
		AT_ConnectionStatus(&(SIM.ipstatus));
		if (SIM.ipstatus == CIPSTAT_IP_CONFIG || SIM.ipstatus == CIPSTAT_IP_GPRSACT) {
			at_cifsr_t param;
			*res = AT_GetLocalIpAddress(&param);
		}
	}

	// upgrade simcom state
	if (*res == SIM_OK)
		SIM.state = SIM_STATE_INTERNET_ON;
	else {
		AT_ConnectionStatus(&(SIM.ipstatus));
		if (SIM.ipstatus != CIPSTAT_IP_INITIAL) {
			Simcom_Cmd("AT+CIPCLOSE\r", SIM_RSP_OK, 5000);
			if (SIM.ipstatus != CIPSTAT_PDP_DEACT)
				Simcom_Cmd("AT+CIPSHUT\r", SIM_RSP_OK, 1000);
		}

		if (SIM.state == SIM_STATE_PDP_ON)
			SIM.state = SIM_STATE_GPRS_ON;
	}
}

static void SetStateInternetOn(SIM_RESULT *res, uint32_t tick, uint32_t timeout) {
	// ============ SOCKET CONFIGURATION
	// Establish connection with server
	AT_ConnectionStatus(&(SIM.ipstatus));
	if (*res == SIM_OK && (SIM.ipstatus != CIPSTAT_CONNECT_OK || SIM.ipstatus != CIPSTAT_CONNECTING)) {
		at_cipstart_t param = {
				.mode = "TCP",
				.ip = NET_TCP_SERVER,
				.port = NET_TCP_PORT
		};
		*res = AT_StartConnection(&param);

		// wait until attached
		do {
			AT_ConnectionStatus(&(SIM.ipstatus));

			if (TimeoutReached(tick, timeout, 1000))
				break;

		} while (SIM.ipstatus == CIPSTAT_CONNECTING);
	}

	// upgrade simcom state
	if (*res == SIM_OK)
		SIM.state = SIM_STATE_SERVER_ON;
	else {
		// Check IP Status
		AT_ConnectionStatus(&(SIM.ipstatus));

		// Close IP
		if (SIM.ipstatus == CIPSTAT_CONNECT_OK) {
			*res = Simcom_Cmd("AT+CIPCLOSE\r", SIM_RSP_OK, 1000);

			// wait until closed
			do {
				AT_ConnectionStatus(&(SIM.ipstatus));

				if (TimeoutReached(tick, timeout, 1000))
					break;

			} while (SIM.ipstatus == CIPSTAT_CLOSING);
		}

		if (SIM.state == SIM_STATE_INTERNET_ON)
			SIM.state = SIM_STATE_PDP_ON;
	}
}

static void SetStateServerOn(void) {
	uint8_t valid = 0;

	AT_ConnectionStatus(&(SIM.ipstatus));
	if (SIM.ipstatus == CIPSTAT_CONNECT_OK)
		if (MQTT_Connect())
			valid = 1;

	// upgrade simcom state
	if (valid)
		SIM.state = SIM_STATE_MQTT_ON;
	else if (SIM.state == SIM_STATE_SERVER_ON) {
		MQTT_Disconnect();
		SIM.state = SIM_STATE_INTERNET_ON;
	}
}

static void SetStateMqttOn(void) {
	if (!MQTT_Willed())
		MQTT_PublishWill(1);

	if (!MQTT_Subscribed())
		MQTT_Subscribe();

	AT_ConnectionStatus(&(SIM.ipstatus));
	if (SIM.ipstatus != CIPSTAT_CONNECT_OK || !MQTT_Ping())
		if (SIM.state == SIM_STATE_MQTT_ON)
			SIM.state = SIM_STATE_SERVER_ON;
}
#endif

