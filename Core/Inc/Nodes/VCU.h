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
	vcu_data_t d;
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

#endif /* INC_NODES_VCU_H_ */
