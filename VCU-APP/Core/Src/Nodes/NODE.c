/*
 * NODE.c
 *
 *  Created on: Mar 23, 2021
 *      Author: pujak
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/NODE.h"
#include "Libs/_remote.h"
#include "Libs/_finger.h"
#include "Libs/_debugger.h"
#include "Nodes/VCU.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/HMI1.h"
#include "Nodes/HMI2.h"

/* Public variables
 * -----------------------------------------------------------*/
node_t NODE = {
		.d = {0},
		.t = {
				NODE_TX_DebugGroup1,
				NODE_TX_DebugGroup2,
				NODE_TX_DebugVCU,
				NODE_TX_DebugGPS,
				NODE_TX_DebugMEMS,
				NODE_TX_DebugRMT,
				NODE_TX_DebugTASK,
		},
		.Init = NODE_Init,
		.Refresh = NODE_Refresh,
};

/* Public functions implementation
 * --------------------------------------------*/
void NODE_Init(void) {
	BMS.Init();
	MCU.Init();
	HMI1.Init();
	HMI2.Init();
}

void NODE_Refresh(void) {
	uint8_t eBMS = BMS.d.fault > 0;
	uint8_t eMCU = (MCU.d.fault.post | MCU.d.fault.run) > 0;

	if (VCU.d.state >= VEHICLE_READY) {
		if (VCU.d.tick.ready && (_GetTickMS() - VCU.d.tick.ready) > NODE_TIMEOUT_MS) {
			eBMS |= !BMS.d.active;
			eMCU |= !MCU.d.active;
		}
	}

	NODE.d.overheat = BMS.d.overheat || MCU.d.overheat;
	NODE.d.error = VCU.d.error || eBMS || eMCU;

	BMS.RefreshIndex();
	MCU.Refresh();
	HMI1.Refresh();
	HMI2.Refresh();
}

/* ====================================== CAN TX
 * =================================== */
void NODE_TX_DebugGroup1(void) {
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
	d->u8[2] |= (FGR.d.registering & 0x01) << 4;

	audio_dbg_t audio;
	DBG_GetAudio(&audio);
	d->u8[3] = (audio.active & 0x01);
	d->u8[3] |= (audio.mute & 0x01) << 1;
	d->u8[4] = audio.volume;

	mcu_dbg_t mcu;
	DBG_GetMCU(&mcu);
	d->u8[5] = (mcu.active & 0x01);
	d->u8[5] |= (mcu.run & 0x01) << 1;
	d->u8[5] |= (MCU.set.rpm_max & 0x01) << 2;
	d->u8[5] |= (MCU.set.template & 0x01) << 3;
	d->u8[5] |= (MCU.synced.rpm_max & 0x01) << 4;
	d->u8[5] |= (MCU.synced.template & 0x01) << 5;

	bms_dbg_t bms;
	DBG_GetBMS(&bms);
	d->u8[6] = (bms.active & 0x01);
	d->u8[6] |= (bms.run & 0x01) << 1;
	d->u8[7] = bms.soc;

	CANBUS_Write(&Tx, CAND_DBG_GROUP_1, 8, 0);
}

void NODE_TX_DebugGroup2(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	mems_dbg_t mems;
	DBG_GetMEMS(&mems);
	d->u8[0] = (mems.active & 0x01);
	d->u8[0] |= (mems.detector & 0x01) << 1;
	d->u8[0] |= (MEMS.d.fall & 0x01) << 2;
	d->u8[0] |= (MEMS.d.crash & 0x01) << 3;
	d->u8[1] = MEMS.det.offset;

	remote_dbg_t rmt;
	DBG_GetRMT(&rmt);
	d->u8[2] = (rmt.active & 0x01);
	d->u8[2] |= (rmt.nearby & 0x01) << 1;
	d->u8[3] = RMT.d.duration.tx;
	d->u8[4] = RMT.d.duration.rx;

	d->u8[5] = HBAR.ctl.starter;
	d->u8[6] = (HBAR.state[HBAR_K_SELECT] & 0x01);
	d->u8[6] |= (HBAR.state[HBAR_K_SET] & 0x01) << 1;
	d->u8[6] |= (HBAR.state[HBAR_K_STARTER] & 0x01) << 2;
	d->u8[6] |= (HBAR.state[HBAR_K_SEIN_L] & 0x01) << 3;
	d->u8[6] |= (HBAR.state[HBAR_K_SEIN_R] & 0x01) << 4;
	d->u8[6] |= (HBAR.state[HBAR_K_REVERSE] & 0x01) << 5;
	d->u8[6] |= (HBAR.state[HBAR_K_LAMP] & 0x01) << 6;
	d->u8[6] |= (HBAR.state[HBAR_K_ABS] & 0x01) << 7;

	CANBUS_Write(&Tx, CAND_DBG_GROUP_2, 7, 0);
}

