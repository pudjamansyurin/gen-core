/*
 * _fota.c
 *
 *  Created on: 18 Jun 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_fota.h"

#include "App/_focan.h"
#include "App/_eeprom.h"
#include "Drivers/_bat.h"
#include "Drivers/_canbus.h"
#include "Drivers/_crc.h"
#include "Drivers/_flasher.h"
#include "adc.h"
#include "can.h"
#include "crc.h"
#include "i2c.h"
#include "iwdg.h"
#include "usart.h"

/* Private functions prototypes
 * --------------------------------------------*/
static SIM_RESULT prepareFTP(at_ftp_t *ftp, uint32_t timeout);
static SIM_RESULT openFTP(at_ftpget_t *ftpGET);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t FOTA_Upgrade(IAP_TYPE type) {
  uint32_t timeout = 60000, crcOld = 0, crcNew = 0, len = 0;
  SIM_RESULT res = SIM_OK;
  at_ftpget_t ftpget;
  at_ftp_t ftp = {
      .file = "CRC_APP.bin",
      .size = 0,
  };

  if (BAT_ScanValue() < SIMCOM_MIN_MV) res = SIM_ERROR;

  // Turn ON HMI-Primary.
  if (res == SIM_OK) {
    GATE_Hmi1Power(1);
    _DelayMS(1000);

    /* Set FTP directory */
    sprintf(ftp.path, "/%s/", (type == IAP_HMI) ? "HMI" : "VCU");

    /* Set current IAP type */
    *(uint32_t *)IAP_RESPONSE_ADDR = IAP_FOTA_ERROR;
    FOCAN_SetProgress(type, 0.0f);

    Simcom_SetState(SIM_STATE_READY, 0);
  }

  /* Backup if needed */
  if (res == SIM_OK) {
    FOCAN_SetProgress(type, 0.0f);
    if (!FOTA_InProgress()) FOTA_SetFlag();
  }

  /* Get the stored crc information */
  if (res == SIM_OK) {
    FOCAN_SetProgress(type, 0.0f);
    if (type == IAP_HMI)
      res = FOCAN_GetCRC(&crcOld);
    else
      FOTA_GetCRC(&crcOld);
  }

  // Initialise SIMCOM
  if (res == SIM_OK) {
    FOCAN_SetProgress(type, 0.0f);
    res = prepareFTP(&ftp, timeout);

    if (res <= 0) *(uint32_t *)IAP_RESPONSE_ADDR = IAP_SIMCOM_TIMEOUT;
  }

  // Get file size
  if (res == SIM_OK) {
    FOCAN_SetProgress(type, 0.0f);
    res = AT_FtpFileSize(&ftp);
  }

  // Open FTP Session
  if (res == SIM_OK) {
    FOCAN_SetProgress(type, 0.0f);
    res = openFTP(&ftpget);
  }

  // Get crc of new firmware
  if (res == SIM_OK) {
    FOCAN_SetProgress(type, 0.0f);
    res = FOTA_DownloadCRC(&ftpget, &crcNew);

    // Only download when image is different
    if (res == SIM_OK) {
      res = (crcOld != crcNew);

      if (res <= 0)
        *(uint32_t *)IAP_RESPONSE_ADDR = IAP_FIRMWARE_SAME;
      else
        printf("FOTA:CRC = 0x%08X => 0x%08X\n", (unsigned int)crcOld,
               (unsigned int)crcNew);
    }

    // Decrease the total size
    if (res == SIM_OK) ftp.size -= sizeof(uint32_t);
  }

  // Download & Program new firmware
  if (res == SIM_OK) {
    res = FOTA_DownloadFirmware(&ftp, &ftpget, &len, type, timeout);

    if (res <= 0)
      if ((*(uint32_t *)IAP_RESPONSE_ADDR) != IAP_CANBUS_FAILED)
        *(uint32_t *)IAP_RESPONSE_ADDR = IAP_DOWNLOAD_ERROR;
  }

  // Buffer filled, compare the crc
  if (res == SIM_OK) {
    if (type == IAP_HMI)
      res = FOCAN_DownloadHook(CAND_FOCAN_PASCA, &crcNew);
    else {
      res = FOTA_ValidateCRC(crcNew, len, APP_START_ADDR);
      // Glue related information to new image
      if (res == SIM_OK) {
        FOTA_GlueInfo32(CRC_OFFSET, &crcNew);
        FOTA_GlueInfo32(SIZE_OFFSET, &len);
      }
    }

    if (res <= 0)
      *(uint32_t *)IAP_RESPONSE_ADDR = IAP_CRC_INVALID;
    else
      _DelayMS(2000);
  }

  // Reset FOTA flag only when FOTA success
  if (res == SIM_OK) {
    FOTA_ResetFlag();

    // Handle success
    *(uint32_t *)IAP_RESPONSE_ADDR = IAP_FOTA_SUCCESS;
  }

  return (res == SIM_OK);
}

