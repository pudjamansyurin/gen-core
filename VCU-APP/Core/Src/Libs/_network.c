/*
 * _network.c
 *
 *  Created on: Jun 6, 2021
 *      Author: pujak
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_network.h"
#include "Drivers/_simcom.h"
#include "Libs/_mqtt.h"

/* External variables -------------------------------------------*/
extern osMessageQueueId_t UssdQueueHandle, QuotaQueueHandle;

/* Private variables -------------------------------------------*/
static char buf[200];

/* Public functions implementation -------------------------------------------*/
void NET_CheckCommand(void) {
	command_t cmd;

	if (Simcom_SetState(SIM_STATE_MQTT_ON, 0)) {
		if (MQTT_GotCommand()) {
			if (MQTT_AckPublish(&cmd)) CMD_Execute(&cmd);
			else MQTT_FlushCommand();
		}
	}
}

void NET_CheckPayload(PAYLOAD_TYPE type) {
	payload_t *payload = &(RPT.payloads[type]);

	if (RPT_PayloadPending(payload))
		if (Simcom_SetState(SIM_STATE_MQTT_ON, 0))
			payload->pending = !MQTT_Publish(payload);
}

uint8_t NET_SendUSSD(void) {
	uint8_t ok = 0;
	char ussd[20];

	memset(buf, 0, sizeof(buf));
	if (_osQueueGet(UssdQueueHandle, ussd))
		if (Simcom_SendUSSD(ussd, buf, sizeof(buf)))
			ok = _osQueuePutRst(QuotaQueueHandle, buf);

	return ok;
}


uint8_t NET_ReadSMS(void) {
	uint8_t ok = 0;

	memset(buf, 0, sizeof(buf));
	if (Simcom_ReadNewSMS(buf, sizeof(buf)))
		ok = _osQueuePutRst(QuotaQueueHandle, buf);

	return ok;
}