void NODE_TX_DebugVCU(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	vcu_dbg_t vcu;
	DBG_GetVCU(&vcu);

	d->u16[0] = vcu.events;
	d->s8[2] = vcu.state;
	d->u8[3] = vcu.buffered;
	d->u8[4] = vcu.battery;
	CANBUS_Write(&Tx, CAND_DBG_VCU, 5, 0);
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
	CANBUS_Write(&Tx, CAND_DBG_GPS_1, 8, 0);

	d->s32[0] = gps.longitude;
	d->s32[1] = gps.latitude;
	CANBUS_Write(&Tx, CAND_DBG_GPS_2, 8, 0);
}

void NODE_TX_DebugMEMS(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	mems_dbg_t mems;
	DBG_GetMEMS(&mems);

	d->s16[0] = mems.accel.x;
	d->s16[1] = mems.accel.y;
	d->s16[2] = mems.accel.z;
	d->u16[3] = mems.total.accel;
	CANBUS_Write(&Tx, CAND_DBG_MEMS_1, 8, 0);

	d->s16[0] = mems.gyro.x;
	d->s16[1] = mems.gyro.y;
	d->s16[2] = mems.gyro.z;
	d->u16[3] = mems.total.gyro;
	CANBUS_Write(&Tx, CAND_DBG_MEMS_2, 8, 0);

	d->s16[0] = mems.ypr.pitch;
	d->s16[1] = mems.ypr.roll;
	d->u16[2] = mems.total.tilt;
	d->u16[3] = mems.total.temp;
	CANBUS_Write(&Tx, CAND_DBG_MEMS_3, 8, 0);
}

void NODE_TX_DebugRMT(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	remote_dbg_t rmt;
	DBG_GetRMT(&rmt);

	d->u32[0] = RMT.d.tick.ping;
	d->u32[1] = RMT.d.tick.heartbeat;
	CANBUS_Write(&Tx, CAND_DBG_RMT, 8, 0);
}

void NODE_TX_DebugTASK(void) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	tasks_dbg_t t;
	DBG_GetTasks(&t);

	d->u16[0] = t.stack.manager;
	d->u16[1] = t.stack.network;
	d->u16[2] = t.stack.reporter;
	d->u16[3] = t.stack.command;
	CANBUS_Write(&Tx, CAND_DBG_TASK_1, 8, 0);

	d->u16[0] = t.stack.gps;
	d->u16[1] = t.stack.mems;
	d->u16[2] = t.stack.remote;
	d->u16[3] = t.stack.finger;
	CANBUS_Write(&Tx, CAND_DBG_TASK_2, 8, 0);

	d->u16[0] = t.stack.audio;
	d->u16[1] = t.stack.gate;
	d->u16[2] = t.stack.canRx;
	d->u16[3] = t.stack.canTx;
	CANBUS_Write(&Tx, CAND_DBG_TASK_3, 8, 0);

	d->u8[0] = t.wakeup.manager;
	d->u8[1] = t.wakeup.network;
	d->u8[2] = t.wakeup.reporter;
	d->u8[3] = t.wakeup.command;
	d->u8[4] = t.wakeup.gps;
	d->u8[5] = t.wakeup.mems;
	d->u8[6] = t.wakeup.remote;
	d->u8[7] = t.wakeup.finger;
	CANBUS_Write(&Tx, CAND_DBG_TASK_4, 8, 0);

	d->u8[0] = t.wakeup.audio;
	d->u8[1] = t.wakeup.gate;
	d->u8[2] = t.wakeup.canRx;
	d->u8[3] = t.wakeup.canTx;

	vcu_dbg_t vcu;
	DBG_GetVCU(&vcu);
	d->u32[1] = vcu.uptime;
	CANBUS_Write(&Tx, CAND_DBG_TASK_5, 8, 0);
}
