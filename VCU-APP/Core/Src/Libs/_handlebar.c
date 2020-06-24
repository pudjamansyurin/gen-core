/*
 * _handlebar.c
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_handlebar.h"
#include "_defines.h"

/* Public variables -----------------------------------------------------------*/
sw_t SW = {
        .list = {
                {
                        .event = "SELECT",
                        .pin = EXT_HBAR_SELECT_Pin,
                        .port = EXT_HBAR_SELECT_GPIO_Port,
                        .state = 0
                },
                {
                        .event = "SET",
                        .pin = EXT_HBAR_SET_Pin,
                        .port = EXT_HBAR_SET_GPIO_Port,
                        .state = 0
                },
                {
                        .event = "SEIN LEFT",
                        .pin = EXT_HBAR_SEIN_L_Pin,
                        .port = EXT_HBAR_SEIN_L_GPIO_Port,
                        .state = 0
                },
                {
                        .event = "SEIN RIGHT",
                        .pin = EXT_HBAR_SEIN_R_Pin,
                        .port = EXT_HBAR_SEIN_R_GPIO_Port,
                        .state = 0
                },
                {
                        .event = "REVERSE",
                        .pin = EXT_HBAR_REVERSE_Pin,
                        .port = EXT_HBAR_REVERSE_GPIO_Port,
                        .state = 0
                },
                {
                        .event = "ABS",
                        .pin = EXT_ABS_IRQ_Pin,
                        .port = EXT_ABS_IRQ_GPIO_Port,
                        .state = 0
                },
                {
                        .event = "LAMP",
                        .pin = EXT_HBAR_LAMP_Pin,
                        .port = EXT_HBAR_LAMP_GPIO_Port,
                        .state = 0
                }
        },
        .timer = {
                {
                        .start = 0,
                        .running = 0,
                        .time = 0
                },
                {
                        .start = 0,
                        .running = 0,
                        .time = 0
                }
        },
        .runner = {
                .listening = 0,
                .hazard = 0,
                .mode = {
                        .val = SW_M_DRIVE,
                        .sub = {
                                .val = {
                                        SW_M_DRIVE_E,
                                        SW_M_TRIP_A,
                                        SW_M_REPORT_RANGE
                                },
                                .max = {
                                        SW_M_DRIVE_MAX,
                                        SW_M_TRIP_MAX,
                                        SW_M_REPORT_MAX
                                },
                                .report = { 0, 0 },
                                .trip = { 0, 0 }
                        }
                }
        }
};

/* Private variables ----------------------------------------------------------*/
static uint8_t mode;

/* Public functions implementation --------------------------------------------*/
void HBAR_ReadStates(void) {
    // Read all EXTI state
    for (uint8_t i = 0; i < SW_TOTAL_LIST; i++) {
        SW.list[i].state = HAL_GPIO_ReadPin(SW.list[i].port, SW.list[i].pin);
    }

    // check is reverse mode active
    HBAR_CheckReverse();
}

void HBAR_RestoreMode(void) {
    if (SW.runner.mode.sub.val[SW_M_DRIVE] == SW_M_DRIVE_R) {
        SW.runner.mode.sub.val[SW_M_DRIVE] = mode;
    }
}

void HBAR_CheckReverse(void) {
    if (SW.list[SW_K_REVERSE].state) {
        // save previous Drive Mode state
        if (SW.runner.mode.sub.val[SW_M_DRIVE] != SW_M_DRIVE_R) {
            mode = SW.runner.mode.sub.val[SW_M_DRIVE];
        }
        // force state
        SW.runner.mode.sub.val[SW_M_DRIVE] = SW_M_DRIVE_R;
    }

    // set hazard
    SW.runner.hazard = SW.list[SW_K_REVERSE].state;
}

