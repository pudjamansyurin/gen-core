/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_reporter.h"
#include "Libs/_handlebar.h"
#include "Libs/_simcom.h"
#include "Libs/_gps.h"
#include "Nodes/VCU.h"
#include "Nodes/BMS.h"
#include "Drivers/_rtc.h"
#include "Drivers/_crc.h"
#include "DMA/_dma_battery.h"

/* External variables ----------------------------------------------------------*/
extern vcu_t VCU;
extern bms_t BMS;
extern gps_t GPS;
extern sw_t SW;
extern sim_t SIM;
extern uint16_t BACKUP_VOLTAGE;

/* Public functions implementation --------------------------------------------*/
void Report_Init(FRAME_TYPE frame, report_t *report) {
    // set default data
    LOG_StrLn("Reporter:ReportInit");
    // =============== REPORT ==============
    // header report
    report->header.prefix[0] = PREFIX_REPORT[1];
    report->header.prefix[1] = PREFIX_REPORT[0];
    report->header.seq_id = VCU.d.seq_id.report;
    // (already set)
    // body required
    // body optional
}

void Response_Init(response_t *response) {
    // set default data
    LOG_StrLn("Reporter:ResponseInit");
    // =============== REPORT ==============
    // header report
    response->header.prefix[0] = PREFIX_REPORT[1];
    response->header.prefix[1] = PREFIX_REPORT[0];
    response->header.seq_id = VCU.d.seq_id.response;
}

void Report_Capture(FRAME_TYPE frame, report_t *report) {
    sw_sub_t *pSub = &(SW.runner.mode.sub);

    // Reconstruct the header
    report->header.seq_id++;
    report->header.unit_id = VCU.d.unit_id;
    report->header.frame_id = frame;
    report->header.size = sizeof(report->header.frame_id) +
            sizeof(report->header.unit_id) +
            sizeof(report->header.seq_id) +
            sizeof(report->data.req);

    // Reconstruct the body
    report->data.req.vcu.driver_id = VCU.d.driver_id;
    report->data.req.vcu.events_group = VCU.d.events;
    report->data.req.vcu.rtc.log = RTC_Read();
    // BMS data
    for (uint8_t i = 0; i < BMS_COUNT ; i++) {
        report->data.req.bms.pack[i].id = BMS.d.pack[i].id;
        report->data.req.bms.pack[i].voltage = BMS.d.pack[i].voltage * 100;
        report->data.req.bms.pack[i].current = (BMS.d.pack[i].current + 50) * 100;
    }

    // Add more (if full frame)
    if (frame == FR_FULL) {
        report->header.size += sizeof(report->data.opt);
        // set parameter
        report->data.opt.vcu.gps.latitude = (int32_t) (GPS.latitude * 10000000);
        report->data.opt.vcu.gps.longitude = (int32_t) (GPS.longitude * 10000000);
        report->data.opt.vcu.gps.altitude = (uint32_t) GPS.altitude;
        report->data.opt.vcu.gps.hdop = (uint8_t) (GPS.dop_h * 10);
        report->data.opt.vcu.gps.vdop = (uint8_t) (GPS.dop_v * 10);
        report->data.opt.vcu.gps.heading = (uint8_t) (GPS.heading / 2);

        report->data.opt.vcu.speed = VCU.d.speed;
        report->data.opt.vcu.odometer = VCU.d.odometer;
        report->data.opt.vcu.odometer_m = VCU.d.odometer_m;
        report->data.opt.vcu.motion.pitch = VCU.d.motion.pitch;
        report->data.opt.vcu.motion.roll = VCU.d.motion.roll;

        report->data.opt.vcu.trip.a = pSub->trip[SW_M_TRIP_A];
        report->data.opt.vcu.trip.b = pSub->trip[SW_M_TRIP_B];
        report->data.opt.vcu.report.range = pSub->report[SW_M_REPORT_RANGE];
        report->data.opt.vcu.report.efficiency = pSub->report[SW_M_REPORT_AVERAGE];

        report->data.opt.vcu.signal = SIM.signal;
        report->data.opt.vcu.backup_voltage = BACKUP_VOLTAGE / 18;

        // BMS data
        for (uint8_t i = 0; i < BMS_COUNT ; i++) {
            report->data.opt.bms.pack[i].soc = BMS.d.pack[i].soc;
            report->data.opt.bms.pack[i].temperature = (BMS.d.pack[i].temperature + 40) * 10;
        }
    }
}

void Response_Capture(response_t *response) {
    //Reconstruct the header
    response->header.seq_id++;
    response->header.unit_id = VCU.d.unit_id;
    response->header.frame_id = FR_RESPONSE;
    response->header.size = sizeof(response->header.frame_id) +
            sizeof(response->header.unit_id) +
            sizeof(response->header.seq_id) +
            sizeof(response->data.code) +
            strlen(response->data.message);
}

void Report_SetCRC(report_t *report) {
    // get current sending date-time
    report->data.req.vcu.rtc.send = RTC_Read();
    // recalculate the CRC
    report->header.crc = CRC_Calculate8(
            (uint8_t*) &(report->header.size),
            report->header.size + sizeof(report->header.size),
            0);
}

void Response_SetCRC(response_t *response) {
    response->header.crc = CRC_Calculate8(
            (uint8_t*) &(response->header.size),
            response->header.size + sizeof(response->header.size),
            0);
}

void Command_Debugger(command_t *cmd) {
    LOG_Str("\nCommand:Payload [");
    LOG_Int(cmd->data.code);
    LOG_Str("-");
    LOG_Int(cmd->data.sub_code);
    LOG_Str("] = ");
    LOG_BufHex((char*) &(cmd->data.value), sizeof(cmd->data.value));
    LOG_Enter();
}

FRAME_TYPE Frame_Decider(void) {
	FRAME_TYPE frame;
	static uint8_t frameDecider = 0;

	if (!VCU.d.state.independent) {
		if (++frameDecider == (RPT_INTERVAL_FULL / RPT_INTERVAL_SIMPLE )) {
			frame = FR_FULL;
			frameDecider = 0;
		} else {
			frame = FR_SIMPLE;
		}
	} else {
		frame = FR_FULL;
		frameDecider = 0;
	}

	return frame;
}
