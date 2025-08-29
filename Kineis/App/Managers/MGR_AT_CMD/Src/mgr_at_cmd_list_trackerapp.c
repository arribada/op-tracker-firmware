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
#include "mcu_nvm.h"

/* Defines -------------------------------------------------------------------------------------- */
/* Private variables -----------------------------------------------------------------------------*/
/* Private function prototypes (usefull for later internal functions) --------*/
/* Public functions ------------------------------------------------------------------------------*/
bool bMGR_AT_CMD_TRACKER_cmd(uint8_t *pu8_cmdParamString, enum atcmd_type_t e_exec_mode)
{

	if (e_exec_mode == ATCMD_STATUS_MODE) {
		tracker_app_vars_t *tracker_conf = NULL;
		if (TRACKER_read_conf(&tracker_conf) != KNS_STATUS_OK)
			/* TODO: add a new error code ? */
			return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN);
		
	 	MCU_AT_CONSOLE_send("+TRACKER: GUI=%u MSG_CNT=%u, MSG_INTERVAL=%us, WKU_INTERVAL=%umin, SEQ_INTERVAL=%u, BAT_LEVEL_THRESH=%u\r\n",
                        tracker_conf->u8_with_gui,
                        tracker_conf->u8_msg_counter,
                        tracker_conf->u8_wait_msg_timer_s,
                        tracker_conf->u8_wait_startup_restimer_min,
                        tracker_conf->u8_wait_sequence_nmb_startup,
                        tracker_conf->u8_bat_level_threshold);
		uint16_t message_counter;
		MCU_NVM_getMC(&message_counter);
		uint16_t message_counter_flash;
		MCU_NVM_getFlashMC(&message_counter_flash);
		MGR_LOG_DEBUG("Message Counter: %u, Flash Message Counter: %u\r\n", message_counter, message_counter_flash);
		return true;
	} else if (e_exec_mode == ATCMD_ACTION_MODE) {
		tracker_app_vars_t tracker_conf = {0};
		unsigned int is_gui, msg_counter, msg_timer, restimer, sequence_timer, bat_threshold;
		if (sscanf((char*)pu8_cmdParamString, "%*[^=]= %u,%u,%u,%u,%u,%u", &is_gui,
				&msg_counter, &msg_timer, &restimer,  &sequence_timer, &bat_threshold) != 6) {
			MGR_LOG_DEBUG("Missing parameter: AT+TRACKER=GUI,MSG_CNT,MSG_INTERVAL,WKU_INTERVAL,SEQ_INTERVAL,BAT_THRES \r\n", msg_counter);

			return bMGR_AT_CMD_logFailedMsg(ERROR_PARAMETER_FORMAT);
		}
		
		if(is_gui > 1) {
			MGR_LOG_DEBUG("IS_GUI (Nmb message per sequence) should be between 0 to 1: %u\r\n", is_gui);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_with_gui = (uint8_t)is_gui;
			MGR_LOG_DEBUG("GUI=: %u - GUI enabled\r\n", tracker_conf.u8_with_gui);
		}

		if(msg_counter > 255) {
			MGR_LOG_DEBUG("MSG_CNT (Nmb message per sequence) should be between 0 to 255: %u\r\n", msg_counter);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_msg_counter = (uint8_t)msg_counter;
			MGR_LOG_DEBUG("MSG_CNT=: %u - Number of transmission in one sequence\r\n", tracker_conf.u8_msg_counter);
		}

		if(msg_timer > 255) {
			MGR_LOG_DEBUG("MSG_INTERVAL (seconds between two messages) should be between 0 to 255: %u\r\n", msg_timer);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_wait_msg_timer_s = (uint8_t)msg_timer;
			MGR_LOG_DEBUG("MSG_INTERVAL=: %u - Interval between two messages in one sequence (seconds)\r\n", tracker_conf.u8_wait_msg_timer_s);
		}

		if(restimer > 255) {
			MGR_LOG_DEBUG("WKU_INTERNAL (fix value of on-board resistors, wake up interval) should be between 0 to 255: %u\r\n", restimer);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_wait_startup_restimer_min = (uint8_t)restimer;
			MGR_LOG_DEBUG("WKU_INTERVAL=: %u - Wake up interval defined by TPL resistors (hours)\r\n", tracker_conf.u8_wait_startup_restimer_min);
		}

		if((sequence_timer > 255) || (sequence_timer < 1)) {
			MGR_LOG_DEBUG("SEQ_INTERVAL (inteval between two sequences modulo WKU_INTERVAL) should be between 1 to 255: %u\r\n", sequence_timer);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_wait_sequence_nmb_startup = (uint8_t)sequence_timer;
			MGR_LOG_DEBUG("SEQ_INTERVAL=: %u - Pseudo time interval based on WKU_INTERVAL, Every X WKU interval\r\n", tracker_conf.u8_wait_sequence_nmb_startup);
		}

		if(bat_threshold > 100) {
			MGR_LOG_DEBUG("Low battery level threshold should be between 0 to 100: %u\r\n", bat_threshold);
            return bMGR_AT_CMD_logFailedMsg(ERROR_INCOMPATIBLE_VALUE);
		} else {
			tracker_conf.u8_bat_level_threshold = (uint8_t)bat_threshold;
			MGR_LOG_DEBUG("BAT_THRES=: %u - Low battery level threshold (not used)\r\n", tracker_conf.u8_bat_level_threshold);
		}

        if (TRACKER_set_conf(&tracker_conf) != KNS_STATUS_OK) {
            return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN);
        }
		
		// Reset counter here:
		MCU_NVM_setMC(0);
		MCU_NVM_setFlashMC(0);
		uint16_t message_counter;
		MCU_NVM_getMC(&message_counter);
		uint16_t message_counter_flash;
		MCU_NVM_getFlashMC(&message_counter_flash);
		MGR_LOG_DEBUG("Reset MC: %u, Flash Message Counter: %u\r\n", message_counter, message_counter_flash);

		MCU_NVM_setWUC(0);
		uint16_t WUC_flash;
		MCU_NVM_getWUC(&WUC_flash);
		MGR_LOG_DEBUG("Reset WUC in Flash: %u\r\n", WUC_flash);
        return bMGR_AT_CMD_logSucceedMsg();
		
	} else {
		return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN_AT_CMD);
	}
}

