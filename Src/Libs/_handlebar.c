/*
 * _handlebar.c
 *
 *  Created on: Apr 16, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "_handlebar.h"

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
            .pin = EXT_ABS_STATUS_Pin,
            .port = EXT_ABS_STATUS_GPIO_Port,
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

/* Public functions implementation --------------------------------------------*/
void HBAR_ReadStates(void) {
  uint8_t i;

  // Read all EXTI state
  for (i = 0; i < SW_TOTAL_LIST; i++) {
    SW.list[i].state = HAL_GPIO_ReadPin(SW.list[i].port, SW.list[i].pin);
  }
}
