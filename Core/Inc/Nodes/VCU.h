/*
 * VCU.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_VCU_H_
#define INC_NODES_VCU_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "_database.h"
#include "_handlebar.h"

/* Exported struct --------------------------------------------------------------*/
typedef struct {
	uint8_t knob;
	uint32_t unit_id;
	uint8_t independent;
	uint16_t interval;
	uint8_t volume;
	uint16_t bat_voltage;
	uint8_t signal_percent;
	uint8_t speed;
	uint32_t odometer;
	rtc_t rtc;
	uint64_t events;
	struct {
		uint32_t keyless;
	//      uint32_t finger;
	} tick;
	struct {
		uint16_t report;
		uint16_t response;
	} seq_id;
} vcu_data_t;

typedef struct {
	struct {
		uint8_t (*Switch)(sw_t*);
		uint8_t (*Datetime)(timestamp_t*);
		uint8_t (*SelectSet)(sw_runner_t*);
		uint8_t (*TripMode)(uint32_t*);
	} t;
} vcu_can_t;

typedef struct {
	vcu_data_t d;
	vcu_can_t can;
	void (*Init)(void);
	void (*SetEvent)(uint64_t, uint8_t);
	uint8_t (*ReadEvent)(uint64_t);
	void (*CheckKnob)(void);
	void (*CheckMainPower)(void);
} vcu_t;

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void);
void VCU_SetEvent(uint64_t event_id, uint8_t value);
uint8_t VCU_ReadEvent(uint64_t event_id);
void VCU_CheckMainPower(void);

uint8_t VCU_CAN_TX_Switch(sw_t *sw);
uint8_t VCU_CAN_TX_Datetime(timestamp_t *timestamp);
uint8_t VCU_CAN_TX_SelectSet(sw_runner_t *runner);
uint8_t VCU_CAN_TX_TripMode(uint32_t *trip);

#endif /* INC_NODES_VCU_H_ */
