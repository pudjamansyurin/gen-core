/*
 * _state.c
 *
 *  Created on: Jun 14, 2021
 *      Author: Pudja Mansyurin
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
static vehicle_t LAST_STATE = VEHICLE_UNKNOWN;

/* Public functions implementation
 * --------------------------------------------*/
void STATE_Check(void) {
  uint8_t ovdState, shutdown = 0, start = 0;
  vehicle_t initialState;

  if (_osQueueGet(OvdStateQueueHandle, &ovdState))
    VCU.d.vehicle = (int8_t)ovdState;

  HB_CheckStarter(&start, &shutdown);

  do {
    initialState = VCU.d.vehicle;

    switch (VCU.d.vehicle) {
      case VEHICLE_LOST:
        if (LAST_STATE != VEHICLE_LOST) {
          LAST_STATE = VEHICLE_LOST;
          osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
        }

        if (GATE_ReadPower5v()) VCU.d.vehicle += 2;
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

        if (_TickOut(VCU.d.tick.independent, VCU_LOST_MODE_S * 1000))
          VCU.d.vehicle--;
        else if (GATE_ReadPower5v())
          VCU.d.vehicle++;
        break;

      case VEHICLE_NORMAL:
        if (LAST_STATE != VEHICLE_NORMAL) {
          if (LAST_STATE > VEHICLE_NORMAL) HB_EE_WriteDeffered();
          LAST_STATE = VEHICLE_NORMAL;

          osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
          osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_TASK_START);
          osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_TASK_START);
          osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_TASK_START);
          osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_TASK_START);
          osThreadFlagsSet(CanTxTaskHandle, FLAG_CAN_TASK_START);
          osThreadFlagsSet(CanRxTaskHandle, FLAG_CAN_TASK_START);

          HB_Init();
          BMS_Init();
          MCU_Init();
          _DelayMS(500);
          shutdown = 0;
          start = 0;
        }

        if (!GATE_ReadPower5v())
          VCU.d.vehicle--;
        else if (start) {
          if (RMT_IO_GetNearby())
            VCU.d.vehicle++;
          else
            osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_RESET);
        }
        break;

      case VEHICLE_STANDBY:
        if (LAST_STATE != VEHICLE_STANDBY) {
          LAST_STATE = VEHICLE_STANDBY;
          start = 0;
          FGR_IO_ClearID();
        }

        if (!GATE_ReadPower5v() || shutdown)
          VCU.d.vehicle--;
        else if (FGR_IO_GetID())
          VCU.d.vehicle++;
        break;

      case VEHICLE_READY:
        if (LAST_STATE != VEHICLE_READY) {
          LAST_STATE = VEHICLE_READY;
          start = 0;
          VCU.d.tick.ready = _GetTickMS();
        }

        if (!GATE_ReadPower5v() || shutdown)
          VCU.d.vehicle--;
        else if (!NODE.d.error && (start))
          VCU.d.vehicle++;
        break;

      case VEHICLE_RUN:
        if (LAST_STATE != VEHICLE_RUN) {
          LAST_STATE = VEHICLE_RUN;
          start = 0;
        }

        if (!GATE_ReadPower5v() || (start && !MCU_Running()) || shutdown ||
            NODE.d.error)
          VCU.d.vehicle--;
        break;

      default:
        break;
    }
    _DelayMS(100);
  } while (initialState != VCU.d.vehicle);
}
