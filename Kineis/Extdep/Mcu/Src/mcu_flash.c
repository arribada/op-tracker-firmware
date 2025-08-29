/* SPDX-License-Identifier: no SPDX license */
/**
 * @file    mcu_flash.c
 * @brief   MCU flash memory system
 * @author  Arribada
 * @date    Creation 2022/01/31
 */

#include "mcu_flash.h"
#include "kns_types.h"
#include <string.h>
#include <limits.h>
#include "stm32wlxx_hal.h"
#include "mgr_log.h" /* optional for debug */

/* ---------- Internal helpers ---------- */

static inline uint64_t read_flash_word(uint32_t address)
{
    return *(const volatile uint64_t*)address;
}

static inline uint32_t flash_page_index(uint32_t addr)
{
    return (addr - FLASH_BASE) / FLASH_PAGE_SIZE;
}

static inline uint32_t ceil_div_u32(uint32_t a, uint32_t b)
{
    return (a + b - 1U) / b;
}

/* Save/restore PRIMASK so we don't re-enable IRQs if they were already off */
static inline uint32_t flash_critical_enter(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void flash_critical_exit(uint32_t primask)
{
    __set_PRIMASK(primask);
}

// Clear journal back to erased (all 1s) using page-safe writer
static inline enum KNS_status_t journal_clear(uint32_t journal_addr)
{
    const uint64_t ones = 0xFFFFFFFFFFFFFFFFULL;
    return MCU_FLASH_write(journal_addr, &ones, sizeof(ones)); // does erase+rewrite of header page
}

static inline uint32_t journal_from_of(uint32_t of_addr) {
    return of_addr + 8U;   // 8-byte word right after OF
}

static inline HAL_StatusTypeDef write_flash_word(uint32_t address, uint64_t value)
{
    // 8-byte alignment is mandatory for DOUBLEWORD programming
    if ((address & 0x7U) != 0U) return HAL_ERROR;

    // Must be inside internal Flash range
    if (address < FLASH_BASE || address > FLASH_END_ADDR - 7U) return HAL_ERROR;

    // Destination must be erased (all 1s)
    if (*(const volatile uint64_t*)address != 0xFFFFFFFFFFFFFFFFULL) return HAL_ERROR;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    HAL_StatusTypeDef st;
    HAL_FLASH_Unlock();

    // Clear any stale error flags that can make the next program fail immediately
#ifdef FLASH_FLAG_ALL_ERRORS
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
#else
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_SIZERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PROGERR | FLASH_FLAG_MISERR |
                           FLASH_FLAG_FASTERR | FLASH_FLAG_RDERR | FLASH_FLAG_OPERR);
#endif

    st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, value);

    HAL_FLASH_Lock();
    __set_PRIMASK(primask);

    return st;
}

// return the HAL status so callers can react
static inline HAL_StatusTypeDef write_qword(uint32_t addr, uint64_t v)
{
    return write_flash_word(addr, v); // this already masks IRQs + unlock/lock
}

/* ---------- Public API ---------- */

enum KNS_status_t MCU_FLASH_read(uint32_t address, void *buffer, size_t size)
{
    if (!buffer || size == 0) {
        return KNS_STATUS_FLASH_ERR;
    }
    /* Basic range check (avoid overflow on address+size-1) */
    if (size > (size_t)(UINT32_MAX - address)) {
        return KNS_STATUS_FLASH_ERR;
    }
    if (address < FLASH_USER_START_ADDR ||
        (address + (uint32_t)size - 1U) > FLASH_USER_END_ADDR) {
        return KNS_STATUS_FLASH_ERR;
    }

    memcpy(buffer, (const void*)address, size);
    return KNS_STATUS_OK;
}

/* Page-sized static backup to avoid stack overflow (STM32WL page = 4 KB) */
static uint64_t s_page_backup[FLASH_PAGE_SIZE / sizeof(uint64_t)];

