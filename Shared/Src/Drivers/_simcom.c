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
#include "Libs/_command.h"
#include "Libs/_mqtt.h"
#include "Nodes/VCU.h"
#else
#include "iwdg.h"
#include "Drivers/_flasher.h"
#include "Libs/_eeprom.h"
#include "Libs/_fota.h"
#include "Libs/_focan.h"
#endif

/* Public variables ---------------------------------------------------------*/
sim_t SIM = {
		.state = SIM_STATE_DOWN,
		.ipstatus = CIPSTAT_UNKNOWN,
		.signal = 0,
		.downloading = 0,
		.response = NULL
};

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT WaitUntilReady(void);
static SIMCOM_RESULT PowerUp(void);
static SIMCOM_RESULT SoftReset(void);
static SIMCOM_RESULT HardReset(void);
static void Simcom_IdleJob(void);
static SIMCOM_RESULT TransmitCmd(char *data, uint16_t size, uint32_t ms, char *reply);
#if (!BOOTLOADER)
static void BeforeTransmitHook(void);
static void Simcom_ProcessResponse(void);
#endif

static uint8_t TimeoutReached(uint32_t tick, uint32_t timeout, uint32_t delay);
static uint8_t StateTimeout(uint32_t *tick, uint32_t timeout, SIMCOM_RESULT res);
static uint8_t StateLockedLoop(SIMCOM_STATE *lastState, uint8_t *retry);
static uint8_t StatePoorSignal(void);
static void NetworkRegistration(char *type, SIMCOM_RESULT *res, uint32_t tick, uint32_t timeout);
static void SetStateDown(SIMCOM_RESULT *res, SIMCOM_STATE *state);
static void SetStateReady(SIMCOM_RESULT *res, SIMCOM_STATE *state);
static void SetStateConfigured(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout);
static void SetStateNetworkOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout);
static void SetStateNetworkOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout);
#if (!BOOTLOADER)
static void SetStateGprsOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout);
static void SetStatePdpOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus);
static void SetStateInternetOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus, uint32_t tick, uint32_t timeout);
static void SetStateServerOn(SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus);
#endif

/* Public functions implementation --------------------------------------------*/
void Simcom_Lock(void) {
	//#if (RTOS_ENABLE)
	//  osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
	//#endif
}

void Simcom_Unlock(void) {
	//#if (RTOS_ENABLE)
	//  osMutexRelease(SimcomRecMutexHandle);
	//#endif
}

char* Simcom_Resp(char *ptr) {
	return strstr(SIMCOM_UART_RX, ptr);
}

void Simcom_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
	SIM.h.uart = huart;
	SIM.h.dma = hdma;

	//  HAL_UART_Init(huart);
	MX_USART1_UART_Init();
	SIMCOM_DMA_Start(huart, hdma);
}

void Simcom_DeInit(void) {
	GATE_SimcomShutdown();
	SIMCOM_DMA_Stop();
	HAL_UART_DeInit(SIM.h.uart);
}


uint8_t Simcom_SetState(SIMCOM_STATE state, uint32_t timeout) {
	SIMCOM_STATE lastState = SIM_STATE_DOWN;
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint32_t tick = _GetTickMS();
	uint8_t retry = 3;

	Simcom_Lock();
	do {
		if (StateTimeout(&tick, timeout, res))
			break;
		if (StateLockedLoop(&lastState, &retry))
			break;
		if (StatePoorSignal())
			break;

		// Set value
		res = SIM_RESULT_OK;

		// Handle states
		switch (SIM.state) {
			case SIM_STATE_DOWN:
				SetStateDown(&res, &(SIM.state));
				_DelayMS(500);
				break;

			case SIM_STATE_READY:
				SetStateReady(&res, &(SIM.state));
				_DelayMS(500);
				break;

			case SIM_STATE_CONFIGURED:
				SetStateConfigured(&res, &(SIM.state), tick, timeout);
				_DelayMS(500);
				break;

			case SIM_STATE_NETWORK_ON:
				SetStateNetworkOn(&res, &(SIM.state), tick, timeout);
				_DelayMS(500);
				break;

#if (!BOOTLOADER)
			case SIM_STATE_GPRS_ON:
				SetStateGprsOn(&res, &(SIM.state), tick, timeout);
				_DelayMS(500);
				break;

			case SIM_STATE_PDP_ON:
				SetStatePdpOn(&res, &(SIM.state), &(SIM.ipstatus));
				_DelayMS(500);
				break;

			case SIM_STATE_INTERNET_ON:
				AT_ConnectionStatusSingle(&(SIM.ipstatus));
				SetStateInternetOn(&res, &(SIM.state), &(SIM.ipstatus), tick, timeout);
				_DelayMS(500);
				break;

			case SIM_STATE_SERVER_ON:
				AT_ConnectionStatusSingle(&(SIM.ipstatus));
				SetStateServerOn(&(SIM.state), &(SIM.ipstatus));
				_DelayMS(500);
				break;

			case SIM_STATE_MQTT_ON:
				if (SIM.ipstatus != CIPSTAT_CONNECT_OK || !MQTT_Ping())
					if (SIM.state == SIM_STATE_MQTT_ON)
						SIM.state--;

				_DelayMS(500);
				break;
#endif

			default:
				break;
		}
	} while (SIM.state < state);
	Simcom_Unlock();

	return (SIM.state >= state);
}

