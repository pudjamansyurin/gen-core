/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/VCU.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Drivers/_canbus.h"
#include "Libs/_eeprom.h"
#include "Libs/_simcom.h"

/* Public variables -----------------------------------------------------------*/
vcu_t VCU = {
    .d = { 0 },
    .can = {
        .t = {
            VCU_CAN_TX_SwitchModeControl,
            VCU_CAN_TX_Datetime,
            VCU_CAN_TX_MixedData,
            VCU_CAN_TX_SubTripData
        }
    },
    VCU_Init,
    VCU_SetEvent,
    VCU_ReadEvent,
    VCU_CheckPower5v,
    VCU_SpeedToVolume,
    VCU_SetOdometer,
};

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void) {
  // reset VCU data
  //	VCU.d.state.vehicle = VEHICLE_INDEPENDENT;
  VCU.d.state.start = 0;
  VCU.d.state.override = 0;
  VCU.d.state.run = 0;
  VCU.d.state.knob = 0;
  VCU.d.state.independent = 1;

  VCU.d.interval = RPT_INTERVAL_SIMPLE;
  VCU.d.driver_id = DRIVER_ID_NONE;
  VCU.d.bat = 0;
  VCU.d.speed = 0;
  VCU.d.odometer = 0;

  VCU.d.motion.yaw = 0;
  VCU.d.motion.pitch = 0;
  VCU.d.motion.roll = 0;

  VCU.d.events = 0;

  VCU.d.seq_id.report = 0;
  VCU.d.seq_id.response = 0;
}

void VCU_SetEvent(uint64_t event_id, uint8_t value) {
  if (value & 1)
    BV(VCU.d.events, _BitPosition(event_id));
  else
    BC(VCU.d.events, _BitPosition(event_id));
}

uint8_t VCU_ReadEvent(uint64_t event_id) {
  return (VCU.d.events & event_id) == event_id;
}

void VCU_CheckPower5v(uint8_t currentState) {
  static TickType_t tick;
  static int8_t lastState = -1;

  // handle only when changed
  if (lastState != currentState) {
    lastState = currentState;
    tick = _GetTickMS();
  }

  // set things
  VCU.d.state.independent = currentState == 0;
  VCU.SetEvent(EV_VCU_INDEPENDENT, currentState == 0);

  // handle when REG_5V is OFF
  if (currentState == 0) {
    if (_GetTickMS() - tick > (VCU_ACTIVATE_LOST_MODE * 1000)) {
      VCU.d.interval = RPT_INTERVAL_LOST;
      VCU.SetEvent(EV_VCU_UNAUTHORIZE_REMOVAL, 1);
    } else {
      VCU.d.interval = RPT_INTERVAL_INDEPENDENT;
      VCU.SetEvent(EV_VCU_UNAUTHORIZE_REMOVAL, 0);
    }
  } else {
    VCU.d.interval = RPT_INTERVAL_SIMPLE;
    VCU.SetEvent(EV_VCU_UNAUTHORIZE_REMOVAL, 0);
  }
}

uint16_t VCU_SpeedToVolume(void) {
  return VCU.d.speed * 100 / MCU_SPEED_KPH_MAX ;
}

void VCU_SetOdometer(uint8_t increment) {
  static uint32_t last_km = 0;
  uint32_t odometer_km;

  VCU.d.odometer += increment;
  odometer_km = (VCU.d.odometer / 1000);

  // init hook
  if (last_km == 0)
    last_km = odometer_km;

  // check every 1km
  if (last_km < odometer_km) {
    last_km = odometer_km;

    // reset on overflow
    if (last_km > VCU_ODOMETER_KM_MAX)
      VCU.d.odometer = 0;

    // accumulate (save permanently)
    EEPROM_Odometer(EE_CMD_W, VCU.d.odometer);
  }

  HBAR_AccumulateSubTrip(increment);
}

/* ====================================== CAN TX =================================== */
uint8_t VCU_CAN_TX_SwitchModeControl(hbar_t *hbar) {
  sein_t sein = HBAR_SeinController(hbar);
  CAN_DATA TxData;

  // set message
  TxData.u8[0] = hbar->list[HBAR_K_ABS].state;
  TxData.u8[0] |= (VCU.d.gps.fix == 0) << 1; //HMI1.d.state.mirroring << 1;
  TxData.u8[0] |= hbar->list[HBAR_K_LAMP].state << 2;
  TxData.u8[0] |= HMI1.d.state.warning << 3;
  TxData.u8[0] |= HMI1.d.state.overheat << 4;
  TxData.u8[0] |= HMI1.d.state.unfinger << 5;
  TxData.u8[0] |= HMI1.d.state.unremote << 6;
  TxData.u8[0] |= HMI1.d.state.daylight << 7;

  // sein value
  TxData.u8[1] = sein.left;
  TxData.u8[1] |= sein.right << 1;
  TxData.u8[1] |= hbar->runner.reverse << 2;

  // mode
  TxData.u8[2] = hbar->runner.mode.d.val[HBAR_M_DRIVE];
  TxData.u8[2] |= hbar->runner.mode.d.val[HBAR_M_TRIP] << 2;
  TxData.u8[2] |= hbar->runner.mode.d.val[HBAR_M_REPORT] << 4;
  TxData.u8[2] |= hbar->runner.mode.m << 5;
  TxData.u8[2] |= HBAR_ModeController(&(hbar->runner)) << 7;

  // others
  TxData.u8[3] = VCU.d.speed;

  // send message
  return CANBUS_Write(CAND_VCU_SWITCH, &TxData, 4);
}

uint8_t VCU_CAN_TX_Datetime(timestamp_t *timestamp) {
  CAN_DATA TxData;

  // set message
  TxData.u8[0] = timestamp->time.Seconds;
  TxData.u8[1] = timestamp->time.Minutes;
  TxData.u8[2] = timestamp->time.Hours;
  TxData.u8[3] = timestamp->date.Date;
  TxData.u8[4] = timestamp->date.Month;
  TxData.u8[5] = timestamp->date.Year;
  TxData.u8[6] = timestamp->date.WeekDay;
  // HMI2 shutdown request
  TxData.u8[7] = !VCU.d.state.start;

  // send message
  return CANBUS_Write(CAND_VCU_DATETIME, &TxData, 8);
}

uint8_t VCU_CAN_TX_MixedData(hbar_runner_t *runner) {
  CAN_DATA TxData;

  // set message
  TxData.u8[0] = SIM.signal;
  TxData.u8[1] = BMS.d.soc;
  TxData.u8[2] = VCU.d.speed; //runner->mode.d.report[HBAR_M_REPORT_RANGE];
  TxData.u8[3] = runner->mode.d.report[HBAR_M_REPORT_AVERAGE];
  TxData.u32[1] = (uint32_t) VCU.d.gps.dop_h; // VCU.d.odometer;

  // send message
  return CANBUS_Write(CAND_VCU_SELECT_SET, &TxData, 8);
}

uint8_t VCU_CAN_TX_SubTripData(uint32_t *trip) {
  CAN_DATA TxData;

  // set message
  TxData.u32[0] = trip[HBAR_M_TRIP_A] / 1000;
  TxData.u32[1] = trip[HBAR_M_TRIP_B] / 1000;

  // send message
  return CANBUS_Write(CAND_VCU_TRIP_MODE, &TxData, 8);
}