enum KNS_status_t MCU_FLASH_write(uint32_t address, const void *data, size_t size)
{
    if (!data || size == 0) {
        return KNS_STATUS_FLASH_ERR;
    }
    /* Range & overflow checks */
    if (size > (size_t)(UINT32_MAX - address)) {
        return KNS_STATUS_FLASH_ERR;
    }
    if (address < FLASH_USER_START_ADDR ||
        (address + (uint32_t)size - 1U) > FLASH_USER_END_ADDR) {
        return KNS_STATUS_FLASH_ERR;
    }

    const uint8_t *src = (const uint8_t*)data;

    while (size) {
        uint32_t page_start = address - (address % FLASH_PAGE_SIZE);
        uint32_t offset_in_page = address - page_start;
        uint32_t bytes_left_in_page = FLASH_PAGE_SIZE - offset_in_page;
        uint32_t chunk = (uint32_t)((size < bytes_left_in_page) ? size : bytes_left_in_page);

        /* Backup and merge */
        memcpy(s_page_backup, (const void*)page_start, FLASH_PAGE_SIZE);
        memcpy(((uint8_t*)s_page_backup) + offset_in_page, src, chunk);

        /* Critical section: erase & program this page */
        uint32_t pm = flash_critical_enter();

        HAL_FLASH_Unlock();

        FLASH_EraseInitTypeDef e = {
            .TypeErase = FLASH_TYPEERASE_PAGES,
            .Page      = flash_page_index(page_start),
            .NbPages   = 1
        };
        uint32_t pe = 0;
        if (HAL_FLASHEx_Erase(&e, &pe) != HAL_OK) {
            HAL_FLASH_Lock();
            flash_critical_exit(pm);
            return KNS_STATUS_FLASH_ERR;
        }

        /* Re-program the full page, 8 bytes at a time */
        for (uint32_t i = 0; i < (FLASH_PAGE_SIZE / 8U); ++i) {
            uint32_t prog_addr = page_start + (i * 8U);
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, prog_addr, s_page_backup[i]) != HAL_OK) {
                HAL_FLASH_Lock();
                flash_critical_exit(pm);
                return KNS_STATUS_FLASH_ERR;
            }
        }

        HAL_FLASH_Lock();
        flash_critical_exit(pm);

        /* Advance */
        address += chunk;
        src     += chunk;
        size    -= chunk;
    }

    return KNS_STATUS_OK;
}

/* Wear-leveling counters -------------------------------------------------- */
uint64_t read_wear_counter(uint32_t start_addr, uint32_t wl_size_words, uint32_t overflow_addr)
{
    uint32_t valid = 0;
    for (uint32_t i = 0; i < wl_size_words; ++i) {
        if (read_flash_word(start_addr + i*8U) == 0xFFFFFFFFFFFFFFFFULL) break;
        valid = i + 1U;
    }

    uint64_t of = read_flash_word(overflow_addr);
    if (of == 0xFFFFFFFFFFFFFFFFULL) of = 0;

    const uint32_t journal_addr = journal_from_of(overflow_addr);
    const uint64_t journal = read_flash_word(journal_addr);

    // Compensate ONLY genuine interrupted-overflow
    if (journal == 0ULL && valid == 0U && of > 0U) {
        return of * wl_size_words + 1U;
    }

    return of * wl_size_words + valid;
}


enum KNS_status_t increment_wear_counter(uint32_t wl_start, uint32_t wl_size, uint32_t of_addr)
{
    // Normal path
    for (uint32_t i = 0; i < wl_size; ++i) {
        if (read_flash_word(wl_start + i * 8U) == 0xFFFFFFFFFFFFFFFFULL) {
            return (write_flash_word(wl_start + i * 8U, 0ULL) == HAL_OK)
                     ? KNS_STATUS_OK : KNS_STATUS_FLASH_ERR;
        }
    }

    uint32_t journal_addr = journal_from_of(of_addr);
    uint64_t jcur = read_flash_word(journal_addr);
    if (jcur != 0ULL) {
        uint64_t zero = 0ULL;
        if (MCU_FLASH_write(journal_addr, &zero, sizeof(zero)) != KNS_STATUS_OK) {
            return KNS_STATUS_FLASH_ERR;
        }
    }

