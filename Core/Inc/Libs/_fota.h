/*
 * _fota.h
 *
 *  Created on: 18 Jun 2020
 *      Author: geni
 */

#ifndef INC_LIBS__FOTA_H_
#define INC_LIBS__FOTA_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_simcom.h"
#include "Drivers/_at.h"

/* Exported macro ------------------------------------------------------------*/
#define FOTA_IN_PROGRESS         0xA1B2C3D4
#define FOTA_FLAG_ADDRESS        FLASH_USER_END_ADDR - sizeof(uint32_t)
#define FOTA_CHECKSUM_ADDRESS    FOTA_FLAG_ADDRESS - sizeof(uint32_t)

/* Public functions implementation --------------------------------------------*/
SIMCOM_RESULT FOTA_BearerInitialize(void);
SIMCOM_RESULT FOTA_GetChecksum(at_ftp_t *setFTP, uint32_t *checksum);
SIMCOM_RESULT FOTA_FirmwareToFlash(at_ftp_t *setFTP, uint32_t *len);
uint8_t FOTA_CompareChecksum(uint32_t crcRemote, uint32_t len);
uint8_t FOTA_SetInProgress(uint32_t checksum);

#endif /* INC_LIBS__FOTA_H_ */
