/*
 * VCU.c
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/VCU.h"
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Drivers/_canbus.h"
#include "Libs/_simcom.h"
#include "Libs/_gps.h"

/* External variables ---------------------------------------------------------*/
extern bms_t BMS;
extern hmi1_t HMI1;
extern sim_t SIM;

/* Public variables -----------------------------------------------------------*/
vcu_t VCU = {
        .d = { 0 },
        .can = {
                .t = {
                        VCU_CAN_TX_SwitchModeControl,
                        VCU_CAN_TX_Datetime,
                        VCU_CAN_TX_MixedData,
                        VCU_CAN_TX_SubTripData
                }
        },
        VCU_Init,
        VCU_SetEvent,
        VCU_ReadEvent,
        VCU_CheckMainPower,
};

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void) {
    // reset VCU data
    VCU.d.state.vehicle = VEHICLE_INDEPENDENT;
    VCU.d.state.starter = 0;
    VCU.d.state.knob = 0;
    VCU.d.state.independent = 1;

    VCU.d.interval = RPT_INTERVAL_SIMPLE;
    VCU.d.driver_id = DRIVER_ID_NONE;
    VCU.d.volume = 0;
    VCU.d.speed = 0;
    VCU.d.odometer = 0;
    VCU.d.events = 0;

    VCU.d.tick.keyless = 0;
    //  VCU.d.tick.finger = 0;

    VCU.d.seq_id.report = 0;
    VCU.d.seq_id.response = 0;
}

void VCU_SetEvent(uint64_t event_id, uint8_t value) {
    if (value & 1) {
        BV(VCU.d.events, _BitPosition(event_id));
    } else {
        BC(VCU.d.events, _BitPosition(event_id));
    }
}

uint8_t VCU_ReadEvent(uint64_t event_id) {
    return (VCU.d.events & event_id) == event_id;
}

void VCU_CheckMainPower(void) {
    static TickType_t tick;
    static int8_t lastState = -1;
    uint8_t currentState;

    // read state
    currentState = HAL_GPIO_ReadPin(EXT_REG_5V_IRQ_GPIO_Port, EXT_REG_5V_IRQ_Pin);
    // handle only when changed
    if (lastState != currentState) {
        lastState = currentState;
        // update tick
        tick = _GetTickMS();
    }

    // set things
    VCU.d.state.independent = !currentState;

    // handle when REG_5V is OFF
    if (currentState == 0) {
        VCU.SetEvent(EV_VCU_INDEPENDENT, 1);
        if (_GetTickMS() - tick > (VCU_ACTIVATE_LOST_MODE * 1000)) {
            VCU.d.interval = RPT_INTERVAL_LOST;
            VCU.SetEvent(EV_VCU_UNAUTHORIZE_REMOVAL, 1);
        } else {
            VCU.d.interval = RPT_INTERVAL_INDEPENDENT;
            VCU.SetEvent(EV_VCU_UNAUTHORIZE_REMOVAL, 0);
        }
    } else {
        VCU.d.interval = RPT_INTERVAL_SIMPLE;
        VCU.SetEvent(EV_VCU_INDEPENDENT, 0);
        VCU.SetEvent(EV_VCU_UNAUTHORIZE_REMOVAL, 0);
    }
    // TODO: DELETE_ME
    VCU.d.interval = GPS_INTERVAL_MS / 3000;
}

/* ====================================== CAN TX =================================== */
uint8_t VCU_CAN_TX_SwitchModeControl(sw_t *sw) {
    sein_t sein = HBAR_SeinController(sw);
    CAN_DATA TxData;

    // set message
    TxData.u8[0] = sw->list[SW_K_ABS].state;
    TxData.u8[0] |= _L(HMI1.d.status.mirroring, 1);
    TxData.u8[0] |= _L(sw->list[SW_K_LAMP].state, 2);
    TxData.u8[0] |= _L(HMI1.d.status.warning, 3);
    TxData.u8[0] |= _L(HMI1.d.status.overheat, 4);
    TxData.u8[0] |= _L(HMI1.d.status.finger, 5);
    TxData.u8[0] |= _L(HMI1.d.status.keyless, 6);
    TxData.u8[0] |= _L(HMI1.d.status.daylight, 7);

    // sein value
    TxData.u8[1] = sein.left;
    TxData.u8[1] |= _L(sein.right, 1);
    TxData.u8[1] |= _L(sw->runner.reverse, 2);

    // mode
    TxData.u8[2] = sw->runner.mode.sub.val[SW_M_DRIVE];
    TxData.u8[2] |= _L(sw->runner.mode.sub.val[SW_M_TRIP], 2);
    TxData.u8[2] |= _L(sw->runner.mode.sub.val[SW_M_REPORT], 4);
    TxData.u8[2] |= _L(sw->runner.mode.sel, 5);
    TxData.u8[2] |= _L(HBAR_ModeController(&(sw->runner)), 7);

    // others
    TxData.u8[3] = VCU.d.speed;

    // send message
    return CANBUS_Write(CAND_VCU_SWITCH, &TxData, 4);
}

uint8_t VCU_CAN_TX_Datetime(timestamp_t *timestamp) {
    CAN_DATA TxData;

    // set message
    TxData.u8[0] = timestamp->time.Seconds;
    TxData.u8[1] = timestamp->time.Minutes;
    TxData.u8[2] = timestamp->time.Hours;
    TxData.u8[3] = timestamp->date.Date;
    TxData.u8[4] = timestamp->date.Month;
    TxData.u8[5] = timestamp->date.Year;
    TxData.u8[6] = timestamp->date.WeekDay;
    // HMI2 shutdown request
    TxData.u8[7] = !VCU.d.state.knob;

    // send message
    return CANBUS_Write(CAND_VCU_DATETIME, &TxData, 8);
}

uint8_t VCU_CAN_TX_MixedData(sw_runner_t *runner) {
    CAN_DATA TxData;

    // set message
    TxData.u8[0] = SIM.signal;
    TxData.u8[1] = BMS.d.soc;
    TxData.u8[2] = runner->mode.sub.report[SW_M_REPORT_RANGE];
    TxData.u8[3] = runner->mode.sub.report[SW_M_REPORT_AVERAGE];
    TxData.u32[1] = VCU.d.odometer;

    // send message
    return CANBUS_Write(CAND_VCU_SELECT_SET, &TxData, 8);
}

uint8_t VCU_CAN_TX_SubTripData(uint32_t *trip) {
    CAN_DATA TxData;

    // set message
    TxData.u32[0] = trip[SW_M_TRIP_A];
    TxData.u32[1] = trip[SW_M_TRIP_B];

    // send message
    return CANBUS_Write(CAND_VCU_TRIP_MODE, &TxData, 8);
}
