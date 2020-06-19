/*
 * VCU.h
 *
 *  Created on: May 11, 2020
 *      Author: pudja
 */

#ifndef INC_NODES_VCU_H_
#define INC_NODES_VCU_H_

/* Includes ------------------------------------------------------------------*/
#include "Libs/_utils.h"
#include "Libs/_handlebar.h"

/* Exported enums --------------------------------------------------------------*/
typedef enum {
    VEHICLE_INDEPENDENT = 0,
    VEHICLE_DOWN,
    VEHICLE_POWERED,
    VEHICLE_RUNNING,
} vehicle_state_t;

/* Exported struct --------------------------------------------------------------*/

typedef struct {
    uint32_t fota;
    uint32_t unit_id;
    uint8_t driver_id;
    uint16_t interval;
    uint8_t volume;
    uint8_t speed;
    uint32_t odometer;
    rtc_t rtc;
    uint64_t events;
    struct {
        vehicle_state_t vehicle;
        uint8_t starter;
        uint8_t knob;
        uint8_t independent;
    } state;
    struct {
        uint32_t keyless;
    } tick;
    struct {
        uint16_t report;
        uint16_t response;
    } seq_id;
} vcu_data_t;

typedef struct {
    struct {
        uint8_t (*SwitchModeControl)(sw_t*);
        uint8_t (*Datetime)(timestamp_t*);
        uint8_t (*MixedData)(sw_runner_t*);
        uint8_t (*SubTripData)(uint32_t*);
    } t;
} vcu_can_t;

typedef struct {
    vcu_data_t d;
    vcu_can_t can;
    void (*Init)(void);
    void (*SetEvent)(uint64_t, uint8_t);
    uint8_t (*ReadEvent)(uint64_t);
    void (*CheckMainPower)(void);
} vcu_t;

/* Public functions implementation --------------------------------------------*/
void VCU_Init(void);
void VCU_SetEvent(uint64_t event_id, uint8_t value);
uint8_t VCU_ReadEvent(uint64_t event_id);
void VCU_CheckMainPower(void);

uint8_t VCU_CAN_TX_SwitchModeControl(sw_t *sw);
uint8_t VCU_CAN_TX_Datetime(timestamp_t *timestamp);
uint8_t VCU_CAN_TX_MixedData(sw_runner_t *runner);
uint8_t VCU_CAN_TX_SubTripData(uint32_t *trip);

#endif /* INC_NODES_VCU_H_ */
