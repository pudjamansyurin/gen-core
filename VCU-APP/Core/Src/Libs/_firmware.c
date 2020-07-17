/*
 * _firmware.c
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_firmware.h"
#include "Libs/_eeprom.h"
#include "DMA/_dma_battery.h"
#include "Nodes/HMI1.h"

/* External variables ---------------------------------------------------------*/
extern uint16_t BACKUP_VOLTAGE;
extern uint16_t FOTA_VERSION;
extern IAP_TYPE FOTA_TYPE;
extern hmi1_t HMI1;
extern osMessageQueueId_t ResponseQueueHandle;

/* Public variables -----------------------------------------------------------*/
uint16_t *HMI_VERSION = &(HMI1.d.device[HMI1_DEV_LEFT].version);

/* Public functions implementation --------------------------------------------*/
uint8_t FW_EnterModeIAP(IAP_TYPE type, char *message) {
    uint16_t version = VCU_VERSION;

    if (BACKUP_VOLTAGE > FOTA_MIN_VOLTAGE) {
        if (type == IAP_HMI) {
            if (*HMI_VERSION == 0) {
                /* HMI device not connected */
                sprintf(message, "HMI Device not connected");

                return 0;
            } else {
                version = *HMI_VERSION;
            }
        }

        /* Retain FOTA */
        EEPROM_FotaType(EE_CMD_W, type);
        EEPROM_FotaVersion(EE_CMD_W, version);

        /* Set flag to SRAM */
        *(uint32_t*) IAP_FLAG_ADDR = IAP_FLAG;

        /* Reset */
        HAL_NVIC_SystemReset();

    } else {
        /* Battery is low */
        sprintf(message, "Battery %u mV (< %u mV)",
                BACKUP_VOLTAGE, FOTA_MIN_VOLTAGE);
    }
    /* Never reached if FOTA executed */
    return 0;
}

void FW_PostFota(response_t *response) {
    uint16_t versionNew = VCU_VERSION;
    uint16_t versionOld = FOTA_VERSION;
    char node[4] = "VCU";

    if (FOTA_TYPE) {
        // decide the node
        if (FOTA_TYPE == IAP_HMI) {
            sprintf(node, "HMI");
            versionNew = *HMI_VERSION;
            /* Handle empty firmware */
            if (versionOld == 0xFFFF) {
                versionOld = 0x0000;
            }
        }

        // set default value
        response->data.code = RESPONSE_STATUS_ERROR;
        sprintf(response->data.message, "%s IAP_FAILED", node);

        // check fota response
        switch (*(uint32_t*) IAP_RESPONSE_ADDR) {
            case IAP_SIMCOM_TIMEOUT:
                sprintf(response->data.message, "%s IAP_SIMCOM_TIMEOUT", node);
                break;
            case IAP_DOWNLOAD_ERROR:
                sprintf(response->data.message, "%s IAP_DOWNLOAD_ERROR", node);
                break;
            case IAP_FIRMWARE_SAME:
                sprintf(response->data.message, "%s IAP_FIRMWARE_SAME", node);
                break;
            case IAP_CHECKSUM_INVALID:
                sprintf(response->data.message, "%s IAP_CHECKSUM_INVALID", node);
                break;
            case IAP_CANBUS_FAILED:
                sprintf(response->data.message, "%s IAP_CANBUS_FAILED", node);
                break;
            case IAP_DFU_ERROR:
                sprintf(response->data.message, "%s IAP_DFU_ERROR", node);
                break;
            case IAP_DFU_SUCCESS:
                sprintf(response->data.message,
                        "%s IAP_DFU_SUCCESS v%d.%d -> v%d.%d",
                        node,
                        _R8(versionOld, 8),
                        _R8(versionOld, 0),
                        _R8(versionNew, 8),
                        _R8(versionNew, 0));

                response->data.code = RESPONSE_STATUS_OK;
                break;
            default:
                break;
        }

        /* Send Response */
        Response_Capture(response);
        osMessageQueuePut(ResponseQueueHandle, response, 0U, 0U);

        /* Reset after FOTA */
        EEPROM_FotaVersion(EE_CMD_W, 0);
        EEPROM_FotaType(EE_CMD_W, 0);
    }
}
