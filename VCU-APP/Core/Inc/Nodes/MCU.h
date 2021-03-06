/*
 * MCU.h
 *
 *  Created on: May 10, 2020
 *      Author: Pudja Mansyurin
 */

#ifndef INC_NODES_MCU_H_
#define INC_NODES_MCU_H_

/* Includes
 * --------------------------------------------*/
#include "Drivers/can.h"
#include "Libs/hbar.h"

/* Exported constants
 * --------------------------------------------*/
#define MCU_TIMEOUT_MS ((uint32_t)4000)
#define MCU_AUDIO_MAX_SPEED_KPH ((uint8_t)30)

/* Exported enums
 * --------------------------------------------*/
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
  MTP_RPM_MAX = 128,
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

/* Exported types
 * --------------------------------------------*/
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
  mcu_template_t tpl[HBMS_DRIVE_MAX];
} mcu_templates_t;

typedef struct {
  int16_t rpm_max;
  mcu_template_t tpl[HBMS_DRIVE_MAX];
} mcu_param_t;

typedef struct {
  uint8_t overheat;
  uint32_t tick;

  uint8_t active;
  uint8_t run;
  uint8_t reverse;
  HBMS_DRIVE drive_mode;
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
    uint8_t rpm_max;
    uint8_t template;
    mcu_param_t par;
  } set;
  struct {
    uint8_t rpm_max;
    uint8_t template;
  } synced;
} mcu_t;

/* Exported variables
 * --------------------------------------------*/
extern mcu_t MCU;

/* Public functions prototype
 * --------------------------------------------*/
void MCU_Init(void);
void MCU_Refresh(void);
void MCU_Power12v(uint8_t on);
void MCU_PowerOverCAN(uint8_t on);
void MCU_SetSpeedMax(uint8_t speed_max);
void MCU_SetTemplates(mcu_templates_t t);
void MCU_SyncCAN(void);
uint8_t MCU_RpmToSpeed(int16_t rpm);
int16_t MCU_SpeedToRpm(uint8_t speed);
uint8_t MCU_SpeedToVolume(void);
uint8_t MCU_GetMileage(uint16_t duration);
uint8_t MCU_Reversed(void);
uint8_t MCU_Running(void);
void MCU_RX_CurrentDC(can_rx_t *Rx);
void MCU_RX_VoltageDC(can_rx_t *Rx);
void MCU_RX_TorqueSpeed(can_rx_t *Rx);
void MCU_RX_FaultCode(can_rx_t *Rx);
void MCU_RX_State(can_rx_t *Rx);
void MCU_RX_Template(can_rx_t *Rx);
uint8_t MCU_TX_Setting(uint8_t on, uint8_t reverse);
uint8_t MCU_TX_Template(uint16_t param, uint8_t write, int16_t data);

#endif /* INC_NODES_MCU_H_ */
