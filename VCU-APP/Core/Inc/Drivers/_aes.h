/*
 * _aes.h
 *
 *  Created on: May 12, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__AES_H_
#define INC_DRIVERS__AES_H_

/* Includes
 * --------------------------------------------*/
#include "App/_common.h"

/* Exported types
 * --------------------------------------------*/
typedef uint32_t aes_key_t[4];

/* Public functions prototype
 * --------------------------------------------*/
uint8_t AES_Init(void);
uint8_t AES_ChangeKey(uint32_t *src);
uint8_t AES_Encrypt(uint8_t *dst, const uint8_t *src, uint16_t Sz);
uint8_t AES_Decrypt(uint8_t *dst, const uint8_t *src, uint16_t Sz);
uint8_t AES_EE_Key(const aes_key_t);

uint32_t AES_IO_QuarterKey(void);
#endif /* INC_DRIVERS__AES_H_ */
