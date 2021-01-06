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
#include "Libs/_utils.h"
#include "Libs/_handlebar.h"
#include "Libs/_gyro.h"
#include "Libs/_gps.h"

/* Exported enums --------------------------------------------------------------*/
typedef enum {
  VEHICLE_UNKNOWN = -3,
  VEHICLE_LOST = -2,
  VEHICLE_BACKUP = -1,
  VEHICLE_NORMAL = 0,
  VEHICLE_STANDBY = 1,
  VEHICLE_READY = 2,
  VEHICLE_RUN = 3,
} vehicle_state_t;

/* Exported struct --------------------------------------------------------------*/
typedef struct __attribute__((packed)) {
  uint8_t wakeup;
  uint16_t stack;
} task_t;

typedef struct __attribute__((packed)) {
  task_t manager;
  task_t iot;
  task_t reporter;
  task_t command;
  task_t gps;
  task_t gyro;
  task_t remote;
  task_t finger;
  task_t audio;
  task_t gate;
  task_t canRx;
  task_t canTx;
  task_t hmi2Power;
} rtos_task_t;

typedef struct {
  uint32_t unit_id;
  uint8_t driver_id;
  uint16_t interval;
  uint8_t speed;
  uint16_t bat;
  uint32_t odometer;
  motion_t motion;
  rtc_t rtc;
  uint64_t events;
  struct {
    uint8_t error;
    uint8_t override;
    vehicle_state_t vehicle;
  } state;
  struct {
    uint16_t report;
    uint16_t response;
  } seq_id;
  struct {
    uint8_t knob;
    uint8_t power5v;
    uint8_t starter;
  } gpio;
  struct {
    uint32_t independent;
  } tick;
  gps_t gps;
  rtos_task_t task;
} vcu_data_t;

typedef struct {
  struct {
    uint8_t (*SwitchModeControl)(hbar_t*);
    uint8_t (*Datetime)(timestamp_t*);
    uint8_t (*MixedData)(hbar_runner_t*);
    uint8_t (*SubTripData)(uint32_t*);
  } t;
} vcu_can_t;

typedef struct {
  vcu_data_t d;
  vcu_can_t can;
  void (*Init)(void);
  void (*SetEvent)(uint64_t, uint8_t);
  uint8_t (*ReadEvent)(uint64_t);
  uint16_t (*SpeedToVolume)(void);
  void (*SetDriver)(uint8_t);
  void (*SetOdometer)(uint8_t);
} vcu_t;

/* Exported variables ---------------------------------------------------------*/
extern vcu_t VCU;

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void);
void VCU_SetEvent(uint64_t event_id, uint8_t value);
uint8_t VCU_ReadEvent(uint64_t event_id);
uint16_t VCU_SpeedToVolume(void);
void VCU_SetDriver(uint8_t driver_id);
void VCU_SetOdometer(uint8_t increment);
void VCU_CheckVehicleState(void);

uint8_t VCU_CAN_TX_SwitchModeControl(hbar_t *hbar);
uint8_t VCU_CAN_TX_Datetime(timestamp_t *timestamp);
uint8_t VCU_CAN_TX_MixedData(hbar_runner_t *runner);
uint8_t VCU_CAN_TX_SubTripData(uint32_t *trip);

#endif /* INC_NODES_VCU_H_ */
