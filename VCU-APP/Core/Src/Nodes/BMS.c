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
		.PowerOverCAN = BMS_PowerOverCAN,
		.RefreshIndex = BMS_RefreshIndex,
		.MinIndex = BMS_MinIndex,
		.GetMPerWH = BMS_GetMPerWH,
		.GetRangeKM = BMS_GetRangeKM,
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
static float GetMinWH(void);
static float MovAvg(averager_t *m, float val);

/* Public functions implementation
 * --------------------------------------------*/
void BMS_Init(void) {
	memset(&(BMS.d), 0, sizeof(bms_data_t));

	for (uint8_t i = 0; i < BMS_COUNT; i++)
		ResetIndex(i);
}

void BMS_PowerOverCAN(uint8_t on) {
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

uint8_t BMS_GetMPerWH(uint32_t odo) {
	static averager_t avgMWH = {0}, avgD = {0};
	static uint32_t _odo = 0;
	static uint8_t _on = 0;
	static float wh, _wh = 0;
	float mwh;

	if (BMS.d.active != _on) {
		_on = BMS.d.active;
		_wh = GetMinWH();
		_odo = odo;
	}

	if (BMS.d.active) {
		wh = GetMinWH();

		if (odo != _odo) {
			if (wh != _wh) {
				float d = MovAvg(&avgD, odo - _odo);
				mwh = d / (wh - _wh);
				mwh *= (mwh < 0) ? -1 : 1;

				_wh = wh;
			} else
				mwh = BMS.d.mwh;

			_odo = odo;
		} else
			mwh = 0;
	} else
		mwh = 0;

	BMS.d.mwh = MovAvg(&avgMWH, mwh);
	return BMS.d.mwh > UINT8_MAX ? UINT8_MAX : BMS.d.mwh;
}

uint8_t BMS_GetRangeKM(void) {
	static averager_t avg = {0};
	float m = BMS.d.mwh * BMS.d.wh;

	BMS.d.km =  MovAvg(&avg, m / 1000.0);
	return BMS.d.km > UINT8_MAX ? UINT8_MAX : BMS.d.km;
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

static float GetMinWH(void) {
	static averager_t avg = {0};
	pack_t *p = &(BMS.packs[BMS.MinIndex()]);
	float V, I, wh;

	I = (p->soc * p->capacity) / 100.0;
	V = p->voltage - 51.34;
	V = V < 0 ? 0 : V;
	wh = (I * V) * 2.0;

	BMS.d.wh = MovAvg(&avg, wh);
	return BMS.d.wh;
}

static float MovAvg(averager_t *m, float val) {
	uint16_t sz = sizeof(m->buf) / sizeof(m->buf[0]);

	// Subtract the oldest number from the prev sum, add the new number
	m->sum = m->sum - m->buf[m->pos] + val;
	// Assign the nextNum to the position in the array
	m->buf[m->pos] = val;
	// Increment position
	m->pos++;
	if (m->pos >= sz)
		m->pos = 0;
	// calculate filled array
	if (m->len < sz)
		m->len++;
	// return the average
	return m->sum / m->len;
}
