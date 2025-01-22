// SPDX-License-Identifier: no SPDX license
/**
 * @file mgr_at_cmd_list.c
 * @author  Kineis
 * @brief AT commands list main file
 */

/**
 * @addtogroup MGR_SPI_CMD
 * @{
 */

/* Includes ------------------------------------------------------------------*/
#include "build_info.h"
#include "mgr_spi_cmd_common.h"
#include "mgr_spi_cmd_list.h"
#include "mgr_spi_cmd_list_general.h"
#include "mgr_spi_cmd_list_mac.h"
#include "mgr_spi_cmd_list_user_data.h"
#include "mgr_spi_cmd_list_previpass.h"
#include "mgr_spi_cmd_list_certif.h"

const uint8_t spicmd_version = 1;

/** @attention update AT cmd version above if you add or remove commands in this list */
const struct spicmd_desc_t cas_spicmd_list_array[SPICMD_MAX_COUNT] = {
	/**< General commands */
	{ CMD_NONE, CMD_NONE,			                NULL},
	{ CMD_READ, CMD_NONE,            			    bMGR_SPI_CMD_READ_cmd},
	{ CMD_PING, CMD_NONE,            			    bMGR_SPI_CMD_PING_cmd},
	{ CMD_MAC_STATUS, CMD_NONE,      			    bMGR_SPI_CMD_MACSTATUS_cmd},
	{ CMD_SPI_STATUS, CMD_NONE,      			    bMGR_SPI_CMD_SPISTATE_cmd},
	{ CMD_READ_VERSION, CMD_NONE,    			    bMGR_SPI_CMD_READSPIVERSION_cmd},
	{ CMD_READ_FIRMWARE, CMD_NONE,   			    bMGR_SPI_CMD_READFIRMWARE_cmd},
	{ CMD_READ_ADDR, CMD_NONE,       			    bMGR_SPI_CMD_READADDRESS_cmd},
	{ CMD_READ_ID, CMD_NONE,         			    bMGR_SPI_CMD_READID_cmd},
	{ CMD_READ_SN, CMD_NONE,         			    bMGR_SPI_CMD_READSN_cmd},
	{ CMD_READ_RCONF, CMD_NONE,      			    bMGR_SPI_CMD_READRCONF_cmd},
	{ CMD_WRITE_RCONF_REQ, CMD_WRITE_RCONF,         bMGR_SPI_CMD_WRITERCONFREQ_cmd},
	{ CMD_WRITE_RCONF, CMD_NONE,      			    bMGR_SPI_CMD_WRITERCONF_cmd},
	{ CMD_SAVE_RCONF, CMD_NONE,      			    bMGR_SPI_CMD_SAVERCONF_cmd},
	{ CMD_READ_KMAC, CMD_NONE,      			    bMGR_SPI_CMD_READKMAC_cmd},
	{ CMD_WRITE_KMAC_REQ, CMD_WRITE_KMAC,		    bMGR_SPI_CMD_WRITEKMACREQ_cmd},
	{ CMD_WRITE_KMAC, CMD_NONE,      			    bMGR_SPI_CMD_WRITEKMAC_cmd},
	{ CMD_READ_LPM, CMD_NONE,      				    bMGR_SPI_CMD_READLPM_cmd},
	{ CMD_WRITE_LPM_REQ, CMD_WRITE_LPM,             bMGR_SPI_CMD_WRITELPMREQ_cmd},
	{ CMD_WRITE_LPM, CMD_NONE,      			    bMGR_SPI_CMD_WRITELPM_cmd},
	{ CMD_WRITE_TX_REQ, CMD_WRITE_TX_SIZE,		    bMGR_SPI_CMD_WRITETXREQ_cmd},
	{ CMD_WRITE_TX_SIZE, CMD_WRITE_TX,              bMGR_SPI_CMD_WRITETXSIZE_cmd},
	{ CMD_WRITE_TX, CMD_NONE,      				    bMGR_SPI_CMD_WRITETX_cmd},
	{ CMD_READ_CW, CMD_NONE,            		    bMGR_SPI_CMD_READCW_cmd},
	{ CMD_WRITE_CW_REQ, CMD_WRITE_CW,               bMGR_SPI_CMD_WRITECWREQ_cmd},
	{ CMD_WRITE_CW, CMD_NONE,      				    bMGR_SPI_CMD_WRITECW_cmd},
	{ CMD_READ_PREPASSEN, CMD_NONE,				    bMGR_SPI_CMD_READPREPASSEN_cmd},
	{ CMD_WRITE_PREPASSEN_REQ, CMD_WRITE_PREPASSEN, bMGR_SPI_CMD_WRITEPREPASSENREQ_cmd},
	{ CMD_WRITE_PREPASSEN, CMD_NONE,      		    bMGR_SPI_CMD_WRITEPREPASSEN_cmd},
	{ CMD_READ_UDATE, CMD_NONE,         		    bMGR_SPI_CMD_READUDATE_cmd},
	{ CMD_WRITE_UDATE_REQ, CMD_WRITE_UDATE,         bMGR_SPI_CMD_WRITEUDATEREQ_cmd},
	{ CMD_WRITE_UDATE, CMD_NONE,      		        bMGR_SPI_CMD_WRITEUDATE_cmd}
};

/**
 * @}
 */
