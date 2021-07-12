/*
 * iap.c
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */

/* Includes
 * --------------------------------------------*/
#include "App/iap.h"

#include "Libs/eeprom.h"
#if (APP)
#include "App/reporter.h"
#include "Nodes/HMI1.h"
#else
#include "Drivers/flasher.h"
#endif

/* Private constants
 * --------------------------------------------*/
#if (APP)
#define HMI_FOTA_MS ((uint32_t)20000)
#endif

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint32_t flag;
  uint16_t version;
  IAP_TYPE type;
} iap_t;

/* Private variables
 * --------------------------------------------*/
static iap_t IAP = {0};

/* Private functions prototypes
 * --------------------------------------------*/
#if (APP)
static void MakeResponse(char *message, const char *node);
#endif

/* Public functions implementation
 * --------------------------------------------*/
void IAP_Init(void) {
  IAP_EE_Type(NULL);
  IAP_EE_Version(NULL);
}

#if (APP)
bool IAP_ValidResponse(void) {
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
    case IRESP_CAN_FAILED:
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

bool IAP_EnterMode(IAP_TYPE type) {
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

void IAP_CaptureResponse(response_t *r) {
  char node[4];

  sprintf(node, IAP_IO_Type() == ITYPE_HMI ? "HMI" : "VCU");

  // set default value
  r->header.code = CMDC_FOTA;
  r->header.sub_code = IAP_IO_Type() == ITYPE_HMI ? CMD_FOTA_HMI : CMD_FOTA_VCU;
  r->data.res_code = CMDR_ERROR;
  sprintf(r->data.message, "%s Failed", node);

  // check fota response
  switch (*(uint32_t *)IAP_RESP_ADDR) {
    case IRESP_BATTERY_LOW:
      sprintf(r->data.message, "%s Battery Low", node);
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
    case IRESP_CAN_FAILED:
      sprintf(r->data.message, "%s CAN-bus Failed", node);
      break;
    case IRESP_FOTA_ERROR:
      sprintf(r->data.message, "%s FOTA Error", node);
      break;
    case IRESP_FOTA_SUCCESS:
      MakeResponse(r->data.message, node);
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
#else
void IAP_SetFlag(void) {
  IAP_FLAG flag = IFLAG_EEPROM;
  IAP_EE_Flag(&flag);
}

void IAP_ResetFlag(void) {
  IAP_FLAG flag = IFLAG_RESET;
  IAP_EE_Flag(&flag);
}

uint8_t IAP_InProgress(void) { return (IAP.flag == IFLAG_EEPROM); }

void IAP_SetAppMeta(uint32_t offset, uint32_t data) {
  FLASHER_WriteAppArea((uint8_t *)&data, sizeof(uint32_t), offset);
}

void IAP_SetBootMeta(uint32_t offset, uint32_t data) {
  FLASHER_WriteBootArea((uint8_t *)&data, sizeof(uint32_t), offset);
}
#endif

uint32_t IAP_GetBootMeta(uint32_t offset) {
  return (*(__IO uint32_t *)(BL_START_ADDR + offset));
}

uint8_t IAP_EE_Type(IAP_TYPE *src) {
  void *dst = &IAP.type;
  uint8_t ok;

  ok = EE_Cmd(VA_IAP_TYPE, src, dst);

  if (!(IAP.type == ITYPE_VCU || IAP.type == ITYPE_HMI)) IAP.type = ITYPE_VCU;

  return ok;
}

uint8_t IAP_EE_Flag(IAP_FLAG *src) {
  void *dst = &IAP.flag;

  return EE_Cmd(VA_IAP_FLAG, src, dst);
}

uint8_t IAP_EE_Version(uint16_t *src) {
  void *dst = &IAP.version;

  return EE_Cmd(VA_IAP_VERSION, src, dst);
}

IAP_TYPE IAP_IO_Type(void) { return IAP.type; }

uint16_t IAP_IO_Version(void) { return IAP.version; }

/* Private functions implementation
 * --------------------------------------------*/
#if (APP)
static void MakeResponse(char *message, const char *node) {
  uint16_t vNew = VCU_VERSION, vOld = IAP_IO_Version();
  uint32_t tick;

  if (IAP_IO_Type() == ITYPE_HMI) {
    tick = tickMs();
    do {
      vNew = HMI1.d.version;
      delayMs(100);
    } while (!vNew && tickIn(tick, HMI_FOTA_MS));

    /* Handle empty firmware */
    if (vOld == 0xFFFF) vOld = 0x0000;
  }

  if (vNew && (vOld != vNew))
    sprintf(message, "%s Upgraded v.%d -> v.%d", node, vOld, vNew);
  else
    sprintf(message, "%s Success", node);
}
#endif
