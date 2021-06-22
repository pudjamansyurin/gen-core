/*
 * _state.c
 *
 *  Created on: Jun 14, 2021
 *      Author: pudja
 */

/* Includes
 * --------------------------------------------*/
#include "App/_state.h"
#include "App/_task.h"
#include "Libs/_finger.h"
#include "Libs/_hbar.h"
#include "Libs/_remote.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/NODE.h"
#include "Nodes/VCU.h"

/* External variables
 * --------------------------------------------*/
extern osMessageQueueId_t OvdStateQueueHandle;

/* Private variables
 * --------------------------------------------*/
static vehicle_state_t LAST_STATE = VEHICLE_UNKNOWN;

/* Public functions implementation
 * --------------------------------------------*/
void STATE_Check(void) {
  uint8_t ovdState, normalize = 0, start = 0;
  HBAR_STARTER starter = HBAR.ctl.starter;
  vehicle_state_t initialState;

  if (_osQueueGet(OvdStateQueueHandle, &ovdState))
    VCU.d.state = (int8_t)ovdState;

  if (starter != HBAR_STARTER_UNKNOWN) {
    HBAR.ctl.starter = HBAR_STARTER_UNKNOWN;
    normalize = starter == HBAR_STARTER_OFF;
    start = starter == HBAR_STARTER_ON;
  }

  do {
    initialState = VCU.d.state;

    switch (VCU.d.state) {
    case VEHICLE_LOST:
      if (LAST_STATE != VEHICLE_LOST) {
        LAST_STATE = VEHICLE_LOST;
        osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
      }

      if (GATE_ReadPower5v())
        VCU.d.state += 2;
      break;

    case VEHICLE_BACKUP:
      if (LAST_STATE != VEHICLE_BACKUP) {
        LAST_STATE = VEHICLE_BACKUP;
        osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
        osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_TASK_STOP);
        osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_TASK_STOP);
        osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_TASK_STOP);
        osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_TASK_STOP);
        osThreadFlagsSet(CanTxTaskHandle, FLAG_CAN_TASK_STOP);
        osThreadFlagsSet(CanRxTaskHandle, FLAG_CAN_TASK_STOP);

        VCU.d.tick.independent = _GetTickMS();
        EVT_Set(EVG_REMOTE_MISSING);
      }

      if ((_GetTickMS() - VCU.d.tick.independent) > (VCU_LOST_MODE_S * 1000))
        VCU.d.state--;
      else if (GATE_ReadPower5v())
        VCU.d.state++;
      break;

    case VEHICLE_NORMAL:
      if (LAST_STATE != VEHICLE_NORMAL) {
        LAST_STATE = VEHICLE_NORMAL;
        osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
        osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_TASK_START);
        osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_TASK_START);
        osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_TASK_START);
        osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_TASK_START);
        osThreadFlagsSet(CanTxTaskHandle, FLAG_CAN_TASK_START);
        osThreadFlagsSet(CanRxTaskHandle, FLAG_CAN_TASK_START);

        HBAR_Init();
        BMS_Init();
        MCU_Init();
        _DelayMS(500);
        normalize = 0;
        start = 0;
      }

      if (!GATE_ReadPower5v())
        VCU.d.state--;
      else if (start) {
        if (RMT.d.nearby)
          VCU.d.state++;
        else
          osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_RESET);
      }
      break;

    case VEHICLE_STANDBY:
      if (LAST_STATE != VEHICLE_STANDBY) {
        LAST_STATE = VEHICLE_STANDBY;
        start = 0;
        FGR.d.id = 0;
      }

      if (!GATE_ReadPower5v() || normalize)
        VCU.d.state--;
      else if (FGR.d.id)
        VCU.d.state++;
      break;

    case VEHICLE_READY:
      if (LAST_STATE != VEHICLE_READY) {
        LAST_STATE = VEHICLE_READY;
        start = 0;
        VCU.d.tick.ready = _GetTickMS();
      }

      if (!GATE_ReadPower5v() || normalize)
        VCU.d.state--;
      else if (!NODE.d.error && (start))
        VCU.d.state++;
      break;

    case VEHICLE_RUN:
      if (LAST_STATE != VEHICLE_RUN) {
        LAST_STATE = VEHICLE_RUN;
        start = 0;
      }

      if (!GATE_ReadPower5v() || (start && !MCU_Running()) || normalize ||
          NODE.d.error)
        VCU.d.state--;
      break;

    default:
      break;
    }
    _DelayMS(100);
  } while (initialState != VCU.d.state);
}
