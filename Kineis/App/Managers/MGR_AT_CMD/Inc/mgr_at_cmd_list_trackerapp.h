/* SPDX-License-Identifier: no SPDX license */
/**
 * @file mgr_at_cmd_list_trackerapp.h
 * @author  Arribada
 * @brief subset of AT commands concerning specifics for tracker application
 */

/**
 * @addtogroup MGR_AT_CMD
 * @{
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MGR_AT_CMD_TRACKERAPP_H
#define __MGR_AT_CMD_TRACKERAPP_H

/* Includes ------------------------------------------------------------------*/

/**
 * @brief Process AT command "AT+TRACKER"
 *
 * - sequence_nmb_startup
 * - Interval between sequence correlated to u16_wait_sequence_timer_min.
 * - 0 to 255 (max interval is u8_wait_startup_restimer_min x u8_wait_sequence_nmb_startup = 2hours x 255 = 510 hours = 21,25 days
 * - example
 *   - u8_wait_startup_restimer_min set to 1 hour
 *   - u16_wait_sequence_timer_min set to 0 = no periodic sequence, only trigger sequence (later)
 *   - u16_wait_sequence_timer_min set to 1 = sequence forwarded every hours
 *   - u16_wait_sequence_timer_min set to 5 = sequence forwarded every 5 hours 
 * 
 * @param[in] msg_counter message per sequences (0 to 255)
 * @param[in] msg_timer message timer interval in seconds (0 to 255)
 * @param[in] startup_restimer_min startup restimer in minutes (0 to 255)
 * @param[in] sequence_nmb_startup number of restimer count before sending sequence (0 to 255)
 * @param[in] bat_level_threshold battery level threshold (0 to 255)
 *
 * @return true if command is correctly received and processed, false if error
 */
bool bMGR_AT_CMD_TRACKER_cmd(uint8_t *pu8_cmdParamString, enum atcmd_type_t e_exec_mode);

/**
 * @brief Process AT command "AT+TRACKER_START"
 *
 * Update the tracker counter to 1 and reset device to start in Standalone mode
 *
 * @return true if command is correctly received and processed, false if error
 */
bool bMGR_AT_CMD_TRACKER_START_cmd(uint8_t *pu8_cmdParamString, enum atcmd_type_t e_exec_mode);

/**
 * @brief Process AT command "AT+WUC"
 *
 * Read/Update the wake up counter of the trakcer application
 *
 * @return true if command is correctly received and processed, false if error
 */
bool bMGR_AT_CMD_WUC_cmd(uint8_t *pu8_cmdParamString, enum atcmd_type_t e_exec_mode);
#endif /* __MGR_AT_CMD_TRACKER_H */

/**
 * @}
 */
