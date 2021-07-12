/*
 * NODE.c
 *
 *  Created on: Mar 23, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Nodes/NODE.h"

#include "App/_debugger.h"
#include "App/_predictor.h"
#include "Drivers/_aes.h"
#include "Libs/_finger.h"
#include "Libs/_remote.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/MCU.h"
#include "Nodes/VCU.h"

/* Public variables
 * --------------------------------------------*/
node_t NODE = {
    .d = {0},
};

/* Public functions implementation
 * --------------------------------------------*/
void NODE_Init(void) {
  BMS_Init();
  MCU_Init();
  HMI1_Init();
}

void NODE_Refresh(void) {
  uint8_t eBMS = BMS.d.fault > 0;
  uint8_t eMCU = (MCU.d.fault.post | MCU.d.fault.run) > 0;

  if (VCU.d.vehicle >= VEHICLE_READY) {
    if (tickOut(VCU.d.tick.ready, NODE_TIMEOUT_MS)) {
      eBMS |= !BMS.d.active;
      eMCU |= !MCU.d.active;
    }
  }

  NODE.d.overheat = BMS.d.overheat || MCU.d.overheat;
  NODE.d.error = VCU.d.error || eBMS || eMCU;
  if (!tickIn(NODE.d.tick.dbg, NODE_DEBUG_MS)) NODE.d.debug = 0;

  BMS_RefreshIndex();
  MCU_Refresh();
  HMI1_Refresh();
}

void NODE_DebugCAN(void) {
  if (CDBG_ID(NODE.d.debug, CDBG_GROUP)) NODE_TX_DebugGroup();
  if (CDBG_ID(NODE.d.debug, CDBG_VCU)) NODE_TX_DebugVCU();
  if (CDBG_ID(NODE.d.debug, CDBG_GPS)) NODE_TX_DebugGPS();
  if (CDBG_ID(NODE.d.debug, CDBG_MEMS)) NODE_TX_DebugMEMS();
  if (CDBG_ID(NODE.d.debug, CDBG_RMT)) NODE_TX_DebugRMT();
  if (CDBG_ID(NODE.d.debug, CDBG_TASK)) NODE_TX_DebugTASK();
  if (CDBG_ID(NODE.d.debug, CDBG_MCU)) NODE_TX_DebugMCU();
  if (CDBG_ID(NODE.d.debug, CDBG_BMS)) NODE_TX_DebugBMS();
}

/* CAN RX
 * --------------------------------------------*/
void NODE_RX_Debug(can_rx_t *Rx) {
  UNION64 *d = &(Rx->data);

  NODE.d.debug = d->u8[0];
  NODE.d.tick.dbg = tickMs();
}

/* CAN TX
 * --------------------------------------------*/
void NODE_TX_DebugGroup(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  net_dbg_t net;
  DBG_GetNET(&net);
  d->s8[0] = net.state;
  d->s8[1] = net.ipstatus;

  finger_dbg_t fgr;
  DBG_GetFGR(&fgr);
  d->u8[2] = (fgr.verified & 0x01);
  d->u8[2] |= (fgr.driver_id & 0x07) << 1;
  //  d->u8[2] |= (FGR.d.registering & 0x01) << 4;

  audio_dbg_t audio;
  DBG_GetAudio(&audio);
  d->u8[3] = (audio.active & 0x01);
  d->u8[3] |= (audio.mute & 0x01) << 1;
  d->u8[4] = audio.volume;

  d->u8[5] = HB_IO_Pin(HBP_SELECT);
  d->u8[5] |= HB_IO_Pin(HBP_SET) << 1;
  d->u8[5] |= HB_IO_Pin(HBP_STARTER) << 2;
  d->u8[5] |= HB_IO_Pin(HBP_SEIN_L) << 3;
  d->u8[5] |= HB_IO_Pin(HBP_SEIN_R) << 4;
  d->u8[5] |= HB_IO_Pin(HBP_REVERSE) << 5;
  d->u8[5] |= HB_IO_Pin(HBP_LAMP) << 6;
  d->u8[5] |= HB_IO_Pin(HBP_ABS) << 7;

  CAN_Write(&Tx, CAND_DBG_GROUP, 6, 0);
}

