/* SPDX-License-Identifier: no SPDX license */
/**
 * @file mgr_at_cmd_common.h
 * @author Kineis
 * @brief header file for common part of the AT cmd manager
 */

/**
 * @addtogroup MGR_AT_CMD
 * @{
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MGR_AT_CMD_COMMON_H
#define __MGR_AT_CMD_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "strutil_lib.h"
#include "kineis_sw_conf.h" // get ERROR_RETURN_T types

/* Defines ------------------------------------------------------------------*/

enum atcmd_type_t {
	ATCMD_ACTION_MODE,
	ATCMD_STATUS_MODE,
	ATCMD_INVALID_USAGE
};

/** enum of command response */
enum atcmd_rsp_type_t {
	ATCMD_RSP_TXOK,		    /**< At command delayed response for TX done */
	ATCMD_RSP_TXACKOK,	    /**< At command delayed response for TX done */
	ATCMD_RSP_TXACKNOTOK,	/**< At command delayed response for TX timeout */
	ATCMD_RSP_SATDET,	    /**< At command delayed response for SAT detection */
	ATCMD_RSP_SATLOST,	    /**< At command delayed response for SAT lost */
	ATCMD_RSP_SATDETTO,	    /**< At command delayed response for SAT detection timeout */
	ATCMD_RSP_RXOK,		    /**< At command delayed response for RX frame reception */
	ATCMD_RSP_DLOK,		    /**< At command delayed response for RX frame reception */
	ATCMD_RSP_RXTIMEOUT,    /**< At command delayed response for RX timeout */
	ATCMD_RSP_TXTIMEOUT,    /**< At command delayed response for TX timeout */
	ATCMD_RSP_RXERROR  	    /**< At command delayed response for RX error */
};


/* functions prototypes -----------------------------------------------------*/

/** @brief : Log in debug interface a succeed message
 *
 * @return : always return true
 */
bool bMGR_AT_CMD_logSucceedMsg(void);

/** @brief Log in debug interface a failed message
 *
 * @param[in] eErrorType : the error type
 *
 * @return : always return false
 */
bool bMGR_AT_CMD_logFailedMsg(enum ERROR_RETURN_T eErrorType);

/** @brief : This function writes in UART the response of specific command
 *
 * @param[in] atcmd_response_type : enum ATCMD_RESPONSE_TYPE_T: the type of response
 * @param[in] atcmd_rsp_data : pointer the at command response data
 *
 * @return true if command is correctly executed  false if error
 */
bool bMGR_AT_CMD_sendResponse(enum atcmd_rsp_type_t atcmd_response_type, void *atcmd_rsp_data);

#endif /* __MGR_AT_CMD_COMMON_H */

/**
 * @}
 */
