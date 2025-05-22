// SPDX-License-Identifier: no SPDX license
/**
 * @file    tracker_app.c
 * @brief   Library to handle tracker application
 * @note
 */

/**
 * @addtogroup TRACKER_APP
 * @{
 */

/* Includes ------------------------------------------------------------------------------------ */

#include <stdbool.h>
#include <stddef.h>
#include "main.h"
#include "gpio.h"
#include "tracker_app.h"
#include "mcu_flash.h"
#include "kineis_sw_conf.h"  // for assert include below
#include KINEIS_SW_ASSERT_H
#include "mcu_flash.h"
#include "mcu_at_console.h"
#include "mgr_log.h" /* @note This log is for debug, can be deleted */
#include "kns_types.h"
#include <stdlib.h>
#include <string.h>
#include "mcu_nvm.h"




/* Struct -------------------------------------------------------------------------------------- */

/* Variables ----------------------------------------------------------------------------------- */
static tracker_app_vars_t *tracker_conf = NULL;
/* Local functions ----------------------------------------------------------------------------- */

/* Public functions ---------------------------------------------------------------------------- */
enum KNS_status_t TRACKER_update_local_conf(tracker_app_vars_t * app_vars) {
	//Store tracker conf to avoid to read flash each time
    if (app_vars == NULL) {
        return KNS_STATUS_TRACKER_ERR;
    }

    // Allocate memory for the global tracker_conf if not already done
    if (tracker_conf == NULL) {
        tracker_conf = malloc(sizeof(tracker_app_vars_t));
        if (tracker_conf == NULL) {
            return KNS_STATUS_TRACKER_ERR;
        }
    }
    // Copy the provided app_vars into tracker_conf
    memcpy(tracker_conf, app_vars, sizeof(tracker_app_vars_t));
    return KNS_STATUS_OK;

}

enum KNS_status_t TRACKER_init() {

	if (tracker_conf == NULL)
	{
		TRACKER_read_conf(&tracker_conf);
	}


  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Pin = MCU_DONE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MCU_DONE_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_RESET);

  // Always used startup_counter -1 since it's incremented at the beginning of the function
  unsigned int startup_counter = MCU_FLASH_read_wku_counter();
  MGR_LOG_DEBUG("%s::Startup counter: %d \r\n", __func__, startup_counter);
  if (startup_counter % tracker_conf->u8_wait_sequence_nmb_startup == 0) {
    // START sequence should be sent
    MGR_LOG_DEBUG("%s::tracker starting sequence:: WKU = %d modulo %d\r\n", __func__, startup_counter, tracker_conf->u8_wait_sequence_nmb_startup);
    //TRACKER_start();
  } else {
	// STOP no sequence should be sent
	MGR_LOG_DEBUG("%s::tracker not starting sequence:: WKU = %d modulo %d\r\n", __func__, startup_counter, tracker_conf->u8_wait_sequence_nmb_startup);
	TRACKER_shutdown(true);
  }
  return KNS_STATUS_OK;
}

enum KNS_status_t TRACKER_set_is_running(uint8_t is_running) {

	MGR_LOG_DEBUG("%s::called\r\n", __func__);
	tracker_app_vars_t temp;
	memcpy(&temp, tracker_conf, sizeof(tracker_app_vars_t));

    tracker_conf->u8_is_running = is_running;
    if (TRACKER_set_conf(tracker_conf) != KNS_STATUS_OK) {
    	TRACKER_update_local_conf(&temp);
        return KNS_STATUS_TRACKER_ERR;
    }

    MCU_NVM_setWUC(0);
    return KNS_STATUS_OK;
}

enum KNS_status_t TRACKER_get_is_running(uint8_t *is_running) {
	MGR_LOG_DEBUG("%s::called\r\n", __func__);
    if (is_running == NULL) {
        return KNS_STATUS_TRACKER_ERR;
    }

    *is_running = tracker_conf->u8_is_running;
    return KNS_STATUS_OK;
}


enum KNS_status_t TRACKER_shutdown(bool increment_wku)
{
	MGR_LOG_DEBUG("%s::called\r\n", __func__);
	if (increment_wku)
	{
		MCU_FLASH_increment_wku_counter();
	}

	if (tracker_conf != NULL) {
		free(tracker_conf);
		tracker_conf = NULL;
	}

    //enum KNS_status_t status;
    // Stop UART
    MCU_UART_DeInit();
	HAL_Delay(10);                 // Short delay for hardware reset
	HAL_RCC_DeInit();
    // stop tracker with MCU_DONE
    HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_SET);
	HAL_Delay(10);                 // Short delay for hardware reset
    HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_RESET);

    return KNS_STATUS_OK;
}

enum KNS_status_t TRACKER_get_conf(tracker_app_vars_t **conf)
{
    if (tracker_conf != NULL && conf != NULL) {
        *conf = tracker_conf;
        return KNS_STATUS_OK;
    } else {
		return KNS_STATUS_TRACKER_ERR;
    }
}


enum KNS_status_t TRACKER_read_conf(tracker_app_vars_t **app_vars)
{
	// Allocate memory for the global tracker_conf if not already done
	if (tracker_conf != NULL) {
		free(tracker_conf);
		tracker_conf = NULL;
	}

	tracker_conf = malloc(sizeof(tracker_app_vars_t));
	if (tracker_conf == NULL) {
		return KNS_STATUS_TRACKER_ERR;
	}

    if(MCU_FLASH_read(FLASH_USER_START_ADDR + FLASH_APP_VARS_OFFSET, tracker_conf, sizeof(tracker_app_vars_t)) == KNS_STATUS_OK)
    {
    		// IF local conf well updated, update return pointer to local_conf
		*app_vars = tracker_conf;
    } else {
    	MGR_LOG_DEBUG("%s:: failed to read tracker conf from flash memory\r\n", __func__);
    	return KNS_STATUS_TRACKER_ERR;
    }
    return KNS_STATUS_OK;
}

enum KNS_status_t TRACKER_set_conf(tracker_app_vars_t *app_vars)
{
    if (app_vars == NULL)
	{
    	return KNS_STATUS_TRACKER_ERR;
	}

    return MCU_FLASH_write(FLASH_USER_START_ADDR + FLASH_APP_VARS_OFFSET, app_vars, sizeof(tracker_app_vars_t));
}
/**
 * @}
 */