void NODE_TX_DebugVCU(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  vcu_dbg_t vcu;
  DBG_GetVCU(&vcu);

  d->u16[0] = vcu.events;
  d->u8[2] = vcu.buffered;
  d->u8[3] = vcu.battery;
  d->u32[1] = HB_IO_Meter();

  CAN_Write(&Tx, CAND_DBG_VCU, 8, 0);
}

void NODE_TX_DebugGPS(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  gps_dbg_t gps;
  DBG_GetGPS(&gps);

  d->u8[0] = (gps.active & 0x01);
  d->u8[1] = gps.sat_in_use;
  d->u8[2] = gps.hdop;
  d->u8[3] = gps.vdop;
  d->u8[4] = gps.speed;
  d->u8[5] = gps.heading;
  d->u16[3] = gps.altitude;
  CAN_Write(&Tx, CAND_DBG_GPS_1, 8, 0);

  d->s32[0] = gps.longitude;
  d->s32[1] = gps.latitude;
  CAN_Write(&Tx, CAND_DBG_GPS_2, 8, 0);
}

void NODE_TX_DebugMEMS(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  mems_dbg_t mems;
  DBG_GetMEMS(&mems);

  d->u8[0] = (mems.active & 0x01);
  d->u8[0] |= (mems.motion_active & 0x01) << 1;
  d->u8[0] |= MEMS_IO_Effect(MEFFECT_FALL) << 2;
  d->u8[0] |= MEMS_IO_Effect(MEFFECT_CRASH) << 3;
  d->u8[1] = MEMS_IO_MotionOffset();
  CAN_Write(&Tx, CAND_DBG_MEMS_1, 2, 0);

  d->s16[0] = mems.accel.x;
  d->s16[1] = mems.accel.y;
  d->s16[2] = mems.accel.z;
  d->u16[3] = mems.total.accel;
  CAN_Write(&Tx, CAND_DBG_MEMS_2, 8, 0);

  d->s16[0] = mems.gyro.x;
  d->s16[1] = mems.gyro.y;
  d->s16[2] = mems.gyro.z;
  d->u16[3] = mems.total.gyro;
  CAN_Write(&Tx, CAND_DBG_MEMS_3, 8, 0);

  d->s16[0] = mems.tilt.pitch;
  d->s16[1] = mems.tilt.roll;
  d->u16[2] = mems.total.tilt;
  d->u16[3] = mems.total.temp;
  CAN_Write(&Tx, CAND_DBG_MEMS_4, 8, 0);
}

void NODE_TX_DebugRMT(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  remote_dbg_t rmt;
  DBG_GetRMT(&rmt);

  d->u8[0] = (rmt.active & 0x01);
  d->u8[0] |= (rmt.nearby & 0x01) << 1;
  d->u8[1] = RMT_IO_Duration(RMT_DUR_TX);
  d->u8[2] = RMT_IO_Duration(RMT_DUR_RX);
  d->u8[3] = RMT_IO_Duration(RMT_DUR_FULL);
  d->u32[1] = AES_IO_QuarterKey();
  CAN_Write(&Tx, CAND_DBG_RMT_1, 8, 0);

  d->u32[0] = RMT_IO_Tick(RMT_TICK_PING);
  d->u32[1] = RMT_IO_Tick(RMT_TICK_HBEAT);
  CAN_Write(&Tx, CAND_DBG_RMT_2, 8, 0);
}

