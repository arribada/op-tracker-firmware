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

/* Struct -------------------------------------------------------------------------------------- */

/* Variables ----------------------------------------------------------------------------------- */

/* Local functions ----------------------------------------------------------------------------- */

/* Public functions ---------------------------------------------------------------------------- */
enum KNS_status_t TRACKER_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = MCU_DONE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MCU_DONE_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_RESET);

  return KNS_STATUS_OK;
}
enum KNS_status_t TRACKER_start(void) 
{
    enum KNS_status_t status;
	MGR_LOG_DEBUG("TRACKER_start\r\n");
    // Increment startup counter, next reset GUI will not start
    status = MCU_FLASH_increment_counter();

    TRACKER_stop();
    return status;
}

enum KNS_status_t TRACKER_stop(void)
{
    MGR_LOG_DEBUG("TRACKER_stop\r\n");

    MCU_UART_DeInit();

    __disable_irq();

	HAL_Delay(100);                 // Short delay for hardware reset
    // stop tracker with MCU_DONE
    HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_SET);
	HAL_Delay(1);                 // Short delay for hardware reset
    HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_RESET);

    return KNS_STATUS_OK;
}
enum KNS_status_t TRACKER_get_conf(tracker_app_vars_t *app_vars)
{
    if (!app_vars) return KNS_STATUS_ERROR;

    return MCU_FLASH_read(FLASH_USER_DATA_ADDR + FLASH_APP_VARS_OFFSET, app_vars, sizeof(tracker_app_vars_t));
}

enum KNS_status_t TRACKER_set_conf(const tracker_app_vars_t *app_vars)
{
    if (!app_vars) return KNS_STATUS_ERROR;

    return MCU_FLASH_write(FLASH_USER_DATA_ADDR + FLASH_APP_VARS_OFFSET, app_vars, sizeof(tracker_app_vars_t));
}
/**
 * @}
 */
