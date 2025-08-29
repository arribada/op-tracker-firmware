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
  unsigned int startup_counter = (unsigned int) MCU_FLASH_read_wku_counter();
  MGR_LOG_DEBUG("%s::WakeUpcounter: %d \r\n", __func__, startup_counter);
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

enum KNS_status_t TRACKER_set_with_gui(uint8_t is_gui) {

	MGR_LOG_DEBUG("%s::called\r\n", __func__);
	tracker_app_vars_t temp;
	memcpy(&temp, tracker_conf, sizeof(tracker_app_vars_t));

    tracker_conf->u8_with_gui = is_gui;
    if (TRACKER_set_conf(tracker_conf) != KNS_STATUS_OK) {
    	TRACKER_update_local_conf(&temp);
        return KNS_STATUS_TRACKER_ERR;
    }

    MCU_NVM_setWUC(0);
    return KNS_STATUS_OK;
}

enum KNS_status_t TRACKER_get_with_gui(uint8_t *is_gui) {
	MGR_LOG_DEBUG("%s::called\r\n", __func__);
    if (is_gui == NULL) {
        return KNS_STATUS_TRACKER_ERR;
    }

    *is_gui = tracker_conf->u8_with_gui;
    return KNS_STATUS_OK;
}


enum KNS_status_t TRACKER_shutdown(bool increment_wku)
{
	MGR_LOG_DEBUG("%s::called\r\n", __func__);
    // Increment wakeup counter before to shutdown
	if (increment_wku)
	{
		MCU_FLASH_increment_wku_counter();
        unsigned int wuk_counter = MCU_FLASH_read_wku_counter();
        MGR_LOG_DEBUG("%s:: read wakeup counter: %d\r\n", __func__, wuk_counter);
	}

    //Save Message couner before to shutdown
    uint16_t mc;
    MCU_NVM_getMC(&mc); // always return KNS_STATUS_OK
    if (MCU_NVM_setFlashMC(++mc) != KNS_STATUS_OK){
    	MGR_LOG_DEBUG("%s:: failed to save message counter to flash\r\n", __func__);
    }

    // Verify
    uint16_t readback = 0;
    if (MCU_NVM_getFlashMC(&readback) != KNS_STATUS_OK){
    	MGR_LOG_DEBUG("%s:: failed to read message counter from flash\r\n", __func__);
    }
    if((mc) != readback) 
    {
        MGR_LOG_DEBUG("%s:: message counter mismatch: expected %d, got %d\r\n", __func__, mc, readback);
    } else {
        MGR_LOG_DEBUG("%s:: message counter verified: %d\r\n", __func__, mc);
    }

	if (tracker_conf != NULL) {
		free(tracker_conf);
		tracker_conf = NULL;
	}

    // 5) Make sure flash is idle before signalling DONE
    (void)MCU_FLASH_Wait_Ready(10);
    // 6) Memory barrier (in case of buffered stores)
    __DSB(); __ISB();
    // Flush UART

    MCU_UART_FlushTx(50);
    MCU_UART_DeInit();
	HAL_Delay(10);                 // Short delay for hardware reset
    // stop tracker with MCU_DONE
    HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_SET);
	HAL_Delay(10);                 // Short to toggle MCU_DONE pin
    HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_RESET);
	HAL_Delay(200);                 // Short delay before shutdown.
	//Deinit
    HAL_RCC_DeInit();

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

    // Update message counter here
    uint16_t mc;
    MCU_NVM_getMC(&mc);
    MGR_LOG_DEBUG("%s:: read message counter: %d\r\n", __func__, mc);
    if(MCU_NVM_getFlashMC(&mc) == KNS_STATUS_OK)
    {
        MGR_LOG_DEBUG("%s:: read message counter from flash: %d\r\n", __func__, mc);
        MCU_NVM_setMC(mc);
    } else {
    	MGR_LOG_DEBUG("%s:: failed to read message counter from flash memory\r\n", __func__);
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