    // (b) erase WL region
    uint32_t pm = flash_critical_enter();
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Page      = flash_page_index(wl_start),
        .NbPages   = ceil_div_u32(wl_size * 8U, FLASH_PAGE_SIZE)
    };
    if (erase.NbPages == 0U) erase.NbPages = 1U;
    uint32_t err = 0;
    if (HAL_FLASHEx_Erase(&erase, &err) != HAL_OK) {
        HAL_FLASH_Lock(); flash_critical_exit(pm);
        (void)journal_clear(journal_addr);  // use page-safe clear
        return KNS_STATUS_FLASH_ERR;
    }
    HAL_FLASH_Lock(); flash_critical_exit(pm);

    // (c) bump OF (treat erased as 0)
    uint64_t of = read_flash_word(of_addr);
    if (of == 0xFFFFFFFFFFFFFFFFULL) of = 0;
    of++;
    if (MCU_FLASH_write(of_addr, &of, sizeof(of)) != KNS_STATUS_OK) {
        // leave journal=0 → read path compensates (+1) if needed
        return KNS_STATUS_FLASH_ERR;
    }

    // (d) write first slot
    if (write_flash_word(wl_start, 0ULL) != HAL_OK) {
        return KNS_STATUS_FLASH_ERR;
    }

    // (e) clear journal
    const uint64_t ones = 0xFFFFFFFFFFFFFFFFULL;
    if (MCU_FLASH_write(journal_addr, &ones, sizeof(ones)) != KNS_STATUS_OK) {
        return KNS_STATUS_FLASH_ERR;
    }

    return KNS_STATUS_OK;
}

enum KNS_status_t reset_wear_counter(uint32_t wl_start, uint32_t wl_size, uint32_t of_addr)
{
    uint64_t of = 0;
    if (MCU_FLASH_write(of_addr, &of, sizeof(of)) != KNS_STATUS_OK) {
        return KNS_STATUS_FLASH_ERR;
    }

    uint32_t pm = flash_critical_enter();

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Page      = flash_page_index(wl_start),
        .NbPages   = ceil_div_u32(wl_size * 8U, FLASH_PAGE_SIZE)
    };
    if (erase.NbPages == 0U) {
        erase.NbPages = 1U;
    }

    uint32_t error = 0;
    HAL_StatusTypeDef st = HAL_FLASHEx_Erase(&erase, &error);

    HAL_FLASH_Lock();
    flash_critical_exit(pm);

    return (st == HAL_OK) ? KNS_STATUS_OK : KNS_STATUS_FLASH_ERR;
}

enum KNS_status_t set_wear_counter(uint32_t wl_start,
                                   uint32_t wl_size,
                                   uint32_t of_addr,
                                   uint64_t value)
{
    const uint32_t journal_addr = journal_from_of(of_addr);

    /* derive overflow count and in-page write index */
    uint64_t of = value / wl_size;
    uint32_t wl_index = (uint32_t)(value % wl_size);

    /* (1) journal start — ensure journal == 0 using page-safe writer */
    {
        uint64_t jcur = read_flash_word(journal_addr);
        if (jcur != 0ULL) {
            const uint64_t zero = 0ULL;
            if (MCU_FLASH_write(journal_addr, &zero, sizeof(zero)) != KNS_STATUS_OK) {
                return KNS_STATUS_FLASH_ERR;
            }
        }
    }

    /* (2) erase WL region (full reset of WL pages) */
    uint32_t pm = flash_critical_enter();
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase = (FLASH_EraseInitTypeDef){
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Page      = flash_page_index(wl_start),
        .NbPages   = ceil_div_u32(wl_size * 8U, FLASH_PAGE_SIZE)
    };
    if (erase.NbPages == 0U) erase.NbPages = 1U;

    uint32_t err = 0;
    if (HAL_FLASHEx_Erase(&erase, &err) != HAL_OK) {
        HAL_FLASH_Lock();
        flash_critical_exit(pm);
        (void)journal_clear(journal_addr); /* best effort */
        return KNS_STATUS_FLASH_ERR;
    }
    HAL_FLASH_Lock();
    flash_critical_exit(pm);

    /* (3) write OF (page-safe, preserves other header words) */
    if (MCU_FLASH_write(of_addr, &of, sizeof(of)) != KNS_STATUS_OK) {
        (void)journal_clear(journal_addr);
        return KNS_STATUS_FLASH_ERR;
    }

    /* (4) program wl_index slots with 0 (normal progression) */
    for (uint32_t i = 0; i < wl_index; ++i) {
        if (write_qword(wl_start + i * 8U, 0ULL) != HAL_OK) {
            (void)journal_clear(journal_addr);
            return KNS_STATUS_FLASH_ERR;
        }
    }

    /* (5) clear journal back to erased state (all 1s) */
    {
        const uint64_t ones = 0xFFFFFFFFFFFFFFFFULL;
        if (MCU_FLASH_write(journal_addr, &ones, sizeof(ones)) != KNS_STATUS_OK) {
            return KNS_STATUS_FLASH_ERR;
        }
    }

    return KNS_STATUS_OK;
}

