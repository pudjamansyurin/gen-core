/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */
#include "_simcom.h"

/* Private functions ---------------------------------------------------------*/
static void Simcom_Reset(void);
static void Simcom_Prepare(void);
static void Simcom_Clear_Buffer(void);
static uint8_t Simcom_Response(char *str);
static uint8_t Simcom_Send_Direct(char *data, uint16_t data_length, uint32_t ms, char *res);
static uint8_t Simcom_Send_Indirect(char *data, uint16_t data_length, uint8_t is_payload, uint32_t ms, char *res, uint8_t n);
static uint8_t Simcom_Boot(void);

/* External variable ---------------------------------------------------------*/
extern char SIMCOM_UART_RX_Buffer[SIMCOM_UART_RX_BUFFER_SIZE];
extern const TickType_t tick5000ms;
extern osMutexId SimcomRecMutexHandle;
extern osThreadId CommandTaskHandle;
extern osMailQId CommandMailHandle;
extern command_t *hCommand;

/* Private variable ---------------------------------------------------------*/
simcom_t simcom;

static void Simcom_Reset(void) {
  HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_SET);
  osDelay(500);
  HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_RESET);
  osDelay(5000);
}

static void Simcom_Prepare(void) {
  // prepare command sequence
  sprintf(simcom.CMD_CNMP, "AT+CNMP=%d\r", NET_SIGNAL);
  sprintf(simcom.CMD_CSTT,
      "AT+CSTT=\"%s\",\"%s\",\"%s\"\r",
      NET_APN, NET_APN_USERNAME, NET_APN_PASSWORD);
  sprintf(simcom.CMD_CIPSTART,
      "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r",
      NET_SERVER_IP, NET_SERVER_PORT);
}

static uint8_t Simcom_Boot(void) {
  // reset rx buffer
  SIMCOM_Reset_Buffer();
  // reset the state of simcom module
  Simcom_Reset();
  // wait until booting is done
  return Simcom_Command(SIMCOM_BOOT_COMMAND, NET_BOOT_TIMEOUT, SIMCOM_STATUS_OK, 1);
}

static void Simcom_Clear_Buffer(void) {
  // handle command (if any)
  if (Simcom_Read_Command(hCommand)) {
    // FIXME: sometime command is timeout
    osMailPut(CommandMailHandle, hCommand);
  }
  // debugging
  //	SWV_SendStrLn("\n=================== START ===================");
  //	SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
  //	SWV_SendStrLn("\n==================== END ====================");
  // reset rx buffer
  SIMCOM_Reset_Buffer();
}

static uint8_t Simcom_Response(char *str) {
  if (strstr(SIMCOM_UART_RX_Buffer, str) != NULL) {
    return 1;
  }
  return 0;
}

static uint8_t Simcom_Send_Direct(char *data, uint16_t data_length, uint32_t ms, char *res) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  uint8_t ret;
  uint32_t tick, timeout_tick = 0;

  Simcom_Clear_Buffer();
  // transmit to serial (low-level)
  SIMCOM_Transmit(data, data_length);
  // convert time to tick
  timeout_tick = pdMS_TO_TICKS(ms + NET_EXTRA_TIME_MS);
  // set timeout guard
  tick = osKernelSysTick();
  // wait response from SIMCOM
  while (1) {
    if (Simcom_Response(res) ||
        Simcom_Response(SIMCOM_STATUS_ERROR) ||
        Simcom_Response(SIMCOM_STATUS_READY) ||
        (osKernelSysTick() - tick) >= timeout_tick) {

      // set flag for timeout & error
      ret = Simcom_Response(res);

      // exception for auto reboot module
      if (strstr(data, SIMCOM_BOOT_COMMAND) != NULL) {
        ret = ret || Simcom_Response(SIMCOM_STATUS_READY);
      }

      // exit loop
      break;
    }

    osDelay(10);
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return ret;
}

static uint8_t Simcom_Send_Indirect(char *data, uint16_t data_length, uint8_t is_payload, uint32_t ms, char *res, uint8_t n) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  uint8_t ret = 0, seq = 0;
  // default response
  if (res == NULL) {
    res = SIMCOM_STATUS_OK;
  }

  // repeat command until desired response
  while (seq++ < n && !ret) {
    // execute command every timeout guard elapsed
    if (seq > 1) {
      osDelay(NET_REPEAT_DELAY * 1000);
    }

    // print command for debugger
    if (!is_payload) {
      SWV_SendStr("\n=> ");
      SWV_SendBuf(data, data_length);
    } else {
      SWV_SendBufHex(data, data_length);
    }
    SWV_SendChar('\n');

    // send command
    ret = Simcom_Send_Direct(data, data_length, ms, res);

    // print response for debugger
    SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
    SWV_SendChar('\n');
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return ret;
}

