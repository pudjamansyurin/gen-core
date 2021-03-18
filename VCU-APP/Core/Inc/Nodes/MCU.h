/*
 * MCU.h
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_MCU_H_
#define INC_NODES_MCU_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"
#include "Libs/_utils.h"
#include "Libs/_handlebar.h"

/* Exported constants --------------------------------------------------------*/
#define MCU_TIMEOUT    							 (uint32_t) 1000					// ms

/* Exported enum ------------------------------------------------------------*/
typedef enum {
	INV_DISCHARGE_DISABLED = 0x000,
	INV_DISCHARGE_ENABLED,
	INV_DISCHARGE_CHECK,
	INV_DISCHARGE_OCCURING,
	INV_DISCHARGE_COMPLETED,
} MCU_INV_DISCHARGE;

typedef enum {
	MPF_HW_DESATURATION = 0,
	MPF_HW_OVER_CURRENT,
	MPF_ACCEL_SHORTED,
	MPF_ACCEL_OPEN,
	MPF_CURRENT_L,
	MPF_CURRENT_H,
	MPF_MOD_TEMP_L,
	MPF_MOD_TEMP_H,
	MPF_PCB_TEMP_L,
	MPF_PCB_TEMP_H,
	MPF_GATE_TEMP_L,
	MPF_GATE_TEMP_H,
	MPF_5V_L,
	MPF_5V_H,
	MPF_12V_L,
	MPF_12V_H,
	MPF_2v5_L,
	MPF_2v5_H,
	MPF_1v5_L,
	MPF_1v5_H,
	MPF_DCBUS_VOLT_H,
	MPF_DCBUS_VOLT_L,
	MPF_PRECHARGE_TO,
	MPF_PRECHARGE_FAIL,
	MPF_EE_CHECKSUM_INVALID,
	MPF_EE_DATA_OUT_RANGE,
	MPF_EE_UPDATE_REQ,
	MPF_RESERVED_1,
	MPF_RESERVED_2,
	MPF_RESERVED_3,
	MPF_BRAKE_SHORTED,
	MPF_BRAKE_OPEN
} MCU_POST_FAULT_BIT;

typedef enum {
	MRF_OVER_SPEED			= 0,
	MRF_OVER_CURRENT,
	MRF_OVER_VOLTAGE,
	MRF_INV_OVER_TEMP,
	MRF_ACCEL_SHORTED,
	MRF_ACCEL_OPEN,
	MRF_DIRECTION_FAULT,
	MRF_INV_TO,
	MRF_HW_DESATURATION,
	MRF_HW_OVER_CURRENT,
	MRF_UNDER_VOLTAGE,
	MRF_CAN_LOST,
	MRF_MOTOR_OVER_TEMP,
	MRF_RESERVER_1,
	MRF_RESERVER_2,
	MRF_RESERVER_3,
	MRF_BRAKE_SHORTED,
	MRF_BRAKE_OPEN,
	MRF_MODA_OVER_TEMP,
	MRF_MODB_OVER_TEMP,
	MRF_MODC_OVER_TEMP,
	MRF_PCB_OVER_TEMP,
	MRF_GATE1_OVER_TEMP,
	MRF_GATE2_OVER_TEMP,
	MRF_GATE3_OVER_TEMP,
	MRF_CURRENT_FAULT,
	MRF_RESERVER_4,
	MRF_RESERVER_5,
	MRF_RESERVER_6,
	MRF_RESERVER_7,
	MRF_RESOLVER_FAULT,
	MRF_INV_DISCHARGE,
} MCU_RUN_FAULT_BIT;

/* Exported struct ------------------------------------------------------------*/
typedef struct {
//	uint8_t run;
	uint8_t overheat;
	uint8_t error;
	uint32_t tick;

	uint32_t rpm;
  uint8_t speed;
  uint8_t reverse;
	float temperature;
	HBAR_MODE_DRIVE drive_mode;
	struct {
		float commanded;
		float feedback;
	} torque;
	struct {
		uint32_t post;
		uint32_t run;
	} fault;
	struct {
		float current;
		float voltage;
	} dcbus;
	struct {
		uint8_t can_mode;
		uint8_t enabled;
		uint8_t lockout;
		MCU_INV_DISCHARGE discharge;
	} inv;
} mcu_data_t;

typedef struct {
	struct {
		void (*CurrentDC)(can_rx_t*);
		void (*VoltageDC)(can_rx_t*);
		void (*TorqueSpeed)(can_rx_t*);
		void (*FaultCode)(can_rx_t*);
		void (*State)(can_rx_t*);
	} r;
	struct {
		uint8_t (*Setting)(uint8_t);
	} t;
} mcu_can_t;

typedef struct {
	mcu_data_t d;
	mcu_can_t can;
	void (*Init)(void);
	void (*PowerOverCan)(uint8_t);
	void (*Refresh)(void);
	uint16_t (*SpeedToVolume)(void);
	uint16_t (*RpmToSpeed)(void);
} mcu_t;

/* Exported variables ---------------------------------------------------------*/
extern mcu_t MCU;

/* Public functions implementation --------------------------------------------*/
void MCU_Init(void);
void MCU_Refresh(void);
void MCU_PowerOverCan(uint8_t on);
uint16_t MCU_SpeedToVolume(void);
uint16_t MCU_RpmToSpeed(void);
void MCU_CAN_RX_CurrentDC(can_rx_t *Rx);
void MCU_CAN_RX_VoltageDC(can_rx_t *Rx);
void MCU_CAN_RX_TorqueSpeed(can_rx_t *Rx);
void MCU_CAN_RX_FaultCode(can_rx_t *Rx);
void MCU_CAN_RX_State(can_rx_t *Rx);
uint8_t MCU_CAN_TX_Setting(uint8_t on);

#endif /* INC_NODES_MCU_H_ */
