/*
 * _aes.c
 *
 *  Created on: May 12, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_aes.h"
#include "_log.h"

/* External variables -------------------------------------------------------*/
extern CRYP_HandleTypeDef hcryp;

/* Private variables ---------------------------------------------------------*/

/* Public functions implementation -------------------------------------------*/
uint8_t AES_Encrypt(uint8_t *pSrc, uint8_t *pDst, uint16_t Sz) {
	return (HAL_CRYP_Encrypt(&hcryp, (uint32_t*) pSrc, Sz, (uint32_t*) pDst, 1000) == HAL_OK);
}

uint8_t AES_Decrypt(uint8_t *pSrc, uint8_t *pDst, uint16_t Sz) {
	return (HAL_CRYP_Decrypt(&hcryp, (uint32_t*) pSrc, Sz, (uint32_t*) pDst, 1000) == HAL_OK);
}

void AES_Tester(void) {
	uint8_t Plaintext[] = {
			0x01, 0x02, 0x03, 0x04,
			0xAA, 0xBB, 0xCC, 0xDD,
			0xA9, 0xA8, 0xA7, 0xA6,
			0x50, 0x60, 0x70, 0x80 };
	uint8_t Encryptedtext[sizeof(Plaintext)] = { 0 };
	uint8_t Decryptedtext[sizeof(Plaintext)] = { 0 };

	// Encrypt & Decrypt
	if (AES_Encrypt(Plaintext, Encryptedtext, sizeof(Plaintext))) {
		LOG_StrLn("AES:EncryptOK");
	}
	if (AES_Decrypt(Encryptedtext, Decryptedtext, sizeof(Encryptedtext))) {
		LOG_StrLn("AES:DecryptOK");
	}

	// result
	if (memcmp(Plaintext, Decryptedtext, sizeof(Plaintext)) == 0) {
		LOG_StrLn("AES:ItWorks!");
	}
}
