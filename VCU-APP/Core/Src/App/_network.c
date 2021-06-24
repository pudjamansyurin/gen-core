/*
 * _network.c
 *
 *  Created on: Jun 6, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_network.h"

#include "Drivers/_rtc.h"
#include "Drivers/_simcom.h"
#include "Libs/_mqtt.h"

/* External variables
 * --------------------------------------------*/
extern osMessageQueueId_t UssdQueueHandle, QuotaQueueHandle;

/* Private variables
 * --------------------------------------------*/
static char NET_BUF[200];

/* Public functions implementation
 * --------------------------------------------*/
void NET_CheckClock(void) {
  timestamp_t ts;

  if (RTC_NeedCalibration())
    if (Simcom_FetchTime(&ts)) RTC_Calibrate(&ts);
}

void NET_CheckCommand(void) {
  command_t cmd;

  if (Simcom_SetState(SIM_STATE_MQTT_ON, 0)) {
    if (MQTT_GotCommand()) {
      if (MQTT_AckPublish(&cmd))
        CMD_Execute(&cmd);
      else
        MQTT_FlushCommand();
    }
  }
}

void NET_CheckPayload(PAYLOAD_TYPE type) {
  payload_t *payload = &(RPT.payloads[type]);

  if (RPT_PayloadPending(payload))
    if (Simcom_SetState(SIM_STATE_MQTT_ON, 0))
      payload->pending = !MQTT_Publish(payload);
}

bool NET_SendUSSD(void) {
	bool ok = false;
  char ussd[20];

  memset(NET_BUF, 0, sizeof(NET_BUF));
  if (_osQueueGet(UssdQueueHandle, ussd))
    if (Simcom_SendUSSD(ussd, NET_BUF, sizeof(NET_BUF)))
      ok = _osQueuePutRst(QuotaQueueHandle, NET_BUF);

  return ok;
}

bool NET_ReadSMS(void) {
	bool ok = false;

  memset(NET_BUF, 0, sizeof(NET_BUF));
  if (Simcom_ReadNewSMS(NET_BUF, sizeof(NET_BUF)))
    ok = _osQueuePutRst(QuotaQueueHandle, NET_BUF);

  return ok;
}
