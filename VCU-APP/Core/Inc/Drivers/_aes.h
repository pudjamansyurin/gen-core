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

/* Typedef -------------------------------------------------------------------*/
typedef struct {
    CRYP_HandleTypeDef *hcryp;
} aes_handler_t;

/* Exported variables -------------------------------------------------------*/
extern uint32_t AesKey[4];

/* Public functions implementation -------------------------------------------*/
uint8_t AES_Init(CRYP_HandleTypeDef *hcryp);
uint8_t AES_Encrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz);
uint8_t AES_Decrypt(uint8_t *pDst, uint8_t *pSrc, uint16_t Sz);
void AES_Tester(void);

#endif /* INC_LIBS__AES_H_ */