bool bMGR_AT_CMD_WUC_cmd(uint8_t *pu8_cmdParamString, enum atcmd_type_t e_exec_mode)
{

	enum KNS_status_t status;
	uint16_t wuc;

	if (e_exec_mode == ATCMD_STATUS_MODE) {
		status = MCU_NVM_getWUC(&wuc);
		if (status != KNS_STATUS_OK)
			return bMGR_AT_CMD_logFailedMsg((enum ERROR_RETURN_T) status);

		MCU_AT_CONSOLE_send("+WUC=%d\r\n", wuc);
		return bMGR_AT_CMD_logSucceedMsg();
	} else if(e_exec_mode == ATCMD_ACTION_MODE) {
		if (sscanf((char*)pu8_cmdParamString, "%*[^=]= %hu", &wuc) != 1)
		{
			MGR_LOG_DEBUG("Missing parameter: AT+WUC=VALUE\r\n");
			return bMGR_AT_CMD_logFailedMsg(ERROR_PARAMETER_FORMAT);
		} else {
			if(MCU_NVM_setWUC(wuc) == KNS_STATUS_OK)
			{
				MGR_LOG_DEBUG("+WKU=%u\r\n", wuc);
			} else {
				MGR_LOG_DEBUG("Failed to update WKU=%u\r\n ", wuc);
				return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN);
			}
			return bMGR_AT_CMD_logSucceedMsg();
		}
	}

	return bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN_AT_CMD);
}

/**
 * @}
 */
