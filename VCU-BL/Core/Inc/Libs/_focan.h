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
#define BLK_SIZE                   (uint16_t) (256*5)
#define FOCAN_RETRY                 (uint8_t) 5

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_GetChecksum(uint32_t *checksum);
uint8_t FOCAN_SetProgress(IAP_TYPE type, uint8_t percent);
uint8_t FOCAN_DownloadHook(uint32_t address, uint32_t *data);
uint8_t FOCAN_DownloadFlash(uint8_t *ptr, uint32_t size, uint32_t offset, uint32_t total_size);

#endif /* INC_LIBS__FOCAN_H_ */
