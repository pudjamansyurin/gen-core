/*
 * _focan.c
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_focan.h"
#include "Drivers/_canbus.h"

/* Private functions prototypes -----------------------------------------------*/
static uint8_t FOCAN_WriteAndWaitResponse(uint32_t address, CAN_DATA *TxData, uint32_t DLC, uint32_t timeout, uint32_t retry);
static uint8_t FOCAN_WriteAndWaitSqueezed(uint32_t address, CAN_DATA *TxData, uint32_t DLC, CAN_DATA *RxData, uint32_t timeout,
        uint32_t retry);
static uint8_t FOCAN_WaitResponse(uint32_t address, uint32_t timeout);
static uint8_t FOCAN_WaitSqueezed(uint32_t address, CAN_DATA *RxData, uint32_t timeout);
static uint8_t FOCAN_FlashBlock(uint8_t *ptr, uint32_t *tmpBlk);

/* Public functions implementation --------------------------------------------*/
uint8_t FOCAN_GetChecksum(uint32_t *checksum) {
    uint32_t address = CAND_GET_CHECKSUM;
    CAN_DATA RxData;
    uint8_t p;

    // send message
    p = FOCAN_WriteAndWaitSqueezed(address, NULL, 0, &RxData, 5000, (20000 / 5000));

    // process response
    if (p) {
        *checksum = RxData.u32[0];
    }

    return p;
}

uint8_t FOCAN_SetProgress(IAP_TYPE type, uint8_t percent) {
    uint32_t address = CAND_SET_PROGRESS;
    uint32_t retry = 5;
    CAN_DATA TxData;
    uint8_t p;

    // set message
    TxData.u32[0] = type;
    TxData.u8[4] = percent;
    // send message
    do {
        // send message
        p = CANBUS_Write(address, &TxData, 5);
    } while (!p && --retry);

    return p;
}

uint8_t FOCAN_DownloadHook(uint32_t address, uint32_t *data) {
    CAN_DATA TxData;
    uint8_t p;

    // set message
    TxData.u32[0] = *data;
    // send message
    p = FOCAN_WriteAndWaitResponse(address, &TxData, 4, 10000, (30000 / 10000));

    return p;
}

uint8_t FOCAN_DownloadFlash(uint8_t *ptr, uint32_t size, uint32_t offset, uint32_t total_size) {
    CAN_DATA TxData;
    uint32_t pendingBlk, tmpBlk;
    uint8_t p, percent;

    // flash each block
    pendingBlk = size;
    do {
        tmpBlk = (pendingBlk > BLK_SIZE ? BLK_SIZE : pendingBlk);

        // set message
        TxData.u32[0] = offset;
        TxData.u16[2] = tmpBlk - 1;
        // send message
        if (offset) {
            p = FOCAN_WriteAndWaitResponse(CAND_INIT_DOWNLOAD, &TxData, 6, 100, (20000 / 100));
        } else {
            p = FOCAN_WriteAndWaitResponse(CAND_INIT_DOWNLOAD, &TxData, 6, 2500, (20000 / 2500));
        }

        // flash
        if (p) {
            p = FOCAN_FlashBlock(ptr, &tmpBlk);
        }

        // update pointer
        if (p) {
            pendingBlk -= tmpBlk;
            offset += tmpBlk;
            ptr += tmpBlk;

            // indicator
            percent = (offset * 100 / total_size);
            FOCAN_SetProgress(IAP_HMI, percent);
        }

        // wait final response
        if (p) {
            p = (FOCAN_WaitResponse(CAND_INIT_DOWNLOAD, 1000) == FOCAN_ACK);
        }
    } while (p && pendingBlk);

    return p;
}

/* Private functions implementation --------------------------------------------*/
static uint8_t FOCAN_FlashBlock(uint8_t *ptr, uint32_t *tmpBlk) {
    CAN_DATA TxData;
    uint32_t pendingSubBlk, tmpSubBlk;
    uint32_t address;
    uint8_t p;

    // flash each sub-block (as CAN packet)
    pendingSubBlk = *tmpBlk;
    do {
        tmpSubBlk = (pendingSubBlk > 8 ? 8 : pendingSubBlk);

        // set message
        memcpy(&TxData, ptr, tmpSubBlk);
        // send message
        address = _L(CAND_DOWNLOADING, 20) | (*tmpBlk - pendingSubBlk);
        p = FOCAN_WriteAndWaitResponse(address, &TxData, tmpSubBlk, 5, (5000 / 5));

        // update pointer
        if (p) {
            pendingSubBlk -= tmpSubBlk;
            ptr += tmpSubBlk;
        }
    } while (p && pendingSubBlk);

    return p;
}

static uint8_t FOCAN_WriteAndWaitResponse(uint32_t address, CAN_DATA *TxData, uint32_t DLC, uint32_t timeout, uint32_t retry) {
    uint8_t p;

    do {
        // send message
        p = CANBUS_Write(address, TxData, DLC);
        // wait response
        if (p) {
            p = (FOCAN_WaitResponse(address, timeout) == FOCAN_ACK);
        }
    } while (!p && --retry);

    // handle error
    if (!p) {
        *(uint32_t*) IAP_RESPONSE_ADDR = IAP_CANBUS_FAILED;
    }

    return p;
}

static uint8_t FOCAN_WriteAndWaitSqueezed(uint32_t address, CAN_DATA *TxData, uint32_t DLC, CAN_DATA *RxData, uint32_t timeout,
        uint32_t retry) {
    uint8_t p;

    do {
        // send message
        p = CANBUS_Write(address, TxData, DLC);
        // wait response
        if (p) {
            p = FOCAN_WaitSqueezed(address, RxData, timeout);
        }
    } while (!p && --retry);

    // handle error
    if (!p) {
        *(uint32_t*) IAP_RESPONSE_ADDR = IAP_CANBUS_FAILED;
    }

    return p;
}

static uint8_t FOCAN_WaitResponse(uint32_t address, uint32_t timeout) {
    FOCAN response = FOCAN_ERROR;
    can_rx_t Rx;
    uint32_t tick;

    // wait response
    tick = _GetTickMS();
    do {
        // read
        if (CANBUS_Read(&Rx)) {
            if (CANBUS_ReadID(&(Rx.header)) == address) {
                response = Rx.data.u8[0];
                break;
            }
        }
    } while (_GetTickMS() - tick < timeout);

    return response;
}

static uint8_t FOCAN_WaitSqueezed(uint32_t address, CAN_DATA *RxData, uint32_t timeout) {
    uint8_t step = 0, reply = 3;
    can_rx_t Rx;
    uint32_t tick;

    // wait response
    tick = _GetTickMS();
    do {
        // read
        if (CANBUS_Read(&Rx)) {
            if (CANBUS_ReadID(&(Rx.header)) == address) {
                switch (step) {
                    case 0: // ack
                        step += (Rx.data.u8[0] == FOCAN_ACK);
                        break;
                    case 1: // data
                        memcpy(RxData, &(Rx.data), 8);
                        step++;
                        break;
                    case 2: // ack
                        step += (Rx.data.u8[0] == FOCAN_ACK);
                        break;
                    default:
                        break;
                }
            }
        }
    } while ((step < reply) && (_GetTickMS() - tick < timeout));

    return (step == reply);
}
