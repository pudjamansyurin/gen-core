/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "DMA/_dma_simcom.h"
#include "Drivers/_crc.h"
#include "Drivers/_simcom.h"
#include "Libs/_at.h"
#if (!BOOTLOADER)
#include "Libs/_reporter.h"
#include "Nodes/VCU.h"
#else
#include "iwdg.h"
#include "Drivers/_flasher.h"
#include "Libs/_eeprom.h"
#include "Libs/_fota.h"
#include "Libs/_focan.h"
#endif

/* Public variables ---------------------------------------------------------*/
sim_t SIM = {
    .state = SIM_STATE_DOWN,
    .ip_status = CIPSTAT_UNKNOWN,
    .signal = 0,
    .downloading = 0,
};

#if (!BOOTLOADER)
extern osMessageQueueId_t CommandQueueHandle;
#endif

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT WaitUntilReady(void);
static SIMCOM_RESULT PowerUp(void);
static SIMCOM_RESULT SoftReset(void);
static SIMCOM_RESULT HardReset(void);

static SIMCOM_RESULT ExecCommand(char *data, uint16_t size, uint32_t ms, char *reply);
#if (!BOOTLOADER)
static void BeforeTransmitHook(void);
static SIMCOM_RESULT GotServerCommand(command_t *command);
#endif

static uint8_t StateTimeout(uint32_t *tick, uint32_t timeout, SIMCOM_RESULT p);
static uint8_t StateLockedLoop(SIMCOM_STATE *lastState, uint8_t *retry);
static uint8_t StatePoorSignal(void);

static void SetStateDown(SIMCOM_RESULT *res, SIMCOM_STATE *state);
static void SetStateReady(SIMCOM_RESULT *res, SIMCOM_STATE *state);
static void SetStateConfigured(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout);
static void SetStateNetworkOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout);
static void SetStateNetworkOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout);
#if (!BOOTLOADER)
static void SetStateGprsOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout);
static void SetStatePdpOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus);
static void SetStateInternetOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus, uint32_t tick, uint32_t timeout);
#endif

/* Public functions implementation --------------------------------------------*/
void Simcom_Lock(void) {
  //#if (!BOOTLOADER)
  //  osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
  //#endif
}

void Simcom_Unlock(void) {
  //#if (!BOOTLOADER)
  //  osMutexRelease(SimcomRecMutexHandle);
  //#endif
}

char* Simcom_Resp(char *ptr) {
  return strstr(SIMCOM_UART_RX, ptr);
}

void Simcom_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma) {
  SIM.h.uart = huart;
  SIM.h.dma = hdma;

  //  HAL_UART_Init(huart);
  MX_USART1_UART_Init();
  SIMCOM_DMA_Start(huart, hdma);
}

void Simcom_DeInit(void) {
  GATE_SimcomShutdown();
  SIMCOM_DMA_Stop();
  HAL_UART_DeInit(SIM.h.uart);
}

uint8_t Simcom_SetState(SIMCOM_STATE state, uint32_t timeout) {
  SIMCOM_STATE lastState = SIM_STATE_DOWN;
  SIMCOM_RESULT res = SIM_RESULT_ERROR;
  uint32_t tick = _GetTickMS();
  uint8_t retry = 3;

  Simcom_Lock();
  do {
    if (StateTimeout(&tick, timeout, res))
      break;
    if (StateLockedLoop(&lastState, &retry))
      break;
    if (StatePoorSignal())
      break;

    // Set value
    res = SIM_RESULT_OK;

    // Handle states
    switch (SIM.state) {
      case SIM_STATE_DOWN:
        SetStateDown(&res, &(SIM.state));
        _DelayMS(500);
        break;

      case SIM_STATE_READY:
        SetStateReady(&res, &(SIM.state));
        _DelayMS(500);
        break;

      case SIM_STATE_CONFIGURED:
        SetStateConfigured(&res, &(SIM.state), tick, timeout);
        _DelayMS(500);
        break;

      case SIM_STATE_NETWORK_ON:
        SetStateNetworkOn(&res, &(SIM.state), tick, timeout);
        _DelayMS(500);
        break;

#if (!BOOTLOADER)
      case SIM_STATE_GPRS_ON:
        SetStateGprsOn(&res, &(SIM.state), tick, timeout);
        _DelayMS(500);
        break;

      case SIM_STATE_PDP_ON:
        SetStatePdpOn(&res, &(SIM.state), &(SIM.ip_status));
        _DelayMS(500);
        break;

      case SIM_STATE_INTERNET_ON:
        AT_ConnectionStatusSingle(&(SIM.ip_status));
        SetStateInternetOn(&res, &(SIM.state), &(SIM.ip_status), tick, timeout);
        _DelayMS(500);
        break;

      case SIM_STATE_SERVER_ON:
        AT_ConnectionStatusSingle(&(SIM.ip_status));

        if (SIM.ip_status != CIPSTAT_CONNECT_OK)
          if (SIM.state == SIM_STATE_SERVER_ON)
            SIM.state--;
        break;
#endif

      default:
        break;
    }
  } while (SIM.state < state);
  Simcom_Unlock();

  return (SIM.state >= state);
}

