/*
 * _task.h
 *
 *  Created on: Jun 22, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__TASK_H_
#define INC_APP__TASK_H_

/* Includes
 * --------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants
 * --------------------------------------------*/
#define MANAGER_WAKEUP_MS ((uint16_t)555)

#define EVENT_READY BIT(0)

#define FLAG_NET_REPORT_DISCARD BIT(0)
#define FLAG_NET_SEND_USSD BIT(1)
#define FLAG_NET_READ_SMS BIT(2)

#define FLAG_REPORTER_YIELD BIT(0)
#define FLAG_REPORTER_FLUSH BIT(1)

#define FLAG_COMMAND_ERROR BIT(0)
#define FLAG_COMMAND_OK BIT(1)

#define FLAG_MEMS_TASK_START BIT(0)
#define FLAG_MEMS_TASK_STOP BIT(1)
#define FLAG_MEMS_DETECTOR_RESET BIT(2)
#define FLAG_MEMS_DETECTOR_TOGGLE BIT(3)

#define FLAG_REMOTE_TASK_START BIT(0)
#define FLAG_REMOTE_TASK_STOP BIT(1)
#define FLAG_REMOTE_RESET BIT(2)
#define FLAG_REMOTE_RX_IT BIT(3)
#define FLAG_REMOTE_PAIRING BIT(4)

#define FLAG_FINGER_TASK_START BIT(0)
#define FLAG_FINGER_TASK_STOP BIT(1)
#define FLAG_FINGER_PLACED BIT(2)
#define FLAG_FINGER_FETCH BIT(3)
#define FLAG_FINGER_ADD BIT(4)
#define FLAG_FINGER_DEL BIT(5)
#define FLAG_FINGER_RST BIT(6)

#define FLAG_AUDIO_TASK_START BIT(0)
#define FLAG_AUDIO_TASK_STOP BIT(1)
#define FLAG_AUDIO_BEEP BIT(2)

#define FLAG_CAN_TASK_START BIT(0)
#define FLAG_CAN_TASK_STOP BIT(1)

#define FLAG_GATE_HBAR BIT(0)
#define FLAG_GATE_OPEN_SEAT BIT(1)
#define FLAG_GATE_ALARM_HORN BIT(2)

/* Exported structs
 * --------------------------------------------*/
typedef struct __attribute__((packed)) {
  TickType_t manager;
  TickType_t network;
  TickType_t reporter;
  TickType_t command;
  TickType_t mems;
  TickType_t remote;
  TickType_t finger;
  TickType_t audio;
  TickType_t gate;
  TickType_t canRx;
  TickType_t canTx;
} tasks_tick_t;

typedef struct __attribute__((packed)) {
  uint16_t manager;
  uint16_t network;
  uint16_t reporter;
  uint16_t command;
  uint16_t mems;
  uint16_t remote;
  uint16_t finger;
  uint16_t audio;
  uint16_t gate;
  uint16_t canRx;
  uint16_t canTx;
} tasks_stack_t;

typedef struct __attribute__((packed)) {
  uint8_t manager;
  uint8_t network;
  uint8_t reporter;
  uint8_t command;
  uint8_t mems;
  uint8_t remote;
  uint8_t finger;
  uint8_t audio;
  uint8_t gate;
  uint8_t canRx;
  uint8_t canTx;
} tasks_wakeup_t;

typedef struct __attribute__((packed)) {
  tasks_tick_t tick;
  tasks_stack_t stack;
  tasks_wakeup_t wakeup;
} tasks_t;

/* External variables
 * --------------------------------------------*/
extern osThreadId_t ManagerTaskHandle;
extern osThreadId_t NetworkTaskHandle;
extern osThreadId_t ReporterTaskHandle;
extern osThreadId_t CommandTaskHandle;
extern osThreadId_t MemsTaskHandle;
extern osThreadId_t RemoteTaskHandle;
extern osThreadId_t FingerTaskHandle;
extern osThreadId_t AudioTaskHandle;
extern osThreadId_t GateTaskHandle;
extern osThreadId_t CanRxTaskHandle;
extern osThreadId_t CanTxTaskHandle;

/* Exported variables
 * --------------------------------------------*/
extern tasks_t TASKS;

/* Public functions prototype
 * --------------------------------------------*/
uint32_t TASK_WaitManager(void);
bool TASK_KernelFailed(void);
void TASK_CheckWakeup(void);
void TASK_CheckStack(void);

#endif /* INC_APP__TASK_H_ */
