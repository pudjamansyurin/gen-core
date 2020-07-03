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

/* Private functions prototypes -----------------------------------------------*/
static uint8_t FOCAN_WaitResponse(uint32_t address, uint32_t timeout);
static uint8_t FOCAN_WaitSqueezed(uint32_t address, CAN_DATA *data, uint32_t timeout);

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_EnterModeIAP(uint32_t side, uint32_t timeout) {
    uint32_t address = CAND_ENTER_IAP;
    CAN_DATA *txd = &(CB.tx.data);
    CAN_DATA rxd;
    uint8_t p;

    /* Set current side to be updated */
    currentSide = side;

    // set message
    txd->u16[0] = currentSide;
    // send message
    p = CANBUS_Write(address, 2);

    // wait response
    if (p) {
        p = FOCAN_WaitSqueezed(address, &rxd, 100);

        if (p) {
            p = (rxd.u32[0] == currentSide);
        }
    }

    return p;
}

uint8_t FOCAN_GetChecksum(uint32_t *checksum, uint32_t timeout) {
    uint32_t address = CAND_GET_CHECKSUM;
    CAN_DATA rxd;
    uint8_t p;

    // send message
    p = CANBUS_Write(address, 0);

    // wait response
    if (p) {
        p = FOCAN_WaitSqueezed(address, &rxd, 100);

        if (p) {
            *checksum = rxd.u32[0];
        }
    }

    return p;
}

uint8_t FOCAN_DownloadHook(uint32_t address, uint32_t *data, uint32_t timeout) {
    CAN_DATA *txd = &(CB.tx.data);
    uint8_t p;

    // set message
    txd->u32[0] = *data;
    // send message
    p = CANBUS_Write(address, sizeof(uint32_t));

    // wait response
    if (p) {
        p = (FOCAN_WaitResponse(address, timeout) == FOCAN_ACK);
    }

    return p;
}

uint8_t FOCAN_DownloadFlash(uint8_t *ptr, uint32_t size, uint32_t offset) {

}

/* Private functions implementation --------------------------------------------*/
static uint8_t FOCAN_WaitResponse(uint32_t address, uint32_t timeout) {
    CAN_DATA *rxd = &(CB.rx.data);
    FOCAN response = FOCAN_ERROR;
    uint32_t tick;

    // wait response
    tick = _GetTickMS();
    while (_GetTickMS() - tick < timeout) {
        // read
        if (CANBUS_Read()) {
            if (CANBUS_ReadID() == address) {
                response = rxd->u8[0];
                break;
            }
        }
    }

    return response;
}

static uint8_t FOCAN_WaitSqueezed(uint32_t address, CAN_DATA *data, uint32_t timeout) {
    CAN_DATA *rxd = &(CB.rx.data);
    uint8_t step = 0, reply = 3;
    uint32_t tick;

    // wait response
    tick = _GetTickMS();
    while ((step < reply) && (_GetTickMS() - tick < timeout)) {
        // read
        if (CANBUS_Read()) {
            if (CANBUS_ReadID() == address) {
                switch (step) {
                    case 0: // ack
                        step += (rxd->u8[0] == FOCAN_ACK);
                        break;
                    case 1: // data
                        memcpy(data, rxd, 8);
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

    return (step == reply);
}
