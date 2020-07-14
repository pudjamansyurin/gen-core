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
static uint8_t FOCAN_WriteAndWaitResponse(uint32_t StdId, uint32_t DLC, uint32_t timeout);
static uint8_t FOCAN_WriteAndWaitSqueezed(uint32_t StdId, uint32_t DLC, CAN_DATA *data, uint32_t timeout);
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
//    CAN_DATA rxd;
    uint32_t pendingBlk, pendingSubBlk, tmpBlk, tmpSubBlk;
    uint32_t timeout = 200;
    uint8_t p = 1;

    // flash each block
    pendingBlk = size;
    while (p && pendingBlk) {
        tmpBlk = (pendingBlk >= BLK_SIZE ? BLK_SIZE : pendingBlk);

        // set message
        txd->u32[0] = offset;
        txd->u8[4] = tmpBlk - 1;
        // send message
        p = FOCAN_WriteAndWaitResponse(CAND_INIT_DOWNLOAD, 5, timeout);

        if (!p) {
            LOG_StrLn("CAND_INIT_DOWNLOAD ERROR");
        }

        // flash each sub-block (as CAN packet)
        if (p) {
            pendingSubBlk = tmpBlk;
            while (p && pendingSubBlk) {
                tmpSubBlk = (pendingSubBlk >= 8 ? 8 : pendingSubBlk);

                // set message
                memcpy(txd, ptr, tmpSubBlk);
                // send message
                p = FOCAN_WriteAndWaitResponse(CAND_DOWNLOADING, tmpSubBlk, timeout);
                //                p = FOCAN_WriteAndWaitSqueezed(CAND_DOWNLOADING, tmpSubBlk, &rxd, timeout);

                // update pointer
                if (p) {
                    pendingSubBlk -= tmpSubBlk;
                    ptr += tmpSubBlk;
                }
            }

            if (!p) {
                LOG_StrLn("CAND_DOWNLOADING ERROR");
            }

            // update pointer
            if (p) {
                pendingBlk -= tmpBlk;
                offset += tmpBlk;
            }
        }

        // wait final response
        if (p) {
            p = (FOCAN_WaitResponse(CAND_INIT_DOWNLOAD, timeout) == FOCAN_ACK);
        }
    }

    if (!p) {
        LOG_StrLn("ERROR");
    }

    return p;
}

/* Private functions implementation --------------------------------------------*/
static uint8_t FOCAN_WriteAndWaitResponse(uint32_t StdId, uint32_t DLC, uint32_t timeout) {
    uint8_t retry = FOCAN_RETRY, p = 0;

    while (!p && retry--) {
        // send message
        p = CANBUS_Write(StdId, DLC);
        // wait response
        if (p) {
            p = (FOCAN_WaitResponse(StdId, timeout) == FOCAN_ACK);
        }
    }

    return p;
}

static uint8_t FOCAN_WriteAndWaitSqueezed(uint32_t StdId, uint32_t DLC, CAN_DATA *data, uint32_t timeout) {
    uint8_t retry = FOCAN_RETRY, p = 0;

    while (!p && retry--) {
        // send message
        p = CANBUS_Write(StdId, DLC);
        // wait response
        if (p) {
            p = FOCAN_WaitSqueezed(StdId, data, timeout);
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
