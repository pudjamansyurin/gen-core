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
// FIXME combine me with simcom_t
char CIPSEND[50], CIPSTART[50], CLPORT[50], CSTT[75];
simcom_t simcom;

/* USER CODE END PV */
void Ublox_Init(gps_t *hgps) {
	HAL_GPIO_WritePin(UBLOX_PWR_GPIO_Port, UBLOX_PWR_Pin, GPIO_PIN_SET);
	osDelay(100);
	gps_init(hgps);
}

// FIXME change Simcom_Reset to Simcom_Off
static void Simcom_Reset(void) {
	HAL_GPIO_WritePin(SIMCOM_RST_GPIO_Port, SIMCOM_RST_Pin, !GPIO_PIN_RESET);
	osDelay(100);
	HAL_GPIO_WritePin(SIMCOM_RST_GPIO_Port, SIMCOM_RST_Pin, !GPIO_PIN_SET);
	osDelay(100);
}

static void Simcom_Prepare(void) {
	strcpy(simcom.server_ip, "36.81.170.31");
	simcom.server_port = 5044;
	simcom.local_port = 5045;
	strcpy(simcom.network_apn, "telkomsel"); 					// "3gprs,telkomsel"
	strcpy(simcom.network_username, "wap");			// "3gprs,wap"
	strcpy(simcom.network_password, "wap123");			// "3gprs,wap123"
	simcom.signal = SIGNAL_3G;
	simcom.boot_timeout = 60;
	simcom.repeat_delay = 5;
	// prepare command sequence
	sprintf(CIPSEND, "AT+CIPSEND\r");
	sprintf(CSTT, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r", simcom.network_apn, simcom.network_username, simcom.network_password);
	sprintf(CLPORT, "AT+CLPORT=\"UDP\",%d\r", simcom.local_port);
	sprintf(CIPSTART, "AT+CIPSTART=\"UDP\",\"%s\",%d\r", simcom.server_ip, simcom.server_port);
}

static uint8_t Simcom_Boot(void) {
	// reset rx buffer
	SIMCOM_Reset_Buffer();
	// reset the state of simcom module
	Simcom_Reset();
	// wait until booting is done
	return Simcom_Send_Response_Repeat("AT\r", SIMCOM_STATUS_OK, ((simcom.boot_timeout * 1000) / 500), 500);
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
	timeout_tick = pdMS_TO_TICKS(ms + SIMCOM_EXTRA_TIME_MS);
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
				//set permanent baudrate
				p = Simcom_Send("AT+IPR=9600\r", 500);
				//disable command echo
				if (p) {
					p = Simcom_Send("ATE1\r", 500);
				}
			}
			// if boot sequence ok, then disable it
			boot = !p;
		} else {
			p = 1;
		}
		// =========== OTHERS CONFIGURATION
		//Hide “+IPD” header
		if (p) {
			p = Simcom_Send("AT+CIPHEAD=0\r", 500);
		}
		//Hide “RECV FROM” header
		if (p) {
			p = Simcom_Send("AT+CIPSRIP=0\r", 500);
		}
		//Set to Single IP Connection (Backend)
		if (p) {
			p = Simcom_Send("AT+CIPMUX=0\r", 500);
		}
		//Select TCPIP application mode (0: Non Transparent (command mode), 1: Transparent (data mode))
		if (p) {
			p = Simcom_Send("AT+CIPMODE=0\r", 500);
		}
		// Get data from network manually
		if (p) {
			p = Simcom_Send("AT+CIPRXGET=1\r", 500);
		}
		// =========== NETWORK CONFIGURATION
		// Check SIM Card
		if (p) {
			p = Simcom_Send("AT+CPIN?\r", 500);
		}
		// Disable presentation of <AcT>&<rac> at CREG and CGREG
		if (p) {
			p = Simcom_Send("AT+CSACT=0,0\r", 500);
		}

		// Set signal Generation 2G/3G/AUTO
		if (p) {
			p = Simcom_Set_Signal(simcom.signal);
		}

		// Network Registration Status
		if (p) {
			p = Simcom_Send("AT+CREG=2\r", 500);
		}
		// Network GPRS Registration Status
		if (p) {
			p = Simcom_Send("AT+CGREG=2\r", 500);
		}
		//Attach to GPRS service
		if (p) {
			p = Simcom_Send_Response_Repeat("AT+CGATT?\r", "+CGATT: 1", 5, 500);
		}

		// =========== TCP/IP CONFIGURATION
		//Set type of authentication for PDP-IP connections of socket
		if (p) {
			p = Simcom_Send_Response_Repeat(CSTT, SIMCOM_STATUS_OK, 5, 500);
		}
		// Bring Up Wireless Connection with GPRS
		if (p) {
			p = Simcom_Send_Response_Repeat("AT+CIICR\r", SIMCOM_STATUS_OK, 5, 500);
		}
		// Check IP Address
		if (p) {
			Simcom_Send("AT+CIFSR\r", 500);
		}
		// Set local UDP port
		if (p) {
			p = Simcom_Send(CLPORT, 500);
		}

		// Establish connection with server
		if (p) {
			p = Simcom_Send_Response_Repeat(CIPSTART, SIMCOM_STATUS_OK, 5, 500);
		}

		// restart module to fix it
		if (!p) {
			// disable all connection
			Simcom_Send("AT+CIPSHUT\r", 500);
			boot = 1;
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
	p = Simcom_Send_Response_Repeat(CNMP, SIMCOM_STATUS_OK, 2, 5000);

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

	if (Simcom_Send("AT+CIPRXGET=2,1024\r", 500)) {
		// get pointer reference
		start = strstr(SIMCOM_UART_RX_Buffer, "AT$");
		end = strstr(start, "\r\nOK");
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

