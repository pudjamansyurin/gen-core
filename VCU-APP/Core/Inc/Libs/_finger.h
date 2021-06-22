/*
 * _finger.h
 *
 *  Created on: Aug 28, 2019
 *      Author: Pudja Mansyurin
 */

#ifndef FINGER_H_
#define FINGER_H_

/* Includes
 * --------------------------------------------*/
#include "_defines.h"

/* Exported constants
 * --------------------------------------------*/
#define FINGER_TIMEOUT_MS ((uint16_t)5000)
#define FINGER_SCAN_MS ((uint16_t)10000)

#define FINGER_CONFIDENCE_MIN_PERCENT ((uint8_t)75)
#define FINGER_USER_MAX ((uint8_t)5)

/* Exported enums
 * --------------------------------------------*/
typedef enum {
  FGR_REG_HIDE = 0,
  FGR_REG_SHOW,
} FGR_REG;

/* Exported structs
 * --------------------------------------------*/
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
 * --------------------------------------------*/
extern finger_t FGR;

/* Public functions prototype
 * --------------------------------------------*/
uint8_t FGR_Init(void);
void FGR_DeInit(void);
uint8_t FGR_Probe(void);
void FGR_Verify(void);
void FGR_Flush(void);
uint8_t FGR_Fetch(void);
uint8_t FGR_Enroll(uint8_t *id, uint8_t *ok);
uint8_t FGR_DeleteID(uint8_t id);
uint8_t FGR_ResetDB(void);
uint8_t FGR_SetPassword(uint32_t password);
void FGR_Authenticate(void);

#endif /* FINGER_H_ */
