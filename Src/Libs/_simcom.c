/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */
#include "_simcom.h"
#include "_crc.h"

/* Private functions ---------------------------------------------------------*/
static void Simcom_Reset(void);
static uint8_t Simcom_Response(char *str);
static uint8_t Simcom_Send_Direct(char *data, uint16_t data_length, uint8_t is_payload, uint32_t ms, char *res);
static uint8_t Simcom_Send_Indirect(char *data, uint16_t data_length, uint8_t is_payload, uint32_t ms, char *res, uint8_t n);
static uint8_t Simcom_Command_Match(char *cmd, uint32_t ms, char *res, uint8_t n);
static uint8_t Simcom_Command(char *cmd, uint32_t ms);
static uint8_t Simcom_Payload(char *payload, uint16_t payload_length);
static void Simcom_Prepare(void);
static uint8_t Simcom_Boot(void);

/* External variable ---------------------------------------------------------*/
extern char SIMCOM_UART_RX_Buffer[SIMCOM_UART_RX_BUFFER_SIZE];
extern osMutexId SimcomRecMutexHandle;
extern osThreadId CommandTaskHandle;
/* Private variable ---------------------------------------------------------*/
simcom_t simcom;

/* USER CODE END PV */
void Ublox_Init(gps_t *hgps) {
	HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, GPIO_PIN_SET);
	osDelay(100);
	gps_init(hgps);
}

static void Simcom_Reset(void) {
	HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_SET);
	osDelay(100);
	HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_RESET);
	osDelay(100);
}

static void Simcom_Prepare(void) {
	// prepare command sequence
	sprintf(simcom.CMD_CNMP, "AT+CNMP=%d\r", NET_SIGNAL);
	sprintf(simcom.CMD_CSTT,
			"AT+CSTT=\"%s\",\"%s\",\"%s\"\r",
			NET_APN, NET_APN_USERNAME, NET_APN_PASSWORD);
	sprintf(simcom.CMD_CIPSTART,
			"AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r",
			NET_SERVER_IP, NET_SERVER_PORT);
}

static uint8_t Simcom_Boot(void) {
	uint16_t ms = 500;
	// reset rx buffer
	SIMCOM_Reset_Buffer();
	// reset the state of simcom module
	Simcom_Reset();
	// wait until booting is done
	return Simcom_Command_Match("AT\r", ms, SIMCOM_STATUS_READY, (NET_BOOT_TIMEOUT * 1000) / ms);
}

static uint8_t Simcom_Response(char *str) {
	if (strstr(SIMCOM_UART_RX_Buffer, str) != NULL) {
		return 1;
	}
	return 0;
}

static uint8_t Simcom_Send_Direct(char *data, uint16_t data_length, uint8_t is_payload, uint32_t ms, char *res) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	char CTRL_Z = 0x1A;
	uint8_t ret;
	uint32_t tick, timeout_tick = 0;

	// when not read command payload
	if (strstr(data, SIMCOM_READ_COMMAND) == NULL) {
		// check if it has new command
		if (Simcom_Check_Command()) {
			SWV_SendStrLn("\nNew Command: from Simcom_Send_Direct()");
			xTaskNotify(CommandTaskHandle, EVENT_COMMAND_ARRIVED, eSetBits);
		}
	}
	// reset rx buffer
	SIMCOM_Reset_Buffer();
	// transmit to serial (low-level)
	SIMCOM_Transmit(data, data_length);
	if (is_payload) {
		// terminate payload mode with CTRL+Z
		SIMCOM_Transmit(&CTRL_Z, 1);
	}
	// convert time to tick
	timeout_tick = pdMS_TO_TICKS(ms + NET_EXTRA_TIME_MS);
	// set timeout guard
	tick = osKernelSysTick();
	// wait response from SIMCOM
	while (1) {
		if (Simcom_Response(res) ||
				Simcom_Response(SIMCOM_STATUS_ERROR) ||
				Simcom_Response(SIMCOM_STATUS_READY) ||
				(osKernelSysTick() - tick) >= timeout_tick) {
			// set flag for timeout & error
			ret = Simcom_Response(res);
			// exit loop
			break;
		}
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}

