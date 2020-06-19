/*
 * _at.c
 *
 *  Created on: May 13, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_at.h"
#include "DMA/_dma_simcom.h"

/* External variables --------------------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];
extern sim_t SIM;

/* Private constants ---------------------------------------------------------*/
#define CHARISNUM(x)                            ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)                            ((x) - '0')

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT AT_SingleInteger(char command[20], AT_MODE mode, int32_t *value, uint8_t executor);
static SIMCOM_RESULT AT_CmdWrite(char *cmd, uint32_t ms, char *res);
static SIMCOM_RESULT AT_CmdRead(char *cmd, uint32_t ms, char *prefix, char **str);
static void AT_ParseText(const char *ptr, uint8_t *cnt, char *text, uint8_t size);
static int32_t AT_ParseNumber(const char *ptr, uint8_t *cnt);
//static float AT_ParseFloat(const char *ptr, uint8_t *cnt);

/* Public functions implementation --------------------------------------------*/
SIMCOM_RESULT AT_CommandEchoMode(uint8_t state) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    char cmd[6];

    Simcom_Lock();
    // Write
    sprintf(cmd, "ATE%d\r", state);
    p = AT_CmdWrite(cmd, 500, NULL);
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_GetLocalIpAddress(at_cifsr_t *param) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    char *str = NULL;

    Simcom_Lock();
    // Read
    p = AT_CmdRead("AT+CIFSR\r", 500, SIMCOM_RSP_NONE, &str);
    if (p > 0) {
        AT_ParseText(&str[0], NULL, param->address, sizeof(param->address));
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_StartConnectionSingle(at_cipstart_t *param) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    char cmd[80];

    Simcom_Lock();
    // Write
    sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",\"%d\"\r",
            param->mode, param->ip, param->port);
    p = AT_CmdWrite(cmd, 30000, "CONNECT");

    // check either connection ok / error
    if (p > 0) {
        if (Simcom_Response("CONNECT OK")
                || Simcom_Response("ALREADY CONNECT")
                || Simcom_Response("TCP CLOSED")) {
            p = SIM_RESULT_OK;
        } else {
            p = SIM_RESULT_ERROR;
        }
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_SignalQualityReport(at_csq_t *signal) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint8_t cnt, len = 0;
    char *str = NULL;
    float dBm;

    Simcom_Lock();
    // Read
    p = AT_CmdRead("AT+CSQ\r", 500, "+CSQ: ", &str);
    if (p > 0) {
        signal->rssi = AT_ParseNumber(&str[len], &cnt);
        len += cnt + 1;
        signal->ber = AT_ParseNumber(&str[len], NULL);

        // Formatting
        {
            // Handle not detectable value
            if (signal->rssi > 31) {
                signal->rssi = 0;
            }

            // Scale RSSI to dBm
            dBm = (signal->rssi * 63.0 / 31.0) - 115.0;
            // Scale dBm to percentage
            signal->percent = (dBm + 115.0) * 100.0 / 63.0;

            // debugging
            LOG_Str("\nSimcom:RSSI = ");
            LOG_Int(signal->percent);
            LOG_StrLn("%");
        }
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_ConnectionStatusSingle(AT_CIPSTATUS *state) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    char status[20];
    char *str = NULL;

    Simcom_Lock();
    // Read
    p = AT_CmdRead("AT+CIPSTATUS\r", 500, "STATE: ", &str);
    if (p > 0) {
        AT_ParseText(&str[0], NULL, status, sizeof(status));

        // decide
        if (!strcmp(status, "IP INITIAL")) {
            *state = CIPSTAT_IP_INITIAL;
        } else if (!strcmp(status, "IP START")) {
            *state = CIPSTAT_IP_START;
        } else if (!strcmp(status, "IP CONFIG")) {
            *state = CIPSTAT_IP_CONFIG;
        } else if (!strcmp(status, "IP GPRSACT")) {
            *state = CIPSTAT_IP_GPRSACT;
        } else if (!strcmp(status, "IP STATUS")) {
            *state = CIPSTAT_IP_STATUS;
        } else if (!strcmp(status, "TCP CONNECTING")
                || !strcmp(status, "UDP CONNECTING")
                || !strcmp(status, "SERVER LISTENING")) {
            *state = CIPSTAT_CONNECTING;
        } else if (!strcmp(status, "CONNECT OK")) {
            *state = CIPSTAT_CONNECT_OK;
        } else if (!strcmp(status, "TCP CLOSING")
                || !strcmp(status, "UDP CLOSING")) {
            *state = CIPSTAT_CLOSING;
        } else if (!strcmp(status, "TCP CLOSED")
                || !strcmp(status, "UDP CLOSED")) {
            *state = CIPSTAT_CLOSED;
        } else if (!strcmp(status, "PDP DEACT")) {
            *state = CIPSTAT_PDP_DEACT;
        } else {
            *state = CIPSTAT_UNKNOWN;
        }
    } else {
        *state = CIPSTAT_UNKNOWN;
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_ConfigureAPN(AT_MODE mode, at_cstt_t *param) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint8_t cnt, len = 0;
    char *str = NULL, cmd[80];

    // Copy by value
    at_cstt_t tmp = *param;

    Simcom_Lock();
    // Read
    p = AT_CmdRead("AT+CSTT?\r", 500, "+CSTT: ", &str);
    if (p > 0) {
        AT_ParseText(&str[len], &cnt, tmp.apn, sizeof(tmp.apn));
        len += cnt + 1;
        AT_ParseText(&str[len], &cnt, tmp.username, sizeof(tmp.username));
        len += cnt + 1;
        AT_ParseText(&str[len], &cnt, tmp.password, sizeof(tmp.password));

        // Write
        if (mode == ATW) {
            if (memcmp(&tmp, param, sizeof(at_cstt_t)) != 0) {
                sprintf(cmd, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r",
                        param->apn, param->username, param->password);
                p = AT_CmdWrite(cmd, 1000, NULL);
            }
        } else {
            *param = tmp;
        }
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_RadioAccessTechnology(AT_MODE mode, at_cnmp_t *param) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint8_t cnt, len = 0;
    char *str = NULL, cmd[14];

    // Copy by value
    at_cnmp_t tmp = *param;

    Simcom_Lock();
    // Read
    p = AT_CmdRead("AT+CNMP?\r", 500, "+CNMP: ", &str);
    if (p > 0) {
        param->mode = AT_ParseNumber(&str[len], &cnt);
        if (param->mode == CNMP_ACT_AUTO) {
            len += cnt + 1;
            param->preferred = AT_ParseNumber(&str[len], &cnt);
        }

        // Write
        if (mode == ATW) {
            if (memcmp(&tmp, param, sizeof(at_cnmp_t)) != 0) {
                if (tmp.mode == CNMP_ACT_AUTO) {
                    sprintf(cmd, "AT+CNMP=%d%d\r", param->mode, param->preferred);
                } else {
                    sprintf(cmd, "AT+CNMP=%d\r", param->mode);
                }

                p = AT_CmdWrite(cmd, 10000, NULL);
            }
        } else {
            *param = tmp;
        }
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_NetworkAttachedStatus(AT_MODE mode, at_csact_t *param) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint8_t cnt, len = 0;
    char *str = NULL, cmd[14];

    // Copy by value
    at_csact_t tmp = *param;

    Simcom_Lock();
    // Read
    p = AT_CmdRead("AT+CSACT?\r", 500, "+CSACT: ", &str);
    if (p > 0) {
        tmp.act = AT_ParseNumber(&str[len], &cnt);
        len += cnt + 1;
        AT_ParseText(&str[len], &cnt, tmp.rac, sizeof(tmp.rac));
        len += cnt + 1;
        tmp.creg = AT_ParseNumber(&str[len], &cnt);
        len += cnt + 1;
        tmp.cgreg = AT_ParseNumber(&str[len], &cnt);

        // Write
        if (mode == ATW) {
            if (tmp.cgreg != param->creg || tmp.cgreg != param->cgreg) {
                sprintf(cmd, "AT+CSACT=%d,%d\r", param->creg, param->cgreg);
                p = AT_CmdWrite(cmd, 500, NULL);
            }
        } else {
            *param = tmp;
        }
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_Clock(AT_MODE mode, timestamp_t *tm) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint8_t cnt, len = 0;
    char *str = NULL, cmd[32];

    Simcom_Lock();
    if (mode == ATW) {
        // Write
        sprintf(cmd, "AT+CCLK=\"%d/%d/%d,%d:%d:%d%+d\"\r",
                tm->date.Year,
                tm->date.Month,
                tm->date.Date,
                tm->time.Hours,
                tm->time.Minutes,
                tm->time.Seconds,
                tm->tzQuarterHour);
        p = AT_CmdWrite(cmd, 500, NULL);

    } else {
        // Read
        p = AT_CmdRead("AT+CCLK?\r", 500, "+CCLK: ", &str);
        if (p > 0) {
            len = 1;
            tm->date.Year = AT_ParseNumber(&str[len], &cnt);
            len += cnt + 1;
            tm->date.Month = AT_ParseNumber(&str[len], &cnt);
            len += cnt + 1;
            tm->date.Date = AT_ParseNumber(&str[len], &cnt);
            len += cnt + 1;
            tm->time.Hours = AT_ParseNumber(&str[len], &cnt);
            len += cnt + 1;
            tm->time.Minutes = AT_ParseNumber(&str[len], &cnt);
            len += cnt + 1;
            tm->time.Seconds = AT_ParseNumber(&str[len], &cnt);
            len += cnt + 1;
            tm->tzQuarterHour = AT_ParseNumber(&str[len], NULL);

            // Formatting
            tm->date.WeekDay = RTC_WEEKDAY_MONDAY;
        }
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_NetworkRegistration(char command[20], AT_MODE mode, at_c_greg_t *param) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    uint8_t cnt, len = 0;
    char *str = NULL, cmd[14], res[15];

    // Copy by value
    at_c_greg_t tmp = *param;

    Simcom_Lock();
    // Read
    sprintf(cmd, "AT+%s?\r", command);
    sprintf(res, "+%s: ", command);
    p = AT_CmdRead(cmd, 500, res, &str);
    if (p > 0) {
        tmp.mode = AT_ParseNumber(&str[len], &cnt);
        len += cnt + 1;
        tmp.stat = AT_ParseNumber(&str[len], &cnt);

        // Write
        if (mode == ATW) {
            if (memcmp(&tmp, param, sizeof(tmp)) != 0) {
                sprintf(cmd, "AT+%s=%d\r", command, param->mode);
                p = AT_CmdWrite(cmd, 500, NULL);
            }
        } else {
            *param = tmp;
        }
    }
    Simcom_Unlock();

    return p;
}

SIMCOM_RESULT AT_GprsAttachment(AT_MODE mode, AT_CGATT *state) {
    return AT_SingleInteger("CGATT", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_ManuallyReceiveData(AT_MODE mode, AT_CIPRXGET *state) {
    return AT_SingleInteger("CIPRXGET", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_MultiIpConnection(AT_MODE mode, AT_CIPMUX *state) {
    return AT_SingleInteger("CIPMUX", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_TcpApllicationMode(AT_MODE mode, AT_CIPMODE *state) {
    return AT_SingleInteger("CIPMODE", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_ShowRemoteIp(AT_MODE mode, AT_BOOL *state) {
    return AT_SingleInteger("CIPSRIP", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_IpPackageHeader(AT_MODE mode, AT_BOOL *state) {
    return AT_SingleInteger("CIPHEAD", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_EnableLocalTimestamp(AT_MODE mode, AT_BOOL *state) {
    return AT_SingleInteger("CLTS", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_ConfigureSlowClock(AT_MODE mode, AT_CSCLK *state) {
    return AT_SingleInteger("CSCLK", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_ReportMobileEquipmentError(AT_MODE mode, AT_CMEE *state) {
    return AT_SingleInteger("CMEE", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_FixedLocalRate(AT_MODE mode, uint32_t *rate) {
    return AT_SingleInteger("IPR", mode, (int32_t*) rate, 0);
}

/* Private functions implementation --------------------------------------------*/
static SIMCOM_RESULT AT_SingleInteger(char command[20], AT_MODE mode, int32_t *value, uint8_t executor) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;
    char *str = NULL, cmd[20], res[20];

    // Copy by vale
    int32_t tmp = *value;

    Simcom_Lock();
    // Read
    sprintf(cmd, "AT+%s%s", command, executor ? "\r" : "?\r");
    sprintf(res, "+%s: ", command);
    p = AT_CmdRead(cmd, 1000, res, &str);
    if (p > 0) {
        tmp = AT_ParseNumber(&str[0], NULL);

        // Write
        if (mode == ATW) {
            if (tmp != *value) {
                sprintf(cmd, "AT+%s=%d\r", command, (int) *value);
                p = AT_CmdWrite(cmd, 500, NULL);
            }
        } else {
            *value = tmp;
        }
    }
    Simcom_Unlock();

    return p;
}

static SIMCOM_RESULT AT_CmdWrite(char *cmd, uint32_t ms, char *res) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;

    if (SIM.state >= SIM_STATE_READY) {
        p = Simcom_Command(cmd, res, ms, 0);
    }

    return p;
}

static SIMCOM_RESULT AT_CmdRead(char *cmd, uint32_t ms, char *prefix, char **str) {
    SIMCOM_RESULT p = SIM_RESULT_ERROR;

    if (SIM.state >= SIM_STATE_READY) {
        p = Simcom_Command(cmd, prefix, ms, 0);

        if (p > 0) {
            *str = Simcom_Response(prefix);

            if (*str != NULL) {
                *str += strlen(prefix);

                p = SIM_RESULT_OK;
            }
        }
    }

    return p;
}

static void AT_ParseText(const char *ptr, uint8_t *cnt, char *text, uint8_t size) {
    uint8_t i = 0;

    // check for double quote start
    if (*ptr == '"') {
        ptr++;
        i++;
    }
    // Parse text
    while (*ptr != '"' && *ptr != '\r' && *ptr != '\n') {
        *text = *ptr;

        // increment
        text++;
        ptr++;
        i++;
        size--;

        // handle overflow
        if (size <= 1) {
            break;
        }
    }
    // end of parsing for : double-quote, tab, new-line
    *text = '\0';
    ptr++;
    i++;
    // Save number of characters used for number
    if (cnt != NULL) {
        *cnt = i;
    }
}

static int32_t AT_ParseNumber(const char *ptr, uint8_t *cnt) {
    uint8_t minus = 0, i = 0;
    int32_t sum = 0;

    if (*ptr == '-') { /* Check for minus character */
        minus = 1;
        ptr++;
        i++;
    }
    while (CHARISNUM(*ptr)) { /* Parse number */
        sum = 10 * sum + CHARTONUM(*ptr);
        ptr++;
        i++;
    }
    if (cnt != NULL) { /* Save number of characters used for number */
        *cnt = i;
    }
    if (minus) { /* Minus detected */
        return 0 - sum;
    }
    return sum; /* Return number */
}

//static float AT_ParseFloat(const char *ptr, uint8_t *cnt) {
//	uint8_t i = 0, j = 0;
//	float sum = 0.0f;
//
//	sum = (float) AT_ParseNumber(ptr, &i); /* Parse number */
//	j += i;
//	ptr += i;
//	if (*ptr == '.') { /* Check decimals */
//		float dec;
//		dec = (float) AT_ParseNumber(ptr + 1, &i);
//		dec /= (float) pow(10, i);
//		if (sum >= 0) {
//			sum += dec;
//		} else {
//			sum -= dec;
//		}
//		j += i + 1;
//	}
//
//	if (cnt != NULL) { /* Save number of characters used for number */
//		*cnt = j;
//	}
//	return sum; /* Return number */
//}
