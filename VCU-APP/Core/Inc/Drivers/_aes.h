/*
 * _aes.h
 *
 *  Created on: May 12, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__AES_H_
#define INC_LIBS__AES_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported variables
 * --------------------------------------------*/
extern uint32_t AES_KEY[4];

/* Public functions prototype
 * --------------------------------------------*/
uint8_t AES_Init(void);
uint8_t AES_ChangeKey(uint32_t *key);
uint8_t AES_Encrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz);
uint8_t AES_Decrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz);

#endif /* INC_LIBS__AES_H_ */
