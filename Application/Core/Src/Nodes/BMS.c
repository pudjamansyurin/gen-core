/*
 * BMS.c
 *
 *  Created on: May 10, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/BMS.h"
#include "Nodes/HMI1.h"
#include "Nodes/VCU.h"
#include "Drivers/_canbus.h"

/* External variables ---------------------------------------------------------*/
extern canbus_t CB;
extern vcu_t VCU;
extern hmi1_t HMI1;

/* Public variables -----------------------------------------------------------*/
bms_t BMS = {
        .d = { 0 },
        .can = {
                .r = {
                        BMS_CAN_RX_Param1,
                        BMS_CAN_RX_Param2
                },
                .t = {
                        BMS_CAN_TX_Setting
                }
        },
        BMS_Init,
        BMS_PowerOverCan,
        BMS_ResetIndex,
        BMS_RefreshIndex,
        BMS_GetIndex,
        BMS_SetEvents,
        BMS_CheckRun,
        BMS_CheckState,
        BMS_MergeData,
};

/* Public functions implementation --------------------------------------------*/
void BMS_Init(void) {
    BMS.d.started = 0;
    BMS.d.soc = 0;
    BMS.d.overheat = 0;
    BMS.d.warning = 0;
    // set each BMS
    for (uint8_t i = 0; i < BMS_COUNT; i++) {
        BMS_ResetIndex(i);
    }
}

void BMS_PowerOverCan(uint8_t on) {
    if (on) {
        if (!BMS.CheckRun(1) && !BMS.CheckState(BMS_STATE_DISCHARGE)) {
            BMS_CAN_TX_Setting(1, BMS_STATE_DISCHARGE);
        } else {
            // completely ON
            BMS.d.started = 1;
        }
    } else {
        if (!BMS.CheckRun(0) || !BMS.CheckState(BMS_STATE_IDLE)) {
            BMS_CAN_TX_Setting(0, BMS_STATE_IDLE);
        } else {
            // completely OFF
            BMS.d.started = 0;
            BMS.d.soc = 0;

            // other parameters
            BMS.d.overheat = 0;
            BMS.d.warning = 1;
        }
    }
}

void BMS_ResetIndex(uint8_t i) {
    BMS.d.pack[i].id = BMS_ID_NONE;
    BMS.d.pack[i].voltage = 0;
    BMS.d.pack[i].current = 0;
    BMS.d.pack[i].soc = 0;
    BMS.d.pack[i].temperature = 0;
    BMS.d.pack[i].state = BMS_STATE_IDLE;
    BMS.d.pack[i].started = 0;
    BMS.d.pack[i].flag = 0;
    BMS.d.pack[i].tick = 0;
}

void BMS_RefreshIndex(void) {
    for (uint8_t i = 0; i < BMS_COUNT; i++) {
        if ((_GetTickMS() - BMS.d.pack[i].tick) > 500) {
            BMS_ResetIndex(i);
        }
    }
    // update data
    BMS_MergeData();
}

uint8_t BMS_GetIndex(uint32_t id) {
    uint8_t i;

    // find index (if already exist)
    for (i = 0; i < BMS_COUNT; i++) {
        if (BMS.d.pack[i].id == id) {
            return i;
        }
    }

    // finx index (if not exist)
    for (i = 0; i < BMS_COUNT; i++) {
        if (BMS.d.pack[i].id == BMS_ID_NONE) {
            return i;
        }
    }

    // force replace first index (if already full)
    return 0;
}