SIMCOM_RESULT Simcom_Cmd(char *data, char *reply, uint32_t ms, uint16_t size) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t upload = 1;

	// only handle command if SIM_STATE_READY or BOOT_CMD
	if (!(SIM.state >= SIM_STATE_READY || (strcmp(data, SIMCOM_CMD_BOOT) == 0)))
		return res;

	// Handle default value
	if (reply == NULL)
		reply = SIMCOM_RSP_OK;

	// Handle command (not upload)
	if (size == 0) {
		upload = 0;
		size = strlen(data);
	}

	// Debug: print payload
	if (SIMCOM_DEBUG) {
		printf("\n=> ");
		if (!upload)
			printf("%.*s", size, data);
		else
			LogBufferHex(data, size);
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
		if (strncmp(data, SIMCOM_CMD_FTPGET, strlen(SIMCOM_CMD_FTPGET)) != 0)
			printf("%.*s\n", sizeof(SIMCOM_UART_RX), SIMCOM_UART_RX);

	return res;
}

SIMCOM_RESULT Simcom_UpdateSignal(void) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	at_csq_t signal;

	res = AT_SignalQualityReport(&signal);
	if (res > 0)
		SIM.signal = signal.percent;

	return res;
}

#if (!BOOTLOADER)
void Simcom_CalibrateTime(void) {
	timestamp_t ts;

	if (AT_Clock(ATR, &ts))
		RTC_Calibrate(&ts);
}


SIMCOM_RESULT Simcom_Upload(void *payload, uint16_t size) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char ptr[20];

	if (SIM.ipstatus != CIPSTAT_CONNECT_OK && SIM.state < SIM_STATE_SERVER_ON)
		return res;

	Simcom_Lock();
	sprintf(ptr, "AT+CIPSEND=%d\r", size);
	res = Simcom_Cmd(ptr, SIMCOM_RSP_SEND, 500, 0);

	if (res > 0)
		res = Simcom_Cmd((char*) payload, SIMCOM_RSP_SENT, 20000, size);

	Simcom_Unlock();

	return res;
}

int Simcom_GetData(unsigned char *buf, int count) {
	if (SIM.response == NULL)
		return -1;

	memcpy(buf, SIM.response, count);
	SIM.response += count;

	return count;
}

uint8_t Simcom_ReceiveResponse(uint32_t timeout) {
	TickType_t tick = _GetTickMS();
	char *ptr = NULL;

	do {
		ptr = Simcom_Resp(SIMCOM_RSP_IPD);
		_DelayMS(10);
	} while (ptr == NULL && (_GetTickMS() - tick) < timeout);

	if (ptr != NULL) {

		if ((ptr = strstr(ptr, ":")) != NULL)
			SIM.response = ptr+1;
	}

	return (ptr != NULL);
}

#endif

/* Private functions implementation --------------------------------------------*/
static SIMCOM_RESULT WaitUntilReady(void) {
	uint32_t tick = _GetTickMS();

	do {
		if (Simcom_Resp(SIMCOM_RSP_READY) || Simcom_Resp(SIMCOM_RSP_OK))
			break;
		_DelayMS(100);
	} while (SIM.state == SIM_STATE_DOWN && (_GetTickMS() - tick) < NET_BOOT_TIMEOUT );

	return Simcom_Cmd(SIMCOM_CMD_BOOT, SIMCOM_RSP_READY, 1000, 0);
}

