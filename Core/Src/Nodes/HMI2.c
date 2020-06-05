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
extern osThreadId_t Hmi2PowerTaskHandle;
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
    // past to thread handler
    HMI2.d.power = state;
    osThreadFlagsSet(Hmi2PowerTaskHandle, EVT_HMI2POWER_CHANGED);
}

/* ====================================== THREAD =================================== */
void StartHmi2PowerTask(void *argument) {
    TickType_t tick;
    uint32_t notif;
    uint8_t activeHigh = 0;

    /* Infinite loop */
    for (;;) {
        // wait forever until triggered
        notif = osThreadFlagsWait(EVT_HMI2POWER_CHANGED, osFlagsWaitAny, osWaitForever);
        if (_RTOS_ValidThreadFlag(notif)) {
            // Handle power control
            if (HMI2.d.power) {
                while (!HMI2.d.started) {
                    // turn ON
                    HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, !activeHigh);
                    osDelay(100);
                    HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, activeHigh);

                    // wait until turned ON
                    tick = osKernelGetTickCount();
                    while (osKernelGetTickCount() - tick < pdMS_TO_TICKS(90 * 1000)) {
                        // already ON
                        if (HMI2.d.started) {
                            break;
                        }
                    }
                }
            } else {
                while (HMI2.d.started) {
                    // wait until turned OFF by CAN
                    tick = osKernelGetTickCount();
                    while (osKernelGetTickCount() - tick < pdMS_TO_TICKS(30 * 1000)) {
                        // already OFF
                        if (!HMI2.d.started) {
                            break;
                        }
                    }

                    // force turn OFF
                    HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, !activeHigh);
                }
            }
        }
    }
}

/* ====================================== CAN RX =================================== */
void HMI2_CAN_RX_State(void) {
    CAN_DATA *data = &(CB.rx.data);

    // read message
    HMI1.d.status.mirroring = _R1(data->u8[0], 0);

    // save state
    HMI2.d.started = 1;
    HMI2.d.tick = osKernelGetTickCount();
}
