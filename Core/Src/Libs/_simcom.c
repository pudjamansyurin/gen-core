/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "Libs/_simcom.h"
#include "DMA/_dma_simcom.h"
#include "Drivers/_crc.h"
#include "Nodes/VCU.h"
#include "Parser/_at.h"

/* External variables ---------------------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];
extern osMutexId_t SimcomRecMutexHandle;
extern osMessageQueueId_t CommandQueueHandle;
extern vcu_t VCU;

/* Public variables ----------------------------------------------------------*/
sim_t SIM;

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT Simcom_Ready(void);
static SIMCOM_RESULT Simcom_Power(void);
static SIMCOM_RESULT Simcom_Reset(void);
static SIMCOM_RESULT Simcom_IdleJob(uint8_t *iteration);
static SIMCOM_RESULT Simcom_SendDirect(char *data, uint16_t len, uint32_t ms, char *res);
static SIMCOM_RESULT Simcom_SendIndirect(char *data, uint16_t len, uint8_t is_payload, uint32_t ms, char *res, uint8_t n);
static void Simcom_ClearBuffer(void);

/* Public functions implementation --------------------------------------------*/
void Simcom_Lock(void) {
	osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
}

void Simcom_Unlock(void) {
	osMutexRelease(SimcomRecMutexHandle);
}

void Simcom_Sleep(uint8_t state) {
	HAL_GPIO_WritePin(INT_NET_DTR_GPIO_Port, INT_NET_DTR_Pin, state);
	osDelay(50);
}

SIMCOM_RESULT Simcom_Response(char *str) {
	if (strstr(SIMCOM_UART_RX, str)) {
		return SIM_RESULT_OK;
	}
	return SIM_RESULT_ERROR;
}

