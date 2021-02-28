/*
 * _at.c
 *
 *  Created on: May 13, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_at.h"

/* Private functions prototype -----------------------------------------------*/
static uint8_t FindInBuffer(char *prefix, char **str);
static SIMCOM_RESULT SingleString(char *command, AT_MODE mode, char *string, uint8_t size, uint8_t executor);
static SIMCOM_RESULT SingleInteger(char *command, AT_MODE mode, int32_t *value, uint8_t executor);
static SIMCOM_RESULT CmdWrite(char *cmd, char *reply, uint32_t ms);
static SIMCOM_RESULT CmdRead(char *cmd, char *reply, uint32_t ms, char **str);
static void ParseText(const char *ptr, uint8_t *cnt, char *text, uint8_t size);
static int32_t ParseNumber(const char *ptr, uint8_t *cnt);
//static float AT_ParseFloat(const char *ptr, uint8_t *cnt);

/* Public functions implementation --------------------------------------------*/
SIMCOM_RESULT AT_CommandEchoMode(uint8_t state) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char cmd[8];

	Simcom_Lock();
	// Write
	sprintf(cmd, "ATE%d\r", state);
	res = CmdWrite(cmd, SIM_RSP_OK, 500);
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_QueryTransmittedData(at_cipack_t *info) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+CIPACK?\r", "+CIPACK: ", 500, &str);
	if (res > 0) {
		info->txlen = ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		info->acklen = ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		info->nacklen = ParseNumber(&str[len], NULL);
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_SignalQualityReport(at_csq_t *signal) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL;
	float dBm;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+CSQ\r", "+CSQ: ", 500, &str);
	if (res > 0) {
		signal->rssi = ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		signal->ber = ParseNumber(&str[len], NULL);

		// Formatting
		{
			// Handle not detectable value
			if (signal->rssi > 31)
				signal->rssi = 0;

			// Scale RSSI to dBm
			dBm = (signal->rssi * 63.0 / 31.0) - 115.0;
			// Scale dBm to percentage
			signal->percent = (dBm + 115.0) * 100.0 / 63.0;

			// debugging
			printf("Simcom:RSSI = %u %%\n", signal->percent);
		}
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_ConnectionStatusSingle(AT_CIPSTATUS *state) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char status[20], *str = NULL;
	uint8_t len;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+CIPSTATUS\r", "STATE: ", 500, &str);
	if (res > 0) {
		ParseText(&str[0], NULL, status, sizeof(status));
		len = strnlen(status, sizeof(status));
		// decide
		if (!strncmp(status, "IP INITIAL", len)) {
			*state = CIPSTAT_IP_INITIAL;
		} else if (!strncmp(status, "IP START", len)) {
			*state = CIPSTAT_IP_START;
		} else if (!strncmp(status, "IP CONFIG", len)) {
			*state = CIPSTAT_IP_CONFIG;
		} else if (!strncmp(status, "IP GPRSACT", len)) {
			*state = CIPSTAT_IP_GPRSACT;
		} else if (!strncmp(status, "IP STATUS", len)) {
			*state = CIPSTAT_IP_STATUS;
		} else if (!strncmp(status, "TCP CONNECTING", len)
				|| !strncmp(status, "UDP CONNECTING", len)
				|| !strncmp(status, "SERVER LISTENING", len)) {
			*state = CIPSTAT_CONNECTING;
		} else if (!strncmp(status, "CONNECT OK", len)) {
			*state = CIPSTAT_CONNECT_OK;
		} else if (!strncmp(status, "TCP CLOSING", len)
				|| !strncmp(status, "UDP CLOSING", len)) {
			*state = CIPSTAT_CLOSING;
		} else if (!strncmp(status, "TCP CLOSED", len)
				|| !strncmp(status, "UDP CLOSED", len)) {
			*state = CIPSTAT_CLOSED;
		} else if (!strncmp(status, "PDP DEACT", len)) {
			*state = CIPSTAT_PDP_DEACT;
		} else {
			*state = CIPSTAT_UNKNOWN;
		}
	} else
		*state = CIPSTAT_UNKNOWN;
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_RadioAccessTechnology(AT_MODE mode, at_cnmp_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL, cmd[14];

	// Copy by value
	at_cnmp_t tmp = *param;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+CNMP?\r", "+CNMP: ", 500, &str);
	if (res > 0) {
		param->mode = ParseNumber(&str[len], &cnt);
		if (param->mode == CNMP_ACT_AUTO) {
			len += cnt + 1;
			param->preferred = ParseNumber(&str[len], &cnt);
		}

		// Write
		if (mode == ATW) {
			if (memcmp(&tmp, param, sizeof(tmp)) != 0) {
				if (tmp.mode == CNMP_ACT_AUTO)
					sprintf(cmd, "AT+CNMP=%d%d\r", param->mode, param->preferred);
				else
					sprintf(cmd, "AT+CNMP=%d\r", param->mode);

				res = CmdWrite(cmd, SIM_RSP_OK, 10000);
			}
		} else
			*param = tmp;
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_NetworkAttachedStatus(AT_MODE mode, at_csact_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL, cmd[20];

	// Copy by value
	at_csact_t tmp = *param;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+CSACT?\r", "+CSACT: ", 500, &str);
	if (res > 0) {
		tmp.act = ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		ParseText(&str[len], &cnt, tmp.rac, sizeof(tmp.rac));
		len += cnt + 1;
		tmp.creg = ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		tmp.cgreg = ParseNumber(&str[len], &cnt);

		// Write
		if (mode == ATW) {
			if (tmp.cgreg != param->creg || tmp.cgreg != param->cgreg) {
				sprintf(cmd, "AT+CSACT=%d,%d\r", param->creg, param->cgreg);
				res = CmdWrite(cmd, SIM_RSP_OK, 500);
			}
		} else
			*param = tmp;
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_NetworkRegistration(char command[20], AT_MODE mode, at_c_greg_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL, cmd[14], reply[15];

	// Copy by value
	at_c_greg_t tmp = *param;

	Simcom_Lock();
	// Read
	sprintf(cmd, "AT+%s?\r", command);
	sprintf(reply, "+%s: ", command);
	res = CmdRead(cmd, reply, 500, &str);
	if (res > 0) {
		tmp.mode = ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		tmp.stat = ParseNumber(&str[len], &cnt);

		// Write
		if (mode == ATW) {
			if (memcmp(&tmp, param, sizeof(tmp)) != 0) {
				sprintf(cmd, "AT+%s=%d\r", command, param->mode);
				res = CmdWrite(cmd, SIM_RSP_OK, 500);
			}
		} else
			*param = tmp;
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_ConfigureSlowClock(AT_MODE mode, AT_CSCLK *state) {
	return SingleInteger("CSCLK", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_ReportMobileEquipmentError(AT_MODE mode, AT_CMEE *state) {
	return SingleInteger("CMEE", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_FixedLocalRate(AT_MODE mode, uint32_t *rate) {
	return SingleInteger("IPR", mode, (int32_t*) rate, 0);
}

SIMCOM_RESULT AT_GprsAttachment(AT_MODE mode, AT_CGATT *state) {
	return SingleInteger("CGATT", mode, (int32_t*) state, 0);
}

#if (!BOOTLOADER)
SIMCOM_RESULT AT_CharacterSetTE(AT_MODE mode, char *chset, uint8_t len) {
	return SingleString("CSCS", mode, chset, len, 0);
}

SIMCOM_RESULT AT_MessageIndicationSMS(uint8_t mode, uint8_t mt) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char cmd[30];

	Simcom_Lock();
	sprintf(cmd, "AT+CNMI=%d,%d,0,0,0\r", mode, mt);
	res = CmdWrite(cmd, SIM_RSP_OK, 500);
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_MessageFormatSMS(AT_MODE mode, AT_CMGF *state) {
	return SingleInteger("CMGF", mode, (int32_t*) state, 0);
}

uint8_t AT_WaitMessageSMS(at_cmti_t *param, uint32_t timeout) {
	char *str = NULL, *prefix = "+CMTI: ";
	uint8_t cnt, len = 0;
	uint32_t tick;

	// get message index
	tick = _GetTickMS();
	do {
		str = Simcom_Resp(prefix);
	} while(_GetTickMS() - tick < timeout && str == NULL);

	if (str != NULL) {
		str += strlen(prefix);
		// parsing
		ParseText(&str[len], &cnt, param->mem, sizeof(param->mem));
		len += cnt + 1;
		param->index = ParseNumber(&str[len], &cnt);
		len += cnt + 1;
	}

	return str != NULL;
}

SIMCOM_RESULT AT_ServiceDataUSSD(AT_MODE mode, at_cusd_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL, cmd[40];

	// Copy by value
	at_cusd_t tmp = *param;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+CUSD?\r", "+CUSD: ", 500, &str);
	if (res > 0) {
		tmp.n = ParseNumber(&str[len], &cnt);
		len += cnt + 1;

		// Write
		if (mode == ATW) {
			sprintf(cmd, "AT+CUSD=%d,\"%s\",%d\r", param->n, param->str, param->dcs);
			res = CmdWrite(cmd, SIM_RSP_OK, 10000);
		} else {
			memset(tmp.str, 0x00, sizeof(tmp.str));
			tmp.dcs = 0;

			*param = tmp;
		}
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_DeleteMessageSMS(at_cmgd_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char cmd[30];

	Simcom_Lock();
	// Write
	sprintf(cmd, "AT+CMGD=%d,%d\r", param->index, param->delflag);
	res = CmdWrite(cmd, SIM_RSP_OK, 500);
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_ReadMessageSMS(at_cmgr_t *param, char *buf, uint8_t buflen) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char *end, *str = NULL, cmd[30], *prefix = "+CMGR: ";

	Simcom_Lock();
	// Write
	sprintf(cmd, "AT+CMGR=%d,%d\r", param->index, param->mode);
	res = CmdWrite(cmd, SIM_RSP_OK, 500);

	if (res > 0) {
		// parsing
		if ((str = Simcom_Resp(prefix)) != NULL) {
			str += strlen(prefix);
			if ((str = Simcom_RespFrom(str, SIM_RSP_NONE)) != NULL) {
				str += strlen(SIM_RSP_NONE);
				if ((end = Simcom_RespFrom(str, SIM_RSP_OK)) != NULL) {
					if ((end - str) < buflen)
						buflen = (end - str);
					strncpy(buf, str, buflen - 1);
				}
			}
		}
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_ListMessageSMS(at_cmgl_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char cmd[30];
	char stat[5][11] = {
			"REC UNREAD",
			"REC READ",
			"STO UNSENT",
			"STO SENT",
			"ALL"
	};

	Simcom_Lock();
	// Write
	sprintf(cmd, "AT+CMGL=\"%s\",%d\r", stat[param->stat], param->mode);
	res = CmdWrite(cmd, SIM_RSP_OK, 500);
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_ConfigureAPN(AT_MODE mode, at_cstt_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL, cmd[80];

	// Copy by value
	at_cstt_t tmp = *param;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+CSTT?\r", "+CSTT: ", 500, &str);
	if (res > 0) {
		ParseText(&str[len], &cnt, tmp.apn, sizeof(tmp.apn));
		len += cnt + 1;
		ParseText(&str[len], &cnt, tmp.username, sizeof(tmp.username));
		len += cnt + 1;
		ParseText(&str[len], &cnt, tmp.password, sizeof(tmp.password));

		// Write
		if (mode == ATW) {
			if (memcmp(&tmp, param, sizeof(tmp)) != 0) {
				sprintf(cmd, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r",
						param->apn, param->username, param->password);
				res = CmdWrite(cmd, SIM_RSP_OK, 1000);
			}
		} else
			*param = tmp;
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_GetLocalIpAddress(at_cifsr_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char *str = NULL;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+CIFSR\r", SIM_RSP_NONE, 500, &str);
	if (res > 0)
		ParseText(&str[0], NULL, param->address, sizeof(param->address));

	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_StartConnectionSingle(at_cipstart_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char cmd[80];

	Simcom_Lock();
	// Write
	sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",\"%d\"\r",
			param->mode, param->ip, param->port);
	res = CmdWrite(cmd, "CONNECT", 20000);

	// check either connection ok / error
	if (res > 0) {
		if (Simcom_Resp("CONNECT OK")
				|| Simcom_Resp("ALREADY CONNECT")
				|| Simcom_Resp("TCP CLOSED")) {
			res = SIM_RESULT_OK;
		} else
			res = SIM_RESULT_ERROR;
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_Clock(AT_MODE mode, timestamp_t *tm) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL, cmd[39];

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
		res = CmdWrite(cmd, SIM_RSP_OK, 500);

	} else {
		// Read
		res = CmdRead("AT+CCLK?\r", "+CCLK: ", 500, &str);
		if (res > 0) {
			len = 1;
			tm->date.Year = ParseNumber(&str[len], &cnt);
			len += cnt + 1;
			tm->date.Month = ParseNumber(&str[len], &cnt);
			len += cnt + 1;
			tm->date.Date = ParseNumber(&str[len], &cnt);
			len += cnt + 1;
			tm->time.Hours = ParseNumber(&str[len], &cnt);
			len += cnt + 1;
			tm->time.Minutes = ParseNumber(&str[len], &cnt);
			len += cnt + 1;
			tm->time.Seconds = ParseNumber(&str[len], &cnt);
			len += cnt + 1;
			tm->tzQuarterHour = ParseNumber(&str[len], NULL);

			// Formatting
			tm->date.WeekDay = RTC_WEEKDAY_MONDAY;
		}
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_ManuallyReceiveData(AT_MODE mode, AT_CIPRXGET *state) {
	return SingleInteger("CIPRXGET", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_MultiIpConnection(AT_MODE mode, AT_CIPMUX *state) {
	return SingleInteger("CIPMUX", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_TcpApllicationMode(AT_MODE mode, AT_CIPMODE *state) {
	return SingleInteger("CIPMODE", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_ShowRemoteIp(AT_MODE mode, AT_BOOL *state) {
	return SingleInteger("CIPSRIP", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_IpPackageHeader(AT_MODE mode, AT_BOOL *state) {
	return SingleInteger("CIPHEAD", mode, (int32_t*) state, 0);
}

SIMCOM_RESULT AT_EnableLocalTimestamp(AT_MODE mode, AT_BOOL *state) {
	return SingleInteger("CLTS", mode, (int32_t*) state, 0);
}

#else
SIMCOM_RESULT AT_BearerInitialize(void) {
	SIMCOM_RESULT res;
	at_sapbr_t getBEARER, setBEARER = {
			.cmd_type = SAPBR_BEARER_OPEN,
			.status = SAPBR_CONNECTED,
			.con = {
					.apn = NET_CON_APN,
					.username = NET_CON_USERNAME,
					.password = NET_CON_PASSWORD,
			},
	};

	// BEARER attach
	res = AT_BearerSettings(ATW, &setBEARER);

	// BEARER init
	if (res > 0)
		res = AT_BearerSettings(ATR, &getBEARER);

	if (res > 0 && getBEARER.status != SAPBR_CONNECTED)
		res = SIM_RESULT_ERROR;

	return res;
}

SIMCOM_RESULT AT_FtpInitialize(at_ftp_t *param) {
	SIMCOM_RESULT res;
	int32_t cid = 1;

	Simcom_Lock();
	res = SingleInteger("FTPCID", ATW, &cid, 0);

	if (res > 0)
		res = SingleString("FTPSERV", ATW, NET_FTP_SERVER, sizeof(NET_FTP_SERVER), 0);

	if (res > 0)
		res = SingleString("FTPUN", ATW, NET_FTP_USERNAME, sizeof(NET_FTP_USERNAME), 0);

	if (res > 0)
		res = SingleString("FTPPW", ATW, NET_FTP_PASSWORD, sizeof(NET_FTP_PASSWORD), 0);

	if (res > 0)
		res = SingleString("FTPGETPATH", ATW, param->path, sizeof(param->path), 0);

	if (res > 0)
		res = SingleString("FTPGETNAME", ATW, param->file, sizeof(param->file), 0);

	Simcom_Unlock();
	return res;
}

SIMCOM_RESULT AT_FtpFileSize(at_ftp_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL;

	Simcom_Lock();
	// Read
	res = CmdRead("AT+FTPSIZE\r", "+FTPSIZE: ", 20000, &str);
	if (res > 0) {
		// parsing
		ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		param->response = ParseNumber(&str[len], &cnt);

		if (param->response == FTP_FINISH) {
			len += cnt + 1;
			param->size = ParseNumber(&str[len], &cnt);
		}
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_FtpDownload(at_ftpget_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint32_t tick;
	uint8_t cnt, len = 0;
	char *resp, *str = NULL, cmd[80];

	Simcom_Lock();
	// Open or Read
	if (param->mode == FTPGET_OPEN)
		sprintf(cmd, "AT+FTPGET=%d\r", param->mode);
	else
		sprintf(cmd, "AT+FTPGET=%d,%d\r", param->mode, param->reqlength);

	res = CmdRead(cmd, "+FTPGET: ", 20000, &str);

	if (res > 0) {
		// parsing
		ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		if (param->mode == FTPGET_OPEN)
			param->response = ParseNumber(&str[len], &cnt);
		else {
			param->cnflength = ParseNumber(&str[len], &cnt);
			len += cnt + 2;
			// start of file content
			param->ptr = &str[len];

			// wait until data transferred (got OK)
			tick = _GetTickMS();
			resp = &str[len + param->cnflength + 2];
			while (strncmp(resp, SIM_RSP_OK, strlen(SIM_RSP_OK)) != 0) {
				if (_GetTickMS() - tick > (10 * 1000)) {
					res = SIM_RESULT_ERROR;
					break;
				};
				_DelayMS(1);
			};
		}
	}
	Simcom_Unlock();

	return res;
}

SIMCOM_RESULT AT_FtpCurrentState(AT_FTP_STATE *state) {
	return SingleInteger("FTPSTATE", ATR, (int32_t*) state, 1);
}

SIMCOM_RESULT AT_FtpResume(uint32_t start) {
	return SingleInteger("FTPREST", ATW, (int32_t*) &start, 0);
}

SIMCOM_RESULT AT_BearerSettings(AT_MODE mode, at_sapbr_t *param) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	uint8_t cnt, len = 0;
	char *str = NULL, cmd[80];

	// Copy by value
	at_sapbr_t tmp = *param;

	Simcom_Lock();
	// Read
	sprintf(cmd, "AT+SAPBR=%d,1\r", SAPBR_BEARER_QUERY);
	res = CmdRead(cmd, "+SAPBR: ", 500, &str);
	if (res > 0) {
		tmp.cmd_type = ParseNumber(&str[len], &cnt);
		len += cnt + 1;
		tmp.status = ParseNumber(&str[len], &cnt);

		// Read parameters
		res = CmdRead("AT+SAPBR=4,1\r", "+SAPBR:", 500, &str);
		if (res > 0) {
			if (FindInBuffer("APN: ", &str))
				ParseText(&str[0], NULL, tmp.con.apn, sizeof(tmp.con.apn));

			if (FindInBuffer("USER: ", &str))
				ParseText(&str[0], NULL, tmp.con.username, sizeof(tmp.con.username));

			if (FindInBuffer("PWD: ", &str))
				ParseText(&str[0], NULL, tmp.con.password, sizeof(tmp.con.password));
		}

		// Write
		if (mode == ATW) {
			if (memcmp(tmp.con.apn, param->con.apn, sizeof(param->con.apn)) != 0) {
				sprintf(cmd, "AT+SAPBR=3,1,\"APN\",\"%s\"\r", param->con.apn);
				res = CmdWrite(cmd, SIM_RSP_OK, 500);
			}
			if (memcmp(tmp.con.username, param->con.username, sizeof(param->con.username)) != 0) {
				sprintf(cmd, "AT+SAPBR=3,1,\"USER\",\"%s\"\r", param->con.username);
				res = CmdWrite(cmd, SIM_RSP_OK, 500);
			}
			if (memcmp(tmp.con.password, param->con.password, sizeof(param->con.password)) != 0) {
				sprintf(cmd, "AT+SAPBR=3,1,\"PWD\",\"%s\"\r", param->con.password);
				res = CmdWrite(cmd, SIM_RSP_OK, 500);
			}

			// open or close
			if (tmp.status != param->status) {
				sprintf(cmd, "AT+SAPBR=%d,1\r", param->cmd_type);
				res = CmdWrite(cmd, SIM_RSP_OK, 20000);
			}
		} else
			*param = tmp;
	}
	Simcom_Unlock();

	return res;
}
#endif

/* Private functions implementation --------------------------------------------*/
static uint8_t FindInBuffer(char *prefix, char **str) {
	*str = Simcom_Resp(prefix);

	if (*str != NULL)
		*str += strlen(prefix);

	return *str != NULL;
}

static SIMCOM_RESULT SingleString(char *command, AT_MODE mode, char *string, uint8_t size, uint8_t executor) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char *str = NULL, cmd[20], reply[20], tmp[size];

	// Copy by vale
	memcpy(tmp, string, size);

	Simcom_Lock();
	// Read
	sprintf(cmd, "AT+%s%s", command, executor ? "\r" : "?\r");
	sprintf(reply, "+%s: ", command);
	res = CmdRead(cmd, reply, 1000, &str);
	if (res > 0) {
		ParseText(&str[0], NULL, tmp, sizeof(tmp));

		// Write
		if (mode == ATW) {
			if (strncmp(tmp, string, strlen(string)) != 0) {
				sprintf(cmd, "AT+%s=\"%s\"\r", command, string);
				res = CmdWrite(cmd, SIM_RSP_OK, 1000);
			}
		} else
			memcpy(string, tmp, size);
	}
	Simcom_Unlock();

	return res;
}

static SIMCOM_RESULT SingleInteger(char *command, AT_MODE mode, int32_t *value, uint8_t executor) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;
	char *str = NULL, cmd[20], reply[20];

	// Copy by vale
	int32_t tmp = *value;

	Simcom_Lock();
	// Read
	sprintf(cmd, "AT+%s%s", command, executor ? "\r" : "?\r");
	sprintf(reply, "+%s: ", command);
	res = CmdRead(cmd, reply, 1000, &str);
	if (res > 0) {
		tmp = ParseNumber(&str[0], NULL);

		// Write
		if (mode == ATW) {
			if (tmp != *value) {
				sprintf(cmd, "AT+%s=%d\r", command, (int) *value);
				res = CmdWrite(cmd, SIM_RSP_OK, 500);
			}
		} else
			*value = tmp;
	}
	Simcom_Unlock();

	return res;
}

static SIMCOM_RESULT CmdWrite(char *cmd, char *reply, uint32_t ms) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;

	if (SIM.state >= SIM_STATE_READY)
		res = Simcom_Cmd(cmd, reply, ms);

	return res;
}

static SIMCOM_RESULT CmdRead(char *cmd, char *reply, uint32_t ms, char **str) {
	SIMCOM_RESULT res = SIM_RESULT_ERROR;

	if (SIM.state >= SIM_STATE_READY) {
		res = Simcom_Cmd(cmd, reply, ms);

		if (res > 0) {
			*str = Simcom_Resp(reply);

			if (*str != NULL) {
				*str += strlen(reply);

				res = SIM_RESULT_OK;
			}
		}
	}

	return res;
}

static void ParseText(const char *ptr, uint8_t *cnt, char *text, uint8_t size) {
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
		if (size <= 1)
			break;
	}
	// end of parsing for : double-quote, tab, new-line
	*text = '\0';
	ptr++;
	i++;
	// Save number of characters used for number
	if (cnt != NULL)
		*cnt = i;
}

static int32_t ParseNumber(const char *ptr, uint8_t *cnt) {
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
	if (cnt != NULL) /* Save number of characters used for number */
		*cnt = i;

	if (minus) /* Minus detected */
		return 0 - sum;

	return sum; /* Return number */
}

//static float AT_ParseFloat(const char *ptr, uint8_t *cnt) {
//  uint8_t i = 0, j = 0;
//  float sum = 0.0f;
//
//  sum = (float) ParseNumber(ptr, &i); /* Parse number */
//  j += i;
//  ptr += i;
//  if (*ptr == '.') { /* Check decimals */
//      float dec;
//      dec = (float) ParseNumber(ptr + 1, &i);
//      dec /= (float) pow(10, i);
//      if (sum >= 0) {
//          sum += dec;
//      } else {
//          sum -= dec;
//      }
//      j += i + 1;
//  }
//
//  if (cnt != NULL) { /* Save number of characters used for number */
//      *cnt = j;
//  }
//  return sum; /* Return number */
//}
