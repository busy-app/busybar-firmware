/***************************************************************************//**
 * @file
 * @brief APIs and defines for the Device Management Client plugin.
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
 * @defgroup device-management-client Device Management Client
 * @ingroup component cluster
 * @brief API and Callbacks for the Device Management Cluster Client Component
 *
 * This component implements the client-side functionality of the
 * Smart Energy 1.2 Device Management cluster.
 *
 */

/**
 * @addtogroup device-management-client
 * @{
 */

/**
 * @name API
 * @{
 */

/** @brief Print clients
 *
 */
void sl_zigbee_af_device_management_client_print();

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup device_mgmt_client_cb Device Management Client
 * @ingroup af_callback
 * @brief Callbacks for Device Management Client Component
 *
 */

/**
 * @addtogroup device_mgmt_client_cb
 * @{
 */

/** @brief Enact a change of tenancy.
 *
 * This callback is implemented by a meter to enact a change of tenancy at
 * the requested time of implementation.
 *
 * @param endpoint   Ver.: always
 * @param tenancy   Ver.: always
 */
void sl_zigbee_af_device_management_client_enact_change_of_tenancy_cb(uint8_t endpoint,
                                                                      sl_zigbee_af_device_management_tenancy_t *tenancy);
/** @brief Enact a change of supplier.
 *
 * This callback is implemented by a meter to enact a change of supplier
 * at the requested time of implementation.
 *
 * @param endpoint   Ver.: always
 * @param supplier   Ver.: always
 */
void sl_zigbee_af_device_management_client_enact_change_of_supplier_cb(uint8_t endpoint,
                                                                       sl_zigbee_af_device_management_supplier_t *supplier);
/** @brief Enact a change of supply.
 *
 * This callback is implemented by a meter to enact a supply change at the
 * requested time of implementation.
 *
 * @param endpoint   Ver.: always
 * @param supply   Ver.: always
 */
sl_status_t sl_zigbee_af_device_management_client_enact_change_supply_cb(uint8_t endpoint,
                                                                         sl_zigbee_af_device_management_supply_t *supply);
/** @brief Set a supply status.
 *
 * This callback is implemented by a meter to appropriately set the supply
 * status.
 *
 * @param endpoint   Ver.: always
 * @param supplyStatus   Ver.: always
 */
void sl_zigbee_af_device_management_client_set_supply_status_cb(uint8_t endpoint,
                                                                sl_zigbee_af_device_management_supply_status_flags_t *supplyStatus);
/** @brief Enact an update of the uncontrolled flow threshold.
 *
 * This callback is implemented by a meter to enact an update to the
 * uncontrolled flow threshold as specified.
 *
 * @param endpoint   Ver.: always
 * @param supplier   Ver.: always
 */
void sl_zigbee_af_device_management_client_enact_update_uncontrolled_flow_threshold_cb(uint8_t endpoint,
                                                                                       sl_zigbee_af_device_management_uncontrolled_flow_threshold_t *supplier);
/** @} */ // end of device_mgmt_client_cb
/** @} */ // end of name Callbacks
/** @} */ // end of device-management-client
