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

/* Exported macro -------------------------------------------------------------*/
#define BLK_SIZE                   (uint16_t) 256
#define FOCAN_RETRY                 (uint8_t) 5

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_EnterModeIAP(uint32_t node, uint32_t timeout);
uint8_t FOCAN_GetChecksum(uint32_t *checksum, uint32_t timeout);
uint8_t FOCAN_DownloadHook(uint32_t address, uint32_t *data, uint32_t timeout);
uint8_t FOCAN_DownloadFlash(uint8_t *ptr, uint32_t size, uint32_t offset);

#endif /* INC_LIBS__FOCAN_H_ */
