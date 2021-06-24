/*
 * _firmware.h
 *
 *  Created on: 29 Jun 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__FIRMWARE_H_
#define INC_LIBS__FIRMWARE_H_

/* Includes
 * --------------------------------------------*/
#include "App/_command.h"

/* Public functions prototype
 * --------------------------------------------*/
bool FW_EnterModeIAP(IAP_TYPE type, char *message);
bool FW_PostFota(response_t *r);

#endif /* INC_LIBS__FIRMWARE_H_ */