/* MSG and WKU counter wrappers ------------------------------------------- */

uint64_t MCU_FLASH_read_msg_counter(void)
{
    return read_wear_counter(FLASH_MSG_COUNTER_WL_START_ADDR,
                             FLASH_MSG_COUNTER_WL_SIZE,
                             FLASH_MSG_COUNTER_OF_ADDR);
}

enum KNS_status_t MCU_FLASH_increment_msg_counter(void)
{
    return increment_wear_counter(FLASH_MSG_COUNTER_WL_START_ADDR,
                                  FLASH_MSG_COUNTER_WL_SIZE,
                                  FLASH_MSG_COUNTER_OF_ADDR);
}

enum KNS_status_t MCU_FLASH_reset_msg_counter(void)
{
    return reset_wear_counter(FLASH_MSG_COUNTER_WL_START_ADDR,
                              FLASH_MSG_COUNTER_WL_SIZE,
                              FLASH_MSG_COUNTER_OF_ADDR);
}

enum KNS_status_t MCU_FLASH_set_msg_counter(uint64_t value)
{
    return set_wear_counter(FLASH_MSG_COUNTER_WL_START_ADDR,
                            FLASH_MSG_COUNTER_WL_SIZE,
                            FLASH_MSG_COUNTER_OF_ADDR,
                            value);
}

uint64_t MCU_FLASH_read_wku_counter(void)
{
    return read_wear_counter(FLASH_WKU_COUNTER_WL_START_ADDR,
                             FLASH_WKU_COUNTER_WL_SIZE,
                             FLASH_WKU_COUNTER_OF_ADDR);
}

enum KNS_status_t MCU_FLASH_increment_wku_counter(void)
{
    return increment_wear_counter(FLASH_WKU_COUNTER_WL_START_ADDR,
                                  FLASH_WKU_COUNTER_WL_SIZE,
                                  FLASH_WKU_COUNTER_OF_ADDR);
}

enum KNS_status_t MCU_FLASH_reset_wku_counter(void)
{
    return reset_wear_counter(FLASH_WKU_COUNTER_WL_START_ADDR,
                              FLASH_WKU_COUNTER_WL_SIZE,
                              FLASH_WKU_COUNTER_OF_ADDR);
}

enum KNS_status_t MCU_FLASH_set_wku_counter(uint64_t value)
{
    return set_wear_counter(FLASH_WKU_COUNTER_WL_START_ADDR,
                            FLASH_WKU_COUNTER_WL_SIZE,
                            FLASH_WKU_COUNTER_OF_ADDR,
                            value);
}

/* Busy wait helper (optional) -------------------------------------------- */

bool MCU_FLASH_Wait_Ready(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) {
        if ((HAL_GetTick() - start) > timeout_ms) {
            return false;
        }
    }
    return true;
}



// /* SPDX-License-Identifier: no SPDX license */
// /**
//  * @file    mcu_flash.c
//  * @brief   MCU flash memory system
//  * @author  Arribada
//  * @date    Creation 2022/01/31
//  */

// /**
//  * @page mcu_flash_page MCU flash
//  *
//  * Flash memory is used to store ID, ADDR, Secret Key
//  *  TODO :RIGH now memory size is not optimized.
//  *
//  * @note
//  *
//  */

