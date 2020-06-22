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
#include "Libs/_fota.h"
#include "Libs/_eeprom.h"
#include "DMA/_dma_simcom.h"
#include "Drivers/_crc.h"
#include "Drivers/_at.h"
#include "Drivers/_flasher.h"

/* External variables ---------------------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];

/* Exported variables ---------------------------------------------------------*/
sim_t SIM = {
        .state = SIM_STATE_DOWN,
        .ip_status = CIPSTAT_UNKNOWN,
        .signal = 0
};

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT Simcom_Ready(void);
static SIMCOM_RESULT Simcom_Power(void);
static SIMCOM_RESULT Simcom_Execute(char *data, uint16_t size, uint32_t ms, char *res);
static void Simcom_Sleep(uint8_t state);
static void Simcom_BeforeTransmitHook(void);

/* Public functions implementation --------------------------------------------*/
void Simcom_Lock(void) {
//    osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
}

void Simcom_Unlock(void) {
//    osMutexRelease(SimcomRecMutexHandle);
}

char* Simcom_Response(char *str) {
    return strstr(SIMCOM_UART_RX, str);
}

void Simcom_Init(void) {
    SIMCOM_DMA_Init();
    Simcom_SetState(SIM_STATE_READY);
}

uint8_t Simcom_SetState(SIMCOM_STATE state) {
    static uint8_t init = 1;
    uint8_t depth = 3;
    SIMCOM_STATE lastState = SIM_STATE_DOWN;
    SIMCOM_RESULT p;

    Simcom_Lock();
    // Handle SIMCOM state properly
    while (SIM.state < state) {
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
            SIM.signal = 0;
        } else {
            Simcom_IdleJob(NULL);
            if (SIM.state >= SIM_STATE_GPRS_ON) {
                // Force to exit loop
                if (SIM.signal < 15) {
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
                if (p > 0) {
                    SIM.state++;
                    LOG_StrLn("Simcom:ON");
                } else {
                    LOG_StrLn("Simcom:Error");
                }

                // disable command echo
                if (p > 0) {
                    p = AT_CommandEchoMode(0);
                }

                _DelayMS(500);
                break;
            case SIM_STATE_READY:
                // =========== BASIC CONFIGURATION
                // Set serial baud-rate
                if (p > 0) {
                    uint32_t rate = 0;
                    p = AT_FixedLocalRate(ATW, &rate);
                }
                // Error report format: 0, 1(Numeric), 2(verbose)
                if (p > 0) {
                    AT_CMEE state = CMEE_VERBOSE;
                    p = AT_ReportMobileEquipmentError(ATW, &state);
                }
                // Use pin DTR as sleep control
                if (p > 0) {
                    AT_CSCLK state = CSCLK_EN_DTR;
                    p = AT_ConfigureSlowClock(ATW, &state);
                }
//                // Enable time reporting
//                if (p > 0) {
//                    AT_BOOL state = AT_ENABLE;
//                    p = AT_EnableLocalTimestamp(ATW, &state);
//                }
//                // Enable “+IPD” header
//                if (p > 0) {
//                    AT_BOOL state = AT_ENABLE;
//                    p = AT_IpPackageHeader(ATW, &state);
//                }
//                // Disable “RECV FROM” header
//                if (p > 0) {
//                    AT_BOOL state = AT_DISABLE;
//                    p = AT_ShowRemoteIp(ATW, &state);
//                }
                // =========== NETWORK CONFIGURATION
                // Check SIM Card
                if (p > 0) {
                    p = Simcom_Command("AT+CPIN?\r", "READY", 500, 0);
                }
                // Disable presentation of <AcT>&<rac> at CREG and CGREG
                if (p > 0) {
                    at_csact_t param = {
                            .creg = 0,
                            .cgreg = 0,
                    };
                    p = AT_NetworkAttachedStatus(ATW, &param);
                }

                // upgrade simcom state
                if (p > 0) {
                    SIM.state++;
                }

                _DelayMS(500);
                break;
            case SIM_STATE_CONFIGURED:
                // =========== NETWORK ATTACH
                // Set signal Generation 2G(13)/3G(14)/AUTO(2)
                if (p > 0) {
                    at_cnmp_t param = {
                            .mode = CNMP_ACT_AUTO,
                            .preferred = CNMP_ACT_P_UMTS
                    };
                    p = AT_RadioAccessTechnology(ATW, &param);
                }
                // Network Registration Status
                if (p > 0) {
                    at_c_greg_t read, param = {
                            .mode = CREG_MODE_DISABLE,
                            .stat = CREG_STAT_REG_HOME
                    };
                    // wait until attached
                    do {
                        p = AT_NetworkRegistration("CREG", ATW, &param);
                        if (p > 0) {
                            p = AT_NetworkRegistration("CREG", ATR, &read);
                        }

                        _DelayMS(1000);
                    } while (p && read.stat != param.stat);
                }

                // upgrade simcom state
                if (p > 0) {
                    SIM.state++;
                }

                _DelayMS(500);
                break;
            case SIM_STATE_NETWORK_ON:
                // =========== GPRS ATTACH
                // GPRS Registration Status
                if (p > 0) {
                    at_c_greg_t read, param = {
                            .mode = CREG_MODE_DISABLE,
                            .stat = CREG_STAT_REG_HOME
                    };
                    // wait until attached
                    do {
                        p = AT_NetworkRegistration("CGREG", ATW, &param);
                        if (p > 0) {
                            p = AT_NetworkRegistration("CGREG", ATR, &read);
                        }

                        _DelayMS(1000);
                    } while (p && read.stat != param.stat);
                }

                // upgrade simcom state
                if (p > 0) {
                    SIM.state++;
                } else {
                    if (SIM.state == SIM_STATE_NETWORK_ON) {
                        SIM.state--;
                    }
                }

                _DelayMS(500);
                break;
            case SIM_STATE_GPRS_ON:

                break;
            default:
                break;
        }
    };
    Simcom_Unlock();

    return SIM.state >= state;
}

uint8_t Simcom_FOTA(void) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint32_t checksum = 0, len = 0;
    at_ftp_t ftp = {
            .path = "/vcu/",
            .version = "APP"
    };

    // DFU flag set
    EEPROM_FlagDFU(EE_CMD_W, 1);
    Simcom_Init();

    Simcom_Lock();
    // FOTA download, program & check
    if (Simcom_SetState(SIM_STATE_GPRS_ON)) {
        // Initialize bearer for TCP based apps.
        p = FOTA_BearerInitialize();

        // Get checksum of new firmware
        if (p > 0) {
            p = FOTA_GetChecksum(&ftp, &checksum);
        }

        // Download & Program new firmware
        if (p > 0) {
            p = FOTA_FirmwareToFlash(&ftp, &len);
        }

        // Buffer filled, compare the checksum
        if (p > 0) {
            p = FOTA_CompareChecksum(checksum, len, APP_START_ADDR);
        }

        // DFU flag reset
        if (p > 0) {
            EEPROM_FlagDFU(EE_CMD_W, 0);
        }
    }

    Simcom_Unlock();
    return (p == SIM_RESULT_OK);
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
    if (p > 0) {
        SIM.signal = signal.percent;
    }

    return p;
}

