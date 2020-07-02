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
static uint32_t currentSide;

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_EnterModeIAP(uint32_t side, uint32_t timeout) {
    CAN_DATA *txd = &(CB.tx.data);
    CAN_DATA *rxd = &(CB.rx.data);
    uint8_t p, step = 0, reply = 1;
    uint32_t tick;

    /* Set current side to be updated */
    currentSide = side;

    // set message
    txd->u16[0] = currentSide;
    // send message
    p = CANBUS_Write(CAND_ENTER_IAP, 2);

    // wait response
    if (p) {
        tick = _GetTickMS();
        while ((step < reply) && (_GetTickMS() - tick < timeout)) {
            // read
            if (CANBUS_Read()) {
                if (CANBUS_ReadID() == CAND_ENTER_IAP) {
                    switch (step) {
                        case 0: // ack
                            step += (rxd->u8[0] == FOCAN_ACK);
                            break;
                        case 1: // side address
                            step += (rxd->u32[0] == currentSide);
                            break;
                        case 2: // ack
                            step += (rxd->u8[0] == FOCAN_ACK);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        p = (step == reply);
    }

    return p;
}

uint8_t FOCAN_GetChecksum(uint32_t *checksum, uint32_t timeout) {
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
                        case 2: // ack
                            step += (rxd->u8[0] == FOCAN_ACK);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        p = (step == reply);
    }

    return p;
}

uint8_t FOCAN_NeedBackup(void) {

}
