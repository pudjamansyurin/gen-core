/*
 * _sim_state.c
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_sim_state.h"

#include "Drivers/_simcom.h"
#include "Libs/_at.h"

#if APP
#include "Libs/_mqtt.h"
#include "Nodes/VCU.h"
#else
#include "Drivers/_iwdg.h"
#endif

/* Private constants
 * --------------------------------------------*/
#define SIM_BOOT_MS ((uint16_t)8000)

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint8_t signal;
  SIM_STATE state;
  SIM_IP ip;
} sim_status_t;

/* Private variables
 * --------------------------------------------*/
static sim_status_t STA = {
		.signal = 0,
		.state = SIM_STATE_DOWN,
		.ip = SIM_IP_UNKNOWN,
};

/* Private functions prototype
 * --------------------------------------------*/
static SIMR PowerUp(void);
static SIMR Reset(uint8_t hard);
static void IdleJob(void);
static void NetworkRegistration(char* type, SIMR* res, uint32_t tick,
                                uint32_t timeout);
static uint8_t TimeoutReached(uint32_t tick, uint32_t timeout, uint32_t delay);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t SIMSta_StateTimeout(SIMR res, uint32_t* tick, uint32_t timeout) {
  if (timeout) {
    if (res == SIM_OK) *tick = _GetTickMS();

    if (TimeoutReached(*tick, timeout, 0)) return 1;
  }
  return 0;
}

uint8_t SIMSta_StateLockedLoop(SIM_STATE* lastState, uint8_t* retry) {
  // Handle locked-loop
  if (STA.state < *lastState) {
    if (*retry == 0) {
      if (STA.state > SIM_STATE_DOWN) STA.state--;
      return 1;
    }
    (*retry)--;
    printf("Simcom:LockedLoop = %u\n", *retry);
  }
  *lastState = STA.state;
  return 0;
}

uint8_t SIMSta_StatePoorSignal(void) {
  // Handle signal strength
  if (STA.state == SIM_STATE_DOWN)
    STA.signal = 0;
  else {
    IdleJob();
    if (STA.state >= SIM_STATE_NETWORK_ON) {
      if (STA.signal < 15) {
        printf("Simcom:PoorSignal\n");
        return 1;
      }
    }
  }
  return 0;
}

void SIMSta_Down(SIMR* res) {
  uint8_t ok;

  printf("Simcom:Init\n");
  // power up the module
  ok = PowerUp() == SIM_OK;

  // upgrade simcom state
  if (ok) STA.state = SIM_STATE_READY;

  *res = ok;
  printf("Simcom:%s\n", ok ? "OK" : "Error");
}

void SIMSta_Ready(SIMR* res) {
  /* BASIC CONFIGURATION */
  // disable command echo
  if (*res == SIM_OK) *res = AT_CommandEchoMode(0);
  // Set serial baud-rate
  if (*res == SIM_OK) {
    uint32_t rate = 0;
    *res = AT_FixedLocalRate(ATW, &rate);
  }
  // Error report format: 0, 1(Numeric), 2(verbose)
  if (*res == SIM_OK) {
    AT_CMEE param = CMEE_VERBOSE;
    *res = AT_ReportMobileEquipmentError(ATW, &param);
  }
  // Use pin DTR as sleep control
  if (*res == SIM_OK) {
    AT_CSCLK param = CSCLK_EN_DTR;
    *res = AT_ConfigureSlowClock(ATW, &param);
  }
#if (APP)
  // Enable time reporting
  if (*res == SIM_OK) {
    AT_BOOL param = AT_ENABLE;
    *res = AT_EnableLocalTimestamp(ATW, &param);
  }
  // Enable “+IPD” header
  if (*res == SIM_OK) {
    AT_BOOL param = AT_ENABLE;
    *res = AT_IpPackageHeader(ATW, &param);
  }
  // Disable “RECV FROM” header
  if (*res == SIM_OK) {
    AT_BOOL param = AT_DISABLE;
    *res = AT_ShowRemoteIp(ATW, &param);
  }
#endif
  /* NETWORK CONFIGURATION */
  // Check SIM Card
  if (*res == SIM_OK) *res = SIM_Cmd("AT+CPIN?\r", "READY", 500);

  // Disable presentation of <AcT>&<rac> at CREG and CGREG
  if (*res == SIM_OK) {
    at_csact_t param = {
        .creg = 0,
        .cgreg = 0,
    };
    *res = AT_NetworkAttachedStatus(ATW, &param);
  }

  // upgrade simcom state
  if (*res == SIM_OK)
    STA.state = SIM_STATE_CONFIGURED;
  else if (STA.state == SIM_STATE_READY)
    STA.state = SIM_STATE_DOWN;
}

