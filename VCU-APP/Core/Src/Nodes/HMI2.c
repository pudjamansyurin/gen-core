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

/* External variables
 * ---------------------------------------------------------*/
extern osThreadId_t Hmi2PowerTaskHandle;

/* Public variables
 * -----------------------------------------------------------*/
hmi2_t HMI2 = {.d = {0},
               .can =
                   {
                       .r =
                           {
                               HMI2_CAN_RX_State,
                           },
                   },
               HMI2_Init,
               HMI2_Refresh,
               HMI2_PowerByCan,
               HMI2_PowerOn,
               HMI2_PowerOff};

/* Public functions implementation
 * --------------------------------------------*/
void HMI2_Init(void) {
  HMI2.d.run = 0;
  HMI2.d.tick = 0;
}

void HMI2_Refresh(void) {
  if ((_GetTickMS() - HMI2.d.tick) > HMI2_TIMEOUT)
    HMI2.d.run = 0;
}

void HMI2_PowerByCan(uint8_t state) {
  if (HMI2.d.powerRequest != state) {
    HMI2.d.powerRequest = state;
    osThreadFlagsSet(Hmi2PowerTaskHandle, EVT_HMI2POWER_CHANGED);
  }
}

void HMI2_PowerOn(void) {
  TickType_t tick;

  GATE_Hmi2Reset();

  // wait until turned ON by CAN
  tick = _GetTickMS();
  while (!HMI2.d.run && _GetTickMS() - tick < HMI2_POWER_ON_TIMEOUT)
    ;
}

void HMI2_PowerOff(void) {
  TickType_t tick;

  // wait until turned OFF by CAN
  tick = _GetTickMS();
  while (HMI2.d.run && _GetTickMS() - tick < HMI2_POWER_OFF_TIMEOUT)
    ;

  GATE_Hmi2Stop();
}

/* ====================================== CAN RX
 * =================================== */
void HMI2_CAN_RX_State(can_rx_t *Rx) {
  HMI1.d.state.mirroring = (Rx->data.u8[0] >> 0) & 0x01;

  // save state
  HMI2.d.run = 1;
  HMI2.d.tick = _GetTickMS();
}
