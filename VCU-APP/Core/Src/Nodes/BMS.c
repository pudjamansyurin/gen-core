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

/* Public variables
 * -----------------------------------------------------------*/
bms_t BMS = {
		.d = {0},
		.r = {BMS_RX_Param1, BMS_RX_Param2},
		.t = {BMS_TX_Setting},
		BMS_Init,
		BMS_PowerOverCan,
		BMS_RefreshIndex,
};

/* Private functions prototypes
 * -----------------------------------------------*/
static void ResetIndex(uint8_t i);
static uint8_t GetIndex(uint32_t addr);
static uint8_t IsOverheat(void);
static uint16_t MergeFault(void);
static uint8_t AverageSOC(void);
static void ResetPacks(void);
static uint8_t RunPacks(uint8_t on);

/* Public functions implementation
 * --------------------------------------------*/
void BMS_Init(void) { ResetPacks(); }

void BMS_PowerOverCan(uint8_t on) {
	BMS_STATE state = on ? BMS_STATE_FULL : BMS_STATE_IDLE;
	uint8_t sc = on && (BMS.d.fault & BIT(BMSF_SHORT_CIRCUIT));

	BMS.t.Setting(state, sc);
}

void BMS_RefreshIndex(void) {
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if ((_GetTickMS() - BMS.d.pack[i].tick) > BMS_TIMEOUT)
			ResetIndex(i);

	BMS.d.soc = AverageSOC();
	BMS.d.fault = MergeFault();
	BMS.d.overheat = IsOverheat();
	BMS.d.run = RunPacks(1);
	BMS.d.error = (VCU.d.state == VEHICLE_RUN && !BMS.d.run) || BMS.d.fault > 0;

	VCU.SetEvent(EVG_BMS_ERROR, BMS.d.error);
}

/* ====================================== CAN RX
 * =================================== */
void BMS_RX_Param1(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId);

	// read the content
	BMS.d.pack[i].voltage = Rx->data.u16[0] * 0.01;
	BMS.d.pack[i].current = Rx->data.u16[1] * 0.1;
	BMS.d.pack[i].soc = Rx->data.u16[2];
	BMS.d.pack[i].temperature = Rx->data.u16[3] - 40;

	// update index
	BMS.d.pack[i].id = BMS_ID(Rx->header.ExtId);
	BMS.d.pack[i].tick = _GetTickMS();
}

void BMS_RX_Param2(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId);

	// read content
	BMS.d.pack[i].capacity = Rx->data.u16[0] * 0.1;
	BMS.d.pack[i].soh = Rx->data.u16[1];
	BMS.d.pack[i].cycle = Rx->data.u16[2];
	BMS.d.pack[i].fault = Rx->data.u16[3] & 0x0FFF;
	BMS.d.pack[i].state =
			(((Rx->data.u8[7] >> 4) & 0x01) << 1) | ((Rx->data.u8[7] >> 5) & 0x01);

	// update index
	BMS.d.pack[i].id = BMS_ID(Rx->header.ExtId);
	BMS.d.pack[i].tick = _GetTickMS();
}

/* ====================================== CAN TX
 * =================================== */
uint8_t BMS_TX_Setting(BMS_STATE state, uint8_t sc) {
	can_tx_t Tx = {0};

	// set message
	Tx.data.u8[0] = state << 1;
	Tx.data.u8[1] = sc;
	Tx.data.u8[2] = BMS_SCALE_15_85 & 0x03;

	// send message
	return CANBUS_Write(&Tx, CAND_BMS_SETTING, 8, 1);
}

/* Private functions implementation
 * --------------------------------------------*/
static void ResetIndex(uint8_t i) {
	BMS.d.pack[i].tick = 0;
	BMS.d.pack[i].state = BMS_STATE_OFF;
	BMS.d.pack[i].id = BMS_ID_NONE;
	BMS.d.pack[i].voltage = 0;
	BMS.d.pack[i].current = 0;
	BMS.d.pack[i].soc = 0;
	BMS.d.pack[i].temperature = 0;
	BMS.d.pack[i].capacity = 0;
	BMS.d.pack[i].soh = 0;
	BMS.d.pack[i].cycle = 0;
	BMS.d.pack[i].fault = 0;
}

static uint8_t GetIndex(uint32_t addr) {
	// find index (if already exist)
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.d.pack[i].id == BMS_ID(addr))
			return i;

	// find index (if not exist)
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.d.pack[i].id == BMS_ID_NONE)
			return i;

	// force replace first index (if already full)
	return 0;
}

static uint8_t IsOverheat(void) {
	BMS_FAULT_BIT overheat[] = {
			BMSF_DISCHARGE_OVER_TEMPERATURE,
			BMSF_DISCHARGE_UNDER_TEMPERATURE,
			BMSF_CHARGE_OVER_TEMPERATURE,
			BMSF_CHARGE_UNDER_TEMPERATURE,
	};

	uint8_t temp = 0;
	for (uint8_t i = 0; i < sizeof(overheat); i++)
		temp |= BMS.d.fault & BIT(overheat[i]);

	return temp;
}

static uint16_t MergeFault(void) {
	uint16_t fault = 0;

	for (uint8_t i = 0; i < BMS_COUNT; i++)
		fault |= BMS.d.pack[i].fault;

	return fault;
}

static uint8_t AverageSOC(void) {
	uint8_t soc = 0, dev = 0;

	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		if (BMS.d.pack[i].state != BMS_STATE_OFF) {
			soc += BMS.d.pack[i].soc;
			dev++;
		}
	}

	return dev ? (soc / dev) : soc;
}

static void ResetPacks(void) {
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		ResetIndex(i);

	BMS.d.run = RunPacks(0);
	BMS.d.soc = 0;
	BMS.d.fault = 0;
	BMS.d.error = 0;
	BMS.d.overheat = 0;
}

static uint8_t RunPacks(uint8_t on) {
	BMS_STATE state = on ? BMS_STATE_FULL : BMS_STATE_IDLE;

	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		if (BMS.d.pack[i].state != state)
			return 0;
		if (on && BMS.d.pack[i].fault != 0)
			return 0;
	}
	return 1;
}
