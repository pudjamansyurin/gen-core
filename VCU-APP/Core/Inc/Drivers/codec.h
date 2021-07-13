/*
 * codec.h
 *
 *  Created on: Mar 8, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_DRIVERS__CODEC_H_
#define INC_DRIVERS__CODEC_H_

/* Includes
 * --------------------------------------------*/
#include "App/util.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t CODEC_Init(uint8_t Addr);
void CODEC_DeInit(void);
uint8_t CODEC_Write(uint8_t Reg, uint8_t Value);
uint8_t CODEC_Read(uint8_t Reg);

#endif /* INC_DRIVERS__CODEC_H_ */
