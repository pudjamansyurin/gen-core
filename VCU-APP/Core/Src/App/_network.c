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
    if (SIM_FetchTime(&ts)) RTC_Calibrate(&ts);
}

void NET_CheckCommand(void) {
  command_t cmd;

  if (SIM_SetState(SIM_STATE_MQTT_ON, 0)) {
    if (MQTT_GotCommand()) {
      if (MQTT_AckPublish(&cmd))
        CMD_Execute(&cmd);
      else
        MQTT_FlushCommand();
    }
  }
}

void NET_CheckPayload(PAYLOAD_TYPE type) {
	const payload_t* pay;
	uint8_t pending;

  if (RPT_PayloadPending(type))
    if (SIM_SetState(SIM_STATE_MQTT_ON, 0)) {
    	pay = RPT_IO_GetPayload(type);
      pending = !MQTT_Publish(pay);
      RPT_IO_SetPayloadPending(type, pending);
    }
}

bool NET_SendUSSD(void) {
  bool ok = false;
  char ussd[20];

  memset(NET_BUF, 0, sizeof(NET_BUF));
  if (_osQueueGet(UssdQueueHandle, ussd))
    if (SIM_SendUSSD(ussd, NET_BUF, sizeof(NET_BUF)))
      ok = _osQueuePutRst(QuotaQueueHandle, NET_BUF);

  return ok;
}

bool NET_ReadSMS(void) {
  bool ok = false;

  memset(NET_BUF, 0, sizeof(NET_BUF));
  if (SIM_ReadNewSMS(NET_BUF, sizeof(NET_BUF)))
    ok = _osQueuePutRst(QuotaQueueHandle, NET_BUF);

  return ok;
}
