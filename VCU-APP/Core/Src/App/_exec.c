/*
 * _request.c
 *
 *  Created on: Jun 14, 2021
 *      Author: pudja
 */

/* Includes
 * --------------------------------------------*/
#include "App/_exec.h"

#include "App/_firmware.h"
#include "App/_reporter.h"
#include "App/_task.h"
#include "Libs/_eeprom.h"
#include "Libs/_finger.h"
#include "Nodes/HMI1.h"
#include "Nodes/VCU.h"

/* External variables
 * --------------------------------------------*/
extern osMessageQueueId_t OvdStateQueueHandle, DriverQueueHandle,
    UssdQueueHandle, QuotaQueueHandle;

/* Private functions prototype
 * --------------------------------------------*/
static void EXEC_GenInfo(response_data_t *resp);
static void EXEC_FingerAdd(response_data_t *rdata, osMessageQueueId_t queue);
static void EXEC_FingerFetch(response_data_t *rdata);
static void EXEC_Finger(response_data_t *rdata);
static void EXEC_RemotePairing(response_data_t *rdata);
static void EXEC_NetQuota(response_data_t *rdata, osMessageQueueId_t queue);

/* Public functions implementation
 * --------------------------------------------*/
void EXEC_Command(command_t *cmd, response_t *resp) {
  uint8_t code = cmd->header.code;
  uint8_t subCode = cmd->header.sub_code;
  void *val = cmd->data.value;

  response_data_t *rdata = &(resp->data);
  uint8_t *resCode = &(rdata->res_code);
  char *resMsg = rdata->message;

  // default command response
  resp->header.code = code;
  resp->header.sub_code = subCode;
  resp->data.res_code = RESP_OK;
  strcpy(resp->data.message, "");

  if (code == CMD_CODE_GEN) {
    switch (subCode) {
      case CMD_GEN_INFO:
        EXEC_GenInfo(rdata);
        break;

      case CMD_GEN_LED:
        GATE_LedWrite(*(uint8_t *)val);
        break;

      case CMD_GEN_RTC:
        RTC_Write(*(datetime_t *)val);
        break;

      case CMD_GEN_ODOM:
        EE_TripMeter(EE_CMD_W, HBAR_M_TRIP_ODO, *(uint16_t *)val);
        break;

      case CMD_GEN_ANTITHIEF:
        osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_DETECTOR_TOGGLE);
        break;

      case CMD_GEN_RPT_FLUSH:
        osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_FLUSH);
        break;

      case CMD_GEN_RPT_BLOCK:
        RPT.block = *(uint8_t *)val;
        break;

      default:
        *resCode = RESP_INVALID;
        break;
    }
  }

  else if (code == CMD_CODE_OVD) {
    switch (subCode) {
      case CMD_OVD_STATE:
        if (VCU.d.state < VEHICLE_NORMAL) {
          sprintf(resMsg, "State should >= {%d}.", VEHICLE_NORMAL);
          *resCode = RESP_ERROR;
        } else
          _osQueuePutRst(OvdStateQueueHandle, (uint8_t *)val);
        break;

      case CMD_OVD_RPT_INTERVAL:
        RPT.override.interval = *(uint16_t *)val;
        break;

      case CMD_OVD_RPT_FRAME:
        RPT.override.frame = *(uint8_t *)val;
        break;

      case CMD_OVD_RMT_SEAT:
        osThreadFlagsSet(GateTaskHandle, FLAG_GATE_OPEN_SEAT);
        break;

      case CMD_OVD_RMT_ALARM:
        osThreadFlagsSet(GateTaskHandle, FLAG_GATE_ALARM_HORN);
        break;

      default:
        *resCode = RESP_INVALID;
        break;
    }
  }

  else if (code == CMD_CODE_AUDIO) {
    if (VCU.d.state < VEHICLE_NORMAL) {
      sprintf(resMsg, "State should >= {%d}.", VEHICLE_NORMAL);
      *resCode = RESP_ERROR;
    } else
      switch (subCode) {
        case CMD_AUDIO_BEEP:
          osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_BEEP);
          break;

        default:
          *resCode = RESP_INVALID;
          break;
      }
  }

  else if (code == CMD_CODE_FGR) {
    if (VCU.d.state != VEHICLE_STANDBY) {
      sprintf(resMsg, "State should = {%d}.", VEHICLE_STANDBY);
      *resCode = RESP_ERROR;
    } else
      switch (subCode) {
        case CMD_FGR_FETCH:
          osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_FETCH);
          EXEC_FingerFetch(rdata);
          break;

        case CMD_FGR_ADD:
          osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_ADD);
          EXEC_FingerAdd(rdata, DriverQueueHandle);
          break;

        case CMD_FGR_DEL:
          _osQueuePutRst(DriverQueueHandle, (uint8_t *)val);
          osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_DEL);
          EXEC_Finger(rdata);
          break;

        case CMD_FGR_RST:
          osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_RST);
          EXEC_Finger(rdata);
          break;

        default:
          *resCode = RESP_INVALID;
          break;
      }
  }

  else if (code == CMD_CODE_RMT) {
    if (VCU.d.state < VEHICLE_NORMAL) {
      sprintf(resMsg, "State should >= {%d}.", VEHICLE_NORMAL);
      *resCode = RESP_ERROR;
    } else
      switch (subCode) {
        case CMD_RMT_PAIRING:
          osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_PAIRING);
          EXEC_RemotePairing(rdata);
          break;

        default:
          *resCode = RESP_INVALID;
          break;
      }
  }

  else if (code == CMD_CODE_FOTA) {
    *resCode = RESP_ERROR;

    if (VCU.d.state == VEHICLE_RUN) {
      sprintf(resMsg, "State should != {%d}.", VEHICLE_RUN);
    } else
      switch (subCode) {
        case CMD_FOTA_VCU:
          FW_EnterModeIAP(IAP_VCU, resMsg);
          break;

        case CMD_FOTA_HMI:
          FW_EnterModeIAP(IAP_HMI, resMsg);
          break;

        default:
          *resCode = RESP_INVALID;
          break;
      }
  }

  else if (code == CMD_CODE_NET) {
    *resCode = RESP_ERROR;

    switch (subCode) {
      case CMD_NET_SEND_USSD:
        _osQueuePutRst(UssdQueueHandle, val);
        osThreadFlagsSet(NetworkTaskHandle, FLAG_NET_SEND_USSD);
        EXEC_NetQuota(rdata, QuotaQueueHandle);
        break;

      case CMD_NET_READ_SMS:
        osThreadFlagsSet(NetworkTaskHandle, FLAG_NET_READ_SMS);
        EXEC_NetQuota(rdata, QuotaQueueHandle);
        break;

      default:
        *resCode = RESP_INVALID;
        break;
    }
  }

  else if (code == CMD_CODE_HBAR) {
    uint8_t v = *(uint8_t *)val;
    switch (subCode) {
      case CMD_HBAR_DRIVE:
        HBAR.d.mode[HBAR_M_DRIVE] = v;
        break;

      case CMD_HBAR_TRIP:
        HBAR.d.mode[HBAR_M_TRIP] = v;
        break;

      case CMD_HBAR_REPORT:
        HBAR.d.mode[HBAR_M_REPORT] = v;
        break;

      case CMD_HBAR_REVERSE:
        HBAR.d.pin[HBAR_K_REVERSE] = v;
        break;

      default:
        *resCode = RESP_INVALID;
        break;
    }
  }

  else if (code == CMD_CODE_MCU) {
    if (!MCU.d.active || MCU.d.run) {
      sprintf(resMsg, "MCU not ready!");
      *resCode = RESP_ERROR;
    } else
      switch (subCode) {
        case CMD_MCU_SPEED_MAX:
          MCU_SetSpeedMax(*(uint8_t *)val);
          break;

        case CMD_MCU_TEMPLATES:
          MCU_SetTemplates(*(mcu_templates_t *)val);
          break;

        default:
          *resCode = RESP_INVALID;
          break;
      }
  }

  else
    *resCode = RESP_INVALID;
}

