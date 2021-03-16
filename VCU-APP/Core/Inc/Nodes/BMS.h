/*
 * BMS.h
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_BMS_H_
#define INC_NODES_BMS_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define BMS_COUNT                     (uint8_t) 2
#define BMS_ID_NONE 	             (uint32_t) 0xFFFFFFFF
#define BMS_ID_MASK                  (uint32_t) 0xFFFFF

/* Exported enum ---------------------------------------------------------------*/
typedef enum {
	BMS_STATE_OFF = -1,
	BMS_STATE_IDLE = 0,
	BMS_STATE_DISCHARGE,
	BMS_STATE_CHARG,
	BMS_STATE_FULL
} BMS_STATE;

typedef enum {
	BMS_SCALE_15_85 = 0,
	BMS_SCALE_20_80,
	BMS_SCALE_10_90,
	BMS_SCALE_0_100,
} BMS_SCALE;

/* Exported struct -------------------------------------------------------------*/
typedef struct {
	uint8_t soc;
	uint8_t run;
	uint8_t overheat;
	uint8_t error;
	struct {
		uint32_t id;
		float voltage;
		float current;
		float soc;
		float temperature;
		float capacity;
		float soh;
		uint16_t cycle;
		uint16_t flag;

		BMS_STATE state;
		uint32_t tick;
	} pack[BMS_COUNT ];
} bms_data_t;

typedef struct {
	struct {
		void (*Param1)(can_rx_t*);
		void (*Param2)(can_rx_t*);
	} r;
	struct {
		uint8_t (*Setting)(BMS_STATE, uint8_t);
	} t;
} bms_can_t;

typedef struct {
	bms_data_t d;
	bms_can_t can;
	void (*Init)(void);
	void (*PowerOverCan)(uint8_t);
	void (*RefreshIndex)(void);
} bms_t;

/* Exported variables ---------------------------------------------------------*/
extern bms_t BMS;

/* Public functions implementation --------------------------------------------*/
void BMS_Init(void);
void BMS_PowerOverCan(uint8_t on);
void BMS_RefreshIndex(void);

void BMS_CAN_RX_Param1(can_rx_t *Rx);
void BMS_CAN_RX_Param2(can_rx_t *Rx);
uint8_t BMS_CAN_TX_Setting(BMS_STATE state, uint8_t recover);

#endif /* INC_NODES_BMS_H_ */
