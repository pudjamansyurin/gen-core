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
mcu_t MCU = {
		.d = {0},
		.r = {
				MCU_RX_CurrentDC,
				MCU_RX_VoltageDC,
				MCU_RX_TorqueSpeed,
				MCU_RX_FaultCode,
				MCU_RX_State,
				MCU_RX_Template
		},
		.t = {
				MCU_TX_Setting,
				MCU_TX_Template
		},
		.Init = MCU_Init,
		.PowerOverCan = MCU_PowerOverCan,
		.Refresh = MCU_Refresh,
		.GetTemplates = MCU_GetTemplates,
		.SetTemplates = MCU_SetTemplates,
		.GetSpeedMax = MCU_GetSpeedMax,
		.SetSpeedMax = MCU_SetSpeedMax,
		.RpmToSpeed = MCU_RpmToSpeed,
		.SpeedToVolume = MCU_SpeedToVolume,
};

/* Private variables
 * -----------------------------------------------------------*/
static mcu_template_addr_t tplAddr[HBAR_M_DRIVE_MAX];

/* Private functions prototypes
 * -----------------------------------------------*/
static void Reset(void);
static void SetTemplateAddr(void);
static void ResetTemplates(void);
static uint8_t IsOverheat(void);

/* Public functions implementation
 * --------------------------------------------*/
void MCU_Init(void) {
	SetTemplateAddr();
	Reset();
}

void MCU_PowerOverCan(uint8_t on) {
	mcu_template_t t[] = {
			{ 100, 111 },
			{ 200, 222 },
			{ 300, 333 }
	};

	if (on) {
		if (!MCU.d.run) {
			GATE_McuPower(0); _DelayMS(500);
			GATE_McuPower(1); _DelayMS(500);
			MCU.t.Setting(0); _DelayMS(50);

			MCU.SetSpeedMax(100);
			MCU.SetTemplates(t);
		}
		MCU.t.Setting(1);
	} else {
		MCU.t.Setting(0);
	}
}

void MCU_Refresh(void) {
	MCU.d.active = MCU.d.tick && (_GetTickMS() - MCU.d.tick) < MCU_TIMEOUT;
	if (!MCU.d.active) Reset();

	MCU.d.overheat = IsOverheat();
	MCU.d.run = MCU.d.active && MCU.d.inv.enabled;
	MCU.d.error = (MCU.d.fault.post || MCU.d.fault.run) > 0;

	VCU.SetEvent(EVG_MCU_ERROR, MCU.d.error);
}

void MCU_GetTemplates(void) {
	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		MCU.t.Template(tplAddr[m].discur_max, 0, 0);
		MCU.t.Template(tplAddr[m].torque_max, 0, 0);
		//		MCU.t.Template(tplAddr[m].rbs_switch, 0, 0);
	}
}

void MCU_SetTemplates(mcu_template_t templates[3]) {
	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		MCU.t.Template(tplAddr[m].discur_max, 1, templates[m].discur_max);
		MCU.t.Template(tplAddr[m].torque_max, 1, templates[m].torque_max * 10);
		//	MCU.t.Template(tplAddr[m].rbs_switch, 1, templates[m].rbs_switch);
	}
}

void MCU_GetSpeedMax(void) {
	MCU.t.Template(MTP_SPEED_MAX, 0, 0);
}

void MCU_SetSpeedMax(uint8_t max) {
	MCU.t.Template(MTP_SPEED_MAX, 1, max);
}

uint16_t MCU_RpmToSpeed(void) {
	return MCU.d.rpm * MCU_SPEED_MAX / MCU_RPM_MAX;
}

uint16_t MCU_SpeedToVolume(void) {
	return MCU.RpmToSpeed() * 100 / MCU_SPEED_MAX;
}

/* ====================================== CAN RX
 * =================================== */