static uint8_t Simcom_Send_Indirect(char *data, uint16_t data_length, uint8_t is_payload, uint32_t ms, char *res, uint8_t n) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	uint8_t ret = 0, seq = 1, p;
	// default response
	if (res == NULL) {
		res = SIMCOM_STATUS_OK;
	}

	// repeat command until desired response
	while (seq <= n) {
		// print command for debugger
		if (!is_payload) {
			SWV_SendStr("\n=> ");
			SWV_SendBuf(data, data_length);
		} else {
			SWV_SendBufHex(data, data_length);
		}
		SWV_SendChar('\n');

		// send command
		p = Simcom_Send_Direct(data, data_length, is_payload, ms, res);

		// print response for debugger
		SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
		SWV_SendChar('\n');

		// handle response match
		if (p) {
			ret = 1;

			// exit loop
			break;
		}

		// handle sequence overflow
		if (seq == n) {
			break;
		}

		// execute command every timeout guard elapsed
		osDelay(NET_REPEAT_DELAY * 1000);
		// increment sequence
		seq++;
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}

static uint8_t Simcom_Command_Match(char *cmd, uint32_t ms, char *res, uint8_t n) {
	return Simcom_Send_Indirect(cmd, strlen(cmd), 0, ms, res, n);
}

static uint8_t Simcom_Command(char *cmd, uint32_t ms) {
	return Simcom_Command_Match(cmd, ms, NULL, 1);
}

static uint8_t Simcom_Payload(char *payload, uint16_t payload_length) {
	return Simcom_Send_Indirect(payload, payload_length, 1, 20000, NULL, 1);
}

void Simcom_Init(void) {
	uint8_t p;

	// FIXME: should use hierarchy algorithm in error handling
	// this do-while is complicated, but it doesn't use recursive function, so it's stack safe
	do {
		// show previous response
		SWV_SendStr("\n========================================\n");
		SWV_SendStr("Before: Simcom_Init()");
		SWV_SendStr("\n----------------------------------------\n");
		SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
		SWV_SendStr("\n========================================\n");

		SWV_SendStrLn("Simcom_Init");

		p = 0;
		//set default value to variable
		Simcom_Prepare();
		// booting
		p = Simcom_Boot();
		// Execute only on first setup
		if (p) {
			// disable command echo
			p = Simcom_Command("ATE0\r", 500);
		}
		// =========== OTHERS CONFIGURATION
		//Hide “+IPD” header
		if (p) {
			p = Simcom_Command("AT+CIPHEAD=0\r", 500);
		}
		//Hide “RECV FROM” header
		if (p) {
			p = Simcom_Command("AT+CIPSRIP=0\r", 500);
		}
		//Set to Single IP Connection (Backend)
		if (p) {
			p = Simcom_Command("AT+CIPMUX=0\r", 500);
		}
		//Select TCPIP application mode (0: Non Transparent (command mode), 1: Transparent (data mode))
		if (p) {
			p = Simcom_Command("AT+CIPMODE=0\r", 500);
		}
		// Get data from network manually
		if (p) {
			p = Simcom_Command("AT+CIPRXGET=1\r", 500);
		}
		// =========== NETWORK CONFIGURATION
		// Check SIM Card
		if (p) {
			p = Simcom_Command("AT+CPIN?\r", 500);
		}
		// Disable presentation of <AcT>&<rac> at CREG and CGREG
		if (p) {
			p = Simcom_Command("AT+CSACT=0,0\r", 500);
		}

		// Set signal Generation 2G/3G/AUTO
		if (p) {
			p = Simcom_Command_Match(simcom.CMD_CNMP, 10000, NULL, 2);
		}

		// Network Registration Status
		if (p) {
			p = Simcom_Command("AT+CREG=2\r", 500);
		}
		// Network GPRS Registration Status
		if (p) {
			p = Simcom_Command("AT+CGREG=2\r", 500);
		}
		//Attach to GPRS service
		if (p) {
			p = Simcom_Command_Match("AT+CGATT?\r", 10000, "+CGATT: 1", 3);
		}

		// =========== TCP/IP CONFIGURATION
		//Set type of authentication for PDP-IP connections of socket
		if (p) {
			p = Simcom_Command_Match(simcom.CMD_CSTT, 500, NULL, 5);
		}
		// Bring Up Wireless Connection with GPRS
		if (p) {
			p = Simcom_Command_Match("AT+CIICR\r", 5000, NULL, 5);
		}
		// Check IP Address
		if (p) {
			Simcom_Command("AT+CIFSR\r", 500);
		}

		// Establish connection with server
		if (p) {
			p = Simcom_Command_Match(simcom.CMD_CIPSTART, 10000, "CONNECT", 1);
			// check either connection ok / error
			if (p) {
				p = Simcom_Response("CONNECT OK");
			}
		}

		// restart module to fix it
		if (!p) {
			// disable all connection
			Simcom_Command("AT+CIPSHUT\r", 500);
		}
	} while (p == 0);
}

