/*
 * _at.c
 *
 *  Created on: May 13, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Parser/_at.h"
#include "DMA/_dma_simcom.h"

/* External variables ---------------------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];
extern sim_t SIM;

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT AT_CmdWrite(char *cmd, uint32_t ms, char *res);
static SIMCOM_RESULT AT_CmdRead(char *cmd, char *prefix, char **str);
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

SIMCOM_RESULT AT_SignalQualityReport(at_csq_t *signal) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL;
	float dBm;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CSQ\r", "+CSQ: ", &str);
	if (p) {
		signal->rssi = AT_ParseNumber(&str[0], &cnt);
		signal->ber = AT_ParseNumber(&str[cnt + 1], NULL);

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

SIMCOM_RESULT AT_GetLocalIpAddress(at_cifsr_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	char *str = NULL;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIFSR\r", SIMCOM_RSP_NONE, &str);
	if (p) {
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
	p = AT_CmdWrite(cmd, 10000, "CONNECT");

	// check either connection ok / error
	if (p) {
		p = Simcom_Response("CONNECT OK") ||
				Simcom_Response("ALREADY CONNECT") ||
				Simcom_Response("TCP CLOSED");
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_ConfigureAPN(AT_MODE mode, at_cstt_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[80];

	// Copy by value
	at_cstt_t tmp = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CSTT?\r", "+CSTT: ", &str);
	if (p) {
		AT_ParseText(&str[0], &cnt, tmp.apn, sizeof(tmp.apn));
		AT_ParseText(&str[cnt + 1], &cnt, tmp.username, sizeof(tmp.username));
		AT_ParseText(&str[cnt + 1], &cnt, tmp.password, sizeof(tmp.password));

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

SIMCOM_RESULT AT_ManuallyReceiveData(AT_MODE mode, AT_CIPRXGET *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[20];

	// Copy by vale
	AT_CIPRXGET tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPRXGET?\r", "+CIPRXGET: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CIPRXGET=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_MultiIpConnection(AT_MODE mode, AT_CIPMUX *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by value
	AT_CIPMUX tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPMUX?\r", "+CIPMUX: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CIPMUX=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_TcpApllicationMode(AT_MODE mode, AT_CIPMODE *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by value
	AT_CIPMODE tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPMODE?\r", "+CIPMODE: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CIPMODE=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_GprsAttachment(AT_MODE mode, AT_CGATT *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by value
	AT_CGATT tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CGATT?\r", "+CGATT: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CGATT=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_NetworkRegistrationStatus(AT_MODE mode, at_c_greg_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by vale
	at_c_greg_t tmp = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CGREG?\r", "+CGREG: ", &str);
	if (p) {
		tmp.mode = AT_ParseNumber(&str[0], &cnt);
		tmp.stat = AT_ParseNumber(&str[cnt + 1], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp.mode != param->mode) {
				sprintf(cmd, "AT+CGREG=%d\r", param->mode);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*param = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_NetworkRegistration(AT_MODE mode, at_c_greg_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by value
	at_c_greg_t tmp = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CREG?\r", "+CREG: ", &str);
	if (p) {
		tmp.mode = AT_ParseNumber(&str[0], &cnt);
		tmp.stat = AT_ParseNumber(&str[cnt + 1], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp.mode != param->mode) {
				sprintf(cmd, "AT+CREG=%d\r", param->mode);
				p = AT_CmdWrite(cmd, 500, NULL);
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
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by value
	at_cnmp_t tmp = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CNMP?\r", "+CNMP: ", &str);
	if (p) {
		param->mode = AT_ParseNumber(&str[0], &cnt);
		if (param->mode == CNMP_ACT_AUTO) {
			param->preferred = AT_ParseNumber(&str[cnt + 1], &cnt);
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
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by value
	at_csact_t tmp = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CSACT?\r", "+CSACT: ", &str);
	if (p) {
		tmp.act = AT_ParseNumber(&str[0], &cnt);
		AT_ParseText(&str[cnt + 1], &cnt, tmp.rac, sizeof(tmp.rac));
		tmp.creg = AT_ParseNumber(&str[cnt + 1], &cnt);
		tmp.cgreg = AT_ParseNumber(&str[cnt + 1], &cnt);

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

SIMCOM_RESULT AT_ShowRemoteIp(AT_MODE mode, AT_BOOL *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by value
	AT_BOOL tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPSRIP?\r", "+CIPSRIP: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CIPSRIP=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_IpPackageHeader(AT_MODE mode, AT_BOOL *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[14];

	// Copy by value
	AT_BOOL tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPHEAD?\r", "+CIPHEAD: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CIPHEAD=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_GetLocalTimestamp(AT_MODE mode, AT_BOOL *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[11];

	// Copy by value
	AT_BOOL tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CLTS?\r", "+CLTS: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CLTS=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_ConfigureSlowClock(AT_MODE mode, AT_CSCLK *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[12];

	// Copy by value
	AT_CSCLK tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CSCLK?\r", "+CSCLK: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CSCLK=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}

	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_ReportMobileEquipmentError(AT_MODE mode, AT_CMEE *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[11];

	// Copy by value
	AT_CMEE tmp = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CMEE?\r", "+CMEE: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *state) {
				sprintf(cmd, "AT+CMEE=%d\r", *state);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*state = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_FixedLocalRate(AT_MODE mode, uint32_t *rate) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[15];

	// Copy by value
	uint32_t tmp = *rate;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+IPR?\r", "+IPR: ", &str);
	if (p) {
		tmp = AT_ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp != *rate) {
				sprintf(cmd, "AT+IPR=%ld\r", *rate);
				p = AT_CmdWrite(cmd, 500, NULL);
			}
		} else {
			*rate = tmp;
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_Clock(AT_MODE mode, timestamp_t *tm) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, cmd[32];

	Simcom_Lock();
	if (mode == ATW) {
		// Write
		sprintf(cmd, "AT+CCLK=\"%d/%d/%d,%d,%d,%d%d\"\r",
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
		p = AT_CmdRead("AT+CCLK?\r", "+CCLK: ", &str);
		if (p) {
			tm->date.Year = AT_ParseNumber(&str[1], &cnt);
			tm->date.Month = AT_ParseNumber(&str[cnt + 1], &cnt);
			tm->date.Date = AT_ParseNumber(&str[cnt + 1], &cnt);
			tm->time.Hours = AT_ParseNumber(&str[cnt + 1], &cnt);
			tm->time.Minutes = AT_ParseNumber(&str[cnt + 1], &cnt);
			tm->time.Seconds = AT_ParseNumber(&str[cnt + 1], &cnt);
			tm->tzQuarterHour = AT_ParseNumber(&str[cnt + 1], NULL);

			// Formatting
			tm->date.WeekDay = RTC_WEEKDAY_MONDAY;
		}
	}
	Simcom_Unlock();

	return p;
}

/* Private functions implementation --------------------------------------------*/
static SIMCOM_RESULT AT_CmdWrite(char *cmd, uint32_t ms, char *res) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;

	if (SIM.state >= SIM_STATE_READY) {
		if (res == NULL) {
			p = Simcom_Cmd(cmd, ms, 1);
		} else {
			p = Simcom_Command(cmd, ms, 1, res);
		}
	}

	return p;
}

static SIMCOM_RESULT AT_CmdRead(char *cmd, char *prefix, char **str) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;

	if (SIM.state >= SIM_STATE_READY) {
		p = Simcom_Cmd(cmd, 500, 1);

		if (p) {
			*str = strstr(SIMCOM_UART_RX, prefix);

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
