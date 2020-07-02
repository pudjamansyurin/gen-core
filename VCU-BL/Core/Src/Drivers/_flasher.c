/*
 * _flash.c
 *
 *  Created on: 12 Jun 2020
 *      Author: geni
 */

/* Includes -------------------------------------------------------------------*/
#include "Drivers/_flasher.h"
#include "Libs/_fota.h"

/* Private functions prototype ------------------------------------------------*/
static uint8_t FLASHER_WriteByte(uint8_t *ptr, uint32_t size, uint32_t address, uint32_t end);
static uint8_t FLASHER_Erase(uint32_t FirstSector, uint32_t NbOfSectors);
static uint32_t FLASHER_GetSector(uint32_t Address);
static uint32_t FLASHER_GetSectorSize(uint32_t Sector);

/* Public functions implementation ---------------------------------------------*/
static uint8_t FLASHER_WriteByte(uint8_t *ptr, uint32_t size, uint32_t address, uint32_t end) {
    uint32_t *ptr32 = (uint32_t*) ptr;
    uint32_t errors = 0;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Writing...... */
    while (size && address <= end) {
        errors += (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *ptr32++) != HAL_OK);

        address += 4;
        size -= 4;
    }

    /* Indicator */
    if (!errors) {
        LOG_StrLn("HAL_FLASH_Program = OK");
    } else {
        LOG_Str("HAL_FLASH_Program = ERROR:");
        LOG_Int(errors);
        LOG_Enter();
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

    return (errors == 0);
}

static uint8_t FLASHER_Erase(uint32_t FirstSector, uint32_t NbOfSectors) {
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
    __HAL_FLASH_CLEAR_FLAG(
            FLASH_FLAG_EOP |
            FLASH_FLAG_OPERR |
            FLASH_FLAG_WRPERR |
            FLASH_FLAG_PGAERR |
            FLASH_FLAG_PGPERR |
            FLASH_FLAG_PGSERR
            );

    /* Erasing......... */
    p = (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK);

    /* Handle error */
    if (!p) {
        /*
         Error occurred while sector erase.
         User can add here some code to deal with this error.
         SectorError will contain the faulty sector and then to know the code error on this sector,
         user can call function 'HAL_FLASH_GetError()'
         HAL_FLASH_ERROR_NONE         0x00000000U    !< No error
         HAL_FLASH_ERROR_RD           0x00000001U    !< Read Protection error
         HAL_FLASH_ERROR_PGS          0x00000002U    !< Programming Sequence error
         HAL_FLASH_ERROR_PGP          0x00000004U    !< Programming Parallelism error
         HAL_FLASH_ERROR_PGA          0x00000008U    !< Programming Alignment error
         HAL_FLASH_ERROR_WRP          0x00000010U    !< Write protection error
         HAL_FLASH_ERROR_OPERATION    0x00000020U    !< Operation Error
         */
        LOG_Str("HAL_FLASHEx_Erase = ERROR:0x");
        LOG_Hex32(HAL_FLASH_GetError());
        LOG_Enter();
    } else {
        LOG_StrLn("HAL_FLASHEx_Erase = OK");
    }

    /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
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
static uint32_t FLASHER_GetSector(uint32_t Address) {
    uint32_t sector = 0;

    if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
        sector = FLASH_SECTOR_0;
    }
    else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
        sector = FLASH_SECTOR_1;
    }
    else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
        sector = FLASH_SECTOR_2;
    }
    else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
        sector = FLASH_SECTOR_3;
    }
    else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
        sector = FLASH_SECTOR_4;
    }
    else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
        sector = FLASH_SECTOR_5;
    }
    else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
        sector = FLASH_SECTOR_6;
    }
    else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7)) {
        sector = FLASH_SECTOR_7;
    }
    else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8)) {
        sector = FLASH_SECTOR_8;
    }
    else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9)) {
        sector = FLASH_SECTOR_9;
    }
    else if ((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10)) {
        sector = FLASH_SECTOR_10;
    }
    else if ((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11)) {
        sector = FLASH_SECTOR_11;
    }
    else if ((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12)) {
        sector = FLASH_SECTOR_12;
    }
    else if ((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13)) {
        sector = FLASH_SECTOR_13;
    }
    else if ((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14)) {
        sector = FLASH_SECTOR_14;
    }
    else { /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_14) */
        sector = FLASH_SECTOR_15;
    }

    return sector;
}

/**
 * @brief  Gets sector Size
 * @param  None
 * @retval The size of a given sector
 */
static uint32_t FLASHER_GetSectorSize(uint32_t Sector) {
    uint32_t sectorsize = 0x00;

    if ((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2)
            || (Sector == FLASH_SECTOR_3)) {
        sectorsize = 16 * 1024;
    }
    else if (Sector == FLASH_SECTOR_4) {
        sectorsize = 64 * 1024;
    }
    else {
        sectorsize = 128 * 1024;
    }
    return sectorsize;
}

/* ============================================================= */
uint8_t FLASHER_EraseBkpArea(void) {
    uint32_t FirstSector = 0, NbOfSectors = 0;

    /* Get the 1st sector to erase */
    FirstSector = FLASHER_GetSector(BKP_START_ADDR);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = FLASHER_GetSector(BKP_END_ADDR) - FirstSector + 1;
    // Erase
    return FLASHER_Erase(FirstSector, NbOfSectors);
}

uint8_t FLASHER_EraseAppArea(void) {
    uint32_t FirstSector = 0, NbOfSectors = 0;

    /* Get the 1st sector to erase */
    FirstSector = FLASHER_GetSector(APP_START_ADDR);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = FLASHER_GetSector(APP_END_ADDR) - FirstSector + 1;
    // Erase
    return FLASHER_Erase(FirstSector, NbOfSectors);
}

uint8_t FLASHER_WriteBkpArea(uint8_t *ptr, uint32_t size, uint32_t offset) {
    uint32_t address = BKP_START_ADDR + offset;

    return FLASHER_WriteByte(ptr, size, address, BKP_END_ADDR);
}

uint8_t FLASHER_WriteAppArea(uint8_t *ptr, uint32_t size, uint32_t offset) {
    uint32_t address = APP_START_ADDR + offset;

    return FLASHER_WriteByte(ptr, size, address, APP_END_ADDR);
}

uint8_t FLASHER_BackupApp(void) {
    uint8_t *ptr = (uint8_t*) APP_START_ADDR;
    uint8_t p;

    p = FOTA_NeedBackup();

    if (p) {
        p = FLASHER_EraseBkpArea();
    }

    if (p) {
        p = FLASHER_WriteBkpArea(ptr, APP_MAX_SIZE, 0);
    }

    return p;
}

uint8_t FLASHER_RestoreApp(void) {
    uint8_t *ptr = (uint8_t*) BKP_START_ADDR;
    uint8_t p;

    p = FLASHER_EraseAppArea();

    if (p) {
        p = FLASHER_WriteAppArea(ptr, APP_MAX_SIZE, 0);
    }

    return p;
}

