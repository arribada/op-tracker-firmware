// SPDX-License-Identifier: no SPDX license
/**
 * @file   mgr_at_cmd.c
 * @author Kineis
 * @brief  APIs Implementation used to parse the UART ASCII data stream and extract AT commands
 */

/**
 * @addtogroup MGR_AT_CMD
 * @{
 */


/* Includes -------------------------------------------------------------------------------------*/
#include <string.h>

#include "kns_types.h"
#include "mgr_at_cmd.h"
#include "mgr_at_cmd_common.h"
#include "mgr_at_cmd_list.h"
#include "mcu_at_console.h"
#include "kineis_sw_conf.h"
#include KINEIS_SW_ASSERT_H
#include "mgr_log.h"

/* Defines --------------------------------------------------------------------------------------*/

/** Define AT commands FIFO length. As long as sending AT+TX one by one, it is not that usefull to
 * enqueue AT commands so far. Later, this may be useful to increase FIFO in case application or
 * GUI is sending several commands in a raw.
 *
 * @attention AT CMD FIFO_MAX_SIZE must be superior or equal to 2
 *
 * @attention Ensure there is always one extra free space in fifo between write index and read
 *            index meaning that if you expect to store 3 AT cmd at maximum, FIFO size should be
 *            3+1 minimum if you want to avoid overflow.
 */
#define FIFO_MAX_SIZE                                   4
#ifdef USE_HDA4
#define FRAME_MAX_LEN                                   1280
#else
#define FRAME_MAX_LEN                                   128
#endif

#if FIFO_MAX_SIZE < 2
#error "FIFO_MAX_SIZE must be superior or equal to 2"
#endif

/* Structure Declaration ------------------------------------------------------------------------*/
struct atcmdfifo_t {
	uint8_t au8_fifo[FIFO_MAX_SIZE][FRAME_MAX_LEN];
	uint8_t u8_widx;
	uint8_t u8_ridx;
};

struct atcmd_info_t {
	enum atcmd_idx_t ATcmdIndex; /**< the At command Index */
	enum atcmd_type_t ATcmdExecType; /**< the At command typr */
};

/* Private variables ----------------------------------------------------------------------------*/

static struct atcmdfifo_t s_atcmdfifo; /**< A FIFO used to store AT commands received from UART */

/* Private functions ----------------------------------------------------------------------------*/

/**
 * @brief API used to extract the latest AT cmds from the incoming received data stream.
 *
 * This fct is looking for the start pattern ("AT+") and the end pattern ("\r\n") in the stream.
 * If several frames are received in a row, only the last AT cmd is extracted
 *
 * Once AT cmd is found, it is removed from the stream and stored into the FIFO if not full.
 *
 * As this fct is the entry point into FW, it is real-time critical and may be more robust. Some
 * design limitations are describerd below.
 *
 * @attention This fct may be called from ISR context
 *
 * @note Any garbage after the found AT cmd is also removed from stream. But the garbage before
 *       remains in the original stream.
 *
 * @param[in,out] pu8_RxBuffer pointer to start of RX buffer
 * @param[in,out] pi16_nbRxValidChar number of valid charecters in RX buffer
 *
 * @retval true if a valid frame was extracted, false if nothing was extracted
 */