void SIMSta_Configured(SIMR* res, uint32_t tick, uint32_t timeout) {
  /*  NETWORK ATTACH */
  // Set signal Generation 2G(13)/3G(14)/AUTO(2)
  if (*res == SIM_OK) {
    at_cnmp_t param = {.mode = CNMP_ACT_AUTO, .preferred = CNMP_ACT_P_UMTS};
    *res = AT_RadioAccessTechnology(ATW, &param);
  }

  // Network Registration Status
  if (*res == SIM_OK) NetworkRegistration("CREG", res, tick, timeout);

  // upgrade simcom state
  if (*res == SIM_OK)
    STA.state = SIM_STATE_NETWORK_ON;
  else if (STA.state == SIM_STATE_CONFIGURED) {
    STA.state = SIM_STATE_DOWN;  // -2 state
    Reset(1);
  }
}

void SIMSta_NetworkOn(SIMR* res, uint32_t tick, uint32_t timeout) {
  /* GPRS ATTACH */
  // GPRS Registration Status
  if (*res == SIM_OK) NetworkRegistration("CGREG", res, tick, timeout);

  // upgrade simcom state
  if (*res == SIM_OK)
    STA.state = SIM_STATE_GPRS_ON;
  else if (STA.state == SIM_STATE_NETWORK_ON) {
    //		STA.state = SIM_STATE_CONFIGURED;
    STA.state = SIM_STATE_DOWN;  // -3 state
    Reset(1);
  }
}

void SIMSta_GprsOn(SIMR* res, uint32_t tick, uint32_t timeout) {
  // Attach to GPRS service
  if (*res == SIM_OK) {
    AT_CGATT param;
    // wait until attached
    do {
      *res = AT_GprsAttachment(ATR, &param);

      if (TimeoutReached(tick, timeout, 1000)) break;

    } while (*res && !param);
  }

  // upgrade simcom state
  if (*res == SIM_OK) {
    AT_ConnectionStatus(&(STA.ip));
    if (STA.ip == SIM_IP_PDP_DEACT)
      STA.state = SIM_STATE_DOWN;
    else
      STA.state = SIM_STATE_PDP_ON;
  } else if (STA.state == SIM_STATE_GPRS_ON)
    STA.state = SIM_STATE_NETWORK_ON;
}

#if (!APP)
void SIMSta_PdpOn(SIMR* res) {
  /* FTP CONFIGURATION */
  // Initiate bearer for TCP based applications.
  *res = AT_BearerInitialize();

  // upgrade simcom state
  if (*res == SIM_OK)
    STA.state = SIM_STATE_BEARER_ON;
  else if (STA.state == SIM_STATE_PDP_ON)
    STA.state = SIM_STATE_GPRS_ON;
}

#else
void SIMSta_PdpOn(SIMR* res) {
  /* PDP ATTACH */
  // Set type of authentication for PDP connections of socket
  if (*res == SIM_OK) {
    *res = AT_ConfigureAPN(ATW, (con_apn_t*) SIMCon_IO_Apn());
  }
  // Select TCPIP application mode:
  // (0: Non Transparent (command mode), 1: Transparent (data mode))
  if (*res == SIM_OK) {
    AT_CIPMODE param = CIPMODE_NORMAL;
    *res = AT_TcpApllicationMode(ATW, &param);
  }
  // Set to Single IP Connection (Backend)
  if (*res == SIM_OK) {
    AT_CIPMUX param = CIPMUX_SINGLE_IP;
    *res = AT_MultiIpConnection(ATW, &param);
  }
  // Get data from network automatically
  if (*res == SIM_OK) {
    AT_CIPRXGET param = CIPRXGET_DISABLE;
    *res = AT_ManuallyReceiveData(ATW, &param);
  }
  // Set data transmit mode
  if (*res == SIM_OK) {
    AT_CIPQSEND param = CIPQSEND_QUICK;
    *res = AT_DataTransmitMode(ATW, &param);
  }

  /* IP ATTACH */
  // Check PDP status
  if (*res == SIM_OK) {
    AT_ConnectionStatus(&(STA.ip));
    if (STA.ip == SIM_IP_INITIAL ||
        STA.ip == SIM_IP_PDP_DEACT)
      *res = SIM_ERROR;
  }
  // Bring Up IP Connection
  if (*res == SIM_OK) {
    AT_ConnectionStatus(&(STA.ip));
    if (STA.ip == SIM_IP_START)
      *res = SIM_Cmd("AT+CIICR\r", SIM_RSP_OK, 15000);
  }

  // Check IP Address
  if (*res == SIM_OK) {
    AT_ConnectionStatus(&(STA.ip));
    if (STA.ip == SIM_IP_CONFIG ||
        STA.ip == SIM_IP_GPRSACT) {
      at_cifsr_t param;
      *res = AT_GetLocalIpAddress(&param);
    }
  }

  // upgrade simcom state
  if (*res == SIM_OK)
    STA.state = SIM_STATE_INTERNET_ON;
  else {
    AT_ConnectionStatus(&(STA.ip));
    if (STA.ip != SIM_IP_INITIAL) {
      SIM_Cmd("AT+CIPCLOSE\r", SIM_RSP_OK, 5000);
      if (STA.ip != SIM_IP_PDP_DEACT)
        SIM_Cmd("AT+CIPSHUT\r", SIM_RSP_OK, 1000);
    }

    if (STA.state == SIM_STATE_PDP_ON) STA.state = SIM_STATE_GPRS_ON;
  }
}

