/*
 * BMS.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/VCU.h"

/* Public variables -----------------------------------------------------------*/
bms_t BMS = {
		.d = { 0 },
		.can = {
				.r = {
						BMS_CAN_RX_Param1,
						BMS_CAN_RX_Param2
				},
				.t = {
						BMS_CAN_TX_Setting
				}
		},
		BMS_Init,
		BMS_PowerOverCan,
		BMS_RefreshIndex,
};

/* Private functions prototypes -----------------------------------------------*/
static void ResetIndex(uint8_t i);
static uint8_t GetIndex(uint32_t id);
static void SetEvents(uint16_t flag);
static uint16_t MergeFlags(void);
static uint8_t AverageSOC(void);
static void ResetPacks(void);
static uint8_t RunPacks(uint8_t on);

/* Public functions implementation --------------------------------------------*/
void BMS_Init(void) {
	ResetPacks();
}

void BMS_PowerOverCan(uint8_t on) {
	BMS_STATE state = on ? BMS_STATE_FULL : BMS_STATE_IDLE;
	uint8_t recover = on && VCU.ReadEvent(EV_BMS_SHORT_CIRCUIT);

	BMS.can.t.Setting(state, recover);
}

void BMS_RefreshIndex(void) {
	for (uint8_t i = 0; i < BMS_COUNT ; i++)
		if ((_GetTickMS() - BMS.d.pack[i].tick) > 500)
			ResetIndex(i);

	SetEvents(MergeFlags());
	BMS.d.soc = AverageSOC();
	BMS.d.run = RunPacks(1);
}

/* ====================================== CAN RX =================================== */
void BMS_CAN_RX_Param1(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId & BMS_ID_MASK);

	// read the content
	BMS.d.pack[i].voltage = Rx->data.u16[0] * 0.01;
	BMS.d.pack[i].current = Rx->data.u16[1] * 0.1;
	BMS.d.pack[i].soc = Rx->data.u16[2];
	BMS.d.pack[i].temperature = Rx->data.u16[3] - 40;
}

void BMS_CAN_RX_Param2(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId & BMS_ID_MASK);

	// read content
	BMS.d.pack[i].capacity = Rx->data.u16[0] * 0.1;
	BMS.d.pack[i].soh = Rx->data.u16[1];
	BMS.d.pack[i].cycle = Rx->data.u16[2];
	BMS.d.pack[i].flag = Rx->data.u16[3];
	BMS.d.pack[i].state = (((Rx->data.u8[7] >> 4) & 0x01) << 1) | ((Rx->data.u8[7] >> 5) & 0x01);

	// update index
	BMS.d.pack[i].tick = _GetTickMS();
	BMS.d.pack[i].id = Rx->header.ExtId & BMS_ID_MASK;
}

/* ====================================== CAN TX =================================== */
uint8_t BMS_CAN_TX_Setting(BMS_STATE state, uint8_t recover) {
	CAN_DATA TxData;

	// set message
	TxData.u8[0] = state << 1;
	TxData.u8[1] = recover;
	TxData.u8[2] = BMS_SCALE_15_85 & 0x03;

	// send message
	return CANBUS_Write(CAND_BMS_SETTING, &TxData, 3);
}

/* Private functions implementation --------------------------------------------*/
static void ResetIndex(uint8_t i) {
	BMS.d.pack[i].id = BMS_ID_NONE;
	BMS.d.pack[i].voltage = 0;
	BMS.d.pack[i].current = 0;
	BMS.d.pack[i].soc = 0;
	BMS.d.pack[i].temperature = 0;
	BMS.d.pack[i].capacity = 0;
	BMS.d.pack[i].soh = 0;
	BMS.d.pack[i].cycle = 0;
	BMS.d.pack[i].flag = 0;
	BMS.d.pack[i].state = BMS_STATE_OFF;
	BMS.d.pack[i].tick = 0;
}

static uint8_t GetIndex(uint32_t id) {
	uint8_t i;

	// find index (if already exist)
	for (i = 0; i < BMS_COUNT ; i++)
		if (BMS.d.pack[i].id == id)
			return i;

	// find index (if not exist)
	for (i = 0; i < BMS_COUNT ; i++)
		if (BMS.d.pack[i].id == BMS_ID_NONE)
			return i;

	// force replace first index (if already full)
	return 0;
}

