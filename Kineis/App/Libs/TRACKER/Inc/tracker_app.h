/* SPDX-License-Identifier: no SPDX license */
/**
 * @file   tracker_app.h
 * @brief  Library to handle tracker application
 * @author Arribada
 */

/**
 * @page tracker_app_page Tracker application library
 *
 * This page is presenting the Tracker app Library.
 *
 */

/**
 * @addtogroup TRACKER_APP
 * @brief  tracker app Library. (refer to \ref user_data page for general description).
 * @{
 */

#ifndef TRACKER_APP_H
#define TRACKER_APP_H

/* Includes ------------------------------------------------------------------------------------ */

#include <stdbool.h>
#include <stdint.h>
#include "kns_types.h"

/* Defines ------------------------------------------------------------------------------------- */

/* Enums --------------------------------------------------------------------------------------- */

/* Struct--------------------------------------------------------------------------------------- */
typedef struct {
    uint8_t u8_msg_counter;
    uint8_t u8_wait_msg_timer_s;
    uint8_t u8_wait_startup_restimer_min;
    uint8_t u8_wait_sequence_nmb_startup;
    uint8_t u8_bat_level_threshold;
} tracker_app_vars_t;
/* Externs ------------------------------------------------------------------------------------- */

/* Public functions ---------------------------------------------------------------------------- */
enum KNS_status_t TRACKER_init(void);
enum KNS_status_t TRACKER_start(void);
enum KNS_status_t TRACKER_stop(void);
enum KNS_status_t TRACKER_get_conf(tracker_app_vars_t *app_vars);
enum KNS_status_t TRACKER_set_conf(const tracker_app_vars_t *app_vars);


#endif /* __TRACKER_APP_H */

/**
 * @}
 */
