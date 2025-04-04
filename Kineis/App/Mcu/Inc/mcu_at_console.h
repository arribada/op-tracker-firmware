/* SPDX-License-Identifier: no SPDX license */
/**
 * @file    mcu_at_console.h
 * @author  Kinéis
 * @brief   MCU wrapper for AT CMD console
 */

/**
 * @page mcu_at_console_page MCU wrapper: AT CMD console
 *
 * This console is used to communicate At commands and responses with the device.
 * Depending on your application need, it could be based on UART link, I2c and so on.
 *
 * **Reception**
 *
 * On reception side, this wrapper should handle incoming ASCII character as a data stream.
 * The client of this wrapper should register a callback which will be invoked each time a new
 * character is received.
 *
 * **Transmission**
 *
 * On transmission side, it is possible to send strings on same base as classic printf fucntion.
 * A specific API is also available to send the content of a binary buffer. It shold convert binary
 * data into ASCII printable strings.
 */

/**
 * @addtogroup MCU_APP_WRAPPERS
 * @brief MCU wrapper used by Kineis Application example.
 *
 * One has to implement API as per its microcontroller and its platform ressources.
 * @{
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MCU_AT_CONSOLE_H
#define __MCU_AT_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Functions prototypes ------------------------------------------------------*/

/** @brief Start AT CMD console for AT cmd reception
 *
 * It registers client's callback used to treat the received data stream. The callback will be
 * called each a new character is received.
 *
 * @attention This callback may be called under interrupt context. thus, it must not last too long.
 *
 * The format of client's callback is:
 @verbatim
bool (*rx_evt_cb)(uint8_t *pu8_RxBuffer, int16_t *pi16_nbRxValidChar));
 @endverbatim
 * At each received character, the callback is said to consume data, it updates the buffer pointer
 * and the number-of-valid-char reduced by what was consummed.
 * * [in/out] pu8_RxBuffer pointer to the data (update internally if some characters are consummed
 * * [in/out] pi16_nbRxValidChar number of valid/remaining char
 *
 * @param[in] context pointer which may be usefull to configure console (e.g. UART handle)
 * @param[in] rx_evt_cb pointer to callback invoked at each RX event, i.e. at each received char
 * @return true if console is correcly started, false otherwise.
 */
bool MCU_AT_CONSOLE_register(void *context,
		bool (*rx_evt_cb)(uint8_t *pu8_RxBuffer, int16_t *pi16_nbRxValidChar));

/** @brief Send AT CMD response to console
 *
 * This function is used to print AT command's response on AT console.
 * The prototype is same as printf. Depending on your needs, this function may only implement
 * limited functionnality of printf (`man 3 printf` onlinux to get detailled description).
 *
 * @note This could be redirected to printf if available in your application
 *
 * @param[in] format refer to printf manual pages
 */
void MCU_AT_CONSOLE_send(const char *format, ...);

/** @brief Write content of a binary data buffer as AT cmd response
 *
 * @param[in] pu8_inDataBuff: pointer to data buffer
 * @param[in] u16_dataLenBit: length of data buffer in bit
 */
void MCU_AT_CONSOLE_send_dataBuf(uint8_t *pu8_inDataBuff, uint16_t u16_dataLenBit);

void MCU_UART_DeInit();
#endif /* __MCU_AT_CONSOLE_H */

/**
 * @}
 */
