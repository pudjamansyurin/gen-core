/*
 * _at.c
 *
 *  Created on: May 13, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_at.h"

#include "Drivers/_simcom.h"

/* Private functions prototype
 * --------------------------------------------*/
#if AT_USE_FTP
static uint8_t FindInBuffer(char* prefix, char** str);
#endif
static SIMR SingleString(char* command, AT_MODE mode, char* string,
                         uint8_t size, uint8_t executor);
static SIMR SingleInteger(char* command, AT_MODE mode, int32_t* value,
                          uint8_t executor);
static SIMR CmdWrite(char* cmd, char* reply, uint32_t ms);
static SIMR CmdRead(char* cmd, char* reply, uint32_t ms, char** str);
static void ParseText(const char* ptr, uint8_t* cnt, char* text, uint8_t size);
static int32_t ParseNumber(const char* ptr, uint8_t* cnt);
// static float AT_ParseFloat(const char *ptr, uint8_t *cnt);

/* Public functions implementation
 * --------------------------------------------*/
SIMR AT_CommandEchoMode(uint8_t state) {
  SIMR res = SIM_ERROR;
  char cmd[8];

  SIM_Lock();
  // Write
  sprintf(cmd, "ATE%d\r", state);
  res = CmdWrite(cmd, SIM_RSP_OK, 500);
  SIM_Unlock();

  return res;
}

SIMR AT_QueryTransmittedData(at_cipack_t* info) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char* str = NULL;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CIPACK?\r", "+CIPACK: ", 500, &str);
  if (res == SIM_OK) {
    info->txlen = ParseNumber(&str[len], &cnt);
    len += cnt + 1;
    info->acklen = ParseNumber(&str[len], &cnt);
    len += cnt + 1;
    info->nacklen = ParseNumber(&str[len], NULL);
  }
  SIM_Unlock();

  return res;
}

SIMR AT_SignalQualityReport(at_csq_t* signal) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char* str = NULL;
  float dBm;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CSQ\r", "+CSQ: ", 500, &str);
  if (res == SIM_OK) {
    signal->rssi = ParseNumber(&str[len], &cnt);
    len += cnt + 1;
    signal->ber = ParseNumber(&str[len], NULL);

    // Formatting
    {
      // Handle not detectable value
      if (signal->rssi > 31) signal->rssi = 0;

      // Scale RSSI to dBm
      dBm = (signal->rssi * 63.0 / 31.0) - 115.0;
      // Scale dBm to percentage
      signal->percent = (dBm + 115.0) * 100.0 / 63.0;

      // debugging
      printf("Simcom:RSSI = %u %%\n", signal->percent);
    }
  }
  SIM_Unlock();

  return res;
}

SIMR AT_ConnectionStatus(SIM_IP* state) {
  SIMR res = SIM_ERROR;
  char status[20], *str = NULL;
  uint8_t len;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CIPSTATUS\r", "STATE: ", 500, &str);
  if (res == SIM_OK) {
    ParseText(&str[0], NULL, status, sizeof(status));
    len = strnlen(status, sizeof(status));
    // decide
    if (!strncmp(status, "IP INITIAL", len)) {
      *state = SIM_IP_INITIAL;
    } else if (!strncmp(status, "IP START", len)) {
      *state = SIM_IP_START;
    } else if (!strncmp(status, "IP CONFIG", len)) {
      *state = SIM_IP_CONFIG;
    } else if (!strncmp(status, "IP GPRSACT", len)) {
      *state = SIM_IP_GPRSACT;
    } else if (!strncmp(status, "IP STATUS", len)) {
      *state = SIM_IP_STATUS;
    } else if (!strncmp(status, "TCP CONNECTING", len) ||
               !strncmp(status, "UDP CONNECTING", len) ||
               !strncmp(status, "SERVER LISTENING", len)) {
      *state = SIM_IP_CONNECTING;
    } else if (!strncmp(status, "CONNECT OK", len)) {
      *state = SIM_IP_CONNECT_OK;
    } else if (!strncmp(status, "TCP CLOSING", len) ||
               !strncmp(status, "UDP CLOSING", len)) {
      *state = SIM_IP_CLOSING;
    } else if (!strncmp(status, "TCP CLOSED", len) ||
               !strncmp(status, "UDP CLOSED", len)) {
      *state = SIM_IP_CLOSED;
    } else if (!strncmp(status, "PDP DEACT", len)) {
      *state = SIM_IP_PDP_DEACT;
    } else {
      *state = SIM_IP_UNKNOWN;
    }
  } else
    *state = SIM_IP_UNKNOWN;
  SIM_Unlock();

  return res;
}

