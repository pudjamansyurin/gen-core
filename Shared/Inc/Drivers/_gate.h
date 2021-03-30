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
void GATE_LedBlink(uint32_t time);
void GATE_SimcomShutdown(void);
void GATE_SimcomReset(void);
void GATE_SimcomSoftReset(void);
void GATE_SimcomSleep(GPIO_PinState state);
void GATE_CanbusShutdown(void);
void GATE_CanbusReset(void);
void GATE_Hmi1Power(GPIO_PinState state);

#if (!BOOTLOADER)
void GATE_System12v(GPIO_PinState state);
void GATE_McuPower(GPIO_PinState state);
void GATE_Hmi2Reset(void);
void GATE_Hmi2Stop(void);
void GATE_MemsShutdown(void);
void GATE_MemsReset(void);
void GATE_GpsShutdown(void);
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
void GATE_HornToggle(void);
void GATE_SeatToggle(void);

GPIO_PinState GATE_ReadPower5v(void);
GPIO_PinState GATE_ReadABS(void);
GPIO_PinState GATE_ReadStarter(void);
GPIO_PinState GATE_ReadSelect(void);
GPIO_PinState GATE_ReadSet(void);
GPIO_PinState GATE_ReadReverse(void);
GPIO_PinState GATE_ReadLamp(void);
GPIO_PinState GATE_ReadSeinL(void);
GPIO_PinState GATE_ReadSeinR(void);
#endif

#endif /* INC_DRIVERS__GATE_H_ */
