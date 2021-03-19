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
#include "Libs/_eeprom.h"
#include "Libs/_handlebar.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/HMI2.h"
#include "Nodes/MCU.h"

/* extern variables
 * -----------------------------------------------------------*/
extern osThreadId_t ManagerTaskHandle;
extern osThreadId_t IotTaskHandle;
extern osThreadId_t ReporterTaskHandle;
extern osThreadId_t CommandTaskHandle;
extern osThreadId_t GpsTaskHandle;
extern osThreadId_t GyroTaskHandle;
extern osThreadId_t RemoteTaskHandle;
extern osThreadId_t FingerTaskHandle;
extern osThreadId_t AudioTaskHandle;
extern osThreadId_t GateTaskHandle;
extern osThreadId_t CanRxTaskHandle;
extern osThreadId_t CanTxTaskHandle;
extern osThreadId_t Hmi2PowerTaskHandle;

/* Public variables
 * -----------------------------------------------------------*/
vcu_t VCU = {
    .d = {0},
    .can = {.t = {VCU_CAN_TX_Heartbeat, VCU_CAN_TX_SwitchModeControl,
                  VCU_CAN_TX_Datetime, VCU_CAN_TX_MixedData,
                  VCU_CAN_TX_TripData}},
    VCU_Init,
		VCU_Refresh,
    VCU_NodesInit,
    VCU_NodesRefresh,
    VCU_CheckState,
    VCU_CheckRTOS,
    VCU_CheckStack,
    VCU_SetEvent,
    VCU_ReadEvent,
    VCU_SetDriver,
    VCU_SetOdometer,
};

/* Public functions implementation
 * --------------------------------------------*/
void VCU_Init(void) {
  // reset VCU data
  VCU.d.error = 0;
  VCU.d.override = 0;
  VCU.d.state = VEHICLE_BACKUP;

  VCU.d.interval = RPT_INTERVAL_BACKUP;
  VCU.d.driver_id = DRIVER_ID_NONE;
  VCU.d.bat = 0;

  VCU.d.motion.yaw = 0;
  VCU.d.motion.roll = 0;
  VCU.d.motion.pitch = 0;

  VCU.d.events = 0;
}

void VCU_Refresh(void) {
  VCU.d.uptime++;
  VCU.d.bat = BAT_ScanValue();
  VCU.d.error = VCU.ReadEvent(EVG_BIKE_FALLEN);
}

void VCU_NodesInit(void) {
  BMS.Init();
  MCU.Init();
  HMI1.Init();
  HMI2.Init();
}

void VCU_NodesRefresh(void) {
  BMS.RefreshIndex();
  MCU.Refresh();
  HMI1.Refresh();
  HMI2.Refresh();
}

void VCU_SetEvent(uint8_t bit, uint8_t value) {
  if (value & 1) BV(VCU.d.events, bit);
  else BC(VCU.d.events, bit);
}

uint8_t VCU_ReadEvent(uint8_t bit) {
  return (VCU.d.events & BIT(bit)) == BIT(bit);
}

