// SPDX-License-Identifier: no SPDX license
/**
 * @file    mcu_misc.c
 * @brief   File to implement miscellaneous features for a specific design
 * @author  Kineis
 * @date    Creation 2023/07/06
 * @version 1.0
 * @note
 */

/**
 * @addtogroup MCU_MISC
 * @{
 */

#include <stdbool.h>
#include "mcu_misc.h"
#include "main.h"
#include "mgr_log.h"

/* Defines -------------------------------------------------------------------------------------- */

/** @note Define a delay macro which does not depend on systick interrupt in a way to be able to run
 * from any ISR (e.g. turn ON PA wrapper below will be called from SUBGHZ ISR during continuous
 * modulated wave)
 *
 * @attention at typical 32MHz clock and 4 instruction cycle per loop, max value is 536870 ms
 *
 * NOP             : 1 cycle
 * SUBS R3, #1     : 1 cycle
 * CMP  R3, #0     : 1 cycle
 * BNE  <ADDR>     : 1 cycle
 */
//#define DELAY_MS(time_ms) HAL_Delay(time_ms)
static uint32_t tcxo_warmup_time_ms = 2000;
extern uint32_t SystemCoreClock;
#define FOR_LOOP_CYCLE_NB 4
#define DELAY_MS(time_ms) \
{ \
	uint32_t _nb_for_loop_per_ms = SystemCoreClock / 1000 / FOR_LOOP_CYCLE_NB; \
	for (uint32_t _count = time_ms * _nb_for_loop_per_ms; \
		_count > 0 ; \
		_count--) __NOP(); \
}

/* Functions ------------------------------------------------------------------------------------ */

void MCU_MISC_turn_on_pa()
{
	/** @attention this code may run under ISR, especially during continuous modulated wave */
#ifdef KRD_FW_MP
	GPIO_InitTypeDef GPIO_InitStruct = {0};


	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(PA_PSU_EN_GPIO_Port, PA_PSU_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PA_PSU_SEL_GPIO_Port, PA_PSU_SEL_Pin, GPIO_PIN_SET);


	//Already defined in MX_GPIO_INIT
	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = PA_PSU_EN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PA_PSU_EN_GPIO_Port, &GPIO_InitStruct);


	GPIO_InitStruct.Pin = PA_PSU_SEL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PA_PSU_SEL_GPIO_Port, &GPIO_InitStruct);

	HAL_GPIO_WritePin(PA_PSU_SEL_GPIO_Port, PA_PSU_SEL_Pin, GPIO_PIN_SET);
	DELAY_MS(10);
	HAL_GPIO_WritePin(PA_PSU_EN_GPIO_Port, PA_PSU_EN_Pin, GPIO_PIN_SET);
	DELAY_MS(MCU_PA_BOOTDELAY_MS);
#endif
}

void MCU_MISC_turn_off_pa()
{
	/** @attention this code may run under ISR, especially during continuous modulated wave */
#ifdef KRD_FW_MP
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_GPIO_WritePin(PA_PSU_EN_GPIO_Port, PA_PSU_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PA_PSU_SEL_GPIO_Port, PA_PSU_SEL_Pin, GPIO_PIN_RESET);


	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = PA_PSU_EN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(PA_PSU_EN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = PA_PSU_SEL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(PA_PSU_SEL_GPIO_Port, &GPIO_InitStruct);

	HAL_GPIO_WritePin(PA_PSU_SEL_GPIO_Port, PA_PSU_SEL_Pin, GPIO_PIN_SET);
	DELAY_MS(10);
	HAL_GPIO_WritePin(PA_PSU_EN_GPIO_Port, PA_PSU_EN_Pin, GPIO_PIN_RESET);
	DELAY_MS(MCU_PA_BOOTDELAY_MS);
#endif
}

enum KNS_status_t MCU_MISC_getSettingsHwRf(int8_t rf_level_dBm, void* rfSettings)
{
	struct rfSettings_t *settings = (struct rfSettings_t *)rfSettings;
	int8_t rficPaLevel, rficPaLevel_min, rficPaLevel_max;


#if defined(KRD_FW_MP)
	rficPaLevel_min = -17; // dBm
	rficPaLevel_max = 15;  // dBm

	settings->isRficHPA = false;
	settings->externalPaGain = 36;
#elif defined(KRD_FW_LP)
	rficPaLevel_min = -9; // dBm
	rficPaLevel_max = 22; // dBm

	settings->isRficHPA = true;
	settings->externalPaGain = 0;
#endif
	rficPaLevel = rf_level_dBm - settings->externalPaGain;

	MGR_LOG_VERBOSE("[MCU_MISC] RFIC PA select (0:LPA, 1:HPA) = %d\r\n",
			settings->isRficHPA);
	MGR_LOG_VERBOSE("[MCU_MISC] RFIC PA level=%d dBm, external PA gain = %d dB\r\n",
			rficPaLevel,
			settings->externalPaGain);

	/* check settings */
	if ((rficPaLevel >= rficPaLevel_min) && (rficPaLevel <= rficPaLevel_max))
		settings->rficPaLevel = rficPaLevel;
	else
		return KNS_STATUS_BAD_SETTING;

	return KNS_STATUS_OK;
}

void MCU_MISC_TCXO_Force_State(bool enable)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    /* Get the Oscillators configuration according to the internal RCC registers */
    HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
    RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_HSE;

    if (enable) {
        RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_PWR;
    }
    else {
        RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
    }

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)  {
            Error_Handler();
    }
    return;
}
void MCU_MISC_TCXO_set_warmup(uint32_t time_ms) {
	tcxo_warmup_time_ms = time_ms;
    return;
}
void MCU_MISC_TCXO_get_warmup(uint32_t *time_ms) {
	*time_ms = tcxo_warmup_time_ms;
	return;
}

/**
 * @}
 */
