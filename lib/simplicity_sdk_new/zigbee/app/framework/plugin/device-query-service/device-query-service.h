/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Device Query Service component.
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
 * @defgroup device-query-service Device Query Service
 * @ingroup component
 * @brief API and Callbacks for the Device Query Service Component
 *
 * This component queries new devices for their endpoints and clusters
 * to record information in the Device Database component. New devices are discovered
 * on receipt of a Device Announce frame or when discovery APIs are called below.
 */

/**
 * @addtogroup device-query-service
 * @{
 */

/**
 * @name API
 * @{
 */

/** @brief Enable or disable the device query service.
 *
 * @param[in] enable True to enable the service, false to disable.
 *
 * @return None
 */
void sl_zigbee_af_device_query_service_enable_disable(bool enable);

/** @brief Get the global enable state of the Device Query service.
 *
 * @return enable True if enabled, false if disabled.
 */
bool sl_zigbee_af_device_query_service_get_enabled_state(void);

/** @brief Get the EUI64 of current discovery target. The service queries one device at a time.
 *
 * @param[out] returnEui64 The EUI of the device in the network currently being queried.
 *
 * @return None
 *
 * @note When a device is fully discovered by this component, it marks the device as discovery-complete
 * in the Device Database component, which issues the ::sl_zigbee_af_device_database_discovery_complete_cb
 * callback to let the user application know.
 */
void sl_zigbee_af_device_query_service_get_current_discovery_target_eui64(sl_802154_long_addr_t returnEui64);

/** @brief Begin discovery of device. While the Device Query Service component automatically
 *  queues the discovery of a device based on reception of a Device Announce frame, this API
 *  is provided for user applications that wish to begin discovery of a target when desired.
 *  It is also worth noting that due to network limitations (e.g. network congestion, bandwidth),
 *  the local device may not register the Device Announce message from a particular node.
 *
 * @param[in] eui The EUI of the device to be queried.
 * @param[in] macCapabilities The 802.15.4 MAC capabilities of the device, if known. A value of 0xFF should be passed in if the capabilities are unknown when calling this function. The capabilities will be overwritten when a Node Descriptor Response is received.
 *
 * @return SL_STATUS_FULL if the Device Database is full, SL_STATUS_ALREADY_EXISTS if
 *  the device is already known, SL_STATUS_INVALID_CONFIGURATION if an rx-off-when-device device is specified
 *  but the configuration SL_ZIGBEE_AF_PLUGIN_DEVICE_QUERY_SERVICE_IGNORE_RX_OFF_WHEN_IDLE_DEVICES dictates
 *  such devices should be skipped, else SL_STATUS_OK.
 *
 * @note When a device is fully discovered by this component, it marks the device as discovery-complete
 * in the Device Database component, which issues the ::sl_zigbee_af_device_database_discovery_complete_cb
 * callback to let the user application know.
 */
sl_status_t sl_zigbee_af_device_query_service_discover_target(sl_802154_long_addr_t eui, uint8_t macCapabilities);

/** @} */ // end of name API
/** @} */ // end of device-query-service
