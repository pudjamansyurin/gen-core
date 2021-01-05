/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Drivers/_crc.h"
#include "Libs/_rtos_utils.h"
#include "Libs/_reporter.h"
#include "Libs/_handlebar.h"
#include "Libs/_simcom.h"
#include "Libs/_eeprom.h"
#include "Nodes/BMS.h"

/* Private functions declarations -------------------------------------------*/
static void Report_SetCRC(report_t *report);
static void Response_SetCRC(response_t *response);

/* Public functions implementation -------------------------------------------*/
void RPT_ReportInit(FRAME_TYPE frame, report_t *report, uint16_t *seq_id_report) {
	// set default data
	LOG_StrLn("Reporter:ReportInit");
	// =============== REPORT ==============
	// header report
	report->header.prefix[0] = PREFIX_REPORT[1];
	report->header.prefix[1] = PREFIX_REPORT[0];
	report->header.seq_id = *seq_id_report;
	// (already set)
	// body required
	// body optional
}

void RPT_ResponseInit(response_t *response, uint16_t *seq_id_response) {
	// set default data
	LOG_StrLn("Reporter:ResponseInit");
	// =============== REPORT ==============
	// header report
	response->header.prefix[0] = PREFIX_REPORT[1];
	response->header.prefix[1] = PREFIX_REPORT[0];
	response->header.seq_id = *seq_id_response;
}

void RPT_ReportCapture(FRAME_TYPE frame, report_t *report, vcu_data_t *vcu, bms_data_t *bms, hbar_data_t *hbar) {
	// Reconstruct the header
	report->header.seq_id++;
	report->header.unit_id = vcu->unit_id;
	report->header.frame_id = frame;
	report->header.size = sizeof(report->header.frame_id) +
			sizeof(report->header.unit_id) +
			sizeof(report->header.seq_id) +
			sizeof(report->data.req);

	// Reconstruct the body
	report->data.req.vcu.driver_id = vcu->driver_id;
	report->data.req.vcu.events_group = vcu->events;
	report->data.req.vcu.rtc.log = RTC_Read();
	// BMS data
	for (uint8_t i = 0; i < BMS_COUNT ; i++) {
		report->data.req.bms.pack[i].id = bms->pack[i].id;
		report->data.req.bms.pack[i].voltage = bms->pack[i].voltage * 100;
		report->data.req.bms.pack[i].current = (bms->pack[i].current + 50) * 100;
	}

	// Add more (if full frame)
	if (frame == FR_FULL) {
		report->header.size += sizeof(report->data.opt);
		// set parameter
    report->data.opt.vcu.vehicle = (int8_t) vcu->state.vehicle;

		report->data.opt.vcu.gps.latitude = (int32_t) (vcu->gps.latitude * 10000000);
		report->data.opt.vcu.gps.longitude = (int32_t) (vcu->gps.longitude * 10000000);
		report->data.opt.vcu.gps.altitude = (uint32_t) vcu->gps.altitude;
		report->data.opt.vcu.gps.hdop = (uint8_t) (vcu->gps.dop_h * 10);
		report->data.opt.vcu.gps.vdop = (uint8_t) (vcu->gps.dop_v * 10);
		report->data.opt.vcu.gps.heading = (uint8_t) (vcu->gps.heading / 2);
    report->data.opt.vcu.gps.sat_in_use = (uint8_t) vcu->gps.sat_in_use;

		report->data.opt.vcu.speed = vcu->speed;
		report->data.opt.vcu.odometer = vcu->odometer;

		report->data.opt.vcu.trip.a = hbar->trip[HBAR_M_TRIP_A];
		report->data.opt.vcu.trip.b = hbar->trip[HBAR_M_TRIP_B];
		report->data.opt.vcu.report.range = hbar->report[HBAR_M_REPORT_RANGE];
		report->data.opt.vcu.report.efficiency = hbar->report[HBAR_M_REPORT_AVERAGE];

		report->data.opt.vcu.signal = SIM.signal;
    report->data.opt.vcu.bat = vcu->bat / 18;

		// BMS data
		for (uint8_t i = 0; i < BMS_COUNT ; i++) {
      report->data.opt.bms.pack[i].soc = bms->pack[i].soc * 100;
			report->data.opt.bms.pack[i].temperature = (bms->pack[i].temperature + 40) * 10;
		}

    // Others data
    memcpy(&(report->data.opt.vcu.motion), &(vcu->motion), sizeof(motion_t));
    memcpy(&(report->data.opt.vcu.task), &(vcu->task), sizeof(rtos_task_t));
	}
}

