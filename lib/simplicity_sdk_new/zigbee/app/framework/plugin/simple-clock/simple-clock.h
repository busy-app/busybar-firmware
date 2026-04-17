/***************************************************************************//**
 * @file
 * @brief Definitions for the Simple Clock plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * @defgroup simple-clock Simple Clock
 * @ingroup component
 * @brief API and Callbacks for the Simple Clock Component
 *
 * Silicon Labs implementation of a simple clock.  The component provides time
 * keeping functionality to the framework and the application through
 * implementations of both sl_zigbee_af_get_current_time_cb and sl_zigbee_af_set_time_cb.
 * When the application knows the current time (e.g., by reading the time
 * attribute from a time server), it should call sl_zigbee_af_set_time to keep the
 * component synchronized.  The component is not a substitute for an accurate clock
 * and is only intended for use during development.  Before production, this
 * component should be replaced with code that provides more accurate time.
 *
 */

/**
 * @addtogroup simple-clock
 * @{
 */

typedef enum {
  SL_ZIGBEE_AF_SIMPLE_CLOCK_NEVER_UTC_SYNC = 0,
  SL_ZIGBEE_AF_SIMPLE_CLOCK_STALE_UTC_SYNC = 1,
  SL_ZIGBEE_AF_SIMPLE_CLOCK_UTC_SYNCED = 2,
} sl_zigbee_af_plugin_simple_clock_time_sync_status_t;

/**
 * @name API
 * @{
 */

/** @brief Set the time and notes it as synchronized with UTC.
 * The Sync status will be set to SL_ZIGBEE_AF_SIMPLE_CLOCK_UTC_SYNCED.
 * @param utcTimeSeconds UTC time in seconds
 */
void sl_zigbee_af_simple_clock_set_utc_synced_time(uint32_t utcTimeSeconds);

/** @brief Retrieve the status of the simple-clock and whether it has been
 * synchronized with UTC via a previous call to sl_zigbee_af_simple_clock_set_utc_synced_time().
 *
 * @return sl_zigbee_af_plugin_simple_clock_time_sync_status_t Simple time sync status
 *
 */
sl_zigbee_af_plugin_simple_clock_time_sync_status_t sl_zigbee_af_simple_clock_get_time_sync_status(void);

/** @brief Retrieve the current time in seconds, and any millisecond remainder is returned
 * in the passed pointer to the milliseconds value.
 *
 * @param millisecondsRemainderReturn Pointer to variable where the remainder milliseconds will be stored
 *
 * @return uint32_t Current time in seconds
 *
 */
uint32_t sl_zigbee_af_get_current_time_seconds_with_ms_precision(uint16_t* millisecondsRemainderReturn);

/** @} */ // end of name API
/** @} */ // end of simple-clock
