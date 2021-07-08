/*
 * _task.c
 *
 *  Created on: Jun 22, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_task.h"

/* External variables
 * --------------------------------------------*/
extern osEventFlagsId_t GlobalEventHandle;

/* Private macros
 * --------------------------------------------*/
#define WAKEUP(cur, prev) (MAX_U8((cur - prev) / 1000))

/* Exported types
 * --------------------------------------------*/
typedef uint32_t tasks_tick_t[TASK_MAX];

typedef struct {
  tasks_tick_t tick;
  tasks_stack_t stack;
  tasks_wakeup_t wakeup;
} tasks_t;

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
  osThreadId_t THREAD[TASK_MAX] = {
      ManagerTaskHandle, NetworkTaskHandle, ReporterTaskHandle,
      CommandTaskHandle, MemsTaskHandle,    RemoteTaskHandle,
      FingerTaskHandle,  AudioTaskHandle,   GateTaskHandle,
      CanRxTaskHandle,   CanTxTaskHandle,
  };

  for (uint8_t task = 0; task < TASK_MAX; task++)
    TASKS.stack[task] = osThreadGetStackSpace(THREAD[task]);
}

uint8_t TASK_IO_Wakeup(TASK task) { return TASKS.wakeup[task]; }

uint16_t TASK_IO_Stack(TASK task) { return TASKS.stack[task]; }

void TASK_IO_SetTick(TASK task) { TASKS.tick[task] = _GetTickMS(); }
