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

/* Private constants
 * --------------------------------------------*/
#define CMD_SUB_MAX ((uint8_t)10)

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMessageQueueId_t CommandQueueHandle;
#endif

/* Private variables
 * --------------------------------------------*/
static uint8_t CMD_SZ[CMDC_MAX][CMD_SUB_MAX];
static uint8_t SUB_SZ[CMDC_MAX];

/* Private functions prototypes
 * --------------------------------------------*/
static void Debugger(command_t *cmd);

/* Public functions implementation
 * --------------------------------------------*/
void CMD_Init(void) {
  memset(CMD_SZ, 0, sizeof(CMD_SZ));
  SUB_SZ[CMDC_GEN] = CMD_GEN_MAX;
  CMD_SZ[CMDC_GEN][CMD_GEN_INFO] = 0;
  CMD_SZ[CMDC_GEN][CMD_GEN_LED] = 1;
  CMD_SZ[CMDC_GEN][CMD_GEN_RTC] = 7;
  CMD_SZ[CMDC_GEN][CMD_GEN_ODOM] = 2;
  CMD_SZ[CMDC_GEN][CMD_GEN_ANTITHIEF] = 0;
  CMD_SZ[CMDC_GEN][CMD_GEN_RPT_FLUSH] = 0;
  CMD_SZ[CMDC_GEN][CMD_GEN_RPT_BLOCK] = 1;

  SUB_SZ[CMDC_OVD] = CMD_OVD_MAX;
  CMD_SZ[CMDC_OVD][CMD_OVD_STATE] = 1;
  CMD_SZ[CMDC_OVD][CMD_OVD_RPT_INTERVAL] = 2;
  CMD_SZ[CMDC_OVD][CMD_OVD_RPT_FRAME] = 1;
  CMD_SZ[CMDC_OVD][CMD_OVD_RMT_SEAT] = 0;
  CMD_SZ[CMDC_OVD][CMD_OVD_RMT_ALARM] = 0;

  SUB_SZ[CMDC_AUDIO] = CMD_AUDIO_MAX;
  CMD_SZ[CMDC_AUDIO][CMD_AUDIO_BEEP] = 0;

  SUB_SZ[CMDC_FGR] = CMD_FGR_MAX;
  CMD_SZ[CMDC_FGR][CMD_FGR_FETCH] = 0;
  CMD_SZ[CMDC_FGR][CMD_FGR_ADD] = 0;
  CMD_SZ[CMDC_FGR][CMD_FGR_DEL] = 1;
  CMD_SZ[CMDC_FGR][CMD_FGR_RST] = 0;

  SUB_SZ[CMDC_RMT] = CMD_RMT_MAX;
  CMD_SZ[CMDC_RMT][CMD_RMT_PAIRING] = 0;

  SUB_SZ[CMDC_FOTA] = CMD_FOTA_MAX;
  CMD_SZ[CMDC_FOTA][CMD_FOTA_VCU] = 0;
  CMD_SZ[CMDC_FOTA][CMD_FOTA_HMI] = 0;

  SUB_SZ[CMDC_NET] = CMD_NET_MAX;
  CMD_SZ[CMDC_NET][CMD_NET_SEND_USSD] = 20;
  CMD_SZ[CMDC_NET][CMD_NET_READ_SMS] = 0;

  SUB_SZ[CMDC_CON] = CMD_CON_MAX;
  CMD_SZ[CMDC_CON][CMD_CON_APN] = 3*30;
  CMD_SZ[CMDC_CON][CMD_CON_FTP] = 3*30;
  CMD_SZ[CMDC_CON][CMD_CON_MQTT] = 4*30;

  SUB_SZ[CMDC_HBAR] = CMD_HBAR_MAX;
  CMD_SZ[CMDC_HBAR][CMD_HBAR_DRIVE] = 1;
  CMD_SZ[CMDC_HBAR][CMD_HBAR_TRIP] = 1;
  CMD_SZ[CMDC_HBAR][CMD_HBAR_PREDICTION] = 1;
  CMD_SZ[CMDC_HBAR][CMD_HBAR_REVERSE] = 1;

  SUB_SZ[CMDC_MCU] = CMD_MCU_MAX;
  CMD_SZ[CMDC_MCU][CMD_MCU_SPEED_MAX] = 1;
  CMD_SZ[CMDC_MCU][CMD_MCU_TEMPLATES] = 4 * 3;
}

bool CMD_ValidateCode(command_t *cmd) {
  command_header_t *h = &(cmd->header);
  bool ok = false;

  if (h->code < CMDC_MAX)
    if (h->sub_code < SUB_SZ[h->code])
      ok = CMD_GetPayloadSize(cmd) <= CMD_SZ[h->code][h->sub_code];

  return ok;
}

bool CMD_ValidateContent(void *ptr, uint8_t len) {
  if (len > sizeof(command_t)) return false;
  if (len < sizeof(command_header_t)) return false;

  command_t *cmd = ptr;
  command_header_t *h = &(cmd->header);
  if (memcmp(h->prefix, PREFIX_COMMAND, 2) != 0) return false;

  uint8_t size = CMD_GetPayloadSize(cmd);
  if (size > sizeof(cmd->data)) return false;
  if (h->vin != VIN_VALUE) return false;

  return true;
}

uint8_t CMD_GetPayloadSize(command_t *cmd) {
  command_header_t *h = &(cmd->header);
  uint8_t headerSz;

  headerSz = sizeof(command_header_t) - (sizeof(h->prefix) + sizeof(h->size));

  if (h->size >= headerSz)
  	return h->size - headerSz;
  return 0;
}

void CMD_Execute(command_t *cmd) {
  _osQueuePutRst(CommandQueueHandle, cmd);
  Debugger(cmd);
}

/* Private functions implementation
 * --------------------------------------------*/
static void Debugger(command_t *cmd) {
  command_header_t *h = &(cmd->header);
  uint8_t len = CMD_GetPayloadSize(cmd);

  printf("Command:Payload (%u-%u)[%u]", h->code, h->sub_code, len);
  if (len) {
    printf(" = ");
    printf_hex(cmd->data.value, len);
  }
  printf("\n");
}
