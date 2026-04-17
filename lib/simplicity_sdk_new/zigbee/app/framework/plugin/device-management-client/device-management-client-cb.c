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

#include "app/framework/include/af.h"

/** @brief Enact Change Of Tenancy
 *
 * This callback will be implemented by a meter to enact a change of tenancy at
 * the requested time of implementation.
 *
 * @param endpoint   Ver.: always
 * @param tenancy   Ver.: always
 */
WEAK(void sl_zigbee_af_device_management_client_enact_change_of_tenancy_cb(uint8_t endpoint,
                                                                           sl_zigbee_af_device_management_tenancy_t *tenancy))
{
  UNUSED_VAR(endpoint);
  UNUSED_VAR(tenancy);
}

/** @brief Enact Change Of Supplier
 *
 * This callback will be implemented by a meter to enact a change of supplier
 * at the requested time of implementation.
 *
 * @param endpoint   Ver.: always
 * @param supplier   Ver.: always
 */
WEAK(void sl_zigbee_af_device_management_client_enact_change_of_supplier_cb(uint8_t endpoint,
                                                                            sl_zigbee_af_device_management_supplier_t *supplier))
{
  UNUSED_VAR(endpoint);
  UNUSED_VAR(supplier);
}

/** @brief Enact Change Supply
 *
 * This callback will be implemented by a meter to enact a supply change at the
 * requested time of implementation.
 *
 * @param endpoint   Ver.: always
 * @param supply   Ver.: always
 */
WEAK(sl_status_t sl_zigbee_af_device_management_client_enact_change_supply_cb(uint8_t endpoint,
                                                                              sl_zigbee_af_device_management_supply_t *supply))
{
  UNUSED_VAR(endpoint);
  UNUSED_VAR(supply);

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

/** @brief Set Supply Status
 *
 * This callback will be implemented by a meter to appropriately set the supply
 * status.
 *
 * @param endpoint   Ver.: always
 * @param supplyStatus   Ver.: always
 */
WEAK(void sl_zigbee_af_device_management_client_set_supply_status_cb(uint8_t endpoint,
                                                                     sl_zigbee_af_device_management_supply_status_flags_t *supplyStatus))
{
  UNUSED_VAR(endpoint);
  UNUSED_VAR(supplyStatus);
}

/** @brief Enact Update Uncontrolled Flow Threshold
 *
 * This callback will be implemented by a meter to enact an update to the
 * uncontrolled flow threshold as specified.
 *
 * @param endpoint   Ver.: always
 * @param supplier   Ver.: always
 */
WEAK(void sl_zigbee_af_device_management_client_enact_update_uncontrolled_flow_threshold_cb(uint8_t endpoint,
                                                                                            sl_zigbee_af_device_management_uncontrolled_flow_threshold_t *supplier))
{
  UNUSED_VAR(endpoint);
  UNUSED_VAR(supplier);
}
