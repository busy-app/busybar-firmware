/***************************************************************************//**
 * @file
 * @brief Definitions for the Update TC Link Key plugin.
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

#ifndef SILABS_ZIGBEE_UPDATE_TC_LINK_KEY_H
#define SILABS_ZIGBEE_UPDATE_TC_LINK_KEY_H

/**
 * @defgroup update-tc-link-key Update TC Link Key
 * @ingroup component
 * @brief API and Callbacks for the Update TC Link Key Component
 *
 * This component provides the functionality to update the trust center
 * link key of a device on a Zigbee R21+ network. The device also requests
 * the trust center link key periodically if the trust center is pre-R21.
 * After the trust center is upgraded to R21+, if not already R21+, the periodic
 * request for a trust center link key update passes and the link key
 * is updated.
 *
 */

/**
 * @addtogroup update-tc-link-key
 * @{
 */

// -----------------------------------------------------------------------------
// Constants

#define SL_ZIGBEE_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME "Update TC Link Key"

/**
 * @name API
 * @{
 */

// -----------------------------------------------------------------------------
// API

/* @brief Start a link key update process.
 *
 * Kicks off a link key update process.
 *
 * @return An ::sl_status_t value. If the current node is not on a network,
 * this will return ::SL_STATUS_NOT_JOINED. If the current node is on a
 * distributed security network, this will return
 * ::SL_STATUS_INVALID_CONFIGURATION. If the current node is the
 * trust center, this will return ::SL_STATUS_INVALID_STATE.
 */
sl_status_t sl_zigbee_af_update_tc_link_key_start(void);

/* @brief Stop a link key update process.
 *
 * Stops a link key update process.
 *
 * @return Whether or not a TCLK update was in progress.
 */
bool sl_zigbee_af_update_tc_link_key_stop(void);

/** @brief Specify the amount of time to wait after one TCLK update operation
 *    completes before beginning a new TCLK update operation.
 *  @param timeInMilliseconds The time to wait after one TCLK update operation completes before issuing a new TCLK update operation.
 *    0 means stop updating the TCLK.
 *  @return None
 */
void sl_zigbee_af_set_tc_link_key_update_timer_ms(uint32_t timeInMilliseconds);

/** @brief Specify the amount of time between each retry after a failure TCLK update
 *  @param timeInMilliseconds The maximum amount of time between each retry after a failure TCLK update in milliseconds.
 *  @return None
 */
void sl_zigbee_af_set_tc_link_key_update_retry_backoff_timer_ms(uint32_t timeInMilliseconds);

/** @brief Specify the maximum number of Trust Center command exchange attempts made before concluding that the exchange has failed
 *  @param maxAttemptsPerIteration The maximum number of Trust Center command exchange attempts made before concluding that the exchange has failed.
 *    An attempt is considered as the initial try plus subsequent retries of each of the TCLK update command frames,
 *    which includes the Node Descriptor, Request Key, and Verify Key messages. For example, if a value of 3 is passed into this API,
 *    then the device will attempt up to 3 Node Descriptor messages, 3 Request Key attempts, and 3 Verify Key messages before concluding
 *    that the process is terminated with failure. This entire process is classified as one iteration.
 *  @return None
 */
void sl_zigbee_af_set_tc_link_key_update_max_attempts_per_iteration(uint8_t maxAttemptsPerIteration);

/** @brief Specify the maximum number of iterations of the TCLK update attempt permitted before a failure is declared using the On-Network TCLK Update procedure.
 *  @param maxIterations The maximum number of iterations of the TCLK update attempt permitted before a failure is declared using the On-Network TCLK Update procedure.
 *  @note sl_zigbee_af_update_tc_link_key_zigbee_key_establishment_cb is issued after max_attempts_per_iteration * max_iterations in the event of total failure.
 *  @return None
 */
void sl_zigbee_af_set_tc_link_key_update_max_iterations(uint8_t maxIterations);

/** @brief Update the TCLK now.
 *  @return SL_STATUS_IN_PROGRESS if TCLK update is already in progress, else SL_STATUS_OK
 */
sl_status_t sl_zigbee_af_tc_link_key_update_now(void);

/* @brief Set the delay until the next request is made to update the trust
 * center link key.
 */
void sl_zigbee_af_update_tc_link_key_set_delay(uint32_t delayMs);

/* @brief Stop the periodic TC link key update process.
 */
void sl_zigbee_af_update_tc_link_key_set_inactive(void);

/* @brief Returns true if the trust center link key (TCLK) on the device is either
 * the install-code derived key or the well-known key. This function is used to
 * detect whether the local device has updated its TCLK at least once.
 */
bool sl_zigbee_af_update_tc_link_key_is_tclk_key_default(void);

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup update_tc_link_key_cb Update TC Link Key Update
 * @ingroup af_callback
 * @brief Callbacks for Update TC Link Key Update Component
 *
 */

/**
 * @addtogroup update_tc_link_key_cb
 * @{
 */

/** @brief Status.
 *
 * This callback is fired when the Update Link Key exchange process is updated
 * with a status from the stack. Implementations will know that the Update TC
 * Link Key plugin has completed its link key request when the keyStatus
 * parameter is SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS.
 *
 * @param keyStatus An ::sl_zigbee_key_status_t value describing the success or failure
 * of the key exchange process. Ver.: always
 */
void sl_zigbee_af_update_tc_link_key_status_cb(sl_zigbee_key_status_t keyStatus);

/** @} */ // end of update_tc_link_key_cb
/** @} */ // end of name Callbacks
/** @} */ // end of update-tc-link-key

#endif // SILABS_ZIGBEE_UPDATE_TC_LINK_KEY_H