// /**
//  * @addtogroup MCU_FLASH
//  * @brief MCU FLASH
//  * @{
//  */


// #include "mcu_flash.h"
// #include "kns_types.h"
// #include <string.h>
// #include "stm32wlxx_hal.h"
// #include "mgr_log.h" /* @note This log is for debug, can be deleted */

// /**
//  * @brief Reads data from flash memory byte-by-byte.
//  *
//  * @param dest Pointer to the buffer where the data will be stored.
//  * @param ofset Flash memory address to read from.
//  * @param length Number of bytes to read.
//  * @return KNS_status_t Status of the operation.
//  */
// enum KNS_status_t MCU_FLASH_read(uint32_t address, void *buffer, size_t size)
// {
//     if (!buffer || size == 0) return KNS_STATUS_FLASH_ERR; // Safety check
//     memcpy(buffer, (void*)address, size);
//     return KNS_STATUS_OK;
// }

// /**
//  * @brief Writes data to flash memory byte-by-byte.
//  *
//  * @param src Pointer to the buffer containing the data.
//  * @param ofset Flash memory address to write to.
//  * @param length Number of bytes to write.
//  * @return KNS_status_t Status of the operation.
//  */
// // enum KNS_status_t MCU_FLASH_write(uint32_t address, const void *data, size_t size)
// // {
// //     if (!data || size == 0) return KNS_STATUS_FLASH_ERR; // Safety check

// //     uint64_t page_backup[FLASH_PAGE_SIZE / sizeof(uint64_t)];
// //     // Align to the base of the flash page containing the address
// //     uint32_t page_start_addr = address - (address % FLASH_PAGE_SIZE);

// //     memcpy(page_backup, (uint64_t*)page_start_addr, FLASH_PAGE_SIZE); // Backup full page

// //     // Copy new data into the correct position in the backup
// //     memcpy(((uint8_t*)page_backup) + (address - page_start_addr), data, size);

// //     HAL_FLASH_Unlock();

// //     // Erase the Flash page
// //     FLASH_EraseInitTypeDef EraseInitStruct;
// //     uint32_t PageError;
// //     EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
// //     EraseInitStruct.Page = (page_start_addr - FLASH_BASE) / FLASH_PAGE_SIZE,
// //     EraseInitStruct.NbPages = 1;

// //     if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
// //         HAL_FLASH_Lock();
// //         return KNS_STATUS_FLASH_ERR;
// //     }

// //     // Re-write full page to Flash
// //     for (uint32_t i = 0; i < FLASH_PAGE_SIZE / sizeof(uint64_t); i++) {
// //         if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, page_start_addr  + (i * sizeof(uint64_t)), page_backup[i]) != HAL_OK) {
// //             HAL_FLASH_Lock();
// //             return KNS_STATUS_FLASH_ERR;
// //         }
// //     }

// //     HAL_FLASH_Lock();
// //     __ISB();
// //     return KNS_STATUS_OK;
// // }
// enum KNS_status_t MCU_FLASH_write(uint32_t address, const void *data, size_t size)
// {
//     if (!data || size == 0) return KNS_STATUS_FLASH_ERR;
//     if (address < FLASH_USER_START_ADDR || (address + size - 1) > FLASH_USER_END_ADDR) return KNS_STATUS_FLASH_ERR;

//     while (size) {
//         uint32_t page_start = address - (address % FLASH_PAGE_SIZE);
//         uint32_t in_page    = FLASH_PAGE_SIZE - (address - page_start);
//         uint32_t chunk      = (size < in_page) ? size : in_page;

//         static uint64_t page_backup[FLASH_PAGE_SIZE / 8];   // static: avoid stack overflow
//         memcpy(page_backup, (void*)page_start, FLASH_PAGE_SIZE);
//         memcpy((uint8_t*)page_backup + (address - page_start), data, chunk);