void HBAR_TimerSelectSet(void) {
    for (uint8_t i = 0; i < SW_TOTAL_LIST; i++) {
        if (i == SW_K_SELECT || i == SW_K_SET) {
            // reset SET timer
            SW.timer[i].time = 0;

            // next job
            if (SW.list[i].state) {
                if (i == SW_K_SELECT || (i == SW_K_SET && SW.runner.listening)) {
                    // start timer if not running
                    if (!SW.timer[i].running) {
                        // set flag
                        SW.timer[i].running = 1;
                        // start timer for SET
                        SW.timer[i].start = _GetTickMS();
                    }
                }
                // reverse it
                SW.list[i].state = 0;
            } else {
                // stop timer if running
                if (SW.timer[i].running) {
                    // set flag
                    SW.timer[i].running = 0;
                    // stop SET
                    SW.timer[i].time = (_GetTickMS() - SW.timer[i].start) / 1000;
                    // reverse it
                    SW.list[i].state = 1;
                }
            }
        }
    }
}

void HBAR_RunSelect(void) {
    if (SW.runner.listening) {
        // change mode position
        if (SW.runner.mode.val == SW_M_MAX) {
            SW.runner.mode.val = 0;
        } else {
            SW.runner.mode.val++;
        }
    }
    // Listening on option
    SW.runner.listening = 1;
}

void HBAR_RunSet(void) {
    SW_MODE sMode = SW.runner.mode.val;

    if (SW.runner.listening ||
            (SW.timer[SW_K_SET].time >= 3 && sMode == SW_M_TRIP)) {
        // handle reset only if push more than n sec, and in trip mode
        if (!SW.runner.listening) {
            // reset value
            SW.runner.mode.sub.trip[SW.runner.mode.sub.val[sMode]] = 0;
        } else {
            // if less than n sec
            if (SW.runner.mode.sub.val[sMode] == SW.runner.mode.sub.max[sMode]) {
                SW.runner.mode.sub.val[sMode] = 0;
            } else {
                SW.runner.mode.sub.val[sMode]++;
            }
        }
    }
}

void HBAR_AccumulateSubTrip(void) {
    SW_MODE_TRIP mTrip = SW.runner.mode.sub.val[SW_M_TRIP];

    if (SW.runner.mode.sub.trip[mTrip] >= VCU_ODOMETER_MAX) {
        SW.runner.mode.sub.trip[mTrip] = 0;
    } else {
        SW.runner.mode.sub.trip[mTrip]++;
    }
}

sein_state_t HBAR_SeinController(sw_t *sw) {
    static TickType_t tickSein;
    static sein_state_t sein = {
            .left = 0,
            .right = 0
    };

    if ((_GetTickMS() - tickSein) >= 500) {
        if (sw->runner.hazard) {
            // hazard
            tickSein = _GetTickMS();
            sein.left = !sein.left;
            sein.right = !sein.right;
        } else if (sw->list[SW_K_SEIN_LEFT].state) {
            // left sein
            tickSein = _GetTickMS();
            sein.left = !sein.left;
            sein.right = 0;
        } else if (sw->list[SW_K_SEIN_RIGHT].state) {
            // right sein
            tickSein = _GetTickMS();
            sein.left = 0;
            sein.right = !sein.right;
        } else {
            sein.left = 0;
            sein.right = 0;
        }
    }

    return sein;
}

uint8_t HBAR_ModeController(sw_runner_t *runner) {
    static TickType_t tick, tickPeriod;
    static uint8_t iHide = 0;
    static int8_t iName = -1, iValue = -1;

    // MODE Show/Hide Manipulator
    if (runner->listening) {
        // if mode same
        if (iName != runner->mode.val) {
            iName = runner->mode.val;
            // reset period tick
            tickPeriod = _GetTickMS();

        } else if (iValue != runner->mode.sub.val[runner->mode.val]) {
            iValue = runner->mode.sub.val[runner->mode.val];
            // reset period tick
            tickPeriod = _GetTickMS();
        }

        if ((_GetTickMS() - tickPeriod) >= 5000 ||
                (runner->mode.sub.val[SW_M_DRIVE] == SW_M_DRIVE_R)) {
            // stop listening
            runner->listening = 0;
            iHide = 0;
            iName = -1;
            iValue = -1;
        } else {
            // blink
            if ((_GetTickMS() - tick) >= 250) {
                tick = _GetTickMS();
                iHide = !iHide;
            }
        }
    } else {
        iHide = 0;
    }

    return iHide;
}