#if (!BOOTLOADER)
SIMCOM_RESULT Simcom_Upload(void *payload, uint16_t size) {
  SIMCOM_RESULT res = SIM_RESULT_ERROR;
  char ptr[20];

  if (!(SIM.state >= SIM_STATE_SERVER_ON && SIM.ip_status == CIPSTAT_CONNECT_OK))
    return res;

  // Check IP Status
  AT_ConnectionStatusSingle(&(SIM.ip_status));

  Simcom_Lock();
  sprintf(ptr, "AT+CIPSEND=%d\r", size);
  res = Simcom_Cmd(ptr, SIMCOM_RSP_SEND, 500, 0);

  if (res > 0) {
    res = Simcom_Cmd((char*) payload, SIMCOM_RSP_SENT, 20000, size);

    if (res > 0) {
      res = SIM_RESULT_OK;

      // Handle crashed with command
      if (Simcom_Resp(PREFIX_COMMAND))
        res = SIM_RESULT_NACK;
    }
  } 
  Simcom_Unlock();

  return res;
}
#endif

SIMCOM_RESULT Simcom_Cmd(char *data, char *reply, uint32_t ms, uint16_t size) {
  SIMCOM_RESULT res = SIM_RESULT_ERROR;
  uint8_t upload = 1;

  // only handle command if SIM_STATE_READY or BOOT_CMD
  if (!(SIM.state >= SIM_STATE_READY || (strcmp(data, SIMCOM_CMD_BOOT) == 0)))
    return res;

  // Handle default value
  if (reply == NULL)
    reply = SIMCOM_RSP_OK;

  // Handle command (not upload)
  if (size == 0) {
    upload = 0;
    size = strlen(data);
  }

  // Debug: print payload
  if (SIMCOM_DEBUG) {
    if (!upload) {
      LOG_Str("\n=> ");
      LOG_Buf(data, size);
    } else
      LOG_BufHex(data, size);
    LOG_Enter();
  }

  // execute payload
  Simcom_Lock();
  GATE_SimcomSleep(0);
  res = ExecCommand(data, size, ms, reply);
  GATE_SimcomSleep(1);
  Simcom_Unlock();

  // Debug: print response (ignore FTPGET command)
  if (SIMCOM_DEBUG && strncmp(data, SIMCOM_CMD_FTPGET, strlen(SIMCOM_CMD_FTPGET)) != 0) {
    LOG_Buf(SIMCOM_UART_RX, sizeof(SIMCOM_UART_RX));
    LOG_Enter();
  }

  return res;
}

SIMCOM_RESULT Simcom_UpdateSignalQuality(void) {
  SIMCOM_RESULT res = SIM_RESULT_ERROR;
  at_csq_t signal;

  // other routines
  res = AT_SignalQualityReport(&signal);
  if (res > 0)
    SIM.signal = signal.percent;

  return res;
}

