/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_database.h"

/* Public variables -----------------------------------------------------------*/
db_t DB = {
    .vcu = {
        .volume = 0,
        .bat_voltage = 0,
        .signal_percent = 0,
        .speed = 0,
        .odometer = 0,
        .rtc = {
            .calibration = { 0 },
            .timestamp = {
                .time = { 0 },
                .date = { 0 }
            },
        },
    },
    .hmi1 = {
        .status = {
            .mirroring = 0,
            .warning = 1,
            .temperature = 1,
            .finger = 1,
            .keyless = 1,
            .daylight = 1
        }
    },
    .hmi2 = {
        .shutdown = 0
    },
    .bms = {
        .on = 1,
        .interval = REPORT_INTERVAL_SIMPLE
    }
};

