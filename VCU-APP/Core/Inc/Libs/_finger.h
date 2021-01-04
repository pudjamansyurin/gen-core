/*
 * _finger.h
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

#ifndef FINGER_H_
#define FINGER_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_fz3387.h"
#include "Libs/_utils.h"

/* Public functions prototype ------------------------------------------------*/
void Finger_Init(void);
uint8_t Finger_Enroll(uint8_t id);
uint8_t Finger_DeleteID(uint8_t id);
uint8_t Finger_EmptyDatabase(void);
uint8_t Finger_SetPassword(uint32_t password);
int8_t Finger_Auth(void);
int8_t Finger_AuthFast(void);

#endif /* FINGER_H_ */
