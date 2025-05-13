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

/* Struct -------------------------------------------------------------------------------------- */

/* Variables ----------------------------------------------------------------------------------- */

/* Local functions ----------------------------------------------------------------------------- */

/* Public functions ---------------------------------------------------------------------------- */
enum KNS_status_t TRACKER_init(uint64_t *startup_counter) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  (*startup_counter)++;
  MGR_LOG_DEBUG("%s::Startup counter: %d \r\n", __func__, *startup_counter);
  MCU_FLASH_set_counter(startup_counter); // Increment counter in flash memory
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Pin = MCU_DONE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MCU_DONE_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(MCU_DONE_GPIO_Port, MCU_DONE_Pin, GPIO_PIN_RESET);

  tracker_app_vars_t app_vars;
  TRACKER_get_conf(&app_vars);

  // Always used startup_counter -1 since it's incremented at the beginning of the function
  if ((app_vars.u8_wait_sequence_nmb_startup != 0) && ((*startup_counter-1) != 0)) {
	if ((*startup_counter-1) % app_vars.u8_wait_sequence_nmb_startup == 0)
	{
	  // STOP no sequence should be sent
      MGR_LOG_DEBUG("%s::next boot after %d \r\n", __func__, (((*startup_counter-1) % app_vars.u8_wait_sequence_nmb_startup == 0)));
	  TRACKER_stop();
	}
  }
  return KNS_STATUS_OK;
}


enum KNS_status_t TRACKER_stop(void)
{
    //enum KNS_status_t status;
    MGR_LOG_DEBUG("TRACKER_stop\r\n");
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
