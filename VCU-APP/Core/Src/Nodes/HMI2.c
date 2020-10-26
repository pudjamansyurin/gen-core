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

/* External variables ----------------------------------------------------------*/
extern osThreadId_t Hmi2PowerTaskHandle;
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
    if ((_GetTickMS() - HMI2.d.tick) > 10000) {
        HMI2.d.started = 0;
    }
}

void HMI2_PowerOverCan(uint8_t state) {
    // past to thread handler
    HMI2.d.power = state;
    osThreadFlagsSet(Hmi2PowerTaskHandle, EVT_HMI2POWER_CHANGED);
}

/* ====================================== CAN RX =================================== */
void HMI2_CAN_RX_State(can_rx_t *Rx) {
    // read message
    HMI1.d.status.mirroring = _R1(Rx->data.u8[0], 0);
    // save state
    HMI2.d.started = 1;
    HMI2.d.tick = _GetTickMS();
}
