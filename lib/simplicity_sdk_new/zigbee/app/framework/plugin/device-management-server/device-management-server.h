/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Device Management Server plugin.
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

#include "app/framework/plugin/device-management-server/device-management-common.h"

/**
 * @defgroup device-management-server Device Management Server
 * @ingroup component cluster
 * @brief API and Callbacks for the Device Management Cluster Server Component
 *
 * This component implements the server-side functionality of the
 * Smart Energy 1.2 Device Management cluster.
 *
 */

/**
 * @addtogroup device-management-server
 * @{
 */

/**
 * @name API
 * @{
 */

/** @brief Set tenancy.
 *
 * @param tenency Ver.: always
 * @param validateOptionalFields Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_tenancy(sl_zigbee_af_device_management_tenancy_t *tenancy,
                                                bool validateOptionalFields);

/** @brief Get tenancy.
 *
 * @param tenancy tenancy Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_get_tenancy(sl_zigbee_af_device_management_tenancy_t *tenancy);

/** @brief Set supplier for the device.
 *
 * @param endpoint Ver.: always
 * @param supplier Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_supplier(uint8_t endpoint, sl_zigbee_af_device_management_supplier_t *supplier);

/** @brief Get supplier for the device.
 *
 * @param supplier Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_get_supplier(sl_zigbee_af_device_management_supplier_t *supplier);

/** @brief Set global data device information.
 *
 * @param providerId Ver.: always
 * @param issuerEventId Ver.: always
 * @param tarrifType Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_info_global_data(uint32_t providerId,
                                                         uint32_t issuerEventId,
                                                         uint8_t tariffType);

/** @brief Set device site ID.
 *
 * @param siteID Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_site_id(sl_zigbee_af_device_management_site_id_t *siteId);

/** @brief Get device site ID.
 *
 * @param siteID Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_get_site_id(sl_zigbee_af_device_management_site_id_t *siteId);

/** @brief Set device CIN.
 *
 * @param cin Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_cin(sl_zigbee_af_device_management_c_i_n_t *cin);

/** @brief Get device CIN.
 *
 * @param cin Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_get_cin(sl_zigbee_af_device_management_c_i_n_t *cin);

/** @brief Set device password.
 *
 * @param password Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_password(sl_zigbee_af_device_management_password_t *password);

/** @brief Get device password.
 *
 * @param password Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_get_password(sl_zigbee_af_device_management_password_t *password,
                                                 uint8_t passwordType);

/** @brief Device management server print
 *
 */
void sl_zigbee_af_device_management_server_print(void);

/** @brief Update device site ID.
 *
 * @param dstAddr Ver.: always
 * @param srcEndpoint Ver.: always
 * @param dstEndpoint Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_cluster_update_site_id(sl_802154_short_addr_t dstAddr,
                                                           uint8_t srcEndpoint,
                                                           uint8_t dstEndpoint);

/** @brief Set device provider ID.
 *
 * @param providerId Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_provider_id(uint32_t providerId);

/** @brief Set device issuer event ID.
 *
 * @param issuerEventId
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_issuer_event_id(uint32_t issuerEventId);

/** @brief Set device tariff type.
 *
 * @param tarrifType Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_set_tariff_type(sl_zigbee_af_tariff_type_t tariffType);

/** @brief Publish the change of tenancy.
 *
 * @param dstAddr Ver.: always
 * @param srcEndpoint Ver.: always
 * @param dstEndpoint Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_cluster_publish_change_of_tenancy(sl_802154_short_addr_t dstAddr,
                                                                      uint8_t srcEndpoint,
                                                                      uint8_t dstEndpoint);

/** @brief Publish the change of supplier.
 *
 * @param dstAddr Ver.: always
 * @param srcEndpoint Ver.: always
 * @param dstEndpoint Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_cluster_publish_change_of_supplier(sl_802154_short_addr_t dstAddr,
                                                                       uint8_t srcEndpoint,
                                                                       uint8_t dstEndpoint);

/** @brief Set device pending updates.
 *
 * @param pendingUpdatesMask Ver.: always
 *
 */
void sl_zigbee_af_device_management_cluster_set_pending_updates(sl_zigbee_af_device_management_change_pending_flags_t pendingUpdatesMask);

/** @brief Get device pending updates.
 *
 * @param pendingUpdatesMask Ver.: always
 *
 */
void sl_zigbee_af_device_management_cluster_get_pending_updates(sl_zigbee_af_device_management_change_pending_flags_t *pendingUpdatesMask);

/** @brief Update CIN for the device.
 *
 * @param dstAddr Ver.: always
 * @param srcEndpoint Ver.: always
 * @param dstEndpoint Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_cluster_update_cin(sl_802154_short_addr_t dstAddr,
                                                       uint8_t srcEndpoint,
                                                       uint8_t dstEndpoint);

/** @brief Send the request new password response.
 *
 * @param passwordType Ver.: always
 * @param dstAddr Ver.: always
 * @param srcEndpoint Ver.: always
 * @param dstEndpoint Ver.: always
 *
 * @return bool true is success
 *
 */
bool sl_zigbee_af_device_management_cluster_send_request_new_password_response(uint8_t passwordType,
                                                                               sl_802154_short_addr_t dstAddr,
                                                                               uint8_t srcEndpoint,
                                                                               uint8_t dstEndpoint);

/** @} */ // end of name API
/** @} */ // end of device-management-server
