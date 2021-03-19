/*
 * MCU.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/MCU.h"
#include "Nodes/VCU.h"

/* Public variables
 * -----------------------------------------------------------*/
mcu_t MCU = {.d = {0},
             .can = {.r =
                         {
                             MCU_CAN_RX_CurrentDC,
                             MCU_CAN_RX_VoltageDC,
                             MCU_CAN_RX_TorqueSpeed,
                             MCU_CAN_RX_FaultCode,
                             MCU_CAN_RX_State,
                         },
                     .t = {MCU_CAN_TX_Setting}},
             MCU_Init,
             MCU_PowerOverCan,
             MCU_Refresh,
             MCU_SpeedToVolume,
             MCU_RpmToSpeed};

/* Private functions prototypes
 * -----------------------------------------------*/
static void Reset(void);
static uint8_t IsOverheat(void);

/* Public functions implementation
 * --------------------------------------------*/
void MCU_Init(void) { Reset(); }

void MCU_Refresh(void) {
  if ((_GetTickMS() - MCU.d.tick) > MCU_TIMEOUT)
    Reset();

  MCU.d.error = MCU.d.fault.post || MCU.d.fault.run;
  MCU.d.overheat = IsOverheat();
  //	MCU.d.run = ?;

  VCU.SetEvent(EVG_MCU_ERROR, MCU.d.error);
}

void MCU_PowerOverCan(uint8_t on) {
  if (on) {
    if (!MCU.d.inv.enabled) {
      GATE_McuPower(1);
      _DelayMS(500);
      MCU.can.t.Setting(0);
      _DelayMS(50);
      MCU.can.t.Setting(1);
    }
  } else {
    if (MCU.d.inv.enabled) {
      MCU.can.t.Setting(0);
      _DelayMS(50);
      GATE_McuPower(0);
    }
  }
}

uint16_t MCU_SpeedToVolume(void) { return MCU.d.speed * 100 / MCU_SPEED_MAX; }

uint16_t MCU_RpmToSpeed(void) {
  return MCU.d.rpm * MCU_SPEED_MAX / MCU_RPM_MAX;
}

/* ====================================== CAN RX
 * =================================== */
void MCU_CAN_RX_CurrentDC(can_rx_t *Rx) {
  MCU.d.dcbus.current = Rx->data.u16[3] * 0.1;

  MCU.d.tick = _GetTickMS();
}

void MCU_CAN_RX_VoltageDC(can_rx_t *Rx) {
  MCU.d.dcbus.voltage = Rx->data.u16[1] * 0.1;

  MCU.d.tick = _GetTickMS();
}

void MCU_CAN_RX_TorqueSpeed(can_rx_t *Rx) {
  MCU.d.temperature = Rx->data.u16[0] * 0.1;
  MCU.d.rpm = Rx->data.u16[1];
  MCU.d.torque.commanded = Rx->data.u16[2] * 0.1;
  MCU.d.torque.feedback = Rx->data.u16[3] * 0.1;

  MCU.d.speed = MCU.RpmToSpeed();
  MCU.d.tick = _GetTickMS();
}

void MCU_CAN_RX_FaultCode(can_rx_t *Rx) {
  MCU.d.fault.post = Rx->data.u32[0];
  MCU.d.fault.run = Rx->data.u32[1];

  MCU.d.tick = _GetTickMS();
}

void MCU_CAN_RX_State(can_rx_t *Rx) {
  MCU.d.drive_mode = Rx->data.u8[4] & 0x03;
  MCU.d.inv.discharge = (Rx->data.u8[4] >> 5) & 0x07;
  MCU.d.inv.can_mode = Rx->data.u8[5] & 0x01;
  MCU.d.inv.enabled = Rx->data.u8[6] & 0x01;
  MCU.d.inv.lockout = (Rx->data.u8[6] >> 7) & 0x01;
  MCU.d.reverse = Rx->data.u8[7] & 0x01;

  MCU.d.tick = _GetTickMS();
}

/* ====================================== CAN TX
 * =================================== */
uint8_t MCU_CAN_TX_Setting(uint8_t on) {
  can_tx_t Tx;

  // set message
  Tx.data.u8[4] = HBAR.reverse;
  Tx.data.u8[5] = on & 0x01;
  Tx.data.u8[5] |= (0 & 0x01) << 1;
  Tx.data.u8[5] |= (HBAR.d.mode[HBAR_M_DRIVE] & 0x03) << 2;

  // send message
  return CANBUS_Write(&Tx, CAND_MCU_SETTING, 6, 0);
}

/* Private functions implementation
 * --------------------------------------------*/
static void Reset(void) {
  //	MCU.d.run = 0;
  MCU.d.tick = 0;
  MCU.d.overheat = 0;
  MCU.d.error = 0;

  MCU.d.rpm = 0;
  MCU.d.speed = 0;
  MCU.d.reverse = 0;
  MCU.d.temperature = 0;
  MCU.d.drive_mode = HBAR_M_DRIVE_STANDARD;
  MCU.d.torque.commanded = 0;
  MCU.d.torque.feedback = 0;
  MCU.d.fault.post = 0;
  MCU.d.fault.run = 0;
  MCU.d.dcbus.current = 0;
  MCU.d.dcbus.voltage = 0;

  MCU.d.inv.can_mode = 0;
  MCU.d.inv.enabled = 0;
  MCU.d.inv.lockout = 0;
  MCU.d.inv.discharge = INV_DISCHARGE_DISABLED;
}

static uint8_t IsOverheat(void) {
  MCU_POST_FAULT_BIT overheat_post[] = {
      MPF_MOD_TEMP_LOW, MPF_MOD_TEMP_HIGH,  MPF_PCB_TEMP_LOW,
      MPF_PCB_TEMP_HIGH, MPF_GATE_TEMP_LOW, MPF_GATE_TEMP_HIGH,
  };
  MCU_RUN_FAULT_BIT overheat_run[] = {
      MRF_INV_OVER_TEMP,   MRF_MOTOR_OVER_TEMP, MRF_MODA_OVER_TEMP,
      MRF_MODB_OVER_TEMP,  MRF_MODC_OVER_TEMP,  MRF_PCB_OVER_TEMP,
      MRF_GATE1_OVER_TEMP, MRF_GATE2_OVER_TEMP, MRF_GATE3_OVER_TEMP,
  };

  uint8_t temp = 0;
  for (uint8_t i = 0; i < sizeof(overheat_post); i++)
    temp |= (MCU.d.fault.post & BIT(overheat_post[i]));
  for (uint8_t i = 0; i < sizeof(overheat_run); i++)
    temp |= (MCU.d.fault.run & BIT(overheat_run[i]));

  return temp;
}