SIMCOM_RESULT Simcom_IdleJob(uint8_t *iteration) {
  SIMCOM_RESULT res = SIM_RESULT_ERROR;

  // debug
  if (iteration != NULL) {
    LOG_Str("Simcom:Iteration = ");
    LOG_Int((*iteration)++);
    LOG_Enter();
  }

  // other routines
  res = Simcom_UpdateSignalQuality();

#if (!BOOTLOADER)
  res = AT_ConnectionStatusSingle(&(SIM.ip_status));
#endif
  return res;
}

/* Private functions implementation --------------------------------------------*/
static SIMCOM_RESULT WaitUntilReady(void) {
  uint32_t tick = _GetTickMS();

  while (SIM.state == SIM_STATE_DOWN && (_GetTickMS() - tick) < NET_BOOT_TIMEOUT) {
    if (Simcom_Resp(SIMCOM_RSP_READY) || Simcom_Resp(SIMCOM_RSP_OK))
      break;

    _DelayMS(1000);
  }

  return Simcom_Cmd(SIMCOM_CMD_BOOT, SIMCOM_RSP_READY, 1000, 0);
}

static SIMCOM_RESULT PowerUp(void) {
  SIMCOM_RESULT res;

  res = SoftReset();
  if (res != SIM_RESULT_OK)
    return HardReset();

  return res;
}

static SIMCOM_RESULT SoftReset(void) {
  LOG_StrLn("Simcom:SoftReset");
  SIMCOM_Reset_Buffer();

  GATE_SimcomSoftReset();
#if (!BOOTLOADER)
  VCU.SetEvent(EV_VCU_NET_SOFT_RESET, 1);
#endif

  return WaitUntilReady();
}

static SIMCOM_RESULT HardReset(void) {
  LOG_StrLn("Simcom:HardReset");
  SIMCOM_Reset_Buffer();

  Simcom_Init(SIM.h.uart, SIM.h.dma);
  GATE_SimcomReset();
#if (!BOOTLOADER)
  VCU.SetEvent(EV_VCU_NET_HARD_RESET, 1);
#endif

  return WaitUntilReady();
}

static SIMCOM_RESULT ExecCommand(char *data, uint16_t size, uint32_t ms, char *reply) {
  SIMCOM_RESULT res = SIM_RESULT_ERROR;
  uint32_t tick, timeout = 0;

  // transmit to serial (low-level)
#if (!BOOTLOADER)
  BeforeTransmitHook();
#endif
  SIMCOM_Transmit(data, size);

  // set timeout guard
  timeout = (ms + NET_EXTRA_TIME );
  tick = _GetTickMS();

  // wait response from SIMCOM
  while (1) {
    if (Simcom_Resp(reply)
        || Simcom_Resp(SIMCOM_RSP_ERROR)
        || Simcom_Resp(SIMCOM_RSP_READY)
#if (!BOOTLOADER)
        || Simcom_Resp(PREFIX_COMMAND)
#endif
        || (_GetTickMS() - tick) > timeout
    ) {

      // check response
      if (Simcom_Resp(reply))
        res = SIM_RESULT_OK;

      // Handle failure
      else {
        // exception for no response
        if (strlen(SIMCOM_UART_RX) == 0) {
          res = SIM_RESULT_NO_RESPONSE;
          SIM.state = SIM_STATE_DOWN;
          LOG_StrLn("Simcom:NoResponse");
        }

        // exception for accidentally reboot
        else if (Simcom_Resp(SIMCOM_RSP_READY) && (SIM.state >= SIM_STATE_READY)) {
          LOG_StrLn("Simcom:Restarted");
          res = SIM_RESULT_RESTARTED;
          SIM.state = SIM_STATE_READY;
#if (!BOOTLOADER)
          VCU.SetEvent(EV_VCU_NET_SOFT_RESET, 1);
#endif
        }

#if (!BOOTLOADER)
        // exception for suddenly got command from server
        else if (Simcom_Resp(PREFIX_COMMAND))
          res = SIM_RESULT_TIMEOUT;
#endif

        // exception for timeout
        else if ((_GetTickMS() - tick) > timeout) {
          LOG_StrLn("Simcom:Timeout");
          res = SIM_RESULT_TIMEOUT;
        }
      }

      // exit loop
      break;
    }

#if (BOOTLOADER)
    HAL_IWDG_Refresh(&hiwdg);
#endif
    _DelayMS(10);
  }

  return res;
}

