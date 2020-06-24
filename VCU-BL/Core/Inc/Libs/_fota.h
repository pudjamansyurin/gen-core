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

/* Public functions implementation --------------------------------------------*/
SIMCOM_RESULT FOTA_BearerInitialize(void);
SIMCOM_RESULT FOTA_GetChecksum(at_ftp_t *setFTP, uint32_t *checksum);
SIMCOM_RESULT FOTA_DownloadAndInstall(at_ftp_t *setFTP, uint32_t *len);
uint8_t FOTA_CompareChecksum(uint32_t checksum, uint32_t len, uint32_t address);

void FOTA_Reboot(void);
uint8_t FOTA_ValidImage(uint32_t address);
uint8_t FOTA_InProgressDFU(void);
void FOTA_JumpToApplication(void);
SIMCOM_RESULT FOTA_Upgrade(void);
#endif /* INC_LIBS__FOTA_H_ */
