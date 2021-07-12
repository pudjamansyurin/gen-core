/*
 * _fota.h
 *
 *  Created on: 18 Jun 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__FOTA_H_
#define INC_APP__FOTA_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/_simcom.h"
#include "App/_iap.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t FOTA_Upgrade(IAP_TYPE type);
void FOTA_JumpToApp(void);
void FOTA_Reboot(void);
uint8_t FOTA_NeedBackup(void);
uint8_t FOTA_ValidImage(uint32_t address);

#endif /* INC_APP__FOTA_H_ */
