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
#include "can.h"
#include "crc.h"
#include "i2c.h"
#include "usart.h"

/* Public functions implementation --------------------------------------------*/
uint8_t FOTA_Upgrade(IAP_TYPE type) {
  SIMCOM_RESULT res = SIM_RESULT_OK;
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

  Simcom_SetState(SIM_STATE_READY, 0);

  // Turn ON HMI-Primary, based on knob state.
  GATE_Hmi1Power(GATE_ReadKnobState());
  _DelayMS(3000);

  /* Set FTP directory */
  strcpy(ftp.path, (type == IAP_VCU) ? "/vcu/" : "/hmi/");

  /* Set current IAP type */
  *(uint32_t*) IAP_RESPONSE_ADDR = IAP_DFU_ERROR;
  FOCAN_SetProgress(type, 0.0f);

  /* Backup if needed */
  if (res > 0)
    if (!FOTA_InProgressDFU())
      FOTA_SetDFU();

  /* Get the stored checksum information */
  if (res > 0) {
    if (type == IAP_VCU)
      FOTA_GetChecksum(&cksumOld);
    else
      res = FOCAN_GetChecksum(&cksumOld);
  }

  // Initialise SIMCOM
  if (res > 0) {
    res = Simcom_SetState(SIM_STATE_GPRS_ON, 60000);

    // Initialise Bearer &  FTP
    if (res > 0) {
      // Initialise bearer for TCP based applications.
      res = AT_BearerInitialize();

      // Initialise FTP
      if (res > 0)
        res = AT_FtpInitialize(&ftp);
    }

    // Handle error
    if (res <= 0)
      *(uint32_t*) IAP_RESPONSE_ADDR = IAP_SIMCOM_TIMEOUT;
  }

  // Get file size
  if (res > 0)
    res = AT_FtpFileSize(&ftp);

  // Open FTP Session
  if (res > 0) {
    ftpget.mode = FTPGET_OPEN;
    res = AT_FtpDownload(&ftpget);

    if (res > 0)
      res = ftpget.response == FTP_READY;
  }

  // Get checksum of new firmware
  if (res > 0) {
    res = FOTA_DownloadChecksum(&ftp, &ftpget, &cksumNew);

    // Only download when image is different
    if (res > 0) {
      res = (cksumOld != cksumNew);

      // Handle error
      if (res <= 0)
        *(uint32_t*) IAP_RESPONSE_ADDR = IAP_FIRMWARE_SAME;
    }

    // Decrease the total size
    if (res > 0)
      ftp.size -= 4;
  }

  // Download & Program new firmware
  if (res > 0) {
    res = FOTA_DownloadFirmware(&ftp, &ftpget, &len, type);

    // Handle error
    if (res <= 0)
      if ((*(uint32_t*) IAP_RESPONSE_ADDR) != IAP_CANBUS_FAILED)
        *(uint32_t*) IAP_RESPONSE_ADDR = IAP_DOWNLOAD_ERROR;
  }

  // Buffer filled, compare the checksum
  if (res > 0) {
    if (type == IAP_VCU) {
      res = FOTA_ValidateChecksum(cksumNew, len, APP_START_ADDR);
      // Glue related information to new image
      if (res > 0) {
        FOTA_GlueInfo32(CHECKSUM_OFFSET, &cksumNew);
        FOTA_GlueInfo32(SIZE_OFFSET, &len);
      }
    } else
      res = FOCAN_DownloadHook(CAND_PASCA_DOWNLOAD, &cksumNew);

    // Handle error
    if (res <= 0)
      *(uint32_t*) IAP_RESPONSE_ADDR = IAP_CHECKSUM_INVALID;
  }

  // Reset DFU flag only when FOTA success
  if (res > 0) {
    FOTA_ResetDFU();

    // Handle success
    *(uint32_t*) IAP_RESPONSE_ADDR = IAP_DFU_SUCCESS;
  }

  return (res > 0);
}

uint8_t FOTA_DownloadChecksum(at_ftp_t *setFTP, at_ftpget_t *setFTPGET, uint32_t *checksum) {
  SIMCOM_RESULT res;

  // Initiate Download
  setFTPGET->mode = FTPGET_READ;
  setFTPGET->reqlength = 4;
  res = AT_FtpDownload(setFTPGET);

  if (res > 0) {
    memcpy(checksum, setFTPGET->ptr, sizeof(uint32_t));

    printf("FOTA:ChecksumOrigin = 0x%08X\n", (unsigned int) *checksum);
  }

  return (res == SIM_RESULT_OK);
}

uint8_t FOTA_DownloadFirmware(at_ftp_t *setFTP, at_ftpget_t *setFTPGET, uint32_t *len, IAP_TYPE type) {
  SIMCOM_RESULT res = SIM_RESULT_OK;
  AT_FTP_STATE state;
  uint32_t timer;
  float percent;

  // Backup and prepare the area
  if (type == IAP_VCU)
    FLASHER_BackupApp();
  else
    res = FOCAN_DownloadHook(CAND_PRA_DOWNLOAD, &(setFTP->size));

  // Read FTP File
  if (res > 0) {
    // Prepare, start timer
    printf("FOTA:Start\n");
    timer = _GetTickMS();
    SIM.downloading = 1;

    // Copy chunk by chunk
    setFTPGET->mode = FTPGET_READ;
    setFTPGET->reqlength = 1460;
    do {
      // Initiate Download
      res = AT_FtpDownload(setFTPGET);

      // Copy buffer to flash
      if (res > 0 && setFTPGET->cnflength) {
        if (type == IAP_VCU)
          res = FLASHER_WriteAppArea((uint8_t*) setFTPGET->ptr, setFTPGET->cnflength, *len);
        else
          res = FOCAN_DownloadFlash((uint8_t*) setFTPGET->ptr, setFTPGET->cnflength, *len, setFTP->size);
      } else
        // failure
        break;

      // Check after flashing
      if (res > 0) {
        // Update pointer position
        *len += setFTPGET->cnflength;

        // Indicator
        percent = (float) (*len * 100.0f / setFTP->size);
        GATE_LedToggle();
        printf("FOTA:Progress = %lu Bytes (%u%%)\n", *len, (uint8_t) percent);

        FOCAN_SetProgress(type, percent);
      }
    } while ((res > 0) && (*len < setFTP->size));

    // Check, stop timer
    if (*len == setFTP->size)
      printf("FOTA:End = %lu ms\n", _GetTickMS() - timer);
    else {
      printf("FOTA:Failed\n");
      res = SIM_RESULT_ERROR;
    }
  }

  // Check state
  AT_FtpCurrentState(&state);
  if (state == FTP_STATE_ESTABLISHED)
    Simcom_Cmd("AT+FTPQUIT\r", NULL, 500, 0);

  return (res == SIM_RESULT_OK);
}

uint8_t FOTA_ValidateChecksum(uint32_t checksum, uint32_t len, uint32_t address) {
  uint8_t *addr = (uint8_t*) address;
  uint32_t crc = 0;

  // Calculate CRC
  crc = CRC_Calculate8(addr, len, 1);

  // Indicator
  if (crc == checksum)
    printf("FOTA:Checksum = MATCH\n");
  else
    printf("FOTA:Checksum = DIFF (0x%08X != 0x%08X)\n",
        (unsigned int) checksum, (unsigned int) crc);

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