void VCU_SetDriver(uint8_t driver_id) {
  VCU.d.driver_id = driver_id;
  HMI1.d.state.unfinger = driver_id == DRIVER_ID_NONE;
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

void VCU_CheckState(void) {
  static vehicle_state_t lastState = VEHICLE_UNKNOWN;
  uint8_t starter, normalize = 0;
  vehicle_state_t initialState;

  do {
    initialState = VCU.d.state;
    starter = 0;

    if (VCU.d.gpio.starter > 0) {
      if (VCU.d.gpio.starter > 1000) {
        if (lastState > VEHICLE_NORMAL)
          normalize = 1;
      } else
        starter = 1;
      VCU.d.gpio.starter = 0;
    }

    switch (VCU.d.state) {
    case VEHICLE_LOST:
      if (lastState != VEHICLE_LOST) {
        lastState = VEHICLE_LOST;
        VCU.d.interval = RPT_INTERVAL_LOST;
        osThreadFlagsSet(ReporterTaskHandle, EVT_REPORTER_YIELD);
      }

      if (GATE_ReadPower5v())
        VCU.d.state += 2;
      break;

    case VEHICLE_BACKUP:
      if (lastState != VEHICLE_BACKUP) {
        lastState = VEHICLE_BACKUP;
        VCU.d.interval = RPT_INTERVAL_BACKUP;
        osThreadFlagsSet(ReporterTaskHandle, EVT_REPORTER_YIELD);

        osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_TASK_STOP);
        osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_TASK_STOP);
        osThreadFlagsSet(GyroTaskHandle, EVT_GYRO_TASK_STOP);
        osThreadFlagsSet(CanTxTaskHandle, EVT_CAN_TASK_STOP);
        osThreadFlagsSet(CanRxTaskHandle, EVT_CAN_TASK_STOP);

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
        VCU.d.interval = RPT_INTERVAL_NORMAL;
        osThreadFlagsSet(ReporterTaskHandle, EVT_REPORTER_YIELD);

        osThreadFlagsSet(RemoteTaskHandle, EVT_REMOTE_TASK_START);
        osThreadFlagsSet(AudioTaskHandle, EVT_AUDIO_TASK_START);
        osThreadFlagsSet(GyroTaskHandle, EVT_GYRO_TASK_START);
        osThreadFlagsSet(CanTxTaskHandle, EVT_CAN_TASK_START);
        osThreadFlagsSet(CanRxTaskHandle, EVT_CAN_TASK_START);
        osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_TASK_STOP);

        normalize = 0;
        VCU.d.override = VEHICLE_NORMAL;
        HBAR_Init();
      }

      if (!GATE_ReadPower5v())
        VCU.d.state--;
      else if ((starter && !HMI1.d.state.unremote) ||
               VCU.d.override >= VEHICLE_STANDBY
               // starter && (!HMI1.d.state.unremote || VCU.d.override >=
               // VEHICLE_STANDBY)
      )
        VCU.d.state++;
      break;

    case VEHICLE_STANDBY:
      if (lastState != VEHICLE_STANDBY) {
        lastState = VEHICLE_STANDBY;

        osThreadFlagsSet(FingerTaskHandle, EVT_FINGER_TASK_START);
      }

      if (!GATE_ReadPower5v() || normalize)
        VCU.d.state--;
      else if (!HMI1.d.state.unfinger || VCU.d.override >= VEHICLE_READY)
        VCU.d.state++;
      break;

    case VEHICLE_READY:
      if (lastState != VEHICLE_READY) {
        lastState = VEHICLE_READY;
      }

      if (!GATE_ReadPower5v() || normalize ||
          (HMI1.d.state.unfinger && !VCU.d.override) ||
          VCU.d.override == VEHICLE_STANDBY)
        VCU.d.state--;
      else if (!HMI1.d.state.warning &&
               (starter || VCU.d.override >= VEHICLE_RUN))
        VCU.d.state++;
      break;

    case VEHICLE_RUN:
      if (lastState != VEHICLE_RUN) {
        lastState = VEHICLE_RUN;
      }

      if (!GATE_ReadPower5v() || normalize || HMI1.d.state.warning ||
          VCU.d.override == VEHICLE_READY)
        VCU.d.state--;
      else if ((starter && MCU.d.speed == 0) ||
               (HMI1.d.state.unfinger &&
                VCU.d.override < VEHICLE_READY) ||
               VCU.d.override == VEHICLE_STANDBY)
        VCU.d.state -= 2;
      break;

    default:
      break;
    }
  } while (initialState != VCU.d.state);
}

uint8_t VCU_CheckRTOS(void) {
  uint8_t expectedThread = (sizeof(rtos_task_t) / sizeof(task_t));
  uint8_t activeThread = (uint8_t)(osThreadGetCount() - 2);
  if (activeThread < expectedThread) {
    printf("RTOS:Failed, active thread %d < %d\n", activeThread,
           expectedThread);
    return 0;
  }
  return 1;
}

