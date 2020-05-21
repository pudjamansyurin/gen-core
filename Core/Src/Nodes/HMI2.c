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
#include "Nodes/HMI2.h"
#include "Nodes/HMI1.h"
#include "Drivers/_canbus.h"

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

void HMI2_PowerOverCan(uint8_t state) {
//	uint8_t lastCommand = 1, lastState = 0;
//	TickType_t tick = 0, timeout;

//	if (HMI2.d.started != state) {
//		// decide timeout
//		timeout = pdMS_TO_TICKS((state ? 30 : 90) * 1000);
//
//		if (lastCommand != state) {
//			lastCommand = state;
//			tick = osKernelGetTickCount();
//		}
//
//		// handle power ON/OFF properly using timeout
//		if (osKernelGetTickCount() - tick > timeout) {
//			HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, !state);
//		}
//	}

//	static TickType_t timeoutOn = pdMS_TO_TICKS(90 * 1000);
//	static TickType_t timeoutOff = pdMS_TO_TICKS(30 * 1000);
//	static TickType_t tickOn = 0, tickOff = 0;
//	static uint8_t initOn = 1, initOff = 1;
//	static uint8_t lastState = 1;
//
//	// PNP transistor is Active Low
//	if (on) {
//		if (initOn) {
//			tickOn = osKernelGetTickCount();
//			initOn = 0;
//			initOff = 1;
//		}
//
//		if (!HMI2.d.started) {
//			// handle timeout
//			if (osKernelGetTickCount() - tickOn > timeoutOn) {
//				tickOn = osKernelGetTickCount();
//				// wait until safe to restart
//				HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, 1);
//				osDelay(500);
//			}
//			// turn ON
//			if (lastState == 0) {
//				if (osKernelGetTickCount() - tickOff > timeoutOff) {
//					HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, 0);
//				}
//			}
//		} else {
//			// completely ON
//			lastState = 1;
//		}
//	} else {
//		if (initOff) {
//			tickOff = osKernelGetTickCount();
//			initOff = 0;
//			initOn = 1;
//		}
//
//		if (HMI2.d.started) {
//			// handle timeout
//			if (osKernelGetTickCount() - tickOff > timeoutOff) {
//				tickOff = osKernelGetTickCount();
//				// wait until safe to shutdown
//				HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, 1);
//			}
//		} else {
//			// completely OFF
//			// turn OFF
//			if (lastState == 1) {
//				if (osKernelGetTickCount() - tickOn > timeoutOn) {
//					lastState = 0;
//					HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, 1);
//				}
//			}
//		}
//	}
}

/* ====================================== CAN RX =================================== */
void HMI2_CAN_RX_State(void) {
	// read message
	HMI1.d.status.mirroring = _R1(CB.rx.data.u8[0], 0);

	// save state
	HMI2.d.started = 1;
	HMI2.d.tick = osKernelGetTickCount();
}