uint8_t FOTA_DownloadCRC(at_ftpget_t *ftpGET, uint32_t *crc) {
  SIM_RESULT res;

  // Initiate Download
  ftpGET->mode = FTPGET_READ;
  ftpGET->reqlength = sizeof(uint32_t);
  res = AT_FtpDownload(ftpGET);

  if (res == SIM_OK) memcpy(crc, ftpGET->ptr, sizeof(uint32_t));

  return (res == SIM_OK);
}

uint8_t FOTA_DownloadFirmware(at_ftp_t *ftp, at_ftpget_t *ftpGET, uint32_t *len,
                              IAP_TYPE type, uint32_t timeout) {
  SIM_RESULT res = SIM_OK;
  AT_FTP_STATE state;
  uint32_t timer;
  float percent;

  // Backup and prepare the area
  if (type == IAP_HMI)
    res = FOCAN_DownloadHook(CAND_FOCAN_PRA, &(ftp->size));
  else
    FLASHER_BackupApp();

  // Read FTP File
  if (res == SIM_OK) {
    // Prepare, start timer
    printf("FOTA:Start\n");
    timer = _GetTickMS();

    // Copy chunk by chunk
    do {
      // Initiate Download
      ftpGET->mode = FTPGET_READ;
      ftpGET->reqlength = 1460;
      res = AT_FtpDownload(ftpGET);

      // Copy buffer to flash
      if (res == SIM_OK && ftpGET->cnflength) {
        if (type == IAP_HMI)
          res = FOCAN_DownloadFlash((uint8_t *)ftpGET->ptr, ftpGET->cnflength,
                                    *len, ftp->size);
        else
          res = FLASHER_WriteAppArea((uint8_t *)ftpGET->ptr, ftpGET->cnflength,
                                     *len);

        // Check after flashing
        if (res == SIM_OK) {
          *len += ftpGET->cnflength;

          // Indicator
          percent = (float)(*len * 100.0f / ftp->size);
          printf("FOTA:Progress = %lu Bytes (%d%%)\n", *len, (uint8_t)percent);
          GATE_LedToggle();

          FOCAN_SetProgress(type, percent);
        }
      } else {
        AT_FtpCurrentState(&state);
        if (state == FTP_STATE_ESTABLISHED)
          Simcom_Cmd("AT+FTPQUIT\r", SIM_RSP_OK, 500);

        res = prepareFTP(ftp, timeout);
        if (res == SIM_OK) res = AT_FtpResume(*len);
        if (res == SIM_OK) res = openFTP(ftpGET);
        if (res == SIM_OK && ftpGET->response != FTP_READY) res = SIM_ERROR;
      }

    } while ((res == SIM_OK) && (*len < ftp->size));

    // Check, stop timer
    if (*len >= ftp->size)
      printf("FOTA:End = %lu ms\n", _GetTickMS() - timer);
    else {
      printf("FOTA:Failed\n");
      res = SIM_ERROR;
    }
  }

  return (res == SIM_OK);
}

