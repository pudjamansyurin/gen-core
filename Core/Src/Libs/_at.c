/*
 * _at.c
 *
 *  Created on: May 13, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_at.h"

/* External variables ---------------------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];
extern sim_t SIM;

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT AT_CmdWrite(char *cmd, uint32_t ms, char *res);
static SIMCOM_RESULT AT_CmdRead(char *cmd, char *prefix, char **str);

/* Public functions implementation --------------------------------------------*/
SIMCOM_RESULT AT_CommandEchoMode(uint8_t state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	char wStr[6];

	Simcom_Lock();
	// Write
	sprintf(wStr, "ATE%d\r", state);
	p = AT_CmdWrite(wStr, 500, NULL);
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
		signal->rssi = _ParseNumber(&str[0], &cnt);
		signal->ber = _ParseNumber(&str[cnt + 1], NULL);

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

SIMCOM_RESULT AT_GetLocalIpAddress(char *ip) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	char *str = NULL;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIFSR\r", SIMCOM_RSP_NONE, &str);
	if (p) {
		_ParseText(&str[0], NULL, ip);
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_StartConnectionSingle(at_cipstart_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	char wStr[80];

	Simcom_Lock();
	// Write
	sprintf(wStr, "AT+CIPSTART=\"%s\",\"%s\",\"%d\"\r",
			param->mode, param->ip, param->port);
	p = AT_CmdWrite(wStr, 10000, "CONNECT");

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
	char *str = NULL, wStr[80];

	// Copy by value
	at_cstt_t wParam = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CSTT?\r", "+CSTT: ", &str);
	if (p) {
		_ParseText(&str[0], &cnt, param->apn);
		_ParseText(&str[cnt + 1], &cnt, param->username);
		_ParseText(&str[cnt + 1], &cnt, param->password);

		// Write
		if (mode == ATW) {
			if (memcmp(&wParam, param, sizeof(at_cstt_t)) != 0) {
				sprintf(wStr, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r",
						wParam.apn, wParam.username, wParam.password);
				p = AT_CmdWrite(wStr, 1000, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_ManuallyReceiveData(AT_MODE mode, AT_CIPRXGET *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[20];

	// Copy by vale
	AT_CIPRXGET wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPRXGET?\r", "+CIPRXGET: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CIPRXGET=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_MultiIpConnection(AT_MODE mode, AT_CIPMUX *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by vale
	AT_CIPMUX wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPMUX?\r", "+CIPMUX: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CIPMUX=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_TcpApllicationMode(AT_MODE mode, AT_CIPMODE *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by vale
	AT_CIPMODE wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPMODE?\r", "+CIPMODE: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CIPMODE=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_GprsAttachment(AT_MODE mode, AT_CGATT *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by vale
	AT_CGATT wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CGATT?\r", "+CGATT: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CGATT=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_NetworkRegistrationStatus(AT_MODE mode, at_c_greg_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by vale
	at_c_greg_t wParam = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CGREG?\r", "+CGREG: ", &str);
	if (p) {
		param->mode = _ParseNumber(&str[0], &cnt);
		param->stat = _ParseNumber(&str[cnt + 1], &cnt);

		// Write
		if (mode == ATW) {
			if (wParam.mode != param->mode) {
				sprintf(wStr, "AT+CGREG=%d\r", wParam.mode);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_NetworkRegistration(AT_MODE mode, at_c_greg_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by vale
	at_c_greg_t wParam = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CREG?\r", "+CREG: ", &str);
	if (p) {
		param->mode = _ParseNumber(&str[0], &cnt);
		param->stat = _ParseNumber(&str[cnt + 1], &cnt);

		// Write
		if (mode == ATW) {
			if (wParam.mode != param->mode) {
				sprintf(wStr, "AT+CREG=%d\r", wParam.mode);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_RadioAccessTechnology(AT_MODE mode, at_cnmp_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by value
	at_cnmp_t wParam = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CNMP?\r", "+CNMP: ", &str);
	if (p) {
		param->mode = _ParseNumber(&str[0], &cnt);
		if (param->mode == CNMP_ACT_AUTO) {
			param->preferred = _ParseNumber(&str[cnt + 1], &cnt);
		}

		// Write
		if (mode == ATW) {
			if (memcmp(&wParam, param, sizeof(at_cnmp_t)) != 0) {
				if (wParam.mode == CNMP_ACT_AUTO) {
					sprintf(wStr, "AT+CNMP=%d%d\r", wParam.mode, wParam.preferred);
				} else {
					sprintf(wStr, "AT+CNMP=%d\r", wParam.mode);
				}

				p = AT_CmdWrite(wStr, 10000, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_NetworkAttachedStatus(AT_MODE mode, at_csact_t *param) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by value
	at_csact_t wParam = *param;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CSACT?\r", "+CSACT: ", &str);
	if (p) {
		param->act = _ParseNumber(&str[0], &cnt);
		_ParseText(&str[cnt + 1], &cnt, param->rac);
		param->creg = _ParseNumber(&str[cnt + 1], &cnt);
		param->cgreg = _ParseNumber(&str[cnt + 1], &cnt);

		// Write
		if (mode == ATW) {
			if (wParam.cgreg != param->creg || wParam.cgreg != param->cgreg) {
				sprintf(wStr, "AT+CSACT=%d,%d\r", wParam.creg, wParam.cgreg);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_ShowRemoteIp(AT_MODE mode, AT_BOOL *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by vale
	AT_BOOL wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPSRIP?\r", "+CIPSRIP: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CIPSRIP=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_IpPackageHeader(AT_MODE mode, AT_BOOL *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[14];

	// Copy by vale
	AT_BOOL wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CIPHEAD?\r", "+CIPHEAD: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CIPHEAD=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_GetLocalTimestamp(AT_MODE mode, AT_BOOL *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[11];

	// Copy by vale
	AT_BOOL wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CLTS?\r", "+CLTS: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CLTS=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_ConfigureSlowClock(AT_MODE mode, AT_CSCLK *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[12];

	// Copy by value
	AT_CSCLK wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CSCLK?\r", "+CSCLK: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CSCLK=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}

	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_ReportMobileEquipmentError(AT_MODE mode, AT_CMEE *state) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[11];

	// Copy by value
	AT_CMEE wState = *state;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+CMEE?\r", "+CMEE: ", &str);
	if (p) {
		*state = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wState != *state) {
				sprintf(wStr, "AT+CMEE=%d\r", wState);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_FixedLocalRate(AT_MODE mode, uint32_t *rate) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[15];

	// Copy by value
	uint32_t wRate = *rate;

	Simcom_Lock();
	// Read
	p = AT_CmdRead("AT+IPR?\r", "+IPR: ", &str);
	if (p) {
		*rate = _ParseNumber(&str[0], &cnt);

		// Write
		if (mode == ATW) {
			if (wRate != *rate) {
				sprintf(wStr, "AT+IPR=%ld\r", wRate);
				p = AT_CmdWrite(wStr, 500, NULL);
			}
		}
	}
	Simcom_Unlock();

	return p;
}

SIMCOM_RESULT AT_Clock(AT_MODE mode, timestamp_t *tm) {
	SIMCOM_RESULT p = SIM_RESULT_ERROR;
	uint8_t cnt;
	char *str = NULL, wStr[32];

	Simcom_Lock();
	if (mode == ATW) {
		// Write
		sprintf(wStr, "AT+CCLK=\"%d/%d/%d,%d,%d,%d%d\"\r",
				tm->date.Year,
				tm->date.Month,
				tm->date.Date,
				tm->time.Hours,
				tm->time.Minutes,
				tm->time.Seconds,
				tm->tzQuarterHour);
		p = AT_CmdWrite(wStr, 500, NULL);

	} else {
		// Read
		p = AT_CmdRead("AT+CCLK?\r", "+CCLK: ", &str);
		if (p) {
			tm->date.Year = _ParseNumber(&str[1], &cnt);
			tm->date.Month = _ParseNumber(&str[cnt + 1], &cnt);
			tm->date.Date = _ParseNumber(&str[cnt + 1], &cnt);
			tm->time.Hours = _ParseNumber(&str[cnt + 1], &cnt);
			tm->time.Minutes = _ParseNumber(&str[cnt + 1], &cnt);
			tm->time.Seconds = _ParseNumber(&str[cnt + 1], &cnt);
			tm->tzQuarterHour = _ParseNumber(&str[cnt + 1], NULL);

			// Formatting
			tm->date.WeekDay = RTC_WEEKDAY_MONDAY;
		}
	}
	Simcom_Unlock();

	return p;
}

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
