/*
 * NODE.c
 *
 *  Created on: Mar 23, 2021
 *      Author: pujak
 */

/* Includes ------------------------------------------------------------------*/
#include "Nodes/NODE.h"
#include "Nodes/VCU.h"
#include "Nodes/BMS.h"
#include "Nodes/MCU.h"
#include "Nodes/HMI1.h"
#include "Nodes/HMI2.h"

/* Public variables
 * -----------------------------------------------------------*/
node_t NODE = {
		.d = {0},
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
	NODE.d.overheat = BMS.d.overheat || MCU.d.overheat;
	NODE.d.error = VCU.d.error || BMS.d.error || MCU.d.error;

	BMS.RefreshIndex();
	MCU.Refresh();
	HMI1.Refresh();
	HMI2.Refresh();
}
