/*
 * _task.c
 *
 *  Created on: Jun 22, 2021
 *      Author: pudja
 */

/* Includes
 * --------------------------------------------*/
#include "App/_task.h"

/* Private macros
 * --------------------------------------------*/
#define WAKEUP(cur, prev) (MAX_U8((cur - prev) / 1000))

/* Public variables
 * --------------------------------------------*/
tasks_t TASKS = {0};

/* External variables
 * --------------------------------------------*/
extern osEventFlagsId_t GlobalEventHandle;

/* Public functions implementation
 * --------------------------------------------*/
uint32_t TASK_WaitManager(void) {
  return osEventFlagsWait(GlobalEventHandle, EVENT_READY, osFlagsNoClear,
                          osWaitForever);
}

uint8_t TASK_KernelFailed(void) {
  uint8_t expectedThread = sizeof(tasks_wakeup_t);
  uint8_t activeThread = (uint8_t)(osThreadGetCount() - 2);
  uint8_t error = 0;

  if (activeThread < expectedThread) {
    printf("RTOS:Failed, active thread %d < %d\n", activeThread,
           expectedThread);
    error = 1;
  }

  return error;
}

void TASK_CheckWakeup(void) {
  tasks_tick_t *tc = &(TASKS.tick);
  tasks_wakeup_t *tw = &(TASKS.wakeup);
  TickType_t now = osKernelGetTickCount();

  tw->manager = WAKEUP(now, tc->manager);
  tw->network = WAKEUP(now, tc->network);
  tw->reporter = WAKEUP(now, tc->reporter);
  tw->command = WAKEUP(now, tc->command);
  tw->mems = WAKEUP(now, tc->mems);
  tw->remote = WAKEUP(now, tc->remote);
  tw->finger = WAKEUP(now, tc->finger);
  tw->audio = WAKEUP(now, tc->audio);
  tw->gate = WAKEUP(now, tc->gate);
  tw->canRx = WAKEUP(now, tc->canRx);
  tw->canTx = WAKEUP(now, tc->canTx);
}

void TASK_CheckStack(void) {
  tasks_stack_t *ts = &(TASKS.stack);

  ts->manager = osThreadGetStackSpace(ManagerTaskHandle);
  ts->network = osThreadGetStackSpace(NetworkTaskHandle);
  ts->reporter = osThreadGetStackSpace(ReporterTaskHandle);
  ts->command = osThreadGetStackSpace(CommandTaskHandle);
  ts->mems = osThreadGetStackSpace(MemsTaskHandle);
  ts->remote = osThreadGetStackSpace(RemoteTaskHandle);
  ts->finger = osThreadGetStackSpace(FingerTaskHandle);
  ts->audio = osThreadGetStackSpace(AudioTaskHandle);
  ts->gate = osThreadGetStackSpace(GateTaskHandle);
  ts->canRx = osThreadGetStackSpace(CanRxTaskHandle);
  ts->canTx = osThreadGetStackSpace(CanTxTaskHandle);
}
