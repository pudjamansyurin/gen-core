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
#if (!BOOTLOADER)
#include "Libs/_reporter.h"
#include "Nodes/VCU.h"
#else
#include "Libs/_fota.h"
#include "Libs/_focan.h"
#include "Drivers/_flasher.h"
#endif

/* External variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern char SIMCOM_UART_RX[SIMCOM_UART_RX_SZ ];
#if (!BOOTLOADER)
extern osMutexId_t SimcomRecMutexHandle;
extern osMessageQueueId_t CommandQueueHandle;
extern vcu_t VCU;
#else
extern IWDG_HandleTypeDef hiwdg;
extern IAP_TYPE FOTA_TYPE;
#endif

/* Public variables ----------------------------------------------------------*/
sim_t SIM = {
    .state = SIM_STATE_DOWN,
    .ip_status = CIPSTAT_UNKNOWN,
    .signal = 0,
    .downloading = 0,
};

/* Private functions prototype -----------------------------------------------*/
static SIMCOM_RESULT Ready(void);
static SIMCOM_RESULT Power(void);
static SIMCOM_RESULT Execute(char *data, uint16_t size, uint32_t ms, char *res);
static void BeforeTransmitHook(void);
//static void SimcomDebugger(void);
static uint8_t StateTimeout(uint32_t *tick, uint32_t timeout, SIMCOM_RESULT p);
static uint8_t StateLockedLoop(SIMCOM_STATE *lastState, uint8_t *depthState);
static uint8_t StatePoorSignal(void);
#if (!BOOTLOADER)
static SIMCOM_RESULT ProcessCommando(command_t *command);
static uint8_t CommandoIRQ(void);
#endif

/* Public functions implementation --------------------------------------------*/
void Simcom_Lock(void) {
#if (!BOOTLOADER)
  osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
#endif
}

void Simcom_Unlock(void) {
#if (!BOOTLOADER)
  osMutexRelease(SimcomRecMutexHandle);
#endif
}

char* Simcom_Response(char *str) {
  return strstr(SIMCOM_UART_RX, str);
}

void Simcom_Init(void) {
  SIMCOM_DMA_Init();
  Simcom_SetState(SIM_STATE_READY, 0);
}

uint8_t Simcom_SetState(SIMCOM_STATE state, uint32_t timeout) {
  SIMCOM_STATE lastState = SIM_STATE_DOWN;
  uint32_t tick = _GetTickMS();
  uint8_t depthState = 3;
  SIMCOM_RESULT p;

  Simcom_Lock();
  // Handle SIMCOM state properly
  do {
    if (StateTimeout(&tick, timeout, p))
      break;
    if (StateLockedLoop(&lastState, &depthState))
      break;
    if (StatePoorSignal())
      break;

    // Set value
    p = SIM_RESULT_OK;

    // Handle states
    switch (SIM.state) {
      case SIM_STATE_DOWN:
        LOG_StrLn("Simcom:Init");

        // power up the module
        p = Power();

        // upgrade simcom state
        if (p > 0) {
          SIM.state++;
          LOG_StrLn("Simcom:ON");
        } else
          LOG_StrLn("Simcom:Error");

        _DelayMS(500);
        break;
      case SIM_STATE_READY:
        // =========== BASIC CONFIGURATION
        // disable command echo
        if (p > 0)
          p = AT_CommandEchoMode(0);

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
#if (!BOOTLOADER)
        // Enable time reporting
        if (p > 0) {
          AT_BOOL state = AT_ENABLE;
          p = AT_EnableLocalTimestamp(ATW, &state);
        }
        // Enable “+IPD” header
        if (p > 0) {
          AT_BOOL state = AT_ENABLE;
          p = AT_IpPackageHeader(ATW, &state);
        }
        // Disable “RECV FROM” header
        if (p > 0) {
          AT_BOOL state = AT_DISABLE;
          p = AT_ShowRemoteIp(ATW, &state);
        }
#endif
        // =========== NETWORK CONFIGURATION
        // Check SIM Card
        if (p > 0)
          p = Simcom_Command("AT+CPIN?\r", "READY", 500, 0);

        // Disable presentation of <AcT>&<rac> at CREG and CGREG
        if (p > 0) {
          at_csact_t param = {
              .creg = 0,
              .cgreg = 0,
          };
          p = AT_NetworkAttachedStatus(ATW, &param);
        }

        // upgrade simcom state
        if (p > 0)
          SIM.state++;

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
            if (p > 0)
              p = AT_NetworkRegistration("CREG", ATR, &read);

            // Handle timeout
            if (timeout && (_GetTickMS() - tick) > timeout) {
              LOG_StrLn("Simcom:StateTimeout");
              break;
            }
            _DelayMS(1000);
          } while (p && read.stat != param.stat);
        }

        // upgrade simcom state
        if (p > 0)
          SIM.state++;

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
            if (p > 0)
              p = AT_NetworkRegistration("CGREG", ATR, &read);

            // Handle timeout
            if (timeout && (_GetTickMS() - tick) > timeout) {
              LOG_StrLn("Simcom:StateTimeout");
              break;
            }
            _DelayMS(1000);
          } while (p && read.stat != param.stat);
        }

        // upgrade simcom state
        if (p > 0)
          SIM.state++;
        else
        if (SIM.state == SIM_STATE_NETWORK_ON)
          SIM.state--;

        _DelayMS(500);
        break;
