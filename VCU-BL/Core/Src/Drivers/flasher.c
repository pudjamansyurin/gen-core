/*
 * flasher.c
 *
 *  Created on: 12 Jun 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/flasher.h"

#include "App/fota.h"

/* Private constants
 * --------------------------------------------*/
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0 \
  ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1 \
  ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2 \
  ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3 \
  ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4 \
  ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5 \
  ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6 \
  ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7 \
  ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8 \
  ((uint32_t)0x08080000) /* Base address of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9 \
  ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10 \
  ((uint32_t)0x080C0000) /* Base address of Sector10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11 \
  ((uint32_t)0x080E0000) /* Base address of Sector11, 128 Kbytes */
#define ADDR_FLASH_SECTOR_12 \
  ((uint32_t)0x08100000) /* Base address of Sector12, 128 Kbytes */
#define ADDR_FLASH_SECTOR_13 \
  ((uint32_t)0x08120000) /* Base address of Sector13, 128 Kbytes */
#define ADDR_FLASH_SECTOR_14 \
  ((uint32_t)0x08140000) /* Base address of Sector14, 128 Kbytes */
#define ADDR_FLASH_SECTOR_15 \
  ((uint32_t)0x08160000) /* Base address of Sector15, 128 Kbytes */

/* Private functions prototype
 * --------------------------------------------*/
static void ClearErrors(void);
static uint8_t WriteByte(uint8_t *ptr, uint32_t size, uint32_t address,
                         uint32_t end);
static uint8_t Erase(uint32_t FirstSector, uint32_t NbOfSectors);
static uint32_t GetSector(uint32_t Address);
static uint8_t WriteBkpArea(uint8_t *ptr, uint32_t size, uint32_t offset);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t FLASH_EraseBkpArea(void) {
  uint32_t FirstSector = 0, NbOfSectors = 0;

  /* Get the 1st sector to erase */
  FirstSector = GetSector(BKP_START_ADDR);
  /* Get the number of sector to erase from 1st sector*/
  NbOfSectors = GetSector(BKP_END_ADDR) - FirstSector + 1;
  // Erase
  return Erase(FirstSector, NbOfSectors);
}

uint8_t FLASH_EraseAppArea(void) {
  uint32_t FirstSector = 0, NbOfSectors = 0;

  /* Get the 1st sector to erase */
  FirstSector = GetSector(APP_START_ADDR);
  /* Get the number of sector to erase from 1st sector*/
  NbOfSectors = GetSector(APP_END_ADDR) - FirstSector + 1;
  // Erase
  return Erase(FirstSector, NbOfSectors);
}

uint8_t FLASH_WriteAppArea(uint8_t *ptr, uint32_t size, uint32_t offset) {
  uint32_t address = APP_START_ADDR + offset;

  return WriteByte(ptr, size, address, APP_END_ADDR);
}

uint8_t FLASH_WriteBootArea(uint8_t *ptr, uint32_t size, uint32_t offset) {
  uint32_t address = BL_START_ADDR + offset;

  return WriteByte(ptr, size, address, BL_END_ADDR);
}

uint8_t FLASH_BackupApp(void) {
  uint8_t *ptr = (uint8_t *)APP_START_ADDR;
  uint8_t p = 1;

  // Move to backup area (if not yet)
  if (FOTA_NeedBackup()) {
    p = FLASH_EraseBkpArea();

    if (p) p = WriteBkpArea(ptr, APP_MAX_SIZE, 0);
  }

  // Erase APP area
  if (p) p = FLASH_EraseAppArea();

  return p;
}

uint8_t FLASH_RestoreApp(void) {
  uint8_t *ptr = (uint8_t *)BKP_START_ADDR;
  uint8_t p;

  p = FLASH_EraseAppArea();

  if (p) p = FLASH_WriteAppArea(ptr, APP_MAX_SIZE, 0);

  return p;
}

/* Private functions implementation
 * --------------------------------------------*/
static void ClearErrors(void) {
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                         FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |
                         FLASH_FLAG_PGSERR);
}

