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
#include "Libs/_handlebar.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define MCU_TIMEOUT (uint32_t)3000 // ms

/* Exported enum ------------------------------------------------------------*/
typedef enum {
	MTP_1_DISCUR_MAX = 80,
	MTP_1_TORQUE_MAX,
	MTP_1_RBS_SWITCH,
	MTP_2_DISCUR_MAX,
	MTP_2_TORQUE_MAX,
	MTP_2_RBS_SWITCH,
	MTP_3_DISCUR_MAX,
	MTP_3_TORQUE_MAX,
	MTP_3_RBS_SWITCH,
	MTP_SPEED_MAX = 128,
} MCU_TEMPLATE_PARAM_ADDR;

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
	MPF_CURRENT_LOW,
	MPF_CURRENT_HIGH,
	MPF_MOD_TEMP_LOW,
	MPF_MOD_TEMP_HIGH,
	MPF_PCB_TEMP_LOW,
	MPF_PCB_TEMP_HIGH,
	MPF_GATE_TEMP_LOW,
	MPF_GATE_TEMP_HIGH,
	MPF_5V_LOW,
	MPF_5V_HIGH,
	MPF_12V_LOW,
	MPF_12V_HIGH,
	MPF_2v5_LOW,
	MPF_2v5_HIGH,
	MPF_1v5_LOW,
	MPF_1v5_HIGH,
	MPF_DCBUS_VOLT_HIGH,
	MPF_DCBUS_VOLT_LOW,
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
	MRF_OVER_SPEED = 0,
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

/* Exported struct
 * ------------------------------------------------------------*/
typedef struct {
	uint16_t discur_max;
	uint16_t torque_max;
	//	uint16_t rbs_switch;
} mcu_template_addr_t;

typedef struct __attribute__((packed)) {
	int16_t discur_max;
	int16_t torque_max;
	//	uint8_t rbs_switch;
} mcu_template_t;

typedef struct __attribute__((packed)) {
	mcu_template_t template[HBAR_M_DRIVE_MAX];
} mcu_templates_t;

typedef struct __attribute__((packed)) {
	uint8_t speed_max;
	mcu_template_t template[HBAR_M_DRIVE_MAX];
} mcu_param_t;

typedef struct {
	uint8_t run;
	uint8_t active;
	uint8_t overheat;
	uint8_t error;
	uint32_t tick;

	HBAR_MODE_DRIVE drive_mode;
	uint8_t reverse;
	int16_t rpm;
	float temperature;
	struct {
		uint32_t post;
		uint32_t run;
	} fault;
	struct {
		float commanded;
		float feedback;
	} torque;
	struct {
		float current;
		float voltage;
	} dcbus;
	struct {
		uint8_t enabled;
		uint8_t lockout;
		MCU_INV_DISCHARGE discharge;
	} inv;
	mcu_param_t par;
} mcu_data_t;

typedef struct {
	mcu_data_t d;
	struct {
		void (*CurrentDC)(can_rx_t *);
		void (*VoltageDC)(can_rx_t *);
		void (*TorqueSpeed)(can_rx_t *);
		void (*FaultCode)(can_rx_t *);
		void (*State)(can_rx_t *);
		void (*Template)(can_rx_t *);
	} r;
	struct {
		uint8_t (*Setting)(uint8_t);
		uint8_t (*Template)(uint16_t, uint8_t, int16_t);
	} t;
	void (*Init)(void);
	void (*PowerOverCan)(uint8_t);
	void (*Refresh)(void);
	void (*GetTemplates)(void);
	void (*SetTemplates)(mcu_template_t[3]);
	void (*GetSpeedMax)(void);
	void (*SetSpeedMax)(uint8_t max);
	uint16_t (*RpmToSpeed)(void);
	uint16_t (*SpeedToVolume)(void);
} mcu_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern mcu_t MCU;

/* Public functions implementation
 * --------------------------------------------*/
void MCU_Init(void);
void MCU_Refresh(void);
void MCU_PowerOverCan(uint8_t on);
void MCU_SetMockTemplates(void);
void MCU_GetTemplates(void);
void MCU_SetTemplates(mcu_template_t templates[3]);
void MCU_GetSpeedMax(void);
void MCU_SetSpeedMax(uint8_t max);
uint16_t MCU_RpmToSpeed(void);
uint16_t MCU_SpeedToVolume(void);
void MCU_RX_CurrentDC(can_rx_t *Rx);
void MCU_RX_VoltageDC(can_rx_t *Rx);
void MCU_RX_TorqueSpeed(can_rx_t *Rx);
void MCU_RX_FaultCode(can_rx_t *Rx);
void MCU_RX_State(can_rx_t *Rx);
void MCU_RX_Template(can_rx_t *Rx);
uint8_t MCU_TX_Setting(uint8_t on);
uint8_t MCU_TX_Template(uint16_t param, uint8_t write, int16_t data);

#endif /* INC_NODES_MCU_H_ */
