/*
 * flasher.h
 *
 *  Created on: 12 Jun 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__FLASHER_H_
#define INC_DRIVERS__FLASHER_H_

/* Includes
 * --------------------------------------------*/
#include "App/util.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t FLASH_EraseBkpArea(void);
uint8_t FLASH_EraseAppArea(void);
uint8_t FLASH_WriteAppArea(uint8_t *ptr, uint32_t size, uint32_t offset);
uint8_t FLASH_WriteBootArea(uint8_t *ptr, uint32_t size, uint32_t offset);
uint8_t FLASH_BackupApp(void);
uint8_t FLASH_RestoreApp(void);

#endif /* INC_DRIVERS__FLASHER_H_ */
