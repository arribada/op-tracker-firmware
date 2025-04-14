// SPDX-License-Identifier: no SPDX license
/**
 * @file    mcu_at_console.c
 * @author  Kinéis
 * @brief   MCU wrapper for AT CMD console
 */

/**
 * @addtogroup MCU_APP_WRAPPERS
 * @brief MCU wrapper used by Kineis Application example.
 *
 * One has to implement API as per its microcontroller and its platform ressources.
 * @{
 */

#ifdef AXM0
#if (defined(STM32WLE5xx) && (STM32WLE5xx == 1U)) || \
    (defined(STM32WL55xx) && (STM32WL55xx == 1U)) || \
	(defined(STM32L442xx) && (STM32L442xx == 1U))
#error choose one uC for MCU AT console wrapper
#endif
#include "mcu_at_console_axm0.c"
#else
#include "mcu_at_console_stm.c"
#endif

/**
 * @}
 */
