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
static uint8_t FOCAN_WriteAndWaitResponse(uint32_t address, uint32_t DLC, uint32_t timeout);
static uint8_t FOCAN_WriteAndWaitSqueezed(uint32_t address, uint32_t DLC, CAN_DATA *data, uint32_t timeout);
static uint8_t FOCAN_WaitResponse(uint32_t address, uint32_t timeout);
static uint8_t FOCAN_WaitSqueezed(uint32_t address, CAN_DATA *data, uint32_t timeout);
static uint8_t FOCAN_DownloadBlock(uint8_t *ptr, uint32_t *tmpBlk, uint32_t timeout);

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
    p = FOCAN_WriteAndWaitSqueezed(address, 2, &rxd, 100);

    // process response
    if (p) {
        p = (rxd.u32[0] == currentSide);
    }

    return p;
}

uint8_t FOCAN_GetChecksum(uint32_t *checksum, uint32_t timeout) {
    uint32_t address = CAND_GET_CHECKSUM;
    CAN_DATA rxd;
    uint8_t p;

    // send message
    p = FOCAN_WriteAndWaitSqueezed(address, 0, &rxd, 100);

    // process response
    if (p) {
        *checksum = rxd.u32[0];
    }

    return p;
}

uint8_t FOCAN_DownloadHook(uint32_t address, uint32_t *data, uint32_t timeout) {
    CAN_DATA *txd = &(CB.tx.data);
    uint8_t p;

    // set message
    txd->u32[0] = *data;
    // send message
    p = FOCAN_WriteAndWaitResponse(address, sizeof(uint32_t), timeout);

    return p;
}

uint8_t FOCAN_DownloadFlash(uint8_t *ptr, uint32_t size, uint32_t offset) {
    CAN_DATA *txd = &(CB.tx.data);
    uint32_t pendingBlk, tmpBlk;
    uint32_t timeout = 100;
    uint8_t p;

    // flash each block
    pendingBlk = size;
    do {
        tmpBlk = (pendingBlk > BLK_SIZE ? BLK_SIZE : pendingBlk);

        // set message
        txd->u32[0] = offset;
        txd->u8[4] = tmpBlk - 1;
        // send message
        p = FOCAN_WriteAndWaitResponse(CAND_INIT_DOWNLOAD, 5, timeout);

        // flash
        if (p) {
            p = FOCAN_DownloadBlock(ptr, &tmpBlk, timeout);
        }

        // update pointer
        if (p) {
            pendingBlk -= tmpBlk;
            offset += tmpBlk;
        }

        // wait final response
        if (p) {
            p = (FOCAN_WaitResponse(CAND_INIT_DOWNLOAD, timeout) == FOCAN_ACK);
        }
    } while (p && pendingBlk);

    return p;
}

/* Private functions implementation --------------------------------------------*/
static uint8_t FOCAN_DownloadBlock(uint8_t *ptr, uint32_t *tmpBlk, uint32_t timeout) {
    CAN_DATA *txd = &(CB.tx.data);
    uint32_t pendingSubBlk, tmpSubBlk;
    uint32_t address;
    uint8_t p;

    // flash each sub-block (as CAN packet)
    pendingSubBlk = *tmpBlk;
    do {
        tmpSubBlk = (pendingSubBlk > 8 ? 8 : pendingSubBlk);

        // set message
        memcpy(txd, ptr, tmpSubBlk);
        // send message
        address = _L(CAND_DOWNLOADING, 20) | (*tmpBlk - pendingSubBlk);
        p = FOCAN_WriteAndWaitResponse(address, tmpSubBlk, timeout);

        // update pointer
        if (p) {
            pendingSubBlk -= tmpSubBlk;
            ptr += tmpSubBlk;
        }
    } while (p && pendingSubBlk);

    return p;
}

static uint8_t FOCAN_WriteAndWaitResponse(uint32_t address, uint32_t DLC, uint32_t timeout) {
    uint8_t retry = FOCAN_RETRY, p = 0;

    while (!p && retry--) {
        // send message
        p = CANBUS_Write(address, DLC);
        // wait response
        if (p) {
            p = (FOCAN_WaitResponse(address, timeout) == FOCAN_ACK);
        }
    }

    return p;
}

static uint8_t FOCAN_WriteAndWaitSqueezed(uint32_t address, uint32_t DLC, CAN_DATA *data, uint32_t timeout) {
    uint8_t retry = FOCAN_RETRY, p = 0;

    while (!p && retry--) {
        // send message
        p = CANBUS_Write(address, DLC);
        // wait response
        if (p) {
            p = FOCAN_WaitSqueezed(address, data, timeout);
        }
    }

    return p;
}

static uint8_t FOCAN_WaitResponse(uint32_t address, uint32_t timeout) {
    CAN_DATA *rxd = &(CB.rx.data);
    FOCAN response = FOCAN_ERROR;
    uint32_t tick;

    // wait response
    tick = _GetTickMS();
    do {
        // read
        if (CANBUS_Read()) {
            if (CANBUS_ReadID() == address) {
                response = rxd->u8[0];
                break;
            }
        }
    } while (_GetTickMS() - tick < timeout);

    return response;
}

static uint8_t FOCAN_WaitSqueezed(uint32_t address, CAN_DATA *data, uint32_t timeout) {
    CAN_DATA *rxd = &(CB.rx.data);
    uint8_t step = 0, reply = 3;
    uint32_t tick;

    // wait response
    tick = _GetTickMS();
    do {
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
    } while ((step < reply) && (_GetTickMS() - tick < timeout));

    return (step == reply);
}
