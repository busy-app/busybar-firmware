/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Polling plugin.
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

#ifndef SL_ZIGBEE_END_DEVICE_SUPPORT_H
#define SL_ZIGBEE_END_DEVICE_SUPPORT_H

/**
 * @defgroup end-device-support  End Device Support
 * @ingroup component
 * @brief API and Callbacks for the End Device Support Component
 *
 * Silicon Labs implementation of polling for end devices.
 * Note that the Poll Control Server component sets the long and short
 * poll intervals at startup.  If both components are enabled, the
 * long and short poll intervals set here will not be used.
 *
 */

/**
 * @addtogroup end-device-support
 * @{
 */

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_END_DEVICE_SUPPORT_PRESENT
#include "end-device-support-config.h"
#else
#include "config/end-device-support-config.h"
#endif

#if (SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_ENABLE_POLL_COMPLETED_CALLBACK == 1)
#define ENABLE_POLL_COMPLETED_CALLBACK
#endif
#if (SL_ZIGBEE_AF_PLUGIN_END_DEVICE_SUPPORT_ALLOW_REJOINS_WITH_WELL_KNOWN_LINK_KEY == 1)
#define ALLOW_REJOINS_WITH_WELL_KNOWN_LINK_KEY
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern bool sli_zigbee_af_enable_poll_completed_callback;

typedef struct {
  uint32_t pollIntervalMs;
  uint8_t numPollsFailing;
} sli_zigbee_af_polling_state;

extern sli_zigbee_af_polling_state sli_zigbee_af_polling_states[];
#endif //DOXYGEN_SHOULD_SKIP_THIS

/**
 * @name Handlers
 * @{
 */

/** @brief End Device Support Move Network Event Handler.
 *
 * This API handles the stack logic when there is a Move Network Event Scheduled.
 * This handler will attempt to call: sl_zigbee_find_and_rejoin_network to move networks.
 * This handler will either be completed upon a successful move or
 * upon failure will reschedule a new move network event until a maximum number of retry events.
 *
 * @param[in] event a type sl_zigbee_af_event_t
 *
 */
void sl_zigbee_af_end_device_support_move_network_event_handler(sl_zigbee_af_event_t * event);

/** @} */ // end of name Handlers

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup end_device_support_cb End Device Support
 * @ingroup af_callback
 * @brief Callbacks for End Device Support Component
 *
 */

/**
 * @addtogroup end_device_support_cb
 * @{
 */

/** @brief Poll is completed.
 *
 * This function is called by the End Device Support plugin after a poll is
 * completed.
 *
 * @param[out] status Return status of a completed poll operation Ver.: always
 */
void sl_zigbee_af_end_device_support_poll_completed_cb(sl_status_t status);

/** @brief Parent connectivity is lost.
 *
 * This function is called by the End Device Support plugin when the end device
 * can't connect to its parent. This callback is called under the following
 * conditions:
 * - When the network state of the device is SL_ZIGBEE_JOINED_NETWORK_NO_PARENT.
 * - If there are 3 or more data poll transaction failures for an end device.
 *
 * Users can consume this callback and return either true or false depending on desired behavior.
 *
 * @return Type boolean value:
 * - True: End Device Support Plugin will Not Send Trust Center Rejoin
 * - False: End Device Support Plugin will send Trust Center Rejoin
 */
bool sl_zigbee_af_end_device_support_lost_parent_connectivity_cb(void);

/** @brief Pre network move.
 *
 * This function is called by the End Device Support plugin just before it
 * issues a network move, or a sequence of a secure rejoin followed by several
 * trust center rejoins. Various plugins issue a network move when certain
 * conditions are met, such as when the network state is EMBER_JOINED_NO_PARENT.
 *
 * Users can consume this callback and return true to prevent the End Device
 * Support plugin from issuing rejoin requests. Consuming the callback also
 * gives the application a chance to implement its own rejoin logic. If this
 * callback returns false, the End Device Support plugin will issue network
 * rejoins as usual.
 *
 * @return Type boolean value:
 * - True: Prevent the End Device Support Plugin to issue Rejoin Request
 * - False: Allow End Device Support Plugin to issue Rejoin Request
 */
bool sl_zigbee_af_end_device_support_pre_network_move_cb(void);
/** @} */ // end of end_device_support_cb
/** @} */ // end of name Callbacks
/** @} */ // end of end-device-support

void sli_zigbee_af_poll_complete_handler(sl_status_t status, uint8_t limit);
void sli_zigbee_af_end_device_support_move_init(uint8_t init_level);
void sli_zigbee_af_end_device_support_polling_init(uint8_t init_level);

#endif // SL_ZIGBEE_END_DEVICE_SUPPORT_H
