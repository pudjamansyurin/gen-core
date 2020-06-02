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
#include "Drivers/_at.h"
#include "Nodes/VCU.h"

/* External variables ---------------------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];
extern osMutexId_t SimcomRecMutexHandle;
extern osMessageQueueId_t CommandQueueHandle;
extern vcu_t VCU;

/* Public variables ----------------------------------------------------------*/
sim_t SIM = {
        .state = SIM_STATE_DOWN,
};

/* Private variables ----------------------------------------------------------*/
static AT_CIPSTATUS ipStatus;

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT Simcom_Ready(void);
static SIMCOM_RESULT Simcom_Power(void);
static SIMCOM_RESULT Simcom_ProcessInstruction(command_t *command);
static SIMCOM_RESULT Simcom_ProcessACK(header_t *header);
static SIMCOM_RESULT Simcom_Execute(char *data, uint16_t size, uint32_t ms, char *res);
static void Simcom_Sleep(uint8_t state);
static void Simcom_ClearBuffer(void);

/* Public functions implementation --------------------------------------------*/
void Simcom_Lock(void) {
    osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
}

void Simcom_Unlock(void) {
    osMutexRelease(SimcomRecMutexHandle);
}

char* Simcom_Response(char *str) {
    return strstr(SIMCOM_UART_RX, str);
}

uint8_t Simcom_SetState(SIMCOM_STATE state) {
    static uint8_t init = 1;
    const uint8_t maxDepth = 3;
    uint8_t iteration, depth = maxDepth;
    SIMCOM_STATE lastState = SIM_STATE_DOWN;
    SIMCOM_RESULT p = SIM_RESULT_OK;

    Simcom_Lock();
    // Handle SIMCOM state properly
    do {
        // Handle locked-loop
        if (SIM.state < lastState) {
            if (!--depth) {
                depth = maxDepth;
                SIM.state = SIM_STATE_DOWN;
            }
            LOG_Str("Simcom:LockedLoop = ");
            LOG_Int(depth);
            LOG_Enter();
        }
        lastState = SIM.state;

        // Handle signal
        if (SIM.state == SIM_STATE_DOWN) {
            VCU.d.signal = 0;
        } else {
            Simcom_IdleJob(NULL);
            if (SIM.state >= SIM_STATE_GPRS_ON) {
                // Force to exit loop
                if (VCU.d.signal < 15) {
                    LOG_StrLn("Simcom:SignalPoor");
                    break;
                }
            }
        }

        // handle simcom states
        switch (SIM.state) {
            case SIM_STATE_DOWN:
                // only executed at power up
                if (init) {
                    init = 0;
                    LOG_StrLn("Simcom:Init");
                } else {
                    LOG_StrLn("Simcom:Restarting...");
                }

                // power up the module
                p = Simcom_Power();
                // upgrade simcom state
                if (p) {
                    SIM.state++;
                    LOG_StrLn("Simcom:ON");
                } else {
                    LOG_StrLn("Simcom:Error");
                }

                osDelay(500);
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
                    p = Simcom_Command("AT+CPIN?\r", "READY", 500, 0);
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

                osDelay(500);
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
                                if (iteration < NET_REPEAT_MAX) {
                                    Simcom_IdleJob(&iteration);
                                    osDelay(NET_REPEAT_DELAY);
                                } else {
                                    p = SIM_RESULT_ERROR;
                                    break;
                                }
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

                osDelay(500);
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
                                if (iteration < NET_REPEAT_MAX) {
                                    Simcom_IdleJob(&iteration);
                                    osDelay(NET_REPEAT_DELAY);
                                } else {
                                    p = SIM_RESULT_ERROR;
                                    break;
                                }
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

                osDelay(500);
                break;
            case SIM_STATE_GPRS_ON:
                // =========== PDP CONFIGURATION
                // Attach to GPRS service
                if (p) {
                    AT_CGATT state = 0;

                    // wait until attached
                    while (p && !state) {
                        p = AT_GprsAttachment(ATR, &state);

                        osDelay(1000);
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

                osDelay(500);
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
                    p = Simcom_Command("AT+CIICR\r", NULL, 20000, 0);
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
                    // Check IP Status
                    AT_ConnectionStatusSingle(&ipStatus);

                    // Close PDP
                    if (ipStatus != CIPSTAT_IP_INITIAL &&
                            ipStatus != CIPSTAT_PDP_DEACT) {
                        p = Simcom_Command("AT+CIPSHUT\r", NULL, 1000, 0);
                    }

                    if (SIM.state == SIM_STATE_PDP_ON) {
                        SIM.state--;
                    }
                }

                osDelay(500);
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
                    // Check IP Status
                    AT_ConnectionStatusSingle(&ipStatus);

                    // Close IP
                    if (ipStatus == CIPSTAT_CONNECT_OK) {
                        p = Simcom_Command("AT+CIPCLOSE\r", NULL, 1000, 0);
                    }

                    if (SIM.state == SIM_STATE_INTERNET_ON) {
                        SIM.state--;
                    }
                }

                osDelay(500);
                break;
            case SIM_STATE_SERVER_ON:
                // Check IP Status
                AT_ConnectionStatusSingle(&ipStatus);

                if (ipStatus != CIPSTAT_CONNECT_OK) {
                    if (SIM.state == SIM_STATE_SERVER_ON) {
                        SIM.state--;
                    }
                }

                break;
            default:
                break;
        }
    } while (SIM.state < state);
    Simcom_Unlock();

    return SIM.state >= state;
}

