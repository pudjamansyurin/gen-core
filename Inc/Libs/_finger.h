/*
 * _finger.h
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

#ifndef FINGER_H_
#define FINGER_H_

#include "_fz3387.h"

#define FINGER_CONFIDENCE_MIN 	10
#define FINGER_SCAN_TIMEOUT			20				// in second

void Finger_On(void);
void Finger_Off(void);
void Finger_Init(void);
uint8_t Finger_Enroll(uint8_t id);
uint8_t Finger_Delete_ID(uint8_t id);
uint8_t Finger_Empty_Database(void);
uint8_t Finger_Set_Password(uint32_t password);
int8_t Finger_Auth(void);
int8_t Finger_Auth_Fast(void);

#endif /* FINGER_H_ */