#if (!BOOTLOADER)
static SIMCOM_RESULT GotServerCommand(command_t *command) {
  SIMCOM_RESULT res = SIM_RESULT_ERROR;
  uint32_t crcValue;
  char *ptr = NULL;

  Simcom_Lock();
  if (Simcom_Resp(SIMCOM_RSP_IPD)) {
    // get pointer reference
    //    ptr = Simcom_Resp(PREFIX_ACK);
    //    if (ptr)
    //      ptr = strstr(ptr + sizeof(ack_t), PREFIX_COMMAND);
    //    else
    ptr = Simcom_Resp(PREFIX_COMMAND);

    if (ptr != NULL) {
      // copy the whole value (any time the buffer can change)
      *command = *(command_t*) ptr;

      // check
      if (command->header.size == sizeof(command->data)) {
        crcValue = CRC_Calculate8(
            (uint8_t*) &(command->header.size),
            sizeof(command->header.size) + sizeof(command->data),
            0);

        if (command->header.crc == crcValue)
          res = SIM_RESULT_OK;
      }
    }
  }
  Simcom_Unlock();

  return res;
}

static void BeforeTransmitHook(void) {
  command_t hCommand;

  if (GotServerCommand(&hCommand))
    osMessageQueuePut(CommandQueueHandle, &hCommand, 0U, 0U);

  // handle things on every request
  //  LOG_StrLn("============ SIMCOM DEBUG ============");
  //  LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
  //  LOG_Enter();
  //  LOG_StrLn("======================================");
}
#endif

static uint8_t StateTimeout(uint32_t *tick, uint32_t timeout, SIMCOM_RESULT res) {
  // Handle timeout
  if (timeout) {
    // Update tick
    if (res == SIM_RESULT_OK)
      *tick = _GetTickMS();

    // Timeout expired
    if ((_GetTickMS() - *tick) > timeout) {
      LOG_StrLn("Simcom:StateTimeout");
      return 1;
    }
  }
  return 0;
}

static uint8_t StateLockedLoop(SIMCOM_STATE *lastState, uint8_t *retry) {
  // Handle locked-loop
  if (SIM.state < *lastState) {
    *retry -= 1;
    if (!(*retry)) {
      SIM.state = SIM_STATE_DOWN;
      return 1;
    }
    LOG_Str("Simcom:LockedLoop = ");
    LOG_Int(*retry);
    LOG_Enter();
  }
  *lastState = SIM.state;
  return 0;
}

static uint8_t StatePoorSignal(void) {
  // Handle signal strength
  if (SIM.state == SIM_STATE_DOWN)
    SIM.signal = 0;
  else {
    Simcom_IdleJob(NULL);
    if (SIM.state >= SIM_STATE_GPRS_ON) {
      if (SIM.signal < 15) {
        LOG_StrLn("Simcom:PoorSignal");
        return 1;
      }
    }
  }
  return 0;
}

static void SetStateDown(SIMCOM_RESULT *res, SIMCOM_STATE *state) {
  LOG_StrLn("Simcom:Init");

  // power up the module
  *res = PowerUp();

  // upgrade simcom state
  if (*res > 0) {
    (*state)++;
    LOG_StrLn("Simcom:ON");
  } else
    LOG_StrLn("Simcom:Error");
}

