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
#include "App/_iap.h"

/* External variables
 * --------------------------------------------*/
#if (APP)
extern osMessageQueueId_t CommandQueueHandle;
#endif

/* Private constants
 * --------------------------------------------*/
#define CMD_SUB_MAX ((uint8_t)10)

/* Private types
 * --------------------------------------------*/
typedef struct {
	uint8_t cmd[CMDC_MAX][CMD_SUB_MAX];
	uint8_t sub[CMDC_MAX];
} cmd_size_t;

/* Private variables
 * --------------------------------------------*/
static cmd_size_t SZ;

/* Private functions prototypes
 * --------------------------------------------*/
static void Debugger(const command_t *cmd);

/* Public functions implementation
 * --------------------------------------------*/
void CMD_Init(void) {
  memset(SZ.cmd, 0, sizeof(SZ.cmd));
  SZ.sub[CMDC_GEN] = CMD_GEN_MAX;
  SZ.cmd[CMDC_GEN][CMD_GEN_INFO] = 0;
  SZ.cmd[CMDC_GEN][CMD_GEN_LED] = 1;
  SZ.cmd[CMDC_GEN][CMD_GEN_RTC] = 7;
  SZ.cmd[CMDC_GEN][CMD_GEN_ODO] = 2;
  SZ.cmd[CMDC_GEN][CMD_GEN_ANTITHIEF] = 0;
  SZ.cmd[CMDC_GEN][CMD_GEN_RPT_FLUSH] = 0;
  SZ.cmd[CMDC_GEN][CMD_GEN_RPT_BLOCK] = 1;

  SZ.sub[CMDC_OVD] = CMD_OVD_MAX;
  SZ.cmd[CMDC_OVD][CMD_OVD_STATE] = 1;
  SZ.cmd[CMDC_OVD][CMD_OVD_RPT_INTERVAL] = 2;
  SZ.cmd[CMDC_OVD][CMD_OVD_RPT_FRAME] = 1;
  SZ.cmd[CMDC_OVD][CMD_OVD_RMT_SEAT] = 0;
  SZ.cmd[CMDC_OVD][CMD_OVD_RMT_ALARM] = 0;

  SZ.sub[CMDC_AUDIO] = CMD_AUDIO_MAX;
  SZ.cmd[CMDC_AUDIO][CMD_AUDIO_BEEP] = 0;

  SZ.sub[CMDC_FGR] = CMD_FGR_MAX;
  SZ.cmd[CMDC_FGR][CMD_FGR_FETCH] = 0;
  SZ.cmd[CMDC_FGR][CMD_FGR_ADD] = 0;
  SZ.cmd[CMDC_FGR][CMD_FGR_DEL] = 1;
  SZ.cmd[CMDC_FGR][CMD_FGR_RST] = 0;

  SZ.sub[CMDC_RMT] = CMD_RMT_MAX;
  SZ.cmd[CMDC_RMT][CMD_RMT_PAIRING] = 0;

  SZ.sub[CMDC_FOTA] = CMD_FOTA_MAX;
  SZ.cmd[CMDC_FOTA][CMD_FOTA_VCU] = 0;
  SZ.cmd[CMDC_FOTA][CMD_FOTA_HMI] = 0;

  SZ.sub[CMDC_NET] = CMD_NET_MAX;
  SZ.cmd[CMDC_NET][CMD_NET_SEND_USSD] = 20;
  SZ.cmd[CMDC_NET][CMD_NET_READ_SMS] = 0;

  SZ.sub[CMDC_CON] = CMD_CON_MAX;
  SZ.cmd[CMDC_CON][CMD_CON_APN] = 3*30;
  SZ.cmd[CMDC_CON][CMD_CON_FTP] = 3*30;
  SZ.cmd[CMDC_CON][CMD_CON_MQTT] = 4*30;

  SZ.sub[CMDC_HBAR] = CMD_HBAR_MAX;
  SZ.cmd[CMDC_HBAR][CMD_HBAR_DRIVE] = 1;
  SZ.cmd[CMDC_HBAR][CMD_HBAR_TRIP] = 1;
  SZ.cmd[CMDC_HBAR][CMD_HBAR_AVG] = 1;
  SZ.cmd[CMDC_HBAR][CMD_HBAR_REVERSE] = 1;

  SZ.sub[CMDC_MCU] = CMD_MCU_MAX;
  SZ.cmd[CMDC_MCU][CMD_MCU_SPEED_MAX] = 1;
  SZ.cmd[CMDC_MCU][CMD_MCU_TEMPLATES] = 4 * 3;
}

bool CMD_ValidateCode(const command_t *cmd) {
	const command_header_t *h = &(cmd->header);
  bool ok = false;

  if (h->code < CMDC_MAX)
    if (h->sub_code < SZ.sub[h->code])
      ok = CMD_GetPayloadSize(cmd) <= SZ.cmd[h->code][h->sub_code];

  return ok;
}

bool CMD_ValidateContent(const void *ptr, uint8_t len) {
  if (len > sizeof(command_t)) return false;
  if (len < sizeof(command_header_t)) return false;

  const command_t *cmd = ptr;
  const command_header_t *h = &(cmd->header);
  if (memcmp(h->prefix, PREFIX_COMMAND, 2) != 0) return false;

  uint8_t size = CMD_GetPayloadSize(cmd);
  if (size > sizeof(cmd->data)) return false;
  if (h->vin != IAP_GetBootMeta(VIN_OFFSET)) return false;

  return true;
}

uint8_t CMD_GetPayloadSize(const command_t *cmd) {
  const command_header_t *h = &(cmd->header);
  uint8_t headerSz;

  headerSz = sizeof(command_header_t) - (sizeof(h->prefix) + sizeof(h->size));

  if (h->size >= headerSz)
  	return h->size - headerSz;
  return 0;
}

void CMD_Execute(const command_t *cmd) {
  _osQueuePutRst(CommandQueueHandle, cmd);
  Debugger(cmd);
}

/* Private functions implementation
 * --------------------------------------------*/
static void Debugger(const command_t *cmd) {
	const command_header_t *h = &(cmd->header);
  uint8_t len = CMD_GetPayloadSize(cmd);

  printf("Command:Payload (%u-%u)[%u]", h->code, h->sub_code, len);
  if (len) {
    printf(" = ");
    printf_hex(cmd->data.value, len);
  }
  printf("\n");
}
