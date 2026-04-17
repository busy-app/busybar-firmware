/***************************************************************************//**
 * @file
 * @brief Definitions for the Update App Link Key plugin.
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

#ifndef SILABS_ZIGBEE_UPDATE_APP_LINK_KEY_H
#define SILABS_ZIGBEE_UPDATE_APP_LINK_KEY_H

/**
 * @defgroup update-app-link-key Update App Link Key
 * @ingroup component
 * @brief API and Callbacks for the Update App Link Key Component
 *
 * This component provides the functionality to retrieve an APS link key with
 * a partner, non-trust center device. The trust center brokers the key between
 * the two devices.
 *
 */

/**
 * @addtogroup update-app-link-key
 * @{
 */

// -----------------------------------------------------------------------------
// Constants

#define SL_ZIGBEE_AF_PLUGIN_UPDATE_APP_LINK_KEY_PLUGIN_NAME "Update App Link Key"

/**
 * @name API
 * @{
 */

// -----------------------------------------------------------------------------
// API

/* @brief Request an application link key with the partner device.
 *
 * @param eui The partner to start a partner link key exchange with. In R21 and
 * future revisions, the trust center acts as the broker between the two partner
 * devices. The partner argument here is not the trust center, but a non-trust
 * center device that the local device wishes to establish a link key with.
 *
 * @return An ::sl_status_t value. If the current node is not on a network,
 * this will return ::SL_STATUS_NOT_JOINED. If the current node is on a
 * distributed security network, this will return
 * ::SL_STATUS_INVALID_CONFIGURATION. If the current node is the
 * trust center, this will return ::SL_STATUS_INVALID_STATE.
 *
 * @note Only one partner link key operation can occur at a time, which means
 * that after calling this API, the user must wait for
 * ::sl_zigbee_af_update_app_link_key_status_cb to be invoked before starting
 * a new operation.
 */
sl_status_t sl_zigbee_af_update_app_link_key(sl_802154_long_addr_t eui);

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup update_app_link_key_cb Update App Link Key Update
 * @ingroup af_callback
 * @brief Callbacks for Update App Link Key Update Component
 *
 */

/**
 * @addtogroup update_app_link_key_cb
 * @{
 */

/** @brief This callback is fired when the Update Link Key exchange process is updated
 * with a status from the stack. Implementations will know that the Update App
 * Link Key plugin has completed its link key request when the keyStatus
 * parameter is SL_ZIGBEE_VERIFY_LINK_KEY_SUCCESS or an error code.
 *
 * @param keyStatus An ::sl_zigbee_key_status_t value describing the success or failure
 * of the key exchange process. Ver.: always
 */
void sl_zigbee_af_update_app_link_key_status_cb(sl_zigbee_key_status_t keyStatus);

/** @} */ // end of update_app_link_key_cb
/** @} */ // end of name Callbacks
/** @} */ // end of update-app-link-key

#endif // SILABS_ZIGBEE_UPDATE_APP_LINK_KEY_H