void Simcom_SetState(SIMCOM_STATE state) {
	Simcom_Lock();

	SIMCOM_RESULT p;
	static uint8_t init = 1;
	uint8_t iteration;

	do {
		// only executed at power up
		if (init) {
			SIM.state = SIM_STATE_DOWN;
			LOG_StrLn("Simcom:Init");

			p = Simcom_Power();
		} else {
			if (SIM.state == SIM_STATE_DOWN) {
				VCU.d.signal_percent = 0;
				LOG_StrLn("Simcom:Down");

				p = SIM_RESULT_ERROR;
			} else {
				if (SIM.state >= SIM_STATE_CONFIGURED) {
					Simcom_IdleJob(NULL);
					if (VCU.d.signal_percent < 20) {
						LOG_StrLn("Simcom:PendingBySignal");
						osDelay(5 * 1000);
						break;
					}
				}

				p = SIM_RESULT_OK;
			}
		}

		// handle simcom states
		switch (SIM.state) {
			case SIM_STATE_DOWN:
				// reset buffer
				SIMCOM_Reset_Buffer();
				if (p != SIM_RESULT_OK) {
					p = Simcom_Reset();
					// force reboot
					//      if (p != SIM_RESULT_OK) {
					//        p = Simcom_Power();
					//      }
				}
				// upgrade simcom state
				if (p) {
					SIM.state++;
				} else {
					LOG_StrLn("Simcom:Error");
				}

				break;
			case SIM_STATE_READY:
				// =========== BASIC CONFIGURATION
				// disable command echo
				if (p) {
					p = AT_CommandEchoMode(0);
				}
				// Set serial baud-rate
				if (p) {
					uint32_t rate = 0;
					p = AT_FixedLocalRate(ATW, &rate);
				}
				// Error report format: 0, 1(Numeric), 2(verbose)
				if (p) {
					AT_CMEE state = CMEE_VERBOSE;
					p = AT_ReportMobileEquipmentError(ATW, &state);
				}
				// Use pin DTR as sleep control
				if (p) {
					AT_CSCLK state = CSCLK_EN_DTR;
					p = AT_ConfigureSlowClock(ATW, &state);
				}
				// Enable time reporting
				if (p) {
					AT_BOOL state = AT_ENABLE;
					p = AT_GetLocalTimestamp(ATW, &state);
				}
				// Enable “+IPD” header
				if (p) {
					AT_BOOL state = AT_ENABLE;
					p = AT_IpPackageHeader(ATW, &state);
				}
				// Disable “RECV FROM” header
				if (p) {
					AT_BOOL state = AT_DISABLE;
					p = AT_ShowRemoteIp(ATW, &state);
				}
				// =========== NETWORK CONFIGURATION
				// Check SIM Card
				if (p) {
					p = Simcom_Command("AT+CPIN?\r", 500, 1, "READY");
				}
				// Disable presentation of <AcT>&<rac> at CREG and CGREG
				if (p) {
					at_csact_t param = {
							.creg = 0,
							.cgreg = 0,
					};
					p = AT_NetworkAttachedStatus(ATW, &param);
				}

				// upgrade simcom state
				if (p) {
					SIM.state++;
				}

				break;
			case SIM_STATE_CONFIGURED:
				// =========== NETWORK ATTACH
				// Set signal Generation 2G(13)/3G(14)/AUTO(2)
				if (p) {
					at_cnmp_t param = {
							.mode = CNMP_ACT_AUTO,
							.preferred = CNMP_ACT_P_UMTS
					};
					p = AT_RadioAccessTechnology(ATW, &param);
				}
				// Network Registration Status
				if (p) {
					at_c_greg_t param = {
							.mode = CREG_MODE_DISABLE,
							.stat = CREG_STAT_REG_HOME
					};
					p = AT_NetworkRegistration(ATW, &param);

					// wait until attached
					if (p) {
						iteration = 0;
						while (param.stat != CREG_STAT_REG_HOME) {
							p = AT_NetworkRegistration(ATW, &param);

							if (p) {
								Simcom_IdleJob(&iteration);
								osDelay(NET_REPEAT_DELAY);
							} else {
								break;
							}
						}
					}
				}

				// upgrade simcom state
				if (p) {
					SIM.state++;
				}

				break;
			case SIM_STATE_NETWORK_ON:
				// =========== GPRS ATTACH
				// GPRS Registration Status
				if (p) {
					at_c_greg_t param = {
							.mode = CREG_MODE_DISABLE,
							.stat = CREG_STAT_REG_HOME
					};
					p = AT_NetworkRegistrationStatus(ATW, &param);

					// wait until attached
					if (p) {
						iteration = 0;
						while (param.stat != CREG_STAT_REG_HOME) {
							p = AT_NetworkRegistrationStatus(ATW, &param);

							if (p) {
								Simcom_IdleJob(&iteration);
								osDelay(NET_REPEAT_DELAY);
							} else {
								break;
							}
						}
					}
				}

				// upgrade simcom state
				if (p) {
					SIM.state++;
				} else {
					if (SIM.state == SIM_STATE_NETWORK_ON) {
						SIM.state--;
					}
				}

				break;
			case SIM_STATE_GPRS_ON:
				// =========== PDP CONFIGURATION
				// Attach to GPRS service
				if (p) {
					AT_CGATT state = 1;
					p = AT_GprsAttachment(ATW, &state);

					// wait until attached
					if (p) {
						iteration = 0;
						while (state != CGATT_ATTACHED) {
							p = AT_GprsAttachment(ATW, &state);

							if (p) {
								Simcom_IdleJob(&iteration);
								osDelay(NET_REPEAT_DELAY);
							} else {
								break;
							}
						}
					}
				}

				// Select TCPIP application mode:
				// (0: Non Transparent (command mode), 1: Transparent (data mode))
				if (p) {
					AT_CIPMODE state = CIPMODE_NORMAL;
					p = AT_TcpApllicationMode(ATW, &state);
				}
				// Set to Single IP Connection (Backend)
				if (p) {
					AT_CIPMUX state = CIPMUX_SINGLE_IP;
					p = AT_MultiIpConnection(ATW, &state);
				}
				// Get data from network automatically
				if (p) {
					AT_CIPRXGET state = CIPRXGET_DISABLE;
					p = AT_ManuallyReceiveData(ATW, &state);
				}

				// upgrade simcom state
				if (p) {
					SIM.state++;
				} else {
					if (SIM.state == SIM_STATE_GPRS_ON) {
						SIM.state--;
					}
				}

				break;
			case SIM_STATE_PDP_ON:
				// =========== PDP ATTACH
				// Set type of authentication for PDP connections of socket
				if (p) {
					at_cstt_t param = {
							.apn = "3gprs",								// "telkomsel"
							.username = "3gprs",					// "wap"
							.password = "3gprs",					// "wap123"
							};
					p = AT_ConfigureAPN(ATW, &param);
				}
				// =========== IP ATTACH
				// Bring Up IP Connection
				if (p) {
					p = Simcom_Cmd("AT+CIICR\r", 20000, 3);
				}
				// Check IP Address
				if (p) {
					at_cifsr_t param;
					p = AT_GetLocalIpAddress(&param);
				}

				// upgrade simcom state
				if (p) {
					SIM.state++;
				} else {
					// Close PDP
					p = Simcom_Cmd("AT+CIPSHUT\r", 1000, 1);

					if (SIM.state == SIM_STATE_PDP_ON) {
						SIM.state--;
					}
				}

				break;
			case SIM_STATE_INTERNET_ON:
				// ============ SOCKET CONFIGURATION
				// Establish connection with server
				if (p) {
					at_cipstart_t param = {
							.mode = "TCP",
							.ip = "pujakusumae-31974.portmap.io",
							.port = 31974
					};
					p = AT_StartConnectionSingle(&param);
				}

				// upgrade simcom state
				if (p) {
					SIM.state++;
				} else {
					// Close IP
					p = Simcom_Cmd("AT+CIPCLOSE\r", 1000, 1);

					if (SIM.state == SIM_STATE_INTERNET_ON) {
						SIM.state--;
					}
				}

				break;
			case SIM_STATE_SERVER_ON:

				break;
			default:
				break;
		}

		// delay on failure
		if (p != SIM_RESULT_OK) {
			osDelay(1000);
		}

		init = 0;
	} while (SIM.state < state);

	Simcom_Unlock();
}

