/*
 * _firmware.h
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

#ifndef INC_LIBS__FIRMWARE_H_
#define INC_LIBS__FIRMWARE_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Libs/_reporter.h"

/* Public functions prototype ------------------------------------------------*/
uint8_t FW_EnterModeIAP(IAP_TYPE type, char *message);
void FW_PostFota(response_t *response);
void FW_MakeResponseIAP(char *message);
uint8_t FW_ValidResponseIAP(void);

#endif /* INC_LIBS__FIRMWARE_H_ */
