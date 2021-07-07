/*
 * _ml.c
 *
 *  Created on: Jun 21, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_ml.h"

#include "Nodes/MCU.h"

/* Exported constants
 * --------------------------------------------*/
#define BMS_AVG_SZ ((uint8_t)10)

/* Private types
 * --------------------------------------------*/
typedef struct {
	avg_float_t handle[BMS_AVG_MAX];
	float buffer[BMS_AVG_MAX][BMS_AVG_SZ];
} bms_avg_t;

typedef struct {
	struct {
		bms_prediction_t d;
		bms_avg_t avg;
	} bms;
} ml_t;

/* Private variables
 * --------------------------------------------*/
static ml_t ML = {
    .bms = {
    		.d = {0},
    }
};

/* Private functions prototype
 * --------------------------------------------*/
static uint8_t CalculateRange(uint32_t dms);
static void BMS_GetPrediction(uint8_t *eff, uint8_t *km, uint8_t distance);
static float BMS_GetEfficiency(uint8_t distance);
static float BMS_GetTotalCapacity(void);
static float BMS_GetDischargeCapacity(uint32_t duration);
static float BMS_AddSample(BMS_AVG_TYPE type, float val);

/* Public functions implementation
 * --------------------------------------------*/
void ML_BMS_Init(void) {
	memset(&ML.bms.avg, 0, sizeof(bms_avg_t));
}

void ML_PredictRange(void) {
  static uint32_t tick = 0;
  uint8_t d, eff, km;

  if (_TickOut(tick, 1000)) {
    d = CalculateRange(_GetTickMS() - tick);
    tick = _GetTickMS();

    BMS_GetPrediction(&eff, &km, d);

    HBAR_SetReport(eff, km);
    HBAR_AddTripMeter(d);
  } else if (tick == 0)
    tick = _GetTickMS();
}

bms_prediction_t ML_IO_GetDataBMS(void) {
	return ML.bms.d;
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t CalculateRange(uint32_t dms) {
  uint8_t meter;
  float mps;

  mps = (float)MCU_RpmToSpeed(MCU.d.rpm) / 3.6;
  meter = (dms * mps) / 1000;

  return meter;
}

static void BMS_GetPrediction(uint8_t *eff, uint8_t *km, uint8_t d) {
  bms_prediction_t *pre = &(ML.bms.d);

  pre->capacity = BMS_GetTotalCapacity();
  pre->efficiency = BMS_GetEfficiency(d);
  pre->distance = (pre->efficiency * pre->capacity) / 1000.0;

  *eff = MAX_U8(pre->efficiency);
  *km = MAX_U8(pre->distance);
}

static float BMS_GetEfficiency(uint8_t d) {
  static uint32_t tick;
  static uint8_t _on = 0;
  static float wh, _wh;
  float mwh = 0;

  if (BMS.d.active != _on) {
    _on = BMS.d.active;
    _wh = 0;
  } else if (BMS.d.active) {
    wh = BMS_GetDischargeCapacity(_GetTickMS() - tick);

    if (d) {
      if (wh != _wh) {
        mwh = d / (wh - _wh);
        mwh *= mwh < 0 ? -1 : 1;

        _wh = wh;
      } else
        mwh = ML.bms.d.efficiency;
    }
  }
  tick = _GetTickMS();

  return BMS_AddSample(BMS_AVG_EFFICIENCY, mwh);
}

static float BMS_GetTotalCapacity(void) {
  bms_pack_t *p = &(BMS.packs[BMS_MinIndex()]);
  float V, I, wh;

  I = (p->soc * p->capacity) / 100.0;
  V = p->voltage;
  wh = I * V;

  return BMS_AddSample(BMS_AVG_CAPACITY, wh * 2.0);
}

static float BMS_GetDischargeCapacity(uint32_t duration) {
  bms_pack_t *p = &(BMS.packs[BMS_MinIndex()]);
  float V, I, wh;

  I = p->current;
  V = p->voltage;
  wh = (I * V * duration) / (3600.0 * 1000.0);

  return BMS_AddSample(BMS_AVG_DISCHARGE, wh * 2.0);
}

static float BMS_AddSample(BMS_AVG_TYPE type, float val) {
	bms_avg_t *avg = &ML.bms.avg;

  return _MovAvgFloat(&(avg->handle[type]), avg->buffer[type], BMS_AVG_SZ, val);
}