static SIMCOM_RESULT PowerUp(void) {
	SIMCOM_RESULT res;

	res = SoftReset();
	if (res != SIM_RESULT_OK)
		return HardReset();

	return res;
}

static SIMCOM_RESULT SoftReset(void) {
	printf("Simcom:SoftReset\n");
	SIMCOM_Reset_Buffer();

	GATE_SimcomSoftReset();
#if (!BOOTLOADER)
	VCU.SetEvent(EV_VCU_NET_SOFT_RESET, 1);
#endif

	return WaitUntilReady();
}

static SIMCOM_RESULT HardReset(void) {
	printf("Simcom:HardReset\n");
	SIMCOM_Reset_Buffer();

	//	Simcom_Init(SIM.h.uart, SIM.h.dma);
	GATE_SimcomReset();
#if (!BOOTLOADER)
	VCU.SetEvent(EV_VCU_NET_HARD_RESET, 1);
#endif

	return WaitUntilReady();
}

static void Simcom_IdleJob(void) {
	Simcom_UpdateSignal();

#if (!BOOTLOADER)
	AT_ConnectionStatusSingle(&(SIM.ipstatus));
#endif
}

static SIMCOM_RESULT TransmitCmd(char *data, uint16_t size, uint32_t ms, char *reply) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint32_t tick, timeout = 0;

	// transmit to serial (low-level)
#if (!BOOTLOADER)
	BeforeTransmitHook();
