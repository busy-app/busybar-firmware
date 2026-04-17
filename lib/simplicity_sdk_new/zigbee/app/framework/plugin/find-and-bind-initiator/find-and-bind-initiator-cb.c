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

/** @brief Bind Target
 *
 * This callback with enable the user to programmatically decide if they want
 * to bind with a potential target. The plugin will try to bind with this
 * target if and only if the function returns true. By default, the callback
 * tells the plugin to try to bind with the target. If the binding type is
 * changed to ::SL_ZIGBEE_MULTICAST_BINDING, a multicast binding will be created.
 *
 * @param nodeId short ID of the potential target Ver.: always
 * @param bindingEntry The binding entry for that target. Ver.: always
 * @param groupName The name of the group if a multicast binding is created.
 * Ver.: always
 */
WEAK(bool sl_zigbee_af_find_and_bind_initiator_bind_target_cb(sl_802154_short_addr_t nodeId,
                                                              sl_zigbee_binding_table_entry_t *bindingEntry,
                                                              uint8_t *groupName))
{
  (void) nodeId;
  (void) bindingEntry;
  (void) groupName;
  return true;
}

/** @brief Complete
 *
 * This callback is fired by the initiator when the Find and Bind process is
 * complete.
 *
 * @param status Status code describing the completion of the find and bind
 * process Ver.: always
 */
WEAK(void sl_zigbee_af_find_and_bind_initiator_complete_cb(sl_status_t status))
{
  sl_zigbee_af_core_println("Find and Bind Initiator: Complete: 0x%02X", status);
}
