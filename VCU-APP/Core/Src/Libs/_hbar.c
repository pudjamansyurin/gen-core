/*
 * _hbar.c
 *
 *  Created on: Apr 16, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_hbar.h"

#include "App/_vehicle.h"
#include "Libs/_eeprom.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/VCU.h"

/* Private constants
 * --------------------------------------------*/
#define MODE_SESSION_MS ((uint16_t)4000)
#define MODE_RESET_MS ((uint16_t)1500)
#define STARTER_LONG_PRESS_MS ((uint16_t)1000)

/* Private enums
 * --------------------------------------------*/
typedef enum {
  HB_STARTER_UNKNOWN = 0,
  HB_STARTER_ON,
  HB_STARTER_OFF
} HB_STARTER;

/* Private types
 * --------------------------------------------*/
typedef struct {
  uint32_t sein;
  uint32_t session;
} hbar_tick_t;

typedef struct {
  uint32_t start;
  uint32_t time;
} hbar_timer_t;

typedef struct {
  uint32_t meter;
  uint8_t session;

  uint8_t sein[HB_SEIN_MAX];
  uint8_t pin[HBP_MAX];

  HB_STARTER starter;
  HBM mode;
  uint8_t sub[HBM_MAX];
  uint8_t avg[HBMS_AVG_MAX];
  uint16_t trip[HBMS_TRIP_MAX];
} hbar_data_t;

typedef struct {
  hbar_data_t d;
  hbar_tick_t tick;
  hbar_timer_t tim[3];
} hbar_t;

/* Private variables
 * --------------------------------------------*/
static hbar_t HBAR = {.d = {0}, .tick = {0}};

/* Private functions prototype
 * --------------------------------------------*/
static uint32_t Timer(uint8_t key);
static uint8_t Reversed(void);
static void RunSelect(void);
static void RunSet(void);
static uint8_t DefferMode(void);
static uint8_t SubMask(HBM mode);

/* Public functions implementation
 * --------------------------------------------*/
void HB_Init(void) {
  memset(&(HBAR.tim), 0, sizeof(HBAR.tim));

  HBAR.d.starter = HB_STARTER_UNKNOWN;
  HBAR.d.session = 0;
  HBAR.tick.session = 0;

  HBAR.d.avg[HBMS_AVG_RANGE] = 0;
  HBAR.d.avg[HBMS_AVG_EFFICIENCY] = 0;

  HB_EE_Read();
  //	HBAR.d.mode = HBM_DRIVE;
  //	HBAR.d.sub[HBM_DRIVE] = HBMS_DRIVE_STANDARD;
  //	HBAR.d.sub[HBM_TRIP] = HBMS_TRIP_ODO;
  //	HBAR.d.sub[HBM_AVG] = HBMS_AVG_RANGE;

  //	HBAR.d.trip[HBMS_TRIP_A] = 0;
  //	HBAR.d.trip[HBMS_TRIP_B] = 0;
  //	HBAR.d.trip[HBMS_TRIP_ODO] = 0;
}

uint8_t HB_SubMax(HBM m) {
  uint8_t max;

  switch (m) {
    case HBM_DRIVE:
      max = HBMS_DRIVE_MAX;
      break;
    case HBM_TRIP:
      max = HBMS_TRIP_MAX;
      break;
    case HBM_AVG:
      max = HBMS_AVG_MAX;
      break;
    default:
      max = 0;
      break;
  }
  return max;
}

void HB_ReadStarter(uint8_t normalState) {
  HBAR.d.pin[HBP_STARTER] = GATE_ReadStarter();

  if (Timer(HBP_STARTER)) {
    uint8_t on =
        (HBAR.tim[HBP_STARTER].time < STARTER_LONG_PRESS_MS) || normalState;

    HBAR.d.starter = on ? HB_STARTER_ON : HB_STARTER_OFF;
  }
}

void HB_CheckStarter(uint8_t *start, uint8_t *shutdown) {
  HB_STARTER starter = HBAR.d.starter;

  if (starter != HB_STARTER_UNKNOWN) {
    HBAR.d.starter = HB_STARTER_UNKNOWN;
    *shutdown = starter == HB_STARTER_OFF;
    *start = starter == HB_STARTER_ON;
  }
}

