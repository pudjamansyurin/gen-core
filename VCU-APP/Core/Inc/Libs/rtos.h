/*
 * rtos.h
 *
 *  Created on: Mar 22, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__RTOS_H_
#define INC_LIBS__RTOS_H_

/* Includes
 * --------------------------------------------*/
#include "defs.h"

/* Public functions prototype
 * --------------------------------------------*/
uint8_t OS_FlagOne(uint32_t *notif, uint32_t flag, uint32_t timeout);
uint8_t OS_FlagAny(uint32_t *notif, uint32_t timeout);
uint8_t OS_FlagClear(void);
uint8_t OS_QueueGet(osMessageQueueId_t mq_id, void *msg_ptr);
uint8_t OS_QueuePut(osMessageQueueId_t mq_id, const void *msg_ptr);
uint8_t OS_QueuePutRst(osMessageQueueId_t mq_id, const void *msg_ptr);

#endif /* INC_LIBS__RTOS_H_ */
