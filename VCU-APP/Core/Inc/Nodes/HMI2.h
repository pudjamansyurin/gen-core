/*
 * HMI2.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_HMI2_H_
#define INC_NODES_HMI2_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Drivers/_canbus.h"

/* Exported constants --------------------------------------------------------*/
#define HMI2_TIMEOUT_MS ((uint32_t)10000)
#define HMI2_POWER_ON_MS ((uint32_t)90000)
#define HMI2_POWER_OFF_MS ((uint32_t)30000)

/* Exported struct
 * ------------------------------------------------------------*/
typedef struct {
  uint8_t run;
  uint32_t tick;
  uint8_t mirroring;
  uint8_t powerRequest;
} hmi2_data_t;

typedef struct {
  hmi2_data_t d;
  struct {
    void (*State)(can_rx_t *);
  } r;
  void (*Init)(void);
  void (*Refresh)(void);
  void (*PowerByCan)(uint8_t);
  void (*PowerOn)(void);
  void (*PowerOff)(void);
} hmi2_t;

/* Exported variables
 * ---------------------------------------------------------*/
extern hmi2_t HMI2;

/* Public functions implementation
 * --------------------------------------------*/
void HMI2_Init(void);
void HMI2_Refresh(void);
void HMI2_PowerByCan(uint8_t state);
void HMI2_RX_State(can_rx_t *Rx);
void HMI2_PowerOn(void);
void HMI2_PowerOff(void);

#endif /* INC_NODES_HMI2_H_ */
