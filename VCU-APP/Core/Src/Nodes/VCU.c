/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/VCU.h"
#include "Drivers/_canbus.h"
#include "Drivers/_simcom.h"
#include "Drivers/_bat.h"
#include "Libs/_reporter.h"
#include "Libs/_eeprom.h"
#include "Libs/_hbar.h"
#include "Libs/_remote.h"
#include "Libs/_finger.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/HMI2.h"
#include "Nodes/MCU.h"
#include "Nodes/NODE.h"

/* Public variables
 * -----------------------------------------------------------*/
vcu_t VCU = {
		.d = {0},
		.t = {
				VCU_TX_Heartbeat,
				VCU_TX_SwitchControl,
				VCU_TX_Datetime,
				VCU_TX_MixedData,
				VCU_TX_TripData
		},
		.Init = VCU_Init,
		.Refresh = VCU_Refresh,
		.CheckState = VCU_CheckState,
		.SetEvent = VCU_SetEvent,
		.ReadEvent = VCU_ReadEvent,
		.Is = VCU_Is,
		.SetOdometer = VCU_SetOdometer,
};

/* Private functions declaration -------------------------------------------*/

/* Public functions implementation
 * --------------------------------------------*/
void VCU_Init(void) {
	VCU.d.error = 0;
	VCU.d.state = VEHICLE_BACKUP;
	VCU.d.events = 0;

	VCU.d.override.state = VEHICLE_UNKNOWN;
}

void VCU_Refresh(void) {
	BAT_ScanValue();

	VCU.d.uptime++;
	VCU.d.error = VCU.ReadEvent(EVG_BIKE_FALLEN);
}

void VCU_CheckState(void) {
	static vehicle_state_t lastState = VEHICLE_UNKNOWN;;
	vehicle_state_t initialState;
	HBAR_STARTER starter;
	uint8_t normalize, start;

	do {
		initialState = VCU.d.state;

		starter = HBAR_RefreshStarter(lastState);
		normalize = starter == HBAR_STARTER_OFF;
		start = starter == HBAR_STARTER_ON;

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
				osThreadFlagsSet(CanTxTaskHandle, FLAG_CAN_TASK_STOP);
				osThreadFlagsSet(CanRxTaskHandle, FLAG_CAN_TASK_STOP);

				VCU.d.tick.independent = _GetTickMS();
				VCU.SetEvent(EVG_REMOTE_MISSING, 1);
			}

			if (_GetTickMS() - VCU.d.tick.independent > (VCU_ACTIVATE_LOST)*1000)
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
				osThreadFlagsSet(CanTxTaskHandle, FLAG_CAN_TASK_START);
				osThreadFlagsSet(CanRxTaskHandle, FLAG_CAN_TASK_START);
				osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_TASK_STOP);

				HBAR_Init();
				VCU.d.override.state = VEHICLE_UNKNOWN;
			}

			if (!GATE_ReadPower5v())
				VCU.d.state--;
			else if ((start && RMT.d.nearby) || VCU.Is(VCU.d.override.state > VEHICLE_NORMAL))
				VCU.d.state++;
			break;

		case VEHICLE_STANDBY:
			if (lastState != VEHICLE_STANDBY) {
				if (lastState > VEHICLE_STANDBY && VCU.Is(VCU.d.override.state > VEHICLE_STANDBY))
					VCU.d.override.state = VEHICLE_STANDBY;
				lastState = VEHICLE_STANDBY;

				osThreadFlagsSet(FingerTaskHandle, FLAG_FINGER_TASK_START);
			}

			if (!GATE_ReadPower5v() || normalize || VCU.Is(VCU.d.override.state < VEHICLE_STANDBY))
				VCU.d.state--;
			else if (FGR.d.id || VCU.Is(VCU.d.override.state > VEHICLE_STANDBY))
				VCU.d.state++;
			break;

		case VEHICLE_READY:
			if (lastState != VEHICLE_READY) {
				if (lastState > VEHICLE_READY && VCU.Is(VCU.d.override.state > VEHICLE_READY))
					VCU.d.override.state = VEHICLE_READY;
				lastState = VEHICLE_READY;
			}

			if (!GATE_ReadPower5v() || normalize || VCU.Is(VCU.d.override.state < VEHICLE_READY))
				VCU.d.state--;
			else if (!NODE.d.error && (start || VCU.Is(VCU.d.override.state > VEHICLE_READY)))
				VCU.d.state++;
			break;

		case VEHICLE_RUN:
			if (lastState != VEHICLE_RUN) {
				lastState = VEHICLE_RUN;
			}

			if (!GATE_ReadPower5v() || normalize || NODE.d.error ||	VCU.Is(VCU.d.override.state == VEHICLE_READY))
				VCU.d.state--;
			else if ((start && MCU.RpmToSpeed(MCU.d.rpm) == 0) || VCU.Is(VCU.d.override.state < VEHICLE_READY))
				VCU.d.state -= 2;
			break;

		default:
			break;
		}
	} while (initialState != VCU.d.state);
}