static bool MGR_AT_CMD_parseStreamCb(uint8_t *pu8_RxBuffer, int16_t *pi16_nbRxValidChar)
{
	int16_t idxEnd = 0;
	int16_t idxStart = 0;
	int16_t i16_atcmdLen = 0;
	bool isEOLdetected = false;
	bool isFirstCharDetected = false;

	char C = (char)(pu8_RxBuffer[*pi16_nbRxValidChar - 1]);
	/* Process buffer only once termination caracter has been found */
	if (C != '\n')
	  return false;

	/* AT cmd starts with 'AT+' and ends with '\r\n'. Minimum packet length is ("AT+\r\n") */
	if (*pi16_nbRxValidChar < 5)
		return false;

	/* Search beginning of AT cmd from the end of the buffer */
	for (idxStart = *pi16_nbRxValidChar - 1; idxStart >= 0; idxStart--)
		if (pu8_RxBuffer[idxStart] == 'A' &&
		    pu8_RxBuffer[idxStart + 1] == 'T' &&
		    pu8_RxBuffer[idxStart + 2] == '+') {
			isFirstCharDetected = true;
			break;
		}

	/* Return if we can not find the beginning of the frame*/
	if (!isFirstCharDetected)
		return false;

	/* Find last characters of the frame : '\r\n' */
	for (idxEnd = idxStart + 2; idxEnd <= *pi16_nbRxValidChar; idxEnd++) {
		if (pu8_RxBuffer[idxEnd - 2] == '\r' && pu8_RxBuffer[idxEnd - 1] == '\n') {
			isEOLdetected = true;
			break;
		}
	}
	/* Return if we can not find the EOL characters */
	if (!isEOLdetected)
		return false;

	/* As some AT cmd was consummed from the end at idxStart position,
	 * idxStart characters remaing to be parsed at the beginning of the buffer.
	 * Assume they are all valid.
	 *
	 */
	*pi16_nbRxValidChar = idxStart;

	/* In case FIFO is full (read index is reached), skip this new AT CMD. We need to wait for
	 * FW to consume the previous AT CMDs before.
	 */
	if ( ((s_atcmdfifo.u8_widx+1) % FIFO_MAX_SIZE) == (s_atcmdfifo.u8_ridx % FIFO_MAX_SIZE))
		return true;
	/* Set the frame in the UART fifo */
	i16_atcmdLen = idxEnd - idxStart;
	/* Check AT cmd length overflow. Limit AT cmd len to maximum if overflow was
	 * detected (reserve last character for end-of-string'\0').
	 */
	if ((i16_atcmdLen + 1) > FRAME_MAX_LEN)
		i16_atcmdLen = FRAME_MAX_LEN - 1;
	memcpy(s_atcmdfifo.au8_fifo[s_atcmdfifo.u8_widx % FIFO_MAX_SIZE],
		&pu8_RxBuffer[idxStart], i16_atcmdLen);
	s_atcmdfifo.au8_fifo[s_atcmdfifo.u8_widx % FIFO_MAX_SIZE][i16_atcmdLen] = '\0';
	/* Increment write index to next free position */
	s_atcmdfifo.u8_widx++;

	/* In case read index is reached, skip oldest stored AT cmd to let a newest one to come */
	if (s_atcmdfifo.u8_widx % FIFO_MAX_SIZE == s_atcmdfifo.u8_ridx % FIFO_MAX_SIZE)
		kns_assert(0); // should no more happen
		// s_atcmdfifo.u8_ridx++;

	return true;
}

/**
 * @brief Identifies AT command and its type (AT+XYZ=... or AT+XYZ=? or AT+XYZ? mode)
 *
 * AT cmds with "=?\r\n" (e.g. "AT+ID=?\r\n") is a status-type command. It is used to get some
 * information from the device.
 *
 * AT cmds without pattern above is an action-type command. It is used to set some information into
 * the device (e.g. "AT+TX=<...>\r\n").
 *
 * @note This fct may be called from ISR context
 *
 * @param[in] pu8_atcmd pointer to the AT command
 *
 * @returns AT cmd info (e.g. index and execution type)
 */
static struct atcmd_info_t MGR_AT_CMD_getAtType(uint8_t *pu8_atcmd)
{
	uint8_t __u8_k;
	uint8_t *p_atcmd_start;
	uint8_t u8foundCmdIndex = ATCMD_UNKNOWN_COMMAND;
	uint16_t u16_offset_search = 0;
	struct atcmd_info_t atcmd_info;


	atcmd_info.ATcmdIndex = u8foundCmdIndex;

