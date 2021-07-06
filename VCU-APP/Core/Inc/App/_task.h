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
#include "App/_common.h"

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

/* Exported enums
 * --------------------------------------------*/
typedef enum {
	TASK_MANAGER,
	TASK_NETWORK,
	TASK_REPORTER,
	TASK_COMMAND,
	TASK_MEMS,
	TASK_REMOTE,
	TASK_FINGER,
	TASK_AUDIO,
	TASK_GATE,
	TASK_CANRX,
	TASK_CANTX,
	TASK_MAX,
} TASK;

/* Exported types
 * --------------------------------------------*/
typedef uint32_t tasks_tick_t[TASK_MAX];
typedef uint16_t tasks_stack_t[TASK_MAX];
typedef uint8_t tasks_wakeup_t[TASK_MAX];

/* Exported structs
 * --------------------------------------------*/
typedef uint32_t tasks_tick_t[TASK_MAX];
typedef uint16_t tasks_stack_t[TASK_MAX];
typedef uint8_t tasks_wakeup_t[TASK_MAX];

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
//extern tasks_t TASKS;

/* Public functions prototype
 * --------------------------------------------*/
uint32_t TASK_WaitManager(void);
bool TASK_KernelFailed(void);
void TASK_CheckWakeup(void);
void TASK_CheckStack(void);

uint8_t TASK_IO_GetWakeup(TASK task);
uint16_t TASK_IO_GetStack(TASK task);
void TASK_IO_SetTick(TASK task);
#endif /* INC_APP__TASK_H_ */
