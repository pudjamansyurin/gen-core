/*
 * _command.c
 *
 *  Created on: 28 Oct 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Libs/_rtos_utils.h"
#include "Libs/_command.h"
#include "Libs/_firmware.h"
#include "Libs/_eeprom.h"

/* External variables ---------------------------------------------------------*/
extern osThreadId_t AudioTaskHandle;
extern osThreadId_t FingerTaskHandle;
extern osThreadId_t RemoteTaskHandle;

/* Public functions implementation --------------------------------------------*/
void CMD_GenInfo(response_t *resp) {
  sprintf(resp->data.message,
      "VCU v%d.%d, "VCU_VENDOR" @ 20%d",
      _R8(VCU_VERSION, 8),
      _R8(VCU_VERSION, 0),
      VCU_BUILD_YEAR);
}

void CMD_GenLed(command_t *cmd) {
  GATE_LedWrite((uint8_t) cmd->data.value);
}

void CMD_GenOverride(command_t *cmd, uint8_t *override_state) {
  *override_state = (uint8_t) cmd->data.value;
}

void CMD_GenFota(IAP_TYPE type, response_t *resp, uint16_t *bat, uint16_t *hmi_version) {
  vehicle_state_t minState = VEHICLE_BACKUP, maxState = VEHICLE_READY;

  if (type == IAP_HMI)
    minState = VEHICLE_STANDBY;

  if (VCU.d.state.vehicle >= minState && VCU.d.state.vehicle <= maxState)
    FW_EnterModeIAP(type, resp->data.message, bat, hmi_version);
  else
    sprintf(resp->data.message, "Allowed vehicle state are (%d) - (%d).", minState, maxState);

  /* This line is never reached (if FOTA is activated) */
  resp->data.code = RESPONSE_STATUS_ERROR;
}

void CMD_ReportRTC(command_t *cmd, rtc_t *rtc) {
  RTC_Write((uint64_t) cmd->data.value, rtc);
}

void CMD_ReportOdom(command_t *cmd) {
  EEPROM_Odometer(EE_CMD_W, (uint32_t) cmd->data.value);
}

void CMD_ReportUnitID(command_t *cmd) {
  EEPROM_UnitID(EE_CMD_W, (uint32_t) cmd->data.value);
}


void CMD_AudioBeep(void) {
  osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_BEEP);
}

void CMD_AudioMute(command_t *cmd) {
  uint32_t flag;

  flag = ((uint8_t) cmd->data.value) ? EVT_AUDIO_MUTE_ON : EVT_AUDIO_MUTE_OFF;
  osThreadFlagsSet(AudioTaskHandle, flag);
}


void CMD_Finger(uint8_t event, response_t *resp) {
  uint32_t notif, timeout;

  osThreadFlagsSet(FingerTaskHandle, event);

  // decide the timeout
  timeout = (event == EVT_FINGER_ADD) ? 20000 : 5000;

  // wait response until timeout
  resp->data.code = RESPONSE_STATUS_ERROR;
  if (RTOS_ThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, timeout))
    if (notif & EVT_COMMAND_OK)
      resp->data.code = RESPONSE_STATUS_OK;
}

void CMD_RemotePairing(response_t *resp) {
  uint32_t notif;

  osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_PAIRING);

  // wait response until timeout
  resp->data.code = RESPONSE_STATUS_ERROR;
  if (RTOS_ThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, COMMAND_TIMEOUT))
    if (notif & EVT_COMMAND_OK)
      resp->data.code = RESPONSE_STATUS_OK;
}