void HB_ReadStates(void) {
  HBAR.d.pin[HBP_SELECT] = GATE_ReadSelect();
  HBAR.d.pin[HBP_SET] = GATE_ReadSet();
  HBAR.d.pin[HBP_SEIN_L] = GATE_ReadSeinL();
  HBAR.d.pin[HBP_SEIN_R] = GATE_ReadSeinR();
  HBAR.d.pin[HBP_REVERSE] = GATE_ReadReverse();
  HBAR.d.pin[HBP_LAMP] = GATE_ReadLamp();
  HBAR.d.pin[HBP_ABS] = GATE_ReadABS();

  if (!Reversed()) {
    if (Timer(HBP_SELECT)) HBAR.d.session++;

    if (HBAR.d.session) {
      Timer(HBP_SET);

      if (HBAR.tim[HBP_SELECT].time || HBAR.tim[HBP_SET].time) {
        HBAR.tick.session = tickMs();
        if (HBAR.tim[HBP_SELECT].time && HBAR.d.session > 1) RunSelect();
        if (HBAR.tim[HBP_SET].time) RunSet();
      }
    }
  }
}

void HB_RefreshSelectSet(void) {
  if (HBAR.d.session) {
    if (!tickIn(HBAR.tick.session, MODE_SESSION_MS) || Reversed()) {
      HBAR.d.session = 0;
      memset(&(HBAR.tim[HBP_SELECT]), 0, sizeof(hbar_timer_t));
      memset(&(HBAR.tim[HBP_SET]), 0, sizeof(hbar_timer_t));
    }
  }
}

void HB_RefreshSein(void) {
  uint8_t *sein = HBAR.d.sein;

  if (!tickIn(HBAR.tick.sein, 250)) {
    if (HBAR.d.pin[HBP_SEIN_L] || HBAR.d.pin[HBP_SEIN_R])
      HBAR.tick.sein = tickMs();

    if (HBAR.d.pin[HBP_SEIN_L] && HBAR.d.pin[HBP_SEIN_R]) {
      sein[HB_SEIN_LEFT] = !sein[HB_SEIN_LEFT];
      sein[HB_SEIN_RIGHT] = sein[HB_SEIN_LEFT];
    } else if (HBAR.d.pin[HBP_SEIN_L]) {
      sein[HB_SEIN_LEFT] = !sein[HB_SEIN_LEFT];
      sein[HB_SEIN_RIGHT] = 0;
    } else if (HBAR.d.pin[HBP_SEIN_R]) {
      sein[HB_SEIN_LEFT] = 0;
      sein[HB_SEIN_RIGHT] = !sein[HB_SEIN_RIGHT];
    } else {
      sein[HB_SEIN_LEFT] = 0;
      sein[HB_SEIN_RIGHT] = 0;
    }
  }
}

void HB_AddTrip(uint8_t m) {
  HBMS_TRIP mTrip = HBAR.d.sub[HBM_TRIP];
  uint16_t km, *odo_km = &(HBAR.d.trip[HBMS_TRIP_ODO]);

  HBAR.d.meter += m;
  km = HBAR.d.meter / 1000;

  if (km > *odo_km) {
    uint8_t d_km = km - *odo_km;

    uint16_t aTrip = HBAR.d.trip[mTrip] + d_km;
    HB_EE_Trip(mTrip, &aTrip);

    if (mTrip != HBMS_TRIP_ODO) {
      uint16_t aOdo = *odo_km + d_km;
      HB_EE_Trip(HBMS_TRIP_ODO, &aOdo);
    }
  }
}

uint8_t HB_HasSession(void) { return HBAR.d.sein > 0; }

void HB_EE_Read(void) {
  HB_EE_Mode(NULL);
  for (uint8_t m = 0; m < HBM_MAX; m++) HB_EE_Sub(m, NULL);
  for (uint8_t mTrip = 0; mTrip < HBMS_TRIP_MAX; mTrip++)
    HB_EE_Trip(mTrip, NULL);
}

void HB_EE_WriteDeffered(void) {
  if (DefferMode()) return;

  // Mode
  HBM mode = HBAR.d.mode;
  HB_EE_Mode(&mode);

  // Sub Mode
  for (uint8_t m = 0; m < HBM_MAX; m++) {
    uint8_t subMode = HBAR.d.sub[m];
    HB_EE_Sub(m, &subMode);
  }

  // Trip
  for (uint8_t mTrip = 0; mTrip < HBMS_TRIP_MAX; mTrip++) {
    uint16_t trip = HBAR.d.trip[mTrip];
    HB_EE_Trip(mTrip, &trip);
  }
}

