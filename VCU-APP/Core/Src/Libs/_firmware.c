/*
 * _firmware.c
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_bat.h"
#include "Libs/_firmware.h"
#include "Libs/_command.h"
#include "Libs/_eeprom.h"
#include "Nodes/HMI1.h"

/* Private functions prototypes
 * -----------------------------------------------*/
static void FW_MakeResponseIAP(char *message, char *node);
static uint8_t FW_ValidResponseIAP(void);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t FW_EnterModeIAP(IAP_TYPE type, char *message) {
  /* Retain FOTA */
  EEPROM_FotaType(EE_CMD_W, type);
  EEPROM_FotaVersion(EE_CMD_W,
                     (type == IAP_HMI) ? HMI1.d.version : VCU_VERSION);

  /* Set flag to SRAM */
  *(uint32_t *)IAP_FLAG_ADDR = IAP_FLAG;

  /* Reset */
  HAL_NVIC_SystemReset();

  /* Never reached if FOTA executed */
  return 0;
}

uint8_t FW_PostFota(response_t *response) {
  char node[4];
  uint8_t valid = 0;

  if (FW_ValidResponseIAP()) {
    sprintf(node, FOTA.TYPE == IAP_HMI ? "HMI" : "VCU");

    // set default value
    response->header.code = CMD_CODE_FOTA;
    response->header.sub_code =
        FOTA.TYPE == IAP_HMI ? CMD_FOTA_HMI : CMD_FOTA_VCU;
    response->data.res_code = RESPONSE_STATUS_ERROR;
    sprintf(response->data.message, "%s Failed", node);

    // check fota response
    switch (*(uint32_t *)IAP_RESPONSE_ADDR) {
    case IAP_BATTERY_LOW:
      sprintf(response->data.message, "%s Battery Low (-%u mV)", node,
              SIMCOM_MIN_MV - BAT_ScanValue());
      break;
    case IAP_SIMCOM_TIMEOUT:
      sprintf(response->data.message, "%s Internet Timeout", node);
      break;
    case IAP_DOWNLOAD_ERROR:
      sprintf(response->data.message, "%s Download Error", node);
      break;
    case IAP_FIRMWARE_SAME:
      sprintf(response->data.message, "%s Version Same", node);
      break;
    case IAP_CRC_INVALID:
      sprintf(response->data.message, "%s CRC Invalid", node);
      break;
    case IAP_CANBUS_FAILED:
      sprintf(response->data.message, "%s Canbus Failed", node);
      break;
    case IAP_FOTA_ERROR:
      sprintf(response->data.message, "%s FOTA Error", node);
      break;
    case IAP_FOTA_SUCCESS:
      FW_MakeResponseIAP(response->data.message, node);

      response->data.res_code = RESPONSE_STATUS_OK;
      break;
    default:
      break;
    }

    /* Send Response */
    RPT_ResponseCapture(response);
    valid = 1;

    /* Reset after FOTA */
    EEPROM_FotaType(EE_CMD_W, 0);
    EEPROM_FotaVersion(EE_CMD_W, 0);
    *(uint32_t *)IAP_RESPONSE_ADDR = 0;
  }

  return valid;
}

/* Private functions implementation
 * --------------------------------------------*/
static void FW_MakeResponseIAP(char *message, char *node) {
  uint32_t tick;
  uint16_t vNew = VCU_VERSION;
  uint16_t vOld = FOTA.VERSION;

  if (FOTA.TYPE == IAP_HMI) {
    tick = _GetTickMS();
    do {
      vNew = HMI1.d.version;
      _DelayMS(100);
    } while (!vNew && (_GetTickMS() - tick < HMI_FOTA_MS));

    /* Handle empty firmware */
    if (vOld == 0xFFFF)
      vOld = 0x0000;
  }

  if (vNew && (vOld != vNew))
    sprintf(message, "%s Upgraded v.%d -> v.%d", node, vOld, vNew);
  else
    sprintf(message, "%s Success", node);
}

static uint8_t FW_ValidResponseIAP(void) {
  uint8_t valid = 1;

  switch (*(uint32_t *)IAP_RESPONSE_ADDR) {
  case IAP_SIMCOM_TIMEOUT:
    break;
  case IAP_DOWNLOAD_ERROR:
    break;
  case IAP_FIRMWARE_SAME:
    break;
  case IAP_CRC_INVALID:
    break;
  case IAP_CANBUS_FAILED:
    break;
  case IAP_FOTA_ERROR:
    break;
  case IAP_FOTA_SUCCESS:
    break;
  default:
    valid = 0;
    break;
  }

  return valid;
}
