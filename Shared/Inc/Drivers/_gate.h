/*
 * _gate.h
 *
 *  Created on: Dec 13, 2020
 *      Author: pudja
 */

#ifndef INC_DRIVERS__GATE_H_
#define INC_DRIVERS__GATE_H_

/* Includes ------------------------------------------------------------------*/
#include "_defines.h"

/* Public functions prototype ------------------------------------------------*/
void GATE_LedWrite(GPIO_PinState state);
void GATE_LedToggle(void);
void GATE_SimcomReset(void);
void GATE_SimcomSoftReset(void);
void GATE_SimcomSleep(GPIO_PinState state);
void GATE_CanbusReset(void);
void GATE_Hmi1Power(GPIO_PinState state);

#if (!BOOTLOADER)
void GATE_Hmi2Reset(void);
void GATE_Hmi2Stop(void);
void GATE_GyroShutdown(void);
void GATE_GyroReset(void);
void GATE_GpsReset(void);
void GATE_RemoteShutdown(void);
void GATE_RemoteReset(void);
void GATE_RemoteCSN(GPIO_PinState state);
void GATE_RemoteCE(GPIO_PinState state);
void GATE_FingerShutdown(void);
void GATE_FingerReset(void);
void GATE_FingerDigitalPower(GPIO_PinState state);
void GATE_AudioShutdown(void);
void GATE_AudioReset(void);
void GATE_AudioCodecStop(void);
void GATE_AudioCodecReset(void);
void GATE_FanBMS(GPIO_PinState state);
void GATE_HornToggle(uint8_t *hazard);
void GATE_SeatToggle(void);

GPIO_PinState GATE_ReadPower5v(void);
#endif

GPIO_PinState GATE_ReadKnobState(void);

#endif /* INC_DRIVERS__GATE_H_ */
