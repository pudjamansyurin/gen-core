/*
 * _gate.c
 *
 *  Created on: Dec 13, 2020
 *      Author: pudja
 */

/* Includes
 * --------------------------------------------*/
#include "Drivers/_gate.h"
#include "Libs/_utils.h"

/* Public functions implementation
 * --------------------------------------------*/
void GATE_LedWrite(GPIO_PinState state) {
  HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, state);
}

void GATE_LedToggle(void) {
  HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void GATE_LedBlink(uint32_t time) {
  GATE_LedWrite(1);
  _DelayMS(time);
  GATE_LedWrite(0);
}

void GATE_SimcomShutdown(void) {
  HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(20000);
}

void GATE_SimcomReset(void) {
  GATE_SimcomShutdown();
  HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(50);
}

void GATE_SimcomSoftReset(void) {
  HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_RESET);
}

void GATE_CanbusShutdown(void) {
  HAL_GPIO_WritePin(INT_CAN_PWR_GPIO_Port, INT_CAN_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(50);
}

void GATE_CanbusReset(void) {
  GATE_CanbusShutdown();
  HAL_GPIO_WritePin(INT_CAN_PWR_GPIO_Port, INT_CAN_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(50);
}

void GATE_SimcomSleep(GPIO_PinState state) {
  HAL_GPIO_WritePin(INT_NET_DTR_GPIO_Port, INT_NET_DTR_Pin, state);
  _DelayMS(50);
}

void GATE_Hmi1Power(GPIO_PinState state) {
  HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, state);
}

#if (!BOOTLOADER)
void GATE_System12v(GPIO_PinState state) {
  HAL_GPIO_WritePin(EXT_GPIO_OUT1_GPIO_Port, EXT_GPIO_OUT1_Pin, state);
}

void GATE_McuPower(GPIO_PinState state) {
  HAL_GPIO_WritePin(EXT_BMS_FAN_PWR_GPIO_Port, EXT_BMS_FAN_PWR_Pin, state);
}

void GATE_Hmi2Stop(void) {
  //	HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin,
  //GPIO_PIN_SET);
}

void GATE_Hmi2Reset(void) {
  //	GATE_Hmi2Stop();
  //	_DelayMS(100);
  //	HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin,
  //GPIO_PIN_RESET);
}

void GATE_MemsShutdown(void) {
  HAL_GPIO_WritePin(INT_MEMS_PWR_GPIO_Port, INT_MEMS_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(500);
}

void GATE_MemsReset(void) {
  GATE_MemsShutdown();
  HAL_GPIO_WritePin(INT_MEMS_PWR_GPIO_Port, INT_MEMS_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(500);
}

void GATE_GpsShutdown(void) {
  HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(500);
}

void GATE_GpsReset(void) {
  GATE_GpsShutdown();
  HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(50);
}

void GATE_RemoteShutdown(void) {
  HAL_GPIO_WritePin(INT_REMOTE_PWR_GPIO_Port, INT_REMOTE_PWR_Pin,
                    GPIO_PIN_RESET);
  _DelayMS(1000);
}

void GATE_RemoteReset(void) {
  GATE_RemoteShutdown();
  HAL_GPIO_WritePin(INT_REMOTE_PWR_GPIO_Port, INT_REMOTE_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(1000);
}

void GATE_RemoteCSN(GPIO_PinState state) {
  HAL_GPIO_WritePin(INT_REMOTE_CSN_GPIO_Port, INT_REMOTE_CSN_Pin, state);
}

void GATE_RemoteCE(GPIO_PinState state) {
  HAL_GPIO_WritePin(INT_REMOTE_CE_GPIO_Port, INT_REMOTE_CE_Pin, state);
}

void GATE_FingerShutdown(void) {
  HAL_GPIO_WritePin(EXT_FINGER_SENSING_PWR_GPIO_Port,
                    EXT_FINGER_SENSING_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(500);
}
void GATE_FingerReset(void) {
  GATE_FingerShutdown();
  HAL_GPIO_WritePin(EXT_FINGER_SENSING_PWR_GPIO_Port,
                    EXT_FINGER_SENSING_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(500);
}

void GATE_FingerChipPower(GPIO_PinState state) {
  // HAL_GPIO_WritePin(EXT_FINGER_MCU_PWR_GPIO_Port, EXT_FINGER_MCU_PWR_Pin,
  // !state);
  HAL_GPIO_WritePin(EXT_HMI1_PWR_GPIO_Port, EXT_HMI1_PWR_Pin, state);
  _DelayMS(200);
}

void GATE_AudioShutdown(void) {
  HAL_GPIO_WritePin(INT_AUDIO_PWR_GPIO_Port, INT_AUDIO_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(1000);
}

void GATE_AudioReset(void) {
  GATE_AudioShutdown();
  HAL_GPIO_WritePin(INT_AUDIO_PWR_GPIO_Port, INT_AUDIO_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(1000);
}

void GATE_AudioCodecStop(void) {
  HAL_GPIO_WritePin(INT_AUDIO_RST_GPIO_Port, INT_AUDIO_RST_Pin, GPIO_PIN_RESET);
  _DelayMS(500);
}

void GATE_AudioCodecReset(void) {
  GATE_AudioCodecStop();
  HAL_GPIO_WritePin(INT_AUDIO_RST_GPIO_Port, INT_AUDIO_RST_Pin, GPIO_PIN_SET);
  _DelayMS(500);
}

void GATE_Horn(uint32_t ms) {
  HAL_GPIO_WritePin(EXT_HORN_PWR_GPIO_Port, EXT_HORN_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(ms);
  HAL_GPIO_WritePin(EXT_HORN_PWR_GPIO_Port, EXT_HORN_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(ms);
}

void GATE_Seat(uint32_t ms) {
  HAL_GPIO_WritePin(EXT_SEAT_PWR_GPIO_Port, EXT_SEAT_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(ms);
  HAL_GPIO_WritePin(EXT_SEAT_PWR_GPIO_Port, EXT_SEAT_PWR_Pin, GPIO_PIN_RESET);
}

GPIO_PinState GATE_ReadPower5v(void) {
  // TODO: Change me back
  return GPIO_PIN_SET;
  //  return HAL_GPIO_ReadPin(EXT_REG_5V_IRQ_GPIO_Port, EXT_REG_5V_IRQ_Pin);
}
GPIO_PinState GATE_ReadStarter(void) {
  return HAL_GPIO_ReadPin(EXT_STARTER_IRQ_GPIO_Port, EXT_STARTER_IRQ_Pin);
}
GPIO_PinState GATE_ReadABS(void) {
  return HAL_GPIO_ReadPin(EXT_ABS_IRQ_GPIO_Port, EXT_ABS_IRQ_Pin);
}
GPIO_PinState GATE_ReadSelect(void) {
  return HAL_GPIO_ReadPin(EXT_HBAR_SELECT_GPIO_Port, EXT_HBAR_SELECT_Pin);
}
GPIO_PinState GATE_ReadSet(void) {
  return HAL_GPIO_ReadPin(EXT_HBAR_SET_GPIO_Port, EXT_HBAR_SET_Pin);
}
GPIO_PinState GATE_ReadReverse(void) {
  return HAL_GPIO_ReadPin(EXT_HBAR_REVERSE_GPIO_Port, EXT_HBAR_REVERSE_Pin);
}
GPIO_PinState GATE_ReadLamp(void) {
  return HAL_GPIO_ReadPin(EXT_HBAR_LAMP_GPIO_Port, EXT_HBAR_LAMP_Pin);
}
GPIO_PinState GATE_ReadSeinL(void) {
  return HAL_GPIO_ReadPin(EXT_HBAR_SEIN_L_GPIO_Port, EXT_HBAR_SEIN_L_Pin);
}
GPIO_PinState GATE_ReadSeinR(void) {
  return HAL_GPIO_ReadPin(EXT_HBAR_SEIN_R_GPIO_Port, EXT_HBAR_SEIN_R_Pin);
}
#endif
