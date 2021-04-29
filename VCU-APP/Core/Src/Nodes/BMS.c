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
		.Init = BMS_Init,
		.PowerOverCan = BMS_PowerOverCan,
		.RefreshIndex = BMS_RefreshIndex,
};

/* Private functions prototypes
 * -----------------------------------------------*/
static void ResetIndex(uint8_t i);
static void ResetFaults(void);
static uint8_t GetIndex(uint32_t addr);
static uint16_t MergeFault(void);
static uint8_t MergeSOC(void);
static uint8_t AreActive(void);
static uint8_t AreOverheat(void);
static uint8_t AreRunning(uint8_t on);

/* Public functions implementation
 * --------------------------------------------*/
void BMS_Init(void) {
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		ResetIndex(i);
	ResetFaults();

	BMS.d.active = 0;
	BMS.d.run = 0;
	BMS.d.soc = 0;
	BMS.d.overheat = 0;
}

void BMS_PowerOverCan(uint8_t on) {
	static uint8_t lastState = 0;
	BMS_STATE state = on ? BMS_STATE_FULL : BMS_STATE_IDLE;
	uint8_t sc = on && (BMS.d.fault & BIT(BMSF_SHORT_CIRCUIT));

	if (lastState != on) {
		lastState = on;
		if (on) ResetFaults();
	}
	BMS.t.Setting(state, sc);
}

void BMS_RefreshIndex(void) {
	BMS.d.active = AreActive();
	BMS.d.run = BMS.d.active && AreRunning(1);

	BMS.d.soc = MergeSOC();
	BMS.d.fault = MergeFault();
	BMS.d.overheat = AreOverheat();

	VCU.SetEvent(EVG_BMS_ERROR, BMS.d.fault > 0);
}

/* ====================================== CAN RX
 * =================================== */
void BMS_RX_Param1(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId);
	UNION64 *d = &(Rx->data);
	pack_t *pack = &(BMS.d.packs[i]);

	// read the content
	pack->voltage = d->u16[0] * 0.01;
	pack->current = d->u16[1] * 0.1;
	pack->soc = d->u16[2];
	pack->temperature = d->u16[3] - 40;

	// update index
	pack->id = BMS_ID(Rx->header.ExtId);
	pack->tick = _GetTickMS();
}

void BMS_RX_Param2(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId);
	UNION64 *d = &(Rx->data);
	pack_t *pack = &(BMS.d.packs[i]);

	// read content
	pack->capacity = d->u16[0] * 0.1;
	pack->soh = d->u16[1];
	pack->cycle = d->u16[2];
	pack->fault = d->u16[3] & 0x0FFF;
	pack->state = (((d->u8[7] >> 4) & 0x01) << 1) | ((d->u8[7] >> 5) & 0x01);

	// update index
	pack->id = BMS_ID(Rx->header.ExtId);
	pack->tick = _GetTickMS();
}

/* ====================================== CAN TX
 * =================================== */
uint8_t BMS_TX_Setting(BMS_STATE state, uint8_t sc) {
	can_tx_t Tx = {0};
	UNION64 *d = &(Tx.data);

	// set message
	d->u8[0] = state << 1;
	d->u8[1] = sc;
	d->u8[2] = BMS_SCALE_15_85 & 0x03;

	// send message
	return CANBUS_Write(&Tx, CAND_BMS_SETTING, 8, 1);
}

/* Private functions implementation
 * --------------------------------------------*/
static void ResetIndex(uint8_t i) {
	pack_t *pack = &(BMS.d.packs[i]);

	pack->run = 0;
	pack->tick = 0;
	pack->state = BMS_STATE_OFF;
	pack->id = BMS_ID_NONE;
	pack->voltage = 0;
	pack->current = 0;
	pack->soc = 0;
	pack->temperature = 0;
	pack->capacity = 0;
	pack->soh = 0;
	pack->cycle = 0;
}

static void ResetFaults(void) {
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		BMS.d.packs[i].fault = 0;
	BMS.d.fault = 0;
}

static uint8_t GetIndex(uint32_t addr) {
	// find index (if already exist)
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.d.packs[i].id == BMS_ID(addr))
			return i;

	// find index (if not exist)
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.d.packs[i].id == BMS_ID_NONE)
			return i;

	// force replace first index (if already full)
	return 0;
}

static uint16_t MergeFault(void) {
	uint16_t fault = 0;

	for (uint8_t i = 0; i < BMS_COUNT; i++)
		fault |= BMS.d.packs[i].fault;

	return fault;
}

static uint8_t MergeSOC(void) {
	uint8_t soc = 100;

	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.d.packs[i].soc < soc)
			soc = BMS.d.packs[i].soc;

	return soc;
}

static uint8_t AreActive(void) {
	uint8_t active = 1;
	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		pack_t *pack = &(BMS.d.packs[i]);

		pack->active = pack->tick && (_GetTickMS() - pack->tick) < BMS_TIMEOUT_MS;
		if (!pack->active) {
			ResetIndex(i);
			active = 0;
		}
	}
	return active;
}

static uint8_t AreOverheat(void) {
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

static uint8_t AreRunning(uint8_t on) {
	BMS_STATE state = on ? BMS_STATE_FULL : BMS_STATE_IDLE;

	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		pack_t *pack = &(BMS.d.packs[i]);

		if (pack->state != state)
			return 0;
		if (on && pack->fault)
			return 0;
	}
	return 1;
}