SIMCOM_RESULT Simcom_Upload(void *payload, uint16_t size, uint8_t *retry) {
	Simcom_Lock();

	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint32_t tick;
	char str[20];
	command_t hCommand;
	header_t *hHeader = NULL;

	// combine the size
	sprintf(str, "AT+CIPSEND=%d\r", size);

	if (SIM.state >= SIM_STATE_SERVER_ON) {
		// wake-up the SIMCOM
		Simcom_Sleep(0);
		SIM.uploading = 1;

		// send command
		p = Simcom_Command(str, 5000, 1, SIMCOM_RSP_SEND);
		if (p) {
			// send the payload
			p = Simcom_SendIndirect((char*) payload, size, 1, 30000, SIMCOM_RSP_SENT, 1);
			// wait for ACK/NACK
			if (p) {
				// set timeout guard
				tick = osKernelGetTickCount();
				// wait ACK for payload
				while (SIM.state >= SIM_STATE_SERVER_ON) {
					if (Simcom_Response(PREFIX_ACK) ||
							Simcom_Response(PREFIX_NACK) ||
							(osKernelGetTickCount() - tick) >= pdMS_TO_TICKS(30000)) {
						break;
					}
					osDelay(10);
				}

				// debug
				LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
				LOG_Enter();

				// exception handler
				if (Simcom_Response(PREFIX_ACK)) {
					p = SIM_RESULT_ACK;
				} else if (Simcom_Response(PREFIX_NACK)) {
					p = SIM_RESULT_NACK;
				} else {
					p = SIM_RESULT_TIMEOUT;
				}
			}
		}

		// handle SIMCOM result
		if (p == SIM_RESULT_ACK) {
			// validate ACK
			hHeader = (header_t*) payload;
			if (Simcom_ProcessACK(hHeader)) {
				p = SIM_RESULT_OK;

				// handle command (if any)
				if (Simcom_ProcessCommand(&hCommand)) {
					osMessageQueuePut(CommandQueueHandle, &hCommand, 0U, 0U);
				}

			} else {
				p = SIM_RESULT_NACK;
			}
		} else {
			// handle communication failure
			if (SIM.state == SIM_STATE_SERVER_ON) {
				if (p != SIM_RESULT_NACK) {
					// handle failure properly
					switch ((*retry)++) {
						case 1:
							SIM.state = SIM_STATE_INTERNET_ON;

							// Check IP Status
							if (Simcom_Command("AT+CIPSTATUS\r", 500, 1, "TCP CLOSED")) {
								// exit the loop
								*retry = SIMCOM_MAX_UPLOAD_RETRY + 1;
							} else {
								// try closing the IP
								Simcom_Cmd("AT+CIPCLOSE\r", 500, 1);
							}

							break;
						case 2:
							// try closing the PDP
							SIM.state = SIM_STATE_PDP_ON;
							Simcom_Cmd("AT+CIPSHUT\r", 1000, 1);

							break;
						case 3:
							// try reset the module
							SIM.state = SIM_STATE_DOWN;

							break;
						default:
							break;
					}
				}
			}
		}

		// sleep the SIMCOM
		Simcom_Sleep(1);
		SIM.uploading = 0;
	}

	Simcom_Unlock();
	return p;
}