	/* Loop over table and check for match */
	for (__u8_k = 0; __u8_k < ATCMD_MAX_COUNT; __u8_k++) {
		u16_offset_search = cas_atcmd_list_array[__u8_k].u8_cmdNameLen;

		if (bUTIL_strcmp((uint8_t *)cas_atcmd_list_array[__u8_k].pu8_cmdNameString,
					pu8_atcmd,
					u16_offset_search)) {
			u8foundCmdIndex = (enum atcmd_idx_t)__u8_k;
			break;
		}
	}
	/* If command is found in list, check type */
	if (u8foundCmdIndex < ATCMD_UNKNOWN_COMMAND) {
		u16_offset_search = cas_atcmd_list_array[u8foundCmdIndex].u8_cmdNameLen;
		p_atcmd_start = pu8_atcmd;

		/* Checks if next characters after command are either "=?\r", "=?\n" or
		 * "=<params>\r" or "=<params>\n"
		 *
		 * @note "AT+<cmd>\r\n" is considered as action mode. it will be filtered in the
		 * next parsing level.
		 */
		if ((p_atcmd_start[u16_offset_search] == '=')
				&& (p_atcmd_start[u16_offset_search+1] == '?')
				&& ((p_atcmd_start[u16_offset_search+2] == '\r')
					|| (p_atcmd_start[u16_offset_search+2] == '\n'))) {
			atcmd_info.ATcmdExecType = ATCMD_STATUS_MODE;
			atcmd_info.ATcmdIndex = u8foundCmdIndex;
		} else if ((p_atcmd_start[u16_offset_search] == '=')
				|| (p_atcmd_start[u16_offset_search] == '\r')
				|| (p_atcmd_start[u16_offset_search] == '\n')) {
			atcmd_info.ATcmdExecType = ATCMD_ACTION_MODE;
			atcmd_info.ATcmdIndex = u8foundCmdIndex;
		} else
			atcmd_info.ATcmdIndex = ATCMD_UNKNOWN_COMMAND;
	}
	return atcmd_info;
}

/* Functions ------------------------------------------------------------------------------------*/

bool MGR_AT_CMD_start(void *context)
{
	return MCU_AT_CONSOLE_register(context, MGR_AT_CMD_parseStreamCb);

}

bool MGR_AT_CMD_isPendingAt(void)
{
	return (s_atcmdfifo.u8_ridx % FIFO_MAX_SIZE != s_atcmdfifo.u8_widx % FIFO_MAX_SIZE);
}

uint8_t *MGR_AT_CMD_popNextAt(void)
{
	if (s_atcmdfifo.u8_ridx % FIFO_MAX_SIZE != s_atcmdfifo.u8_widx % FIFO_MAX_SIZE)
		return s_atcmdfifo.au8_fifo[s_atcmdfifo.u8_ridx++ % FIFO_MAX_SIZE];
	else
		return NULL;
}

bool MGR_AT_CMD_decodeAt(uint8_t *pu8_atcmd)
{
	bool status = false;
	struct atcmd_info_t atcmdInfo;

	if (pu8_atcmd != NULL) {
		atcmdInfo = MGR_AT_CMD_getAtType(pu8_atcmd);

		if ((atcmdInfo.ATcmdIndex < ATCMD_UNKNOWN_COMMAND) &&
		    (cas_atcmd_list_array[atcmdInfo.ATcmdIndex].f_ht_cmd_fun_proc != NULL))
			status = (cas_atcmd_list_array[atcmdInfo.ATcmdIndex].f_ht_cmd_fun_proc)(
					pu8_atcmd,
					atcmdInfo.ATcmdExecType);
		else {
			bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN_AT_CMD);
			MGR_LOG_VERBOSE("[ERROR] Unknown AT command\r\n");
		}
	} else {
		bMGR_AT_CMD_logFailedMsg(ERROR_UNKNOWN);
		MGR_LOG_DEBUG("[ERROR] input param is Nil\r\n");
	}
	return status;
}

__attribute((__weak__))
enum KNS_status_t MGR_AT_CMD_macEvtProcess(void)
{
	/** Empty weak core, can be overwritten, depending on AT cmd processed */
	return KNS_STATUS_OK;
}

/**
 * @}
 */
