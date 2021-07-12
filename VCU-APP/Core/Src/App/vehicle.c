/*
 * vehicle.c
 *
 *  Created on: Jun 14, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/vehicle.h"

#include "App/task.h"
#include "Libs/finger.h"
#include "Libs/hbar.h"
#include "Libs/remote.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/NODE.h"
#include "Nodes/VCU.h"

/* External variables
 * --------------------------------------------*/
extern osMessageQueueId_t OvdStateQueueHandle;

/* Private constants
 * --------------------------------------------*/
#define VCU_LOST_MODE_S ((uint16_t)(5 * 60))

/* Private variables
 * --------------------------------------------*/
static vehicle_t LAST_STATE = VEHICLE_UNKNOWN;
static vehicle_t CURR_STATE = VEHICLE_BACKUP;

/* Public functions implementation
 * --------------------------------------------*/
void VHC_CheckState(void) {
  uint8_t ovdState, shutdown = 0, start = 0;
  vehicle_t initialState;

  if (OS_QueueGet(OvdStateQueueHandle, &ovdState))
    CURR_STATE = (int8_t)ovdState;

  HB_CheckStarter(&start, &shutdown);

  do {
    initialState = CURR_STATE;

    switch (CURR_STATE) {
      case VEHICLE_LOST:
        if (LAST_STATE != VEHICLE_LOST) {
          LAST_STATE = VEHICLE_LOST;
          osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
        }

        if (GATE_ReadPower5v()) CURR_STATE += 2;
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

          VCU.d.tick.independent = tickMs();
          EVT_Set(EVG_REMOTE_MISSING);
        }

        if (tickOut(VCU.d.tick.independent, VCU_LOST_MODE_S * 1000))
          CURR_STATE--;
        else if (GATE_ReadPower5v())
          CURR_STATE++;
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
          delayMs(500);
          shutdown = 0;
          start = 0;
        }

        if (!GATE_ReadPower5v())
          CURR_STATE--;
        else if (start) {
          if (RMT_IO_Nearby())
            CURR_STATE++;
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
          CURR_STATE--;
        else if (FGR_IO_ID())
          CURR_STATE++;
        break;

      case VEHICLE_READY:
        if (LAST_STATE != VEHICLE_READY) {
          LAST_STATE = VEHICLE_READY;
          start = 0;
          VCU.d.tick.ready = tickMs();
        }

        if (!GATE_ReadPower5v() || shutdown)
          CURR_STATE--;
        else if (!NODE.d.error && (start))
          CURR_STATE++;
        break;

      case VEHICLE_RUN:
        if (LAST_STATE != VEHICLE_RUN) {
          LAST_STATE = VEHICLE_RUN;
          start = 0;
        }

        if (!GATE_ReadPower5v() || (start && !MCU_Running()) || shutdown ||
            NODE.d.error)
          CURR_STATE--;
        break;

      default:
        break;
    }
    delayMs(100);
  } while (initialState != CURR_STATE);
}

vehicle_t VHC_IO_State(void) { return CURR_STATE; }
