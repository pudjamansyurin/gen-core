/*
 * _fota.c
 *
 *  Created on: 18 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_fota.h"
#include "Libs/_focan.h"
#include "Libs/_eeprom.h"
#include "Drivers/_flasher.h"
#include "Drivers/_crc.h"

/* External variables ---------------------------------------------------------*/
extern CAN_HandleTypeDef hcan1;
extern CRC_HandleTypeDef hcrc;
extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart1;
extern uint32_t DFU_FLAG;
extern sim_t SIM;

/* Public functions implementation --------------------------------------------*/
uint8_t FOTA_Upgrade(IAP_TYPE type) {
  SIMCOM_RESULT p = SIM_RESULT_OK;
  uint32_t cksumOld, cksumNew = 0, len = 0;
  at_ftpget_t ftpget;
  at_ftp_t ftp = {
      .id = 1,
      .server = NET_FTP_SERVER,
      .username = NET_FTP_USERNAME,
      .password = NET_FTP_PASSWORD,
      .file = "CRC_APP.bin",
      .size = 0,
  };

  /* Set FTP directory */
  if (type == IAP_VCU) 
    strcpy(ftp.path, "/vcu/");
  else {
    strcpy(ftp.path, "/hmi/");

    FOCAN_RefreshPowerHMI();
    _DelayMS(1000);
  }

  /* Set current IAP type */
  *(uint32_t*) IAP_RESPONSE_ADDR = IAP_DFU_ERROR;
  FOCAN_SetProgress(type, 0.0f);

  /* Backup if needed */
  if (p > 0) 
    if (!FOTA_InProgressDFU()) 
      FOTA_SetDFU();  

  /* Get the stored checksum information */
  if (p > 0) {
    if (type == IAP_VCU) 
      FOTA_GetChecksum(&cksumOld);
    else 
      p = FOCAN_GetChecksum(&cksumOld);
  }
    

  // Initialise SIMCOM
  if (p > 0) {
    p = Simcom_SetState(SIM_STATE_GPRS_ON, 60000);

    // Initialise Bearer &  FTP
    if (p > 0) {
      // Initialise bearer for TCP based applications.
      p = AT_BearerInitialize();

      // Initialise FTP
      if (p > 0) 
        p = AT_FtpInitialize(&ftp);
    }

    // Handle error
    if (p <= 0) 
      *(uint32_t*) IAP_RESPONSE_ADDR = IAP_SIMCOM_TIMEOUT;
  }

  // Get file size
  if (p > 0) 
    p = AT_FtpFileSize(&ftp);

  // Open FTP Session
  if (p > 0) {
    ftpget.mode = FTPGET_OPEN;
    p = AT_FtpDownload(&ftpget);

    if (p > 0) 
      p = ftpget.response == FTP_READY;
  }

  // Get checksum of new firmware
  if (p > 0) {
    p = FOTA_DownloadChecksum(&ftp, &ftpget, &cksumNew);

    // Only download when image is different
    if (p > 0) {
      p = (cksumOld != cksumNew);

      // Handle error
      if (p <= 0) 
        *(uint32_t*) IAP_RESPONSE_ADDR = IAP_FIRMWARE_SAME;
    }

    // Decrease the total size
    if (p > 0) 
      ftp.size -= 4;
  }

  // Download & Program new firmware
  if (p > 0) {
    p = FOTA_DownloadFirmware(&ftp, &ftpget, &len, type);

    // Handle error
    if (p <= 0) 
      if ((*(uint32_t*) IAP_RESPONSE_ADDR) != IAP_CANBUS_FAILED) 
        *(uint32_t*) IAP_RESPONSE_ADDR = IAP_DOWNLOAD_ERROR;
  }

  // Buffer filled, compare the checksum
  if (p > 0) {
    if (type == IAP_VCU) {
      p = FOTA_ValidateChecksum(cksumNew, len, APP_START_ADDR);
      // Glue related information to new image
      if (p > 0) {
        FOTA_GlueInfo32(CHECKSUM_OFFSET, &cksumNew);
        FOTA_GlueInfo32(SIZE_OFFSET, &len);
      }
    } else 
      p = FOCAN_DownloadHook(CAND_PASCA_DOWNLOAD, &cksumNew);

    // Handle error
    if (p <= 0) 
      *(uint32_t*) IAP_RESPONSE_ADDR = IAP_CHECKSUM_INVALID;
  }

  // Reset DFU flag only when FOTA success
  if (p > 0) {
    FOTA_ResetDFU();

    // Handle success
    *(uint32_t*) IAP_RESPONSE_ADDR = IAP_DFU_SUCCESS;
  }

  return (p > 0);
}

uint8_t FOTA_DownloadChecksum(at_ftp_t *setFTP, at_ftpget_t *setFTPGET, uint32_t *checksum) {
  SIMCOM_RESULT p;

  // Initiate Download
  setFTPGET->mode = FTPGET_READ;
  setFTPGET->reqlength = 4;
  p = AT_FtpDownload(setFTPGET);

  if (p > 0) {
    // Copy to Buffer
    memcpy(checksum, setFTPGET->ptr, sizeof(uint32_t));

    // Indicator
    LOG_Str("FOTA:ChecksumOrigin = ");
    LOG_Hex32(*checksum);
    LOG_Enter();
  }

  return (p == SIM_RESULT_OK);
}

