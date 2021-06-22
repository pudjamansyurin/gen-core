/*
 * _focan.h
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

#ifndef INC_LIBS__FOCAN_H_
#define INC_LIBS__FOCAN_H_

/* Includes
 * --------------------------------------------*/
#include "App/_fota.h"

/* Exported constants
 * --------------------------------------------*/
#define BLK_SIZE ((uint16_t)(256 * 5))
#define FOCAN_RETRY ((uint8_t)5)

/* Public functions prototype
 * --------------------------------------------*/
uint8_t FOCAN_GetCRC(uint32_t *crc);
uint8_t FOCAN_SetProgress(IAP_TYPE type, float percent);
uint8_t FOCAN_DownloadHook(uint32_t address, uint32_t *data);
uint8_t FOCAN_DownloadFlash(uint8_t *ptr, uint32_t size, uint32_t offset,
                            uint32_t total_size);

#endif /* INC_LIBS__FOCAN_H_ */
