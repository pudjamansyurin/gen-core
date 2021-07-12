/*
 * predictor.c
 *
 *  Created on: Jun 21, 2021
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/predictor.h"

#include "Nodes/MCU.h"

/* Exported constants
 * --------------------------------------------*/
#define SAMPLE_SZ ((uint8_t)10)

/* Private types
 * --------------------------------------------*/
typedef struct {
  sample_float_t handle[BMS_SAMPLE_MAX];
  float buffer[BMS_SAMPLE_MAX][SAMPLE_SZ];
} bms_sample_t;

typedef struct {
  bms_avg_t avg;
  bms_sample_t sample;
} predictor_t;

/* Private variables
 * --------------------------------------------*/
static predictor_t PR = {
    .avg = {0},
};

/* Private functions prototype
 * --------------------------------------------*/
static uint16_t Duration(void);
static void CalculateAverage(uint8_t distance);
static float GetTotalCapacity(void);
static float GetDischargeCapacity(uint32_t duration);
static float GetEfficiency(uint8_t distance);
static float AddSample(BMS_SAMPLE_TYPE type, float val);

/* Public functions implementation
 * --------------------------------------------*/
void PR_Init(void) { memset(&PR.sample, 0, sizeof(bms_sample_t)); }

void PR_EstimateRange(void) {
  uint16_t duration = Duration();

  if (duration == 0) return;

  uint8_t distance = MCU_GetMileage(duration);
  CalculateAverage(distance);

  HB_AddTrip(distance);
  HB_IO_SetAverage(HBMS_AVG_EFFICIENCY, PR.avg.efficiency);
  HB_IO_SetAverage(HBMS_AVG_RANGE, PR.avg.range);
}

const bms_avg_t *PR_IO_Avg(void) { return &(PR.avg); }

/* Private functions implementation
 * --------------------------------------------*/
static uint16_t Duration(void) {
  static uint32_t tick = 0;
  uint16_t duration = 0;

  if (tick == 0)
    tick = tickMs();
  else if (tickOut(tick, 1000)) {
    duration = tickMs() - tick;
    tick = tickMs();
  }

  return duration;
}

static void CalculateAverage(uint8_t distance) {
  bms_avg_t *avg = &(PR.avg);

  avg->capacity = GetTotalCapacity();
  avg->efficiency = MAX_U8(GetEfficiency(distance));
  avg->range = MAX_U8((avg->efficiency * avg->capacity) / 1000.0);
}

static float GetTotalCapacity(void) {
  bms_pack_t *p = &(BMS.packs[BMS_MinIndex()]);
  float V, I, wh;

  I = (p->soc * p->capacity) / 100.0;
  V = p->voltage;
  wh = I * V;

  return AddSample(BMS_SAMPLE_CAPACITY, wh * 2.0);
}

static float GetDischargeCapacity(uint32_t duration) {
  bms_pack_t *p = &(BMS.packs[BMS_MinIndex()]);
  float V, I, wh;

  I = p->current;
  V = p->voltage;
  wh = (I * V * duration) / (3600.0 * 1000.0);

  return AddSample(BMS_SAMPLE_DISCHARGE, wh * 2.0);
}

static float GetEfficiency(uint8_t d) {
  static uint32_t tick;
  static uint8_t _on = 0;
  static float wh, _wh;
  float mwh = 0;

  if (BMS.d.active != _on) {
    _on = BMS.d.active;
    _wh = 0;
  } else if (BMS.d.active) {
    wh = GetDischargeCapacity(tickMs() - tick);

    if (d) {
      if (wh != _wh) {
        mwh = d / (wh - _wh);
        mwh *= mwh < 0 ? -1 : 1;

        _wh = wh;
      } else
        mwh = PR.avg.efficiency;
    }
  }
  tick = tickMs();

  return AddSample(BMS_SAMPLE_EFFICIENCY, mwh);
}

static float AddSample(BMS_SAMPLE_TYPE type, float val) {
  bms_sample_t *s = &PR.sample;

  return samplingFloat(&(s->handle[type]), s->buffer[type], SAMPLE_SZ, val);
}