SIMR AT_RadioAccessTechnology(AT_MODE mode, at_cnmp_t* param) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char *str = NULL, cmd[14];

  // Copy by value
  at_cnmp_t tmp = *param;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CNMP?\r", "+CNMP: ", 500, &str);
  if (res == SIM_OK) {
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
  SIM_Unlock();

  return res;
}

SIMR AT_NetworkAttachedStatus(AT_MODE mode, at_csact_t* param) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char *str = NULL, cmd[20];

  // Copy by value
  at_csact_t tmp = *param;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CSACT?\r", "+CSACT: ", 500, &str);
  if (res == SIM_OK) {
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
  SIM_Unlock();

  return res;
}

SIMR AT_NetworkRegistration(char command[20], AT_MODE mode,
                            at_c_greg_t* param) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char *str = NULL, cmd[14], reply[15];

  // Copy by value
  at_c_greg_t tmp = *param;

  SIM_Lock();
  // Read
  sprintf(cmd, "AT+%s?\r", command);
  sprintf(reply, "+%s: ", command);
  res = CmdRead(cmd, reply, 500, &str);
  if (res == SIM_OK) {
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
  SIM_Unlock();

  return res;
}

SIMR AT_ConfigureSlowClock(AT_MODE mode, AT_CSCLK* state) {
  return SingleInteger("CSCLK", mode, (int32_t*)state, 0);
}

SIMR AT_ReportMobileEquipmentError(AT_MODE mode, AT_CMEE* state) {
  return SingleInteger("CMEE", mode, (int32_t*)state, 0);
}

SIMR AT_FixedLocalRate(AT_MODE mode, uint32_t* rate) {
  return SingleInteger("IPR", mode, (int32_t*)rate, 0);
}

SIMR AT_GprsAttachment(AT_MODE mode, AT_CGATT* state) {
  return SingleInteger("CGATT", mode, (int32_t*)state, 0);
}

#if AT_USE_CLK
SIMR AT_EnableLocalTimestamp(AT_MODE mode, AT_BOOL* state) {
  return SingleInteger("CLTS", mode, (int32_t*)state, 0);
}

SIMR AT_Clock(AT_MODE mode, timestamp_t* tm) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char *str = NULL, cmd[39];

  SIM_Lock();
  if (mode == ATW) {
    // Write
    sprintf(cmd, "AT+CCLK=\"%d/%d/%d,%d:%d:%d%+d\"\r", tm->date.Year,
            tm->date.Month, tm->date.Date, tm->time.Hours, tm->time.Minutes,
            tm->time.Seconds, tm->tzQuarterHour);
    res = CmdWrite(cmd, SIM_RSP_OK, 500);

  } else {
    // Read
    res = CmdRead("AT+CCLK?\r", "+CCLK: ", 500, &str);
    if (res == SIM_OK) {
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
  SIM_Unlock();

  return res;
}
#endif

#if AT_USE_SMS
SIMR AT_CharacterSetTE(AT_MODE mode, char* chset, uint8_t len) {
  return SingleString("CSCS", mode, chset, len, 0);
}

SIMR AT_ServiceDataUSSD(AT_MODE mode, at_cusd_t* param, char* buf,
                        uint8_t buflen) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char *str = NULL, *prefix = "+CUSD: ", cmd[40];

  // Copy by value
  at_cusd_t tmp = *param;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CUSD?\r", prefix, 500, &str);
  if (res == SIM_OK) {
    tmp.n = ParseNumber(&str[len], &cnt);
    len += cnt + 1;

    // Write
    if (mode == ATW) {
      sprintf(cmd, "AT+CUSD=%d,\"%s\",%d\r", param->n, param->str, param->dcs);
      res = CmdWrite(cmd, SIM_RSP_OK, 10000);
      // parsing
      if (res == SIM_OK) {
        // parsing
        if ((str = SIM_Resp(prefix, NULL)) != NULL) {
          str += strlen(prefix);
          len = 0;

          tmp.n = ParseNumber(&str[len], &cnt);
          len += cnt + 1;
          ParseText(&str[len], &cnt, buf, buflen);
          len += cnt + 1;
          tmp.dcs = ParseNumber(&str[len], &cnt);
          len += cnt + 1;
        }
      }
    } else {
      memset(tmp.str, 0x00, sizeof(tmp.str));
      tmp.dcs = 0;

      *param = tmp;
    }
  }
  SIM_Unlock();

  return res;
}