void BMS_SetEvents(uint16_t flag) {
    // Set events
    VCU.SetEvent(EV_BMS_SHORT_CIRCUIT, _R1(flag, 0));
    VCU.SetEvent(EV_BMS_DISCHARGE_OVER_CURRENT, _R1(flag, 1));
    VCU.SetEvent(EV_BMS_CHARGE_OVER_CURRENT, _R1(flag, 2));
    VCU.SetEvent(EV_BMS_DISCHARGE_OVER_TEMPERATURE, _R1(flag, 3));
    VCU.SetEvent(EV_BMS_DISCHARGE_UNDER_TEMPERATURE, _R1(flag, 4));
    VCU.SetEvent(EV_BMS_CHARGE_OVER_TEMPERATURE, _R1(flag, 5));
    VCU.SetEvent(EV_BMS_CHARGE_UNDER_TEMPERATURE, _R1(flag, 6));
    VCU.SetEvent(EV_BMS_UNBALANCE, _R1(flag, 7));
    VCU.SetEvent(EV_BMS_UNDER_VOLTAGE, _R1(flag, 8));
    VCU.SetEvent(EV_BMS_OVER_VOLTAGE, _R1(flag, 9));
    VCU.SetEvent(EV_BMS_OVER_DISCHARGE_CAPACITY, _R1(flag, 10));
    VCU.SetEvent(EV_BMS_SYSTEM_FAILURE, _R1(flag, 11));

    // Parse event for indicator
    BMS.d.overheat = VCU.ReadEvent(EV_BMS_DISCHARGE_OVER_TEMPERATURE) ||
            VCU.ReadEvent(EV_BMS_DISCHARGE_UNDER_TEMPERATURE) ||
            VCU.ReadEvent(EV_BMS_CHARGE_OVER_TEMPERATURE) ||
            VCU.ReadEvent(EV_BMS_CHARGE_UNDER_TEMPERATURE);
    BMS.d.warning = VCU.ReadEvent(EV_BMS_SHORT_CIRCUIT) ||
            VCU.ReadEvent(EV_BMS_DISCHARGE_OVER_CURRENT) ||
            VCU.ReadEvent(EV_BMS_CHARGE_OVER_CURRENT) ||
            VCU.ReadEvent(EV_BMS_UNBALANCE) ||
            VCU.ReadEvent(EV_BMS_UNDER_VOLTAGE) ||
            VCU.ReadEvent(EV_BMS_OVER_VOLTAGE) ||
            VCU.ReadEvent(EV_BMS_OVER_DISCHARGE_CAPACITY) ||
            VCU.ReadEvent(EV_BMS_SYSTEM_FAILURE);

    // Handle overheat
    HAL_GPIO_WritePin(EXT_BMS_FAN_PWR_GPIO_Port, EXT_BMS_FAN_PWR_Pin, BMS.d.overheat);
}

uint8_t BMS_CheckRun(uint8_t state) {
    for (uint8_t i = 0; i < BMS_COUNT; i++) {
        if (BMS.d.pack[i].started != state) {
            return 0;
        }
    }
    return 1;
}

uint8_t BMS_CheckState(BMS_STATE state) {
    for (uint8_t i = 0; i < BMS_COUNT; i++) {
        if (BMS.d.pack[i].state != state) {
            return 0;
        }
    }
    return 1;
}

void BMS_MergeData(void) {
    uint16_t flags = 0;
    uint8_t soc = 0, device = 0;

    // Merge flags (OR-ed)
    for (uint8_t i = 0; i < BMS_COUNT; i++) {
        flags |= BMS.d.pack[i].flag;
    }
    BMS_SetEvents(flags);

    // Average SOC
    for (uint8_t i = 0; i < BMS_COUNT; i++) {
        if (BMS.d.pack[i].started == 1) {
            soc += BMS.d.pack[i].soc;
            device++;
        }
    }
    BMS.d.soc = device ? (soc / device) : soc;
}

/* ====================================== CAN RX =================================== */
void BMS_CAN_RX_Param1(void) {
    CAN_DATA *data = &(CB.rx.data);
    uint8_t index = BMS.GetIndex(CB.rx.header.ExtId & BMS_ID_MASK);

    // read the content
    BMS.d.pack[index].voltage = data->u16[0] * 0.01;
    BMS.d.pack[index].current = (data->u16[1] * 0.01) - 50;
    BMS.d.pack[index].soc = data->u16[2];
    BMS.d.pack[index].temperature = (data->u16[3] * 0.1) - 40;

    // read the id
    BMS.d.pack[index].id = CB.rx.header.ExtId & BMS_ID_MASK;
    BMS.d.pack[index].started = 1;
    BMS.d.pack[index].tick = _GetTickMS();
}

void BMS_CAN_RX_Param2(void) {
    CAN_DATA *data = &(CB.rx.data);
    uint8_t index = BMS.GetIndex(CB.rx.header.ExtId & BMS_ID_MASK);

    // save flag
    BMS.d.pack[index].flag = data->u16[3];

    // save state
    BMS.d.pack[index].state = _L(_R1(data->u8[7], 4), 1) | _R1(data->u8[7], 5);
}

/* ====================================== CAN TX =================================== */
uint8_t BMS_CAN_TX_Setting(uint8_t start, BMS_STATE state) {
    CAN_DATA *data = &(CB.tx.data);

    // set message
    data->u8[0] = start;
    data->u8[0] |= _L(state, 1);

    // set default header
    CANBUS_Header(&(CB.tx.header), CAND_BMS_SETTING, 1);
    // send message
    return CANBUS_Write(&(CB.tx));
}

