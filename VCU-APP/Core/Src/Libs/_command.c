/*
 * _command.c
 *
 *  Created on: 28 Oct 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_command.h"
#include "Libs/_finger.h"
#include "Nodes/HMI1.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMessageQueueId_t CommandQueueHandle;
#endif

/* Private variables
 * -----------------------------------------------*/
static uint8_t CMD_SZ[CMD_CODE_MAX][CMD_SUB_MAX];
static uint8_t SUB_SZ[CMD_CODE_MAX];

/* Private functions prototypes
 * -----------------------------------------------*/
static void Debugger(command_t *cmd);
static uint8_t PayloadLen(command_t *cmd);

/* Public functions implementation
 * --------------------------------------------*/
void CMD_Init(void) {
	memset(CMD_SZ, 0, sizeof(CMD_SZ));

	SUB_SZ[CMD_CODE_GEN] = CMD_GEN_MAX;
	CMD_SZ[CMD_CODE_GEN][CMD_GEN_INFO] = 0;
	CMD_SZ[CMD_CODE_GEN][CMD_GEN_LED] = 1;
	CMD_SZ[CMD_CODE_GEN][CMD_GEN_RTC] = 7;
	CMD_SZ[CMD_CODE_GEN][CMD_GEN_ODOM] = 2;
	CMD_SZ[CMD_CODE_GEN][CMD_GEN_ANTITHIEF] = 0;
	CMD_SZ[CMD_CODE_GEN][CMD_GEN_RPT_FLUSH] = 0;
	CMD_SZ[CMD_CODE_GEN][CMD_GEN_RPT_BLOCK] = 1;

	SUB_SZ[CMD_CODE_OVD] = CMD_OVD_MAX;
	CMD_SZ[CMD_CODE_OVD][CMD_OVD_STATE] = 1;
	CMD_SZ[CMD_CODE_OVD][CMD_OVD_RPT_INTERVAL] = 2;
	CMD_SZ[CMD_CODE_OVD][CMD_OVD_RPT_FRAME] = 1;
	CMD_SZ[CMD_CODE_OVD][CMD_OVD_RMT_SEAT] = 0;
	CMD_SZ[CMD_CODE_OVD][CMD_OVD_RMT_ALARM] = 0;

	SUB_SZ[CMD_CODE_AUDIO] = CMD_AUDIO_MAX;
	CMD_SZ[CMD_CODE_AUDIO][CMD_AUDIO_BEEP] = 0;

	SUB_SZ[CMD_CODE_FGR] = CMD_FGR_MAX;
	CMD_SZ[CMD_CODE_FGR][CMD_FGR_FETCH] = 0;
	CMD_SZ[CMD_CODE_FGR][CMD_FGR_ADD] = 0;
	CMD_SZ[CMD_CODE_FGR][CMD_FGR_DEL] = 1;
	CMD_SZ[CMD_CODE_FGR][CMD_FGR_RST] = 0;

	SUB_SZ[CMD_CODE_RMT] = CMD_RMT_MAX;
	CMD_SZ[CMD_CODE_RMT][CMD_RMT_PAIRING] = 0;

	SUB_SZ[CMD_CODE_FOTA] = CMD_FOTA_MAX;
	CMD_SZ[CMD_CODE_FOTA][CMD_FOTA_VCU] = 0;
	CMD_SZ[CMD_CODE_FOTA][CMD_FOTA_HMI] = 0;

	SUB_SZ[CMD_CODE_NET] = CMD_NET_MAX;
	CMD_SZ[CMD_CODE_NET][CMD_NET_SEND_USSD] = 20;
	CMD_SZ[CMD_CODE_NET][CMD_NET_READ_SMS] = 0;

	SUB_SZ[CMD_CODE_HBAR] = CMD_HBAR_MAX;
	CMD_SZ[CMD_CODE_HBAR][CMD_HBAR_DRIVE] = 1;
	CMD_SZ[CMD_CODE_HBAR][CMD_HBAR_TRIP] = 1;
	CMD_SZ[CMD_CODE_HBAR][CMD_HBAR_REPORT] = 1;
	CMD_SZ[CMD_CODE_HBAR][CMD_HBAR_REVERSE] = 1;

	SUB_SZ[CMD_CODE_MCU] = CMD_MCU_MAX;
	CMD_SZ[CMD_CODE_MCU][CMD_MCU_SPEED_MAX] = 1;
	CMD_SZ[CMD_CODE_MCU][CMD_MCU_TEMPLATES] = 4 * 3;

}