uint8_t FOTA_ValidateCRC(uint32_t crc, uint32_t len, uint32_t address) {
  uint8_t *addr = (uint8_t *)address;
  uint32_t crcVal = 0;

  // Calculate CRC
  crcVal = CRC_Calculate8(addr, len, 1);

  // Indicator
  if (crcVal == crc)
    printf("FOTA:CRC = MATCH\n");
  else
    printf("FOTA:CRC = DIFF (0x%08X != 0x%08X)\n", (unsigned int)crc,
           (unsigned int)crcVal);

  _DelayMS(1000);

  return (crc == crcVal);
}

uint8_t FOTA_ValidImage(uint32_t address) {
  uint32_t size, crc;
  uint8_t p;

  /* Check beginning stack pointer */
  p = IS_VALID_SP(address);

  /* Check the size */
  if (p) {
    /* Get the stored size information */
    size = *(uint32_t *)(address + SIZE_OFFSET);
    p = (size < APP_MAX_SIZE);
  }

  /* Check the crc */
  if (p) {
    /* Get the stored crc information */
    crc = *(uint32_t *)(address + CRC_OFFSET);

    /* Validate crc */
    p = FOTA_ValidateCRC(crc, size, address);
  }

  return p;
}

void FOTA_JumpToApplication(void) {
  /* Set MSP & Reset address */
  uint32_t appStack = *(__IO uint32_t *)APP_START_ADDR;
  uint32_t appEntry = *(__IO uint32_t *)(APP_START_ADDR + 4);
  void (*jump)(void) = (void (*)(void))(appEntry);

  /* Shutdown all peripherals */
  //	HAL_IWDG_Refresh(&hiwdg);
  HAL_CRC_MspDeInit(&hcrc);
  HAL_CAN_MspDeInit(&hcan1);
  HAL_I2C_MspDeInit(&hi2c2);
  HAL_UART_MspDeInit(&huart1);
  HAL_UART_MspDeInit(&huart9);
  HAL_ADC_DeInit(&hadc1);
  HAL_RCC_DeInit();
  HAL_DeInit();

  /* Reset systick */
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  // __disable_irq();
  SCB->VTOR = appStack;

  // Set MSP & Jump to ResetHandler
  __set_MSP(appStack);
  jump();

  /* Never reached */
  while (1) {
  };
}

void FOTA_Reboot(IAP_TYPE type) {
  /* Clear backup area */
  if (type == IAP_VCU) FLASHER_EraseBkpArea();

  FOTA_ResetFlag();
  HAL_NVIC_SystemReset();
}

void FOTA_GetCRC(uint32_t *crc) {
  uint32_t address = BKP_START_ADDR;

  if (FOTA_NeedBackup()) address = APP_START_ADDR;

  *crc = *(uint32_t *)(address + CRC_OFFSET);
}

void FOTA_GlueInfo32(uint32_t offset, uint32_t *data) {
  FLASHER_WriteAppArea((uint8_t *)data, sizeof(uint32_t), offset);
}

uint8_t FOTA_NeedBackup(void) {
  return (FOTA_ValidImage(APP_START_ADDR) && !FOTA_ValidImage(BKP_START_ADDR));
}

uint8_t FOTA_InProgress(void) { return (FOTA.FLAG == FOTA_PROGRESS_FLAG); }

void FOTA_SetFlag(void) { EE_FotaFlag(EE_CMD_W, FOTA_PROGRESS_FLAG); }

void FOTA_ResetFlag(void) { EE_FotaFlag(EE_CMD_W, 0); }

/* Private functions implementation
 * --------------------------------------------*/
static SIM_RESULT prepareFTP(at_ftp_t *ftp, uint32_t timeout) {
  SIM_RESULT res;

  res = Simcom_SetState(SIM_STATE_BEARER_ON, timeout);
  if (res == SIM_OK) res = AT_FtpInitialize(ftp);

  return res;
}

static SIM_RESULT openFTP(at_ftpget_t *ftpGET) {
  ftpGET->mode = FTPGET_OPEN;
  return AT_FtpDownload(ftpGET);
}
