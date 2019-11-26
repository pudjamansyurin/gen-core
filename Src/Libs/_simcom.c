/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */
#include <_simcom.h>

/* Private functions ---------------------------------------------------------*/
//static void Simcom_On(void);
static void Simcom_Reset(void);
static uint8_t Simcom_Response(char *str);
static uint8_t Simcom_Send(char *cmd, uint32_t ms);
static uint8_t Simcom_Send_Response_Repeat(char *command, char *response, uint8_t n, uint32_t ms);
static void Simcom_Prepare(void);
static uint8_t Simcom_Boot(void);

/* External variable ---------------------------------------------------------*/
extern char SIMCOM_UART_RX_Buffer[SIMCOM_UART_RX_BUFFER_SIZE];
extern osMutexId SimcomRecMutexHandle;
extern osThreadId CommandTaskHandle;
/* Private variable ---------------------------------------------------------*/
extern char PAYLOAD[];
char CIPSEND[50], CIPOPEN[50], CGSOCKCONT[50], CSOCKAUTH[65];
simcom_t simcom;

/* USER CODE END PV */
void Ublox_Init(gps_t *hgps) {
	HAL_GPIO_WritePin(UBLOX_PWR_GPIO_Port, UBLOX_PWR_Pin, GPIO_PIN_SET);
	osDelay(100);
	gps_init(hgps);
}

//static void Simcom_On(void) {
//	HAL_GPIO_WritePin(SIMCOM_PWR_GPIO_Port, SIMCOM_PWR_Pin, GPIO_PIN_RESET);
//	osDelay(100);
//	HAL_GPIO_WritePin(SIMCOM_PWR_GPIO_Port, SIMCOM_PWR_Pin, GPIO_PIN_SET);
//	osDelay(100);
//}

// FIXME change Simcom_Reset to Simcom_Off
static void Simcom_Reset(void) {
	HAL_GPIO_WritePin(SIMCOM_RST_GPIO_Port, SIMCOM_RST_Pin, GPIO_PIN_SET);
	osDelay(1);
	HAL_GPIO_WritePin(SIMCOM_RST_GPIO_Port, SIMCOM_RST_Pin, GPIO_PIN_RESET);
//	osDelay(100);
}

static uint8_t Simcom_Boot(void) {
	uint32_t tick, timeout_tick;

	// reset rx buffer
	SIMCOM_Reset_Buffer();
	// reset the state of simcom module
	Simcom_Reset();
	// turn off sequence
	//	Simcom_On();
	// set timeout guard (for first boot)
	timeout_tick = pdMS_TO_TICKS(simcom.boot_timeout * 1000);
	tick = osKernelSysTick();
	// wait until booting is done
	while (!(Simcom_Response(SIMCOM_STATUS_READY) || (osKernelSysTick() - tick) > timeout_tick)) {
	};
	// handle timeout
	return (uint8_t) ((osKernelSysTick() - tick) < timeout_tick);
}

static uint8_t Simcom_Response(char *str) {
	if (strstr(SIMCOM_UART_RX_Buffer, str) != NULL) {

		return 1;
	}
	return 0;
}

static uint8_t Simcom_Send(char *cmd, uint32_t ms) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	uint8_t ret;
	uint32_t tick, timeout_tick = 0;
	// reset rx buffer
	SIMCOM_Reset_Buffer();
	// print command for debugger
	if (strstr(cmd, SIMCOM_MESSAGE_END) == NULL) {
		SWV_SendStr("\n=> ");
		SWV_SendBuf(cmd, strlen(cmd));
		SWV_SendStrLn("");
	}
	// transmit to serial
	SIMCOM_Transmit(cmd, strlen(cmd));
	// convert time to tick
	timeout_tick = pdMS_TO_TICKS(ms);
	// set timeout guard
	tick = osKernelSysTick();
	// wait response from SIMCOM
	while (!(Simcom_Response(SIMCOM_STATUS_SEND) || Simcom_Response(SIMCOM_STATUS_CIPSEND) || Simcom_Response(SIMCOM_STATUS_OK)
			|| Simcom_Response(SIMCOM_STATUS_RESTARTED) || Simcom_Response(SIMCOM_STATUS_ERROR)
			|| (osKernelSysTick() - tick) >= timeout_tick))
		;
	// handle timeout & error
	ret = !(Simcom_Response(SIMCOM_STATUS_ERROR) || Simcom_Response(SIMCOM_STATUS_RESTARTED)
			|| (osKernelSysTick() - tick) > timeout_tick);
	// print response for debugger
	SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
	SWV_SendStrLn("");
	// check if it has new command
	if (Simcom_Check_Command()) {
		xTaskNotify(CommandTaskHandle, EVENT_COMMAND_ARRIVED, eSetBits);
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}