SIMCOM_RESULT Simcom_Upload(void *payload, uint16_t size) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint32_t tick;
    char str[20];
    command_t hCommand;
    header_t *hHeader = NULL;

    Simcom_Lock();
    // combine the size
    sprintf(str, "AT+CIPSEND=%d\r", size);

    // Check IP Status
    AT_ConnectionStatusSingle(&ipStatus);

    if (SIM.state >= SIM_STATE_SERVER_ON && ipStatus == CIPSTAT_CONNECT_OK) {
        // send command
        p = Simcom_Command(str, SIMCOM_RSP_SEND, 5000, 0);
        if (p) {
            // send the payload
            p = Simcom_Command((char*) payload, SIMCOM_RSP_SENT, 20000, size);
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

                // handle instruction (if any)
                if (Simcom_ProcessInstruction(&hCommand)) {
                    osMessageQueuePut(CommandQueueHandle, &hCommand, 0U, 0U);
                }

            } else {
                p = SIM_RESULT_NACK;
            }
        }
    }

    Simcom_Unlock();
    return p;
}

SIMCOM_RESULT Simcom_Command(char *data, char *res, uint32_t ms, uint16_t size) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint8_t upload = 1;

    // Handle default value
    if (!res) {
        res = SIMCOM_RSP_OK;
    }
    if (!size) {
        upload = 0;
        size = strlen(data);
    }

    // only handle command if SIM_STATE_READY or BOOT_CMD
    if (SIM.state >= SIM_STATE_READY || (!strcmp(data, SIMCOM_CMD_BOOT))) {
        Simcom_Lock();

//        // Debug: print command
//        if (!upload) {
//            LOG_Str("\n=> ");
//            LOG_Buf(data, size);
//        } else {
//            LOG_BufHex(data, size);
//        }
//        LOG_Enter();

        // send command
        p = Simcom_Execute(data, size, ms, res);

//        // Debug: print response
//        if (!upload) {
//            LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
//            LOG_Enter();
//        }

        Simcom_Unlock();
    }
    return p;
}

SIMCOM_RESULT Simcom_IdleJob(uint8_t *iteration) {
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
        VCU.d.signal = signal.percent;
    }
    AT_ConnectionStatusSingle(&ipStatus);

    return p;
}

/* Private functions implementation --------------------------------------------*/
static SIMCOM_RESULT Simcom_ProcessInstruction(command_t *command) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint32_t crcValue;
    char *str = NULL;

    Simcom_Lock();
    if (Simcom_Response(SIMCOM_RSP_IPD)) {
        str = Simcom_Response(PREFIX_ACK);
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

static SIMCOM_RESULT Simcom_ProcessACK(header_t *header) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    ack_t ack;
    char *str = NULL;

    Simcom_Lock();
    if (Simcom_Response(SIMCOM_RSP_IPD)) {
        // parse ACK
        str = Simcom_Response(PREFIX_ACK);
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

static SIMCOM_RESULT Simcom_Ready(void) {
    uint32_t tick;

    // save event
    VCU.SetEvent(EV_VCU_NETWORK_RESTART, 1);

    // wait until 1s response
    tick = osKernelGetTickCount();
    while (SIM.state == SIM_STATE_DOWN) {
        if (Simcom_Response(SIMCOM_RSP_READY) ||
                //				Simcom_Response(SIMCOM_RSP_OK) ||
                (osKernelGetTickCount() - tick) >= NET_BOOT_TIMEOUT) {
            break;
        }
        osDelay(1);
    }

    // check
    return Simcom_Command(SIMCOM_CMD_BOOT, SIMCOM_RSP_READY, 1000, 0);
}

static SIMCOM_RESULT Simcom_Power(void) {
    LOG_StrLn("Simcom:Powered");
    // reset buffer
    SIMCOM_Reset_Buffer();

    // power control
    HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, 0);
    osDelay(100);
    HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, 1);
    osDelay(100);

    // simcom reset pin
    HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, 1);
    HAL_Delay(1);
    HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, 0);

    // wait response
    return Simcom_Ready();
}

static void Simcom_Sleep(uint8_t state) {
    HAL_GPIO_WritePin(INT_NET_DTR_GPIO_Port, INT_NET_DTR_Pin, state);
    osDelay(50);
}

static SIMCOM_RESULT Simcom_Execute(char *data, uint16_t size, uint32_t ms, char *res) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint32_t tick, timeout_tick = 0;

    Simcom_Lock();
    // wake-up the SIMCOM
    Simcom_Sleep(0);
    // transmit to serial (low-level)
    Simcom_ClearBuffer();
    SIMCOM_Transmit(data, size);
    // convert time to tick
    timeout_tick = pdMS_TO_TICKS(ms + NET_EXTRA_TIME);
    // set timeout guard
    tick = osKernelGetTickCount();

    // wait response from SIMCOM
    while (1) {
        if (Simcom_Response(res)
                || Simcom_Response(SIMCOM_RSP_ERROR)
                || Simcom_Response(SIMCOM_RSP_READY)
                || (osKernelGetTickCount() - tick) >= timeout_tick) {

            // check response
            if (Simcom_Response(res)) {
                p = SIM_RESULT_OK;
            }

            // Handle failure
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
                        VCU.SetEvent(EV_VCU_NETWORK_RESTART, 1);

                        LOG_StrLn("Simcom:Restarted");
                    } else if ((osKernelGetTickCount() - tick) >= timeout_tick) {
                        // exception for timeout

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

    // sleep the SIMCOM
    Simcom_Sleep(1);
    Simcom_Unlock();
    return p;
}

static void Simcom_ClearBuffer(void) {
    // handle things on every request
    //	LOG_StrLn("============ SIMCOM DEBUG ============");
    //	LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
    //	LOG_Enter();
    //	LOG_StrLn("======================================");

    // reset rx buffer
    SIMCOM_Reset_Buffer();
}
