/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */
/* Includes ------------------------------------------------------------------*/
#include "_simcom.h"

/* External variables ---------------------------------------------------------*/
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];
extern osMutexId SimcomRecMutexHandle;
extern osMailQId CommandMailHandle;

/* Public variables ----------------------------------------------------------*/
sim_t SIM;

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT Simcom_Ready(void);
static SIMCOM_RESULT Simcom_Power(void);
static SIMCOM_RESULT Simcom_Reset(void);
static SIMCOM_RESULT Simcom_Iterate(char cmd[20], char contain[10], char resp[15]);
static SIMCOM_RESULT Simcom_SendDirect(char *data, uint16_t len, uint32_t ms, char *res);
static SIMCOM_RESULT Simcom_SendIndirect(char *data, uint16_t len, uint8_t is_payload, uint32_t ms, char *res, uint8_t n);
static SIMCOM_RESULT Simcom_Response(char *str);
static void Simcom_Prepare(void);
static void Simcom_ClearBuffer(void);
static SIMCOM_RESULT Simcom_Command(char *cmd, uint32_t ms, uint8_t n, char *res);

/* Public functions implementation --------------------------------------------*/
void Simcom_Sleep(uint8_t state) {
  HAL_GPIO_WritePin(INT_NET_DTR_GPIO_Port, INT_NET_DTR_Pin, state);
  osDelay(50);
  // save it
  SIM.sleep = state;
  // debug
  //  LOG_Str("Simcom:Sleep = ");
  //  LOG_Int(state);
  //  LOG_Enter();
}

