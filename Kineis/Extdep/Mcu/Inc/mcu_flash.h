/* SPDX-License-Identifier: no SPDX license */
/**
 * @file    mcu_flash.h
 * @brief   MCU flash memory system
 * @author  Arribada
 * @date    Creation 2022/01/31
 */

/**
 * @page mcu_flash_page MCU flash
 *
 * Flash memory is used to store ID, ADDR, Secret Key
 *
 * @note 
 *
 */

/**
 * @addtogroup MCU_FLASH
 * @brief MCU FLASH
 * @{
 */

#ifndef MCU_FLASH_H
#define MCU_FLASH_H
#include "stm32wlxx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define FLASH_BASE_ADDR   0x08000000   // Base address of Flash memory
#define FLASH_TOTAL_SIZE  256 * 1024   // 256 KB Flash
                                       //
#define FLASH_USER_DATA_ADDR     0x0803F800
#define FLASH_USER_DATA_PAGE  ((FLASH_USER_DATA_ADDR - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE)

// Variables to manage ID, ADDR and RADIO conf used by KINEIS stack
#define FLASH_ID_OFFSET  0
#define FLASH_ID_SIZE  1 //64 bits
#define FLASH_ID_BYTE_SIZE  4 //64 bits
#define FLASH_ADDR_OFFSET (FLASH_ID_OFFSET + FLASH_ID_BYTE_SIZE)
#define FLASH_ADDR_SIZE 1
#define FLASH_ADDR_BYTE_SIZE  4 //64 bits
#define FLASH_SECKEY_OFFSET (FLASH_ADDR_OFFSET + FLASH_ADDR_BYTE_SIZE)
#define FLASH_SECKEY_SIZE 2
#define FLASH_SECKEY_BYTE_SIZE 16
#define FLASH_RADIOCONF_OFFSET (FLASH_SECKEY_OFFSET + FLASH_SECKEY_SIZE)
#define FLASH_RADIOCONF_SIZE 2
#define FLASH_RADIOCONF_BYTE_SIZE 16


// Application variables offsets and sizes (between RADIOCONF and COUNTER)
#define FLASH_APP_VARS_OFFSET                 (FLASH_RADIOCONF_OFFSET + FLASH_RADIOCONF_BYTE_SIZE)
#define FLASH_U8_MSG_COUNTER_OFFSET           (FLASH_APP_VARS_OFFSET)
#define FLASH_U8_WAIT_MSG_TIMER_S_OFFSET      (FLASH_U8_MSG_COUNTER_OFFSET + 1)
#define FLASH_U8_WAIT_STARTUP_RESTIMER_MIN_OFFSET (FLASH_U8_WAIT_MSG_TIMER_S_OFFSET + 1)
#define FLASH_U8_WAIT_SEQUENCE_NMB_STARTUP_OFFSET (FLASH_U8_WAIT_STARTUP_RESTIMER_MIN_OFFSET + 1)
#define FLASH_U8_BAT_LEVEL_THRESHOLD_OFFSET   (FLASH_U8_WAIT_SEQUENCE_NMB_STARTUP_OFFSET + 1)

// Add padding (reserved bytes) to maintain 64-bit alignment
#define FLASH_APP_VARS_RESERVED_OFFSET        (FLASH_U8_BAT_LEVEL_THRESHOLD_OFFSET + 1)
#define FLASH_APP_VARS_RESERVED_SIZE          3

#define FLASH_APP_VARS_BYTE_SIZE              (5 + FLASH_APP_VARS_RESERVED_SIZE) // Total size = 8 bytes (64-bit aligned)
#define FLASH_APP_VARS_SIZE                   (1) 

// Counter variables aligned after APP_VARS
#define FLASH_COUNTER_OFFSET                  (FLASH_APP_VARS_OFFSET + FLASH_APP_VARS_BYTE_SIZE)
#define FLASH_COUNTER_SIZE                    8 // 64-bit counter
#define FLASH_COUNTER_MAX_WRITES              ((FLASH_PAGE_SIZE - FLASH_COUNTER_OFFSET) / FLASH_COUNTER_SIZE)

// TODO: Move it to another pertinent place? 

enum KNS_status_t MCU_FLASH_read(uint32_t address, void *buffer, size_t size);
enum KNS_status_t MCU_FLASH_write(uint32_t address, const void *data, size_t size);

enum KNS_status_t MCU_FLASH_increment_counter(void);
enum KNS_status_t MCU_FLASH_get_latest_counter(uint64_t *counter);
bool MCU_FLASH_is_counter_full(void);
enum KNS_status_t MCU_FLASH_reset_counter(void);

#endif
