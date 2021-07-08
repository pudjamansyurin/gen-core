/*
 * _request.c
 *
 *  Created on: Jun 14, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_exec.h"

#include "App/_firmware.h"
#include "App/_reporter.h"
#include "App/_task.h"
#include "Drivers/_simcom.h"
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
static uint8_t IsReadRequest(const command_t *cmd);
static void EXEC_GenInfo(response_data_t *resp);
static void EXEC_FingerAdd(response_data_t *rdata);
static void EXEC_FingerFetch(response_data_t *rdata);
static void EXEC_Finger(response_data_t *rdata);
static void EXEC_RemotePairing(response_data_t *rdata);
static void EXEC_NetQuota(response_data_t *rdata);
static void EXEC_ConApn(const command_t *cmd, char *rMsg);

/* Public functions implementation
 * --------------------------------------------*/
void EXEC_Command(const command_t *cmd, response_t *resp) {
  uint8_t code = cmd->header.code;
  uint8_t subCode = cmd->header.sub_code;
  const void *val = cmd->data.value;

  response_data_t *rdata = &(resp->data);
  uint8_t *rCode = &(rdata->res_code);
  char *rMsg = rdata->message;

  // default command response
  resp->header.code = code;
  resp->header.sub_code = subCode;
  resp->data.res_code = CMDR_OK;
  strcpy(resp->data.message, "");

  if (code == CMDC_GEN) {
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
        HB_EE_Trip(HBMS_TRIP_ODO, (uint16_t *)val);
        break;

      case CMD_GEN_ANTITHIEF:
        osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_DETECTOR_TOGGLE);
        break;

      case CMD_GEN_RPT_FLUSH:
        osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_FLUSH);
        break;

      case CMD_GEN_RPT_BLOCK:
        RPT_IO_SetBlock(*(uint8_t *)val);
        break;

      default:
        *rCode = CMDR_INVALID;
        break;
    }
  }

  else if (code == CMDC_OVD) {
    switch (subCode) {
      case CMD_OVD_STATE:
        if (VCU.d.vehicle < VEHICLE_NORMAL) {
          sprintf(rMsg, "State should >= {%d}.", VEHICLE_NORMAL);
          *rCode = CMDR_ERROR;
        } else
          _osQueuePutRst(OvdStateQueueHandle, (uint8_t *)val);
        break;

      case CMD_OVD_RPT_INTERVAL:
        RPT_IO_SetOvdInterval(*(uint16_t *)val);
        break;

      case CMD_OVD_RPT_FRAME:
        RPT_IO_SetOvdFrame(*(uint8_t *)val);
        break;

      case CMD_OVD_RMT_SEAT:
        osThreadFlagsSet(GateTaskHandle, FLAG_GATE_OPEN_SEAT);
        break;

      case CMD_OVD_RMT_ALARM:
        osThreadFlagsSet(GateTaskHandle, FLAG_GATE_ALARM_HORN);
        break;

      default:
        *rCode = CMDR_INVALID;
        break;
    }
  }

  else if (code == CMDC_AUDIO) {
    if (VCU.d.vehicle < VEHICLE_NORMAL) {
      sprintf(rMsg, "State should >= {%d}.", VEHICLE_NORMAL);
      *rCode = CMDR_ERROR;
    } else
      switch (subCode) {
        case CMD_AUDIO_BEEP:
          osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_BEEP);
          break;

        default:
          *rCode = CMDR_INVALID;
          break;
      }
  }

  else if (code == CMDC_FGR) {
    if (VCU.d.vehicle != VEHICLE_STANDBY) {
      sprintf(rMsg, "State should = {%d}.", VEHICLE_STANDBY);
      *rCode = CMDR_ERROR;
    } else
      switch (subCode) {
        case CMD_FGR_FETCH:
          osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_FETCH);
          EXEC_FingerFetch(rdata);
          break;

        case CMD_FGR_ADD:
          osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_ADD);
          EXEC_FingerAdd(rdata);
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
          *rCode = CMDR_INVALID;
          break;
      }
  }

  else if (code == CMDC_RMT) {
    if (VCU.d.vehicle < VEHICLE_NORMAL) {
      sprintf(rMsg, "State should >= {%d}.", VEHICLE_NORMAL);
      *rCode = CMDR_ERROR;
    } else
      switch (subCode) {
        case CMD_RMT_PAIRING:
          osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_PAIRING);
          EXEC_RemotePairing(rdata);
          break;

        default:
          *rCode = CMDR_INVALID;
          break;
      }
  }

  else if (code == CMDC_FOTA) {
    *rCode = CMDR_ERROR;

    if (VCU.d.vehicle == VEHICLE_RUN) {
      sprintf(rMsg, "State should != {%d}.", VEHICLE_RUN);
    } else
      switch (subCode) {
        case CMD_FOTA_VCU:
          FW_EnterModeIAP(ITYPE_VCU);
          break;

        case CMD_FOTA_HMI:
          FW_EnterModeIAP(ITYPE_HMI);
          break;

        default:
          *rCode = CMDR_INVALID;
          break;
      }
  }

  else if (code == CMDC_NET) {
    *rCode = CMDR_ERROR;

    switch (subCode) {
      case CMD_NET_SEND_USSD:
        _osQueuePutRst(UssdQueueHandle, val);
        osThreadFlagsSet(NetworkTaskHandle, FLAG_NET_SEND_USSD);
        EXEC_NetQuota(rdata);
        break;

      case CMD_NET_READ_SMS:
        osThreadFlagsSet(NetworkTaskHandle, FLAG_NET_READ_SMS);
        EXEC_NetQuota(rdata);
        break;

      default:
        *rCode = CMDR_INVALID;
        break;
    }
  }

  else if (code == CMDC_CON) {
    *rCode = CMDR_ERROR;

    switch (subCode) {
      case CMD_CON_APN:
        EXEC_ConApn(cmd, rMsg);
        break;
      case CMD_CON_FTP:
      case CMD_CON_MQTT:
        memcpy(rdata->message, val, sizeof(rdata->message));
        break;

      default:
        *rCode = CMDR_INVALID;
        break;
    }
  }

  else if (code == CMDC_HBAR) {
    uint8_t v = *(uint8_t *)val;
    switch (subCode) {
      case CMD_HBAR_DRIVE:
        HB_IO_SetSub(HBM_DRIVE, v);
        break;

      case CMD_HBAR_TRIP:
        HB_IO_SetSub(HBM_TRIP, v);
        break;

      case CMD_HBAR_AVG:
        HB_IO_SetSub(HBM_AVG, v);
        break;

      case CMD_HBAR_REVERSE:
        HB_IO_SetPin(HBP_REVERSE, v);
        break;

      default:
        *rCode = CMDR_INVALID;
        break;
    }
  }

  else if (code == CMDC_MCU) {
    if (!MCU.d.active || MCU.d.run) {
      sprintf(rMsg, "MCU not ready!");
      *rCode = CMDR_ERROR;
    } else
      switch (subCode) {
        case CMD_MCU_SPEED_MAX:
          MCU_SetSpeedMax(*(uint8_t *)val);
          break;

        case CMD_MCU_TEMPLATES:
          MCU_SetTemplates(*(mcu_templates_t *)val);
          break;

        default:
          *rCode = CMDR_INVALID;
          break;
      }
  }

  else
    *rCode = CMDR_INVALID;
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t IsReadRequest(const command_t *cmd) {
  if (CMD_GetPayloadSize(cmd) > 1) return 0;
  return cmd->data.value[0] == '?';
}

