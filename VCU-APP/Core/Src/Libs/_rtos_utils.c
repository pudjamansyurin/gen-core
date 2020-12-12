/*
 * _rtos_utils.c
 *
 *  Created on: Dec 11, 2020
 *      Author: pudja
 */
#include "Libs/_rtos_utils.h"

/* Private functions declarations ---------------------------------------------*/
static uint8_t _RTOS_ValidThreadFlag(uint32_t flag);
//static uint8_t _RTOS_ValidEventFlag(uint32_t flag);

/* Public functions implementation --------------------------------------------*/
// void _RTOS_Debugger(uint32_t ms) {
//   static uint8_t init = 1, thCount;
//   static TickType_t lastDebug;
//   static osThreadId_t threads[20];
//   static uint32_t thLowestFreeStack[20];
//   uint32_t thFreeStack;
//   char thName[15];
//
//   // Initialize
//   if (init) {
//     init = 0;
//     lastDebug = osKernelGetTickCount();
//
//     // Get list of active threads
//     thCount = osThreadGetCount();
//     osThreadEnumerate(threads, thCount);
//
//     // Fill initial free stack
//     for (uint8_t i = 0; i < thCount; i++)
//       if (threads[i] != NULL)
//         thLowestFreeStack[i] = osThreadGetStackSpace(threads[i]);
//   }
//
//   if ((osKernelGetTickCount() - lastDebug) >= ms) {
//     lastDebug = osKernelGetTickCount();
//
//     // Debug each thread's Stack Space
//     LOG_StrLn("============ LOWEST FREE STACK ============");
//     for (uint8_t i = 0; i < thCount; i++) {
//       if (threads[i] != NULL) {
//         // check stack used
//         thFreeStack = osThreadGetStackSpace(threads[i]);
//         if (thFreeStack < thLowestFreeStack[i]) {
//           thLowestFreeStack[i] = thFreeStack;
//         }
//
//         // print
//         sprintf(thName, "%-14s", osThreadGetName(threads[i]));
//         LOG_Buf(thName, strlen(thName));
//         LOG_Str(" : ");
//         LOG_Int(thLowestFreeStack[i]);
//         LOG_StrLn(" Words");
//       }
//     }
//     LOG_StrLn("===========================================");
//   }
// }

uint8_t _RTOS_ThreadFlagsWait(uint32_t *notif, uint32_t flags, uint32_t options, uint32_t timeout) {
  *notif = osThreadFlagsWait(flags, options, timeout);

  return _RTOS_ValidThreadFlag(*notif);
}

uint8_t _RTOS_CalculateStack(osThreadId_t thread_id) {
  return osThreadGetStackSpace(thread_id) * 100 / osThreadGetStackSize(thread_id);
}

/* Private functions implementation --------------------------------------------*/
static uint8_t _RTOS_ValidThreadFlag(uint32_t flag) {
  // check is empty
  if (!flag || (flag & (~EVT_MASK)))
    return 0;
  return 1;
}

//static uint8_t _RTOS_ValidEventFlag(uint32_t flag) {
//  uint8_t ret = 1;
//
//  // check is empty
//  if (!flag || (flag & (~EVENT_MASK)))
//    ret = 0;
//
//  return ret;
//}