static uint8_t Simcom_Send_Response_Repeat(char *command, char *response, uint8_t n, uint32_t ms) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	uint8_t ret = 1, seq = 1;
	// check is it on reboot state
	if (Simcom_Response(SIMCOM_STATUS_RESTARTED)) {
		ret = 0;
	} else {
		// repeat command until desired response
		do {
			if (seq > 1) {
				// execute command every timeout guard elapsed
				osDelay(simcom.repeat_delay * 1000);
			}
			// send command
			if (seq <= n) {
				Simcom_Send(command, ms);
				// if device error, break
				if (Simcom_Response(SIMCOM_STATUS_RESTARTED) || Simcom_Response("+IP ERROR")) {
					ret = 0;
					break;
				}
			} else {
				// if until max sequence still error, break
				ret = 0;
				break;
			}
			// increment sequence
			seq++;
		} while (!Simcom_Response(response));
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}

static void Simcom_Prepare(void) {
	strcpy(simcom.server_ip, "180.247.126.111");
	simcom.server_port = 5044;
	simcom.local_port = 5044;
	strcpy(simcom.network_apn, "3gprs"); 					// "telkomsel"
	strcpy(simcom.network_username, "3gprs");			// "wap"
	strcpy(simcom.network_password, "3gprs");			// "wap123"
	simcom.signal = SIGNAL_3G;
	simcom.boot_timeout = 60;
	simcom.repeat_delay = 5;
	// prepare command sequence
	sprintf(CIPSEND, "AT+CIPSEND=0,,\"%s\",%d\r", simcom.server_ip, simcom.server_port);
	sprintf(CIPOPEN, "AT+CIPOPEN=0,\"UDP\",,,%d\r", simcom.local_port);
	sprintf(CGSOCKCONT, "AT+CGSOCKCONT=1,\"IP\",\"%s\"\r", simcom.network_apn);
	sprintf(CSOCKAUTH, "AT+CSOCKAUTH=1,1,\"%s\",\"%s\"\r", simcom.network_username, simcom.network_password);
}

void Simcom_Init(uint8_t skipFirstBoot) {
	uint8_t p, boot = 1;
	// this do-while is complicated, but it doesn't use recursive function, so it's stack safe
	do {
		// show previous response
		SWV_SendStrLn("=========================");
		SWV_SendStrLn("       BEFORE BOOT       ");
		SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
		SWV_SendStrLn("\n=========================");

		SWV_SendStrLn("Simcom_Init");

		p = 0;
		// Turn on the module & wait till ready
		if (boot && !skipFirstBoot) {
			skipFirstBoot = 0;
			//set default value to variable
			Simcom_Prepare();
			// booting
			p = Simcom_Boot();
			// Execute only on first setup
			if (p) {
				// show boot response
				SWV_SendStrLn("=========================");
				SWV_SendStrLn("        AFTER BOOT       ");
				SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
				SWV_SendStrLn("\n=========================");
				//disable command echo
				p = Simcom_Send("ATE1\r", 500);
				//set permanent baudrate
				if (p) {
					p = Simcom_Send("AT+IPREX=9600\r", 500);
				}
				//save user setting to ME
				if (p) {
					p = Simcom_Send("AT&W\r", 500);
				}
			}
			// if boot sequence ok, then disable it
			boot = !p;
		} else {
			p = 1;
		}
		//Hide “+IPD” header
		if (p) {
			p = Simcom_Send("AT+CIPHEAD=0\r", 500);
		}
		//Hide “RECV FROM” header
		if (p) {
			p = Simcom_Send("AT+CIPSRIP=0\r", 500);
		}
		//Set module to cache received data.
		if (p) {
			p = Simcom_Send("AT+CIPRXGET=1\r", 500);
		}
		// Set signal
		if (p) {
			p = Simcom_Set_Signal(simcom.signal);
		}

		//Check network registration
		if (p) {
			p = Simcom_Send_Response_Repeat("AT+CREG?\r", "+CREG: 0,1", 10, 500);
		}
		//Check signal
		if (p) {
			// wait until signal catch up
			p = Simcom_Signal_Locked(10);
			// restart module to fix it
			boot = !p;
		}
		//Check GPSRS network registration
		if (simcom.signal == SIGNAL_2G) {
			if (p) {
				p = Simcom_Send_Response_Repeat("AT+CGREG?\r", "+CGREG: 0,1", 10, 500);
			}
		}

		//Define socket PDP context (APN Settings)
		if (p) {
			p = Simcom_Send(CGSOCKCONT, 500);
		}
		//Set active PDP context profile number
		if (p) {
			p = Simcom_Send("AT+CSOCKSETPN=1\r", 500);
		}
		//Set type of authentication for PDP-IP connections of socket
		if (p) {
			p = Simcom_Send(CSOCKAUTH, 500);
		}
		//Select TCPIP application mode (0: Non Transparent (command mode), 1: Transparent (data mode))
		if (p) {
			p = Simcom_Send("AT+CIPMODE=0\r", 500);
		}
		//Open network
		if (p) {
			p = Simcom_Send("AT+NETOPEN\r", 1000);
		}
		//Open local UDP Connection
		if (p) {
			p = Simcom_Send_Response_Repeat(CIPOPEN, SIMCOM_STATUS_OK, 10, 1000);
			// restart module to fix it
			boot = !p;
		}
	} while (p == 0);
}