static void EXEC_GenInfo(response_data_t *rdata) {
  char msg[20];
  sprintf(msg, "VCU v.%d,", VCU_VERSION);

  if (HMI1.d.active)
    sprintf(rdata->message, "%.*s HMI v.%d,", strlen(msg), msg, HMI1.d.version);

  sprintf(rdata->message, "%.*s " VCU_VENDOR " - 20%d", strlen(msg), msg,
          VCU_BUILD_YEAR);
}

static void EXEC_FingerAdd(response_data_t *rdata) {
  uint32_t notif;
  uint8_t id;

  rdata->res_code = CMDR_ERROR;
  if (_osFlagAny(&notif, (FINGER_SCAN_MS * 2) + 5000)) {
    if (notif & FLAG_COMMAND_OK) {
      if (_osQueueGet(DriverQueueHandle, &id)) {
        sprintf(rdata->message, "%u", id);
        rdata->res_code = CMDR_OK;
      }
    } else {
      if (_osQueueGet(DriverQueueHandle, &id))
        if (id == 0)
          sprintf(rdata->message, "Max. reached : %u", FINGER_USER_MAX);
    }
  }
}

static void EXEC_FingerFetch(response_data_t *rdata) {
  uint32_t notif, len;
  char fingers[3];

  FGR_IO_ClearDB();
  rdata->res_code = CMDR_ERROR;
  if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000)) {
    rdata->res_code = CMDR_OK;

    for (uint8_t id = 1; id <= FINGER_USER_MAX; id++) {
      if (FGR_IO_GetDB(id - 1)) {
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

  rdata->res_code = CMDR_ERROR;
  if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000)) rdata->res_code = CMDR_OK;
}

static void EXEC_RemotePairing(response_data_t *rdata) {
  uint32_t notif;

  rdata->res_code = CMDR_ERROR;
  if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000)) rdata->res_code = CMDR_OK;
}

static void EXEC_NetQuota(response_data_t *rdata) {
  uint32_t notif;

  rdata->res_code = CMDR_ERROR;
  if (_osFlagOne(&notif, FLAG_COMMAND_OK, 40000))
    if (_osQueueGet(QuotaQueueHandle, rdata->message))
      rdata->res_code = CMDR_OK;
}

static void EXEC_ConApn(const command_t *cmd, char *rMsg) {
  con_apn_t apn;

  if (IsReadRequest(cmd))
    apn = SIM.con.apn;
  else {
    char *token, *rest;

    if ((token = strtok_r(rMsg, ";", &rest)))
      strncpy(apn.name, token, sizeof(apn.name));
    if ((token = strtok_r(NULL, ";", &rest)))
      strncpy(apn.user, token, sizeof(apn.user));
    if ((token = strtok_r(NULL, ";", &rest)))
      strncpy(apn.pass, token, sizeof(apn.pass));
  }

  sprintf(rMsg, "%s,%s,%s", apn.name, apn.user, apn.pass);
}
