/*
 * _focan.h
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

#ifndef INC_LIBS__FOCAN_H_
#define INC_LIBS__FOCAN_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_fota.h"

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_EnterModeIAP(uint32_t node, uint32_t timeout);
uint8_t FOCAN_GetChecksum(uint32_t *checksum, uint32_t timeout);

#endif /* INC_LIBS__FOCAN_H_ */
