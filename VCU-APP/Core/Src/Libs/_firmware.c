/*
 * _firmware.c
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_firmware.h"
#include "Libs/_eeprom.h"

/* Private functions prototypes -----------------------------------------------*/
static void FW_MakeResponseIAP(char *message, char *node, uint16_t *hmi_version);
static uint8_t FW_ValidResponseIAP(void);

/* Public functions implementation --------------------------------------------*/
uint8_t FW_EnterModeIAP(IAP_TYPE type, char *message, uint16_t *bat, uint16_t *hmi_version) {
	if (*bat < FOTA_MIN_VOLTAGE) {
		sprintf(message, "Battery %u mV (-%u mV)", *bat, FOTA_MIN_VOLTAGE - *bat);
		return 0;
	}

	//  if (type == IAP_HMI && *hmi_version == 0) {
	//    sprintf(message, "HMI not connected");
	//    return 0;
	//  }

	/* Retain FOTA */
	EEPROM_FotaType(EE_CMD_W, type);
	EEPROM_FotaVersion(EE_CMD_W, (type == IAP_HMI) ? *hmi_version : VCU_VERSION);

	/* Set flag to SRAM */
	*(uint32_t*) IAP_FLAG_ADDR = IAP_FLAG;

	/* Reset */
	HAL_NVIC_SystemReset();

	/* Never reached if FOTA executed */
	return 0;
}

uint8_t FW_PostFota(response_t *response, uint32_t *unit_id, uint16_t *hmi_version) {
	char node[4];
	uint8_t valid = 0;

	if (FW_ValidResponseIAP()) {
		sprintf(node, FOTA.TYPE == IAP_HMI ? "HMI":"VCU");

		// set default value
		response->data.code = CMD_CODE_FOTA;
		response->data.sub_code = FOTA.TYPE == IAP_HMI ? CMD_FOTA_HMI : CMD_FOTA_VCU;
		response->data.res_code = RESPONSE_STATUS_ERROR;
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
			case IAP_FOTA_ERROR:
				sprintf(response->data.message, "%s FOTA Error", node);
				break;
			case IAP_FOTA_SUCCESS:
				FW_MakeResponseIAP(response->data.message, node, hmi_version);

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
		*(uint32_t*) IAP_RESPONSE_ADDR = 0;
	}

	return valid;
}

/* Private functions implementation --------------------------------------------*/
static void FW_MakeResponseIAP(char *message, char *node, uint16_t *hmi_version) {
	uint32_t tick;
	uint16_t vNew = VCU_VERSION;
	uint16_t vOld = FOTA.VERSION;

	if (FOTA.TYPE == IAP_HMI) {
		tick = _GetTickMS();
		do {
			vNew = *hmi_version;
			_DelayMS(100);
		} while (!vNew && (_GetTickMS() - tick < COMMAND_HMI_FOTA_TIMEOUT));

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