uint8_t FOTA_DownloadFirmware(at_ftp_t *setFTP, at_ftpget_t *setFTPGET, uint32_t *len, IAP_TYPE type) {
  SIMCOM_RESULT p = SIM_RESULT_OK;
  AT_FTP_STATE state;
  uint32_t timer;
  float percent;

  // Backup and prepare the area
  if (type == IAP_VCU) 
    FLASHER_BackupApp();
  else 
    p = FOCAN_DownloadHook(CAND_PRA_DOWNLOAD, &(setFTP->size));

  // Read FTP File
  if (p > 0) {
    // Prepare, start timer
    LOG_StrLn("FOTA:Start");
    timer = _GetTickMS();
    SIM.downloading = 1;

    // Copy chunk by chunk
    setFTPGET->mode = FTPGET_READ;
    setFTPGET->reqlength = 1460;
    do {
      // Initiate Download
      p = AT_FtpDownload(setFTPGET);

      // Copy buffer to flash
      if (p > 0 && setFTPGET->cnflength) {
        if (type == IAP_VCU) 
          p = FLASHER_WriteAppArea((uint8_t*) setFTPGET->ptr, setFTPGET->cnflength, *len);
        else 
          p = FOCAN_DownloadFlash((uint8_t*) setFTPGET->ptr, setFTPGET->cnflength, *len, setFTP->size);
      } else
        // failure
        break;

      // Check after flashing
      if (p > 0) {
        // Update pointer position
        *len += setFTPGET->cnflength;

        // Indicator
        percent = (float) (*len * 100.0f / setFTP->size);
        _LedToggle();
        LOG_Str("FOTA:Progress = ");
        LOG_Int(*len);
        LOG_Str(" Bytes (");
        LOG_Int((uint8_t) percent);
        LOG_StrLn("%)");

        FOCAN_SetProgress(type, percent);
      }
    } while ((p > 0) && (*len < setFTP->size));

    // Check, stop timer
    if (*len == setFTP->size) {
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
  if (state == FTP_STATE_ESTABLISHED) 
    Simcom_Command("AT+FTPQUIT\r", NULL, 500, 0);

  return (p == SIM_RESULT_OK);
}

uint8_t FOTA_ValidateChecksum(uint32_t checksum, uint32_t len, uint32_t address) {
  uint8_t *addr = (uint8_t*) address;
  uint32_t crc = 0;

  // Calculate CRC
  crc = CRC_Calculate8(addr, len, 1);

  // Indicator
  LOG_Str("FOTA:Checksum = ");
  if (crc == checksum) 
    LOG_StrLn("MATCH");
  else {
    LOG_StrLn("NOT MATCH");
    LOG_Hex32(checksum);
    LOG_Str(" != ");
    LOG_Hex32(crc);
    LOG_Enter();
  }

  return (crc == checksum);
}

uint8_t FOTA_ValidImage(uint32_t address) {
  uint32_t size, checksum;
  uint8_t p;

  /* Check beginning stack pointer */
  p = IS_VALID_SP(address);

  /* Check the size */
  if (p) {
    /* Get the stored size information */
    size = *(uint32_t*) (address + SIZE_OFFSET);
    p = (size < APP_MAX_SIZE );
  }

  /* Check the checksum */
  if (p) {
    /* Get the stored checksum information */
    checksum = *(uint32_t*) (address + CHECKSUM_OFFSET);

    /* Validate checksum */
    p = FOTA_ValidateChecksum(checksum, size, address);
  }

  return p;
}

void FOTA_JumpToApplication(void) {
  uint32_t appStack, appEntry;

  /* Get stack & entry pointer */
  appStack = *(__IO uint32_t*) APP_START_ADDR;
  appEntry = *(__IO uint32_t*) (APP_START_ADDR + 4);

  /* Shutdown all peripherals */
  HAL_CAN_MspDeInit(&hcan1);
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

void FOTA_Reboot(IAP_TYPE type) {
  /* Clear backup area */
  if (type == IAP_VCU) 
    FLASHER_EraseBkpArea();

  FOTA_ResetDFU();
  HAL_NVIC_SystemReset();
}

void FOTA_GetChecksum(uint32_t *checksum) {
  uint32_t address = BKP_START_ADDR;

  if (FOTA_NeedBackup()) 
    address = APP_START_ADDR;

  *checksum = *(uint32_t*) (address + CHECKSUM_OFFSET);
}

void FOTA_GlueInfo32(uint32_t offset, uint32_t *data) {
  FLASHER_WriteAppArea((uint8_t*) data, sizeof(uint32_t), offset);
}

uint8_t FOTA_NeedBackup(void) {
  return (FOTA_ValidImage(APP_START_ADDR) && !FOTA_ValidImage(BKP_START_ADDR));
}

uint8_t FOTA_InProgressDFU(void) {
  return (DFU_FLAG == DFU_PROGRESS_FLAG );
}

void FOTA_SetDFU(void) {
  EEPROM_FlagDFU(EE_CMD_W, DFU_PROGRESS_FLAG);
}

void FOTA_ResetDFU(void) {
  EEPROM_FlagDFU(EE_CMD_W, 0);
}
