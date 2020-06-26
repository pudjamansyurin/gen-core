/*
 * _fota.c
 *
 *  Created on: 18 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_fota.h"
#include "Libs/_eeprom.h"
#include "Drivers/_flasher.h"
#include "Drivers/_crc.h"

/* External variables ---------------------------------------------------------*/
extern uint32_t DFU_FLAG;
extern CRC_HandleTypeDef hcrc;
extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart1;
//DMA_HandleTypeDef hdma_usart1_rx;

/* Public functions implementation --------------------------------------------*/
SIMCOM_RESULT FOTA_BearerInitialize(void) {
    SIMCOM_RESULT p;
    at_sapbr_t getBEARER, setBEARER = {
            .cmd_type = SAPBR_BEARER_OPEN,
            .status = SAPBR_CONNECTED,
            .con = {
                    .apn = NET_CON_APN,
                    .username = NET_CON_USERNAME,
                    .password = NET_CON_PASSWORD,
            },
    };

    // BEARER attach
    p = AT_BearerSettings(ATW, &setBEARER);

    // BEARER init
    if (p > 0) {
        p = AT_BearerSettings(ATR, &getBEARER);
    }

    if (p > 0 && getBEARER.status != SAPBR_CONNECTED) {
        p = SIM_RESULT_ERROR;
    }

    return p;
}

SIMCOM_RESULT FOTA_GetChecksum(at_ftp_t *setFTP, uint32_t *checksum) {
    SIMCOM_RESULT p;
    AT_FTP_STATE state;
    at_ftpget_t setFTPGET;

    // FTP Set file name
    sprintf(setFTP->file, "%s.crc", setFTP->version);
    p = AT_FtpSetFile(setFTP);

    // Open FTP Session
    if (p > 0) {
        setFTPGET.mode = FTPGET_OPEN;
        p = AT_FtpDownload(&setFTPGET);
    }

    // Read FTP File
    if (p > 0 && setFTPGET.response == FTP_READY) {
        // Initiate Download
        setFTPGET.mode = FTPGET_READ;
        setFTPGET.reqlength = 8;
        p = AT_FtpDownload(&setFTPGET);

        if (p > 0) {
            // Copy to Buffer
            *checksum = strtoul(setFTPGET.ptr, (char**) NULL, 16);

            // Indicator
            LOG_Str("FOTA:ChecksumOrigin = ");
            LOG_Hex32(*checksum);
            LOG_Enter();
        }
    }

    // Check state
    AT_FtpCurrentState(&state);
    if (state == FTP_STATE_ESTABLISHED) {
        // Close session
        Simcom_Command("AT+FTPQUIT\r", NULL, 500, 0);
    }

    return p;
}

SIMCOM_RESULT FOTA_DownloadAndInstall(at_ftp_t *setFTP, uint32_t *len) {
    SIMCOM_RESULT p;
    uint32_t timer;
    AT_FTP_STATE state;
    at_ftpget_t setFTPGET;

    // FTP Set file name
    sprintf(setFTP->file, "%s.bin", setFTP->version);
    p = AT_FtpSetFile(setFTP);

    // Get file size
    if (p > 0) {
        p = AT_FtpFileSize(setFTP);
    }

    // Open FTP Session
    if (p > 0 && setFTP->size) {
        setFTPGET.mode = FTPGET_OPEN;
        p = AT_FtpDownload(&setFTPGET);
    }

    // Read FTP File
    if (p > 0 && setFTPGET.response == FTP_READY) {
        // Prepare, start timer
        LOG_StrLn("FOTA:Start");
        timer = _GetTickMS();

        // Erase APP area
        FLASHER_EraseAppArea();

        // Copy chunk by chunk
        setFTPGET.mode = FTPGET_READ;
        setFTPGET.reqlength = 1376;
        do {
            // Initiate Download
            p = AT_FtpDownload(&setFTPGET);

            if (p > 0 && setFTPGET.cnflength) {
                // Copy to Buffer
                FLASHER_WriteAppArea((uint8_t*) setFTPGET.ptr, setFTPGET.cnflength, *len);
                *len += setFTPGET.cnflength;

                // Indicator
                _LedToggle();
                LOG_Str("FOTA:Progress = ");
                LOG_Int(*len);
                LOG_Str(" Bytes (");
                LOG_Int(*len * 100 / setFTP->size);
                LOG_StrLn("%)");
            } else {
                break;
            }
        } while (*len < setFTP->size);

        // Check, stop timer
        if (*len && *len == setFTP->size) {
            /* Glue size information to image */
            FLASHER_WriteAppArea((uint8_t*) len, sizeof(uint32_t), SIZE_OFFSET);

            LOG_Str("FOTA:End = ");
            LOG_Int(_GetTickMS() - timer);
            LOG_StrLn("ms");
        } else {
            LOG_StrLn("FOTA:Failed");
            p = SIM_RESULT_ERROR;
        }
    }

    // Check state
    AT_FtpCurrentState(&state);
    if (state == FTP_STATE_ESTABLISHED) {
        // Close session
        Simcom_Command("AT+FTPQUIT\r", NULL, 500, 0);
    }

    return p;
}

