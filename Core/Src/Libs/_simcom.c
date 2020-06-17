/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

// FIXME: if you want to use MQTT
// https://github.com/eclipse/paho.mqtt.embedded-c/blob/master/MQTTPacket/samples/transport.c
// https://github.com/eclipse/paho.mqtt.embedded-c/blob/master/MQTTPacket/samples/pub0sub1.c
/* Includes ------------------------------------------------------------------*/
#include "Libs/_simcom.h"
#include "DMA/_dma_simcom.h"
#include "Drivers/_crc.h"
#include "Drivers/_at.h"
#include "Drivers/_flasher.h"
#include "Nodes/VCU.h"

/* External variables ---------------------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];
extern osMutexId_t SimcomRecMutexHandle;
extern osMessageQueueId_t CommandQueueHandle;
extern vcu_t VCU;

/* Public variables ----------------------------------------------------------*/
sim_t SIM = {
        .state = SIM_STATE_DOWN,
        .commando = 0,
        .payload_type = PAYLOAD_REPORT,
        .ip_status = CIPSTAT_UNKNOWN,
};

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT Simcom_Ready(void);
static SIMCOM_RESULT Simcom_Power(void);
static SIMCOM_RESULT Simcom_ProcessCommando(command_t *command);
static SIMCOM_RESULT Simcom_ProcessACK(header_t *header);
static SIMCOM_RESULT Simcom_Execute(char *data, uint16_t size, uint32_t ms, char *res);
static uint8_t Simcom_CommandoIRQ(void);
static void Simcom_Sleep(uint8_t state);
static void Simcom_BeforeTransmitHook(void);

/* Public functions implementation --------------------------------------------*/
void Simcom_Lock(void) {
    osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
}

void Simcom_Unlock(void) {
    osMutexRelease(SimcomRecMutexHandle);
}

char* Simcom_Response(char *str) {
//    return memmem(SIMCOM_UART_RX, sizeof(SIMCOM_UART_RX), str, strlen(str));
    return strstr(SIMCOM_UART_RX, str);
}

