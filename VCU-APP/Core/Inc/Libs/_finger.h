/*
 * _finger.h
 *
 *  Created on: Aug 28, 2019
 *      Author: Puja
 */

#ifndef FINGER_H_
#define FINGER_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define FINGER_TIMEOUT (uint16_t)5000 // in ms
#define FINGER_SCAN_TIMEOUT (uint16_t)10000 // in ms

#define FINGER_CONFIDENCE_MIN (uint8_t)75  // in %
#define FINGER_USER_MAX (uint8_t)5

/* Structs
 * --------------------------------------------------------------------*/
typedef struct {
	uint8_t id;
	uint8_t verified;
	uint8_t registering;
	uint8_t db[FINGER_USER_MAX];
} finger_data_t;

typedef struct {
	finger_data_t d;
	UART_HandleTypeDef *puart;
	DMA_HandleTypeDef *pdma;
} finger_t;

/* Exported variables
 * ----------------------------------------------------------*/
extern finger_t FGR;

/* Public functions prototype ------------------------------------------------*/
uint8_t FGR_Init(void);
void FGR_DeInit(void);
uint8_t FGR_Probe(void);
uint8_t FGR_Verify(void);
void FGR_Flush(void);
uint8_t FGR_Fetch(void);
uint8_t FGR_Enroll(uint8_t *id, uint8_t *ok);
uint8_t FGR_DeleteID(uint8_t id);
uint8_t FGR_ResetDB(void);
uint8_t FGR_SetPassword(uint32_t password);
uint8_t FGR_Auth(void);
uint8_t FGR_AuthFast(void);
void FGR_Registering(uint8_t state);

#endif /* FINGER_H_ */
