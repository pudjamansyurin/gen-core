/*
 * _finger.h
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

#ifndef FINGER_H_
#define FINGER_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_fz3387.h"
#include "Libs/_utils.h"

/* Structs --------------------------------------------------------------------*/
typedef struct {
	uint8_t db[FINGER_USER_MAX];
  struct {
    UART_HandleTypeDef *uart;
    DMA_HandleTypeDef *dma;
  } h;
} finger_t;

/* Public functions prototype ------------------------------------------------*/
void FINGER_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
void FINGER_DeInit(void);
void FINGER_GetDatabase(uint8_t *db);
uint8_t FINGER_Enroll(int8_t *id);
uint8_t FINGER_DeleteID(uint8_t id);
uint8_t FINGER_EmptyDatabase(void);
uint8_t FINGER_SetPassword(uint32_t password);
int8_t FINGER_Auth(void);
int8_t FINGER_AuthFast(void);

#endif /* FINGER_H_ */
