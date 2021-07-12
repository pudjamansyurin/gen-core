/*
 * focan.h
 *
 *  Created on: 29 Jun 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__FOCAN_H_
#define INC_APP__FOCAN_H_

/* Includes
 * --------------------------------------------*/
#include "App/fota.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t FOCAN_GetCRC(uint32_t *crc);
uint8_t FOCAN_SetProgress(IAP_TYPE type, float percent);
uint8_t FOCAN_Hook(uint32_t address, uint32_t *data);
uint8_t FOCAN_Flash(uint8_t *ptr, uint32_t size, uint32_t offset, uint32_t len);

#endif /* INC_APP__FOCAN_H_ */