#endif
	SIMCOM_Transmit(data, size);

	// set timeout guard
	timeout = (ms + NET_EXTRA_TIME );
	tick = _GetTickMS();

	// wait response from SIMCOM
	while (1) {
		if (Simcom_Resp(reply)
				|| Simcom_Resp(SIMCOM_RSP_ERROR)
				|| Simcom_Resp(SIMCOM_RSP_READY)
#if (!BOOTLOADER)
				|| Simcom_ReceiveResponse(0)
#endif
				|| (_GetTickMS() - tick) > timeout
		) {

			// check response
			if (Simcom_Resp(reply))
				res = SIM_RESULT_OK;

			// Handle failure
			else {
				// exception for no response
				if (strlen(SIMCOM_UART_RX) == 0) {
					printf("Simcom:NoResponse\n");
					res = SIM_RESULT_NO_RESPONSE;
					SIM.state = SIM_STATE_DOWN;
				}

				// exception for accidentally reboot
				else if (Simcom_Resp(SIMCOM_RSP_READY) && (SIM.state >= SIM_STATE_READY)) {
					printf("Simcom:Restarted\n");
					res = SIM_RESULT_RESTARTED;
					SIM.state = SIM_STATE_READY;
#if (!BOOTLOADER)
					VCU.SetEvent(EV_VCU_NET_SOFT_RESET, 1);
#endif
				}

#if (!BOOTLOADER)
				// exception for server command collision
				else if (Simcom_ReceiveResponse(0)) {
					printf("Simcom:CommandCollision\n");
					Simcom_ProcessResponse();
					res = SIM_RESULT_TIMEOUT;
				}
#endif

				// exception for timeout
				else if ((_GetTickMS() - tick) > timeout) {
					printf("Simcom:Timeout\n");
					res = SIM_RESULT_TIMEOUT;
				}
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
static void BeforeTransmitHook(void) {

	// Handle Server Command
	if (Simcom_ReceiveResponse(0))
		Simcom_ProcessResponse();

	// handle things on every request
	// printf("============ SIMCOM DEBUG ============\n");
	// printf("%.s\n", strlen(SIMCOM_UART_RX), SIMCOM_UART_RX);
	// printf("======================================\n");
}

static void Simcom_ProcessResponse(void) {
	command_t cmd = *(command_t*) SIM.response;

	if (MQTT_Receive(&cmd, SIM.response))
		CMD_CheckCommand(cmd);
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

static uint8_t StateTimeout(uint32_t *tick, uint32_t timeout, SIMCOM_RESULT res) {
	if (timeout) {
		if (res == SIM_RESULT_OK)
			*tick = _GetTickMS();

		if (TimeoutReached(*tick, timeout, 0))
			return 1;
	}
	return 0;
}

static uint8_t StateLockedLoop(SIMCOM_STATE *lastState, uint8_t *retry) {
	// Handle locked-loop
	if (SIM.state < *lastState) {
		*retry -= 1;
		if (!(*retry)) {
			SIM.state = SIM_STATE_DOWN;
			return 1;
		}
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

static void NetworkRegistration(char *type, SIMCOM_RESULT *res, uint32_t tick, uint32_t timeout) {
	at_c_greg_t read, param = {
			.mode = CREG_MODE_DISABLE,
			.stat = CREG_STAT_REG_HOME
	};
	// wait until attached
	do {
		*res = AT_NetworkRegistration(type, ATW, &param);
		if (*res > 0)
			*res = AT_NetworkRegistration(type, ATR, &read);

		if (TimeoutReached(tick, timeout, 1000))
			break;

	} while (*res && read.stat != param.stat);
}

static void SetStateDown(SIMCOM_RESULT *res, SIMCOM_STATE *state) {
	printf("Simcom:Init\n");

	// power up the module
	*res = PowerUp();

	// upgrade simcom state
	if (*res > 0) {
		(*state)++;
		printf("Simcom:ON\n");
	} else
		printf("Simcom:Error\n");
}

static void SetStateReady(SIMCOM_RESULT *res, SIMCOM_STATE *state) {
	// =========== BASIC CONFIGURATION
	// disable command echo
	if (*res > 0)
		*res = AT_CommandEchoMode(0);
	// Set serial baud-rate
	if (*res > 0) {
		uint32_t rate = 0;
		*res = AT_FixedLocalRate(ATW, &rate);
	}
	// Error report format: 0, 1(Numeric), 2(verbose)
	if (*res > 0) {
		AT_CMEE param = CMEE_VERBOSE;
		*res = AT_ReportMobileEquipmentError(ATW, &param);
	}
	// Use pin DTR as sleep control
	if (*res > 0) {
		AT_CSCLK param = CSCLK_EN_DTR;
		*res = AT_ConfigureSlowClock(ATW, &param);
	}
#if (!BOOTLOADER)
	// Enable time reporting
	if (*res > 0) {
		AT_BOOL param = AT_ENABLE;
		*res = AT_EnableLocalTimestamp(ATW, &param);
	}
	// Enable “+IPD” header
	if (*res > 0) {
		AT_BOOL param = AT_ENABLE;
		*res = AT_IpPackageHeader(ATW, &param);
	}
	// Disable “RECV FROM” header
	if (*res > 0) {
		AT_BOOL param = AT_DISABLE;
		*res = AT_ShowRemoteIp(ATW, &param);
	}
#endif
	// =========== NETWORK CONFIGURATION
	// Check SIM Card
	if (*res > 0)
		*res = Simcom_Cmd("AT+CPIN?\r", "READY", 500, 0);
	// Disable presentation of <AcT>&<rac> at CREG and CGREG
	if (*res > 0) {
		at_csact_t param = {
				.creg = 0,
				.cgreg = 0,
		};
		*res = AT_NetworkAttachedStatus(ATW, &param);
	}

	// upgrade simcom state
	if (*res > 0)
		(*state)++;
}

static void SetStateConfigured(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout) {
	// =========== NETWORK ATTACH
	// Set signal Generation 2G(13)/3G(14)/AUTO(2)
	if (*res > 0) {
		at_cnmp_t param = {
				.mode = CNMP_ACT_AUTO,
				.preferred = CNMP_ACT_P_UMTS
		};
		*res = AT_RadioAccessTechnology(ATW, &param);
	}
	// Network Registration Status
	if (*res > 0)
		NetworkRegistration("CREG", res, tick, timeout);

	// upgrade simcom state
	if (*res > 0)
		(*state)++;
}

static void SetStateNetworkOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout) {
	// =========== GPRS ATTACH
	// GPRS Registration Status
	if (*res > 0)
		NetworkRegistration("CGREG", res, tick, timeout);

	// upgrade simcom state
	if (*res > 0)
		(*state)++;
	else
		if (*state == SIM_STATE_NETWORK_ON)
			(*state)--;
}

#if (!BOOTLOADER)
static void SetStateGprsOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout) {
	// =========== PDP CONFIGURATION
	// Attach to GPRS service
	if (*res > 0) {
		AT_CGATT param;
		// wait until attached
		do {
			*res = AT_GprsAttachment(ATR, &param);

			if (TimeoutReached(tick, timeout, 1000))
				break;

		} while (*res && !param);
	}

	// upgrade simcom state
	if (*res > 0)
		(*state)++;
	else
		if (*state == SIM_STATE_GPRS_ON)
			(*state)--;
}

static void SetStatePdpOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus) {
	// =========== PDP ATTACH
	// Set type of authentication for PDP connections of socket
	AT_ConnectionStatusSingle(ipStatus);
	if (*res > 0 && (*ipStatus == CIPSTAT_IP_INITIAL || *ipStatus == CIPSTAT_PDP_DEACT)) {
		at_cstt_t param = {
				.apn = NET_CON_APN,
				.username = NET_CON_USERNAME,
				.password = NET_CON_PASSWORD,
		};
		*res = AT_ConfigureAPN(ATW, &param);
	}
	// Select TCPIP application mode:
	// (0: Non Transparent (command mode), 1: Transparent (data mode))
	if (*res > 0) {
		AT_CIPMODE param = CIPMODE_NORMAL;
		*res = AT_TcpApllicationMode(ATW, &param);
	}
	// Set to Single IP Connection (Backend)
	if (*res > 0) {
		AT_CIPMUX param = CIPMUX_SINGLE_IP;
		*res = AT_MultiIpConnection(ATW, &param);
	}
	// Get data from network automatically
	if (*res > 0) {
		AT_CIPRXGET param = CIPRXGET_DISABLE;
		*res = AT_ManuallyReceiveData(ATW, &param);
	}

	// =========== IP ATTACH
	// Bring Up IP Connection
	AT_ConnectionStatusSingle(ipStatus);
	if (*res > 0 && *ipStatus == CIPSTAT_IP_START)
		*res = Simcom_Cmd("AT+CIICR\r", NULL, 10000, 0);

	// Check IP Address
	AT_ConnectionStatusSingle(ipStatus);
	if (*res > 0 && (*ipStatus == CIPSTAT_IP_CONFIG || *ipStatus == CIPSTAT_IP_GPRSACT)) {
		at_cifsr_t param;
		*res = AT_GetLocalIpAddress(&param);
	}

	// upgrade simcom state
	if (*res > 0)
		(*state)++;
	else {
		// Check IP Status
		AT_ConnectionStatusSingle(ipStatus);

		// Close PDP
		if (*ipStatus != CIPSTAT_IP_INITIAL &&
				*ipStatus != CIPSTAT_PDP_DEACT)
			*res = Simcom_Cmd("AT+CIPSHUT\r", NULL, 1000, 0);

		if ((*state) == SIM_STATE_PDP_ON)
			(*state)--;
	}
}

static void SetStateInternetOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus, uint32_t tick, uint32_t timeout) {
	// ============ SOCKET CONFIGURATION
	// Establish connection with server
	if (*res > 0 && (*ipStatus != CIPSTAT_CONNECT_OK || *ipStatus != CIPSTAT_CONNECTING)) {
		at_cipstart_t param = {
				.mode = "TCP",
				.ip = NET_TCP_SERVER,
				.port = NET_TCP_PORT
		};
		*res = AT_StartConnectionSingle(&param);

		// wait until attached
		do {
			AT_ConnectionStatusSingle(ipStatus);

			if (TimeoutReached(tick, timeout, 1000))
				break;

		} while (*ipStatus == CIPSTAT_CONNECTING);
	}

	// upgrade simcom state
	if (*res > 0)
		(*state)++;
	else {
		// Check IP Status
		AT_ConnectionStatusSingle(ipStatus);

		// Close IP
		if (*ipStatus == CIPSTAT_CONNECT_OK) {
			*res = Simcom_Cmd("AT+CIPCLOSE\r", NULL, 1000, 0);

			// wait until closed
			do {
				AT_ConnectionStatusSingle(ipStatus);

				if (TimeoutReached(tick, timeout, 1000))
					break;

			} while (*ipStatus == CIPSTAT_CLOSING);
		}

		if (*state == SIM_STATE_INTERNET_ON)
			(*state)--;
	}
}

static void SetStateServerOn(SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus) {
	uint8_t valid = 0;

	if (*ipStatus == CIPSTAT_CONNECT_OK)
		if (MQTT_Connect())
			if (MQTT_DoSubscribe())
				valid = 1;

	// upgrade simcom state
	if (valid)
		(*state)++;
	else
		if (*state == SIM_STATE_SERVER_ON) {
			MQTT_Disconnect();
			(*state)--;
		}
}
#endif