uint8_t FOTA_CompareChecksum(uint32_t checksum, uint32_t len, uint32_t address) {
    uint32_t crc = 0;
    uint8_t *addr = (uint8_t*) address;

    // Calculate CRC
    crc = CRC_Calculate8(addr, len, 1);

    // Indicator
    LOG_Str("FOTA:Checksum = ");
    if (crc == checksum) {
        LOG_StrLn("MATCH");
        /* Glue checksum information to image */
        FLASHER_WriteAppArea((uint8_t*) &crc, sizeof(uint32_t), CHECKSUM_OFFSET);
    } else {
        LOG_StrLn("NOT MATCH");
        LOG_Hex32(checksum);
        LOG_Str(" != ");
        LOG_Hex32(crc);
        LOG_Enter();
    }

    return (crc == checksum);
}

void FOTA_Reboot(void) {
    /* Clear backup area */
    FLASHER_EraseBkpArea();
    /* Reset DFU flag */
    EEPROM_FlagDFU(EE_CMD_W, 0);

    HAL_NVIC_SystemReset();
}

uint8_t FOTA_ValidImage(uint32_t address) {
    uint32_t size, checksum = 0, crc = 0;
    uint8_t ret;
    uint8_t *ptr = (uint8_t*) address;

    /* Check beginning stack pointer */
    ret = IS_VALID_SP(APP_START_ADDR);

    /* Check the size */
    if (ret) {
        /* Get the stored size information */
        size = *(uint32_t*) (address + SIZE_OFFSET);
        ret = (size < APP_MAX_SIZE );
    }

    /* Check the checksum */
    if (ret) {
        /* Get the stored checksum information */
        checksum = *(uint32_t*) (address + CHECKSUM_OFFSET);
        /* Calculate CRC */
        crc = CRC_Calculate8(ptr, size, 1);

        // Indicator
        LOG_Str("APP:Checksum = ");
        if (crc == checksum) {
            LOG_StrLn("MATCH");
        } else {
            LOG_StrLn("NOT MATCH");
            LOG_Hex32(checksum);
            LOG_Str(" != ");
            LOG_Hex32(crc);
            LOG_Enter();
        }

        ret = (checksum == crc);
    }

    return ret;
}

uint8_t FOTA_InProgressDFU(void) {
    return IS_DFU_IN_PROGRESS(DFU_FLAG);
}

void FOTA_JumpToApplication(void) {
    uint32_t appStack, appEntry;

    /* Get stack & entry pointer */
    appStack = *(__IO uint32_t*) APP_START_ADDR;
    appEntry = *(__IO uint32_t*) (APP_START_ADDR + 4);

    /* Shutdown all peripherals */
    HAL_CRC_MspDeInit(&hcrc);
    HAL_I2C_MspDeInit(&hi2c2);
    HAL_UART_MspDeInit(&huart1);
    HAL_RCC_DeInit();
    HAL_DeInit();

    /* Reset systick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /* Set stack pointer */
    __set_MSP(appStack);

    /* Jump to user ResetHandler */
    void (*jump)(void) = (void (*)(void))(appEntry);
    jump();

    /* Never reached */
    while (1)
        ;
}

SIMCOM_RESULT FOTA_Upgrade(void) {
    SIMCOM_RESULT p;
    uint32_t checksum;

    /* Set DFU */
    if (!FOTA_InProgressDFU()) {
        EEPROM_FlagDFU(EE_CMD_W, DFU_IN_PROGRESS);

        // Backup current application (if necessary)
        if (FOTA_ValidImage(APP_START_ADDR)) {
            FLASHER_BackupApp();
        }
    }
    /* Get the stored checksum information */
    checksum = *(uint32_t*) (BKP_START_ADDR + CHECKSUM_OFFSET);

    /* Download image and install */
    p = Simcom_FOTA(checksum);

    // DFU flag reset
    if (p > 0) {
        EEPROM_FlagDFU(EE_CMD_W, 0);
    }

    return p;
}
