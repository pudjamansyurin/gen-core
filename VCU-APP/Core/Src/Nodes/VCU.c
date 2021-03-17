/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_canbus.h"
#include "Drivers/_simcom.h"
#include "Libs/_eeprom.h"
#include "Libs/_handlebar.h"
#include "Nodes/VCU.h"
#include "Nodes/MCU.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"

/* Public variables -----------------------------------------------------------*/
vcu_t VCU = {
    .d = { 0 },
    .can = {
        .t = {
            VCU_CAN_TX_SwitchModeControl,
            VCU_CAN_TX_Datetime,
            VCU_CAN_TX_MixedData,
            VCU_CAN_TX_TripData
        }
    },
    VCU_Init,
    VCU_SetEvent,
    VCU_ReadEvent,
    VCU_SetDriver,
    VCU_SetOdometer,
};

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void) {
  // reset VCU data
  VCU.d.state.error = 0;
  VCU.d.state.override = 0;
  VCU.d.state.vehicle = VEHICLE_BACKUP;

  VCU.d.interval = RPT_INTERVAL_BACKUP;
  VCU.d.driver_id = DRIVER_ID_NONE;
  VCU.d.bat = 0;

  VCU.d.motion.yaw = 0;
  VCU.d.motion.roll = 0;
  VCU.d.motion.pitch = 0;

  VCU.d.events = 0;
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

/* ====================================== CAN TX =================================== */
uint8_t VCU_CAN_TX_SwitchModeControl(void) {
	can_tx_t Tx;

  // set message
  Tx.data.u8[0] = HBAR.list[HBAR_K_ABS].state;
  Tx.data.u8[0] |= (VCU.d.gps.fix == 0) << 1; //HMI1.d.state.mirroring << 1;
  Tx.data.u8[0] |= HBAR.list[HBAR_K_LAMP].state << 2;
  Tx.data.u8[0] |= HMI1.d.state.warning << 3;
  Tx.data.u8[0] |= HMI1.d.state.overheat << 4;
  Tx.data.u8[0] |= (HMI1.d.state.unfinger && VCU.d.state.override < VEHICLE_READY) << 5;
  Tx.data.u8[0] |= HMI1.d.state.unremote << 6;
  Tx.data.u8[0] |= HMI1.d.state.daylight << 7;

  // sein value
  sein_t sein = HBAR_SeinController();
  Tx.data.u8[1] = sein.left;
  Tx.data.u8[1] |= sein.right << 1;
  Tx.data.u8[1] |= HBAR.reverse << 2;

  // mode
  Tx.data.u8[2] = HBAR.d.mode[HBAR_M_DRIVE];
  Tx.data.u8[2] |= HBAR.d.mode[HBAR_M_TRIP] << 2;
  Tx.data.u8[2] |= HBAR.d.mode[HBAR_M_REPORT] << 4;
  Tx.data.u8[2] |= HBAR.m << 5;
  Tx.data.u8[2] |= HBAR_ModeController() << 7;

  // others
  Tx.data.u8[3] = MCU.d.speed;

  // send message
  return CANBUS_Write(&Tx, CAND_VCU_SWITCH, 4, 0);
}

uint8_t VCU_CAN_TX_Datetime(datetime_t dt) {
	can_tx_t Tx;

  // set message
  Tx.data.u8[0] = dt.Seconds;
  Tx.data.u8[1] = dt.Minutes;
  Tx.data.u8[2] = dt.Hours;
  Tx.data.u8[3] = dt.Date;
  Tx.data.u8[4] = dt.Month;
  Tx.data.u8[5] = dt.Year;
  Tx.data.u8[6] = dt.WeekDay;
  // HMI2 shutdown request
  Tx.data.u8[7] = VCU.d.state.vehicle < VEHICLE_STANDBY;

  // send message
  return CANBUS_Write(&Tx, CAND_VCU_DATETIME, 8, 0);
}

uint8_t VCU_CAN_TX_MixedData(void) {
	can_tx_t Tx;

  // set message
  Tx.data.u8[0] = SIM.signal;
  Tx.data.u8[1] = BMS.d.soc;
  Tx.data.u8[2] = HBAR.d.report[HBAR_M_REPORT_RANGE];
  Tx.data.u8[3] = HBAR.d.report[HBAR_M_REPORT_AVERAGE];

  // send message
  return CANBUS_Write(&Tx, CAND_VCU_SELECT_SET, 4, 0);
}

uint8_t VCU_CAN_TX_TripData(void) {
	can_tx_t Tx;

  // set message
  Tx.data.u16[0] = HBAR.d.trip[HBAR_M_TRIP_A] / 1000;
  Tx.data.u16[1] = HBAR.d.trip[HBAR_M_TRIP_B] / 1000;
  Tx.data.u32[1] = HBAR.d.trip[HBAR_M_TRIP_ODO] / 1000;

  // send message
  return CANBUS_Write(&Tx, CAND_VCU_TRIP_MODE, 8, 0);
}