uint8_t Simcom_SetState(SIMCOM_STATE state) {
    static uint8_t init = 1;
    uint8_t depth = 3;
    SIMCOM_STATE lastState = SIM_STATE_DOWN;
    SIMCOM_RESULT p;

    Simcom_Lock();
    // Handle SIMCOM state properly
    do {

        // Handle locked-loop
        if (SIM.state < lastState) {
            if (!--depth) {
                SIM.state = SIM_STATE_DOWN;
                break;
            }
            LOG_Str("Simcom:LockedLoop = ");
            LOG_Int(depth);
            LOG_Enter();
        }

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

        // Set value
        p = SIM_RESULT_OK;
        lastState = SIM.state;

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

                // disable command echo
                if (p) {
                    p = AT_CommandEchoMode(0);
                }

                osDelay(500);
                break;
            case SIM_STATE_READY:
                // =========== BASIC CONFIGURATION
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
                    p = AT_EnableLocalTimestamp(ATW, &state);
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
                    at_c_greg_t read, param = {
                            .mode = CREG_MODE_DISABLE,
                            .stat = CREG_STAT_REG_HOME
                    };
                    // wait until attached
                    do {
                        p = AT_NetworkRegistration("CREG", ATW, &param);
                        if (p) {
                            p = AT_NetworkRegistration("CREG", ATR, &read);
                        }

                        osDelay(1000);
                    } while (p && read.stat != param.stat);
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
                    at_c_greg_t read, param = {
                            .mode = CREG_MODE_DISABLE,
                            .stat = CREG_STAT_REG_HOME
                    };
                    // wait until attached
                    do {
                        p = AT_NetworkRegistration("CGREG", ATW, &param);
                        if (p) {
                            p = AT_NetworkRegistration("CGREG", ATR, &read);
                        }

                        osDelay(1000);
                    } while (p && read.stat != param.stat);
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
                    AT_CGATT state;
                    // wait until attached
                    do {
                        p = AT_GprsAttachment(ATR, &state);

                        osDelay(1000);
                    } while (p && !state);
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
                AT_ConnectionStatusSingle(&(SIM.ip_status));
                if (p && (SIM.ip_status == CIPSTAT_IP_INITIAL || SIM.ip_status == CIPSTAT_PDP_DEACT)) {
                    at_cstt_t param = {
                            .apn = NET_CON_APN,
                            .username = NET_CON_USERNAME,
                            .password = NET_CON_PASSWORD,
                    };
                    p = AT_ConfigureAPN(ATW, &param);
                }
                // =========== IP ATTACH
                // Bring Up IP Connection
                AT_ConnectionStatusSingle(&(SIM.ip_status));
                if (p && SIM.ip_status == CIPSTAT_IP_START) {
                    p = Simcom_Command("AT+CIICR\r", NULL, 10000, 0);
                }
                // Check IP Address
                AT_ConnectionStatusSingle(&(SIM.ip_status));
                if (p && (SIM.ip_status == CIPSTAT_IP_CONFIG || SIM.ip_status == CIPSTAT_IP_GPRSACT)) {
                    at_cifsr_t param;
                    p = AT_GetLocalIpAddress(&param);
                }

                // upgrade simcom state
                if (p) {
                    SIM.state++;
                } else {
                    // Check IP Status
                    AT_ConnectionStatusSingle(&(SIM.ip_status));

                    // Close PDP
                    if (SIM.ip_status != CIPSTAT_IP_INITIAL &&
                            SIM.ip_status != CIPSTAT_PDP_DEACT) {
                        p = Simcom_Command("AT+CIPSHUT\r", NULL, 1000, 0);
                    }

                    if (SIM.state == SIM_STATE_PDP_ON) {
                        SIM.state--;
                    }
                }

                osDelay(500);
                break;
            case SIM_STATE_INTERNET_ON:
                AT_ConnectionStatusSingle(&(SIM.ip_status));
                // ============ SOCKET CONFIGURATION
                // Establish connection with server
                if (p && (SIM.ip_status != CIPSTAT_CONNECT_OK || SIM.ip_status != CIPSTAT_CONNECTING)) {
                    at_cipstart_t param = {
                            .mode = "TCP",
                            .ip = "pujakusumae-31974.portmap.io",
                            .port = 31974
                    };
                    p = AT_StartConnectionSingle(&param);

                    // wait until attached
                    do {
                        AT_ConnectionStatusSingle(&(SIM.ip_status));
                        osDelay(1000);
                    } while (SIM.ip_status == CIPSTAT_CONNECTING);
                }

                // upgrade simcom state
                if (p) {
                    SIM.state++;
                } else {
                    // Check IP Status
                    AT_ConnectionStatusSingle(&(SIM.ip_status));

                    // Close IP
                    if (SIM.ip_status == CIPSTAT_CONNECT_OK) {
                        p = Simcom_Command("AT+CIPCLOSE\r", NULL, 1000, 0);

                        // wait until closed
                        do {
                            AT_ConnectionStatusSingle(&(SIM.ip_status));
                            osDelay(1000);
                        } while (SIM.ip_status == CIPSTAT_CLOSING);
                    }

                    if (SIM.state == SIM_STATE_INTERNET_ON) {
                        SIM.state--;
                    }
                }

                osDelay(500);
                break;
            case SIM_STATE_SERVER_ON:
                // Check IP Status
                AT_ConnectionStatusSingle(&(SIM.ip_status));

                if (SIM.ip_status != CIPSTAT_CONNECT_OK) {
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

SIMCOM_RESULT Simcom_Upload(PAYLOAD_TYPE type, void *payload, uint16_t size) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    header_t *hHeader = NULL;
    uint32_t tick;
    char str[20];

    // Check IP Status
    AT_ConnectionStatusSingle(&(SIM.ip_status));
    // combine the size
    sprintf(str, "AT+CIPSEND=%d\r", size);

    Simcom_Lock();
    SIM.payload_type = type;

    if (SIM.state >= SIM_STATE_SERVER_ON && SIM.ip_status == CIPSTAT_CONNECT_OK) {
        // send command
        p = Simcom_Command(str, SIMCOM_RSP_SEND, 500, 0);
        if (p) {
            // send the payload
            p = Simcom_Command((char*) payload, SIMCOM_RSP_SENT, 10000, size);
            // wait for ACK/NACK
            if (p) {
                // set timeout guard
                tick = osKernelGetTickCount();
                // wait ACK for payload
                while (SIM.state >= SIM_STATE_SERVER_ON) {
                    if (Simcom_Response(PREFIX_ACK)
                            || Simcom_Response(PREFIX_NACK)
                            || Simcom_Response(PREFIX_COMMAND)
                            || (osKernelGetTickCount() - tick) >= pdMS_TO_TICKS(10000)) {
                        break;
                    }
                    osDelay(10);
                }

                // handle SIMCOM result
                if (Simcom_Response(PREFIX_ACK)) {
                    p = SIM_RESULT_NACK;

                    // validate ACK
                    hHeader = (header_t*) payload;
                    if (Simcom_ProcessACK(hHeader)) {
                        p = SIM_RESULT_OK;
                    }
                } else if (Simcom_Response(PREFIX_NACK) || Simcom_Response(PREFIX_COMMAND)) {
                    p = SIM_RESULT_NACK;
                } else {
                    p = SIM_RESULT_TIMEOUT;
                }
            }
        }
    }

    Simcom_Unlock();
    return p;
}

SIMCOM_RESULT Simcom_FOTA(void) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint32_t checksum, len = 0;
    AT_FTP_STATE state;
    at_sapbr_t getBEARER, setBEARER = {
            .cmd_type = SAPBR_BEARER_OPEN,
            .status = SAPBR_CONNECTED,
            .con = {
                    .apn = NET_CON_APN,
                    .username = NET_CON_USERNAME,
                    .password = NET_CON_PASSWORD,
            },
    };
    at_ftp_t setFTP = {
            .id = 1,
            .server = NET_FTP_SERVER,
            .username = NET_FTP_USERNAME,
            .password = NET_FTP_PASSWORD,
            .path = "/vcu/",
            .file = "HUB2.bin",
            .size = 0,
    };
    at_ftpget_t setFTPGET = {
            .mode = FTPGET_OPEN,
            .reqlength = 512
    };

    Simcom_Lock();
    // send command
    if (SIM.state >= SIM_STATE_INTERNET_ON) {
        // BEARER attach
        p = AT_BearerSettings(ATW, &setBEARER);

        // BEARER init
        if (p) {
            p = AT_BearerSettings(ATR, &getBEARER);
        }

        // FTP Init
        if (p && getBEARER.status == SAPBR_CONNECTED) {
            p = AT_FtpInitialize(&setFTP);
        }

        // Get file size
        if (p) {
            p = AT_FtpFileSize(&setFTP);
        }

        // Open FTP Session
        if (p && setFTP.size) {
            p = AT_FtpDownload(&setFTPGET);
        }

        // Read FTP File
        if (p && setFTPGET.response == FTP_READY) {
            // Preparing
            FLASHER_Erase();
            setFTPGET.mode = FTPGET_READ;

            // Copy chunk by chunk
            do {
                // Initiate Download
                p = AT_FtpDownload(&setFTPGET);

                // Copy to Buffer
                FLASHER_Write8(setFTPGET.ptr, setFTPGET.cnflength, len);
                len += setFTPGET.cnflength;

                // Indicator
                LOG_Str("FOTA Progress = ");
                LOG_Int(len);
                LOG_Str(" Bytes (");
                LOG_Int(len * 100 / setFTP.size);
                LOG_StrLn("%)");

                // Handle
                if (setFTPGET.cnflength == 0 || len >= setFTP.size) {
                    break;
                }
            } while (p);
        }

        // Buffer filled
        if (p && len && len == setFTP.size) {
            // Calculate CRC
            checksum = CRC_Calculate32((uint32_t*) FLASH_USER_START_ADDR, len / 4);

            // Indicator
            LOG_Str("Simcom:Checksum = 0x");
            LOG_Hex32(checksum);
            LOG_Enter();
        }

        // Check state
        AT_FtpCurrentState(&state);
        if (state == FTP_STATE_ESTABLISHED) {
            // Close session
            Simcom_Command("AT+FTPQUIT\r", NULL, 500, 0);
        }
    }

    Simcom_Unlock();
    return p;
}

SIMCOM_RESULT Simcom_Command(char *data, char *res, uint32_t ms, uint16_t size) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint8_t upload = 1;

    // Handle default value
    if (res == NULL) {
        res = SIMCOM_RSP_OK;
    }
    if (!size) {
        upload = 0;
        size = strlen(data);
    }

    // only handle command if SIM_STATE_READY or BOOT_CMD
    if (SIM.state >= SIM_STATE_READY || (strcmp(data, SIMCOM_CMD_BOOT) == 0)) {
        Simcom_Lock();

        // Debug: print command
        if (SIMCOM_DEBUG) {
            if (!upload) {
                LOG_Str("\n=> ");
                LOG_Buf(data, size);
            } else {
                LOG_BufHex(data, size);
            }
            LOG_Enter();
        }

        // send command
        p = Simcom_Execute(data, size, ms, res);

        // Debug: print response
        if (SIMCOM_DEBUG) {
            char *FTPGET = "AT+FTPGET=2";
            if (strncmp(data, FTPGET, strlen(FTPGET)) != 0) {
                LOG_Buf(SIMCOM_UART_RX, sizeof(SIMCOM_UART_RX));
                LOG_Enter();
            }
        }

        Simcom_Unlock();
    }

    return p;
}

