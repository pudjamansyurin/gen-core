/*
 * BMS.h
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_BMS_H_
#define INC_NODES_BMS_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Exported constants --------------------------------------------------------*/
#define BMS_COUNT                               2
#define BMS_ID_NONE 	                          0xFFFFFFFF
#define BMS_ID_MASK                             0xFFFFF

/* Exported enum ----------------------------------------------------------------*/
typedef enum {
	BMS_STATE_IDLE = 0,
	BMS_STATE_DISCHARGE = 1,
	BMS_STATE_CHARGE = 2,
	BMS_STATE_FULL = 3
} BMS_STATE;

/* Exported struct -------------------------------------------------------------*/
typedef struct {
	uint8_t started;
	uint8_t soc;
	struct {
		uint32_t id;
		float voltage;
		float current;
		float soc;
		float temperature;
		uint16_t flag;
		BMS_STATE state;
		uint8_t started;
		uint32_t tick;
	} pack[BMS_COUNT];
} bms_data_t;

typedef struct {
	struct {
		void (*Param1)(void);
		void (*Param2)(void);
	} r;
	struct {
		uint8_t (*Setting)(uint8_t, BMS_STATE);
	} t;
} bms_can_t;

typedef struct {
	bms_data_t d;
	bms_can_t can;
	void (*Init)(void);
	void (*PowerOverCan)(uint8_t);
	void (*ResetIndex)(uint8_t);
	void (*RefreshIndex)(void);
	uint8_t (*GetIndex)(uint32_t);
	void (*SetEvents)(uint16_t);
	uint8_t (*CheckRun)(uint8_t);
	uint8_t (*CheckState)(BMS_STATE);
	void (*MergeData)(void);
} bms_t;

/* Public functions implementation --------------------------------------------*/
void BMS_Init(void);
void BMS_PowerOverCan(uint8_t on);
void BMS_ResetIndex(uint8_t i);
void BMS_RefreshIndex(void);
uint8_t BMS_GetIndex(uint32_t id);
void BMS_SetEvents(uint16_t flag);
uint8_t BMS_CheckRun(uint8_t state);
uint8_t BMS_CheckState(BMS_STATE state);
void BMS_MergeData(void);

void BMS_CAN_RX_Param1(void);
void BMS_CAN_RX_Param2(void);
uint8_t BMS_CAN_TX_Setting(uint8_t start, BMS_STATE state);

#endif /* INC_NODES_BMS_H_ */