void VCU_SetEvent(uint8_t bit, uint8_t value) {
	if (value & 1) BV(VCU.d.events, bit);
	else BC(VCU.d.events, bit);
}

uint8_t VCU_ReadEvent(uint8_t bit) {
	return (VCU.d.events & BIT(bit)) == BIT(bit);
}

uint8_t VCU_Is(uint8_t state) {
	return (VCU.d.override.state != VEHICLE_UNKNOWN) && state;
}

void VCU_SetOdometer(uint8_t meter) {
	static uint32_t last_km = 0;
	uint32_t odometer, odometer_km;

	odometer = HBAR_AccumulateTrip(meter);
	odometer_km = odometer / 1000;

	// init hook
	if (last_km == 0)
		last_km = odometer_km;

	// check every 1km
	if (odometer_km > last_km) {
		last_km = odometer_km;

		// accumulate (save permanently)
		EEPROM_Odometer(EE_CMD_W, odometer);
	}
}


/* ====================================== CAN TX
 * =================================== */
uint8_t VCU_TX_Heartbeat(void) {
	can_tx_t Tx = {0};

	Tx.data.u16[0] = VCU_VERSION;

	return CANBUS_Write(&Tx, CAND_VCU, 2, 0);
}

uint8_t VCU_TX_SwitchControl(void) {
	can_tx_t Tx = {0};

	Tx.data.u8[0] = HBAR.state[HBAR_K_ABS];
	Tx.data.u8[0] |= HMI2.d.mirroring << 1;
	Tx.data.u8[0] |= HBAR.state[HBAR_K_LAMP] << 2;
	Tx.data.u8[0] |= NODE.d.error << 3;
	Tx.data.u8[0] |= NODE.d.overheat << 4;
	Tx.data.u8[0] |= !FGR.d.id << 5;
	Tx.data.u8[0] |= !RMT.d.nearby << 6;
	Tx.data.u8[0] |= RTC_Daylight() << 7;

	// sein value
	hbar_sein_t sein = HBAR_SeinController();
	Tx.data.u8[1] = sein.left;
	Tx.data.u8[1] |= sein.right << 1;
	// TODO: validate MCU reverse state
	Tx.data.u8[1] |= HBAR.state[HBAR_K_REVERSE] << 2;

	// mode
	// TODO: validate MCU drive mode
	Tx.data.u8[2] = HBAR.d.mode[HBAR_M_DRIVE];
	Tx.data.u8[2] |= HBAR.d.mode[HBAR_M_TRIP] << 2;
	Tx.data.u8[2] |= HBAR.d.mode[HBAR_M_REPORT] << 4;
	Tx.data.u8[2] |= HBAR.d.m << 5;
	Tx.data.u8[2] |= HBAR.d.ctl.blink << 7;

	// others
	Tx.data.u8[3] = MCU.RpmToSpeed(MCU.d.rpm);
	Tx.data.u8[4] = (uint8_t)MCU.d.dcbus.current;

	// send message
	return CANBUS_Write(&Tx, CAND_VCU_SWITCH_CTL, 5, 0);
}

uint8_t VCU_TX_Datetime(datetime_t dt) {
	can_tx_t Tx = {0};
	uint8_t hmi2shutdown = VCU.d.state < VEHICLE_STANDBY;

	Tx.data.u8[0] = dt.Seconds;
	Tx.data.u8[1] = dt.Minutes;
	Tx.data.u8[2] = dt.Hours;
	Tx.data.u8[3] = dt.Date;
	Tx.data.u8[4] = dt.Month;
	Tx.data.u8[5] = dt.Year;
	Tx.data.u8[6] = dt.WeekDay;
	Tx.data.u8[7] = hmi2shutdown;

	return CANBUS_Write(&Tx, CAND_VCU_DATETIME, 8, 0);
}

uint8_t VCU_TX_MixedData(void) {
	can_tx_t Tx = {0};

	Tx.data.u8[0] = SIM.d.signal;
	Tx.data.u8[1] = BMS.d.soc;
	Tx.data.u8[2] = HBAR.d.report[HBAR_M_REPORT_RANGE];
	Tx.data.u8[3] = HBAR.d.report[HBAR_M_REPORT_AVERAGE];

	return CANBUS_Write(&Tx, CAND_VCU_MIXED_DATA, 4, 0);
}

uint8_t VCU_TX_TripData(void) {
	can_tx_t Tx = {0};

	Tx.data.u16[0] = HBAR.d.trip[HBAR_M_TRIP_A] / 1000;
	Tx.data.u16[1] = HBAR.d.trip[HBAR_M_TRIP_B] / 1000;
	Tx.data.u32[1] = HBAR.d.trip[HBAR_M_TRIP_ODO] / 1000;

	return CANBUS_Write(&Tx, CAND_VCU_TRIP_DATA, 8, 0);
}


/* Private functions implementation -------------------------------------------*/
