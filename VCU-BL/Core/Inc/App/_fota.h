/*
 * _fota.h
 *
 *  Created on: 18 Jun 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__FOTA_H_
#define INC_LIBS__FOTA_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_simcom.h"
#include "Libs/_at.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t FOTA_Upgrade(IAP_TYPE type);
uint8_t FOTA_DownloadCRC(at_ftpget_t *ftpGET, uint32_t *crc);
uint8_t FOTA_DownloadFirmware(at_ftp_t *ftp, at_ftpget_t *ftpGET, uint32_t *len,
                              IAP_TYPE type, uint32_t timeout);
uint8_t FOTA_ValidateCRC(uint32_t crc, uint32_t len, uint32_t address);
uint8_t FOTA_ValidImage(uint32_t address);
void FOTA_JumpToApplication(void);
void FOTA_Reboot(IAP_TYPE type);
void FOTA_GetCRC(uint32_t *crc);
uint8_t FOTA_NeedBackup(void);

#endif /* INC_LIBS__FOTA_H_ */
