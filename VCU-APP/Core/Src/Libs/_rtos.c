/*
 * _rtos.c
 *
 *  Created on: Mar 22, 2021
 *      Author: pujak
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_rtos.h"

/* Private functions prototype
 * --------------------------------------------*/
static uint8_t _osFlag(uint32_t *notif, uint32_t flags, uint32_t options,
                       uint32_t timeout);

/* Public functions implementation
 * --------------------------------------------*/
uint32_t _osFlagOne(uint32_t *notif, uint32_t flag, uint32_t timeout) {
  return _osFlag(notif, flag, osFlagsWaitAny, timeout);
}

uint32_t _osFlagAny(uint32_t *notif, uint32_t timeout) {
  return _osFlag(notif, FLAG_MASK, osFlagsWaitAny, timeout);
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

  if (*notif > FLAG_MASK)
    return 0;

  return (*notif & flags) > 0;
}