uint8_t Simcom_Command(char *cmd, uint32_t ms, char *res, uint8_t n) {
  return Simcom_Send_Indirect(cmd, strlen(cmd), 0, ms, res, n);
}

void Simcom_Init(void) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  uint8_t p;

  // FIXME: should use hierarchy algorithm in error handling
  // this do-while is complicated, but it doesn't use recursive function, so it's stack safe
  do {
    // show previous response
    //		SWV_SendStr("\n========================================\n");
    //		SWV_SendStr("Before: Simcom_Init()");
    //		SWV_SendStr("\n----------------------------------------\n");
    //		SWV_SendBuf(SIMCOM_UART_RX_Buffer, strlen(SIMCOM_UART_RX_Buffer));
    //		SWV_SendStr("\n========================================\n");

    SWV_SendStrLn("Simcom_Init");

    p = 0;
    //set default value to variable
    Simcom_Prepare();
    // booting
    p = Simcom_Boot();
    // Execute only on first setup
    if (p) {
      // disable command echo
      p = Simcom_Command("ATE0\r", 500, NULL, 1);
    }
    // =========== OTHERS CONFIGURATION
    // enable time reporting
    if (p) {
      p = Simcom_Command("AT+CLTS=1\r", 500, NULL, 1);
    }
    //Hide “+IPD” header
    if (p) {
      p = Simcom_Command("AT+CIPHEAD=1\r", 500, NULL, 1);
    }
    //Hide “RECV FROM” header
    if (p) {
      p = Simcom_Command("AT+CIPSRIP=0\r", 500, NULL, 1);
    }
    //Set to Single IP Connection (Backend)
    if (p) {
      p = Simcom_Command("AT+CIPMUX=0\r", 500, NULL, 1);
    }
    //Select TCPIP application mode (0: Non Transparent (command mode), 1: Transparent (data mode))
    if (p) {
      p = Simcom_Command("AT+CIPMODE=0\r", 500, NULL, 1);
    }
    // Get data from network automatically
    if (p) {
      p = Simcom_Command("AT+CIPRXGET=0\r", 500, NULL, 1);
    }

    // =========== NETWORK CONFIGURATION
    // Check SIM Card
    if (p) {
      p = Simcom_Command("AT+CPIN?\r", 500, NULL, 1);
    }
    // Disable presentation of <AcT>&<rac> at CREG and CGREG
    if (p) {
      p = Simcom_Command("AT+CSACT=0,0\r", 500, NULL, 1);
    }

    // Set signal Generation 2G/3G/AUTO
    if (p) {
      p = Simcom_Command(simcom.CMD_CNMP, 10000, NULL, 2);
    }

    // Network Registration Status
    if (p) {
      p = Simcom_Command("AT+CREG=2\r", 500, NULL, 1);
    }
    // Network GPRS Registration Status
    if (p) {
      p = Simcom_Command("AT+CGREG=2\r", 500, NULL, 1);
    }
    //Attach to GPRS service
    if (p) {
      p = Simcom_Command("AT+CGATT?\r", 10000, "+CGATT: 1", 3);
    }

    // =========== TCP/IP CONFIGURATION
    //Set type of authentication for PDP-IP connections of socket
    if (p) {
      p = Simcom_Command(simcom.CMD_CSTT, 500, NULL, 5);
    }
    // Bring Up Wireless Connection with GPRS
    if (p) {
      p = Simcom_Command("AT+CIICR\r", 5000, NULL, 5);
    }
    // Check IP Address
    if (p) {
      Simcom_Command("AT+CIFSR\r", 500, NULL, 1);
    }

    // Establish connection with server
    if (p) {
      p = Simcom_Command(simcom.CMD_CIPSTART, 10000, "CONNECT", 1);
      // check either connection ok / error
      if (p) {
        p = Simcom_Response("CONNECT OK");
      }
    }

    // restart module to fix it
    if (!p) {
      // disable all connection
      Simcom_Command("AT+CIPSHUT\r", 500, NULL, 1);
    }
  } while (p == 0);

  osRecursiveMutexRelease(SimcomRecMutexHandle);
}

