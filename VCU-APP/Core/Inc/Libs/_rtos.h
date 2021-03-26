/*
 * _rtos.h
 *
 *  Created on: Mar 22, 2021
 *      Author: pujak
 */

#ifndef INC_LIBS__RTOS_H_
#define INC_LIBS__RTOS_H_

/* Includes ------------------------------------------------------------------*/
#include "_defines.h"

#define MAX_U8(_X_)				((_X_) > UINT8_MAX ? UINT8_MAX : (_X_))

/* Exported defines
 * ------------------------------------------------------------*/
#define EVENT_MASK (uint32_t)0xFFFFFF
#define EVENT_READY BIT(0)

#define FLAG_MASK (uint32_t)0x7FFFFFFF

#define FLAG_NET_REPORT_DISCARD BIT(0)
#define FLAG_NET_SEND_USSD BIT(1)
#define FLAG_NET_READ_SMS BIT(2)

#define FLAG_REPORTER_YIELD BIT(0)

#define FLAG_COMMAND_ERROR BIT(0)
#define FLAG_COMMAND_OK BIT(1)

#define FLAG_GPS_TASK_START BIT(0)
#define FLAG_GPS_TASK_STOP BIT(1)
#define FLAG_GPS_RECEIVED BIT(2)

#define FLAG_GYRO_TASK_START BIT(0)
#define FLAG_GYRO_TASK_STOP BIT(1)
#define FLAG_GYRO_MOVED_RESET BIT(2)

#define FLAG_REMOTE_TASK_START BIT(0)
#define FLAG_REMOTE_TASK_STOP BIT(1)
#define FLAG_REMOTE_REINIT BIT(2)
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
#define FLAG_AUDIO_BEEP_START BIT(3)
#define FLAG_AUDIO_BEEP_STOP BIT(4)
#define FLAG_AUDIO_MUTE_ON BIT(5)
#define FLAG_AUDIO_MUTE_OFF BIT(6)

#define FLAG_CAN_TASK_START BIT(0)
#define FLAG_CAN_TASK_STOP BIT(1)

#define FLAG_GATE_HBAR BIT(0)
#define FLAG_GATE_STARTER_IRQ BIT(1)

#define FLAG_HMI2POWER_CHANGED BIT(0)

/* Exported struct
 * --------------------------------------------------------------*/
typedef struct __attribute__((packed)) {
	TickType_t manager;
	TickType_t network;
	TickType_t reporter;
	TickType_t command;
	TickType_t gps;
	TickType_t gyro;
	TickType_t remote;
	TickType_t finger;
	TickType_t audio;
	TickType_t gate;
	TickType_t canRx;
	TickType_t canTx;
	TickType_t hmi2Power;
} tasks_tick_t;

typedef struct __attribute__((packed)) {
	uint16_t manager;
	uint16_t network;
	uint16_t reporter;
	uint16_t command;
	uint16_t gps;
	uint16_t gyro;
	uint16_t remote;
	uint16_t finger;
	uint16_t audio;
	uint16_t gate;
	uint16_t canRx;
	uint16_t canTx;
	uint16_t hmi2Power;
} tasks_stack_t;

typedef struct __attribute__((packed)) {
	uint8_t manager;
	uint8_t network;
	uint8_t reporter;
	uint8_t command;
	uint8_t gps;
	uint8_t gyro;
	uint8_t remote;
	uint8_t finger;
	uint8_t audio;
	uint8_t gate;
	uint8_t canRx;
	uint8_t canTx;
	uint8_t hmi2Power;
} tasks_wakeup_t;

typedef struct __attribute__((packed)) {
	tasks_tick_t tick;
	tasks_stack_t stack;
	tasks_wakeup_t wakeup;
} tasks_t;

/* Exported variable
 * ------------------------------------------------------------*/
extern osThreadId_t ManagerTaskHandle;
extern osThreadId_t NetworkTaskHandle;
extern osThreadId_t ReporterTaskHandle;
extern osThreadId_t CommandTaskHandle;
extern osThreadId_t GpsTaskHandle;
extern osThreadId_t GyroTaskHandle;
extern osThreadId_t RemoteTaskHandle;
extern osThreadId_t FingerTaskHandle;
extern osThreadId_t AudioTaskHandle;
extern osThreadId_t GateTaskHandle;
extern osThreadId_t CanRxTaskHandle;
extern osThreadId_t CanTxTaskHandle;
extern osThreadId_t Hmi2PowerTaskHandle;

extern tasks_t TASKS;

/* Public functions prototype ------------------------------------------------*/
uint8_t _osFlag(uint32_t *notif, uint32_t flags, uint32_t options, uint32_t timeout);
uint32_t _osEventManager(void);
uint32_t _osFlagOne(uint32_t *notif, uint32_t flag, uint32_t timeout);
uint32_t _osFlagAny(uint32_t *notif, uint32_t timeout);
uint8_t _osQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr);
uint8_t _osQueuePutRst(osMessageQueueId_t mq_id, const void *msg_ptr);
uint8_t _osCheckRTOS(void);
void _osCheckTasks(void);

#endif /* INC_LIBS__RTOS_H_ */
