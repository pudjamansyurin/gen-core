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

/* Private functions prototypes -----------------------------------------------*/
static SIM_RESULT prepareFTP(at_ftp_t *ftp, uint32_t timeout);
static SIM_RESULT openFTP(at_ftpget_t *ftpGET);

/* Public functions implementation --------------------------------------------*/
uint8_t FOTA_Upgrade(IAP_TYPE type) {
	SIM_RESULT res = SIM_OK;
	uint32_t timeout = 60000;
	uint32_t cksumOld = 0, cksumNew = 0, len = 0;
	at_ftpget_t ftpget;
	at_ftp_t ftp = {
			.file = "CRC_APP.bin",
			.size = 0,
	};

	// Turn ON HMI-Primary.
	GATE_Hmi1Power(1);
	_DelayMS(1000);

	/* Set FTP directory */
	sprintf(ftp.path, "/%s/", (type == IAP_HMI) ? "hmi" : "vcu");

	/* Set current IAP type */
	*(uint32_t*) IAP_RESPONSE_ADDR = IAP_DFU_ERROR;
	FOCAN_SetProgress(type, 0.0f);

	Simcom_SetState(SIM_STATE_READY, 0);

	/* Backup if needed */
	if (res > 0) {
		FOCAN_SetProgress(type, 0.0f);
		if (!FOTA_InProgressDFU())
			FOTA_SetDFU();
	}

	/* Get the stored checksum information */
	if (res > 0) {
		FOCAN_SetProgress(type, 0.0f);
		if (type == IAP_HMI)
			res = FOCAN_GetChecksum(&cksumOld);
		else
			FOTA_GetChecksum(&cksumOld);
	}

	// Initialise SIMCOM
	if (res > 0) {
		FOCAN_SetProgress(type, 0.0f);
		res = prepareFTP(&ftp, timeout);

		if (res <= 0)
			*(uint32_t*) IAP_RESPONSE_ADDR = IAP_SIMCOM_TIMEOUT;
	}

	// Get file size
	if (res > 0) {
		FOCAN_SetProgress(type, 0.0f);
		res = AT_FtpFileSize(&ftp);
	}

	// Open FTP Session
	if (res > 0) {
		FOCAN_SetProgress(type, 0.0f);
		res = openFTP(&ftpget);
	}

	// Get checksum of new firmware
	if (res > 0) {
		FOCAN_SetProgress(type, 0.0f);
		res = FOTA_DownloadChecksum(&ftpget, &cksumNew);

		// Only download when image is different
		if (res > 0) {
			res = (cksumOld != cksumNew);

			if (res <= 0)
				*(uint32_t*) IAP_RESPONSE_ADDR = IAP_FIRMWARE_SAME;
			else
				printf("FOTA:Checksum = 0x%08X => 0x%08X\n", (unsigned int) cksumOld, (unsigned int) cksumNew);
		}

		// Decrease the total size
		if (res > 0)
			ftp.size -= sizeof(uint32_t);
	}

	// Download & Program new firmware
	if (res > 0) {
		res = FOTA_DownloadFirmware(&ftp, &ftpget, &len, type, timeout);

		if (res <= 0)
			if ((*(uint32_t*) IAP_RESPONSE_ADDR) != IAP_CANBUS_FAILED)
				*(uint32_t*) IAP_RESPONSE_ADDR = IAP_DOWNLOAD_ERROR;
	}

	// Buffer filled, compare the checksum
	if (res > 0) {
		if (type == IAP_HMI)
			res = FOCAN_DownloadHook(CAND_PASCA_DOWNLOAD, &cksumNew);
		else {
			res = FOTA_ValidateChecksum(cksumNew, len, APP_START_ADDR);
			// Glue related information to new image
			if (res > 0) {
				FOTA_GlueInfo32(CHECKSUM_OFFSET, &cksumNew);
				FOTA_GlueInfo32(SIZE_OFFSET, &len);
			}
		}

		if (res <= 0)
			*(uint32_t*) IAP_RESPONSE_ADDR = IAP_CHECKSUM_INVALID;
		else
			_DelayMS(2000);
	}

	// Reset DFU flag only when FOTA success
	if (res > 0) {
		FOTA_ResetDFU();

		// Handle success
		*(uint32_t*) IAP_RESPONSE_ADDR = IAP_DFU_SUCCESS;
	}

	return (res > 0);
}

uint8_t FOTA_DownloadChecksum(at_ftpget_t *ftpGET, uint32_t *checksum) {
	SIM_RESULT res;

	// Initiate Download
	ftpGET->mode = FTPGET_READ;
	ftpGET->reqlength = sizeof(uint32_t);
	res = AT_FtpDownload(ftpGET);

	if (res > 0)
		memcpy(checksum, ftpGET->ptr, sizeof(uint32_t));

	return (res == SIM_OK);
}

uint8_t FOTA_DownloadFirmware(at_ftp_t *ftp, at_ftpget_t *ftpGET, uint32_t *len, IAP_TYPE type, uint32_t timeout) {
	SIM_RESULT res = SIM_OK;
	AT_FTP_STATE state;
	uint32_t timer;
	float percent;

	// Backup and prepare the area
	if (type == IAP_HMI)
		res = FOCAN_DownloadHook(CAND_PRA_DOWNLOAD, &(ftp->size));
	else
		FLASHER_BackupApp();

	// Read FTP File
	if (res > 0) {
		// Prepare, start timer
		printf("FOTA:Start\n");
		timer = _GetTickMS();
		SIM.downloading = 1;

		// Copy chunk by chunk
		do {
			// Initiate Download
			ftpGET->mode = FTPGET_READ;
			ftpGET->reqlength = 1460;
			res = AT_FtpDownload(ftpGET);

			// Copy buffer to flash
			if (res > 0 && ftpGET->cnflength) {
				if (type == IAP_HMI)
					res = FOCAN_DownloadFlash((uint8_t*) ftpGET->ptr, ftpGET->cnflength, *len, ftp->size);
				else
					res = FLASHER_WriteAppArea((uint8_t*) ftpGET->ptr, ftpGET->cnflength, *len);

				// Check after flashing
				if (res > 0) {
					*len += ftpGET->cnflength;

					// Indicator
					percent = (float) (*len * 100.0f / ftp->size);
					printf("FOTA:Progress = %lu Bytes (%d%%)\n", *len, (uint8_t) percent);
					GATE_LedToggle();

					FOCAN_SetProgress(type, percent);
				}
			} else {
				AT_FtpCurrentState(&state);
				if (state == FTP_STATE_ESTABLISHED) 
					Simcom_Cmd("AT+FTPQUIT\r", SIM_RSP_OK, 500);

				res = prepareFTP(ftp, timeout);
				if (res > 0)
					res = AT_FtpResume(*len);
				if (res > 0)
					res = openFTP(ftpGET);
			}

		} while ((res > 0) && (*len < ftp->size));

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

	_DelayMS(1000);

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

/* Private functions implementation --------------------------------------------*/
static SIM_RESULT prepareFTP(at_ftp_t *ftp, uint32_t timeout) {
	SIM_RESULT res;

	res = Simcom_SetState(SIM_STATE_BEARER_ON, timeout);
	if (res > 0)
		res = AT_FtpInitialize(ftp);

	return res;
}

static SIM_RESULT openFTP(at_ftpget_t *ftpGET) {
	SIM_RESULT res;

	ftpGET->mode = FTPGET_OPEN;
	res = AT_FtpDownload(ftpGET);

	if (res > 0)
		res = ftpGET->response == FTP_READY;

	return res;
}
