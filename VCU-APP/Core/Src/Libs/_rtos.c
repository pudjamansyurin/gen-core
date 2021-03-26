/*
 * _rtos.c
 *
 *  Created on: Mar 22, 2021
 *      Author: pujak
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_rtos.h"

/* public variable
 * ------------------------------------------------------------*/
tasks_t TASKS = {0};

/* external variables
 * -----------------------------------------------------------*/
extern osEventFlagsId_t GlobalEventHandle;

/* Public functions implementation
 * --------------------------------------------*/
uint8_t _osFlag(uint32_t *notif, uint32_t flags, uint32_t options, uint32_t timeout) {
	*notif = osThreadFlagsWait(flags, options, timeout);

	if (*notif > FLAG_MASK)
		return 0;

	return (*notif & flags) > 0;
}

uint32_t _osEventManager(void) {
	return osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear, osWaitForever);
}

uint32_t _osFlagOne(uint32_t *notif, uint32_t flag, uint32_t timeout) {
	return _osFlag(notif, flag, osFlagsWaitAny, timeout);
}

uint32_t _osFlagAny(uint32_t *notif, uint32_t timeout) {
	return _osFlag(notif, FLAG_MASK, osFlagsWaitAny, timeout);
}

uint8_t _osQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr) {
	return osMessageQueuePut(mq_id, msg_ptr, 0U, 0U) == osOK;
}

uint8_t _osQueuePutRst(osMessageQueueId_t mq_id, const void *msg_ptr) {
	osMessageQueueReset(mq_id);
	return osMessageQueuePut(mq_id, msg_ptr, 0U, 0U) == osOK;
}

uint8_t _osCheckRTOS(void) {
	uint8_t expectedThread = sizeof(tasks_wakeup_t);
	uint8_t activeThread = (uint8_t)(osThreadGetCount() - 2);
	if (activeThread < expectedThread) {
		printf("RTOS:Failed, active thread %d < %d\n", activeThread, expectedThread);
		return 0;
	}
	return 1;
}

void _osCheckTasks(void) {
	tasks_t *t = &TASKS;
	TickType_t now = osKernelGetTickCount();

	t->wakeup.manager = MAX_U8((now - t->tick.manager)/1000);
	t->wakeup.network = MAX_U8((now - t->tick.network)/1000);
	t->wakeup.reporter = MAX_U8((now - t->tick.reporter)/1000);
	t->wakeup.command = MAX_U8((now - t->tick.command)/1000);
	t->wakeup.gps = MAX_U8((now - t->tick.gps)/1000);
	t->wakeup.gyro = MAX_U8((now - t->tick.gyro)/1000);
	t->wakeup.remote = MAX_U8((now - t->tick.remote)/1000);
	t->wakeup.finger = MAX_U8((now - t->tick.finger)/1000);
	t->wakeup.audio = MAX_U8((now - t->tick.audio)/1000);
	t->wakeup.gate = MAX_U8((now - t->tick.gate)/1000);
	t->wakeup.canRx = MAX_U8((now - t->tick.canRx)/1000);
	t->wakeup.canTx = MAX_U8((now - t->tick.canTx)/1000);
	t->wakeup.hmi2Power = MAX_U8((now - t->tick.hmi2Power)/1000);

	t->stack.manager = osThreadGetStackSpace(ManagerTaskHandle);
	t->stack.network = osThreadGetStackSpace(NetworkTaskHandle);
	t->stack.reporter = osThreadGetStackSpace(ReporterTaskHandle);
	t->stack.command = osThreadGetStackSpace(CommandTaskHandle);
	t->stack.gps = osThreadGetStackSpace(GpsTaskHandle);
	t->stack.gyro = osThreadGetStackSpace(GyroTaskHandle);
	t->stack.remote = osThreadGetStackSpace(RemoteTaskHandle);
	t->stack.finger = osThreadGetStackSpace(FingerTaskHandle);
	t->stack.audio = osThreadGetStackSpace(AudioTaskHandle);
	t->stack.gate = osThreadGetStackSpace(GateTaskHandle);
	t->stack.canRx = osThreadGetStackSpace(CanRxTaskHandle);
	t->stack.canTx = osThreadGetStackSpace(CanTxTaskHandle);
	t->stack.hmi2Power = osThreadGetStackSpace(Hmi2PowerTaskHandle);
}
