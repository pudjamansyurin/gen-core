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

/* Exported typedef
 * --------------------------------------------*/
typedef uint32_t aes_key_t[4];

/* Exported variables
 * --------------------------------------------*/
extern aes_key_t AES_KEY;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t AES_Init(void);
uint8_t AES_ChangeKey(uint32_t *key);
uint8_t AES_Encrypt(uint8_t *dst, uint8_t *src, uint16_t Sz);
uint8_t AES_Decrypt(uint8_t *dst, uint8_t *src, uint16_t Sz);
uint8_t AES_EE_Key(aes_key_t);

#endif /* INC_DRIVERS__AES_H_ */
