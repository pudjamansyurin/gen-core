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
#include "Libs/_mems.h"

/* Exported enum
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
typedef struct {
	uint8_t error;
	uint16_t bat;
	uint16_t events;
	uint32_t uptime;
	uint16_t interval;
	vehicle_state_t state;
	struct {
		int8_t state;
		uint16_t interval;
		uint8_t full;
	} override;
	struct {
		uint32_t independent;
	} tick;
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
	void (*CheckState)(void);
	void (*SetEvent)(uint8_t, uint8_t);
	uint8_t (*ReadEvent)(uint8_t);
	uint8_t (*Is)(uint8_t);
	void (*SetOdometer)(uint8_t);
} vcu_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern vcu_t VCU;

/* Public functions implementation
 * --------------------------------------------*/
void VCU_Init(void);
void VCU_Refresh(void);
void VCU_CheckState(void);
void VCU_SetEvent(uint8_t bit, uint8_t value);
uint8_t VCU_ReadEvent(uint8_t bit);
uint8_t VCU_Is(uint8_t state);
void VCU_SetOdometer(uint8_t meter);

uint8_t VCU_TX_Heartbeat(void);
uint8_t VCU_TX_SwitchModeControl(void);
uint8_t VCU_TX_Datetime(datetime_t dt);
uint8_t VCU_TX_MixedData(void);
uint8_t VCU_TX_TripData(void);

#endif /* INC_NODES_VCU_H_ */
