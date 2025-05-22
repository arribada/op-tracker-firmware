/* SPDX-License-Identifier: no SPDX license */
/**
 * @file    mcu_nvm.h
 * @brief   MCU wrappers for Non volatile memory management
 * @author  Kineis
 * @date    Creation 2023/07/06
 */

/**
 * @page mcu_nvm_page MCU wrappers: NVM
 *
 * This is about storing ID and address into some Non-Volatile Memory (NVM).
 * Those settings are part of the credentials (ID, address, DSK) provided by Kineis operator.
 *
 * @note DSK will be managed by the used by the AES wrapper (mcu_aes.h)
 *
 * @attention It is up to you to manage the storing stategy of those values during the entire life
 * of your device.
 */

/**
 * @addtogroup MCU_WRAPPERS
 * @brief MCU wrapper used for Non volatile memory management.
 * @{
 */

#ifndef MCU_NVM_H
#define MCU_NVM_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "kns_types.h"
#include <stdbool.h>

/* Global defines -------------------------------------------------------------------------------*/
#define DEVICE_ADDR_LENGTH        4
#define DEVICE_SN_LENGTH          14

/* Function declaration -------------------------------------------------------------*/

/**
 * @brief get a 16-bit-long non-volatile value
 *
 * This is a read-only operation.
 *
 * @param[out] mc_ptr : 16-bit value to read
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_getMC(uint16_t *mc_ptr);

/**
 * @brief set a 16-bit-long non-volatile value
 *
 * The value may be set for each new user message sent to the Kineis stack.
 *
 * @attention Depending the expected lifetime of your device, ensure your flash memory
 * can support enough write/erase cycles.
 *
 * @param[in] mcTmp : 16-bit value to store
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_setMC(uint16_t mcTmp);

/**
 * @brief get a 16-bit-long non-volatile value
 *
 * This is a read-only operation.
 *
 * @param[out] wuc_ptr : 16-bit value to read
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_getWUC(uint16_t *wuc_ptr);

/**
 * @brief set a 16-bit-long non-volatile value
 *
 * The value may be set for tracker app at each wakeup.
 *
 * @attention Depending the expected lifetime of your device, ensure your flash memory
 * can support enough write/erase cycles.
 *
 * @param[in] wucTmp : 16-bit value to store
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_setWUC(uint16_t wucTmp);

/**
 * @brief get a pointer to the Kineis radio configuration
 *
 * This is a read-only operation.
 *
 * @param[out] ConfZonePtr : pointer to the Kineis radio configuration
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_getRadioConfZonePtr(void **ConfZonePtr);

/**
 * @brief set the Kineis radio configuration
 *
 * @param[in] ConfZonePtr : pointer to the Kineis radio configuration to write
 * @param[in] ConfZoneSize : number of bytes to write
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_setRadioConfZone(void *ConfZonePtr, uint16_t ConfZoneSize);

/**
 * @brief save the Kineis radio configuration into NVM
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_saveRadioConfZone(void);

/**
 * @brief get the Kineis identifier (32-bits long)
 *
 * This is a read-only operation.
 *
 * @param[out] dev_id : Kineis identifier
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_getID(uint32_t *id);

/**
 * @brief Set the Kineis identifier (32-bits long)
 *
 *
 * @param[out] id : Kineis identifier
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_setID(uint32_t *id);

/**
 * @brief get the Kineis address (4-bytes long)
 *
 * This is a read-only operation.
 *
 * @param[out] dev_addr : Kineis address
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_setAddr(uint8_t addr[]);

/**
 * @brief set the Kineis address (4-bytes long)
 *
 *
 * @param[out] dev_addr : Kineis address
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_getAddr(uint8_t addr[]);

/**
 * @brief get the device serial number (14-bytes null terminated ASCII string)
 *
 * @param[out] sn : serial number
 *
 * @return Status @ref KNS_status_t
 */
enum KNS_status_t MCU_NVM_getSN(uint8_t sn[]);

#endif /* MCU_NVM_H */

/**
 * @}
 */
