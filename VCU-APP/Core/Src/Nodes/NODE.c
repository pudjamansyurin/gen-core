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
		.Init = NODE_Init,
		.Refresh = NODE_Refresh,
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
	uint8_t eBMS = BMS.d.fault > 0;
	uint8_t eMCU = (MCU.d.fault.post | MCU.d.fault.run) > 0;

	if (VCU.d.state >= VEHICLE_READY) {
		if (VCU.d.tick.ready && (_GetTickMS() - VCU.d.tick.ready) > NODE_RISE_TIME) {
			eBMS |= !BMS.d.active;
			eMCU |= !MCU.d.active;
		}
	}

	NODE.d.overheat = BMS.d.overheat || MCU.d.overheat;
	NODE.d.error = VCU.d.error || eBMS || eMCU;

	BMS.RefreshIndex();
	MCU.Refresh();
	HMI1.Refresh();
	HMI2.Refresh();
}