void MCU_RX_CurrentDC(can_rx_t *Rx) {
	MCU.d.dcbus.current = Rx->data.s16[3] * 0.1;

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_VoltageDC(can_rx_t *Rx) {
	MCU.d.dcbus.voltage = Rx->data.s16[1] * 0.1;

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_TorqueSpeed(can_rx_t *Rx) {
	MCU.d.temperature = Rx->data.s16[0] * 0.1;
	MCU.d.rpm = Rx->data.s16[1];
	MCU.d.torque.commanded = Rx->data.s16[2] * 0.1;
	MCU.d.torque.feedback = Rx->data.s16[3] * 0.1;

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_FaultCode(can_rx_t *Rx) {
	MCU.d.fault.post = Rx->data.u32[0];
	MCU.d.fault.run = Rx->data.u32[1];

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_State(can_rx_t *Rx) {
	MCU.d.drive_mode = Rx->data.u8[4] & 0x03;
	MCU.d.inv.discharge = (Rx->data.u8[4] >> 5) & 0x07;
//	MCU.d.inv.can_mode = Rx->data.u8[5] & 0x01;
	MCU.d.inv.enabled = Rx->data.u8[6] & 0x01;
	MCU.d.inv.lockout = (Rx->data.u8[6] >> 7) & 0x01;
	MCU.d.reverse = Rx->data.u8[7] & 0x01;

	MCU.d.tick = _GetTickMS();
}

void MCU_RX_Template(can_rx_t *Rx) {
	uint16_t param = Rx->data.u16[0];
	//	uint8_t write = Rx->data.u8[2];
	int16_t data = Rx->data.s16[2];

	if (param == MTP_SPEED_MAX) {
		MCU.d.par.speed_max = data;
		return;
	}

	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		if (param == tplAddr[m].discur_max) {
			MCU.d.par.template[m].discur_max = data;
			return;
		}
		else if (param == tplAddr[m].torque_max) {
			MCU.d.par.template[m].torque_max = data * 0.1;
			return;
		}
		//		else if (param == tplAddr[m].rbs_switch) {
		//			MCU.d.par.template[m].rbs_switch = data;
		//			return;
		//		}
	}
}

/* ====================================== CAN TX
 * =================================== */
uint8_t MCU_TX_Setting(uint8_t on) {
	can_tx_t Tx = {0};

	// set message
	Tx.data.u8[4] = HBAR.d.reverse;
	Tx.data.u8[5] = on & 0x01;
	Tx.data.u8[5] |= (0 & 0x01) << 1;
	Tx.data.u8[5] |= (HBAR.d.mode[HBAR_M_DRIVE] & 0x03) << 2;

	// send message
	return CANBUS_Write(&Tx, CAND_MCU_SETTING, 6, 0);
}

uint8_t MCU_TX_Template(uint16_t param, uint8_t write, int16_t data) {
	can_tx_t Tx = {0};

	// set message
	Tx.data.u16[0] = param;
	Tx.data.u8[2] = write;
	Tx.data.s16[2] = data;

	// send message
	return CANBUS_Write(&Tx, CAND_MCU_TEMPLATE_W, 6, 0);
}

/* Private functions implementation
 * --------------------------------------------*/
static void Reset(void) {
	MCU.d.run = 0;
	MCU.d.tick = 0;
	MCU.d.overheat = 0;
	MCU.d.error = 0;

	MCU.d.rpm = 0;
	MCU.d.reverse = 0;
	MCU.d.temperature = 0;
	MCU.d.drive_mode = HBAR_M_DRIVE_STANDARD;
	MCU.d.torque.commanded = 0;
	MCU.d.torque.feedback = 0;
	MCU.d.fault.post = 0;
	MCU.d.fault.run = 0;
	MCU.d.dcbus.current = 0;
	MCU.d.dcbus.voltage = 0;

	MCU.d.inv.enabled = 0;
	MCU.d.inv.lockout = 0;
	MCU.d.inv.discharge = INV_DISCHARGE_DISABLED;

	ResetTemplates();
	MCU.d.par.speed_max = 0;
}

static void SetTemplateAddr(void) {
	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		tplAddr[m].discur_max = (m*3) + MTP_1_DISCUR_MAX;
		tplAddr[m].torque_max = (m*3) + MTP_1_TORQUE_MAX;
		//		tplAddr[m].rbs_switch = (m*3) + MTP_1_RBS_SWITCH;
	}
}

static void ResetTemplates(void) {
	for (uint8_t m=0; m<HBAR_M_DRIVE_MAX; m++) {
		MCU.d.par.template[m].discur_max = 0;
		MCU.d.par.template[m].torque_max = 0;
		//		MCU.d.par.template[m].rbs_switch = 0;
	}
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
