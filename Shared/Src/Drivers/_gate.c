/*
 * _gate.c
 *
 *  Created on: Dec 13, 2020
 *      Author: pudja
 */

/* Includes ------------------------------------------------------------------*/
#include "Drivers/_gate.h"
#include "Libs/_utils.h"

/* Public functions implementation -------------------------------------------*/
void GATE_LedWrite(GPIO_PinState state) {
  HAL_GPIO_WritePin(SYS_LED_GPIO_Port, SYS_LED_Pin, state);
}

void GATE_LedToggle(void) {
  HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
}

void GATE_SimcomReset(void) {
  HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(1000);
  HAL_GPIO_WritePin(INT_NET_PWR_GPIO_Port, INT_NET_PWR_Pin, GPIO_PIN_RESET);
}

void GATE_SimcomSoftReset(void) {
  HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(INT_NET_RST_GPIO_Port, INT_NET_RST_Pin, GPIO_PIN_RESET);
}

void GATE_CanbusReset(void) {
  HAL_GPIO_WritePin(INT_CAN_PWR_GPIO_Port, INT_CAN_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(50);
  HAL_GPIO_WritePin(INT_CAN_PWR_GPIO_Port, INT_CAN_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(50);
}

void GATE_SimcomSleep(GPIO_PinState state) {
  HAL_GPIO_WritePin(INT_NET_DTR_GPIO_Port, INT_NET_DTR_Pin, state);
  _DelayMS(50);
}

void GATE_Hmi1Power(GPIO_PinState state) {
  //  HAL_GPIO_WritePin(EXT_HMI1_PWR_GPIO_Port, EXT_HMI1_PWR_Pin, state);
  HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, !state);
}

#if (!BOOTLOADER)
void GATE_Hmi2Reset(void) {
  GATE_Hmi2Stop();
  _DelayMS(100);
  HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, GPIO_PIN_RESET);
}

void GATE_Hmi2Stop(void) {
  HAL_GPIO_WritePin(EXT_HMI2_PWR_GPIO_Port, EXT_HMI2_PWR_Pin, GPIO_PIN_SET);
}

void GATE_GyroReset(void) {
  HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(500);
  HAL_GPIO_WritePin(INT_GYRO_PWR_GPIO_Port, INT_GYRO_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(500);
}

void GATE_GpsReset(void) {
  HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(500);
  HAL_GPIO_WritePin(INT_GPS_PWR_GPIO_Port, INT_GPS_PWR_Pin, GPIO_PIN_SET);
}

void GATE_RemoteReset(void) {
  HAL_GPIO_WritePin(INT_REMOTE_PWR_GPIO_Port, INT_REMOTE_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(1000);
  HAL_GPIO_WritePin(INT_REMOTE_PWR_GPIO_Port, INT_REMOTE_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(1000);
}

void GATE_RemoteCSN(GPIO_PinState state) {
  HAL_GPIO_WritePin(INT_REMOTE_CSN_GPIO_Port, INT_REMOTE_CSN_Pin, state);
}

void GATE_RemoteCE(GPIO_PinState state) {
  HAL_GPIO_WritePin(INT_REMOTE_CE_GPIO_Port, INT_REMOTE_CE_Pin, state);
}

void GATE_FingerReset(void) {
  HAL_GPIO_WritePin(EXT_FINGER_SENSING_PWR_GPIO_Port, EXT_FINGER_SENSING_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(500);
  HAL_GPIO_WritePin(EXT_FINGER_SENSING_PWR_GPIO_Port, EXT_FINGER_SENSING_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(500);
}

void GATE_FingerPower(GPIO_PinState state) {
  HAL_GPIO_WritePin(EXT_FINGER_MCU_PWR_GPIO_Port, EXT_FINGER_MCU_PWR_Pin, !state);
  _DelayMS(500);
}

void GATE_AudioReset(void) {
  HAL_GPIO_WritePin(INT_AUDIO_PWR_GPIO_Port, INT_AUDIO_PWR_Pin, GPIO_PIN_RESET);
  _DelayMS(500);
  HAL_GPIO_WritePin(INT_AUDIO_PWR_GPIO_Port, INT_AUDIO_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(1000);
}

void GATE_AudioCodecStop(void) {
  HAL_GPIO_WritePin(INT_AUDIO_RST_GPIO_Port, INT_AUDIO_RST_Pin, GPIO_PIN_RESET);
}

void GATE_AudioCodecReset(void) {
  GATE_AudioCodecStop();
  _DelayMS(500);
  HAL_GPIO_WritePin(INT_AUDIO_RST_GPIO_Port, INT_AUDIO_RST_Pin, GPIO_PIN_SET);
  _DelayMS(500);
}

void GATE_FanBMS(GPIO_PinState state) {
  HAL_GPIO_WritePin(EXT_BMS_FAN_PWR_GPIO_Port, EXT_BMS_FAN_PWR_Pin, state);
}

void GATE_HornToggle(uint8_t *hazard) {
  HAL_GPIO_WritePin(EXT_HORN_PWR_GPIO_Port, EXT_HORN_PWR_Pin, GPIO_PIN_SET);
  *hazard = GPIO_PIN_SET;
  _DelayMS(200);
  HAL_GPIO_WritePin(EXT_HORN_PWR_GPIO_Port, EXT_HORN_PWR_Pin, GPIO_PIN_RESET);
  *hazard = GPIO_PIN_RESET;
  _DelayMS(100);
}

void GATE_SolenoidToggle(void) {
  HAL_GPIO_WritePin(EXT_SOLENOID_PWR_GPIO_Port, EXT_SOLENOID_PWR_Pin, GPIO_PIN_SET);
  _DelayMS(100);
  HAL_GPIO_WritePin(EXT_SOLENOID_PWR_GPIO_Port, EXT_SOLENOID_PWR_Pin, GPIO_PIN_RESET);
}


GPIO_PinState GATE_ReadPower5v(void) {
  return HAL_GPIO_ReadPin(EXT_REG_5V_IRQ_GPIO_Port, EXT_REG_5V_IRQ_Pin);
}
#endif

GPIO_PinState GATE_ReadKnobState(void) {
  return HAL_GPIO_ReadPin(EXT_KNOB_IRQ_GPIO_Port, EXT_KNOB_IRQ_Pin);
}