static void SetStateReady(SIMCOM_RESULT *res, SIMCOM_STATE *state) {
  // =========== BASIC CONFIGURATION
  // disable command echo
  if (*res > 0)
    *res = AT_CommandEchoMode(0);
  // Set serial baud-rate
  if (*res > 0) {
    uint32_t rate = 0;
    *res = AT_FixedLocalRate(ATW, &rate);
  }
  // Error report format: 0, 1(Numeric), 2(verbose)
  if (*res > 0) {
    AT_CMEE param = CMEE_VERBOSE;
    *res = AT_ReportMobileEquipmentError(ATW, &param);
  }
  // Use pin DTR as sleep control
  if (*res > 0) {
    AT_CSCLK param = CSCLK_EN_DTR;
    *res = AT_ConfigureSlowClock(ATW, &param);
  }
#if (!BOOTLOADER)
  // Enable time reporting
  if (*res > 0) {
    AT_BOOL param = AT_ENABLE;
    *res = AT_EnableLocalTimestamp(ATW, &param);
  }
  // Enable “+IPD” header
  if (*res > 0) {
    AT_BOOL param = AT_ENABLE;
    *res = AT_IpPackageHeader(ATW, &param);
  }
  // Disable “RECV FROM” header
  if (*res > 0) {
    AT_BOOL param = AT_DISABLE;
    *res = AT_ShowRemoteIp(ATW, &param);
  }
#endif
  // =========== NETWORK CONFIGURATION
  // Check SIM Card
  if (*res > 0)
    *res = Simcom_Cmd("AT+CPIN?\r", "READY", 500, 0);

  // Disable presentation of <AcT>&<rac> at CREG and CGREG
  if (*res > 0) {
    at_csact_t param = {
        .creg = 0,
        .cgreg = 0,
    };
    *res = AT_NetworkAttachedStatus(ATW, &param);
  }

  // upgrade simcom state
  if (*res > 0)
    (*state)++;
}

static void SetStateConfigured(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout) {
  // =========== NETWORK ATTACH
  // Set signal Generation 2G(13)/3G(14)/AUTO(2)
  if (*res > 0) {
    at_cnmp_t param = {
        .mode = CNMP_ACT_AUTO,
        .preferred = CNMP_ACT_P_UMTS
    };
    *res = AT_RadioAccessTechnology(ATW, &param);
  }
  // Network Registration Status
  if (*res > 0) {
    at_c_greg_t read, param = {
        .mode = CREG_MODE_DISABLE,
        .stat = CREG_STAT_REG_HOME
    };
    // wait until attached
    do {
      *res = AT_NetworkRegistration("CREG", ATW, &param);
      if (*res > 0)
        *res = AT_NetworkRegistration("CREG", ATR, &read);

      // Handle timeout
      if (timeout && (_GetTickMS() - tick) > timeout) {
        LOG_StrLn("Simcom:StateTimeout");
        break;
      }
      _DelayMS(1000);
    } while (*res && read.stat != param.stat);
  }

  // upgrade simcom state
  if (*res > 0)
    (*state)++;
}

static void SetStateNetworkOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout) {
  // =========== GPRS ATTACH
  // GPRS Registration Status
  if (*res > 0) {
    at_c_greg_t read, param = {
        .mode = CREG_MODE_DISABLE,
        .stat = CREG_STAT_REG_HOME
    };
    // wait until attached
    do {
      *res = AT_NetworkRegistration("CGREG", ATW, &param);
      if (*res > 0)
        *res = AT_NetworkRegistration("CGREG", ATR, &read);

      // Handle timeout
      if (timeout && (_GetTickMS() - tick) > timeout) {
        LOG_StrLn("Simcom:StateTimeout");
        break;
      }
      _DelayMS(1000);
    } while (*res && read.stat != param.stat);
  }

  // upgrade simcom state
  if (*res > 0)
    (*state)++;
  else
    if (*state == SIM_STATE_NETWORK_ON)
      (*state)--;
}

#if (!BOOTLOADER)
static void SetStateGprsOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, uint32_t tick, uint32_t timeout) {
  // =========== PDP CONFIGURATION
  // Attach to GPRS service
  if (*res > 0) {
    AT_CGATT param;
    // wait until attached
    do {
      *res = AT_GprsAttachment(ATR, &param);

      // Handle timeout
      if (timeout && (_GetTickMS() - tick) > timeout) {
        LOG_StrLn("Simcom:StateTimeout");
        break;
      }
      _DelayMS(1000);
    } while (*res && !param);
  }

  // upgrade simcom state
  if (*res > 0)
    (*state)++;
  else
    if (*state == SIM_STATE_GPRS_ON)
      (*state)--;
}

