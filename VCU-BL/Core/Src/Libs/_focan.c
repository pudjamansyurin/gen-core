/*
 * _focan.c
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_focan.h"
#include "Drivers/_canbus.h"

/* External variables ---------------------------------------------------------*/
extern canbus_t CB;

/* Private variables ----------------------------------------------------------*/
static uint32_t HMI_ADDR;

/* Private functions prototypes -----------------------------------------------*/
static void FOCAN_SetTarget(uint32_t address);
static uint8_t FOCAN_xResponse(uint32_t address, FOCAN response, uint32_t timeout);
static uint8_t FOCAN_EnterModeIAP(uint32_t timeout);
static uint8_t FOCAN_GetChecksum(uint32_t *checksum, uint32_t timeout);

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_Upgrade(void) {
    uint32_t checksum = 0;
    uint8_t p;

    /* Set HMI target */
    FOCAN_SetTarget(CAND_HMI1_LEFT);

    /* Tell HMI to enter IAP mode */
    p = FOCAN_EnterModeIAP(1000);

    /* Get HMI checksum via CAN */
    if (p > 0) {
        p = FOCAN_GetChecksum(&checksum, 500);
    }

    return p;
}

/* Private functions implementation ------------------------------------------*/
static void FOCAN_SetTarget(uint32_t address) {
    HMI_ADDR = address;
}

static uint8_t FOCAN_xResponse(uint32_t address, FOCAN response, uint32_t timeout) {
    CAN_DATA *rxd = &(CB.rx.data);
    uint32_t tick;
    uint8_t p = 0;

    // wait response
    tick = _GetTickMS();
    while (!p && _GetTickMS() - tick < timeout) {
        // read
        if (CANBUS_Read()) {
            if (CANBUS_ReadID() == address) {
                if (rxd->u8[0] == response) {
                    p = 1;
                }
            }
        }
    }

    return p;
}

static uint8_t FOCAN_EnterModeIAP(uint32_t timeout) {
    return FOCAN_xResponse(CAND_ENTER_IAP, FOCAN_ACK, timeout);
}

static uint8_t FOCAN_GetChecksum(uint32_t *checksum, uint32_t timeout) {
    CAN_DATA *txd = &(CB.tx.data);
    CAN_DATA *rxd = &(CB.rx.data);
    uint8_t p, step = 0, reply = 3;
    uint32_t address = CAND_GET_CHECKSUM;
    uint32_t tick;

    // set message (dummy)
    txd->u8[0] = 0x00;
    // send message
    p = CANBUS_Write(address, 1);

    // wait response
    if (p) {
        tick = _GetTickMS();
        while ((step < reply) && (_GetTickMS() - tick < timeout)) {
            // read
            if (CANBUS_Read()) {
                if (CANBUS_ReadID() == address) {
                    switch (step) {
                        case 0: // ack
                            step += (rxd->u8[0] == FOCAN_ACK);
                            break;
                        case 1: // checksum
                            *checksum = rxd->u32[0];
                            step++;
                            break;
                        case 3: // ack
                            step += (rxd->u8[0] == FOCAN_ACK);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        /* return value */
        p = (step == reply);
    }

    return p;
}

