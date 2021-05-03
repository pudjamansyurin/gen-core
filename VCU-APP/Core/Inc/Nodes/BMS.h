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
#define BMS_AVG_SZ ((uint8_t)10)

#define BMS_COUNT 2
#define BMS_TIMEOUT_MS ((uint32_t)4000)
#define BMS_ID_NONE ((uint32_t)0xFFFFFFFF)

#define BMS_CAND(_X_) ((_X_) >> 20)
#define BMS_ID(_X_) ((_X_)&0xFFFFF)

/* Exported enum
 * ---------------------------------------------------------------*/
typedef enum {
	BMS_STATE_OFF = -1,
	BMS_STATE_IDLE = 0,
	BMS_STATE_DISCHARGE,
	BMS_STATE_CHARGE,
	BMS_STATE_FULL = 1
} BMS_STATE;

typedef enum {
	BMS_SCALE_15_85 = 0,
	BMS_SCALE_20_80,
	BMS_SCALE_10_90,
	BMS_SCALE_0_100,
} BMS_SCALE;

typedef enum {
	BMSF_DISCHARGE_OVER_CURRENT = 0,
	BMSF_CHARGE_OVER_CURRENT,
	BMSF_SHORT_CIRCUIT,
	BMSF_DISCHARGE_OVER_TEMPERATURE,
	BMSF_DISCHARGE_UNDER_TEMPERATURE,
	BMSF_CHARGE_OVER_TEMPERATURE,
	BMSF_CHARGE_UNDER_TEMPERATURE,
	BMSF_UNDER_VOLTAGE,
	BMSF_OVER_VOLTAGE,
	BMSF_OVER_DISCHARGE_CAPACITY,
	BMSF_UNBALANCE,
	BMSF_SYSTEM_FAILURE,
} BMS_FAULT_BIT;

/* Exported struct
 * -------------------------------------------------------------*/
typedef struct {
	uint8_t run;
	uint8_t active;
	uint32_t id;
	float voltage;
	float current;
	float soc;
	float temperature;
	float capacity;
	float soh;
	uint16_t cycle;
	uint16_t fault;
	BMS_STATE state;
	uint32_t tick;
} pack_t;

typedef struct {
	uint8_t run;
	uint8_t active;
	uint8_t overheat;
	uint16_t fault;
	uint8_t soc;
	float wh;
	float m_wh;
	float buf[BMS_AVG_SZ];
} bms_data_t;

typedef struct {
	bms_data_t d;
	pack_t packs[BMS_COUNT];
	struct {
		void (*Param1)(can_rx_t *);
		void (*Param2)(can_rx_t *);
	} r;
	struct {
		uint8_t (*Setting)(BMS_STATE, uint8_t);
	} t;
	void (*Init)(void);
	void (*PowerOverCan)(uint8_t);
	void (*RefreshIndex)(void);
	uint8_t (*MinIndex)(void);
	float (*GetMinWH)(void);
	float (*GetMPerWH)(uint32_t);
} bms_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern bms_t BMS;

/* Public functions implementation
 * --------------------------------------------*/
void BMS_Init(void);
void BMS_PowerOverCan(uint8_t on);
void BMS_RefreshIndex(void);
uint8_t BMS_MinIndex(void);
float BMS_GetMinWH(void);
float BMS_GetMPerWH(uint32_t odo);

void BMS_RX_Param1(can_rx_t *Rx);
void BMS_RX_Param2(can_rx_t *Rx);
uint8_t BMS_TX_Setting(BMS_STATE state, uint8_t sc);

#endif /* INC_NODES_BMS_H_ */
