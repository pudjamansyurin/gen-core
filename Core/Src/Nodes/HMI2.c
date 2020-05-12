/*
 * HMI2.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/*
 * HMI1.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "HMI2.h"
#include "HMI1.h"
#include "_canbus.h"

/* External variables ----------------------------------------------------------*/
extern canbus_t CB;
extern hmi1_t HMI1;

/* Public variables -----------------------------------------------------------*/
hmi2_t HMI2 = {
		.d = { 0 },
		.can = {
				.r = {
						HMI2_CAN_RX_State,
				},
		},
		HMI2_Init,
		HMI2_Refresh,
		HMI2_PowerOverCan
};

/* Public functions implementation --------------------------------------------*/
void HMI2_Init(void) {
	HMI2.d.started = 0;
	HMI2.d.tick = 0;
}

void HMI2_Refresh(void) {
	if ((osKernelGetTickCount() - HMI2.d.tick) > pdMS_TO_TICKS(10000)) {
		HMI2.d.started = 0;
	}
}

void HMI2_PowerOverCan(uint8_t on) {
	TickType_t timeout = pdMS_TO_TICKS(60000);
	static TickType_t tick = 0;

	if (on) {
		if (!HMI2.d.started) {
			// handle timeout
			if (osKernelGetTickCount() - tick > timeout) {
				tick = osKernelGetTickCount();

				HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, 0);
				osDelay(500);
			}
			// turn ON
			HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, 1);
		} else {
			// completely ON
		}
	} else {
		if (HMI2.d.started) {
			// handle timeout
			if (osKernelGetTickCount() - tick > timeout) {
				tick = osKernelGetTickCount();

				HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, 0);
			}
		} else {
			// completely OFF
			HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, 0);
		}
	}
}

/* ====================================== CAN RX =================================== */
void HMI2_CAN_RX_State(void) {
	// read message
	HMI1.d.status.mirroring = _R1(CB.rx.data.u8[0], 0);

	// save state
	HMI2.d.started = 1;
	HMI2.d.tick = osKernelGetTickCount();
}