void NODE_TX_DebugTASK(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  tasks_dbg_t t;
  DBG_GetTasks(&t);

  d->u16[0] = TASK_IO_Stack(TASK_MANAGER);
  d->u16[1] = TASK_IO_Stack(TASK_NETWORK);
  d->u16[2] = TASK_IO_Stack(TASK_REPORTER);
  d->u16[3] = TASK_IO_Stack(TASK_COMMAND);
  CAN_Write(&Tx, CAND_DBG_TASK_1, 8, 0);

  d->u16[0] = 0;
  d->u16[1] = TASK_IO_Stack(TASK_MEMS);
  d->u16[2] = TASK_IO_Stack(TASK_REMOTE);
  d->u16[3] = TASK_IO_Stack(TASK_FINGER);
  CAN_Write(&Tx, CAND_DBG_TASK_2, 8, 0);

  d->u16[0] = TASK_IO_Stack(TASK_AUDIO);
  d->u16[1] = TASK_IO_Stack(TASK_GATE);
  d->u16[2] = TASK_IO_Stack(TASK_CANRX);
  d->u16[3] = TASK_IO_Stack(TASK_CANTX);
  CAN_Write(&Tx, CAND_DBG_TASK_3, 8, 0);

  d->u8[0] = TASK_IO_Wakeup(TASK_MANAGER);
  d->u8[1] = TASK_IO_Wakeup(TASK_NETWORK);
  d->u8[2] = TASK_IO_Wakeup(TASK_REPORTER);
  d->u8[3] = TASK_IO_Wakeup(TASK_COMMAND);
  d->u8[4] = 0;
  d->u8[5] = TASK_IO_Wakeup(TASK_MEMS);
  d->u8[6] = TASK_IO_Wakeup(TASK_REMOTE);
  d->u8[7] = TASK_IO_Wakeup(TASK_FINGER);
  CAN_Write(&Tx, CAND_DBG_TASK_4, 8, 0);

  d->u8[0] = TASK_IO_Wakeup(TASK_AUDIO);
  d->u8[1] = TASK_IO_Wakeup(TASK_GATE);
  d->u8[2] = TASK_IO_Wakeup(TASK_CANRX);
  d->u8[3] = TASK_IO_Wakeup(TASK_CANTX);

  vcu_dbg_t vcu;
  DBG_GetVCU(&vcu);
  d->u32[1] = vcu.uptime;
  CAN_Write(&Tx, CAND_DBG_TASK_5, 8, 0);
}

void NODE_TX_DebugMCU(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  mcu_dbg_t mcu;
  DBG_GetMCU(&mcu);

  d->u8[0] = (mcu.active & 0x01);
  d->u8[0] |= (mcu.run & 0x01) << 1;
  d->u8[0] |= (MCU.set.rpm_max & 0x01) << 2;
  d->u8[0] |= (MCU.set.template & 0x01) << 3;
  d->u8[0] |= (MCU.synced.rpm_max & 0x01) << 4;
  d->u8[0] |= (MCU.synced.template & 0x01) << 5;
  d->u8[1] = mcu.par.speed_max;
  d->s16[1] = mcu.par.tpl[0].discur_max;
  d->s16[2] = mcu.par.tpl[1].discur_max;
  d->s16[3] = mcu.par.tpl[2].discur_max;
  CAN_Write(&Tx, CAND_DBG_MCU_1, 8, 0);

  d->s16[0] = mcu.par.rpm_max;
  d->s16[1] = mcu.par.tpl[0].torque_max;
  d->s16[2] = mcu.par.tpl[1].torque_max;
  d->s16[3] = mcu.par.tpl[2].torque_max;
  CAN_Write(&Tx, CAND_DBG_MCU_2, 8, 0);
}

void NODE_TX_DebugBMS(void) {
  can_tx_t Tx = {0};
  UNION64 *d = &(Tx.data);

  bms_dbg_t bms;
  DBG_GetBMS(&bms);
  d->u8[0] = (bms.active & 0x01);
  d->u8[0] |= (bms.run & 0x01) << 1;
  d->u8[1] = bms.soc;

  d->u16[1] = PR_IO_Avg()->capacity * 10;
  d->u16[2] = PR_IO_Avg()->efficiency * 10;
  d->u16[3] = PR_IO_Avg()->range * 10;

  CAN_Write(&Tx, CAND_DBG_BMS, 8, 0);
}