#if (!BOOTLOADER)
      case SIM_STATE_GPRS_ON:
        // =========== PDP CONFIGURATION
        // Attach to GPRS service
        if (p > 0) {
          AT_CGATT state;
          // wait until attached
          do {
            p = AT_GprsAttachment(ATR, &state);

            // Handle timeout
            if (timeout && (_GetTickMS() - tick) > timeout) {
              LOG_StrLn("Simcom:StateTimeout");
              break;
            }
            _DelayMS(1000);
          } while (p && !state);
        }

        // upgrade simcom state
        if (p > 0)
          SIM.state++;
        else
        if (SIM.state == SIM_STATE_GPRS_ON)
          SIM.state--;

        _DelayMS(500);
        break;
      case SIM_STATE_PDP_ON:
        // =========== PDP ATTACH
        // Set type of authentication for PDP connections of socket
        AT_ConnectionStatusSingle(&(SIM.ip_status));
        if (p > 0 && (SIM.ip_status == CIPSTAT_IP_INITIAL || SIM.ip_status == CIPSTAT_PDP_DEACT)) {
          at_cstt_t param = {
              .apn = NET_CON_APN,
              .username = NET_CON_USERNAME,
              .password = NET_CON_PASSWORD,
          };
          p = AT_ConfigureAPN(ATW, &param);
        }
        // Select TCPIP application mode:
        // (0: Non Transparent (command mode), 1: Transparent (data mode))
        if (p > 0) {
          AT_CIPMODE state = CIPMODE_NORMAL;
          p = AT_TcpApllicationMode(ATW, &state);
        }
        // Set to Single IP Connection (Backend)
        if (p > 0) {
          AT_CIPMUX state = CIPMUX_SINGLE_IP;
          p = AT_MultiIpConnection(ATW, &state);
        }
        // Get data from network automatically
        if (p > 0) {
          AT_CIPRXGET state = CIPRXGET_DISABLE;
          p = AT_ManuallyReceiveData(ATW, &state);
        }

        // =========== IP ATTACH
        // Bring Up IP Connection
        AT_ConnectionStatusSingle(&(SIM.ip_status));
        if (p > 0 && SIM.ip_status == CIPSTAT_IP_START)
          p = Simcom_Command("AT+CIICR\r", NULL, 10000, 0);

        // Check IP Address
        AT_ConnectionStatusSingle(&(SIM.ip_status));
        if (p > 0 && (SIM.ip_status == CIPSTAT_IP_CONFIG || SIM.ip_status == CIPSTAT_IP_GPRSACT)) {
          at_cifsr_t param;
          p = AT_GetLocalIpAddress(&param);
        }

        // upgrade simcom state
        if (p > 0)
          SIM.state++;
        else {
          // Check IP Status
          AT_ConnectionStatusSingle(&(SIM.ip_status));

          // Close PDP
          if (SIM.ip_status != CIPSTAT_IP_INITIAL &&
              SIM.ip_status != CIPSTAT_PDP_DEACT)
            p = Simcom_Command("AT+CIPSHUT\r", NULL, 1000, 0);

          if (SIM.state == SIM_STATE_PDP_ON)
            SIM.state--;
        }

        _DelayMS(500);
        break;
      case SIM_STATE_INTERNET_ON:
        AT_ConnectionStatusSingle(&(SIM.ip_status));
        // ============ SOCKET CONFIGURATION
        // Establish connection with server
        if (p > 0 && (SIM.ip_status != CIPSTAT_CONNECT_OK || SIM.ip_status != CIPSTAT_CONNECTING)) {
          at_cipstart_t param = {
              .mode = "TCP",
              .ip = NET_TCP_SERVER,
              .port = NET_TCP_PORT
          };
          p = AT_StartConnectionSingle(&param);

          // wait until attached
          do {
            AT_ConnectionStatusSingle(&(SIM.ip_status));

            // Handle timeout
            if (timeout && (_GetTickMS() - tick) > timeout) {
              LOG_StrLn("Simcom:StateTimeout");
              break;
            }
            _DelayMS(1000);
          } while (SIM.ip_status == CIPSTAT_CONNECTING);
        }

        // upgrade simcom state
        if (p > 0)
          SIM.state++;
        else {
          // Check IP Status
          AT_ConnectionStatusSingle(&(SIM.ip_status));

          // Close IP
          if (SIM.ip_status == CIPSTAT_CONNECT_OK) {
            p = Simcom_Command("AT+CIPCLOSE\r", NULL, 1000, 0);

            // wait until closed
            do {
              AT_ConnectionStatusSingle(&(SIM.ip_status));

              // Handle timeout
              if (timeout && (_GetTickMS() - tick) > timeout) {
                LOG_StrLn("Simcom:StateTimeout");
                break;
              }
              _DelayMS(1000);
            } while (SIM.ip_status == CIPSTAT_CLOSING);
          }

          if (SIM.state == SIM_STATE_INTERNET_ON)
            SIM.state--;
        }

        _DelayMS(500);
        break;
      case SIM_STATE_SERVER_ON:
        // Check IP Status
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
  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  char str[20];

  // Check IP Status
  AT_ConnectionStatusSingle(&(SIM.ip_status));
  sprintf(str, "AT+CIPSEND=%d\r", size);

  Simcom_Lock();

  if (SIM.state >= SIM_STATE_SERVER_ON && SIM.ip_status == CIPSTAT_CONNECT_OK) {
    // send command
    p = Simcom_Command(str, SIMCOM_RSP_SEND, 500, 0);
    if (p > 0) {
      // send the payload
      p = Simcom_Command((char*) payload, SIMCOM_RSP_SENT, 20000, size);
      // wait for ACK/NACK
      if (p > 0) {
        p = SIM_RESULT_OK;
        if (Simcom_Response(PREFIX_COMMAND))
          p = SIM_RESULT_NACK;
      }
    }
  }

  Simcom_Unlock();
  return p;
}
#endif

