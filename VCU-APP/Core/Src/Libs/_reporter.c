/*
 * _reporter.c
 *
 *  Created on: Oct 2, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_rtc.h"
#include "Drivers/_bat.h"
#include "Drivers/_simcom.h"
#include "Libs/_eeprom.h"
#include "Libs/_hbar.h"
#include "Libs/_audio.h"
#include "Libs/_remote.h"
#include "Libs/_finger.h"
#include "Libs/_reporter.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/VCU.h"
#include "Nodes/HMI1.h"

/* Public variables
 * ----------------------------------------------------------*/
reporter_t RPT = {
		.override = {0},
};

/* Public functions implementation -------------------------------------------*/
void RPT_ReportCapture(FRAME_TYPE frame, report_t *report) {
	report_header_t *header = (report_header_t *)report;
	report_data_t *d = &(report->data);

	memcpy(header->prefix, PREFIX_REPORT, 2);
	header->size = sizeof(header->vin) + sizeof(header->send_time);
	header->vin = VIN_VALUE;

	// Required data
	header->size += sizeof(d->req);

	d->req.frame_id = frame;
	d->req.log_time = RTC_Read();
	d->req.events_group = VCU.d.events;
	d->req.vehicle = (int8_t)VCU.d.state;
	d->req.uptime = (VCU.d.uptime * MANAGER_WAKEUP) / 1000;
	d->req.buffered = VCU.d.buffered;
	d->req.bat = BAT_ScanValue() / 18;

	// Optional data
	if (frame == FR_FULL) {
		header->size += sizeof(d->opt);

		hbar_report_t *hbar = &(d->opt.hbar);
		hbar->reverse = HBAR.state[HBAR_K_REVERSE];
		hbar->mode.drive = HBAR.d.mode[HBAR_M_DRIVE];
		hbar->mode.trip = HBAR.d.mode[HBAR_M_TRIP];
		hbar->mode.report = HBAR.d.mode[HBAR_M_REPORT];
		hbar->trip.a = HBAR.d.trip[HBAR_M_TRIP_A];
		hbar->trip.b = HBAR.d.trip[HBAR_M_TRIP_B];
		hbar->trip.odometer = HBAR.d.trip[HBAR_M_TRIP_ODO];
		hbar->report.range = HBAR.d.report[HBAR_M_REPORT_RANGE];
		hbar->report.efficiency = HBAR.d.report[HBAR_M_REPORT_AVERAGE];

		net_report_t *net = &(d->opt.net);
		net->signal = SIM.d.signal;
		net->state = SIM.d.state;
		net->ipstatus = SIM.d.ipstatus;

		gps_report_t *gps = &(d->opt.gps);
		gps->active = GPS.d.active;
		gps->latitude = (int32_t)(GPS.d.nmea.latitude * 10000000);
		gps->longitude = (int32_t)(GPS.d.nmea.longitude * 10000000);
		gps->altitude = (uint32_t)GPS.d.nmea.altitude;
		gps->hdop = (uint8_t)(GPS.d.nmea.dop_h * 10);
		gps->vdop = (uint8_t)(GPS.d.nmea.dop_v * 10);
		gps->speed = (uint8_t)nmea_to_speed(GPS.d.nmea.speed, nmea_speed_kph);
		gps->heading = (uint8_t)(GPS.d.nmea.coarse / 2);
		gps->sat_in_use = (uint8_t)GPS.d.nmea.sats_in_use;

		mems_report_t *mems = &(d->opt.mems);
		mems->active = MEMS.d.active;
		mems->detector = MEMS.detector.active;
		mems->accel.x = MEMS.d.raw.accelerometer.x * 100;
		mems->accel.y = MEMS.d.raw.accelerometer.y * 100;
		mems->accel.z = MEMS.d.raw.accelerometer.z * 100;

		mems->gyro.x = MEMS.d.raw.gyroscope.x * 10;
		mems->gyro.y = MEMS.d.raw.gyroscope.y * 10;
		mems->gyro.z = MEMS.d.raw.gyroscope.z * 10;

		mems->ypr.pitch = MEMS.detector.tilt.cur.pitch * 10;
		mems->ypr.roll = MEMS.detector.tilt.cur.roll * 10;

		mems->total.accelerometer = MEMS.d.tot.accelerometer * 100;
		mems->total.gyroscope = MEMS.d.tot.gyroscope * 10;
		mems->total.tilt = MEMS.d.tot.tilt * 10;
		mems->total.temperature = MEMS.d.raw.temperature * 10;

		remote_report_t *rmt = &(d->opt.rmt);
		rmt->active = RMT.d.active;
		rmt->nearby = RMT.d.nearby;

		finger_report_t *fgr = &(d->opt.fgr);
		fgr->verified = FGR.d.verified;
		fgr->driver_id = FGR.d.id;

		audio_report_t *audio = &(d->opt.audio);
		audio->active = AUDIO.d.active;
		audio->mute = AUDIO.d.mute;
		audio->volume = AUDIO.d.volume;

		// NODEs
		hmi1_report_t *hmi1 = &(d->opt.hmi1);
		hmi1->active = HMI1.d.active;

		bms_report_t *bms = &(d->opt.bms);
		bms->active = BMS.d.active;
		bms->run = BMS.d.run;
		bms->fault = BMS.d.fault;
		bms->soc= BMS.d.soc;
		for (uint8_t i = 0; i < BMS_COUNT; i++) {
			bms->pack[i].id = BMS.d.pack[i].id;
			bms->pack[i].fault = BMS.d.pack[i].fault;
			bms->pack[i].voltage = BMS.d.pack[i].voltage * 100;
			bms->pack[i].current = BMS.d.pack[i].current * 10;
			bms->pack[i].soc = BMS.d.pack[i].soc;
			bms->pack[i].temperature = BMS.d.pack[i].temperature;
		}

		mcu_report_t *mcu = &(d->opt.mcu);
		mcu->active = MCU.d.active;
		mcu->run = MCU.d.run;
		mcu->rpm = MCU.d.rpm;
		mcu->speed = MCU.RpmToSpeed(MCU.d.rpm);
		mcu->reverse = MCU.d.reverse;
		mcu->temperature = (uint16_t)(MCU.d.temperature * 10);
		mcu->drive_mode = MCU.d.drive_mode;
		mcu->torque.commanded = (uint16_t)(MCU.d.torque.commanded * 10);
		mcu->torque.feedback = (uint16_t)(MCU.d.torque.feedback * 10);
		mcu->fault.post = MCU.d.fault.post;
		mcu->fault.run = MCU.d.fault.run;
		mcu->dcbus.current = (uint16_t)(MCU.d.dcbus.current * 10);
		mcu->dcbus.voltage = (uint16_t)(MCU.d.dcbus.voltage * 10);
		mcu->inv.enabled = MCU.d.inv.enabled;
		mcu->inv.lockout = MCU.d.inv.lockout;
		mcu->inv.discharge = MCU.d.inv.discharge;
		mcu->par.rpm_max = MCU.d.par.rpm_max;
		mcu->par.speed_max = MCU.RpmToSpeed(MCU.d.par.rpm_max);
		for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
			mcu->par.tpl[m].discur_max = MCU.d.par.tpl[m].discur_max;
			mcu->par.tpl[m].torque_max = MCU.d.par.tpl[m].torque_max * 10;
		}

		tasks_report_t *tasks = &(d->opt.task);
		memcpy(&(tasks->stack), &(TASKS.stack), sizeof(tasks_stack_t));
		memcpy(&(tasks->wakeup), &(TASKS.wakeup), sizeof(tasks_wakeup_t));
	}
}

