/*
 * _firmware.c
 *
 *  Created on: 29 Jun 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_firmware.h"

#include "App/_command.h"
#include "App/_reporter.h"
#include "Drivers/_bat.h"
#include "Libs/_eeprom.h"
#include "Nodes/HMI1.h"

/* Private functions prototypes
 * --------------------------------------------*/
static void FW_MakeResponseIAP(char *message, const char *node);

/* Public functions implementation
 * --------------------------------------------*/
bool FW_ValidResponseIAP(void) {
  uint8_t ok = 1;

  switch (*(uint32_t *)IAP_RESP_ADDR) {
    case IRESP_SIM_TIMEOUT:
      break;
    case IRESP_DOWNLOAD_ERROR:
      break;
    case IRESP_FIRMWARE_SAME:
      break;
    case IRESP_CRC_INVALID:
      break;
    case IRESP_CANBUS_FAILED:
      break;
    case IRESP_FOTA_ERROR:
      break;
    case IRESP_FOTA_SUCCESS:
      break;
    default:
      ok = 0;
      break;
  }

  return ok;
}

bool FW_EnterModeIAP(IAP_TYPE type) {
  uint16_t version = (type == ITYPE_HMI) ? HMI1.d.version : VCU_VERSION;

  /* Retain FOTA */
  IAP_EE_Type(&type);
  IAP_EE_Version(&version);

  /* Set flag to SRAM */
  *(uint32_t *)IAP_FLAG_ADDR = IFLAG_SRAM;

  /* Reset */
  HAL_NVIC_SystemReset();
  /* Never reached if FOTA executed */
  return false;
}

void FW_CaptureResponseIAP(response_t *r) {
  char node[4];

  sprintf(node, IAP.type == ITYPE_HMI ? "HMI" : "VCU");

  // set default value
  r->header.code = CMDC_FOTA;
  r->header.sub_code = IAP.type == ITYPE_HMI ? CMD_FOTA_HMI : CMD_FOTA_VCU;
  r->data.res_code = CMDR_ERROR;
  sprintf(r->data.message, "%s Failed", node);

  // check fota response
  switch (*(uint32_t *)IAP_RESP_ADDR) {
    case IRESP_BATTERY_LOW:
      sprintf(r->data.message, "%s Battery Low (-%u mV)", node,
              SIM_MIN_MV - BAT_ScanValue());
      break;
    case IRESP_SIM_TIMEOUT:
      sprintf(r->data.message, "%s Internet Timeout", node);
      break;
    case IRESP_DOWNLOAD_ERROR:
      sprintf(r->data.message, "%s Download Error", node);
      break;
    case IRESP_FIRMWARE_SAME:
      sprintf(r->data.message, "%s Version Same", node);
      break;
    case IRESP_CRC_INVALID:
      sprintf(r->data.message, "%s CRC Invalid", node);
      break;
    case IRESP_CANBUS_FAILED:
      sprintf(r->data.message, "%s Canbus Failed", node);
      break;
    case IRESP_FOTA_ERROR:
      sprintf(r->data.message, "%s FOTA Error", node);
      break;
    case IRESP_FOTA_SUCCESS:
      FW_MakeResponseIAP(r->data.message, node);
      r->data.res_code = CMDR_OK;
      break;
    default:
      break;
  }

  /* Send Response */
  RPT_ResponseCapture(r);

  /* Reset after FOTA */
  IAP_TYPE d = ITYPE_RESET;
  IAP_EE_Type(&d);

  uint16_t v = 0;
  IAP_EE_Version(&v);

  *(uint32_t *)IAP_RESP_ADDR = 0;
}

/* Private functions implementation
 * --------------------------------------------*/
static void FW_MakeResponseIAP(char *message, const char *node) {
  uint16_t vNew = VCU_VERSION, vOld = IAP.version;
  uint32_t tick;

  if (IAP.type == ITYPE_HMI) {
    tick = _GetTickMS();
    do {
      vNew = HMI1.d.version;
      _DelayMS(100);
    } while (!vNew && _TickIn(tick, HMI_FOTA_MS));

    /* Handle empty firmware */
    if (vOld == 0xFFFF) vOld = 0x0000;
  }

  if (vNew && (vOld != vNew))
    sprintf(message, "%s Upgraded v.%d -> v.%d", node, vOld, vNew);
  else
    sprintf(message, "%s Success", node);
}
