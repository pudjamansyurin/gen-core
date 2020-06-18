/*
 * _flash.h
 *
 *  Created on: 12 Jun 2020
 *      Author: geni
 */

#ifndef INC_DRIVERS__FLASHER_H_
#define INC_DRIVERS__FLASHER_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported macro ------------------------------------------------------------*/
#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_10
#define FLASH_USER_END_ADDR     (ADDR_FLASH_SECTOR_11  +  FLASHER_GetSectorSize(ADDR_FLASH_SECTOR_11) - 1)

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base address of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base address of Sector10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base address of Sector11, 128 Kbytes */
#define ADDR_FLASH_SECTOR_12    ((uint32_t)0x08100000) /* Base address of Sector12, 128 Kbytes */
#define ADDR_FLASH_SECTOR_13    ((uint32_t)0x08120000) /* Base address of Sector13, 128 Kbytes */
#define ADDR_FLASH_SECTOR_14    ((uint32_t)0x08140000) /* Base address of Sector14, 128 Kbytes */
#define ADDR_FLASH_SECTOR_15    ((uint32_t)0x08160000) /* Base address of Sector15, 128 Kbytes */

/* Public functions prototype ------------------------------------------------*/
uint8_t FLASHER_WriteByte(char *ptr, uint16_t size, uint32_t offset);
uint8_t FLASHER_Erase(void);
uint32_t FLASHER_GetSector(uint32_t Address);
uint32_t FLASHER_GetSectorSize(uint32_t Sector);

#endif /* INC_DRIVERS__FLASHER_H_ */