//         HAL_FLASH_Unlock();
//         FLASH_EraseInitTypeDef e = { .TypeErase = FLASH_TYPEERASE_PAGES,
//                                      .Page = (page_start - FLASH_BASE) / FLASH_PAGE_SIZE,
//                                      .NbPages = 1 };
//         uint32_t pe;
//         if (HAL_FLASHEx_Erase(&e, &pe) != HAL_OK) { HAL_FLASH_Lock(); return KNS_STATUS_FLASH_ERR; }

//         for (uint32_t i = 0; i < FLASH_PAGE_SIZE / 8; ++i) {
//             if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, page_start + i*8, page_backup[i]) != HAL_OK) {
//                 HAL_FLASH_Lock(); return KNS_STATUS_FLASH_ERR;
//             }
//         }
//         HAL_FLASH_Lock();

//         address += chunk;
//         data = (const uint8_t*)data + chunk;
//         size -= chunk;
//     }
//     return KNS_STATUS_OK;
// }



// /**
//  * @brief Read a 64-bit word from flash memory.
//  * @param address Address to read from.
//  * @return 64-bit value at the given address.
//  */
// static inline uint64_t read_flash_word(uint32_t address) {
//     return *(const volatile uint64_t*)address;
// }

// /**
//  * @brief Write a 64-bit word to flash memory.
//  * @param address Address to write to.
//  * @param value 64-bit value to write.
//  * @return HAL status of the operation.
//  */
// static HAL_StatusTypeDef write_flash_word(uint32_t address, uint64_t value) {
//     HAL_FLASH_Unlock();
//     HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, value);
//     HAL_FLASH_Lock();

//     }

// /**
//  * @brief Read the current wear-leveled counter value.
//  * @param start_addr Start address of the wear-leveling area.
//  * @param wl_size_words Number of 64-bit words in the wear-leveling area.
//  * @param overflow_addr Address where the overflow counter is stored.
//  * @return Current counter value.
//  */
// uint64_t read_wear_counter(uint32_t start_addr, uint32_t wl_size_words, uint32_t overflow_addr) {
//     uint32_t valid_index = 0;
//     for (uint32_t i = 0; i < wl_size_words; ++i) {
//         uint64_t val = read_flash_word(start_addr + i * 8);
//         if (val == 0xFFFFFFFFFFFFFFFFULL) break;
//         valid_index = i + 1;
//     }
//     uint64_t overflow = *(uint64_t*)overflow_addr;
//     return overflow * wl_size_words + valid_index;
// }

// /**
//  * @brief Increment the wear-leveled counter.
//  * @param wl_start Start address of the wear-leveling area.
//  * @param wl_size Size in 64-bit words.
//  * @param of_addr Address of the overflow counter.
//  * @return KNS status of the operation.
//  */
// enum KNS_status_t increment_wear_counter(uint32_t wl_start, uint32_t wl_size, uint32_t of_addr)
// {
//     uint32_t i = 0;
//     for (; i < wl_size; ++i) {
//         if (read_flash_word(wl_start + i * 8) == 0xFFFFFFFFFFFFFFFFULL) break;
//     }

//     if (i < wl_size) {
//         return write_flash_word(wl_start + i * 8, 0ULL);
//     }

//     // full: bump overflow, erase WL area, write first slot
//     uint64_t of = read_flash_word(of_addr);
//     if (of == 0xFFFFFFFFFFFFFFFFULL) of = 0;  // treat erased as 0
//     ++of;
//     if (MCU_FLASH_write(of_addr, &of, sizeof(of)) != KNS_STATUS_OK) return KNS_STATUS_FLASH_ERR;

//     HAL_FLASH_Unlock();
//     FLASH_EraseInitTypeDef erase = {
//         .TypeErase = FLASH_TYPEERASE_PAGES,
//         .Page     = (wl_start - FLASH_BASE) / FLASH_PAGE_SIZE,
//         .NbPages  = (uint32_t)((wl_size * 8 + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE)  // ceil
//     };
//     uint32_t error;
//     if (HAL_FLASHEx_Erase(&erase, &error) != HAL_OK) { HAL_FLASH_Lock(); return KNS_STATUS_FLASH_ERR; }
//     HAL_FLASH_Lock();

//     return write_flash_word(wl_start, 0ULL);
// }