void Simcom_Init(SIMCOM_STATE state) {
  SIMCOM_RESULT p;
  uint8_t step = 1;
  static uint8_t init = 1;

  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);
  // this do-while is complicated, but it doesn't use recursive function, so it's stack safe
  do {
    // debug
    //    LOG_Str("Simcom:State = ");
    //    LOG_Int(SIM.state);
    //    LOG_Enter();

    // only executed at power up
    if (init) {
      init = 0;
      LOG_StrLn("Simcom:Init");
      // set default value to variable
      Simcom_Prepare();
      // simcom power control
      p = Simcom_Power();
    } else {
      if (SIM.state == SIM_STATE_DOWN) {
        LOG_StrLn("Simcom:Down");
        p = SIM_RESULT_ERROR;
      } else {
        p = SIM_RESULT_OK;
      }
    }

    // handle simcom states
    switch (SIM.state) {
      case SIM_STATE_DOWN:
        // save event
        Reporter_WriteEvent(REPORT_NETWORK_RESTART, 1);
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
        if (p == SIM_RESULT_OK) {
          SIM.state = SIM_STATE_READY;
        }

        break;
      case SIM_STATE_READY:
        // =========== BASIC CONFIGURATION
        // disable command echo
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("ATE0\r", 500, 1);
        }
        // Set serial baud-rate
        //        if (p == SIM_RESULT_OK) {
        //                p = Simcom_Cmd("AT+IPR=9600\r", 500, 1);
        //        }
        // Error report format: 0, 1(Numeric), 2(verbose)
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CMEE=2\r", 500, 1);
        }
        // Use pin DTR as sleep control
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CSCLK=1\r", 500, 1);
        }
        // Enable time reporting
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CLTS=1\r", 500, 1);
        }
        // Show “+IPD” header
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CIPHEAD=1\r", 500, 1);
        }
        // Hide “RECV FROM” header
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CIPSRIP=0\r", 500, 1);
        }
        // =========== NETWORK CONFIGURATION
        // Check SIM Card
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CPIN?\r", 500, 1);
        }
        // Disable presentation of <AcT>&<rac> at CREG and CGREG
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CSACT=0,0\r", 500, 1);
        }
        // Set signal Generation 2G(13)/3G(14)/AUTO(2)
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CNMP=2,14\r", 10000, 1);
        }

        // upgrade simcom state
        if (p == SIM_RESULT_OK) {
          SIM.state = SIM_STATE_CONFIGURED;
        }

        break;
      case SIM_STATE_CONFIGURED:
        // =========== NETWORK ATTACH
        // Network Registration Status
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CREG=0\r", 500, 1);
        }
        // wait until attached
        if (p == SIM_RESULT_OK) {
          p = Simcom_Iterate("AT+CREG?\r", "+CREG:", "+CREG: 0,1");
        }
        // upgrade simcom state
        if (p == SIM_RESULT_OK) {
          SIM.state = SIM_STATE_NETWORK_ON;
        }

        break;
      case SIM_STATE_NETWORK_ON:
        // =========== GPRS ATTACH
        // GPRS Registration Status
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CGREG=0\r", 500, 1);
        }
        // wait until attached
        if (p == SIM_RESULT_OK) {
          p = Simcom_Iterate("AT+CGREG?\r", "+CGREG:", "+CGREG: 0,1");
        }
        // upgrade simcom state
        if (p == SIM_RESULT_OK) {
          SIM.state = SIM_STATE_GPRS_ON;
        } else {
          if (SIM.state == SIM_STATE_NETWORK_ON) {
            SIM.state = SIM_STATE_CONFIGURED;
          }
        }

        break;
      case SIM_STATE_GPRS_ON:
        // =========== PDP CONFIGURATION
        // Attach to GPRS service
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CGATT=1\r", 500, 1);
        }
        // wait until attached
        if (p == SIM_RESULT_OK) {
          p = Simcom_Iterate("AT+CGATT?\r", "+CGATT:", "+CGATT: 1");
        }

        // Select TCPIP application mode:
        // (0: Non Transparent (command mode), 1: Transparent (data mode))
        if (p == SIM_RESULT_OK) {
          p = Simcom_Command("AT+CIPMODE?\r", 500, 1, "+CIPMODE:");

          if (p == SIM_RESULT_OK) {
            if (!Simcom_Response("+CIPMODE: 0")) {
              p = Simcom_Cmd("AT+CIPMODE=0\r", 500, 1);
            }
          }
        }
        // Set to Single IP Connection (Backend)
        if (p == SIM_RESULT_OK) {
          p = Simcom_Command("AT+CIPMUX?\r", 500, 1, "+CIPMUX:");

          if (p == SIM_RESULT_OK) {
            if (!Simcom_Response("+CIPMUX: 0")) {
              p = Simcom_Cmd("AT+CIPMUX=0\r", 500, 1);
            }
          }
        }
        // Get data from network automatically
        if (p == SIM_RESULT_OK) {
          p = Simcom_Command("AT+CIPRXGET?\r", 500, 1, "+CIPRXGET:");

          if (p == SIM_RESULT_OK) {
            if (!Simcom_Response("+CIPRXGET: 0")) {
              p = Simcom_Cmd("AT+CIPRXGET=0\r", 500, 1);
            }
          }
        }

        // upgrade simcom state
        if (p == SIM_RESULT_OK) {
          SIM.state = SIM_STATE_PDP_ON;
        } else {
          if (SIM.state == SIM_STATE_GPRS_ON) {
            SIM.state = SIM_STATE_NETWORK_ON;
          }
        }

        break;
      case SIM_STATE_PDP_ON:
        // =========== PDP ATTACH
        // Set type of authentication for PDP connections of socket
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd(SIM.cmd.CSTT, 1000, 1);
        }
        // =========== IP ATTACH
        // Bring Up IP Connection
        if (p == SIM_RESULT_OK) {
          p = Simcom_Cmd("AT+CIICR\r", 10000, 3);
        }
        // Check IP Address
        if (p == SIM_RESULT_OK) {
          p = Simcom_Command("AT+CIFSR\r", 500, 1, SIMCOM_RSP_NONE);
        }

        // upgrade simcom state
        if (p == SIM_RESULT_OK) {
          SIM.state = SIM_STATE_INTERNET_ON;
        } else {
          Simcom_Cmd("AT+CIPSHUT\r", 1000, 1);
          if (SIM.state == SIM_STATE_PDP_ON) {
            SIM.state = SIM_STATE_GPRS_ON;
          }
        }

        break;
      case SIM_STATE_INTERNET_ON:
        // ============ SOCKET CONFIGURAITON
        //        // Check IP Status
        //        if (p == SIM_RESULT_OK) {
        //          Simcom_Command("AT+CIPSTATUS\r", 500, 1, SIMCOM_RSP_NONE);
        //        }
        // Establish connection with server
        if (p == SIM_RESULT_OK) {
          p = Simcom_Command(SIM.cmd.CIPSTART, 15000, 1, "CONNECT");
          // check either connection ok / error
          if (p == SIM_RESULT_OK) {
            p = Simcom_Response("CONNECT OK");
          }
        }
        // upgrade simcom state
        if (p == SIM_RESULT_OK) {
          SIM.state = SIM_STATE_SERVER_ON;
        } else {
          Simcom_Cmd("AT+CIPSHUT\r", 1000, 1);
          if (SIM.state == SIM_STATE_INTERNET_ON) {
            SIM.state = SIM_STATE_PDP_ON;
          }
        }

        break;
      default:
        break;
    }

    // delay x seconds after retry 3 times failed
    if (p != SIM_RESULT_OK) {
      if (step++ == 3) {
        step = 1;
        LOG_StrLn("Simcom:DelayLong");
        osDelay(30 * 1000);
      } else {
        LOG_StrLn("Simcom:DelayShort");
        osDelay(1000);
      }
    }
  } while (SIM.state < state);

  osRecursiveMutexRelease(SimcomRecMutexHandle);
}

