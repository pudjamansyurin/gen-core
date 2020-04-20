/*
 * _canbus.h
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

#ifndef CAN_H_
#define CAN_H_

/* Includes ------------------------------------------------------------------*/
#include "_handlebar.h"
#include "_canbus.h"
#include "_rtc.h"

/* Exported constants --------------------------------------------------------*/
// CAN Message Address
#define CAND_VCU_SWITCH					 			    0x000
#define CAND_VCU_RTC						 			0x001
#define CAND_VCU_SELECT_SET			 					0x002
#define CAND_VCU_TRIP_MODE			 					0x003

#define CAND_BMS_PARAM_1                                0x0B0
#define CAND_BMS_PARAM_2                                0x0B1
#define CAND_BMS_BATTERY_ID                             0x0B2

#define CAND_MCU_DUMMY									0x7A0

#define CAND_HMI1_LEFT                                  0x7C0
#define CAND_HMI1_RIGHT                                 0x7C1
#define CAND_HMI2                                       0x7D0

/* Public functions prototype ------------------------------------------------*/
uint8_t CANT_VCU_Switch(db_t *db, sw_t *sw);
uint8_t CANT_VCU_RTC(timestamp_t *timestamp);
uint8_t CANT_VCU_Select_Set(sw_runner_t *runner);
uint8_t CANT_VCU_Trip_Mode(uint32_t *trip);
void CANR_BMS_Param1(db_t *db);
void CANR_BMS_BatteryID(db_t *db);
void CANR_MCU_Dummy(db_t *db);
void CANR_HMI2(db_t *db);

#endif /* CAN_H_ */
