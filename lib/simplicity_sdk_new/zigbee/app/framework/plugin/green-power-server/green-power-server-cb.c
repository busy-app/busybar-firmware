/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "app/framework/include/af.h"
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-adapter.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-server.h"

/** @brief Green power commissioning notification callback
 *
 * This function is called by the green power server plugin to notify the
 * application about gp commissioning notification received by the sink.
 * If the this function returns false indicating application does not handle
 * the notification, the plugin will handle. If this returns true, the plugin
 * will skip processing of the commissioning notification.
 *
 * @param commandId                    Ver.: always
 * @param commNotificationOptions      Ver.: always
 * @param gpdAddr                      Ver.: always
 * @param gpdSecurityFrameCounte       Ver.: always
 * @param gpdCommandId                 Ver.: always
 * @param gpdCommandPayload            Ver.: always
 * @param gppShortAddress              Ver.: always
 * @param rssi                         Ver.: always
 * @param linkQuality                  Ver.: always
 * @param gppDistance                  Ver.: always
 * @param commissioningNotificationMic Ver.: always
 *
 * @returns true if application handles it and plugin will not process it anymore.
 * else return false to process the notification by the plugin  Ver.: always
 */
WEAK(bool sl_zigbee_af_green_power_server_gpd_commissioning_notification_cb(uint8_t commandId,
                                                                            uint16_t commNotificationOptions,
                                                                            sl_zigbee_gp_address_t *gpdAddr,
                                                                            uint32_t gpdSecurityFrameCounter,
                                                                            uint8_t gpdCommandId,
                                                                            uint8_t* gpdCommandPayload,
                                                                            uint16_t gppShortAddress,
                                                                            int8_t rssi,
                                                                            uint8_t linkQuality,
                                                                            uint8_t gppDistance,
                                                                            uint32_t commissioningNotificationMic))
{
  UNUSED_VAR(commandId);
  UNUSED_VAR(commNotificationOptions);
  UNUSED_VAR(gpdAddr);
  UNUSED_VAR(gpdSecurityFrameCounter);
  UNUSED_VAR(gpdCommandId);
  UNUSED_VAR(gpdCommandPayload);
  UNUSED_VAR(gppShortAddress);
  UNUSED_VAR(rssi);
  UNUSED_VAR(linkQuality);
  UNUSED_VAR(gppDistance);
  UNUSED_VAR(commissioningNotificationMic);
  return false;
}

/** @brief Green power server commissioning callback
 *
 * This function is called by the green power server plugin to notify the
 * application of a Green Power Device that has requested commissioning with
 * this sink. Returns false if callback is not handled, true if callback is handled.
 * When the callback is handled, it must set the matchFound argument appropriately
 * to indicate if the matching functionality is found on the sink or not.
 *
 * @param appInfo   Application information of the commissioning GPD. Ver.: always
 * @param matchFound Output flag to notify matching functionality. Ver.: always
 *
 * @returns true if application handled it Ver.: always
 */
WEAK(bool sl_zigbee_af_green_power_server_gpd_commissioning_cb(sl_zigbee_gp_application_info_t * appInfo,
                                                               bool* matchFound))
{
  UNUSED_VAR(appInfo);
  bool handled = false;
  // When this is implemented by the user :
  // iterate through the sink commissioning endpoints to process the matching of the
  // cluster functionality based on the gpd application information and set the supplied
  // matchFound flag.
  *matchFound = false;
  return handled;
}

/** @brief Green power server security failure callback
 *
 * This function is called by the green power server plugin to notify the
 * application of a Green Power Security Processing failed for an incoming notification.
 *
 * @param gpdAddr   Ver.: always
 */
WEAK(void sl_zigbee_af_green_power_server_gpd_security_failure_cb(sl_zigbee_gp_address_t *gpdAddr))
{
  UNUSED_VAR(gpdAddr);
}