uint8_t HB_EE_Mode(uint8_t *src) {
  void *dst = &HBAR.d.mode;
  uint8_t ok = 1;

  if (src != NULL && DefferMode())
    memcpy(dst, src, sizeof(uint8_t));
  else
    ok = EE_Cmd(VA_MODE, src, dst);

  if (HBAR.d.mode >= HBM_MAX) HBAR.d.mode = 0;

  return ok;
}

uint8_t HB_EE_Sub(HBM m, uint8_t *src) {
  void *dst = &HBAR.d.sub[m];
  uint8_t ok = 1;

  if (src != NULL && DefferMode())
    memcpy(dst, src, sizeof(uint8_t));
  else
    ok = EE_Cmd(VA_MODE_DRIVE + m, src, dst);

  if (HBAR.d.sub[m] > HB_SubMax(m)) HBAR.d.sub[m] = 0;

  return ok;
}

uint8_t HB_EE_Trip(HBMS_TRIP mTrip, uint16_t *src) {
  void *dst = &HBAR.d.trip[mTrip];
  uint8_t ok = 1;

  if (src != NULL && DefferMode())
    memcpy(dst, src, sizeof(uint16_t));
  else
    ok = EE_Cmd(VA_TRIP_A + mTrip, src, dst);

  if (mTrip == HBMS_TRIP_ODO) {
    HBAR.d.meter = HBAR.d.trip[mTrip] * 1000;
  }

  return ok;
}

uint32_t HB_IO_Meter(void) { return HBAR.d.meter; }

uint8_t HB_IO_Pin(HBP key) { return HBAR.d.pin[key] & 0x01; }

uint8_t HB_IO_Mode(void) { return HBAR.d.mode; }

uint8_t HB_IO_Sub(HBM mode) { return HBAR.d.sub[mode] & SubMask(mode); }

uint8_t HB_IO_Sein(HB_SEIN side) { return HBAR.d.sein[side]; }

uint16_t HB_IO_Trip(HBMS_TRIP key) { return HBAR.d.trip[key]; }

uint8_t HB_IO_Average(HBMS_AVG key) { return HBAR.d.avg[key]; }

void HB_IO_SetPin(HBP key, uint8_t value) { HBAR.d.pin[key] = value & 0x01; }

void HB_IO_SetSub(HBM mode, uint8_t value) {
  HBAR.d.sub[mode] = value & SubMask(mode);
}

void HB_IO_SetAverage(HBMS_AVG key, uint8_t value) { HBAR.d.avg[key] = value; }

/* Private functions implementation
 * --------------------------------------------*/
static uint32_t Timer(uint8_t key) {
  hbar_timer_t *tim = &(HBAR.tim[key]);
  uint8_t *pin = &(HBAR.d.pin[key]);

  tim->time = 0;
  if (*pin && !tim->start) {
    tim->start = tickMs();
  } else if (!(*pin) && tim->start) {
    tim->time = tickMs() - tim->start;
    tim->start = 0;
  }

  return tim->time;
}

static uint8_t Reversed(void) {
  //	return HBAR.d.pin[HBP_REVERSE];
  return MCU_Reversed();
}

static void RunSelect(void) {
  HBM mode;

  if (HBAR.d.mode >= (HBM_MAX - 1))
    mode = 0;
  else
    mode = HBAR.d.mode + 1;

  HB_EE_Mode(&mode);
}

static void RunSet(void) {
  HBM m = HBAR.d.mode;
  HBMS_TRIP mTrip = HBAR.d.sub[HBM_TRIP];
  uint16_t meter = 0;
  uint8_t mode = 0;

  if (m == HBM_TRIP) {
    if (mTrip != HBMS_TRIP_ODO)
      if (HBAR.tim[HBP_SET].time > MODE_RESET_MS) HB_EE_Trip(mTrip, &meter);
  } else {
    if (HBAR.d.sub[m] < (HB_SubMax(m) - 1)) mode = HBAR.d.sub[m] + 1;
    HB_EE_Sub(m, &mode);
  }
}

static uint8_t DefferMode(void) { return VHC_IO_State() > VEHICLE_NORMAL; }

static uint8_t SubMask(HBM mode) {
  uint8_t MASK = 0x01;

  if (HB_SubMax(mode) > 2) MASK = 0x03;
  return MASK;
}