static SIMCOM_RESULT Simcom_Ready(void) {
    uint32_t tick;

    // wait until 1s response
    tick = _GetTickMS();
    while (SIM.state == SIM_STATE_DOWN) {
        if (Simcom_Response(SIMCOM_RSP_READY)
                || Simcom_Response(SIMCOM_RSP_OK)
                || (_GetTickMS() - tick) >= NET_BOOT_TIMEOUT) {
            break;
        }
        _DelayMS(1);
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
    _DelayMS(100);
    HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, 1);
    _DelayMS(1000);

    // simcom reset pin
    HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, 1);
    HAL_Delay(1);
    HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, 0);

    // wait response
    return Simcom_Ready();
}

static void Simcom_Sleep(uint8_t state) {
    HAL_GPIO_WritePin(INT_NET_DTR_GPIO_Port, INT_NET_DTR_Pin, state);
    _DelayMS(50);
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
    timeout_tick = (ms + NET_EXTRA_TIME);
    // set timeout guard
    tick = _GetTickMS();

    // wait response from SIMCOM
    while (1) {
        if (Simcom_Response(res)
                || Simcom_Response(SIMCOM_RSP_ERROR)
                || Simcom_Response(SIMCOM_RSP_READY)
                || (_GetTickMS() - tick) >= timeout_tick) {

            // check response
            if (Simcom_Response(res)) {
                p = SIM_RESULT_OK;
            }

            // Handle failure
            if (p != SIM_RESULT_OK) {
                if (strlen(SIMCOM_UART_RX) == 0) {
                    // exception for no response
                    p = SIM_RESULT_NO_RESPONSE;
                    SIM.state = SIM_STATE_DOWN;
                    LOG_StrLn("Simcom:NoResponse");

                } else {
                    // exception for auto reboot module
                    if (Simcom_Response(SIMCOM_RSP_READY) && (SIM.state >= SIM_STATE_READY)) {
                        p = SIM_RESULT_RESTARTED;
                        SIM.state = SIM_STATE_READY;

                        LOG_StrLn("Simcom:Restarted");
                    } else if ((_GetTickMS() - tick) >= timeout_tick) {
                        // exception for timeout
                        p = SIM_RESULT_TIMEOUT;
                        LOG_StrLn("Simcom:Timeout");
                    }
                }
            }

            // exit loop
            break;
        }
        _DelayMS(10);
    }

    // sleep the SIMCOM
    Simcom_Sleep(1);
    Simcom_Unlock();
    return p;
}

static void Simcom_BeforeTransmitHook(void) {
    // handle things on every request
    //	LOG_StrLn("============ SIMCOM DEBUG ============");
    //	LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
    //	LOG_Enter();
    //	LOG_StrLn("======================================");
}
