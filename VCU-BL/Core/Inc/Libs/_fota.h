/*
 * _fota.h
 *
 *  Created on: 18 Jun 2020
 *      Author: geni
 */

#ifndef INC_LIBS__FOTA_H_
#define INC_LIBS__FOTA_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_simcom.h"
#include "Libs/_at.h"

/* Public functions implementation --------------------------------------------*/
uint8_t FOTA_Upgrade(IAP_TYPE type);
uint8_t FOTA_DownloadChecksum(at_ftpget_t *ftpGET, uint32_t *checksum);
uint8_t FOTA_DownloadFirmware(at_ftp_t *ftp, at_ftpget_t *ftpGET, uint32_t *len, IAP_TYPE type, uint32_t timeout);
uint8_t FOTA_ValidateChecksum(uint32_t checksum, uint32_t len, uint32_t address);
uint8_t FOTA_ValidImage(uint32_t address);
void FOTA_JumpToApplication(void);
void FOTA_Reboot(IAP_TYPE type);
void FOTA_GetChecksum(uint32_t *checksum);
void FOTA_GlueInfo32(uint32_t offset, uint32_t *data);
uint8_t FOTA_NeedBackup(void);
uint8_t FOTA_InProgressDFU(void);
void FOTA_SetDFU(void);
void FOTA_ResetDFU(void);

#endif /* INC_LIBS__FOTA_H_ */
