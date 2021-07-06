/*
 * _hbar.c
 *
 *  Created on: Apr 16, 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_hbar.h"

#include "Libs/_eeprom.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/VCU.h"

/* Public variables
 * --------------------------------------------*/
hbar_t HBAR = {.d = {0}, .ctl = {0}};

/* Private functions prototype
 * --------------------------------------------*/
static uint32_t Timer(uint8_t key);
static uint8_t Reversed(void);
static void RunSelect(void);
static void RunSet(void);
static uint8_t Deffered(void *src);

/* Public functions implementation
 * --------------------------------------------*/
void HBAR_Init(void) {
  memset(&(HBAR.tim), 0, sizeof(HBAR.tim));

  HBAR.ctl.starter = HBAR_STARTER_UNKNOWN;
  HBAR.ctl.session = 0;
  HBAR.ctl.tick.session = 0;

  HBAR.d.prediction[HBAR_M_PREDICTION_RANGE] = 0;
  HBAR.d.prediction[HBAR_M_PREDICTION_EFFICIENCY] = 0;

  HBAR_EE_Read();
  //	HBAR.d.m = HBAR_M_DRIVE;
  //	HBAR.d.mode[HBAR_M_DRIVE] = HBAR_M_DRIVE_STANDARD;
  //	HBAR.d.mode[HBAR_M_TRIP] = HBAR_M_TRIP_ODO;
  //	HBAR.d.mode[HBAR_M_PREDICTION] = HBAR_M_PREDICTION_RANGE;

  //	HBAR.d.trip[HBAR_M_TRIP_A] = 0;
  //	HBAR.d.trip[HBAR_M_TRIP_B] = 0;
  //	HBAR.d.trip[HBAR_M_TRIP_ODO] = 0;
}

uint8_t HBAR_SubModeMax(HBAR_MODE m) {
  uint8_t max;

  switch (m) {
    case HBAR_M_DRIVE:
      max = HBAR_M_DRIVE_MAX;
      break;
    case HBAR_M_TRIP:
      max = HBAR_M_TRIP_MAX;
      break;
    case HBAR_M_PREDICTION:
      max = HBAR_M_PREDICTION_MAX;
      break;
    default:
      max = 0;
      break;
  }
  return max;
}

void HBAR_ReadStarter(uint8_t normalState) {
  HBAR.d.pin[HBAR_K_STARTER] = GATE_ReadStarter();

  if (Timer(HBAR_K_STARTER)) {
    uint8_t on =
        (HBAR.tim[HBAR_K_STARTER].time < STARTER_LONG_PRESS_MS) || normalState;

    HBAR.ctl.starter = on ? HBAR_STARTER_ON : HBAR_STARTER_OFF;
  }
}

void HBAR_CheckStarter(uint8_t *start, uint8_t *shutdown) {
  HBAR_STARTER starter = HBAR.ctl.starter;

  if (starter != HBAR_STARTER_UNKNOWN) {
    HBAR.ctl.starter = HBAR_STARTER_UNKNOWN;
    *shutdown = starter == HBAR_STARTER_OFF;
    *start = starter == HBAR_STARTER_ON;
  }
}

void HBAR_ReadStates(void) {
  HBAR.d.pin[HBAR_K_SELECT] = GATE_ReadSelect();
  HBAR.d.pin[HBAR_K_SET] = GATE_ReadSet();
  HBAR.d.pin[HBAR_K_SEIN_L] = GATE_ReadSeinL();
  HBAR.d.pin[HBAR_K_SEIN_R] = GATE_ReadSeinR();
  HBAR.d.pin[HBAR_K_REVERSE] = GATE_ReadReverse();
  HBAR.d.pin[HBAR_K_LAMP] = GATE_ReadLamp();
  HBAR.d.pin[HBAR_K_ABS] = GATE_ReadABS();

  if (!Reversed()) {
    if (Timer(HBAR_K_SELECT)) HBAR.ctl.session++;

    if (HBAR.ctl.session) {
      Timer(HBAR_K_SET);

      if (HBAR.tim[HBAR_K_SELECT].time || HBAR.tim[HBAR_K_SET].time) {
        HBAR.ctl.tick.session = _GetTickMS();
        if (HBAR.tim[HBAR_K_SELECT].time && HBAR.ctl.session > 1) RunSelect();
        if (HBAR.tim[HBAR_K_SET].time) RunSet();
      }
    }
  }
}

void HBAR_RefreshSelectSet(void) {
  if (HBAR.ctl.session) {
    if (!_TickIn(HBAR.ctl.tick.session, MODE_SESSION_MS) || Reversed()) {
      HBAR.ctl.session = 0;
      memset(&(HBAR.tim[HBAR_K_SELECT]), 0, sizeof(hbar_timer_t));
      memset(&(HBAR.tim[HBAR_K_SET]), 0, sizeof(hbar_timer_t));
    }
  }
}

void HBAR_RefreshSein(void) {
  hbar_sein_t *sein = &(HBAR.ctl.sein);

  if (!_TickIn(HBAR.ctl.tick.sein, 250)) {
    if (HBAR.d.pin[HBAR_K_SEIN_L] || HBAR.d.pin[HBAR_K_SEIN_R])
      HBAR.ctl.tick.sein = _GetTickMS();

    if (HBAR.d.pin[HBAR_K_SEIN_L] && HBAR.d.pin[HBAR_K_SEIN_R]) {
      sein->left = !sein->left;
      sein->right = sein->left;
    } else if (HBAR.d.pin[HBAR_K_SEIN_L]) {
      sein->left = !sein->left;
      sein->right = 0;
    } else if (HBAR.d.pin[HBAR_K_SEIN_R]) {
      sein->left = 0;
      sein->right = !sein->right;
    } else {
      sein->left = 0;
      sein->right = 0;
    }
  }
}