static void SetEvents(uint16_t flag) {
	// Set events
	VCU.SetEvent(EV_BMS_DISCHARGE_OVER_CURRENT, (flag >> 0) & 0x01);
	VCU.SetEvent(EV_BMS_CHARGE_OVER_CURRENT, (flag >> 1) & 0x01);
	VCU.SetEvent(EV_BMS_SHORT_CIRCUIT, (flag >> 2) & 0x01);
	VCU.SetEvent(EV_BMS_DISCHARGE_OVER_TEMPERATURE, (flag >> 3) & 0x01);
	VCU.SetEvent(EV_BMS_DISCHARGE_UNDER_TEMPERATURE, (flag >> 4) & 0x01);
	VCU.SetEvent(EV_BMS_CHARGE_OVER_TEMPERATURE, (flag >> 5) & 0x01);
	VCU.SetEvent(EV_BMS_CHARGE_UNDER_TEMPERATURE, (flag >> 6) & 0x01);
	VCU.SetEvent(EV_BMS_UNDER_VOLTAGE, (flag >> 7) & 0x01);
	VCU.SetEvent(EV_BMS_OVER_VOLTAGE, (flag >> 8) & 0x01);
	VCU.SetEvent(EV_BMS_OVER_DISCHARGE_CAPACITY, (flag >> 9) & 0x01);
	VCU.SetEvent(EV_BMS_UNBALANCE, (flag >> 10) & 0x01);
	VCU.SetEvent(EV_BMS_SYSTEM_FAILURE, (flag >> 11) & 0x01);

	// Parse event for indicator
	BMS.d.overheat = VCU.ReadEvent(EV_BMS_DISCHARGE_OVER_TEMPERATURE) ||
			VCU.ReadEvent(EV_BMS_DISCHARGE_UNDER_TEMPERATURE) ||
			VCU.ReadEvent(EV_BMS_CHARGE_OVER_TEMPERATURE) ||
			VCU.ReadEvent(EV_BMS_CHARGE_UNDER_TEMPERATURE);
	BMS.d.error = BMS.d.overheat ||
			VCU.ReadEvent(EV_BMS_DISCHARGE_OVER_CURRENT) ||
			VCU.ReadEvent(EV_BMS_CHARGE_OVER_CURRENT) ||
			VCU.ReadEvent(EV_BMS_SHORT_CIRCUIT) ||
			VCU.ReadEvent(EV_BMS_UNDER_VOLTAGE) ||
			VCU.ReadEvent(EV_BMS_OVER_VOLTAGE) ||
			VCU.ReadEvent(EV_BMS_OVER_DISCHARGE_CAPACITY) ||
			VCU.ReadEvent(EV_BMS_UNBALANCE) ||
			VCU.ReadEvent(EV_BMS_SYSTEM_FAILURE);
}

static uint16_t MergeFlags(void) {
	uint16_t flags = 0;

	for (uint8_t i = 0; i < BMS_COUNT ; i++)
		flags |= BMS.d.pack[i].flag;

	return flags;
}

static uint8_t AverageSOC(void) {
	uint8_t soc = 0, dev = 0;

	for (uint8_t i = 0; i < BMS_COUNT ; i++) {
		if (BMS.d.pack[i].state != BMS_STATE_OFF) {
			soc += BMS.d.pack[i].soc;
			dev++;
		}
	}

	return dev ? (soc / dev) : soc;
}

static void ResetPacks(void) {
	for (uint8_t i = 0; i < BMS_COUNT ; i++)
		ResetIndex(i);

	BMS.d.run = 0;
	BMS.d.soc = 0;
	BMS.d.error = 0;
	BMS.d.overheat = 0;
}

static uint8_t RunPacks(uint8_t on) {
	BMS_STATE state = on ? BMS_STATE_FULL : BMS_STATE_IDLE;

	for (uint8_t i = 0; i < BMS_COUNT ; i++) {
		if (BMS.d.pack[i].state != state)
			return 0;
		if (on && BMS.d.pack[i].flag != 0)
			return 0;
	}
	return 1;
}
