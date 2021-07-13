/*
 * network.c
 *
 *  Created on: Jun 6, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/network.h"

#include "Drivers/rtc.h"
#include "Drivers/simcom.h"
#include "Libs/mqtt.h"

/* External variables
 * --------------------------------------------*/
extern osMessageQueueId_t UssdQueueHandle;
extern osMessageQueueId_t QuotaQueueHandle;

/* Private variables
 * --------------------------------------------*/
static char NET_BUF[200];

/* Public functions implementation
 * --------------------------------------------*/
void NET_CheckClock(void) {
	timestamp_t ts;

	if (!RTC_NeedCalibration()) return;
	if (!SIM_FetchTime(&ts)) return;
	RTC_Calibrate(&ts);
}

void NET_CheckCommand(void) {
	command_t cmd;

	if (!SIMSta_SetState(SIM_STATE_MQTT_ON, 0)) return;
	if (!MQTT_GotCommand()) return;

	if (MQTT_AckPublish(&cmd))
		CMD_Execute(&cmd);
	else
		MQTT_FlushCommand();
}

void NET_CheckPayload(PAYLOAD_TYPE type) {
	const payload_t* pld;
	uint8_t pending;

	if (!RPT_PayloadPending(type)) return;
	if (!SIMSta_SetState(SIM_STATE_MQTT_ON, 0)) return;

	pld = RPT_IO_Payload(type);
	pending = !MQTT_Publish(pld);
	RPT_IO_SetPayloadPending(type, pending);
}

bool NET_SendUSSD(void) {
	bool ok = false;
	char ussd[20];

	memset(NET_BUF, 0, sizeof(NET_BUF));
	if (OS_QueueGet(UssdQueueHandle, ussd))
		if (SIM_SendUSSD(ussd, NET_BUF, sizeof(NET_BUF)))
			ok = OS_QueuePutRst(QuotaQueueHandle, NET_BUF);

	return ok;
}

bool NET_ReadSMS(void) {
	bool ok = false;

	memset(NET_BUF, 0, sizeof(NET_BUF));
	if (SIM_ReadLastSMS(NET_BUF, sizeof(NET_BUF)))
		ok = OS_QueuePutRst(QuotaQueueHandle, NET_BUF);

	return ok;
}