static uint8_t WriteByte(uint8_t *ptr, uint32_t size, uint32_t address,
                         uint32_t end) {
  uint32_t *ptr32 = (uint32_t *)ptr;
  uint32_t errors = 0;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();
  ClearErrors();

  /* Writing...... */
  while (size && address <= end) {
    errors += (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *ptr32++) !=
               HAL_OK);

    address += 4;
    size -= 4;
  }

  /* Indicator */
  if (errors)
    printf("FLASHER:Program = ERROR:%lu\n", errors);
  else
    printf("FLASHER:Program = OK\n");

  /* Lock the Flash to disable the flash control register access (recommended
   to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return (errors == 0);
}

static uint8_t Erase(uint32_t FirstSector, uint32_t NbOfSectors) {
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t SectorError = 0;
  uint8_t p;

  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Sector = FirstSector;
  EraseInitStruct.NbSectors = NbOfSectors;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();
  ClearErrors();

  /* Erasing......... */
  p = (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK);

  /* Handle error */
  if (!p) {
    /*
     Error occurred while sector erase.
     User can add here some code to deal with this error.
     SectorError will contain the faulty sector and then to know the code error
     on this sector, user can call function 'HAL_FLASH_GetError()'
     HAL_FLASH_ERROR_NONE         0x00000000U    !< No error
     HAL_FLASH_ERROR_RD           0x00000001U    !< Read Protection error
     HAL_FLASH_ERROR_PGS          0x00000002U    !< Programming Sequence error
     HAL_FLASH_ERROR_PGP          0x00000004U    !< Programming Parallelism
     error HAL_FLASH_ERROR_PGA          0x00000008U    !< Programming Alignment
     error HAL_FLASH_ERROR_WRP          0x00000010U    !< Write protection error
     HAL_FLASH_ERROR_OPERATION    0x00000020U    !< Operation Error
     */
    printf("FLASHER:Erase = ERROR:0x%08X\n",
           (unsigned int)HAL_FLASH_GetError());
  } else
    printf("FLASHER:Erase = OK\n");

  /* Note: If an erase operation in Flash memory also concerns data in the data
   or instruction cache, you have to make sure that these data are rewritten
   before they are accessed during code execution. If this cannot be done
   safely, it is recommended to flush the caches by setting the
   DCRST and ICRST bits in the FLASH_CR register. */
  __HAL_FLASH_DATA_CACHE_DISABLE();
  __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

  __HAL_FLASH_DATA_CACHE_RESET();
  __HAL_FLASH_INSTRUCTION_CACHE_RESET();

  __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  __HAL_FLASH_DATA_CACHE_ENABLE();

  /* Lock the Flash to disable the flash control register access (recommended
   to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return p;
}

/**
 * @brief  Gets the sector of a given address
 * @param  None
 * @retval The sector of a given address
 */
static uint32_t GetSector(uint32_t Address) {
  uint32_t sector = 0;

  if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
    sector = FLASH_SECTOR_0;
  } else if ((Address < ADDR_FLASH_SECTOR_2) &&
             (Address >= ADDR_FLASH_SECTOR_1)) {
    sector = FLASH_SECTOR_1;
  } else if ((Address < ADDR_FLASH_SECTOR_3) &&
             (Address >= ADDR_FLASH_SECTOR_2)) {
    sector = FLASH_SECTOR_2;
  } else if ((Address < ADDR_FLASH_SECTOR_4) &&
             (Address >= ADDR_FLASH_SECTOR_3)) {
    sector = FLASH_SECTOR_3;
  } else if ((Address < ADDR_FLASH_SECTOR_5) &&
             (Address >= ADDR_FLASH_SECTOR_4)) {
    sector = FLASH_SECTOR_4;
  } else if ((Address < ADDR_FLASH_SECTOR_6) &&
             (Address >= ADDR_FLASH_SECTOR_5)) {
    sector = FLASH_SECTOR_5;
  } else if ((Address < ADDR_FLASH_SECTOR_7) &&
             (Address >= ADDR_FLASH_SECTOR_6)) {
    sector = FLASH_SECTOR_6;
  } else if ((Address < ADDR_FLASH_SECTOR_8) &&
             (Address >= ADDR_FLASH_SECTOR_7)) {
    sector = FLASH_SECTOR_7;
  } else if ((Address < ADDR_FLASH_SECTOR_9) &&
             (Address >= ADDR_FLASH_SECTOR_8)) {
    sector = FLASH_SECTOR_8;
  } else if ((Address < ADDR_FLASH_SECTOR_10) &&
             (Address >= ADDR_FLASH_SECTOR_9)) {
    sector = FLASH_SECTOR_9;
  } else if ((Address < ADDR_FLASH_SECTOR_11) &&
             (Address >= ADDR_FLASH_SECTOR_10)) {
    sector = FLASH_SECTOR_10;
  } else if ((Address < ADDR_FLASH_SECTOR_12) &&
             (Address >= ADDR_FLASH_SECTOR_11)) {
    sector = FLASH_SECTOR_11;
  } else if ((Address < ADDR_FLASH_SECTOR_13) &&
             (Address >= ADDR_FLASH_SECTOR_12)) {
    sector = FLASH_SECTOR_12;
  } else if ((Address < ADDR_FLASH_SECTOR_14) &&
             (Address >= ADDR_FLASH_SECTOR_13)) {
    sector = FLASH_SECTOR_13;
  } else if ((Address < ADDR_FLASH_SECTOR_15) &&
             (Address >= ADDR_FLASH_SECTOR_14)) {
    sector = FLASH_SECTOR_14;
  } else { /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_14) */
    sector = FLASH_SECTOR_15;
  }

  return sector;
}

static uint8_t WriteBkpArea(uint8_t *ptr, uint32_t size, uint32_t offset) {
  uint32_t address = BKP_START_ADDR + offset;

  return WriteByte(ptr, size, address, BKP_END_ADDR);
}
