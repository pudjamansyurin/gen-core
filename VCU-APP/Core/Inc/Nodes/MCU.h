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
	DRIVE_MODE_EFFICIENT = 0x00,
	DRIVE_MODE_STANDARD,
	DRIVE_MODE_ECONOMIC
} MCU_DRIVE_MODE;

typedef enum {
	MPF_HW_DESATURATION = 0,
	MPF_HW_OVER_CURRENT,
	MPF_ACCEL_SHORTED,
	MPF_ACCEL_OPEN,
	MPF_CURRENT_L,
	MPF_CURRENT_H,
	MPF_MODULE_TEMP_L,
	MPF_MODULE_TEMP_H,
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
	MRF_OVER_SPEED			= 0x00,
//	Motor Over-speed Fault
//	Over-current Fault
//	Over-voltage Fault
//	Inverter Over-temperature Fault
//	Accelerator Input Shorted Fault
//	Accelerator Input Open Fault
//	Direction Command Fault
//	Inverter Response Time-out Fault
//	Hardware Gate/Desaturation Fault
//	Hardware Over-current Fault
//	Under-voltage Fault
//	CAN Command Message Lost Fault
//	Motor Over-temperature Fault
//	Reserved
//	Reserved
//	Reserved
//	Brake Input Shorted Fault
//	Brake Input Open Fault
//	Module A Over-temperature Fault
//	Module B Over-temperature Fault
//	Module C Over-temperature Fault
//	PCB Over-temperature Fault
//	Gate Drive Board 1 Over-temperature Fault
//	Gate Drive Board 2 Over-temperature Fault
//	Gate Drive Board 3 Over-temperature Fault
//	Current Sensor Fault
//	Reserved
//	Reserved
//	Reserved
//	Reserved
//	Resolver Not Connected
//	Inverter Discharge Active

} MCU_RUN_FAULT_BIT;

/* Exported struct ------------------------------------------------------------*/
typedef struct {
	uint32_t rpm;
  uint8_t speed;
  uint8_t reverse;
	float temperature;
	MCU_DRIVE_MODE drive_mode;
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

	uint32_t tick;
	uint8_t run;
} mcu_data_t;

typedef struct {
	struct {
		void (*CurrentDC)(can_rx_t*);
		void (*VoltageDC)(can_rx_t*);
		void (*TorqueSpeed)(can_rx_t*);
		void (*FaultCode)(can_rx_t*);
		void (*State)(can_rx_t*);
	} r;
} mcu_can_t;

typedef struct {
	mcu_data_t d;
	mcu_can_t can;
	void (*Init)(void);
	void (*Refresh)(void);
	uint16_t (*SpeedToVolume)(void);
} mcu_t;

/* Exported variables ---------------------------------------------------------*/
extern mcu_t MCU;

/* Public functions implementation --------------------------------------------*/
void MCU_Init(void);
void MCU_Refresh(void);
uint16_t MCU_SpeedToVolume(void);
void MCU_CAN_RX_CurrentDC(can_rx_t *Rx);
void MCU_CAN_RX_VoltageDC(can_rx_t *Rx);
void MCU_CAN_RX_TorqueSpeed(can_rx_t *Rx);
void MCU_CAN_RX_FaultCode(can_rx_t *Rx);
void MCU_CAN_RX_State(can_rx_t *Rx);

#endif /* INC_NODES_MCU_H_ */
