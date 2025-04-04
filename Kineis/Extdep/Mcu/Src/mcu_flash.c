/* SPDX-License-Identifier: no SPDX license */
/**
 * @file    mcu_flash.c
 * @brief   MCU flash memory system
 * @author  Arribada
 * @date    Creation 2022/01/31
 */

/**
 * @page mcu_flash_page MCU flash
 *
 * Flash memory is used to store ID, ADDR, Secret Key
 *  TODO :RIGH now memory size is not optimized.
 *
 * @note
 *
 */

/**
 * @addtogroup MCU_FLASH
 * @brief MCU FLASH
 * @{
 */


#include "mcu_flash.h"
#include "kns_types.h"
#include <string.h>
#include "stm32wlxx_hal.h"

/**
 * @brief Reads data from flash memory byte-by-byte.
 *
 * @param dest Pointer to the buffer where the data will be stored.
 * @param ofset Flash memory address to read from.
 * @param length Number of bytes to read.
 * @return KNS_status_t Status of the operation.
 */
enum KNS_status_t MCU_FLASH_read(uint32_t address, void *buffer, size_t size)
{
    if (!buffer || size == 0) return KNS_STATUS_ERROR; // Safety check
    memcpy(buffer, (void*)address, size);
    return KNS_STATUS_OK;
}

/**
 * @brief Writes data to flash memory byte-by-byte.
 *
 * @param src Pointer to the buffer containing the data.
 * @param ofset Flash memory address to write to.
 * @param length Number of bytes to write.
 * @return KNS_status_t Status of the operation.
 */
enum KNS_status_t MCU_FLASH_write(uint32_t address, const void *data, size_t size)
{
    if (!data || size == 0) return KNS_STATUS_ERROR; // Safety check

    uint64_t page_backup[FLASH_PAGE_SIZE / sizeof(uint64_t)];
    memcpy(page_backup, (uint64_t*)FLASH_USER_DATA_ADDR, FLASH_PAGE_SIZE); // Backup full page

    // Copy new data into the correct position in the backup
    memcpy(((uint8_t*)page_backup) + (address - FLASH_USER_DATA_ADDR), data, size);

    HAL_FLASH_Unlock();

    // Erase the Flash page
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page = FLASH_USER_DATA_PAGE;
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return KNS_STATUS_ERROR;
    }

    // Re-write full page to Flash
    for (uint32_t i = 0; i < FLASH_PAGE_SIZE / sizeof(uint64_t); i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, FLASH_USER_DATA_ADDR + (i * sizeof(uint64_t)), page_backup[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return KNS_STATUS_ERROR;
        }
    }

    HAL_FLASH_Lock();
    return KNS_STATUS_OK;
}

enum KNS_status_t MCU_FLASH_increment_counter(void)
{
    uint64_t counter = 0;
    uint32_t write_address = FLASH_USER_DATA_ADDR + FLASH_COUNTER_OFFSET;

    if (MCU_FLASH_is_counter_full()) {
        return KNS_STATUS_ERROR; // flash is full
    }

    for (uint32_t i = 0; i < FLASH_COUNTER_MAX_WRITES; i++) {
        uint64_t temp;
        MCU_FLASH_read(write_address + (i * FLASH_COUNTER_SIZE), &temp, sizeof(temp));
        if (temp == 0xFFFFFFFFFFFFFFFF) { // unused flash
            if (i == 0)
                counter = 1;
            else {
                MCU_FLASH_read(write_address + ((i - 1) * FLASH_COUNTER_SIZE), &counter, sizeof(counter));
                counter++;
            }

            return MCU_FLASH_write(write_address + (i * FLASH_COUNTER_SIZE), &counter, sizeof(counter));
        }
    }
    return KNS_STATUS_ERROR;
}

enum KNS_status_t MCU_FLASH_get_latest_counter(uint64_t *counter)
{
    if (!counter) return KNS_STATUS_ERROR;

    uint32_t write_address = FLASH_USER_DATA_ADDR + FLASH_COUNTER_OFFSET;
    uint64_t temp = 0;

    for (uint32_t i = 0; i < FLASH_COUNTER_MAX_WRITES; i++) {
        MCU_FLASH_read(write_address + (i * FLASH_COUNTER_SIZE), &temp, sizeof(temp));
        if (temp == 0xFFFFFFFFFFFFFFFF) {
            if (i == 0) *counter = 0;
            else MCU_FLASH_read(write_address + ((i - 1) * FLASH_COUNTER_SIZE), counter, sizeof(*counter));
            return KNS_STATUS_OK;
        }
    }

    MCU_FLASH_read(write_address + ((FLASH_COUNTER_MAX_WRITES - 1) * FLASH_COUNTER_SIZE), counter, sizeof(*counter));
    return KNS_STATUS_OK;
}

bool MCU_FLASH_is_counter_full(void)
{
    uint64_t temp;
    uint32_t last_address = FLASH_USER_DATA_ADDR + FLASH_COUNTER_OFFSET +
                            ((FLASH_COUNTER_MAX_WRITES - 1) * FLASH_COUNTER_SIZE);
    MCU_FLASH_read(last_address, &temp, sizeof(temp));
    return (temp != 0xFFFFFFFFFFFFFFFF);
}

enum KNS_status_t MCU_FLASH_reset_counter(void)
{
    uint64_t page_backup[FLASH_PAGE_SIZE / sizeof(uint64_t)];
    memcpy(page_backup, (uint64_t*)FLASH_USER_DATA_ADDR, FLASH_PAGE_SIZE);

    memset(((uint8_t*)page_backup) + FLASH_COUNTER_OFFSET, 0xFF,
           (FLASH_COUNTER_MAX_WRITES * FLASH_COUNTER_SIZE));

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page = FLASH_USER_DATA_PAGE;
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return KNS_STATUS_ERROR;
    }

    for (uint32_t i = 0; i < FLASH_PAGE_SIZE / sizeof(uint64_t); i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
                              FLASH_USER_DATA_ADDR + (i * sizeof(uint64_t)),
                              page_backup[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return KNS_STATUS_ERROR;
        }
    }

    HAL_FLASH_Lock();
    return KNS_STATUS_OK;
}