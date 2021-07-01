/*
 * _iap.c
 *
 *  Created on: Jun 28, 2021
 *      Author: pudja
 */

/* Includes
 * --------------------------------------------*/
#include "App/_iap.h"

#include "Libs/_eeprom.h"
#if (!APP)
#include "Drivers/_flasher.h"
#endif

/* Public variables
 * --------------------------------------------*/
iap_t IAP = {0};

/* Public functions implementation
 * --------------------------------------------*/
uint8_t IAP_TypeStore(IAP_TYPE *src) {
  void *dst = &IAP.type;
  uint8_t ok;

  ok = EE_Cmd(VA_IAP_TYPE, src, dst);

  if (!(IAP.type == ITYPE_VCU || IAP.type == ITYPE_HMI)) IAP.type = ITYPE_VCU;

  return ok;
}

uint8_t IAP_FlagStore(IAP_FLAG *src) {
  void *dst = &IAP.flag;

  return EE_Cmd(VA_IAP_FLAG, src, dst);
}

uint8_t IAP_VersionStore(uint16_t *src) {
  void *dst = &IAP.version;

  return EE_Cmd(VA_IAP_VERSION, src, dst);
}

#if (!APP)
void IAP_SetAppMeta(uint32_t offset, uint32_t data) {
  FLASHER_WriteAppArea((uint8_t *)&data, sizeof(uint32_t), offset);
}

void IAP_SetBootMeta(uint32_t offset, uint32_t data) {
  FLASHER_WriteBootArea((uint8_t *)&data, sizeof(uint32_t), offset);
}

void IAP_SetFlag(void) {
  IAP_FLAG flag = IFLAG_EEPROM;
  IAP_FlagStore(&flag);
}

void IAP_ResetFlag(void) {
  IAP_FLAG flag = IFLAG_RESET;
  IAP_FlagStore(&flag);
}

uint8_t IAP_InProgress(void) { return (IAP.flag == IFLAG_EEPROM); }
#endif
