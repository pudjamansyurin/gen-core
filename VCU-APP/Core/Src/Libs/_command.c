/*
 * _command.c
 *
 *  Created on: 28 Oct 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Drivers/_crc.h"
#include "Libs/_command.h"
#include "Libs/_firmware.h"
#include "Libs/_eeprom.h"

/* Private variables ----------------------------------------------------------*/
osMessageQueueId_t cmdQueue;
osMessageQueueId_t driverQueue;

/* Private functions prototypes -----------------------------------------------*/
static void Debugger(command_t *cmd);

/* Public functions implementation --------------------------------------------*/
void CMD_Init(osMessageQueueId_t mCmdQueue, osMessageQueueId_t mDriverQueue) {
	cmdQueue = mCmdQueue;
	driverQueue = mDriverQueue;
}

void CMD_CheckCommand(command_t command) {
	uint32_t crc;

	if (command.header.size != sizeof(command.data))
		return;

	crc = CRC_Calculate8(
			(uint8_t*) &(command.header.size),
			sizeof(command.header.size) + sizeof(command.data),
			0);

	if (command.header.crc != crc)
		return;

	// Debugger(&command);
	osMessageQueuePut(cmdQueue, &command, 0U, 0U);
}

void CMD_GenInfo(response_t *resp) {
	sprintf(resp->data.message,
			"VCU v.%d, "VCU_VENDOR" @ 20%d",
			VCU_VERSION,
			VCU_BUILD_YEAR);
}

void CMD_GenLed(command_t *cmd) {
	GATE_LedWrite(cmd->data.value[0]);
}

void CMD_GenOverride(command_t *cmd, uint8_t *override_state) {
	*override_state = cmd->data.value[0];
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

void CMD_ReportRTC(command_t *cmd) {
	RTC_Write(*(datetime_t*) cmd->data.value);
}

void CMD_ReportOdom(command_t *cmd) {
	EEPROM_Odometer(EE_CMD_W, *(uint32_t*) cmd->data.value);
}

void CMD_ReportUnitID(command_t *cmd) {
	EEPROM_UnitID(EE_CMD_W, *(uint32_t*) cmd->data.value);
}


void CMD_AudioBeep(osThreadId_t threadId) {
	osThreadFlagsSet(threadId, EVT_AUDIO_BEEP);
}

void CMD_AudioMute(osThreadId_t threadId, command_t *cmd) {
	uint32_t flag;

	flag =  cmd->data.value[0] ? EVT_AUDIO_MUTE_ON : EVT_AUDIO_MUTE_OFF;
	osThreadFlagsSet(threadId, flag);
}


void CMD_Finger(osThreadId_t threadId, uint8_t event, response_t *resp) {
	uint32_t notif, timeout;
	uint8_t id;

	osThreadFlagsSet(threadId, event);

	// decide the timeout
	timeout = (event == EVT_FINGER_ADD) ? 20000 : 5000;

	// wait response until timeout
	resp->data.code = RESPONSE_STATUS_ERROR;
	if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, timeout))
		if (notif & EVT_COMMAND_OK) {
			resp->data.code = RESPONSE_STATUS_OK;

			if (event == EVT_FINGER_ADD) {
				if (osMessageQueueGet(driverQueue, &id, NULL, 0U) == osOK)
					sprintf(resp->data.message, "%u", id);
				else
					resp->data.code = RESPONSE_STATUS_ERROR;
			}
		}
}

void CMD_RemotePairing(osThreadId_t threadId, response_t *resp) {
	uint32_t notif;

	osThreadFlagsSet(threadId, EVT_REMOTE_PAIRING);

	// wait response until timeout
	resp->data.code = RESPONSE_STATUS_ERROR;
	if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, COMMAND_TIMEOUT))
		if (notif & EVT_COMMAND_OK)
			resp->data.code = RESPONSE_STATUS_OK;
}

/* Private functions implementation -------------------------------------------*/
static void Debugger(command_t *cmd) {
	printf("Command:Payload [%u-%u] = %.*s\n",
			cmd->data.code,
			cmd->data.sub_code,
			sizeof(cmd->data.value),
			(char*) &(cmd->data.value)
	);
}
