/*
 * simcom.c
 *
 *  Created on: Aug 14, 2019
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_simcom.h"

#include "Libs/_at.h"
#include "usart.h"

#if (APP)
#include "Libs/_mqtt.h"
#include "Nodes/VCU.h"

#else
#include "Drivers/_iwdg.h"
#endif

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMutexId_t SimcomRecMutexHandle;
#endif

/* Public variables
 * --------------------------------------------*/
sim_t SIM = {.d =
                 {
                     .state = SIM_STATE_DOWN,
                     .ipstatus = CIPSTAT_UNKNOWN,
                     .signal = 0,
                     .response = NULL,
                 },
             .puart = &huart1,
             .pdma = &hdma_usart1_rx};

/* Private functions prototype
 * --------------------------------------------*/
static SIMR Simcom_CmdRaw(char* data, uint16_t size, char* reply, uint32_t ms);
static SIMR TransmitCmd(char* data, uint16_t size, uint32_t ms, char* reply);
#if (APP)
static uint8_t Simcom_ProcessResponse(void);
#endif

/* Public functions implementation
 * --------------------------------------------*/
void Simcom_Lock(void) {
#if (APP)
  osMutexAcquire(SimcomRecMutexHandle, osWaitForever);
#endif
}

void Simcom_Unlock(void) {
#if (APP)
  osMutexRelease(SimcomRecMutexHandle);
#endif
}

void Simcom_Init(void) {
  MX_USART1_UART_Init();
  SIMCOM_DMA_Start(SIM.puart, SIM.pdma);
}

void Simcom_DeInit(void) {
  GATE_SimcomShutdown();
  SIMCOM_DMA_Stop();
  HAL_UART_DeInit(SIM.puart);
}

uint8_t Simcom_SetState(SIMCOM_STATE state, uint32_t timeout) {
  SIMCOM_STATE lastState = SIM_STATE_DOWN;
  SIMR res = SIM_ERROR;
  uint32_t tick = _GetTickMS();
  uint8_t retry = 0;

  Simcom_Lock();
  do {
    if (STAT_StateTimeout(&tick, timeout, res)) break;
    if (STAT_StateLockedLoop(&lastState, &retry)) break;
    if (STAT_StatePoorSignal()) break;

    // Set value
    res = SIM_OK;

    // Handle states
    switch (SIM.d.state) {
      case SIM_STATE_DOWN:
        STAT_Down(&res);
        break;

      case SIM_STATE_READY:
        STAT_Ready(&res);
        break;

      case SIM_STATE_CONFIGURED:
        STAT_Configured(&res, tick, timeout);
        break;

      case SIM_STATE_NETWORK_ON:
        STAT_NetworkOn(&res, tick, timeout);
        break;

      case SIM_STATE_GPRS_ON:
        STAT_GprsOn(&res, tick, timeout);
        break;

#if (!APP)
      case SIM_STATE_PDP_ON:
        STAT_PdpOn(&res);
        break;
      case SIM_STATE_BEARER_ON:
        /*nothing*/
        break;
#else

      case SIM_STATE_PDP_ON:
        STAT_PdpOn(&res);
        break;

      case SIM_STATE_INTERNET_ON:
        STAT_InternetOn(&res, tick, timeout);
        break;

      case SIM_STATE_SERVER_ON:
        STAT_ServerOn();
        break;

      case SIM_STATE_MQTT_ON:
        STAT_MqttOn();
        break;
#endif

      default:
        break;
    }
    _DelayMS(500);
  } while (SIM.d.state < state);
  Simcom_Unlock();

  return (SIM.d.state >= state);
}

SIMR Simcom_Cmd(char* command, char* reply, uint32_t ms) {
  return Simcom_CmdRaw(command, strlen(command), reply, ms);
}

char* Simcom_Resp(char* keyword, char* from) {
  char* start = &SIMCOM_UART_RX[0];
  char* stop = &SIMCOM_UART_RX[SIMCOM_UART_RX_SZ];

  if (from != NULL)
    if (from >= start && from <= stop) start = from;

#if (!APP)
  IWDG_Refresh();
#endif

  return strnstr(start, keyword, stop - start);
}

#if (APP)
uint8_t Simcom_FetchTime(timestamp_t* ts) {
  uint8_t ok = 0;

  if (Simcom_SetState(SIM_STATE_READY, 0))
    if (AT_Clock(ATR, ts) == SIM_OK) ok = (ts->date.Year >= VCU_BUILD_YEAR);

  return ok;
}

uint8_t Simcom_SendUSSD(char* ussd, char* buf, uint8_t buflen) {
  SIMR res;

  if (!Simcom_SetState(SIM_STATE_NETWORK_ON, 0)) return 0;

  // Set TE Character
  {
    char chset[5] = "GSM";
    res = AT_CharacterSetTE(ATW, chset, sizeof(chset));
  }
  // Delete all message
  {
    at_cmgd_t param = {.index = 0, .delflag = CMGD_ALL};
    res = AT_DeleteMessageSMS(&param);
  }
  // Dial USSD (check quota)
  if (res == SIM_OK) {
    at_cusd_t param = {.n = CUSD_ENABLE, .dcs = 15};
    strncpy(param.str, ussd, sizeof(param.str) - 1);
    res = AT_ServiceDataUSSD(ATW, &param, buf, buflen);
  }

  return res == SIM_OK;
}

uint8_t Simcom_ReadNewSMS(char* buf, uint8_t buflen) {
  SIMR res;

  if (!Simcom_SetState(SIM_STATE_NETWORK_ON, 0)) return 0;

  // Set SMS format
  {
    AT_CMGF param = CMGF_TEXT;
    res = AT_MessageFormatSMS(ATW, &param);
  }
  // Read SMS response
  if (res == SIM_OK) {
    at_cmgr_t par = {.index = 1, .mode = CMG_MODE_NORMAL};
    res = AT_ReadMessageSMS(&par, buf, buflen);
  }

  return res == SIM_OK;
}

