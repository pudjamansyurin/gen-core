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
void FOTA_JumpToApplication(void);
void FOTA_Reboot(void);
uint8_t FOTA_NeedBackup(void);
uint8_t FOTA_ValidImage(uint32_t address);

#endif /* INC_LIBS__FOTA_H_ */
