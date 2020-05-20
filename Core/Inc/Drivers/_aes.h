/*
 * _aes.h
 *
 *  Created on: May 12, 2020
 *      Author: pudja
 */

#ifndef INC_LIBS__AES_H_
#define INC_LIBS__AES_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Public functions implementation -------------------------------------------*/
void AES_Init(void);
void AES_UpdateKey(uint32_t secret[4]);
uint8_t AES_Encrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz);
uint8_t AES_Decrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz);
void AES_Tester(void);

#endif /* INC_LIBS__AES_H_ */