uint8_t CMD_Validate(command_t *cmd) {
	command_header_t *h = &(cmd->header);
	uint8_t valid = 0;

	if (h->code < CMD_CODE_MAX)
		if (h->sub_code < SUB_SZ[h->code])
			valid = PayloadLen(cmd) <= CMD_SZ[h->code][h->sub_code];

	return valid;
}

uint8_t CMD_ValidateRaw(void *ptr, uint8_t len) {
	if (len > sizeof(command_t))
		return 0;

	if (len < sizeof(command_header_t))
		return 0;

	command_t *cmd = ptr;
	command_header_t *h = &(cmd->header);

	if (memcmp(h->prefix, PREFIX_COMMAND, 2) != 0)
		return 0;

	uint8_t size = sizeof(h->vin) + sizeof(h->send_time) +
			sizeof(h->code) + sizeof(h->sub_code) +
			sizeof(cmd->data);
	if (h->size > size)
		return 0;

	if (h->vin != VIN_VALUE)
		return 0;

	return 1;
}

void CMD_Execute(command_t *cmd) {
	Debugger(cmd);
	_osQueuePutRst(CommandQueueHandle, cmd);
}

void CMD_GenInfo(response_t *resp) {
	char msg[20];
	sprintf(msg, "VCU v.%d,", VCU_VERSION);

	if (HMI1.d.active)
		sprintf(resp->data.message, "%.*s HMI v.%d,", strlen(msg), msg,
				HMI1.d.version);

	sprintf(resp->data.message, "%.*s " VCU_VENDOR " - 20%d", strlen(msg), msg,
			VCU_BUILD_YEAR);
}

void CMD_FingerAdd(response_t *resp, osMessageQueueId_t queue) {
	uint32_t notif;
	uint8_t id;

	// wait response until timeout
	resp->data.res_code = RESP_ERROR;
	if (_osFlagAny(&notif, (FINGER_SCAN_MS*2) + 5000)) {
		if (notif & FLAG_COMMAND_OK) {
			if (_osQueueGet(queue, &id)) {
				sprintf(resp->data.message, "%u", id);
				resp->data.res_code = RESP_OK;
			}
		} else {
			if (_osQueueGet(queue, &id))
				if (id == 0)
					sprintf(resp->data.message, "Max. reached : %u", FINGER_USER_MAX);
		}
	}
}

void CMD_FingerFetch(response_t *resp) {
	uint32_t notif, len;
	char fingers[3];

	// wait response until timeout
	memset(FGR.d.db, 0, FINGER_USER_MAX);
	resp->data.res_code = RESP_ERROR;
	if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000)){
		resp->data.res_code = RESP_OK;

		for (uint8_t id = 1; id <= FINGER_USER_MAX; id++) {
			if (FGR.d.db[id - 1]) {
				sprintf(fingers, "%1d,", id);
				strcat(resp->data.message, fingers);
			}
		}

		// remove last comma
		len = strnlen(resp->data.message, sizeof(resp->data.message));
		if (len > 0)
			resp->data.message[len - 1] = '\0';
	}
}

void CMD_Finger(response_t *resp) {
	uint32_t notif;

	// wait response until timeout
	resp->data.res_code = RESP_ERROR;
	if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000))
		resp->data.res_code = RESP_OK;
}

void CMD_RemotePairing(response_t *resp) {
	uint32_t notif;

	// wait response until timeout
	resp->data.res_code = RESP_ERROR;
	if (_osFlagOne(&notif, FLAG_COMMAND_OK, 5000))
		resp->data.res_code = RESP_OK;
}

void CMD_NetQuota(response_t *resp, osMessageQueueId_t queue) {
	uint32_t notif;

	// wait response until timeout
	resp->data.res_code = RESP_ERROR;
	if (_osFlagOne(&notif, FLAG_COMMAND_OK, 40000))
		if (_osQueueGet(queue, resp->data.message))
			resp->data.res_code = RESP_OK;
}

/* Private functions implementation
 * -------------------------------------------*/
static void Debugger(command_t *cmd) {
	uint8_t len = PayloadLen(cmd);

	printf("Command:Payload (%u-%u)[%u]", cmd->header.code, cmd->header.sub_code, len);
	if (len) {
		printf(" = ");
		printf_hex(cmd->data.value, len);
	}
	printf("\n");
}

static uint8_t PayloadLen(command_t *cmd) {
	command_header_t *h = &(cmd->header);
	uint8_t len = h->size;

	len -= sizeof(h->vin) + sizeof(h->send_time) + sizeof(h->code) + sizeof(h->sub_code);

	return len;
}
