/*
 * VCU.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_VCU_H_
#define INC_NODES_VCU_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Libs/_gyro.h"
#include "Libs/_gps.h"

/* Exported define ------------------------------------------------------------*/
typedef enum {
	EV_VCU_NET_SOFT_RESET 					 			= 0,
	EV_VCU_NET_HARD_RESET,
	EV_VCU_REMOTE_MISSING,
	EV_VCU_BIKE_FALLEN,
	EV_VCU_BIKE_MOVED,

  EV_BMS_DISCHARGE_OVER_CURRENT         = 50,
  EV_BMS_CHARGE_OVER_CURRENT,
  EV_BMS_SHORT_CIRCUIT,
  EV_BMS_DISCHARGE_OVER_TEMPERATURE,
  EV_BMS_DISCHARGE_UNDER_TEMPERATURE,
  EV_BMS_CHARGE_OVER_TEMPERATURE,
  EV_BMS_CHARGE_UNDER_TEMPERATURE,
  EV_BMS_UNDER_VOLTAGE,
  EV_BMS_OVER_VOLTAGE,
  EV_BMS_OVER_DISCHARGE_CAPACITY,
  EV_BMS_UNBALANCE,
  EV_BMS_SYSTEM_FAILURE,
} EVENTS_GROUP_BIT;

/* Exported struct --------------------------------------------------------------*/
typedef struct __attribute__((packed)) {
	uint8_t wakeup;
	uint16_t stack;
} task_t;

typedef struct __attribute__((packed)) {
	task_t manager;
	task_t iot;
	task_t reporter;
	task_t command;
	task_t gps;
	task_t gyro;
	task_t remote;
	task_t finger;
	task_t audio;
	task_t gate;
	task_t canRx;
	task_t canTx;
	//  task_t hmi2Power;
} rtos_task_t;

typedef struct {
	uint8_t driver_id;
	uint16_t interval;
	uint16_t bat;
	uint64_t events;
	uint32_t uptime;
	struct {
		uint8_t error;
		uint8_t override;
		vehicle_state_t vehicle;
	} state;
	struct {
		//    uint8_t power5v;
		struct {
			uint32_t tick;
		} starter;
	} gpio;
	struct {
		uint32_t independent;
	} tick;
	motion_t motion;
	gps_data_t gps;
	rtos_task_t task;
} vcu_data_t;

typedef struct {
	struct {
		uint8_t (*SwitchModeControl)(void);
		uint8_t (*Datetime)(datetime_t);
		uint8_t (*MixedData)(void);
		uint8_t (*TripData)(void);
	} t;
} vcu_can_t;

typedef struct {
	vcu_data_t d;
	vcu_can_t can;
	void (*Init)(void);
	void (*SetEvent)(uint8_t, uint8_t);
	uint8_t (*ReadEvent)(uint8_t);
	void (*SetDriver)(uint8_t);
	void (*SetOdometer)(uint8_t);
} vcu_t;

/* Exported variables ---------------------------------------------------------*/
extern vcu_t VCU;

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void);
void VCU_SetEvent(uint8_t bit, uint8_t value);
uint8_t VCU_ReadEvent(uint8_t bit);
void VCU_SetDriver(uint8_t driver_id);
void VCU_SetOdometer(uint8_t meter);

uint8_t VCU_CAN_TX_SwitchModeControl(void);
uint8_t VCU_CAN_TX_Datetime(datetime_t dt);
uint8_t VCU_CAN_TX_MixedData(void);
uint8_t VCU_CAN_TX_TripData(void);

#endif /* INC_NODES_VCU_H_ */