/** @brief Green power server notification forward callback
 *
 * This function is called by the green power server plugin to notify the
 * application of a Green Power Gp Notification of an incoming gpd command.
 * Return true to handle in application.
 *
 * @param options from the incoming Gp Notification Command Ver.: always
 * @param addr GPD address        Ver.: always
 * @param gpdSecurityFrameCounter Ver.: always
 * @param gpdCommandId            Ver.: always
 * @param gpdCommandPayload first byte is length of the payload Ver.: always
 * @param gppShortAddress         Ver.: always
 * @param gppDistance             Ver.: always
 *
 * @returns true if application handled it and plugin will not process it anymore.
 * else return false to process the notification by the plugin  Ver.: always
 */
WEAK(bool sl_zigbee_af_green_power_cluster_gp_notification_forward_cb(uint16_t options,
                                                                      sl_zigbee_gp_address_t * addr,
                                                                      uint32_t gpdSecurityFrameCounter,
                                                                      uint8_t gpdCommandId,
                                                                      uint8_t * gpdCommandPayload,
                                                                      uint16_t gppShortAddress,
                                                                      uint8_t  gppDistance))
{
  UNUSED_VAR(options);
  UNUSED_VAR(addr);
  UNUSED_VAR(gpdSecurityFrameCounter);
  UNUSED_VAR(gpdCommandId);
  UNUSED_VAR(gpdCommandPayload);
  UNUSED_VAR(gppShortAddress);
  UNUSED_VAR(gppDistance);
  return false;
}

/** @brief Green power server pairing complete callback
 *
 * This function is called by the Green Power Server upon the completion of the pairing
 * and to indicate the closure of the pairing session.
 *
 * @param numberOfEndpoints number of sink endpoints participated in the pairing Ver.: always
 * @param endpoints list of sink endpoints Ver.: always
 */
WEAK(void sl_zigbee_af_green_power_server_pairing_complete_cb(uint8_t numberOfEndpoints,
                                                              uint8_t * endpoints))
{
  UNUSED_VAR(numberOfEndpoints);
  UNUSED_VAR(endpoints);
}

/** @brief Green power server commissioning timeout callback
 *
 * This function is called by the Green Power Server upon expiry of any of the
 * commissioning timer , those are server commissioning window expiry, generic
 * switch commissioning or multi-sensor commissioning timer expiry.
 *
 * @param commissioningTimeoutType one of the types
 *                     COMMISSIONING_TIMEOUT_TYPE_COMMISSIONING_WINDOW_TIMEOUT,
 *                     COMMISSIONING_TIMEOUT_TYPE_GENERIC_SWITCH
 *                     or COMMISSIONING_TIMEOUT_TYPE_MULTI_SENSOR Ver.: always
 * @param numberOfEndpoints Number of sink endpoints participated in the commissioning Ver.: always
 * @param endpoints list of sink endpoints Ver.: always
 */
WEAK(void sl_zigbee_af_green_power_server_commissioning_timeout_cb(uint8_t commissioningTimeoutType,
                                                                   uint8_t numberOfEndpoints,
                                                                   uint8_t * endpoints))
{
  UNUSED_VAR(commissioningTimeoutType);
  UNUSED_VAR(numberOfEndpoints);
  UNUSED_VAR(endpoints);
}

/** @brief Green power server update involved TC callback
 *
 * This function is called by the Green Power Server to proceed with updating the InvolveTC bit
 * of the security level attribute.
 *
 * @param status   Ver.: always
 *
 * @returns true if application handled it and plugin will not process it  Ver.: always
 */
WEAK(bool sl_zigbee_af_green_power_server_update_involve_t_c_cb(sl_status_t status))
{
  UNUSED_VAR(status);

  bool handled = false;
  return handled;
}

/** @brief Green power cluster GP Pairing Search callback
 *
 * This function is called to allow application to respond to the GP pairing search
 * request.
 *
 * @param options   Ver.: since gp-1.0-09-5499-24
 * @param gpdSrcId   Ver.: since gp-1.0-09-5499-24
 * @param gpdIeee   Ver.: since gp-1.0-09-5499-24
 * @param endpoint   Ver.: always
 *
 * @returns Return true if the response to the command is handled.
 */