/* Private functions implementation
 * --------------------------------------------*/
static void EXEC_GenInfo(response_data_t *rdata) {
  char msg[20];
  sprintf(msg, "VCU v.%d,", VCU_VERSION);

  if (HMI1.d.active)
    sprintf(rdata->message, "%.*s HMI v.%d,", strlen(msg), msg, HMI1.d.version);

  sprintf(rdata->message, "%.*s " VCU_VENDOR " - 20%d", strlen(msg), msg,
          VCU_BUILD_YEAR);
}

static void EXEC_FingerAdd(response_data_t *rdata, osMessageQueueId_t queue) {
  uint32_t notif;
  uint8_t id;

  // wait response until timeout
  rdata->res_code = RESP_ERROR;
  if (_osFlagAny(&notif, (FINGER_SCAN_MS * 2) + 5000)) {
    if (notif & FLAG_COMMAND_OK) {
      if (_osQueueGet(queue, &id)) {
        sprintf(rdata->message, "%u", id);
        rdata->res_code = RESP_OK;
      }
    } else {
      if (_osQueueGet(queue, &id))
        if (id == 0)
          sprintf(rdata->message, "Max. reached : %u", FINGER_USER_MAX);
    }
  }
}

static void EXEC_FingerFetch(response_data_t *rdata) {
  uint32_t notif, len;
  char fingers[3];

  // wait response until timeout
  memset(FGR.d.db, 0, FINGER_USER_MAX);
  rdata->res_code = RESP_ERROR;
  if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000)) {
    rdata->res_code = RESP_OK;

    for (uint8_t id = 1; id <= FINGER_USER_MAX; id++) {
      if (FGR.d.db[id - 1]) {
        sprintf(fingers, "%1d,", id);
        strcat(rdata->message, fingers);
      }
    }

    // remove last comma
    len = strnlen(rdata->message, sizeof(rdata->message));
    if (len > 0) rdata->message[len - 1] = '\0';
  }
}

static void EXEC_Finger(response_data_t *rdata) {
  uint32_t notif;

  // wait response until timeout
  rdata->res_code = RESP_ERROR;
  if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000)) rdata->res_code = RESP_OK;
}

static void EXEC_RemotePairing(response_data_t *rdata) {
  uint32_t notif;

  // wait response until timeout
  rdata->res_code = RESP_ERROR;
  if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000)) rdata->res_code = RESP_OK;
}

static void EXEC_NetQuota(response_data_t *rdata, osMessageQueueId_t queue) {
  uint32_t notif;

  // wait response until timeout
  rdata->res_code = RESP_ERROR;
  if (_osFlagOne(&notif, FLAG_COMMAND_OK, 40000))
    if (_osQueueGet(queue, rdata->message)) rdata->res_code = RESP_OK;
}