SIMCOM_RESULT Simcom_Cmd(char *cmd, uint32_t ms, uint8_t n) {
	return Simcom_Command(cmd, ms, n, NULL);
}

SIMCOM_RESULT Simcom_Command(char *cmd, uint32_t ms, uint8_t n, char *res) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;

	// wake-up the SIMCOM
	if (!SIM.uploading) {
		Simcom_Sleep(0);
	}

	// only handle command if BOOT_CMD or SIM_STATE_READY
	if ((strstr(cmd, SIMCOM_CMD_BOOT) != NULL) || SIM.state >= SIM_STATE_READY) {
		p = Simcom_SendIndirect(cmd, strlen(cmd), 0, ms, res, n);
	}

	// sleep the SIMCOM
	if (!SIM.uploading) {
		Simcom_Sleep(1);
	}

	return p;
}

SIMCOM_RESULT Simcom_ProcessCommand(command_t *command) {
	Simcom_Lock();

	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint32_t crcValue;
	char *str = NULL;

	if (Simcom_Response(SIMCOM_RSP_IPD)) {
		str = strstr(SIMCOM_UART_RX, PREFIX_ACK);
		// get pointer reference
		str = strstr(str + sizeof(ack_t), PREFIX_COMMAND);
		if (str != NULL) {
			// copy the whole value (any time the buffer can change)
			*command = *(command_t*) str;

			// check the Size
			if (command->header.size == sizeof(command->data)) {
				// calculate the CRC
				crcValue = CRC_Calculate8((uint8_t*) &(command->header.size),
						sizeof(command->header.size) + sizeof(command->data));

				// check the CRC
				if (command->header.crc == crcValue) {
					p = SIM_RESULT_OK;
				}
			}
		}
	}

	Simcom_Unlock();
	return p;
}

SIMCOM_RESULT Simcom_ProcessACK(header_t *header) {
	Simcom_Lock();

	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	ack_t ack;
	char *str = NULL;

	if (Simcom_Response(SIMCOM_RSP_IPD)) {
		// parse ACK
		str = strstr(SIMCOM_UART_RX, PREFIX_ACK);
		if (str != NULL) {
			ack = *(ack_t*) str;

			// validate the value
			if (header->frame_id == ack.frame_id &&
					header->seq_id == ack.seq_id) {
				p = SIM_RESULT_OK;
			}
		}
	}

	Simcom_Unlock();
	return p;
}

/* Private functions implementation --------------------------------------------*/
static SIMCOM_RESULT Simcom_Ready(void) {
	uint32_t tick;

	// save event
	VCU.SetEvent(EV_VCU_NETWORK_RESTART, 1);

	// wait until 1s response
	tick = osKernelGetTickCount();
	while (SIM.state == SIM_STATE_DOWN) {
		if (Simcom_Response(SIMCOM_RSP_OK) ||
				Simcom_Response(SIMCOM_RSP_READY) ||
				(osKernelGetTickCount() - tick) >= NET_BOOT_TIMEOUT) {
			break;
		}
		osDelay(1);
	}

	// check
	return Simcom_Command(SIMCOM_CMD_BOOT, 1000, 1, SIMCOM_RSP_READY);
	//  return Simcom_Cmd(SIMCOM_CMD_BOOT, 1000, 1);
}