WEAK(bool sl_zigbee_af_green_power_cluster_gp_pairing_search_cb(uint16_t options,
                                                                uint32_t gpdSrcId,
                                                                uint8_t* gpdIeee,
                                                                uint8_t endpoint))
{
  // Returns true if handled else return false for a default response
  // to be generated by application framework cluster command handler
  UNUSED_VAR(options);
  UNUSED_VAR(gpdSrcId);
  UNUSED_VAR(gpdIeee);
  UNUSED_VAR(endpoint);
  return false;
}

/** @brief Sink table access notification callback
 *
 * This function is called by the green power server plugin to notify the
 * application about Green Power Device addition or removal by the green power
 * server to the Sink Table. If returned false, the sink table remains un-accessed.
 *
 * @param data void pointer to the data Ver.: always
 * @param accessType accessType         Ver.: always
 *
 * @returns true if the access is granted.
 */
WEAK(bool  sl_zigbee_af_green_power_server_sink_table_access_notification_cb(void* data,
                                                                             sl_zigbee_af_gp_server_sink_table_access_type_t accessType))
{
  // The data is a void pointer must be typecast to required type based on the access type for correct access.
  // Example :
  // if accessType is GREEN_POWER_SERVER_SINK_TABLE_ACCESS_TYPE_REMOVE_GPD
  //                   or GREEN_POWER_SERVER_SINK_TABLE_ACCESS_TYPE_ADD_GPD
  // The data must be typecast to (sl_zigbee_gp_address_t *) e.g. and access
  //      sl_zigbee_gp_address_t * addr = (sl_zigbee_gp_address_t *) data;
  UNUSED_VAR(data);
  UNUSED_VAR(accessType);
  return true;
}

/** @brief Sink commissioning enter call status notification callback
 *
 * This function is called by the green power server plugin from the sink
 * commissioning enter command to notify the application of the status of the
 * proxy commissioning enter message submission to network layer.
 *
 * @param commissioningState Sink commissioning state Ver.: always
 * @param apsFrame aps frame header        Ver.: always
 * @param messageType unicast or broadcast Ver.: always
 * @param destination destination node     Ver.: always
 * @param status status of the network submission Ver.: always
 *
 */
WEAK(void sl_zigbee_af_green_power_cluster_commissioning_message_status_notification_cb(sl_zigbee_af_green_power_server_commissioning_state_t *commissioningState,
                                                                                        sl_zigbee_aps_frame_t *apsFrame,
                                                                                        sl_zigbee_outgoing_message_type_t messageType,
                                                                                        uint16_t destination,
                                                                                        sl_status_t status))
{
  UNUSED_VAR(commissioningState);
  UNUSED_VAR(apsFrame);
  UNUSED_VAR(messageType);
  UNUSED_VAR(destination);
  UNUSED_VAR(status);
}

/** @brief Update alias information callback.
 *
 * This function is called by the green power server plugin during
 * commissioning to update alias information from user.
 *
 * @param gpdAddr GPD address Ver.: always
 * @param alias        Ver.: always
 *
 * @returns true if the alias is updated by the caller.
 */
WEAK(bool sl_zigbee_af_green_power_server_update_alias_cb(sl_zigbee_gp_address_t *gpdAddr,
                                                          uint16_t *alias))
{
  UNUSED_VAR(gpdAddr);
  UNUSED_VAR(alias);

  return false;
}

/** @brief Green power server pairing status callback
 *
 * This function is called by the Green Power Server plugin during the pairing
 * process to indicate the status. This may be called multiple times for a single
 * pairing session. This provides the status as well as the current GPD context.
 * This callback can be monitored to get information in case a GPD commissioning
 * that has started ended up in success or failure. This callback does not give
 * any information about a commissioning GPDF that gets filtered out ealier in the
 * commissioning processing.
 *
 * @param status status of the pairing Ver.: always
 * @param commissioningGpd context of the GPD that is currently commissioning Ver.: always
 */
WEAK(void sl_zigbee_af_green_power_server_pairing_status_cb(sl_zigbee_sink_pairing_status_t status,
                                                            sl_zigbee_commissioning_gpd_t *commissioningGpd))
{
  UNUSED_VAR(status);
  UNUSED_VAR(commissioningGpd);
}
