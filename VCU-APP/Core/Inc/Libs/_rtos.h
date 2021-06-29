/*
 * _rtos.h
 *
 *  Created on: Mar 22, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_LIBS__RTOS_H_
#define INC_LIBS__RTOS_H_

/* Includes
 * --------------------------------------------*/
#include "_defs.h"

/* Exported constants
 * --------------------------------------------*/
#define EVENT_MASK ((uint32_t)0xFFFFFF)
#define FLAG_MASK ((uint32_t)0x7FFFFFFF)

/* Public functions prototype
 * --------------------------------------------*/
uint32_t _osFlagOne(uint32_t *notif, uint32_t flag, uint32_t timeout);
uint32_t _osFlagAny(uint32_t *notif, uint32_t timeout);
uint8_t _osQueueGet(osMessageQueueId_t mq_id, void *msg_ptr);
uint8_t _osQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr);
uint8_t _osQueuePutRst(osMessageQueueId_t mq_id, const void *msg_ptr);

#endif /* INC_LIBS__RTOS_H_ */