SIMR AT_MessageIndicationSMS(uint8_t mode, uint8_t mt) {
  SIMR res = SIM_ERROR;
  char cmd[30];

  SIM_Lock();
  sprintf(cmd, "AT+CNMI=%d,%d,0,0,0\r", mode, mt);
  res = CmdWrite(cmd, SIM_RSP_OK, 500);
  SIM_Unlock();

  return res;
}

SIMR AT_MessageFormatSMS(AT_MODE mode, AT_CMGF* state) {
  return SingleInteger("CMGF", mode, (int32_t*)state, 0);
}

uint8_t AT_WaitMessageSMS(at_cmti_t* param, uint32_t timeout) {
  char *str = NULL, *prefix = "+CMTI: ";
  uint8_t cnt, len = 0;
  uint32_t tick;

  // get message index
  tick = _GetTickMS();
  do {
    str = SIM_Resp(prefix, NULL);
  } while (_TickIn(tick, timeout) && str == NULL);

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

SIMR AT_StorageMessageSMS(AT_MODE mode, at_cpms_t* param) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char *str = NULL, *prefix = "+CPMS: ", cmd[50];
  const uint8_t memTot = sizeof(at_cpms_t) / sizeof(at_cpms_mem_t);

  // Copy by value
  at_cpms_t tmp = *param;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CPMS?\r", prefix, 1000, &str);
  if (res == SIM_OK) {
    for (uint8_t i = 0; i < memTot; i++) {
      ParseText(&str[len], &cnt, tmp.mem[i].storage,
                sizeof(tmp.mem[i].storage));
      len += cnt + 1;
      tmp.mem[i].used = ParseNumber(&str[len], &cnt);
      len += cnt + 1;
      tmp.mem[i].total = ParseNumber(&str[len], &cnt);
      len += cnt + 1;
    }

    // Write
    if (mode == ATW) {
      uint8_t diff = 0;
      for (uint8_t i = 0; i < memTot; i++) {
        if (memcmp(tmp.mem[i].storage, param->mem[i].storage,
                   sizeof(tmp.mem[i].storage)) != 0) {
          diff = 1;
          break;
        }
      }

      if (diff) {
        sprintf(cmd, "AT+CPMS=\"%s\",\"%s\",\"%s\"\r", param->mem[0].storage,
                param->mem[1].storage, param->mem[2].storage);
        res = CmdWrite(cmd, SIM_RSP_OK, 1000);
      }
    }

    for (uint8_t i = 0; i < memTot; i++) {
      param->mem[i].used = tmp.mem[i].used;
      param->mem[i].total = tmp.mem[i].total;
    }
  }
  SIM_Unlock();

  return res;
}

SIMR AT_DeleteMessageSMS(at_cmgd_t* param) {
  SIMR res = SIM_ERROR;
  char cmd[30];

  SIM_Lock();
  // Write
  sprintf(cmd, "AT+CMGD=%d,%d\r", param->index, param->delflag);
  res = CmdWrite(cmd, SIM_RSP_OK, 500);
  SIM_Unlock();

  return res;
}

SIMR AT_ReadMessageSMS(at_cmgr_t* param, char* buf, uint8_t buflen) {
  SIMR res = SIM_ERROR;
  char *end, *str = NULL, cmd[30], *prefix = "+CMGR: ";

  SIM_Lock();
  // Write
  sprintf(cmd, "AT+CMGR=%d,%d\r", param->index, param->mode);
  res = CmdWrite(cmd, SIM_RSP_OK, 500);

  if (res == SIM_OK) {
    // parsing
    if ((str = SIM_Resp(prefix, NULL)) != NULL) {
      str += strlen(prefix);
      if ((str = SIM_Resp(SIM_RSP_NONE, str)) != NULL) {
        str += strlen(SIM_RSP_NONE);
        if ((end = SIM_Resp(SIM_RSP_OK, str)) != NULL) {
          if ((end - str) < buflen) buflen = (end - str);
          strncpy(buf, str, buflen - 1);
        }
      }
    }
  }
  SIM_Unlock();

  return res;
}