SIMCOM_RESULT Simcom_IdleJob(uint8_t *iteration) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
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
    p = AT_ConnectionStatusSingle(&(SIM.ip_status));

    return p;
}

/* Private functions implementation --------------------------------------------*/
static SIMCOM_RESULT Simcom_ProcessCommando(command_t *command) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint32_t crcValue;
    char *str = NULL;

    Simcom_Lock();
    if (Simcom_Response(SIMCOM_RSP_IPD)) {
        // get pointer reference
        str = Simcom_Response(PREFIX_ACK);
        if (str) {
            str = strstr(str + sizeof(ack_t), PREFIX_COMMAND);
        } else {
            str = Simcom_Response(PREFIX_COMMAND);
        }

        if (str != NULL) {
            // copy the whole value (any time the buffer can change)
            *command = *(command_t*) str;

            // check the Size
            if (command->header.size == sizeof(command->data)) {
                // calculate the CRC
                crcValue = CRC_Calculate8(
                        (uint8_t*) &(command->header.size),
                        sizeof(command->header.size) + sizeof(command->data),
                        0);

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
        if (Simcom_Response(SIMCOM_RSP_READY)
                || Simcom_Response(SIMCOM_RSP_OK)
                || (osKernelGetTickCount() - tick) >= NET_BOOT_TIMEOUT) {
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
    osDelay(1000);

    // simcom reset pin
    HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, 1);
    HAL_Delay(5);
    HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, 0);

    // wait response
    return Simcom_Ready();
}

static void Simcom_Sleep(uint8_t state) {
    HAL_GPIO_WritePin(INT_NET_DTR_GPIO_Port, INT_NET_DTR_Pin, state);
    osDelay(50);
}

static uint8_t Simcom_CommandoIRQ(void) {
    return Simcom_Response(PREFIX_COMMAND) ||
            (SIM.payload_type == PAYLOAD_REPORT && SIM.commando);
}

static SIMCOM_RESULT Simcom_Execute(char *data, uint16_t size, uint32_t ms, char *res) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint32_t tick, timeout_tick = 0;

    Simcom_Lock();
    // wake-up the SIMCOM
    Simcom_Sleep(0);

    // transmit to serial (low-level)
    Simcom_BeforeTransmitHook();
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
                || Simcom_CommandoIRQ()
                || (osKernelGetTickCount() - tick) >= timeout_tick) {

            // check response
            if (Simcom_Response(res)) {
                p = SIM_RESULT_OK;
            }

            // Handle failure
            if (p != SIM_RESULT_OK) {
                if (Simcom_CommandoIRQ()) {
                    p = SIM_RESULT_TIMEOUT;
                } else if (strlen(SIMCOM_UART_RX) == 0) {
                    // exception for no response
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

static void Simcom_BeforeTransmitHook(void) {
    command_t hCommand;

    // handle things on every request
    //	LOG_StrLn("============ SIMCOM DEBUG ============");
    //	LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
    //	LOG_Enter();
    //	LOG_StrLn("======================================");

    // handle Commando (if any)
    if (Simcom_ProcessCommando(&hCommand)) {
        SIM.commando = 1;
        osMessageQueuePut(CommandQueueHandle, &hCommand, 0U, 0U);
    }
}
