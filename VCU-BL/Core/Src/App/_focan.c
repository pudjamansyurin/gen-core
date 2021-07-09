/*
 * _focan.c
 *
 *  Created on: 29 Jun 2020
 *      Author: Pudja Mansyurin
 */

/* Includes
 * --------------------------------------------*/
#include "App/_focan.h"

#include "Drivers/_can.h"
#include "Drivers/_iwdg.h"
#include "iwdg.h"

/* Private constants
 * --------------------------------------------*/
#define FOCAN_BLOCK ((uint16_t)(256 * 5))
#define FOCAN_RETRY ((uint8_t)5)

/* Private enums
 * --------------------------------------------*/
typedef enum { FOCAN_ERROR = 0x00, FOCAN_ACK = 0x79, FOCAN_NACK = 0x1F } FOCAN;

/* Private functions prototypes
 * --------------------------------------------*/
static uint8_t SendWaitResp(can_tx_t* Tx, uint32_t addr, uint32_t DLC,
                            uint8_t response, uint32_t timeout, uint32_t retry);
static uint8_t WaitResp(uint32_t addr, uint32_t timeout);
static uint8_t SendWaitRespAcked(can_tx_t* Tx, uint32_t addr, uint32_t DLC,
                                 CAN_DATA* RxData, uint32_t timeout,
                                 uint32_t retry);
static uint8_t WaitRespAcked(uint32_t addr, CAN_DATA* RxData, uint32_t timeout);
static uint8_t FlashBlock(const uint8_t* ptr, const uint32_t* tmpBlk);

/* Public functions implementation
 * --------------------------------------------*/
uint8_t FOCAN_SetProgress(IAP_TYPE type, float percent) {
  uint32_t retry = 1;
  can_tx_t Tx = {0};
  uint8_t p;

  Tx.data.u32[0] = type;
  Tx.data.FLOAT[1] = percent;

  do {
    p = CAN_Write(&Tx, CAND_FOCAN_PROGRESS, 8, 0);
  } while (!p && --retry);

  return p;
}

uint8_t FOCAN_GetCRC(uint32_t* crc) {
  CAN_DATA RxData = {0};
  can_tx_t Tx = {0};
  uint8_t p;

  p = SendWaitRespAcked(&Tx, CAND_FOCAN_CRC, 0, &RxData, 20000, 1);

  if (p) *crc = RxData.u32[0];

  return p;
}

uint8_t FOCAN_Hook(uint32_t addr, uint32_t* data) {
  can_tx_t Tx = {0};
  uint8_t p;

  Tx.data.u32[0] = *data;

  p = SendWaitResp(&Tx, addr, 4, FOCAN_ACK, 40000, 1);

  return p;
}

uint8_t FOCAN_Flash(uint8_t* ptr, uint32_t size, uint32_t offset,
                    uint32_t len) {
  uint32_t pendingBlk, tmpBlk;
  can_tx_t Tx = {0};
  float percent;
  uint8_t p;

  // flash each block
  pendingBlk = size;
  do {
    tmpBlk = (pendingBlk > FOCAN_BLOCK ? FOCAN_BLOCK : pendingBlk);

    Tx.data.u32[0] = offset;
    Tx.data.u16[2] = tmpBlk - 1;

    p = SendWaitResp(&Tx, CAND_FOCAN_INIT, 6, FOCAN_ACK, 500, 20);

    // flash
    if (p) p = FlashBlock(ptr, &tmpBlk);

    // wait final response
    if (p) p = (WaitResp(CAND_FOCAN_INIT, 5000) == FOCAN_ACK);

    // update pointer
    if (p) {
      pendingBlk -= tmpBlk;
      offset += tmpBlk;
      ptr += tmpBlk;

      // indicator
      percent = (float)(offset * 100.0f / len);
      FOCAN_SetProgress(ITYPE_HMI, percent);
    }

  } while (p && pendingBlk);

  return p;
}

/* Private functions implementation
 * --------------------------------------------*/
static uint8_t FlashBlock(const uint8_t* ptr, const uint32_t* tmpBlk) {
  uint32_t pendingSubBlk, pos, len;
  can_tx_t Tx = {0};
  uint8_t p;

  // flash each sub-block (as CAN packet)
  pendingSubBlk = *tmpBlk;
  len = sizeof(uint32_t);
  do {
    pos = (*tmpBlk - pendingSubBlk);

    Tx.data.u16[0] = pos;
    Tx.data.u32[1] = *(uint32_t*)ptr;

    p = SendWaitResp(&Tx, CAND_FOCAN_RUN, 8, pos, 5, (100 / 5));

    // update pointer
    if (p) {
      pendingSubBlk -= len;
      ptr += len;
    }
  } while (p && pendingSubBlk);

  return p;
}

static uint8_t SendWaitResp(can_tx_t* Tx, uint32_t addr, uint32_t DLC,
                            uint8_t response, uint32_t timeout,
                            uint32_t retry) {
  uint8_t p;

  do {
    p = CAN_Write(Tx, addr, DLC, 0);

    if (p) p = (WaitResp(addr, timeout) == response);
  } while (!p && --retry);

  // handle error
  if (!p) *(uint32_t*)IAP_RESP_ADDR = IRESP_CAN_FAILED;

  return p;
}

static uint8_t WaitResp(uint32_t addr, uint32_t timeout) {
  FOCAN response = FOCAN_ERROR;
  can_rx_t Rx = {0};
  uint32_t tick;

  // wait response
  tick = tickMs();
  do {
    if (CAN_Read(&Rx)) {
      if (CAN_ReadID(&(Rx.header)) == addr) {
        response = Rx.data.u8[0];
        break;
      }
    }

    IWDG_Refresh();
  } while (tickIn(tick, timeout));

  return response;
}

static uint8_t SendWaitRespAcked(can_tx_t* Tx, uint32_t addr, uint32_t DLC,
                                 CAN_DATA* RxData, uint32_t timeout,
                                 uint32_t retry) {
  uint8_t p;

  do {
    p = CAN_Write(Tx, addr, DLC, 0);

    if (p) p = WaitRespAcked(addr, RxData, timeout);
  } while (!p && --retry);

  // handle error
  if (!p) *(uint32_t*)IAP_RESP_ADDR = IRESP_CAN_FAILED;

  return p;
}

static uint8_t WaitRespAcked(uint32_t addr, CAN_DATA* RxData,
                             uint32_t timeout) {
  uint8_t step = 0, reply = 3;
  can_rx_t Rx = {0};
  uint32_t tick;

  // wait response
  tick = tickMs();
  do {
    // read
    if (CAN_Read(&Rx)) {
      if (CAN_ReadID(&(Rx.header)) == addr) {
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
    IWDG_Refresh();
  } while ((step < reply) && tickIn(tick, timeout));

  return (step == reply);
}