SIMCOM_RESULT Simcom_Command(char *data, char *res, uint32_t ms, uint16_t size) {
  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  uint8_t upload = 1;

  // Handle default value
  if (res == NULL)
    res = SIMCOM_RSP_OK;

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
      } else
        LOG_BufHex(data, size);
      LOG_Enter();
    }

    // send command
    p = Execute(data, size, ms, res);

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

SIMCOM_RESULT Simcom_UpdateSignalQuality(void) {
  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  at_csq_t signal;

  // other routines
  p = AT_SignalQualityReport(&signal);
  if (p > 0)
    SIM.signal = signal.percent;

  return p;
}

SIMCOM_RESULT Simcom_IdleJob(uint8_t *iteration) {
  SIMCOM_RESULT p = SIM_RESULT_ERROR;

  // debug
  if (iteration != NULL) {
    LOG_Str("Simcom:Iteration = ");
    LOG_Int((*iteration)++);
    LOG_Enter();
  }

  // other routines
  p = Simcom_UpdateSignalQuality();

#if (!BOOTLOADER)
  p = AT_ConnectionStatusSingle(&(SIM.ip_status));
#endif
  return p;
}

/* Private functions implementation --------------------------------------------*/
#if (!BOOTLOADER)
static SIMCOM_RESULT ProcessCommando(command_t *command) {
  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  uint32_t crcValue;
  char *str = NULL;

  Simcom_Lock();
  if (Simcom_Response(SIMCOM_RSP_IPD)) {
    // get pointer reference
    str = Simcom_Response(PREFIX_ACK);
    if (str)
      str = strstr(str + sizeof(ack_t), PREFIX_COMMAND);
    else
      str = Simcom_Response(PREFIX_COMMAND);

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
        if (command->header.crc == crcValue)
          p = SIM_RESULT_OK;
      }
    }
  }
  Simcom_Unlock();

  return p;
}

static uint8_t CommandoIRQ(void) {
  return Simcom_Response(PREFIX_COMMAND) != NULL;
}
#endif

static SIMCOM_RESULT Ready(void) {
  uint32_t tick;

  // wait until 1s response
  tick = _GetTickMS();
  while (SIM.state == SIM_STATE_DOWN) {
    if (Simcom_Response(SIMCOM_RSP_READY)
        || Simcom_Response(SIMCOM_RSP_OK)
        || (_GetTickMS() - tick) >= NET_BOOT_TIMEOUT)
      break;
    _DelayMS(1000);

#if (BOOTLOADER)
    FOCAN_SetProgress(FOTA_TYPE, 0);
#endif
  }

  // check
  return Simcom_Command(SIMCOM_CMD_BOOT, SIMCOM_RSP_READY, 1000, 0);
}

