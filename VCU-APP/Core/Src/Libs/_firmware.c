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

/* External variables ---------------------------------------------------------*/
extern uint16_t BACKUP_VOLTAGE;
extern uint16_t VCU_FW_VERSION;
extern osMessageQueueId_t ResponseQueueHandle;

/* Public functions implementation --------------------------------------------*/
uint8_t FW_EnterModeIAP(IAP_TYPE type) {
    if (BACKUP_VOLTAGE > FOTA_MIN_VOLTAGE) {
        /* Set flag to SRAM */
        *(uint32_t*) IAP_FLAG_ADDR = IAP_FLAG;
        *(uint32_t*) IAP_TYPE_ADDR = type;
        /* Retain VCU version */
        if (type == IAP_TYPE_VCU) {
            EEPROM_FirmwareVersion(EE_CMD_W, VCU_VERSION, IAP_TYPE_VCU);
        }
        /* Reset */
        HAL_NVIC_SystemReset();
    }
    /* Never reached if FOTA executed */
    return 0;
}

void FW_PostFota(response_t *response) {
    if (VCU_FW_VERSION) {
        /* Success or Failed*/
        if (VCU_FW_VERSION != VCU_VERSION) {
            sprintf(response->data.message,
                    "FW upgraded v%d.%d to v%d.%d",
                    _R8(VCU_FW_VERSION, 8),
                    _R8(VCU_FW_VERSION, 0),
                    _R8(VCU_VERSION, 8),
                    _R8(VCU_VERSION, 0));
            response->data.code = RESPONSE_STATUS_OK;
        } else {
            strcpy(response->data.message, "FOTA failed");
            response->data.code = RESPONSE_STATUS_ERROR;
        }

        /* Send Response */
        Response_Capture(response);
        osMessageQueuePut(ResponseQueueHandle, response, 0U, 0U);

        /* Reset after FOTA */
        EEPROM_FirmwareVersion(EE_CMD_W, 0, IAP_TYPE_VCU);
    }
}
