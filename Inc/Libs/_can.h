/*
 * _canbus.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef CAN_H_
#define CAN_H_

/* Includes ------------------------------------------------------------------*/
#include "_canbus.h"
#include "_rtc.h"

// CAN Message Address
#define CAN_ADDR_VCU_SWITCH					 			    0x000
#define CAN_ADDR_VCU_RTC						 			0x001
#define CAN_ADDR_VCU_SELECT_SET			 					0x002
#define CAN_ADDR_VCU_TRIP_MODE			 					0x003
#define CAN_ADDR_MCU_DUMMY									0x7A0
#define CAN_ADDR_BMS_DUMMY									0x7B0

/* Public functions prototype ------------------------------------------------*/

uint8_t CAN_VCU_Switch(db_t *db);
uint8_t CAN_VCU_RTC(timestamp_t *timestamp);
uint8_t CAN_VCU_Select_Set(sw_runner_t *runner);
uint8_t CAN_VCU_Trip_Mode(uint32_t *trip);
void CAN_MCU_Dummy_Read(db_t *db);

#endif /* CAN_H_ */
