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

#define FLASH_BASE_ADDR   0x08000000   // Base address of Flash memory
#define FLASH_TOTAL_SIZE  256 * 1024   // 256 KB Flash
                                       //
#define FLASH_USER_DATA_ADDR     0x0803F800
#define FLASH_USER_DATA_PAGE  ((FLASH_USER_DATA_ADDR - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE)

#define FLASH_ID_OFFSET  0
#define FLASH_ID_SIZE  1 //64 bits size
#define FLASH_ID_BYTE_SIZE  4
#define FLASH_ADDR_OFFSET (FLASH_ID_OFFSET + FLASH_ID_BYTE_SIZE)
#define FLASH_ADDR_SIZE 1
#define FLASH_ADDR_BYTE_SIZE  4
#define FLASH_SECKEY_OFFSET (FLASH_ADDR_OFFSET + FLASH_ADDR_BYTE_SIZE)
#define FLASH_SECKEY_SIZE 2
#define FLASH_SECKEY_BYTE_SIZE 16
#define FLASH_RADIOCONF_OFFSET (FLASH_SECKEY_OFFSET + FLASH_SECKEY_BYTE_SIZE)
#define FLASH_RADIOCONF_SIZE 2
#define FLASH_RADIOCONF_BYTE_SIZE 16

enum KNS_status_t MCU_FLASH_read(uint32_t address, void *buffer, size_t size);
enum KNS_status_t MCU_FLASH_write(uint32_t address, const void *data, size_t size);
#endif