SIMR AT_ListMessageSMS(at_cmgl_t* param) {
  SIMR res = SIM_ERROR;
  char cmd[30];
  char stat[5][11] = {"REC UNREAD", "REC READ", "STO UNSENT", "STO SENT",
                      "ALL"};

  SIM_Lock();
  // Write
  sprintf(cmd, "AT+CMGL=\"%s\",%d\r", stat[param->stat], param->mode);
  res = CmdWrite(cmd, SIM_RSP_OK, 5000);
  SIM_Unlock();

  return res;
}
#endif

#if AT_USE_TCP
SIMR AT_ConfigureAPN(AT_MODE mode, con_apn_t* param) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char *str = NULL, cmd[120];

  // Copy by value
  con_apn_t tmp = *param;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CSTT?\r", "+CSTT: ", 500, &str);
  if (res == SIM_OK) {
    ParseText(&str[len], &cnt, tmp.name, sizeof(tmp.name));
    len += cnt + 1;
    ParseText(&str[len], &cnt, tmp.user, sizeof(tmp.user));
    len += cnt + 1;
    ParseText(&str[len], &cnt, tmp.pass, sizeof(tmp.pass));

    // Write
    if (mode == ATW) {
      if (memcmp(&tmp, param, sizeof(tmp)) != 0) {
        sprintf(cmd, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r", param->name, param->user,
                param->pass);
        res = CmdWrite(cmd, SIM_RSP_OK, 1000);
      }
    } else
      *param = tmp;
  }
  SIM_Unlock();

  return res;
}

SIMR AT_GetLocalIpAddress(at_cifsr_t* param) {
  SIMR res = SIM_ERROR;
  char* str = NULL;

  SIM_Lock();
  // Read
  res = CmdRead("AT+CIFSR\r", SIM_RSP_NONE, 500, &str);
  if (res == SIM_OK)
    ParseText(&str[0], NULL, param->address, sizeof(param->address));

  SIM_Unlock();

  return res;
}

SIMR AT_StartConnection(const con_mqtt_t* param) {
  SIMR res = SIM_ERROR;
  char cmd[80];

  SIM_Lock();
  // Write
  sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r", param->host, param->port);
  res = CmdWrite(cmd, "CONNECT", 30000);

  // check either connection ok / error
  if (res == SIM_OK) {
    if (SIM_Resp("CONNECT OK", NULL) || SIM_Resp("ALREADY CONNECT", NULL) ||
        SIM_Resp("TCP CLOSED", NULL)) {
      res = SIM_OK;
    } else
      res = SIM_ERROR;
  }
  SIM_Unlock();

  return res;
}

SIMR AT_ManuallyReceiveData(AT_MODE mode, AT_CIPRXGET* state) {
  return SingleInteger("CIPRXGET", mode, (int32_t*)state, 0);
}

SIMR AT_MultiIpConnection(AT_MODE mode, AT_CIPMUX* state) {
  return SingleInteger("CIPMUX", mode, (int32_t*)state, 0);
}

SIMR AT_TcpApllicationMode(AT_MODE mode, AT_CIPMODE* state) {
  return SingleInteger("CIPMODE", mode, (int32_t*)state, 0);
}

SIMR AT_ShowRemoteIp(AT_MODE mode, AT_BOOL* state) {
  return SingleInteger("CIPSRIP", mode, (int32_t*)state, 0);
}

SIMR AT_IpPackageHeader(AT_MODE mode, AT_BOOL* state) {
  return SingleInteger("CIPHEAD", mode, (int32_t*)state, 0);
}

SIMR AT_DataTransmitMode(AT_MODE mode, AT_CIPQSEND* state) {
  return SingleInteger("CIPQSEND", mode, (int32_t*)state, 0);
}
#endif

#if AT_USE_FTP
SIMR AT_BearerInitialize(void) {
  SIMR res;
  at_sapbr_t getBEARER, setBEARER = {
                            .cmd_type = SAPBR_BEARER_OPEN,
                            .status = SAPBR_CONNECTED,
                        };

  // BEARER attach
  res = AT_BearerSettings(ATW, &setBEARER);

  // BEARER init
  if (res == SIM_OK) res = AT_BearerSettings(ATR, &getBEARER);

  if (res == SIM_OK && getBEARER.status != SAPBR_CONNECTED) res = SIM_ERROR;

  return res;
}

