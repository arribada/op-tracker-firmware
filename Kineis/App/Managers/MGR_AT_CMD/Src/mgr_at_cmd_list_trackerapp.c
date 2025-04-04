// SPDX-License-Identifier: no SPDX license
/**
 * @file mgr_at_cmd_list_tracker.c
 * @author Arribada
 * @brief subset of AT commands concerning specifics for Tracker application
 */

/**
 * @addtogroup MGR_AT_CMD
 * @{
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "kns_types.h"
#include "tracker_app.h"
#include "mcu_flash.h"
#include "mcu_at_console.h"
#include "mgr_at_cmd.h" // needed for MGR_AT_CMD_isPendingAt
#include "mgr_at_cmd_common.h"
#include "mgr_log.h"
#include "kns_assert.h" // for kns_assert only

/* Defines -------------------------------------------------------------------------------------- */
/* Private variables -----------------------------------------------------------------------------*/
/* Private function prototypes (usefull for later internal functions) --------*/
/* Public functions ------------------------------------------------------------------------------*/
bool bMGR_AT_CMD_TRACKER_cmd(uint8_t *pu8_cmdParamString, enum atcmd_type_t e_exec_mode)
{
	tracker_app_vars_t tracker_conf = {0};

	if (e_exec_mode == ATCMD_STATUS_MODE) {
		if (TRACKER_get_conf(&tracker_conf) != KNS_STATUS_OK)
			/* TODO: add a new error code ? */
			return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN);
		
	 MCU_AT_CONSOLE_send("+TRACKER: MSG_CNT=%u, WAIT_MSG_TIMER_S=%us, WAIT_STARTUP_RESTIMER_MIN=%umin, WAIT_SEQ_NMB_STARTUP=%u, BAT_LEVEL_THRESH=%u\r\n",
                        tracker_conf.u8_msg_counter,
                        tracker_conf.u8_wait_msg_timer_s,
                        tracker_conf.u8_wait_startup_restimer_min,
                        tracker_conf.u8_wait_sequence_nmb_startup,
                        tracker_conf.u8_bat_level_threshold);
		return true;
	} else if (e_exec_mode == ATCMD_ACTION_MODE) {
		unsigned int msg_counter, msg_timer, restimer, sequence_timer, bat_threshold;
		if (sscanf((char*)pu8_cmdParamString, "%*[^=]= %u,%u,%u,%u,%u",
				&msg_counter, &msg_timer, &restimer,  &sequence_timer, &bat_threshold) != 5) {
			return bMGR_AT_CMD_logFailedMsg(ERROR_PARAMETER_FORMAT);
		}
		
		if(msg_counter > 255) {
			MGR_LOG_DEBUG("msg_counter per sequence should be between 0 to 255: %u", msg_counter);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_msg_counter = (uint8_t)msg_counter;
		}

		if(msg_timer > 255) {
			MGR_LOG_DEBUG("msg_timer per sequence should be between 0 to 255: %u", msg_timer);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_wait_msg_timer_s = (uint8_t)msg_timer;
		}

		if(restimer > 255) {
			MGR_LOG_DEBUG("wait startup restimer (fix value of on-board resistors) should be between 0 to 255: %u", restimer);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_wait_startup_restimer_min = (uint8_t)restimer;
		}

		if(sequence_timer > 255) {
			MGR_LOG_DEBUG("wait sequence number of startup (hw timer modulation) should be between 0 to 255: %u", sequence_timer);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_wait_sequence_nmb_startup = (uint8_t)sequence_timer;
		}

		if(bat_threshold > 100) {
			MGR_LOG_DEBUG("Low battery level threshold should be between 0 to 100: %u", bat_threshold);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_bat_level_threshold = (uint8_t)bat_threshold;
		}

        if (TRACKER_set_conf(&tracker_conf) != KNS_STATUS_OK) {
            return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN);
        }
        return bMGR_AT_CMD_logSucceedMsg();
		
	} else {
		return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN_AT_CMD);
	}
}

bool bMGR_AT_CMD_TRACKER_START_cmd(uint8_t *pu8_cmdParamString, enum atcmd_type_t e_exec_mode)
{

	if (e_exec_mode == ATCMD_STATUS_MODE) {
		uint64_t startup_counter = 0;
		if (MCU_FLASH_get_latest_counter(&startup_counter) != KNS_STATUS_OK)
			/* TODO: add a new error code ? */
			return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN);
		
		MCU_AT_CONSOLE_send("+TRACKER_START: %u (if 0 not started)\r\n", (unsigned int)startup_counter);
		return true;
	} else if (e_exec_mode == ATCMD_ACTION_MODE) {
		unsigned int startup_counter;
		if (sscanf((char*)pu8_cmdParamString, "%*[^=]= %u",
				&startup_counter) != 1) {
			return bMGR_AT_CMD_logFailedMsg(ERROR_PARAMETER_FORMAT);
		}
		if (startup_counter == 0) {
			MCU_AT_CONSOLE_send("+TRACKER_START: Not started set value different from 0\r\n");
		} else {
			TRACKER_start();
			MCU_AT_CONSOLE_send("+TRACKER_START: Starting ... Increment counter to 1\r\n");
		}

        return bMGR_AT_CMD_logSucceedMsg();
		
	} else {
		return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN_AT_CMD);
	}
}

/**
 * @}
 */
