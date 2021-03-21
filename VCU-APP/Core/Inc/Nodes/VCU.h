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
#include "Libs/_gps.h"
#include "Libs/_gyro.h"

#define TO_U8(_X_)				((_X_) > UINT8_MAX ? UINT8_MAX : (_X_))

/* Exported define
 * ------------------------------------------------------------*/
typedef enum {
	EVG_NET_SOFT_RESET = 0,
	EVG_NET_HARD_RESET,
	EVG_REMOTE_MISSING,
	EVG_BIKE_FALLEN,
	EVG_BIKE_MOVED,
	EVG_BMS_ERROR,
	EVG_MCU_ERROR
} EVENTS_GROUP_BIT;

/* Exported struct
 * --------------------------------------------------------------*/
typedef struct __attribute__((packed)) {
	TickType_t manager;
	TickType_t iot;
	TickType_t reporter;
	TickType_t command;
	TickType_t gps;
	TickType_t gyro;
	TickType_t remote;
	TickType_t finger;
	TickType_t audio;
	TickType_t gate;
	TickType_t canRx;
	TickType_t canTx;
	//  TickType_t hmi2Power;
} tasks_tick_t;

typedef struct __attribute__((packed)) {
	uint16_t manager;
	uint16_t iot;
	uint16_t reporter;
	uint16_t command;
	uint16_t gps;
	uint16_t gyro;
	uint16_t remote;
	uint16_t finger;
	uint16_t audio;
	uint16_t gate;
	uint16_t canRx;
	uint16_t canTx;
	//  uint16_t hmi2Power;
} tasks_stack_t;

typedef struct __attribute__((packed)) {
	uint8_t manager;
	uint8_t iot;
	uint8_t reporter;
	uint8_t command;
	uint8_t gps;
	uint8_t gyro;
	uint8_t remote;
	uint8_t finger;
	uint8_t audio;
	uint8_t gate;
	uint8_t canRx;
	uint8_t canTx;
	//  uint8_t hmi2Power;
} tasks_wakeup_t;

typedef struct __attribute__((packed)) {
	tasks_tick_t tick;
	tasks_stack_t stack;
	tasks_wakeup_t wakeup;
} tasks_t;

typedef struct {
	uint8_t error;
	uint8_t override;
	vehicle_state_t state;
	uint16_t bat;
	uint16_t events;
	uint32_t uptime;
	uint8_t driver_id;
	uint16_t interval;
	struct {
		uint32_t starter;
	} gpio;
	struct {
		uint32_t independent;
	} tick;
	motion_t motion;
	gps_data_t gps;
	tasks_t task;
} vcu_data_t;

typedef struct {
	vcu_data_t d;
	struct {
		uint8_t (*Heartbeat)(void);
		uint8_t (*SwitchModeControl)(void);
		uint8_t (*Datetime)(datetime_t);
		uint8_t (*MixedData)(void);
		uint8_t (*TripData)(void);
	} t;
	void (*Init)(void);
	void (*Refresh)(void);
	void (*NodesInit)(void);
	void (*NodesRefresh)(void);
	void (*CheckState)(void);
	uint8_t (*CheckRTOS)(void);
	void (*CheckTasks)(void);
	void (*SetEvent)(uint8_t, uint8_t);
	uint8_t (*ReadEvent)(uint8_t);
	void (*SetDriver)(uint8_t);
	void (*SetOdometer)(uint8_t);
} vcu_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern vcu_t VCU;

/* Public functions implementation
 * --------------------------------------------*/
void VCU_Init(void);
void VCU_Refresh(void);
void VCU_NodesInit(void);
void VCU_NodesRefresh(void);
void VCU_SetEvent(uint8_t bit, uint8_t value);
uint8_t VCU_ReadEvent(uint8_t bit);
void VCU_SetDriver(uint8_t driver_id);
void VCU_SetOdometer(uint8_t meter);
void VCU_CheckState(void);
uint8_t VCU_CheckRTOS(void);
void VCU_CheckTasks(void);

uint8_t VCU_TX_Heartbeat(void);
uint8_t VCU_TX_SwitchModeControl(void);
uint8_t VCU_TX_Datetime(datetime_t dt);
uint8_t VCU_TX_MixedData(void);
uint8_t VCU_TX_TripData(void);

#endif /* INC_NODES_VCU_H_ */