void SIMSta_InternetOn(SIMR* res, uint32_t tick, uint32_t timeout) {
  /* SOCKET CONFIGURATION */
  // Establish connection with server
  AT_ConnectionStatus(&(STA.ip));
  if (*res == SIM_OK && (STA.ip != SIM_IP_CONNECT_OK ||
                         STA.ip != SIM_IP_CONNECTING)) {
    *res = AT_StartConnection(SIMCon_IO_Mqtt());

    // wait until attached
    do {
      AT_ConnectionStatus(&(STA.ip));

      if (TimeoutReached(tick, timeout, 1000)) break;

    } while (STA.ip == SIM_IP_CONNECTING);
  }

  // upgrade simcom state
  if (*res == SIM_OK)
    STA.state = SIM_STATE_SERVER_ON;
  else {
    // Check IP Status
    AT_ConnectionStatus(&(STA.ip));

    // Close IP
    if (STA.ip == SIM_IP_CONNECT_OK) {
      *res = SIM_Cmd("AT+CIPCLOSE\r", SIM_RSP_OK, 1000);

      // wait until closed
      do {
        AT_ConnectionStatus(&(STA.ip));

        if (TimeoutReached(tick, timeout, 1000)) break;

      } while (STA.ip == SIM_IP_CLOSING);
    }

    if (STA.state == SIM_STATE_INTERNET_ON) STA.state = SIM_STATE_PDP_ON;
  }
}

void SIMSta_ServerOn(void) {
  uint8_t ok = 0;

  AT_ConnectionStatus(&(STA.ip));
  if (STA.ip == SIM_IP_CONNECT_OK)
    if (MQTT_Connect())
      if (MQTT_PublishWill(1)) ok = MQTT_Subscribe();

  // upgrade simcom state
  if (ok)
    STA.state = SIM_STATE_MQTT_ON;
  else if (STA.state == SIM_STATE_SERVER_ON) {
    // MQTT_Disconnect();
    STA.state = SIM_STATE_INTERNET_ON;
  }
}

void SIMSta_MqttOn(void) {
  //	if (!MQTT_Willed())
  //		MQTT_PublishWill(1);
  //	if (!MQTT_Subscribed())
  //		MQTT_Subscribe();

  AT_ConnectionStatus(&(STA.ip));
  if (STA.ip != SIM_IP_CONNECT_OK || !MQTT_Ping())
    if (STA.state == SIM_STATE_MQTT_ON) STA.state = SIM_STATE_SERVER_ON;
}
#endif

SIM_IP SIMSta_IO_Ip(void) {
	return STA.ip;
}

SIM_STATE SIMSta_IO_State(void) {
	return STA.state;
}

uint8_t SIMSta_IO_Signal(void) {
	return STA.signal;
}

void SIMSta_IO_SetState(SIM_STATE value) {
	STA.state = value;
}
/* Private functions implementation
 * --------------------------------------------*/
static SIMR PowerUp(void) {
  SIMR res;

  res = Reset(0);
  if (res != SIM_OK)
    if (SIM_BatSufficient()) return Reset(1);

  return res;
}

static SIMR Reset(uint8_t hard) {
  uint32_t tick;

  SIM_Reset_Buffer();
  printf("Simcom:%s\n", hard ? "HardReset" : "SoftReset");

  if (hard)
    GATE_SimcomReset();
  else
    GATE_SimcomSoftReset();

#if (APP)
  EVT_Set(hard ? EVG_NET_HARD_RESET : EVG_NET_SOFT_RESET);
#endif

  // Wait until ready
  tick = _GetTickMS();
  do {
    if (SIM_Resp(SIM_RSP_READY, NULL) || SIM_Resp(SIM_RSP_OK, NULL)) break;

#if (!APP)
    IWDG_Refresh();
#endif

    _DelayMS(100);
  } while (STA.state == SIM_STATE_DOWN && _TickIn(tick, SIM_BOOT_MS));

  return SIM_Cmd(SIM_CMD_BOOT, SIM_RSP_READY, 1000);
}

static void IdleJob(void) {
  at_csq_t signal;
  if (AT_SignalQualityReport(&signal) == SIM_OK) STA.signal = signal.percent;

#if (APP)
  AT_ConnectionStatus(&(STA.ip));
#endif
}

static void NetworkRegistration(char* type, SIMR* res, uint32_t tick,
                                uint32_t timeout) {
  at_c_greg_t read,
      param = {.mode = CREG_MODE_DISABLE, .stat = CREG_STAT_REG_HOME};

  // wait until attached
  do {
    *res = AT_NetworkRegistration(type, ATW, &param);
    if (*res == SIM_OK) *res = AT_NetworkRegistration(type, ATR, &read);

    if (TimeoutReached(tick, timeout, 1000)) break;

  } while (*res && read.stat != param.stat);
}

static uint8_t TimeoutReached(uint32_t tick, uint32_t timeout, uint32_t delay) {
  if (_TickOut(tick, timeout)) {
    if (timeout) printf("Simcom:StateTimeout\n");
    return 1;
  }

  _DelayMS(delay);
  return 0;
}
