/*
 * _canbus.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef CAN_H_
#define CAN_H_

#include "_canbus.h"
#include "_config.h"

// CAN Nodes
#define CAN_NODE_VCU													SetBit(0)
#define CAN_NODE_MCU													SetBit(1)
#define CAN_NODE_BMS													SetBit(2)
#define CAN_NODE_HMI													SetBit(3)

// CAN Node Selector
#define CAN_NODE															(CAN_NODE_VCU)

// CAN Message Address
#define CAN_ADDR_VCU_SWITCH					 					0x000
#define CAN_ADDR_VCU_RTC						 					0x001
#define CAN_ADDR_VCU_SELECT_SET			 					0x002
#define CAN_ADDR_VCU_TRIP_MODE			 					0x003
#define CAN_ADDR_MCU_DUMMY										0x7A0
#define CAN_ADDR_BMS_DUMMY										0x7B0

// Others Parameters
#define MCU_SPEED_MAX 												200
#define MCU_RPM_MAX 													99999
#define VCU_ODOMETER_MAX											99999
#define HMI_DRIVE_MODE_MAX										3

// ==================================== VCU =========================================
#if (CAN_NODE & CAN_NODE_VCU)
#include "_rtc.h"
#include "_database.h"

uint8_t CAN_VCU_Switch(void);
uint8_t CAN_VCU_RTC(void);
uint8_t CAN_VCU_Select_Set(void);
uint8_t CAN_VCU_Trip_Mode(void);

void CAN_MCU_Dummy_Read(void);
#endif
#endif /* CAN_H_ */
