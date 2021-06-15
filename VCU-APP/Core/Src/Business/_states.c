/*
 * _states.c
 *
 *  Created on: Jun 14, 2021
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Business/_states.h"
#include "Nodes/VCU.h"
#include "Nodes/NODE.h"
#include "Nodes/MCU.h"
#include "Nodes/BMS.h"
#include "Libs/_remote.h"
#include "Libs/_hbar.h"
#include "Libs/_finger.h"

/* External variables -----------------------------------------*/
extern osMessageQueueId_t OvdStateQueueHandle;

/* Public functions implementation
 * --------------------------------------------*/
void StatesCheck(void) {
	static vehicle_state_t lastState = VEHICLE_UNKNOWN;
	uint8_t ovdState, normalize = 0, start = 0;
	HBAR_STARTER starter = HBAR.ctl.starter;
	vehicle_state_t initialState;

	if (_osQueueGet(OvdStateQueueHandle, &ovdState))
		VCU.d.state = (int8_t) ovdState;

	if (starter != HBAR_STARTER_UNKNOWN) {
		HBAR.ctl.starter = HBAR_STARTER_UNKNOWN;
		normalize = starter == HBAR_STARTER_OFF;
		start = starter == HBAR_STARTER_ON;
	}

	do {
		initialState = VCU.d.state;

		switch (VCU.d.state) {
		case VEHICLE_LOST:
			if (lastState != VEHICLE_LOST) {
				lastState = VEHICLE_LOST;
				osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
			}

			if (GATE_ReadPower5v())
				VCU.d.state += 2;
			break;

		case VEHICLE_BACKUP:
			if (lastState != VEHICLE_BACKUP) {
				lastState = VEHICLE_BACKUP;
				osThreadFlagsSet(ReporterTaskHandle, FLAG_REPORTER_YIELD);
				osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_TASK_STOP);
				osThreadFlagsSet(AudioTaskHandle, FLAG_AUDIO_TASK_STOP);
				osThreadFlagsSet(MemsTaskHandle, FLAG_MEMS_TASK_STOP);
				osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_TASK_STOP);
				osThreadFlagsSet(CanTxTaskHandle, FLAG_CAN_TASK_STOP);
				osThreadFlagsSet(CanRxTaskHandle, FLAG_CAN_TASK_STOP);

				VCU.d.tick.independent = _GetTickMS();
				VCU_SetEvent(EVG_REMOTE_MISSING, 1);
			}

			if ((_GetTickMS() - VCU.d.tick.independent) > (VCU_LOST_MODE_S*1000))
				VCU.d.state--;
			else if (GATE_ReadPower5v())
				VCU.d.state++;
			break;

		case VEHICLE_NORMAL:
			if (lastState != VEHICLE_NORMAL) {
				lastState = VEHICLE_NORMAL;
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
				if (RMT.d.nearby) VCU.d.state++;
				else
					osThreadFlagsSet(RemoteTaskHandle, FLAG_REMOTE_RESET);
			}
			break;

		case VEHICLE_STANDBY:
			if (lastState != VEHICLE_STANDBY) {
				lastState = VEHICLE_STANDBY;
				start = 0;
				FGR.d.id = 0;
			}

			if (!GATE_ReadPower5v() || normalize)
				VCU.d.state--;
			else if (FGR.d.id)
				VCU.d.state++;
			break;

		case VEHICLE_READY:
			if (lastState != VEHICLE_READY) {
				lastState = VEHICLE_READY;
				start = 0;
				VCU.d.tick.ready = _GetTickMS();
			}

			if (!GATE_ReadPower5v() || normalize)
				VCU.d.state--;
			else if (!NODE.d.error && (start))
				VCU.d.state++;
			break;

		case VEHICLE_RUN:
			if (lastState != VEHICLE_RUN) {
				lastState = VEHICLE_RUN;
				start = 0;
			}

			if (!GATE_ReadPower5v() || (start && !MCU_Running()) || normalize || NODE.d.error)
				VCU.d.state--;
			break;

		default:
			break;
		}
		_DelayMS(100);
	} while (initialState != VCU.d.state);
}
