/*
 * _firmware.c
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_firmware.h"
#include "Libs/_eeprom.h"

/* External variables ---------------------------------------------------------*/
extern osMessageQueueId_t ResponseQueueHandle;

/* Public functions implementation --------------------------------------------*/
uint8_t FW_EnterModeIAP(IAP_TYPE type, char *message, uint16_t *bat, uint16_t *hmi_version) {
  uint16_t version = VCU_VERSION;

  if (*bat < FOTA_MIN_VOLTAGE)
    sprintf(message, "Battery %u mV (< %u mV)",
        *bat, FOTA_MIN_VOLTAGE - *bat);
  else {
    if (type == IAP_HMI) {
      if (*hmi_version == 0) {
        /* HMI device not connected */
        sprintf(message, "HMI Device not connected");

        return 0;
      } else
        version = *hmi_version;
    }

    /* Retain FOTA */
    EEPROM_FotaType(EE_CMD_W, type);
    EEPROM_FotaVersion(EE_CMD_W, version);

    /* Set flag to SRAM */
    *(uint32_t*) IAP_FLAG_ADDR = IAP_FLAG;

    /* Reset */
    HAL_NVIC_SystemReset();
  }
  /* Never reached if FOTA executed */
  return 0;
}

void FW_PostFota(response_t *response, uint32_t *unit_id, uint16_t *hmi_version) {
  char node[4] = "VCU";

  if (FW_ValidResponseIAP()) {
    if (FOTA_TYPE == IAP_HMI)
      sprintf(node, "HMI");

    // set default value
    response->data.code = RESPONSE_STATUS_ERROR;
    sprintf(response->data.message, "%s Failed", node);

    // check fota response
    switch (*(uint32_t*) IAP_RESPONSE_ADDR) {
      case IAP_SIMCOM_TIMEOUT:
        sprintf(response->data.message, "%s Internet Timeout", node);
        break;
      case IAP_DOWNLOAD_ERROR:
        sprintf(response->data.message, "%s Download Error", node);
        break;
      case IAP_FIRMWARE_SAME:
        sprintf(response->data.message, "%s Version Same", node);
        break;
      case IAP_CHECKSUM_INVALID:
        sprintf(response->data.message, "%s Checksum Invalid", node);
        break;
      case IAP_CANBUS_FAILED:
        sprintf(response->data.message, "%s Canbus Failed", node);
        break;
      case IAP_DFU_ERROR:
        sprintf(response->data.message, "%s DFU Error", node);
        break;
      case IAP_DFU_SUCCESS:
        sprintf(response->data.message, "%s Success", node);
        FW_MakeResponseIAP(response->data.message, hmi_version);

        response->data.code = RESPONSE_STATUS_OK;
        break;
      default:
        break;
    }

    /* Send Response */
    Response_Capture(response, unit_id);
    osMessageQueuePut(ResponseQueueHandle, response, 0U, 0U);

    /* Reset after FOTA */
    EEPROM_FotaVersion(EE_CMD_W, 0);
    EEPROM_FotaType(EE_CMD_W, 0);
  }
}

void FW_MakeResponseIAP(char *message, uint16_t *hmi_version) {
  uint32_t tick;
  uint16_t versionNew = VCU_VERSION;
  uint16_t versionOld = FOTA_VERSION;

  if (FOTA_TYPE == IAP_HMI) {
    tick = _GetTickMS();
    do {
      versionNew = *hmi_version;
      _DelayMS(100);
    } while (!versionNew && (_GetTickMS() - tick > 5000));

    /* Handle empty firmware */
    if (versionOld == 0xFFFF)
      versionOld = 0x0000;
  }

  if (versionNew && (versionOld != versionNew)) {
    sprintf(message,
        "%s v%d.%d (> v%d.%d)",
        message,
        _R8(versionOld, 8),
        _R8(versionOld, 0),
        _R8(versionNew, 8),
        _R8(versionNew, 0));
  }
}

uint8_t FW_ValidResponseIAP(void) {
  uint8_t valid = 1;

  switch (*(uint32_t*) IAP_RESPONSE_ADDR) {
    case IAP_SIMCOM_TIMEOUT:
      break;
    case IAP_DOWNLOAD_ERROR:
      break;
    case IAP_FIRMWARE_SAME:
      break;
    case IAP_CHECKSUM_INVALID:
      break;
    case IAP_CANBUS_FAILED:
      break;
    case IAP_DFU_ERROR:
      break;
    case IAP_DFU_SUCCESS:
      break;
    default:
      valid = 0;
      break;
  }

  return valid;
}