SIMCOM_RESULT Simcom_Upload(char *payload, uint16_t payload_length) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  uint32_t tick;
  char str[20];

  // combine the size
  sprintf(str, "AT+CIPSEND=%d\r", payload_length);

  Simcom_Init(SIM_STATE_SERVER_ON);
  if (SIM.state >= SIM_STATE_SERVER_ON) {
    // wake-up the SIMCOM
    Simcom_Sleep(0);
    SIM.uploading = 1;

    // confirm to server that command is executed
    p = Simcom_Command(str, 5000, 1, SIMCOM_RSP_SEND);
    if (p == SIM_RESULT_OK) {
      // send response
      p = Simcom_SendIndirect(payload, payload_length, 1, 15000, SIMCOM_RSP_SENT, 1);
      // wait for ACK/NACK
      if (p == SIM_RESULT_OK) {
        // set timeout guard
        tick = osKernelSysTick();
        // wait ACK for payload
        while (1) {
          if (Simcom_Response(PREFIX_ACK) ||
              Simcom_Response(PREFIX_NACK) ||
              (osKernelSysTick() - tick) >= pdMS_TO_TICKS(15000)) {
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

    // sleep the SIMCOM
    Simcom_Sleep(1);
    SIM.uploading = 0;
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return p;
}

SIMCOM_RESULT Simcom_Cmd(char *cmd, uint32_t ms, uint8_t n) {
  return Simcom_Command(cmd, ms, n, NULL);
}

SIMCOM_RESULT Simcom_ReadCommand(command_t *command) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

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
            sizeof(command->header.size) + sizeof(command->data), 1);

        // check the CRC
        if (command->header.crc == crcValue) {
          p = SIM_RESULT_OK;
        }
      }
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return p;
}

SIMCOM_RESULT Simcom_ReadACK(report_header_t *report_header) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  ack_t ack;
  char *str = NULL;

  if (Simcom_Response(SIMCOM_RSP_IPD)) {
    // parse ACK
    str = strstr(SIMCOM_UART_RX, PREFIX_ACK);
    if (str != NULL) {
      ack = *(ack_t*) str;

      // validate the value
      if (report_header->frame_id == ack.frame_id &&
          report_header->seq_id == ack.seq_id) {
        p = SIM_RESULT_OK;
      }
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return p;
}

SIMCOM_RESULT Simcom_ReadSignal(uint8_t *signal_percentage) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  uint8_t i, cnt, rssi = 0;
  rssi_t rssiQuality;
  char *str, *prefix = "+CSQ: ";
  // see: http://wiki.teltonika-networks.com/view/Mobile_Signal_Strength_Recommendations
  const rssi_t rssiQualities[5] = {
      { .name = "Excellent", .min = 22, .percentage = 100 },
      { .name = "Good", .min = 14, .percentage = 75 },
      { .name = "Fair", .min = 7, .percentage = 50 },
      { .name = "Poor", .min = 2, .percentage = 25 },
      { .name = "NoSignal", .min = 0, .percentage = 0 }
  };

  if (SIM.state >= SIM_STATE_READY) {
    // check signal quality
    if (Simcom_Cmd("AT+CSQ\r", 500, 1) == SIM_RESULT_OK) {
      // get pointer reference
      str = strstr(SIMCOM_UART_RX, prefix);

      if (str != NULL) {
        str += strlen(prefix);
        rssi = _ParseNumber(&str[0], &cnt);
        //        ber = _ParseNumber(&str[cnt + 1], NULL);

        // handle not detectable rssi value
        rssi = rssi == 99 ? 0 : rssi;

        // find the rssi quality
        for (i = 0; i < (sizeof(rssiQualities) / sizeof(rssiQualities[0])); i++) {
          if (rssi >= rssiQualities[i].min) {
            rssiQuality = rssiQualities[i];
            // set result
            *signal_percentage = rssiQuality.percentage;

            break;
          }
        }

        // debugging
        LOG_Str("\nSimcom:RSSI = ");
        LOG_Buf(rssiQuality.name, strlen(rssiQuality.name));
        LOG_Enter();

        p = SIM_RESULT_OK;
      }
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return p;
}

SIMCOM_RESULT Simcom_ReadTime(timestamp_t *timestamp) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  uint8_t len = 0, cnt;
  char *str, *prefix = "+CCLK: \"";

  if (SIM.state >= SIM_STATE_READY) {
    // get local timestamp (from base station)
    if (Simcom_Cmd("AT+CCLK?\r", 500, 1) == SIM_RESULT_OK) {
      // get pointer reference
      str = strstr(SIMCOM_UART_RX, prefix);

      if (str != NULL) {
        str += strlen(prefix);
        // get date part
        timestamp->date.Year = _ParseNumber(&str[0], &cnt);
        len += cnt + 1;
        timestamp->date.Month = _ParseNumber(&str[len], &cnt);
        len += cnt + 1;
        timestamp->date.Date = _ParseNumber(&str[len], &cnt);
        // get time part
        len += cnt + 1;
        timestamp->time.Hours = _ParseNumber(&str[len], &cnt);
        len += cnt + 1;
        timestamp->time.Minutes = _ParseNumber(&str[len], &cnt);
        len += cnt + 1;
        timestamp->time.Seconds = _ParseNumber(&str[len], NULL);

        // check is carrier timestamp valid
        if (timestamp->date.Year >= VCU_BUILD_YEAR) {
          // set weekday to default
          timestamp->date.WeekDay = RTC_WEEKDAY_MONDAY;

          p = SIM_RESULT_OK;
        }
      }
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return p;
}

/* Private functions implementation --------------------------------------------*/
static SIMCOM_RESULT Simcom_Ready(void) {
  uint32_t tick;

  tick = osKernelSysTick();
  while (1) {
    if (Simcom_Response(SIMCOM_RSP_READY) ||
        (osKernelSysTick() - tick) >= NET_BOOT_TIMEOUT) {
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

static SIMCOM_RESULT Simcom_Iterate(char cmd[20], char contain[10], char resp[15]) {
  SIMCOM_RESULT p;
  uint8_t iteration = 0;

  while (1) {
    p = Simcom_Command(cmd, 500, 1, contain);
    // handle error
    if (SIM.state == SIM_STATE_DOWN) {
      break;
    }
    // handle contain
    if (p == SIM_RESULT_OK) {
      if (Simcom_Response(resp)) {
        break;
      }
    }

    Simcom_ReadSignal(NULL);

    LOG_Str("Simcom:Iteration[");
    LOG_Buf(contain, strlen(contain));
    LOG_Str("] = ");
    LOG_Int(iteration++);
    LOG_Enter();

    osDelay(NET_REPEAT_DELAY);
  }

  return p;
}

static SIMCOM_RESULT Simcom_SendDirect(char *data, uint16_t len, uint32_t ms, char *res) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  SIMCOM_RESULT p;
  uint32_t tick, timeout_tick = 0;

  Simcom_ClearBuffer();
  // transmit to serial (low-level)
  if (!SIMCOM_Transmit(data, len)) {
    LOG_StrLn("Simcom:Transmit = ERROR");
  }
  // convert time to tick
  timeout_tick = pdMS_TO_TICKS(ms + NET_EXTRA_TIME_MS);
  // set timeout guard
  tick = osKernelSysTick();
  // wait response from SIMCOM
  while (1) {
    if (Simcom_Response(res) ||
        Simcom_Response(SIMCOM_RSP_ERROR) ||
        Simcom_Response(SIMCOM_RSP_READY) ||
        (osKernelSysTick() - tick) >= timeout_tick) {

      // set flag for timeout & error
      p = Simcom_Response(res);

      // check & check
      // exception for auto reboot module
      if (Simcom_Response(SIMCOM_RSP_READY) && (SIM.state >= SIM_STATE_READY)) {
        p = SIM_RESULT_RESTARTED;
      }
      // exception for timeout
      if ((osKernelSysTick() - tick) >= timeout_tick) {
        p = SIM_RESULT_TIMEOUT;
      }
      // exception for no response
      if (strlen(SIMCOM_UART_RX) == 0) {
        p = SIM_RESULT_NO_RESPONSE;
      }

      // exit loop
      break;
    }
    osDelay(10);
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return p;
}

static SIMCOM_RESULT Simcom_SendIndirect(char *data, uint16_t len, uint8_t is_payload, uint32_t ms, char *res,
    uint8_t n) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

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

    // exception debug
    switch (p) {
      case SIM_RESULT_RESTARTED:
        LOG_StrLn("Simcom:Restarted");
        SIM.state = SIM_STATE_DOWN;
        break;
      case SIM_RESULT_NO_RESPONSE:
        LOG_StrLn("Simcom:NoResponse");
        SIM.state = SIM_STATE_DOWN;
        break;
      case SIM_RESULT_TIMEOUT:
        LOG_StrLn("Simcom:Timeout");
        break;
      default:
        break;
    }

  } while (seq-- && p == SIM_RESULT_ERROR);

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return p;
}

static void Simcom_Prepare(void) {
  SIM.state = SIM_STATE_DOWN;
  // prepare command sequence
  sprintf(SIM.cmd.CSTT,
      "AT+CSTT=\"%s\",\"%s\",\"%s\"\r",
      NET_APN, NET_APN_USERNAME, NET_APN_PASSWORD);
  sprintf(SIM.cmd.CIPSTART,
      "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r",
      NET_SERVER_IP, NET_SERVER_PORT);
}

static void Simcom_ClearBuffer(void) {
  // debugging
  //  LOG_StrLn("\n=================== START ===================");
  //  LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
  //  LOG_StrLn("\n==================== END ====================");
  // reset rx buffer
  SIMCOM_Reset_Buffer();
}

static SIMCOM_RESULT Simcom_Response(char *str) {
  if (strstr(SIMCOM_UART_RX, str) != NULL) {
    return SIM_RESULT_OK;
  }
  return SIM_RESULT_ERROR;
}

static SIMCOM_RESULT Simcom_Command(char *cmd, uint32_t ms, uint8_t n, char *res) {
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

