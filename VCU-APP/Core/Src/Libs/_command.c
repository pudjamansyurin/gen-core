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
#include "Libs/_finger.h"
#include "Nodes/VCU.h"

/* Private variables ----------------------------------------------------------*/
osMessageQueueId_t cmdQueue;

/* Private functions prototypes -----------------------------------------------*/
static void Debugger(command_t *cmd);

/* Public functions implementation --------------------------------------------*/
void CMD_Init(osMessageQueueId_t mCmdQueue) {
	cmdQueue = mCmdQueue;
}

void CMD_CheckCommand(command_t *cmd) {
	//  uint32_t crc;

	if (memcmp(cmd->rx.header.prefix, PREFIX_COMMAND, 2) != 0)
		return;

	//	crc = CRC_Calculate8(
	//			(uint8_t*) &(cmd->rx.header.size),
	//			sizeof(cmd->rx.header.size) + size,
	//			0);
	//
	//	if (cmd->rx.header.crc != crc)
	//		return;

	if (cmd->rx.header.unit_id != VCU.d.unit_id)
		return;

	cmd->length = cmd->rx.header.size -
			(sizeof(cmd->rx.header.unit_id) +
			sizeof(cmd->rx.header.send_time) +
			sizeof(cmd->rx.data.code) +
			sizeof(cmd->rx.data.sub_code));
	if (cmd->length > sizeof(cmd->rx.data.value) )
		return;

	Debugger(cmd);
	osMessageQueueReset(cmdQueue);
	osMessageQueuePut(cmdQueue, cmd, 0U, 0U);
}

void CMD_GenInfo(response_t *resp, uint8_t *hmi_started, uint16_t *hmi_version) {
	char msg[20];
	sprintf(msg, "VCU v.%d,", VCU_VERSION);

	if (*hmi_started)
		sprintf(resp->data.message,
				"%.*s HMI v.%d,",
				strlen(msg),
				msg,
				*hmi_version);

	sprintf(resp->data.message,
			"%.*s "VCU_VENDOR" - 20%d",
			strlen(msg),
			msg,
			VCU_BUILD_YEAR);
}

void CMD_GenQuota(response_t *resp, osThreadId_t threadId, osMessageQueueId_t queue) {
	uint32_t notif;

	osThreadFlagsSet(threadId, EVT_IOT_CHECK_QUOTA);

	// wait response until timeout
	resp->data.res_code = RESPONSE_STATUS_ERROR;
	if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 40000))
		if (notif & EVT_COMMAND_OK)
			if (osMessageQueueGet(queue, resp->data.message, NULL, 0U) == osOK)
				resp->data.res_code = RESPONSE_STATUS_OK;
}

void CMD_GenLed(command_t *cmd) {
	GATE_LedWrite(*(uint8_t*) cmd->rx.data.value);
}

void CMD_GenOverride(command_t *cmd, uint8_t *override_state) {
	*override_state = *(uint8_t*) cmd->rx.data.value;
}

void CMD_Fota(response_t *resp, IAP_TYPE type, uint16_t *bat, uint16_t *hmi_version) {
	FW_EnterModeIAP(type, resp->data.message, bat, hmi_version);

	/* This line is never reached (if FOTA is activated) */
}

void CMD_ReportRTC(command_t *cmd) {
	RTC_Write(*(datetime_t*) cmd->rx.data.value);
}

void CMD_ReportOdom(command_t *cmd) {
	uint32_t value = *(uint32_t*) cmd->rx.data.value;
	EEPROM_Odometer(EE_CMD_W, value * 1000);
}


void CMD_AudioBeep(osThreadId_t threadId) {
	osThreadFlagsSet(threadId, EVT_AUDIO_BEEP);
}

void CMD_AudioMute(command_t *cmd, osThreadId_t threadId) {
	uint8_t value = *(uint8_t*) cmd->rx.data.value;
	uint32_t flag;

	flag = value ? EVT_AUDIO_MUTE_ON : EVT_AUDIO_MUTE_OFF;
	osThreadFlagsSet(threadId, flag);
}

