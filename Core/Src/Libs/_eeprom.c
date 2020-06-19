/*
 * _flash.c
 *
 *  Created on: Sep 9, 2019
 *      Author: Puja
 */

/* Includes ------------------------------------------------------------------*/
#include "Libs/_eeprom.h"
#include "Drivers/_eeprom24xx.h"

/* Private functions prototype ------------------------------------------------*/
static uint8_t EE_Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size);
static void lock(void);
static void unlock(void);

/* Public functions implementation --------------------------------------------*/
uint8_t EEPROM_Init(void) {
    const uint8_t MAX_RETRY = 5;
    const EEPROM24_DEVICE EEPROMS[2] = {
            EEPROM24_MAIN,
            EEPROM24_BACKUP
    };
    uint8_t retry, ret = 0;

    lock();
    LOG_StrLn("EEPROM:Init");
    // check each eeprom
    for (uint8_t i = 0; i < 2; i++) {
        if (!ret) {
            retry = MAX_RETRY;
            EEPROM24XX_SetDevice(EEPROMS[i]);
            do {
                if (EEPROM24XX_IsConnected()) {
                    LOG_Str("EEPROM:Device = ");
                    LOG_Int(i + 1);
                    LOG_Enter();

                    ret = 1;
                    break;
                }
                _DelayMS(50);
            } while (retry--);
        }
    }

    // all failed
    if (!ret) {
        LOG_StrLn("EEPROM:Error");
    }

    unlock();
    return ret;
}

/* Private functions implementation --------------------------------------------*/
static uint8_t EE_Command(uint16_t vaddr, EEPROM_COMMAND cmd, void *value, void *ptr, uint16_t size) {
    uint8_t ret = 0;

    lock();

    // check if new value is same with old value
    if (cmd == EE_CMD_W) {
        // apply the value
        memcpy(ptr, value, size);
        // save the value
        ret = EEPROM24XX_Save(vaddr, value, size);
    } else {
        // load the value
        ret = EEPROM24XX_Load(vaddr, value, size);
        // apply the value
        if (ret) {
            memcpy(ptr, value, size);
        }
    }

    unlock();
    return ret;
}

static void lock(void) {
//    osMutexAcquire(EepromMutexHandle, osWaitForever);
}

static void unlock(void) {
//    osMutexRelease(EepromMutexHandle);
}
