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
uint8_t FW_EnterModeIAP(IAP_TYPE type, char *message);
uint8_t FW_PostFota(response_t *r);

#endif /* INC_LIBS__FIRMWARE_H_ */
