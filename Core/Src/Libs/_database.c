/*
 * _database.c
 *
 *  Created on: Oct 11, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "_database.h"
#include "_reporter.h"
#include "VCU.h"
#include "BMS.h"
#include "HMI1.h"
#include "HMI2.h"

/* External variables ---------------------------------------------------------*/
extern vcu_t VCU;
extern bms_t BMS;
extern hmi1_t HMI1;
extern hmi2_t HMI2;

/* Public functions implementation --------------------------------------------*/
void DB_Init(void) {
	VCU.Init();
	BMS.Init();
	HMI1.Init();
	HMI2.Init();
}

