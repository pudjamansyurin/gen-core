/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Nodes/VCU.h"

#include "Drivers/_bat.h"
#include "Drivers/_simcom.h"
#include "Libs/_finger.h"
#include "Libs/_remote.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/NODE.h"

/* Public variables
 * --------------------------------------------*/
vcu_t VCU = {
    .d = {0},
};

/* External variables
 * --------------------------------------------*/
extern osMessageQueueId_t ReportQueueHandle;

/* Public functions implementation
 * --------------------------------------------*/
void VCU_Init(void) {
  memset(&(VCU.d), 0, sizeof(vcu_data_t));
  VCU.d.vehicle = VEHICLE_BACKUP;
}

void VCU_Refresh(void) {
  BAT_ScanValue();

  VCU.d.uptime++;
  VCU.d.buffered = osMessageQueueGetCount(ReportQueueHandle);
  VCU.d.error = EVT_Get(EVG_BIKE_FALLEN);
}

/* CAN TX
 * --------------------------------------------*/
uint8_t VCU_TX_Heartbeat(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  d->u16[0] = VCU_VERSION;

  return CANBUS_Write(&Tx, CAND_VCU, 2, 0);
}

uint8_t VCU_TX_SwitchControl(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  const finger_data_t *finger = FGR_IO_GetData();

  d->u8[0] = HB_IO_GetPin(HBP_ABS);
  d->u8[0] |= HB_IO_GetPin(HBP_LAMP) << 2;
  d->u8[0] |= NODE.d.error << 3;
  d->u8[0] |= NODE.d.overheat << 4;
  d->u8[0] |= !finger->id << 5;
  d->u8[0] |= !RMT_IO_GetNearby() << 6;
  d->u8[0] |= RTC_Daylight() << 7;

  // sein value
  HB_RefreshSein();

  d->u8[1] = HB_IO_GetSein(HB_SEIN_LEFT);
  d->u8[1] |= HB_IO_GetSein(HB_SEIN_RIGHT) << 1;
  //	d->u8[1] |= HB_IO_GetPin(HBP_REVERSE) << 2;
  d->u8[1] |= MCU_Reversed() << 2;
  d->u8[1] |= BMS.d.run << 3;
  d->u8[1] |= MCU.d.run << 4;
  d->u8[1] |= (finger->registering & 0x03) << 5;

  // mode
  d->u8[2] = HB_IO_GetSub(HBM_DRIVE);
  //	d->u8[2] = MCU.d.drive_mode;
  d->u8[2] |= HB_IO_GetSub(HBM_TRIP) << 2;
  d->u8[2] |= HB_IO_GetSub(HBM_AVG) << 4;
  d->u8[2] |= HB_IO_GetMode() << 5;
  d->u8[2] |= HB_HasSession() << 7;

  // others
  d->u8[3] = MCU_RpmToSpeed(MCU.d.rpm);
  d->u8[4] = (uint8_t)MCU.d.dcbus.current;
  d->u8[5] = BMS.d.soc;
  d->u8[6] = SIM.d.signal;
  d->u8[7] = (int8_t)VCU.d.vehicle;

  // send message
  return CANBUS_Write(&Tx, CAND_VCU_SWITCH_CTL, 8, 0);
}

uint8_t VCU_TX_Datetime(datetime_t dt) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  uint8_t hmi2shutdown = VCU.d.vehicle < VEHICLE_STANDBY;

  d->u8[0] = dt.Seconds;
  d->u8[1] = dt.Minutes;
  d->u8[2] = dt.Hours;
  d->u8[3] = dt.Date;
  d->u8[4] = dt.Month;
  d->u8[5] = dt.Year;
  d->u8[6] = dt.WeekDay;
  d->u8[7] = hmi2shutdown;

  return CANBUS_Write(&Tx, CAND_VCU_DATETIME, 8, 0);
}

uint8_t VCU_TX_ModeData(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  d->u16[0] = HB_IO_GetTrip(HBMS_TRIP_A);
  d->u16[1] = HB_IO_GetTrip(HBMS_TRIP_B);
  d->u16[2] = HB_IO_GetTrip(HBMS_TRIP_ODO);
  d->u8[6] = HB_IO_GetAverage(HBMS_AVG_RANGE);
  d->u8[7] = HB_IO_GetAverage(HBMS_AVG_EFFICIENCY);

  return CANBUS_Write(&Tx, CAND_VCU_MODE_DATA, 8, 0);
}