SIMR AT_BearerSettings(AT_MODE mode, at_sapbr_t* param) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char *str = NULL, cmd[80];

  // Copy by value
  at_sapbr_t tmp = *param;
  con_apn_t tmpApn;

  SIM_Lock();
  // Read
  sprintf(cmd, "AT+SAPBR=%d,1\r", SAPBR_BEARER_QUERY);
  res = CmdRead(cmd, "+SAPBR: ", 500, &str);
  if (res == SIM_OK) {
    tmp.cmd_type = ParseNumber(&str[len], &cnt);
    len += cnt + 1;
    tmp.status = ParseNumber(&str[len], &cnt);

    // Read parameters
    res = CmdRead("AT+SAPBR=4,1\r", "+SAPBR:", 500, &str);
    if (res == SIM_OK) {
      if (FindInBuffer("APN: ", &str))
        ParseText(&str[0], NULL, tmpApn.name, sizeof(tmpApn.name));

      if (FindInBuffer("USER: ", &str))
        ParseText(&str[0], NULL, tmpApn.user, sizeof(tmpApn.user));

      if (FindInBuffer("PWD: ", &str))
        ParseText(&str[0], NULL, tmpApn.pass, sizeof(tmpApn.pass));
    }

    // Write
    if (mode == ATW) {
      const con_apn_t* parApn = SIMCon_IO_Apn();

      if (memcmp(tmpApn.name, parApn->name, sizeof(tmpApn.name)) != 0) {
        sprintf(cmd, "AT+SAPBR=3,1,\"APN\",\"%s\"\r", parApn->name);
        res = CmdWrite(cmd, SIM_RSP_OK, 500);
      }
      if (memcmp(tmpApn.user, parApn->user, sizeof(tmpApn.user)) != 0) {
        sprintf(cmd, "AT+SAPBR=3,1,\"USER\",\"%s\"\r", parApn->user);
        res = CmdWrite(cmd, SIM_RSP_OK, 500);
      }
      if (memcmp(tmpApn.pass, parApn->pass, sizeof(tmpApn.pass)) != 0) {
        sprintf(cmd, "AT+SAPBR=3,1,\"PWD\",\"%s\"\r", parApn->pass);
        res = CmdWrite(cmd, SIM_RSP_OK, 500);
      }

      // open or close
      if (tmp.status != param->status) {
        sprintf(cmd, "AT+SAPBR=%d,1\r", param->cmd_type);
        res = CmdWrite(cmd, SIM_RSP_OK, 30000);
      }
    } else {
      *param = tmp;
      SIMCon_IO_SetApn(&tmpApn);
    }
  }
  SIM_Unlock();

  return res;
}

SIMR AT_FtpInitialize(at_ftp_t* param) {
  SIMR res;
  int32_t cid = 1;

  con_ftp_t* ftp = (con_ftp_t*)SIMCon_IO_Ftp();

  SIM_Lock();
  res = SingleInteger("FTPCID", ATW, &cid, 0);

  if (res == SIM_OK)
    res = SingleString("FTPSERV", ATW, ftp->host, sizeof(ftp->host), 0);

  if (res == SIM_OK)
    res = SingleString("FTPUN", ATW, ftp->user, sizeof(ftp->user), 0);

  if (res == SIM_OK)
    res = SingleString("FTPPW", ATW, ftp->pass, sizeof(ftp->pass), 0);

  if (res == SIM_OK)
    res = SingleString("FTPGETPATH", ATW, param->path, sizeof(param->path), 0);

  if (res == SIM_OK)
    res = SingleString("FTPGETNAME", ATW, param->file, sizeof(param->file), 0);

  SIM_Unlock();
  return res;
}

SIMR AT_FtpFileSize(at_ftp_t* param) {
  SIMR res = SIM_ERROR;
  uint8_t cnt, len = 0;
  char* str = NULL;

  SIM_Lock();
  // Read
  res = CmdRead("AT+FTPSIZE\r", "+FTPSIZE: ", 30000, &str);
  if (res == SIM_OK) {
    // parsing
    ParseNumber(&str[len], &cnt);
    len += cnt + 1;
    param->response = ParseNumber(&str[len], &cnt);
    len += cnt + 1;

    if (param->response != FTP_FINISH)
      res = SIM_ERROR;
    else
      param->size = ParseNumber(&str[len], &cnt);
  }
  SIM_Unlock();

  return res;
}