void RPT_ResponseCapture(response_t *response) {
	report_header_t *header = (report_header_t *)response;

	memcpy(header->prefix, PREFIX_RESPONSE, 2);
	header->size = sizeof(header->vin) + sizeof(header->send_time);
	header->vin = VIN_VALUE;

	header->size +=
			sizeof(response->header.code) + sizeof(response->header.sub_code) +
			sizeof(response->data.res_code) +
			strnlen(response->data.message, sizeof(response->data.message));
}

FRAME_TYPE RPT_FrameDecider(void) {
	static uint8_t frameDecider = 0;
	uint8_t override = RPT.override.frame;
	FRAME_TYPE frame = FR_FULL;

	if (!GATE_ReadPower5v()) {
		frame = FR_FULL;
		frameDecider = 0;
	} else {
		if (++frameDecider < (RPT_FRAME_FULL / RPT_INTERVAL_NORMAL))
			frame = FR_SIMPLE;
		else {
			frame = FR_FULL;
			frameDecider = 0;
		}
	}

	if (override)
		frame = override;

	return frame;
}

uint16_t RPT_IntervalDecider(void) {
	vehicle_state_t state = VCU.d.state;
	uint8_t override = RPT.override.interval;
	uint16_t interval = RPT_INTERVAL_NORMAL;

	if (state >= VEHICLE_NORMAL)
		interval = RPT_INTERVAL_NORMAL;
	else if (state >= VEHICLE_BACKUP)
		interval = RPT_INTERVAL_BACKUP;
	else if (state >= VEHICLE_LOST)
		interval = RPT_INTERVAL_LOST;

	if (override)
		interval = override;

	return interval;
}

uint8_t RPT_PayloadPending(payload_t *payload) {
	if (!payload->pending)
		if (osMessageQueueGet(*(payload->pQueue), payload->pPayload, NULL, 0) == osOK)
			payload->pending = 1;

	return payload->pending;
}

uint8_t RPT_WrapPayload(payload_t *payload) {
	report_header_t *header = (report_header_t *)(payload->pPayload);

	header->send_time = RTC_Read();
	payload->size = sizeof(header->prefix) + sizeof(header->size) + header->size;

	return payload->size;
}
