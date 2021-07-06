/*
 * _firmware.h
 *
 *  Created on: 29 Jun 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__FIRMWARE_H_
#define INC_APP__FIRMWARE_H_

/* Includes
 * --------------------------------------------*/
#include "App/_command.h"

/* Public functions prototype
 * --------------------------------------------*/
bool FW_ValidResponseIAP(void);
bool FW_EnterModeIAP(IAP_TYPE type, char *message);
void FW_CaptureResponseIAP(response_t *r);

#endif /* INC_APP__FIRMWARE_H_ */