uint8_t Simcom_Upload(char *payload, uint16_t payload_length) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	uint8_t ret = 0;
	// confirm to server that command is executed
	if (Simcom_Command_Match("AT+CIPSEND\r", 5000, SIMCOM_STATUS_SEND, 1)) {
		// send response
		if (Simcom_Payload(payload, payload_length)) {
			ret = 1;
		}
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}

uint8_t Simcom_Check_Command(void) {
	// check if it has new command
	return Simcom_Response("+CIPRXGET: 1");
}

uint8_t Simcom_Read_Command(command_t *command) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	uint32_t crcValue;
	uint8_t ret = 0;
	char *start;

	if (Simcom_Command_Match(SIMCOM_READ_COMMAND, 500, NET_COMMAND_PREFIX, 1)) {
		// check is command not empty
		if (!Simcom_Response("CIPRXGET: 2,0")) {
			// get pointer reference
			start = strstr(SIMCOM_UART_RX_Buffer, NET_COMMAND_PREFIX);
			// copy the whole value (any time the buffer can change)
			*command = *(command_t*) start;

			// check the Size
			if (command->header.size == sizeof(command->data)) {
				// calculate the CRC
				crcValue = CRC_Calculate8((uint8_t*) &(command->header.size),
						sizeof(command->header.size) + sizeof(command->data), 1);

				// check the CRC
				if (command->header.crc == crcValue) {
					// response OK
					ret = 1;
				}
			}

			// reset rx buffer
			SIMCOM_Reset_Buffer();
		}
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}

uint8_t Simcom_Read_Signal(uint8_t *signal) {
	osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

	uint8_t ret = 0, rssi = 0;
	char *prefix = "+CSQ: ";
	char *start, *end;
	char str[3];
	// check signal quality
	if (Simcom_Command("AT+CSQ\r", 500)) {
		// get pointer reference
		start = strstr(SIMCOM_UART_RX_Buffer, prefix);
		end = strchr(start, ',');

		// get rssi string
		strncpy(str, start + strlen(prefix), end - (start + strlen(prefix)));
		*(str + (end - (start + strlen(prefix)))) = '\0';

		// convert rssi to integer
		rssi = atoi(str);

		// handle not detectable rssi value
		rssi = (rssi == 99 ? 0 : rssi);

		// convert rssi value to percentage
		*signal = (rssi * 100) / 31;

		// debugging
		SWV_SendStr("SIGNAL_QUALITY = ");
		SWV_SendInt(*signal);
		SWV_SendStrLn(" %");

		ret = 1;
	}

	osRecursiveMutexRelease(SimcomRecMutexHandle);
	return ret;
}
