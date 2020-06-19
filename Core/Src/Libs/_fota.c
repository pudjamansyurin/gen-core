/*
 * _fota.c
 *
 *  Created on: 18 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_fota.h"
#include "Drivers/_flasher.h"
#include "Drivers/_crc.h"

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

    // Set Default Parameter
    setFTP->id = 1;
    setFTP->size = 0;
    strcpy(setFTP->server, NET_FTP_SERVER);
    strcpy(setFTP->username, NET_FTP_USERNAME);
    strcpy(setFTP->password, NET_FTP_PASSWORD);
    sprintf(setFTP->file, "%s.crc", setFTP->version);

    // FTP Init
    p = AT_FtpInitialize(setFTP);

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
            *checksum = strtol(setFTPGET.ptr, (char**) NULL, 16);
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

SIMCOM_RESULT FOTA_FirmwareToFlash(at_ftp_t *setFTP, uint32_t *len) {
    SIMCOM_RESULT p;
    uint32_t timer;
    AT_FTP_STATE state;
    at_ftpget_t setFTPGET;

    // Set Default Parameter
    setFTP->id = 1;
    setFTP->size = 0;
    strcpy(setFTP->server, NET_FTP_SERVER);
    strcpy(setFTP->username, NET_FTP_USERNAME);
    strcpy(setFTP->password, NET_FTP_PASSWORD);
    sprintf(setFTP->file, "%s.bin", setFTP->version);

    // FTP Init
    p = AT_FtpInitialize(setFTP);

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
        FLASHER_Erase();
        timer = _GetTickMS();

        // Copy chunk by chunk
        setFTPGET.mode = FTPGET_READ;
        setFTPGET.reqlength = 1376;
        do {
            // Initiate Download
            p = AT_FtpDownload(&setFTPGET);

            if (p > 0 && setFTPGET.cnflength) {
                // Copy to Buffer
                FLASHER_WriteByte((uint8_t*) setFTPGET.ptr, setFTPGET.cnflength, *len);
                *len += setFTPGET.cnflength;

                // Indicator
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
            LOG_Str("FOTA:End = ");
            LOG_Int(_GetTickMS() - timer);
            LOG_StrLn("ms");
        } else {
            LOG_StrLn("FOTA:Failed");
            p = SIM_RESULT_ERROR;
            FLASHER_Erase();
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

uint8_t FOTA_CompareChecksum(uint32_t crcRemote, uint32_t len) {
    uint32_t crcLocal = 0;

    // Calculate CRC
    crcLocal = CRC_Calculate8((uint8_t*) FLASH_USER_START_ADDR, len, 1);

    // Indicator
    LOG_Str("FOTA:Checksum = ");
    if (crcLocal == crcRemote) {
        LOG_StrLn("MATCH");
    } else {
        LOG_StrLn("NOT MATCH");
        FLASHER_Erase();
    }

    return (crcLocal == crcRemote);
}

uint8_t FOTA_SetInProgress(uint32_t checksum) {
    uint32_t data = FOTA_IN_PROGRESS;
    uint8_t ret;

    // Set Checksum
    ret = FLASHER_WriteByte((uint8_t*) &checksum, sizeof(checksum), FOTA_CHECKSUM_ADDRESS);

    // Set DFU flag
    if (ret) {
        ret = FLASHER_WriteByte((uint8_t*) &data, sizeof(data), FOTA_FLAG_ADDRESS);
    }

    return ret;
}

