/***************************************************************************//**
 * @file
 * @brief Zigbee Dynamic Node Type
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "stack/zigbee/dynamic-node-type.h"
#include "stack/core/sl_zigbee_stack.h"
#include "stack/include/stack-info.h"
#include "stack/internal/inc/stack-info-internal-def.h"
#include "stack/internal/inc/network-formation-internal-def.h"
#include "stack/routing/zigbee/leave.h"
#include "stack/routing/zigbee/child.h"
#include "stack/routing/zigbee/association.h"
#include "stack/include/network-formation.h"
#include "stack/include/multi-network.h"
#include "app/framework/include/af-types.h"

// External declaration for internal configuration function
extern void sli_zigbee_set_end_device_configuration(uint8_t end_device_configuration);

/**
 * @brief Switches the Zigbee node type to Sleepy End Device if currently a Router.
 *
 * This function checks if the current node type is SL_ZIGBEE_ROUTER. If so, it leaves the network quietly,
 * erases the child table, sets the end device configuration, changes the node type to
 * SL_ZIGBEE_SLEEPY_END_DEVICE, and attempts to rejoin the network as a sleepy end device.
 *
 * @param end_device_configuration End device configuration flags to set.
 * @return SL_STATUS_OK if the operation was successful, or an error status otherwise.
 */
sl_status_t slxi_zigbee_stack_switch_role_sleepy_end_device(uint8_t end_device_configuration)
{
  sl_zigbee_node_type_t current_node_type;
  sl_status_t status = sli_zigbee_stack_get_network_parameters(&current_node_type, NULL);
  bool connected = (sli_zigbee_stack_network_state() == SL_ZIGBEE_JOINED_NETWORK);

  if (status == SL_STATUS_OK && current_node_type == SL_ZIGBEE_ROUTER) {
    if (connected) {
      sli_zigbee_leave_network_quietly();
    }

    sli_zigbee_erase_child_table();

    sli_zigbee_set_end_device_configuration(end_device_configuration);

    sli_zigbee_set_node_type(SL_ZIGBEE_SLEEPY_END_DEVICE);
    sli_zigbee_write_node_type_token(SL_ZIGBEE_SLEEPY_END_DEVICE);

    if (connected) {
      (void)sli_zigbee_stack_find_and_rejoin_network(true, // secure rejoin
                                                     SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK,
                                                     SL_ZIGBEE_REJOIN_REASON_NONE,
                                                     SL_ZIGBEE_SLEEPY_END_DEVICE);
    }

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

/**
 * @brief Switches the Zigbee node type to Router if currently a Sleepy End Device.
 *
 * This function checks if the current node type is SL_ZIGBEE_SLEEPY_END_DEVICE. If so, it leaves the network quietly,
 * sets the node type to SL_ZIGBEE_ROUTER, and attempts to rejoin the network as a router.
 *
 * @return SL_STATUS_OK if the operation was successful, or SL_STATUS_FAIL otherwise.
 */
sl_status_t slxi_zigbee_stack_switch_role_router(void)
{
  sl_zigbee_node_type_t current_node_type;
  sl_status_t status = sli_zigbee_stack_get_network_parameters(&current_node_type, NULL);
  bool connected = (sli_zigbee_stack_network_state() == SL_ZIGBEE_JOINED_NETWORK);

  if (status == SL_STATUS_OK && current_node_type == SL_ZIGBEE_SLEEPY_END_DEVICE) {
    if (connected) {
      sli_zigbee_leave_network_quietly();
    }

    sli_zigbee_set_node_type(SL_ZIGBEE_ROUTER);
    sli_zigbee_write_node_type_token(SL_ZIGBEE_ROUTER);

    if (connected) {
      (void)sli_zigbee_stack_find_and_rejoin_network(true, // secure rejoin
                                                     SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK,
                                                     SL_ZIGBEE_REJOIN_REASON_NONE,
                                                     SL_ZIGBEE_DEVICE_TYPE_UNCHANGED);
    }

    status = SL_STATUS_OK;
  } else if (current_node_type == SL_ZIGBEE_ROUTER) {
    status = SL_STATUS_ALREADY_INITIALIZED;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}