SIMR AT_FtpDownload(at_ftpget_t* param) {
  SIMR res = SIM_ERROR;
  uint32_t tick;
  uint8_t cnt, len = 0;
  char *resp, *str = NULL, cmd[80];

  SIM_Lock();
  // Open or Read
  if (param->mode == FTPGET_OPEN)
    sprintf(cmd, "AT+FTPGET=%d\r", param->mode);
  else
    sprintf(cmd, "AT+FTPGET=%d,%d\r", param->mode, param->reqlength);

  res = CmdRead(cmd, "+FTPGET: ", 30000, &str);

  if (res == SIM_OK) {
    // parsing
    ParseNumber(&str[len], &cnt);
    len += cnt + 1;

    if (param->mode == FTPGET_OPEN) {
      param->response = ParseNumber(&str[len], &cnt);

      if (param->response != FTP_READY) res = SIM_ERROR;
    } else {
      param->cnflength = ParseNumber(&str[len], &cnt);
      len += cnt + 2;
      // start of file content
      param->ptr = &str[len];

      // wait until data transferred (got OK)
      resp = &str[len + param->cnflength + 2];
      tick = _GetTickMS();
      while (strncmp(resp, SIM_RSP_OK, strlen(SIM_RSP_OK)) != 0) {
        if (_TickOut(tick, 10 * 1000)) {
          res = SIM_ERROR;
          break;
        };
        _DelayMS(1);
      };
    }
  }
  SIM_Unlock();

  return res;
}

SIMR AT_FtpCurrentState(AT_FTP_STATE* state) {
  return SingleInteger("FTPSTATE", ATR, (int32_t*)state, 1);
}

SIMR AT_FtpResume(uint32_t start) {
  return SingleInteger("FTPREST", ATW, (int32_t*)&start, 0);
}
#endif

/* Private functions implementation
 * --------------------------------------------*/
#if AT_USE_FTP
static uint8_t FindInBuffer(char* prefix, char** str) {
  *str = SIM_Resp(prefix, NULL);

  if (*str != NULL) *str += strlen(prefix);

  return *str != NULL;
}
#endif

static SIMR SingleString(char* command, AT_MODE mode, char* string,
                         uint8_t size, uint8_t executor) {
  SIMR res = SIM_ERROR;
  char *str = NULL, cmd[20], reply[20], tmp[size];

  // Copy by vale
  // memcpy(tmp, string, size);

  SIM_Lock();
  // Read
  sprintf(cmd, "AT+%s%s", command, executor ? "\r" : "?\r");
  sprintf(reply, "+%s: ", command);
  res = CmdRead(cmd, reply, 1000, &str);
  if (res == SIM_OK) {
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
  SIM_Unlock();

  return res;
}

static SIMR SingleInteger(char* command, AT_MODE mode, int32_t* value,
                          uint8_t executor) {
  SIMR res = SIM_ERROR;
  char *str = NULL, cmd[20], reply[20];

  // Copy by vale
  int32_t tmp = *value;

  SIM_Lock();
  // Read
  sprintf(cmd, "AT+%s%s", command, executor ? "\r" : "?\r");
  sprintf(reply, "+%s: ", command);
  res = CmdRead(cmd, reply, 1000, &str);
  if (res == SIM_OK) {
    tmp = ParseNumber(&str[0], NULL);

    // Write
    if (mode == ATW) {
      if (tmp != *value) {
        sprintf(cmd, "AT+%s=%d\r", command, (int)*value);
        res = CmdWrite(cmd, SIM_RSP_OK, 500);
      }
    } else
      *value = tmp;
  }
  SIM_Unlock();

  return res;
}

static SIMR CmdWrite(char* cmd, char* reply, uint32_t ms) {
  SIMR res = SIM_ERROR;

  if (SIMSta_IO_State() >= SIM_STATE_READY) res = SIM_Cmd(cmd, reply, ms);

  return res;
}

static SIMR CmdRead(char* cmd, char* reply, uint32_t ms, char** str) {
  SIMR res = SIM_ERROR;

  if (SIMSta_IO_State() >= SIM_STATE_READY) {
    res = SIM_Cmd(cmd, reply, ms);

    if (res == SIM_OK) {
      *str = SIM_Resp(reply, NULL);

      if (*str != NULL) {
        *str += strlen(reply);

        res = SIM_OK;
      }
    }
  }

  return res;
}

static void ParseText(const char* ptr, uint8_t* cnt, char* text, uint8_t size) {
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
    if (size <= 1) break;
  }
  // end of parsing for : double-quote, tab, new-line
  *text = '\0';
  ptr++;
  i++;
  // Save number of characters used for number
  if (cnt != NULL) *cnt = i;
}

static int32_t ParseNumber(const char* ptr, uint8_t* cnt) {
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

// static float AT_ParseFloat(const char *ptr, uint8_t *cnt) {
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