void RPT_ResponseCapture(response_t *response, uint32_t *unit_id) {
	//Reconstruct the header
	response->header.seq_id++;
	response->header.unit_id = *unit_id;
	response->header.frame_id = FR_RESPONSE;
	response->header.size = sizeof(response->header.frame_id) +
			sizeof(response->header.unit_id) +
			sizeof(response->header.seq_id) +
			sizeof(response->data.code) +
			strlen(response->data.message);
}

void RPT_CommandDebugger(command_t *cmd) {
	LOG_Str("\nCommand:Payload [");
	LOG_Int(cmd->data.code);
	LOG_Str("-");
	LOG_Int(cmd->data.sub_code);
	LOG_Str("] = ");
	LOG_BufHex((char*) &(cmd->data.value), sizeof(cmd->data.value));
	LOG_Enter();
}

FRAME_TYPE RPT_FrameDecider(uint8_t backup) {
	static uint8_t frameDecider = 0;
  FRAME_TYPE frame;

  if (backup) {
    frame = FR_FULL;
    frameDecider = 0;
  } else {
    if (++frameDecider < (RPT_INTERVAL_NORMAL * 4))
      frame = FR_SIMPLE;
    else {
      frame = FR_FULL;
      frameDecider = 0;
    }
  }

	return frame;
}

uint8_t RPT_PacketPending(payload_t *payload) {
	uint32_t notif;
	osStatus_t status;

	// Handle Full Buffer
  if (payload->type == PAYLOAD_REPORT)
    if (RTOS_ThreadFlagsWait(&notif, EVT_IOT_DISCARD, osFlagsWaitAny, 0))
			payload->pending = 0;

	// Check logs
	if (!payload->pending) {
		status = osMessageQueueGet(*(payload->pQueue), payload->pPayload, NULL, 0);
		// check is mail ready
		if (status == osOK) {
			payload->retry = SIMCOM_MAX_UPLOAD_RETRY;
			payload->pending = 1;
		}
	}

	return payload->pending;
}

uint8_t RPT_SendPayload(payload_t *payload) {
	SIMCOM_RESULT p;
	report_t reporter;
	header_t *pHeader;
	const uint8_t size = sizeof(reporter.header.prefix)
			+ sizeof(reporter.header.crc)
			+ sizeof(reporter.header.size);

	// get the header
	pHeader = (header_t*) (payload->pPayload);

	// Re-calculate CRC
  if (payload->type == PAYLOAD_REPORT)
		Report_SetCRC((report_t*) payload->pPayload);
  else
		Response_SetCRC((response_t*) payload->pPayload);

	// Send to server
	p = Simcom_Upload(payload->pPayload, size + pHeader->size);

	// Handle looping NACK
  if (p == SIM_RESULT_NACK)
		// Probably  CRC not valid, cancel but force as success
    if (!--payload->retry)
			p = SIM_RESULT_OK;

	// Release back
	if (p == SIM_RESULT_OK) {
		EEPROM_SequentialID(EE_CMD_W, pHeader->seq_id, payload->type);
		payload->pending = 0;
	}

	return (p == SIM_RESULT_OK);
}

/* Private functions implementation -------------------------------------------*/
static void Report_SetCRC(report_t *report) {
  // get current sending date-time
  report->data.req.vcu.rtc.send = RTC_Read();
  // recalculate the CRC
  report->header.crc = CRC_Calculate8(
      (uint8_t*) &(report->header.size),
      report->header.size + sizeof(report->header.size),
      0);
}

static void Response_SetCRC(response_t *response) {
  response->header.crc = CRC_Calculate8(
      (uint8_t*) &(response->header.size),
      response->header.size + sizeof(response->header.size),
      0);
}