void VCU_CheckStack(void) {
  rtos_task_t *rtos = &(VCU.d.task);

  rtos->manager.stack = osThreadGetStackSpace(ManagerTaskHandle);
  rtos->iot.stack = osThreadGetStackSpace(IotTaskHandle);
  rtos->reporter.stack = osThreadGetStackSpace(ReporterTaskHandle);
  rtos->command.stack = osThreadGetStackSpace(CommandTaskHandle);
  rtos->gps.stack = osThreadGetStackSpace(GpsTaskHandle);
  rtos->gyro.stack = osThreadGetStackSpace(GyroTaskHandle);
  rtos->remote.stack = osThreadGetStackSpace(RemoteTaskHandle);
  rtos->finger.stack = osThreadGetStackSpace(FingerTaskHandle);
  rtos->audio.stack = osThreadGetStackSpace(AudioTaskHandle);
  rtos->gate.stack = osThreadGetStackSpace(GateTaskHandle);
  rtos->canRx.stack = osThreadGetStackSpace(CanRxTaskHandle);
  rtos->canTx.stack = osThreadGetStackSpace(CanTxTaskHandle);
  //  rtos->hmi2Power.stack = osThreadGetStackSpace(Hmi2PowerTaskHandle);
}

/* ====================================== CAN TX
 * =================================== */
uint8_t VCU_CAN_TX_Heartbeat(void) {
  can_tx_t Tx;

  Tx.data.u16[0] = VCU_VERSION;

  return CANBUS_Write(&Tx, CAND_VCU, 2, 0);
}

uint8_t VCU_CAN_TX_SwitchModeControl(void) {
  can_tx_t Tx;

  Tx.data.u8[0] = HBAR.list[HBAR_K_ABS].state;
  Tx.data.u8[0] |= (VCU.d.gps.fix == 0) << 1; // HMI1.d.state.mirroring << 1;
  Tx.data.u8[0] |= HBAR.list[HBAR_K_LAMP].state << 2;
  Tx.data.u8[0] |= HMI1.d.state.warning << 3;
  Tx.data.u8[0] |= HMI1.d.state.overheat << 4;
  Tx.data.u8[0] |=
      (HMI1.d.state.unfinger && VCU.d.override < VEHICLE_READY) << 5;
  Tx.data.u8[0] |= HMI1.d.state.unremote << 6;
  Tx.data.u8[0] |= HMI1.d.state.daylight << 7;

  // sein value
  sein_t sein = HBAR_SeinController();
  Tx.data.u8[1] = sein.left;
  Tx.data.u8[1] |= sein.right << 1;
  // TODO: validate MCU reverse state
  Tx.data.u8[1] |= HBAR.reverse << 2;

  // mode
  // TODO: validate MCU drive mode
  Tx.data.u8[2] = HBAR.d.mode[HBAR_M_DRIVE];
  Tx.data.u8[2] |= HBAR.d.mode[HBAR_M_TRIP] << 2;
  Tx.data.u8[2] |= HBAR.d.mode[HBAR_M_REPORT] << 4;
  Tx.data.u8[2] |= HBAR.m << 5;
  Tx.data.u8[2] |= HBAR_ModeController() << 7;

  // others
  Tx.data.u8[3] = MCU.d.speed;
  Tx.data.u8[4] = (uint8_t)MCU.d.dcbus.current;

  // send message
  return CANBUS_Write(&Tx, CAND_VCU_SWITCH, 5, 0);
}

uint8_t VCU_CAN_TX_Datetime(datetime_t dt) {
  can_tx_t Tx;
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

uint8_t VCU_CAN_TX_MixedData(void) {
  can_tx_t Tx;

  Tx.data.u8[0] = SIM.signal;
  Tx.data.u8[1] = BMS.d.soc;
  Tx.data.u8[2] = HBAR.d.report[HBAR_M_REPORT_RANGE];
  Tx.data.u8[3] = HBAR.d.report[HBAR_M_REPORT_AVERAGE];

  return CANBUS_Write(&Tx, CAND_VCU_SELECT_SET, 4, 0);
}

uint8_t VCU_CAN_TX_TripData(void) {
  can_tx_t Tx;

  Tx.data.u16[0] = HBAR.d.trip[HBAR_M_TRIP_A] / 1000;
  Tx.data.u16[1] = HBAR.d.trip[HBAR_M_TRIP_B] / 1000;
  Tx.data.u32[1] = HBAR.d.trip[HBAR_M_TRIP_ODO] / 1000;

  return CANBUS_Write(&Tx, CAND_VCU_TRIP_MODE, 8, 0);
}