uint8_t Simcom_Signal_Locked(uint8_t n) {
	uint8_t p = 0;
	char ONLINE[14];

	// Inquiring UE system information
	sprintf(ONLINE, "%s,Online,", simcom.signal == SIGNAL_3G ? "WCDMA" : "GSM");
	p = Simcom_Send_Response_Repeat("AT+CPSI?\r", ONLINE, n, 500);

	return p;
}

uint8_t Simcom_Set_Signal(signal_t signal) {
	uint8_t p;
	char CNMP[11];

	//Lock to WCDMA
	sprintf(CNMP, "AT+CNMP=%d\r", signal);
	p = Simcom_Send(CNMP, 500);

	return p;
}

uint8_t Simcom_Send_Payload(void) {
	// set sending time
	Reporter_Set_Sending_Time();
	// send payload
	return Simcom_To_Server(PAYLOAD, strlen(PAYLOAD));
}

uint8_t Simcom_To_Server(char *message, uint16_t length) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	uint8_t ret = 0;
	char str[length + 1];
	// add message end character
	sprintf(str, "%s%s", message, SIMCOM_MESSAGE_END);
	// confirm to server that command is executed
	if (Simcom_Send(CIPSEND, 500)) {
		// send response
		if (Simcom_Send(str, 500)) {
			ret = 1;
		}
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}

uint8_t Simcom_Check_Command(void) {
	// check if it has new command
	return (strstr(SIMCOM_UART_RX_Buffer, "+CIPRXGET: 1") != NULL);
}

uint8_t Simcom_Get_Command(command_t *command) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	uint8_t ret = 0;
	char *start, *delim, *end;

	if (Simcom_Send("AT+CIPRXGET=2,0\r", 500)) {
		// get pointer reference
		start = strstr(SIMCOM_UART_RX_Buffer, "AT$");
		end = strstr(start, "\r\n\r\n");
		delim = strchr(start, '=');

		// check if command has value
		if (delim != NULL) {
			// get command
			strncpy(command->var, start + 3, delim - (start + 3));
			*(command->var + (delim - (start + 3))) = '\0';
			// get value
			strncpy(command->val, delim + 1, end - delim);
			*(command->val + (end - delim)) = '\0';
		} else {
			// get command
			strncpy(command->var, start + 3, end - (start + 3));
			*(command->var + (end - (start + 3))) = '\0';
			// set value
			*(command->val) = '\0';
		}

		// get full command
		strncpy(command->cmd, start + 3, end - (start + 3));
		*(command->cmd + (end - (start + 3))) = '\0';

		// reset rx buffer
		SIMCOM_Reset_Buffer();

		ret = 1;
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}

