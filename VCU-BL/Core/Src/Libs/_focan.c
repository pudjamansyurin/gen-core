/*
 * _focan.c
 *
 *  Created on: 29 Jun 2020
 *      Author: geni
 */

/* Includes
 * --------------------------------------------*/
#include "Libs/_focan.h"

#include "Drivers/_canbus.h"
#include "Drivers/_iwdg.h"
#include "iwdg.h"

/* Private functions prototypes
 * --------------------------------------------*/
static uint8_t WriteAndWaitResponse(can_tx_t* Tx, uint32_t addr, uint32_t DLC,
                                    uint8_t response, uint32_t timeout,
                                    uint32_t retry);
static uint8_t WriteAndWaitSqueezed(can_tx_t* Tx, uint32_t addr, uint32_t DLC,
                                    CAN_DATA* RxData, uint32_t timeout,
                                    uint32_t retry);
static uint8_t WaitResponse(uint32_t addr, uint32_t timeout);
static uint8_t WaitSqueezed(uint32_t addr, CAN_DATA* RxData, uint32_t timeout);
static uint8_t FlashBlock(uint8_t* ptr, uint32_t* tmpBlk);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t FOCAN_SetProgress(IAP_TYPE type, float percent) {
  uint32_t retry = 1;
  can_tx_t Tx = {0};
  uint8_t p;

  // set message
  Tx.data.u32[0] = type;
  Tx.data.FLOAT[1] = percent;
  // send message
  do {
    p = CANBUS_Write(&Tx, CAND_FOCAN_PROGRESS, 8, 0);
  } while (!p && --retry);

  return p;
}

uint8_t FOCAN_GetCRC(uint32_t* crc) {
  CAN_DATA RxData = {0};
  can_tx_t Tx = {0};
  uint8_t p;

  // send message
  p = WriteAndWaitSqueezed(&Tx, CAND_FOCAN_CRC, 0, &RxData, 20000, 1);

  // process response
  if (p) *crc = RxData.u32[0];

  return p;
}

uint8_t FOCAN_DownloadHook(uint32_t addr, uint32_t* data) {
  can_tx_t Tx = {0};
  uint8_t p;

  // set message
  Tx.data.u32[0] = *data;
  // send message
  p = WriteAndWaitResponse(&Tx, addr, 4, FOCAN_ACK, 40000, 1);

  return p;
}

uint8_t FOCAN_DownloadFlash(uint8_t* ptr, uint32_t size, uint32_t offset,
                            uint32_t total_size) {
  uint32_t pendingBlk, tmpBlk;
  can_tx_t Tx = {0};
  float percent;
  uint8_t p;

  // flash each block
  pendingBlk = size;
  do {
    tmpBlk = (pendingBlk > BLK_SIZE ? BLK_SIZE : pendingBlk);

    // set message
    Tx.data.u32[0] = offset;
    Tx.data.u16[2] = tmpBlk - 1;
    // send message
    p = WriteAndWaitResponse(&Tx, CAND_FOCAN_INIT, 6, FOCAN_ACK, 500, 20);

    // flash
    if (p) p = FlashBlock(ptr, &tmpBlk);

    // wait final response
    if (p) p = (WaitResponse(CAND_FOCAN_INIT, 5000) == FOCAN_ACK);

    // update pointer
    if (p) {
      pendingBlk -= tmpBlk;
      offset += tmpBlk;
      ptr += tmpBlk;

      // indicator
      percent = (float)(offset * 100.0f / total_size);
      FOCAN_SetProgress(IAP_HMI, percent);
    }

  } while (p && pendingBlk);

  return p;
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t FlashBlock(uint8_t* ptr, uint32_t* tmpBlk) {
  uint32_t pendingSubBlk, pos, len;
  can_tx_t Tx = {0};
  uint8_t p;

  // flash each sub-block (as CAN packet)
  pendingSubBlk = *tmpBlk;
  len = sizeof(uint32_t);
  do {
    pos = (*tmpBlk - pendingSubBlk);
    // set message
    Tx.data.u16[0] = pos;
    Tx.data.u32[1] = *(uint32_t*)ptr;
    // send message
    p = WriteAndWaitResponse(&Tx, CAND_FOCAN_RUN, 8, pos, 5, (100 / 5));

    // update pointer
    if (p) {
      pendingSubBlk -= len;
      ptr += len;
    }
  } while (p && pendingSubBlk);

  return p;
}

static uint8_t WriteAndWaitResponse(can_tx_t* Tx, uint32_t addr, uint32_t DLC,
                                    uint8_t response, uint32_t timeout,
                                    uint32_t retry) {
  uint8_t p;

  do {
    // send message
    p = CANBUS_Write(Tx, addr, DLC, 0);
    // wait response
    if (p) p = (WaitResponse(addr, timeout) == response);
  } while (!p && --retry);

  // handle error
  if (!p) *(uint32_t*)IAP_RESPONSE_ADDR = IAP_CANBUS_FAILED;

  return p;
}

static uint8_t WriteAndWaitSqueezed(can_tx_t* Tx, uint32_t addr, uint32_t DLC,
                                    CAN_DATA* RxData, uint32_t timeout,
                                    uint32_t retry) {
  uint8_t p;

  do {
    // send message
    p = CANBUS_Write(Tx, addr, DLC, 0);
    // wait response
    if (p) p = WaitSqueezed(addr, RxData, timeout);
  } while (!p && --retry);

  // handle error
  if (!p) *(uint32_t*)IAP_RESPONSE_ADDR = IAP_CANBUS_FAILED;

  return p;
}

static uint8_t WaitResponse(uint32_t addr, uint32_t timeout) {
  FOCAN response = FOCAN_ERROR;
  can_rx_t Rx = {0};
  uint32_t tick;

  // wait response
  tick = _GetTickMS();
  do {
    // read
    if (CANBUS_Read(&Rx)) {
      if (CANBUS_ReadID(&(Rx.header)) == addr) {
        response = Rx.data.u8[0];
        break;
      }
    }
    // reset watchdog
    IWDG_Refresh();
  } while (_GetTickMS() - tick < timeout);

  return response;
}

static uint8_t WaitSqueezed(uint32_t addr, CAN_DATA* RxData, uint32_t timeout) {
  uint8_t step = 0, reply = 3;
  can_rx_t Rx = {0};
  uint32_t tick;

  // wait response
  tick = _GetTickMS();
  do {
    // read
    if (CANBUS_Read(&Rx)) {
      if (CANBUS_ReadID(&(Rx.header)) == addr) {
        switch (step) {
          case 0:  // ack
            step += (Rx.data.u8[0] == FOCAN_ACK);
            break;
          case 1:  // data
            memcpy(RxData, &(Rx.data), 8);
            step++;
            break;
          case 2:  // ack
            step += (Rx.data.u8[0] == FOCAN_ACK);
            break;
          default:
            break;
        }
      }
    }
    // reset watchdog
    IWDG_Refresh();
  } while ((step < reply) && (_GetTickMS() - tick < timeout));

  return (step == reply);
}
