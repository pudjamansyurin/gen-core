/*
 * MCU.h
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_MCU_H_
#define INC_NODES_MCU_H_

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"
#include "Libs/_utils.h"

/* Exported constants --------------------------------------------------------*/
#define MCU_TIMEOUT    							 (uint32_t) 1000					// ms

/* Exported struct ------------------------------------------------------------*/
typedef struct {
	uint8_t speed;
	uint32_t tick;
	//  uint8_t started;
	//  uint16_t version;
} mcu_data_t;

typedef struct {
	struct {
		void (*State)(can_rx_t*);
	} r;
} mcu_can_t;

typedef struct {
	mcu_data_t d;
	mcu_can_t can;
	void (*Init)(void);
	void (*Refresh)(void);
	uint16_t (*SpeedToVolume)(void);
} mcu_t;

/* Exported variables ---------------------------------------------------------*/
extern mcu_t MCU;

/* Public functions implementation --------------------------------------------*/
void MCU_Init(void);
void MCU_Refresh(void);
uint16_t MCU_SpeedToVolume(void);
void MCU_CAN_RX_State(can_rx_t *Rx);

#endif /* INC_NODES_MCU_H_ */
