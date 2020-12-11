/*
 * VCU.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_VCU_H_
#define INC_NODES_VCU_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Libs/_handlebar.h"
#include "Libs/_gyro.h"

/* Exported enums --------------------------------------------------------------*/
typedef enum {
  VEHICLE_INDEPENDENT = 0,
  VEHICLE_DOWN,
  VEHICLE_POWERED,
  VEHICLE_RUNNING,
} vehicle_state_t;

/* Exported struct --------------------------------------------------------------*/
typedef struct __attribute__((packed)) {
  uint32_t wakeup;
  uint32_t stack;
} task_t;

typedef struct __attribute__((packed)) {
  task_t manager;
  task_t iot;
  task_t reporter;
  task_t command;
  task_t gps;
  task_t gyro;
  task_t keyless;
  task_t finger;
  task_t audio;
  task_t switches;
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
  struct {
    int8_t pitch;
    int8_t roll;
  } motion;
  rtc_t rtc;
  uint64_t events;
  struct {
    //		vehicle_state_t vehicle;
    uint8_t start;
    uint8_t override;
    uint8_t run;
    uint8_t knob;
    uint8_t independent;
  } state;
  struct {
    uint32_t keyless;
  } tick;
  struct {
    uint16_t report;
    uint16_t response;
  } seq_id;
  rtos_task_t task;
} vcu_data_t;

typedef struct {
  struct {
    uint8_t (*SwitchModeControl)(sw_t*);
    uint8_t (*Datetime)(timestamp_t*);
    uint8_t (*MixedData)(sw_runner_t*);
    uint8_t (*SubTripData)(uint32_t*);
  } t;
} vcu_can_t;

typedef struct {
  vcu_data_t d;
  vcu_can_t can;
  void (*Init)(void);
  void (*SetEvent)(uint64_t, uint8_t);
  uint8_t (*ReadEvent)(uint64_t);
  void (*CheckPower5v)(void);
  void (*CheckKnob)(void);
  uint16_t (*SpeedToVolume)(void);
  void (*SetOdometer)(uint8_t);
} vcu_t;

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void);
void VCU_SetEvent(uint64_t event_id, uint8_t value);
uint8_t VCU_ReadEvent(uint64_t event_id);
void VCU_CheckPower5v(void);
void VCU_CheckKnob(void);
uint16_t VCU_SpeedToVolume(void);
void VCU_SetOdometer(uint8_t increment);

uint8_t VCU_CAN_TX_SwitchModeControl(sw_t *sw);
uint8_t VCU_CAN_TX_Datetime(timestamp_t *timestamp);
uint8_t VCU_CAN_TX_MixedData(sw_runner_t *runner);
uint8_t VCU_CAN_TX_SubTripData(uint32_t *trip);

#endif /* INC_NODES_VCU_H_ */
