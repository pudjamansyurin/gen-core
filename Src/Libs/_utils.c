/*
 * _config.c
 *
 *  Created on: Aug 26, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_utils.h"

/* Public functions implementation --------------------------------------------*/
uint8_t _LedRead(void) {
  return HAL_GPIO_ReadPin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void _LedWrite(uint8_t state) {
  HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, state);
}

void _LedToggle(void) {
  HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void _LedDisco(uint16_t ms) {
  uint8_t i = 20, temp;
  uint16_t delay = ms / i;

  // preserve previous state
  temp = _LedRead();

  // run the disco
  while (i--) {
    _LedWrite(i % 2);
    osDelay(delay);
  }

  // restore previous state
  _LedWrite(temp);
}

void _Error(char msg[50]) {
  if (osKernelRunning()) {
    LOG_StrLn(msg);
  } else {
    while (1) {
      _LedToggle();
      HAL_Delay(50);
    }
  }
}

void _DebugTask(char name[20]) {
  //  LOG_Str("Task:");
  //  LOG_Buf(name, strlen(name));
  //  LOG_Enter();
}

void _DummyGenerator(db_t *db, sw_t *sw) {
  // Control HMI brightness by daylight
  db->hmi1.status.daylight = _TimeCheckDaylight(db->vcu.rtc.timestamp);
  //  // Dummy algorithm
  //  db->vcu.odometer = (db->vcu.odometer >= VCU_ODOMETER_MAX ? 0 : (db->vcu.odometer + 1));

  // Dummy Report Range
  if (!sw->runner.mode.sub.report[SW_M_REPORT_RANGE]) {
    sw->runner.mode.sub.report[SW_M_REPORT_RANGE] = 255;
  } else {
    sw->runner.mode.sub.report[SW_M_REPORT_RANGE]--;
  }

  if (sw->runner.mode.sub.report[SW_M_REPORT_AVERAGE] >= 255) {
    sw->runner.mode.sub.report[SW_M_REPORT_AVERAGE] = 0;
  } else {
    sw->runner.mode.sub.report[SW_M_REPORT_AVERAGE]++;
  }

  // Dummy Report Trip
  if (sw->runner.mode.sub.val[sw->runner.mode.val] == SW_M_TRIP_A) {
    if (sw->runner.mode.sub.trip[SW_M_TRIP_A] >= VCU_ODOMETER_MAX) {
      sw->runner.mode.sub.trip[SW_M_TRIP_A] = 0;
    } else {
      sw->runner.mode.sub.trip[SW_M_TRIP_A]++;
    }
  } else {
    if (sw->runner.mode.sub.trip[SW_M_TRIP_B] >= VCU_ODOMETER_MAX) {
      sw->runner.mode.sub.trip[SW_M_TRIP_B] = 0;
    } else {
      sw->runner.mode.sub.trip[SW_M_TRIP_B]++;
    }
  }
}

uint8_t _TimeCheckDaylight(timestamp_t timestamp) {
  return (timestamp.time.Hours >= 5 && timestamp.time.Hours <= 16);
}

uint8_t _TimeNeedCalibration(rtc_t rtc) {
  return (rtc.calibration.Year != rtc.timestamp.date.Year ||
      rtc.calibration.Month != rtc.timestamp.date.Month ||
      rtc.calibration.Date != rtc.timestamp.date.Date);
}

int8_t _BitPosition(uint64_t event_id) {
  uint8_t pos = -1;

  for (int8_t i = 0; i < 64; i++) {
    if (event_id & BIT(i)) {
      pos = i;
      break;
    }
  }

  return pos;
}

int32_t _ParseNumber(const char *ptr, uint8_t *cnt) {
  uint8_t minus = 0, i = 0;
  int32_t sum = 0;

  if (*ptr == '-') { /* Check for minus character */
    minus = 1;
    ptr++;
    i++;
  }
  while (CHARISNUM(*ptr)) { /* Parse number */
    sum = 10 * sum + CHARTONUM(*ptr);
    ptr++;
    i++;
  }
  if (cnt != NULL) { /* Save number of characters used for number */
    *cnt = i;
  }
  if (minus) { /* Minus detected */
    return 0 - sum;
  }
  return sum; /* Return number */
}

float _ParseFloatNumber(const char *ptr, uint8_t *cnt) {
  uint8_t i = 0, j = 0;
  float sum = 0.0f;

  sum = (float) _ParseNumber(ptr, &i); /* Parse number */
  j += i;
  ptr += i;
  if (*ptr == '.') { /* Check decimals */
    float dec;
    dec = (float) _ParseNumber(ptr + 1, &i);
    dec /= (float) pow(10, i);
    if (sum >= 0) {
      sum += dec;
    } else {
      sum -= dec;
    }
    j += i + 1;
  }

  if (cnt != NULL) { /* Save number of characters used for number */
    *cnt = j;
  }
  return sum; /* Return number */
}

