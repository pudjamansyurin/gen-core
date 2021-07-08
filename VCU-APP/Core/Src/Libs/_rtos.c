/*
 * _rtos.c
 *
 *  Created on: Mar 22, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_rtos.h"

/* Exported constants
 * --------------------------------------------*/
#define EVENT_MASK ((uint32_t)0xFFFFFF)
#define FLAG_MASK ((uint32_t)0x7FFFFFFF)

/* Private functions prototype
 * --------------------------------------------*/
static uint8_t _osFlag(uint32_t *notif, uint32_t flags, uint32_t options,
                       uint32_t timeout);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t _osFlagOne(uint32_t *notif, uint32_t flag, uint32_t timeout) {
  return _osFlag(notif, flag, osFlagsWaitAny, timeout);
}

uint8_t _osFlagAny(uint32_t *notif, uint32_t timeout) {
  return _osFlag(notif, FLAG_MASK, osFlagsWaitAny, timeout);
}

uint8_t _osFlagClear(void) {
	return osThreadFlagsClear(FLAG_MASK) == osOK;
}

uint8_t _osQueueGet(osMessageQueueId_t mq_id, void *msg_ptr) {
  return osMessageQueueGet(mq_id, msg_ptr, NULL, 0U) == osOK;
}

uint8_t _osQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr) {
  return osMessageQueuePut(mq_id, msg_ptr, 0U, 0U) == osOK;
}

uint8_t _osQueuePutRst(osMessageQueueId_t mq_id, const void *msg_ptr) {
  osMessageQueueReset(mq_id);
  return _osQueuePut(mq_id, msg_ptr);
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t _osFlag(uint32_t *notif, uint32_t flags, uint32_t options,
                       uint32_t timeout) {
  *notif = osThreadFlagsWait(flags, options, timeout);

  if (*notif > FLAG_MASK) return 0;

  return (*notif & flags) > 0;
}