static void SetStatePdpOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus) {
  // =========== PDP ATTACH
  // Set type of authentication for PDP connections of socket
  AT_ConnectionStatusSingle(ipStatus);
  if (*res > 0 && (*ipStatus == CIPSTAT_IP_INITIAL || *ipStatus == CIPSTAT_PDP_DEACT)) {
    at_cstt_t param = {
        .apn = NET_CON_APN,
        .username = NET_CON_USERNAME,
        .password = NET_CON_PASSWORD,
    };
    *res = AT_ConfigureAPN(ATW, &param);
  }
  // Select TCPIP application mode:
  // (0: Non Transparent (command mode), 1: Transparent (data mode))
  if (*res > 0) {
    AT_CIPMODE param = CIPMODE_NORMAL;
    *res = AT_TcpApllicationMode(ATW, &param);
  }
  // Set to Single IP Connection (Backend)
  if (*res > 0) {
    AT_CIPMUX param = CIPMUX_SINGLE_IP;
    *res = AT_MultiIpConnection(ATW, &param);
  }
  // Get data from network automatically
  if (*res > 0) {
    AT_CIPRXGET param = CIPRXGET_DISABLE;
    *res = AT_ManuallyReceiveData(ATW, &param);
  }

  // =========== IP ATTACH
  // Bring Up IP Connection
  AT_ConnectionStatusSingle(ipStatus);
  if (*res > 0 && *ipStatus == CIPSTAT_IP_START)
    *res = Simcom_Cmd("AT+CIICR\r", NULL, 10000, 0);

  // Check IP Address
  AT_ConnectionStatusSingle(ipStatus);
  if (*res > 0 && (*ipStatus == CIPSTAT_IP_CONFIG || *ipStatus == CIPSTAT_IP_GPRSACT)) {
    at_cifsr_t param;
    *res = AT_GetLocalIpAddress(&param);
  }

  // upgrade simcom state
  if (*res > 0)
    (*state)++;
  else {
    // Check IP Status
    AT_ConnectionStatusSingle(ipStatus);

    // Close PDP
    if (*ipStatus != CIPSTAT_IP_INITIAL &&
        *ipStatus != CIPSTAT_PDP_DEACT)
      *res = Simcom_Cmd("AT+CIPSHUT\r", NULL, 1000, 0);

    if ((*state) == SIM_STATE_PDP_ON)
      (*state)--;
  }
}

static void SetStateInternetOn(SIMCOM_RESULT *res, SIMCOM_STATE *state, AT_CIPSTATUS *ipStatus, uint32_t tick, uint32_t timeout) {
  // ============ SOCKET CONFIGURATION
  // Establish connection with server
  if (*res > 0 && (*ipStatus != CIPSTAT_CONNECT_OK || *ipStatus != CIPSTAT_CONNECTING)) {
    at_cipstart_t param = {
        .mode = "TCP",
        .ip = NET_TCP_SERVER,
        .port = NET_TCP_PORT
    };
    *res = AT_StartConnectionSingle(&param);

    // wait until attached
    do {
      AT_ConnectionStatusSingle(ipStatus);

      // Handle timeout
      if (timeout && (_GetTickMS() - tick) > timeout) {
        LOG_StrLn("Simcom:StateTimeout");
        break;
      }
      _DelayMS(1000);
    } while (*ipStatus == CIPSTAT_CONNECTING);
  }

  // upgrade simcom state
  if (*res > 0)
    (*state)++;
  else {
    // Check IP Status
    AT_ConnectionStatusSingle(ipStatus);

    // Close IP
    if (*ipStatus == CIPSTAT_CONNECT_OK) {
      *res = Simcom_Cmd("AT+CIPCLOSE\r", NULL, 1000, 0);

      // wait until closed
      do {
        AT_ConnectionStatusSingle(ipStatus);

        // Handle timeout
        if (timeout && (_GetTickMS() - tick) > timeout) {
          LOG_StrLn("Simcom:StateTimeout");
          break;
        }
        _DelayMS(1000);
      } while (*ipStatus == CIPSTAT_CLOSING);
    }

    if (*state == SIM_STATE_INTERNET_ON)
      (*state)--;
  }
}
#endif

