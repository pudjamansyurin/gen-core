/*
 * NODE.c
 *
 *  Created on: Mar 23, 2021
 *      Author: pujak
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/NODE.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/HMI1.h"
#include "Nodes/HMI2.h"

/* Public variables
 * -----------------------------------------------------------*/
node_t NODE = {
		.r = {
				NODE_RX_Handler
		},
		NODE_Init,
		NODE_Refresh,
};

/* Public functions implementation
 * --------------------------------------------*/
void NODE_Init(void) {
	BMS.Init();
	MCU.Init();
	HMI1.Init();
	HMI2.Init();
}

void NODE_Refresh(void) {
	BMS.RefreshIndex();
	MCU.Refresh();
	HMI1.Refresh();
	HMI2.Refresh();
}

/* ====================================== CAN RX
 * =================================== */
void NODE_RX_Handler(can_rx_t *Rx) {
	if (Rx->header.IDE == CAN_ID_STD) {
		switch (Rx->header.StdId) {
		case CAND_HMI1:
			HMI1.r.State(Rx);
			break;
		case CAND_HMI2:
			HMI2.r.State(Rx);
			break;
		case CAND_MCU_CURRENT_DC:
			MCU.r.CurrentDC(Rx);
			break;
		case CAND_MCU_VOLTAGE_DC:
			MCU.r.VoltageDC(Rx);
			break;
		case CAND_MCU_TORQUE_SPEED:
			MCU.r.TorqueSpeed(Rx);
			break;
		case CAND_MCU_FAULT_CODE:
			MCU.r.FaultCode(Rx);
			break;
		case CAND_MCU_STATE:
			MCU.r.State(Rx);
			break;
		case CAND_MCU_TEMPLATE_R:
			MCU.r.Template(Rx);
			break;
		default:
			break;
		}
	} else {
		switch (BMS_CAND(Rx->header.ExtId)) {
		case BMS_CAND(CAND_BMS_PARAM_1):
				  BMS.r.Param1(Rx);
		break;
		case BMS_CAND(CAND_BMS_PARAM_2):
				  BMS.r.Param2(Rx);
		break;
		default:
			break;
		}
	}
}