static SIMCOM_RESULT Power(void) {
  LOG_StrLn("Simcom:Powered");
  SIMCOM_Reset_Buffer();

  // simcom reset pin
  GATE_SimcomSoftReset();

  if (Ready() == SIM_RESULT_OK) {
#if (!BOOTLOADER)
    VCU.SetEvent(EV_VCU_NET_SOFT_RESET, 1);
#endif
    return SIM_RESULT_OK;
  }

  // relay power control
  GATE_SimcomReset();

#if (!BOOTLOADER)
  VCU.SetEvent(EV_VCU_NET_HARD_RESET, 1);
#endif
  // wait response
  return Ready();
}

static SIMCOM_RESULT Execute(char *data, uint16_t size, uint32_t ms, char *res) {
  SIMCOM_RESULT p = SIM_RESULT_ERROR;
  uint32_t tick, timeout = 0;

  Simcom_Lock();
  // wake-up the SIMCOM
  GATE_SimcomSleep(GPIO_PIN_RESET);

  // transmit to serial (low-level)
  BeforeTransmitHook();
  SIMCOM_Transmit(data, size);

  // convert time to tick
  timeout = (ms + NET_EXTRA_TIME );
  // set timeout guard
  tick = _GetTickMS();

  // wait response from SIMCOM
  while (1) {
    if (Simcom_Response(res)
        || Simcom_Response(SIMCOM_RSP_ERROR)
        || Simcom_Response(SIMCOM_RSP_READY)
        #if (!BOOTLOADER)
        || CommandoIRQ()
        #endif
        || (_GetTickMS() - tick) >= timeout) {

      // check response
      if (Simcom_Response(res))
        p = SIM_RESULT_OK;

      // Handle failure
      if (p != SIM_RESULT_OK) {
        // exception for no response
        if (strlen(SIMCOM_UART_RX) == 0) {
          p = SIM_RESULT_NO_RESPONSE;
          SIM.state = SIM_STATE_DOWN;
          LOG_StrLn("Simcom:NoResponse");
        }
        #if (!BOOTLOADER)
        // Handle command from server
        else if (CommandoIRQ())
          p = SIM_RESULT_TIMEOUT;
#endif
        else {
          // exception for auto reboot module
          if (Simcom_Response(SIMCOM_RSP_READY) && (SIM.state >= SIM_STATE_READY)) {
            LOG_StrLn("Simcom:Restarted");
            p = SIM_RESULT_RESTARTED;
            SIM.state = SIM_STATE_READY;
#if (!BOOTLOADER)
            VCU.SetEvent(EV_VCU_NET_SOFT_RESET, 1);
#endif
          }
          // exception for timeout
          else if ((_GetTickMS() - tick) >= timeout) {
            LOG_StrLn("Simcom:Timeout");
            p = SIM_RESULT_TIMEOUT;
          }
        }
      }

      // exit loop
      break;
    }
#if (BOOTLOADER)
    // reset watchdog
    HAL_IWDG_Refresh(&hiwdg);
#endif
    _DelayMS(10);
  }

  // sleep the SIMCOM
  GATE_SimcomSleep(GPIO_PIN_SET);
  Simcom_Unlock();
  return p;
}

static void BeforeTransmitHook(void) {
#if (!BOOTLOADER)
  command_t hCommand;
  // handle Commando (if any)
  if (ProcessCommando(&hCommand))
    osMessageQueuePut(CommandQueueHandle, &hCommand, 0U, 0U);
#else
  if (SIM.downloading == 0)
    FOCAN_SetProgress(FOTA_TYPE, 0);
#endif

  // handle things on every request
  // SimcomDebugger();
}

//static void SimcomDebugger(void) {
//  LOG_StrLn("============ SIMCOM DEBUG ============");
//  LOG_Buf(SIMCOM_UART_RX, strlen(SIMCOM_UART_RX));
//  LOG_Enter();
//  LOG_StrLn("======================================");
//}

static uint8_t StateTimeout(uint32_t *tick, uint32_t timeout, SIMCOM_RESULT p) {
  // Handle timeout
  if (timeout) {
    // Update tick
    if (p == SIM_RESULT_OK)
      *tick = _GetTickMS();

    // Timeout expired
    if ((_GetTickMS() - *tick) > timeout) {
      LOG_StrLn("Simcom:StateTimeout");
      return 1;
    }
  }
  return 0;
}

static uint8_t StateLockedLoop(SIMCOM_STATE *lastState, uint8_t *depthState) {
  // Handle locked-loop
  if (SIM.state < *lastState) {
    *depthState -= 1;
    if (!(*depthState)) {
      SIM.state = SIM_STATE_DOWN;
      return 1;
    }
    LOG_Str("Simcom:LockedLoop = ");
    LOG_Int(*depthState);
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