uint8_t Simcom_Upload(char *payload, uint16_t payload_length) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  uint8_t ret = 0;
  uint32_t tick;
  char str[20];

  // combine the size
  sprintf(str, "AT+CIPSEND=%d\r", payload_length);

  // confirm to server that command is executed
  if (Simcom_Command(str, 5000, SIMCOM_STATUS_SEND, 1)) {
    // send response
    if (Simcom_Send_Indirect(payload, payload_length, 1, 5000, SIMCOM_STATUS_SENT, 1)) {
      // set timeout guard
      tick = osKernelSysTick();
      // wait ACK for payload
      while (1) {
        if (Simcom_Response(NET_ACK_PREFIX) ||
            (osKernelSysTick() - tick) >= tick5000ms) {

          break;
        }

        osDelay(10);
      }

      // if receive ACK, valid
      if (Simcom_Response(NET_ACK_PREFIX)) {
        ret = 1;
      }
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return ret;
}

uint8_t Simcom_Read_ACK(report_header_t *report_header) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  uint8_t ret = 0;
  ack_t ack;
  char *str = NULL;

  if (strstr(SIMCOM_UART_RX_Buffer, SIMCOM_RESPONSE_IPD)) {
    // parse ACK
    str = strstr(SIMCOM_UART_RX_Buffer, NET_ACK_PREFIX);
    if (str != NULL) {
      ack = *(ack_t*) str;

      // validate the value
      if (report_header->frame_id == ack.frame_id &&
          report_header->seq_id == ack.seq_id) {

        // clear rx buffer
        Simcom_Clear_Buffer();

        ret = 1;
      }
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return ret;
}

uint8_t Simcom_Read_Command(command_t *command) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  uint32_t crcValue;
  uint8_t ret = 0;
  char *str = NULL;

  if (strstr(SIMCOM_UART_RX_Buffer, SIMCOM_RESPONSE_IPD)) {
    // get pointer reference
    str = strstr(SIMCOM_UART_RX_Buffer, NET_COMMAND_PREFIX);
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
          // reset rx buffer
          SIMCOM_Reset_Buffer();

          ret = 1;
        }
      }
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return ret;
}

uint8_t Simcom_Read_Signal(uint8_t *signal_percentage) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  uint8_t i, ret = 0, rssi = 0, cnt;
  rssi_t rssiQuality;
  char *str, *prefix = "+CSQ: ";
  // see: http://wiki.teltonika-networks.com/view/Mobile_Signal_Strength_Recommendations
  const rssi_t rssiQualities[5] = {
      { .name = "Excellent", .minValue = -70, .linMinValue = 22, .percentage = 100 },
      { .name = "Good", .minValue = -85, .linMinValue = 14, .percentage = 75 },
      { .name = "Fair", .minValue = -100, .linMinValue = 7, .percentage = 50 },
      { .name = "Poor", .minValue = -110, .linMinValue = 2, .percentage = 25 },
      { .name = "No Signal", .minValue = -115, .linMinValue = 0, .percentage = 0 }
  };
  // check signal quality
  if (Simcom_Command("AT+CSQ\r", 500, NULL, 1)) {
    // get pointer reference
    str = strstr(SIMCOM_UART_RX_Buffer, prefix);

    if (str != NULL) {
      str += strlen(prefix);
      rssi = _ParseNumber(&str[0], &cnt);
      //			ber = _ParseNumber(&str[cnt + 1], NULL);

      // handle not detectable rssi value
      rssi = rssi == 99 ? 0 : rssi;

      // find the rssi quality
      for (i = 0; i < (sizeof(rssiQualities) / sizeof(rssiQualities[0])); i++) {
        if (rssi >= rssiQualities[i].linMinValue) {
          rssiQuality = rssiQualities[i];
          // set result
          *signal_percentage = rssiQuality.percentage;

          break;
        }
      }

      // debugging
      SWV_SendStr("\nSIGNAL RSSI = ");
      SWV_SendBuf(rssiQuality.name, strlen(rssiQuality.name));
      SWV_SendStr(", ");
      SWV_SendInt(*signal_percentage);
      SWV_SendStrLn("%");

      ret = 1;
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return ret;
}

uint8_t Simcom_Read_Carrier_Time(timestamp_t *timestamp) {
  osRecursiveMutexWait(SimcomRecMutexHandle, osWaitForever);

  uint8_t ret = 0, len = 0, cnt;
  char *str, *prefix = "+CCLK: \"";

  // get local timestamp (from base station)
  if (Simcom_Command("AT+CCLK?\r", 500, NULL, 1)) {
    // get pointer reference
    str = strstr(SIMCOM_UART_RX_Buffer, prefix);

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

      // make sure it is valid datetime
      if (RTC_Encode(*timestamp)) {
        // check is carrier timestamp valid
        if (timestamp->date.Year >= VCU_BUILD_YEAR) {
          // set weekday to default
          timestamp->date.WeekDay = RTC_WEEKDAY_MONDAY;

          ret = 1;
        }
      }
    }
  }

  osRecursiveMutexRelease(SimcomRecMutexHandle);
  return ret;
}
