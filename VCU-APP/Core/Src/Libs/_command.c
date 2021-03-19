/*
 * _command.c
 *
 *  Created on: 28 Oct 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_command.h"
#include "Drivers/_crc.h"
#include "Drivers/_rtc.h"
#include "Libs/_eeprom.h"
#include "Libs/_finger.h"
#include "Libs/_firmware.h"
#include "Nodes/VCU.h"

/* External variables -------------------------------------------------------*/
#if (RTOS_ENABLE)
extern osMessageQueueId_t CommandQueueHandle;
#endif

/* Private functions prototypes
 * -----------------------------------------------*/
static void Debugger(command_t *cmd, uint8_t len);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t CMD_ValidateCommand(void *ptr, uint8_t len) {
  if (len > sizeof(command_t))
    return 0;

  if (len < sizeof(command_header_t))
    return 0;

  command_t *cmd = ptr;

  if (memcmp(cmd->header.prefix, PREFIX_COMMAND, 2) != 0)
    return 0;

  uint8_t size = sizeof(cmd->header.vin) + sizeof(cmd->header.send_time) +
                 sizeof(cmd->header.code) + sizeof(cmd->header.sub_code) +
                 sizeof(cmd->data);
  if (cmd->header.size > size)
    return 0;

  //  uint32_t crc;
  //	crc = CRC_Calculate8(
  //			(uint8_t*) &(cmd->header.size),
  //			sizeof(cmd->header.size) + size,
  //			0);
  //
  //	if (cmd->header.crc != crc)
  //		return;

  if (cmd->header.vin != VIN_VALUE)
    return 0;

  // TODO: check length according to command id

  return 1;
}

void CMD_ExecuteCommand(command_t *cmd) {
  uint8_t len = cmd->header.size -
                (sizeof(cmd->header.vin) + sizeof(cmd->header.send_time) +
                 sizeof(cmd->header.code) + sizeof(cmd->header.sub_code));

  Debugger(cmd, len);
  osMessageQueueReset(CommandQueueHandle);
  osMessageQueuePut(CommandQueueHandle, cmd, 0U, 0U);
}

void CMD_GenInfo(response_t *resp) {
  char msg[20];
  sprintf(msg, "VCU v.%d,", VCU_VERSION);

  if (HMI1.d.run)
    sprintf(resp->data.message, "%.*s HMI v.%d,", strlen(msg), msg,
            HMI1.d.version);

  sprintf(resp->data.message, "%.*s " VCU_VENDOR " - 20%d", strlen(msg), msg,
          VCU_BUILD_YEAR);
}

void CMD_FingerAdd(response_t *resp, osMessageQueueId_t queue) {
  uint32_t notif;
  uint8_t id;

  // wait response until timeout
  resp->data.res_code = RESPONSE_STATUS_ERROR;
  if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny,
                         FINGER_SCAN_TIMEOUT + 3000)) {
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

void CMD_FingerFetch(response_t *resp, osMessageQueueId_t queue) {
  uint32_t notif, len;
  finger_db_t finger;
  char fingers[3];

  // wait response until timeout
  resp->data.res_code = RESPONSE_STATUS_ERROR;
  if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 5000))
    if (notif & EVT_COMMAND_OK) {
      if (osMessageQueueGet(queue, finger.db, NULL, 0U) == osOK) {
        resp->data.res_code = RESPONSE_STATUS_OK;

        for (uint8_t id = 1; id <= FINGER_USER_MAX; id++) {
          if (finger.db[id - 1]) {
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
}

void CMD_Finger(response_t *resp) {
  uint32_t notif;

  // wait response until timeout
  resp->data.res_code = RESPONSE_STATUS_ERROR;
  if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 5000))
    if (notif & EVT_COMMAND_OK)
      resp->data.res_code = RESPONSE_STATUS_OK;
}

void CMD_RemotePairing(response_t *resp) {
  uint32_t notif;

  // wait response until timeout
  resp->data.res_code = RESPONSE_STATUS_ERROR;
  if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 5000))
    if (notif & EVT_COMMAND_OK)
      resp->data.res_code = RESPONSE_STATUS_OK;
}

void CMD_NetQuota(response_t *resp, osMessageQueueId_t queue) {
  uint32_t notif;

  // wait response until timeout
  resp->data.res_code = RESPONSE_STATUS_ERROR;
  if (_osThreadFlagsWait(&notif, EVT_MASK, osFlagsWaitAny, 40000))
    if (notif & EVT_COMMAND_OK)
      if (osMessageQueueGet(queue, resp->data.message, NULL, 0U) == osOK)
        resp->data.res_code = RESPONSE_STATUS_OK;
}

/* Private functions implementation
 * -------------------------------------------*/
static void Debugger(command_t *cmd, uint8_t len) {
  printf("Command:Payload (%u-%u)[%u]", cmd->header.code, cmd->header.sub_code,
         len);
  if (len) {
    printf(" = ");
    printf_hex(cmd->data.value, len);
  }
  printf("\n");
}
