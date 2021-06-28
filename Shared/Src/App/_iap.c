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

/* Public variables
 * --------------------------------------------*/
iap_t IAP = {0};

/* Public functions implementation
 * --------------------------------------------*/
uint8_t IAP_TypeStore(IAP_TYPE *src) {
  void *dst = &IAP.type;
  uint8_t ok;

  ok = EE_Cmd(VA_IAP_TYPE, src, dst, sizeof(IAP_TYPE));

  if (!(IAP.type == ITYPE_VCU || IAP.type == ITYPE_HMI)) IAP.type = ITYPE_VCU;

  return ok;
}

uint8_t IAP_FlagStore(IAP_FLAG *src) {
  void *dst = &IAP.flag;

  return EE_Cmd(VA_IAP_FLAG, src, dst, sizeof(uint32_t));
}

uint8_t IAP_VersionStore(uint16_t *src) {
  void *dst = &IAP.version;

  return EE_Cmd(VA_IAP_VERSION, src, dst, sizeof(uint16_t));
}
