/*
 * _command.c
 *
 *  Created on: 28 Oct 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_rtos_utils.h"
#include "Libs/_command.h"
#include "Libs/_firmware.h"
#include "Libs/_eeprom.h"
#include "Drivers/_rtc.h"
#include "Nodes/VCU.h"

/* External variables ---------------------------------------------------------*/
extern vcu_t VCU;
extern osThreadId_t AudioTaskHandle;
extern osThreadId_t FingerTaskHandle;
extern osThreadId_t KeylessTaskHandle;

/* Public functions implementation --------------------------------------------*/
void CMD_GenInfo(response_t *resp) {
	sprintf(resp->data.message,
			"VCU v%d.%d, "VCU_VENDOR" @ 20%d",
			_R8(VCU_VERSION, 8),
			_R8(VCU_VERSION, 0),
			VCU_BUILD_YEAR);
}

void CMD_GenLed(command_t *cmd) {
	_LedWrite((uint8_t) cmd->data.value);
}

void CMD_GenOverride(command_t *cmd) {
  VCU.d.state.override = (uint8_t) cmd->data.value;
}

void CMD_GenFota(command_t *cmd, response_t *resp) {
	IAP_TYPE type = IAP_HMI;

	/* Enter IAP mode */
  if (cmd->data.sub_code == CMD_GEN_FOTA_VCU)
		type = IAP_VCU;

	FW_EnterModeIAP(type, resp->data.message);

	/* This line is never reached (if FOTA is activated) */
	resp->data.code = RESPONSE_STATUS_ERROR;
}

void CMD_ReportRTC(command_t *cmd) {
	RTC_Write((uint64_t) cmd->data.value, &(VCU.d.rtc));
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

	flag = ((uint8_t) cmd->data.value) ?  EVT_AUDIO_MUTE_ON : EVT_AUDIO_MUTE_OFF;
	osThreadFlagsSet(AudioTaskHandle, flag);
}

void CMD_AudioVol(command_t *cmd) {
	VCU.d.volume = (uint8_t) cmd->data.value;
}

void CMD_Finger(uint8_t event, response_t *resp) {
	uint32_t notif, timeout;

	osThreadFlagsSet(FingerTaskHandle, event);

	// decide the timeout
	timeout = (event == EVT_FINGER_ADD) ? 20000 : 5000;

	// wait response until timeout
	resp->data.code = RESPONSE_STATUS_ERROR;
  if (_RTOS_ThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, timeout))
    if (notif & EVT_COMMAND_OK)
			resp->data.code = RESPONSE_STATUS_OK;
}

void CMD_KeylessPairing(response_t *resp) {
	uint32_t notif;

	osThreadFlagsSet(KeylessTaskHandle, EVT_KEYLESS_PAIRING);

	// wait response until timeout
	resp->data.code = RESPONSE_STATUS_ERROR;
  if (_RTOS_ThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, COMMAND_TIMEOUT))
    if (notif & EVT_COMMAND_OK)
      resp->data.code = RESPONSE_STATUS_OK;
}