void CMD_FingerAdd(response_t *resp, osThreadId_t threadId, osMessageQueueId_t queue) {
	uint32_t notif;
	uint8_t id;

	osThreadFlagsSet(threadId, EVT_FINGER_ADD);

	// wait response until timeout
	resp->data.res_code = RESPONSE_STATUS_ERROR;
	if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, FINGER_SCAN_TIMEOUT+3000)) {
		if (notif & EVT_COMMAND_OK) {
			if (osMessageQueueGet(queue, &id, NULL, 0U) == osOK) {
				sprintf(resp->data.message, "%u", id);
				resp->data.res_code = RESPONSE_STATUS_OK;
			}
		} else {
			if (osMessageQueueGet(queue, &id, NULL, 0U) == osOK)
				if (id == 0)
					sprintf(resp->data.message, "Max. reached : %u", FINGER_USER_MAX);
		}
	}
}

void CMD_FingerFetch(response_t *resp, osThreadId_t threadId, osMessageQueueId_t queue) {
	uint32_t notif, len;
	finger_db_t finger;
	char fingers[3];

	osThreadFlagsSet(threadId, EVT_FINGER_FETCH);

	// wait response until timeout
	resp->data.res_code = RESPONSE_STATUS_ERROR;
	if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 5000))
		if (notif & EVT_COMMAND_OK) {
			if (osMessageQueueGet(queue, finger.db, NULL, 0U) == osOK) {
				resp->data.res_code = RESPONSE_STATUS_OK;

				for (uint8_t id=1; id<=FINGER_USER_MAX; id++) {
					if (finger.db[id-1]) {
						sprintf(fingers, "%1d,", id);
						strcat(resp->data.message, fingers);
					}
				}

				// remove last comma
				len = strnlen(resp->data.message, sizeof(resp->data.message));
				if (len > 0)
					resp->data.message[len-1] = '\0';
			}
		}
}


void CMD_FingerDelete(response_t *resp, command_t *cmd, osThreadId_t threadId, osMessageQueueId_t queue) {
    uint8_t value = *(uint8_t*) cmd->rx.data.value;

	osMessageQueueReset(queue);
    osMessageQueuePut(queue, &value, 0U, 0U);
    CMD_Finger(resp, threadId, EVT_FINGER_DEL);
}

void CMD_Finger(response_t *resp, osThreadId_t threadId, uint8_t event) {
	uint32_t notif;

	osThreadFlagsSet(threadId, event);

	// wait response until timeout
	resp->data.res_code = RESPONSE_STATUS_ERROR;
	if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 5000))
		if (notif & EVT_COMMAND_OK)
			resp->data.res_code = RESPONSE_STATUS_OK;
}

void CMD_RemoteUnitID(command_t *cmd, osThreadId_t threadIot, osThreadId_t threadRemote) {
	uint32_t value = *(uint32_t*) cmd->rx.data.value;
	// persist changes
	EEPROM_UnitID(EE_CMD_W, value);
	// resubscribe mqtt topic
	osThreadFlagsSet(threadIot, EVT_IOT_RESUBSCRIBE);
	// change nrf address
	osThreadFlagsSet(threadRemote, EVT_REMOTE_REINIT);
}

void CMD_RemotePairing(response_t *resp, osThreadId_t threadId) {
	uint32_t notif;

	osThreadFlagsSet(threadId, EVT_REMOTE_PAIRING);

	// wait response until timeout
	resp->data.res_code = RESPONSE_STATUS_ERROR;
	if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 5000))
		if (notif & EVT_COMMAND_OK)
			resp->data.res_code = RESPONSE_STATUS_OK;
}

/* Private functions implementation -------------------------------------------*/
static void Debugger(command_t *cmd) {
	printf("Command:Payload (%u-%u)[%u]",
			cmd->rx.data.code,
			cmd->rx.data.sub_code,
			cmd->length
	);
	if (cmd->length) {
		printf(" = ");
		printf_hex(cmd->rx.data.value, cmd->length);
	}
	printf("\n");
}