void HBAR_AddTripMeter(uint8_t m) {
  HBAR_MODE_TRIP mTrip = HBAR.d.mode[HBAR_M_TRIP];
  uint16_t km, *odo_km = &(HBAR.d.trip[HBAR_M_TRIP_ODO]);

  HBAR.d.meter += m;
  km = HBAR.d.meter / 1000;

  if (km > *odo_km) {
    uint8_t d_km = km - *odo_km;

    uint16_t aTrip = HBAR.d.trip[mTrip] + d_km;
    HBAR_EE_TripMeter(mTrip, &aTrip);

    if (mTrip != HBAR_M_TRIP_ODO) {
      uint16_t aOdo = *odo_km + d_km;
      HBAR_EE_TripMeter(HBAR_M_TRIP_ODO, &aOdo);
    }
  }
}

void HBAR_SetReport(uint8_t eff, uint8_t km) {
  HBAR.d.prediction[HBAR_M_PREDICTION_EFFICIENCY] = eff;
  HBAR.d.prediction[HBAR_M_PREDICTION_RANGE] = km;
}

void HBAR_EE_Read(void) {
  HBAR_EE_Mode(NULL);
  for (uint8_t m = 0; m < HBAR_M_MAX; m++) HBAR_EE_SubMode(m, NULL);
  for (uint8_t mTrip = 0; mTrip < HBAR_M_TRIP_MAX; mTrip++)
    HBAR_EE_TripMeter(mTrip, NULL);
}

void HBAR_EE_WriteDeffered(void) {
  uint8_t dummy;

  if (!Deffered(&dummy)) {
    // Mode
    HBAR_MODE mode = HBAR.d.m;
    HBAR_EE_Mode(&mode);
    // Sub Mode
    for (uint8_t m = 0; m < HBAR_M_MAX; m++) {
      uint8_t subMode = HBAR.d.mode[m];
      HBAR_EE_SubMode(m, &subMode);
    }
    // Trip (Non-ODO)
    for (uint8_t mTrip = 0; mTrip < HBAR_M_TRIP_MAX; mTrip++) {
      if (mTrip != HBAR_M_TRIP_ODO) {
        uint16_t trip = HBAR.d.trip[mTrip];
        HBAR_EE_TripMeter(mTrip, &trip);
      }
    }
  }
}

uint8_t HBAR_EE_Mode(uint8_t *src) {
  void *dst = &HBAR.d.m;
  uint8_t ok = 1;

  if (Deffered(src))
    memcpy(dst, src, sizeof(uint8_t));
  else
    ok = EE_Cmd(VA_MODE, src, dst);

  if (HBAR.d.m >= HBAR_M_MAX) HBAR.d.m = 0;

  return ok;
}

uint8_t HBAR_EE_SubMode(HBAR_MODE m, uint8_t *src) {
  void *dst = &HBAR.d.mode[m];
  uint8_t ok = 1;

  if (Deffered(src))
    memcpy(dst, src, sizeof(uint8_t));
  else
    ok = EE_Cmd(VA_MODE_DRIVE + m, src, dst);

  if (HBAR.d.mode[m] > HBAR_SubModeMax(m)) HBAR.d.mode[m] = 0;

  return ok;
}

uint8_t HBAR_EE_TripMeter(HBAR_MODE_TRIP mTrip, uint16_t *src) {
  void *dst = &HBAR.d.trip[mTrip];
  uint8_t ok = 1;

  if (Deffered(src) && mTrip != HBAR_M_TRIP_ODO)
    memcpy(dst, src, sizeof(uint16_t));
  else
    ok = EE_Cmd(VA_TRIP_A + mTrip, src, dst);

  if (mTrip == HBAR_M_TRIP_ODO) HBAR.d.meter = *src * 1000;

  return ok;
}

/* Private functions implementation
 * --------------------------------------------*/
static uint32_t Timer(uint8_t key) {
  hbar_timer_t *tim = &(HBAR.tim[key]);
  uint8_t *pin = &(HBAR.d.pin[key]);

  tim->time = 0;
  if (*pin && !tim->start) {
    tim->start = _GetTickMS();
  } else if (!(*pin) && tim->start) {
    tim->time = _GetTickMS() - tim->start;
    tim->start = 0;
  }

  return tim->time;
}

static uint8_t Reversed(void) {
  //	return HBAR.d.pin[HBAR_K_REVERSE];
  return MCU_Reversed();
}

static void RunSelect(void) {
  HBAR_MODE mode;

  if (HBAR.d.m >= (HBAR_M_MAX - 1))
    mode = 0;
  else
    mode = HBAR.d.m + 1;

  HBAR_EE_Mode(&mode);
}

static void RunSet(void) {
  HBAR_MODE_TRIP mTrip = HBAR.d.mode[HBAR_M_TRIP];
  HBAR_MODE m = HBAR.d.m;
  uint16_t meter = 0;
  uint8_t mode = 0;

  if (m == HBAR_M_TRIP) {
    if (mTrip != HBAR_M_TRIP_ODO)
      if (HBAR.tim[HBAR_K_SET].time > MODE_RESET_MS)
        HBAR_EE_TripMeter(mTrip, &meter);
  } else {
    if (HBAR.d.mode[m] < (HBAR_SubModeMax(m) - 1)) mode = HBAR.d.mode[m] + 1;
    HBAR_EE_SubMode(m, &mode);
  }
}

static uint8_t Deffered(void *src) {
  return (src != NULL && VCU.d.state > VEHICLE_NORMAL);
}
