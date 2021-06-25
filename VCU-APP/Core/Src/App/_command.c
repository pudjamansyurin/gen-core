/*
 * _command.c
 *
 *  Created on: 28 Oct 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_command.h"

#include "App/_reporter.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMessageQueueId_t CommandQueueHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static uint8_t CMD_SZ[CMD_CODE_MAX][CMD_SUB_MAX];
static uint8_t SUB_SZ[CMD_CODE_MAX];

/* Private functions prototypes
 * --------------------------------------------*/
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
  CMD_SZ[CMD_CODE_HBAR][CMD_HBAR_PREDICTION] = 1;
  CMD_SZ[CMD_CODE_HBAR][CMD_HBAR_REVERSE] = 1;

  SUB_SZ[CMD_CODE_MCU] = CMD_MCU_MAX;
  CMD_SZ[CMD_CODE_MCU][CMD_MCU_SPEED_MAX] = 1;
  CMD_SZ[CMD_CODE_MCU][CMD_MCU_TEMPLATES] = 4 * 3;
}

bool CMD_ValidateCode(command_t *cmd) {
  command_header_t *h = &(cmd->header);
  bool ok = false;

  if (h->code < CMD_CODE_MAX)
    if (h->sub_code < SUB_SZ[h->code])
      ok = PayloadLen(cmd) <= CMD_SZ[h->code][h->sub_code];

  return ok;
}

bool CMD_ValidateContent(void *ptr, uint8_t len) {
  if (len > sizeof(command_t)) return false;
  if (len < sizeof(command_header_t)) return false;

  command_t *cmd = ptr;
  command_header_t *h = &(cmd->header);
  if (memcmp(h->prefix, PREFIX_COMMAND, 2) != 0) return false;

  uint8_t size = sizeof(h->vin) + sizeof(h->send_time) + sizeof(h->code) +
                 sizeof(h->sub_code) + sizeof(cmd->data);
  if (h->size > size) return false;
  if (h->vin != VIN_VALUE) return false;

  return true;
}

void CMD_Execute(command_t *cmd) {
  _osQueuePutRst(CommandQueueHandle, cmd);
  Debugger(cmd);
}

/* Private functions implementation
 * --------------------------------------------*/
static void Debugger(command_t *cmd) {
  uint8_t len = PayloadLen(cmd);

  printf("Command:Payload (%u-%u)[%u]", cmd->header.code, cmd->header.sub_code,
         len);
  if (len) {
    printf(" = ");
    printf_hex(cmd->data.value, len);
  }
  printf("\n");
}

static uint8_t PayloadLen(command_t *cmd) {
  command_header_t *h = &(cmd->header);
  uint8_t len = h->size;

  len -= sizeof(h->vin) + sizeof(h->send_time) + sizeof(h->code) +
         sizeof(h->sub_code);

  return len;
}
