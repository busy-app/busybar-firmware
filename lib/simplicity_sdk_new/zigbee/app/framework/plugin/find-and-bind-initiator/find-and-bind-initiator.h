/***************************************************************************//**
 * @file
 * @brief Initiator APIs and defines for the Find and Bind Initiator plugin.
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

#ifndef SILABS_FIND_AND_BIND_INITIATOR_H
#define SILABS_FIND_AND_BIND_INITIATOR_H

/**
 * @defgroup find-and-bind-initiator Find and Bind Initiator
 * @ingroup component
 * @brief API and Callbacks for the Find and Bind Initiator Component
 *
 * This component  provides the functionality to find devices that are
 * performing the find and bind process for a target and then optionally
 * attempts to create a binding to those devices.
 *
 */

/**
 * @addtogroup find-and-bind-initiator
 * @{
 */

// -----------------------------------------------------------------------------
// Constants

#define SL_ZIGBEE_AF_PLUGIN_FIND_AND_BIND_INITIATOR_PLUGIN_NAME "Find and Bind Initiator"

// -----------------------------------------------------------------------------
// API

/**
 * @name API
 * @{
 */

/** @brief Start initiator finding and binding operations.
 *
 * A call to this function will commence the initiator finding and
 * binding operations. Specifically, the initiator will attempt to start
 * searching for potential bindings by broadcasting Identify Query commands,
 * collecting responses from targets, and attempting to create bindings for matching clusters.
 *
 * @param[in] endpoint The endpoint on which to begin initiator operations.
 *
 * @return An ::sl_status_t value describing the success of the
 * commencement of the initiator operations.
 */
sl_status_t sl_zigbee_af_find_and_bind_initiator_start(uint8_t endpoint);

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup find_bind_initiator_cb Find and Bind Initiator
 * @ingroup af_callback
 * @brief Callbacks for Find and Bind Initiator Component
 *
 */

/**
 * @addtogroup find_bind_initiator_cb
 * @{
 */

/** @brief Callback fired when a potential bind target has been found.
 *
 * This callback with enable the user to programmatically decide if they want
 * to bind with a potential target. The plugin will try to bind with this
 * target if and only if the function returns true. By default, the callback
 * tells the plugin to try to bind with the target. If the binding type is
 * changed to ::SL_ZIGBEE_MULTICAST_BINDING, a multicast binding will be created.
 *
 * @param[out] nodeId short ID of the potential target Ver.: always
 * @param[out] bindingEntry The binding entry for that target. Ver.: always
 * @param[out] groupName The name of the group if a multicast binding is created.
 * Ver.: always
 * @return Whether or not to bind to target. Defaults to true.
 */
bool sl_zigbee_af_find_and_bind_initiator_bind_target_cb(sl_802154_short_addr_t nodeId,
                                                         sl_zigbee_binding_table_entry_t *bindingEntry,
                                                         uint8_t *groupName);

/** @brief Find and Bind Complete
 *
 * This callback is fired by the initiator when the Find and Bind process is
 * complete.
 *
 * @param[out] status Status code describing the completion of the find and bind
 * process Ver.: always
 */
void sl_zigbee_af_find_and_bind_initiator_complete_cb(sl_status_t status);
/** @} */ // end of find_bind_initiator_cb
/** @} */ // end of name Callbacks
/** @} */ // end of find-and-bind-initiator

#endif /* __FIND_AND_BIND_INITIATOR_H__ */
