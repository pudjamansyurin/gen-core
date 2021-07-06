/*
 * _task.c
 *
 *  Created on: Jun 22, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_task.h"

/* Private macros
 * --------------------------------------------*/
#define WAKEUP(cur, prev) (MAX_U8((cur - prev) / 1000))

/* External variables
 * --------------------------------------------*/
extern osEventFlagsId_t GlobalEventHandle;

/* Private variables
 * --------------------------------------------*/
static tasks_t TASKS = {0};

/* Public functions implementation
 * --------------------------------------------*/
uint32_t TASK_WaitManager(void) {
  return osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear,
                          osWaitForever);
}

bool TASK_KernelFailed(void) {
  uint8_t expectedThread = sizeof(tasks_wakeup_t);
  uint8_t activeThread = (uint8_t)(osThreadGetCount() - 2);
  bool error;

  error = (activeThread < expectedThread);
  if (error)
    printf("RTOS:Failed, active thread %d < %d\n", activeThread,
           expectedThread);

  return error;
}

void TASK_CheckWakeup(void) {
  uint32_t now = osKernelGetTickCount();

  for (uint8_t task = 0; task < TASK_MAX; task++)
    TASKS.wakeup[task] = WAKEUP(now, TASKS.tick[task]);
}

void TASK_CheckStack(void) {
  TASKS.stack[TASK_MANAGER] = osThreadGetStackSpace(ManagerTaskHandle);
  TASKS.stack[TASK_NETWORK] = osThreadGetStackSpace(NetworkTaskHandle);
  TASKS.stack[TASK_REPORTER] = osThreadGetStackSpace(ReporterTaskHandle);
  TASKS.stack[TASK_COMMAND] = osThreadGetStackSpace(CommandTaskHandle);
  TASKS.stack[TASK_MEMS] = osThreadGetStackSpace(MemsTaskHandle);
  TASKS.stack[TASK_REMOTE] = osThreadGetStackSpace(RemoteTaskHandle);
  TASKS.stack[TASK_FINGER] = osThreadGetStackSpace(FingerTaskHandle);
  TASKS.stack[TASK_AUDIO] = osThreadGetStackSpace(AudioTaskHandle);
  TASKS.stack[TASK_GATE] = osThreadGetStackSpace(GateTaskHandle);
  TASKS.stack[TASK_CANRX] = osThreadGetStackSpace(CanRxTaskHandle);
  TASKS.stack[TASK_CANTX] = osThreadGetStackSpace(CanTxTaskHandle);
}

uint8_t TASK_GetWakeup(TASK task) {
	return TASKS.wakeup[task];
}

uint16_t TASK_GetStack(TASK task) {
	return TASKS.stack[task];
}

void TASK_SetTick(TASK task) {
	TASKS.tick[task] = _GetTickMS();
}