static SIMCOM_RESULT Simcom_Power(void) {
	LOG_StrLn("Simcom:Powered");
	// power control
	HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, 0);
	osDelay(3000);
	HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, 1);
	osDelay(5000);
	// wait response
	return Simcom_Ready();
}

static SIMCOM_RESULT Simcom_Reset(void) {
	LOG_StrLn("Simcom:Reset");
	// simcom reset pin
	HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, 1);
	HAL_Delay(1);
	HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, 0);
	// wait response
	return Simcom_Ready();
}

static SIMCOM_RESULT Simcom_IdleJob(uint8_t *iteration) {
	SIMCOM_RESULT p;
	at_csq_t signal;

	// debug
	if (iteration != NULL) {
		LOG_Str("Simcom:Iteration = ");
		LOG_Int((*iteration)++);
		LOG_Enter();
	}

	// other routines
	p = AT_SignalQualityReport(&signal);
	if (p) {
		VCU.d.signal_percent = signal.percent;
	}

	return p;
}

static SIMCOM_RESULT Simcom_SendDirect(char *data, uint16_t len, uint32_t ms, char *res) {
	Simcom_Lock();

	SIMCOM_RESULT p;
	uint32_t tick, timeout_tick = 0;

	Simcom_ClearBuffer();
	// transmit to serial (low-level)
	SIMCOM_Transmit(data, len);
	// convert time to tick
	timeout_tick = pdMS_TO_TICKS(ms + NET_EXTRA_TIME_MS);
	// set timeout guard
	tick = osKernelGetTickCount();
	// wait response from SIMCOM
	while (1) {
		if (Simcom_Response(res) ||
				Simcom_Response(SIMCOM_RSP_ERROR) ||
				Simcom_Response(SIMCOM_RSP_READY) ||
				(osKernelGetTickCount() - tick) >= timeout_tick) {

			// set flag for timeout & error
			p = Simcom_Response(res);

			if (p != SIM_RESULT_OK) {
				// exception for no response
				if (strlen(SIMCOM_UART_RX) == 0) {
					p = SIM_RESULT_NO_RESPONSE;
					SIM.state = SIM_STATE_DOWN;
					LOG_StrLn("Simcom:NoResponse");
				} else {
					// exception for auto reboot module
					if (Simcom_Response(SIMCOM_RSP_READY) && (SIM.state >= SIM_STATE_READY)) {
						p = SIM_RESULT_RESTARTED;
						SIM.state = SIM_STATE_READY;
						LOG_StrLn("Simcom:Restarted");
						// save event
						VCU.SetEvent(EV_VCU_NETWORK_RESTART, 1);
					}
					// exception for timeout
					if ((osKernelGetTickCount() - tick) >= timeout_tick) {
						p = SIM_RESULT_TIMEOUT;
						LOG_StrLn("Simcom:Timeout");
					}
				}
			}

			// exit loop
			break;
		}
		osDelay(10);
	}

	Simcom_Unlock();
	return p;
}

static SIMCOM_RESULT Simcom_SendIndirect(char *data, uint16_t len, uint8_t is_payload, uint32_t ms, char *res,
		uint8_t n) {
	Simcom_Lock();

	SIMCOM_RESULT p;
	uint8_t seq;
	// default response
	if (res == NULL) {
		res = SIMCOM_RSP_OK;
	}

	// repeat command until desired response
	seq = n - 1;
	do {
		// before next iteration
		if (seq < (n - 1)) {
			// execute command every timeout guard elapsed
			osDelay(NET_REPEAT_DELAY);
		}

		// print command for debugger
		if (!is_payload) {
			LOG_Str("\n=> ");
			LOG_Buf(data, len);
		} else {
			LOG_BufHex(data, len);
		}
		LOG_Enter();

		// send command
		p = Simcom_SendDirect(data, len, ms, res);

		// print response for debugger
		if (!is_payload) {
			LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
			LOG_Enter();
		}

	} while (seq-- && p == SIM_RESULT_ERROR);

	Simcom_Unlock();
	return p;
}

static void Simcom_ClearBuffer(void) {
	// reset rx buffer
	SIMCOM_Reset_Buffer();
}
