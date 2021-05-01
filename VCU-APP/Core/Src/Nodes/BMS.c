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
		.r = {
				BMS_RX_Param1,
				BMS_RX_Param2
		},
		.t = {
				BMS_TX_Setting
		},
		.Init = BMS_Init,
		.PowerOverCan = BMS_PowerOverCan,
		.RefreshIndex = BMS_RefreshIndex,
		.MinIndex = BMS_MinIndex,
		.GetMinKWH = BMS_GetMinKWH,
		.GetKmPerKwh = BMS_GetKmPerKwh,
};

/* Private functions prototypes
 * -----------------------------------------------*/
static void ResetIndex(uint8_t i);
static void ResetFaults(void);
static uint8_t GetIndex(uint32_t addr);
static uint16_t MergeFault(void);
static uint8_t AreActive(void);
static uint8_t AreOverheat(void);
static uint8_t AreRunning(uint8_t on);

/* Public functions implementation
 * --------------------------------------------*/
void BMS_Init(void) {
	memset(&(BMS.d), 0, sizeof(bms_data_t));

	for (uint8_t i = 0; i < BMS_COUNT; i++)
		ResetIndex(i);
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

	BMS.d.soc = BMS.packs[BMS.MinIndex()].soc;
	BMS.d.fault = MergeFault();
	BMS.d.overheat = AreOverheat();

	VCU.SetEvent(EVG_BMS_ERROR, BMS.d.fault > 0);
}

uint8_t BMS_MinIndex(void) {
	uint8_t soc = 100;
	uint8_t index = 0;

	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		if (BMS.packs[i].soc < soc) {
			soc = BMS.packs[i].soc;
			index = i;
		}
	}

	return index;
}

float BMS_GetMinKWH(void) {
	pack_t *p = &(BMS.packs[BMS.MinIndex()]);
	float kwh;

	kwh = (float) (p->soc * p->capacity * p->voltage) / 100.0;

	BMS.d.kwh = kwh;
	return kwh;
}

float BMS_GetKmPerKwh(uint8_t m) {
	static uint8_t lastON = 0;
	static uint8_t lastM = 0;
	static float lastKWH = 0;
	float km_kwh = 0;

	if (BMS.d.active != lastON) {
		lastON = BMS.d.active;
		lastKWH = BMS.GetMinKWH();
		lastM = m;
	}

	if (BMS.d.active) {
		float kwh = BMS.GetMinKWH();

		if (kwh < lastKWH) {
			km_kwh = ((m - lastM) / (kwh - lastKWH)) / 1000.0;
			lastKWH = kwh;
			lastM = m;
		}
	}

	BMS.d.km_kwh = km_kwh;
	return km_kwh;
}

/* ====================================== CAN RX
 * =================================== */
void BMS_RX_Param1(can_rx_t *Rx) {
	uint8_t i = GetIndex(Rx->header.ExtId);
	pack_t *pack = &(BMS.packs[i]);
	UNION64 *d = &(Rx->data);

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
	pack_t *pack = &(BMS.packs[i]);
	UNION64 *d = &(Rx->data);

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
	pack_t *pack = &(BMS.packs[i]);

	memset(pack, 0, sizeof(pack_t));
	pack->state = BMS_STATE_OFF;
	pack->id = BMS_ID_NONE;
}

static void ResetFaults(void) {
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		BMS.packs[i].fault = 0;
	BMS.d.fault = 0;
}

static uint8_t GetIndex(uint32_t addr) {
	// find index (if already exist)
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.packs[i].id == BMS_ID(addr))
			return i;

	// find index (if not exist)
	for (uint8_t i = 0; i < BMS_COUNT; i++)
		if (BMS.packs[i].id == BMS_ID_NONE)
			return i;

	// force replace first index (if already full)
	return 0;
}

static uint16_t MergeFault(void) {
	uint16_t fault = 0;

	for (uint8_t i = 0; i < BMS_COUNT; i++)
		fault |= BMS.packs[i].fault;

	return fault;
}

static uint8_t AreActive(void) {
	uint8_t active = 1;

	for (uint8_t i = 0; i < BMS_COUNT; i++) {
		pack_t *pack = &(BMS.packs[i]);

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
		pack_t *pack = &(BMS.packs[i]);

		if (pack->state != state)
			return 0;
		if (on && pack->fault)
			return 0;
	}
	return 1;
}