uint8_t Simcom_Upload(void* payload, uint16_t size) {
  SIMR res;
  char cmd[20];

  if (SIM.d.ipstatus != CIPSTAT_CONNECT_OK && SIM.d.state < SIM_STATE_SERVER_ON)
    return 0;

  Simcom_Lock();
  sprintf(cmd, "AT+CIPSEND=%d\r", size);
  res = Simcom_Cmd(cmd, SIM_RSP_SEND, 500);

  if (res == SIM_OK)
    res = Simcom_CmdRaw((char*)payload, size, SIM_RSP_ACCEPT, 30000);
  Simcom_Unlock();

  return res == SIM_OK;
}

int Simcom_GetData(unsigned char* buf, int count) {
  if (SIM.d.response == NULL) return -1;
  if (SIM.d.response < &SIMCOM_UART_RX[0]) return -1;
  if ((SIM.d.response + count) > &SIMCOM_UART_RX[SIMCOM_UART_RX_SZ]) return -1;

  memcpy(buf, SIM.d.response, count);
  SIM.d.response += count;

  return count;
}

uint8_t Simcom_ReceivedResponse(uint32_t timeout) {
  TickType_t tick;
  char* ptr = NULL;

  tick = _GetTickMS();
  do {
    ptr = Simcom_Resp(SIM_RSP_IPD, NULL);
    _DelayMS(10);
  } while (ptr == NULL && _TickIn(tick, timeout));

  if (ptr != NULL)
    if ((ptr = Simcom_Resp(":", ptr)) != NULL) SIM.d.response = ptr + 1;

  return (ptr != NULL);
}
#endif

/* Private functions implementation
 * --------------------------------------------*/
static SIMR Simcom_CmdRaw(char* data, uint16_t size, char* reply, uint32_t ms) {
  SIMR res = SIM_ERROR;

  // only handle command if SIM_STATE_READY or BOOT_CMD
  if (!(SIM.d.state >= SIM_STATE_READY ||
        (strncmp(data, SIM_CMD_BOOT, size) == 0)))
    return res;

  // Debug: print payload
  if (SIMCOM_DEBUG) {
    if (strcmp(reply, SIM_RSP_ACCEPT) == 0)
      printf_hex(data, size);
    else {
      printf("\n=> ");
      printf("%.*s", size, data);
    }
    printf("\n");
  }

  // execute payload
  Simcom_Lock();
  GATE_SimcomSleep(0);
  res = TransmitCmd(data, size, ms, reply);
  GATE_SimcomSleep(1);
  Simcom_Unlock();

  // Debug: print response (ignore FTPGET command)
  if (SIMCOM_DEBUG)
    if (strnstr(data, SIM_CMD_FTPGET, size) == NULL)
      printf("%.*s\n", sizeof(SIMCOM_UART_RX), SIMCOM_UART_RX);

  return res;
}

static SIMR TransmitCmd(char* data, uint16_t size, uint32_t ms, char* reply) {
  SIMR res = SIM_ERROR;
  uint32_t tick;

#if (APP)
  if (Simcom_ReceivedResponse(0)) Simcom_ProcessResponse();
#endif
  SIMCOM_Transmit(data, size);

  // wait response from SIMCOM
  ms += NET_GUARD_MS;
  tick = _GetTickMS();
  while (1) {
    if (Simcom_Resp(reply, NULL) || Simcom_Resp(SIM_RSP_ERROR, NULL) ||
        Simcom_Resp(SIM_RSP_READY, NULL)
#if (APP)
        || Simcom_Resp(SIM_RSP_IPD, NULL)
#endif
        || (_GetTickMS() - tick) > ms) {

      // check response
      if (Simcom_Resp(reply, NULL)) res = SIM_OK;

      // Handle failure
      else {
        // exception for accidentally reboot
        if (Simcom_Resp(SIM_RSP_READY, NULL) &&
            (SIM.d.state >= SIM_STATE_READY)) {
          printf("Simcom:Restarted\n");
          res = SIM_RESTARTED;
          SIM.d.state = SIM_STATE_READY;
#if (APP)
          EVT_Set(EVG_NET_SOFT_RESET);
#endif
        }
#if (APP)
        // exception for server command collision
        else if (strcmp(reply, SIM_RSP_ACCEPT) != 0) {
          if (Simcom_ReceivedResponse(0)) {
            if (Simcom_ProcessResponse()) {
              printf("Simcom:CommandCollision\n");
              res = SIM_TIMEOUT;
            }
          }
        }
#endif
        // exception for no response
        else if (strnlen(SIMCOM_UART_RX, sizeof(SIMCOM_UART_RX)) == 0) {
          printf("Simcom:NoResponse\n");
          res = SIM_NORESPONSE;
          // SIM.d.state = SIM_STATE_DOWN;
        }

        // exception for timeout
        else if ((_GetTickMS() - tick) > ms) {
          printf("Simcom:Timeout\n");
          res = SIM_TIMEOUT;
        }

        else
          printf("Simcom:UnknownError\n");
      }

      // exit loop
      break;
    }

    _DelayMS(10);
  }

  return res;
}

#if (APP)
static uint8_t Simcom_ProcessResponse(void) {
  char* ptr = SIM.d.response;

  _DelayMS(1000);
  if (!MQTT_GotPublish()) {
    SIM.d.response = ptr;
    return 0;
  }
  return 1;
}
#endif
