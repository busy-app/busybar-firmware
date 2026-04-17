/***************************************************************************//**
 * @file
 * @brief Routines for Zigbee 4.0/BDB 3.1 trust centers.
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

#include "app/framework/include/af.h"
#include "zigbee-bdb-3dot1-trust-center-extension.h"
#include "device-database.h"
#include "device-query-service.h"
#include "zigbee-security-manager.h"
#include "app/util/zigbee-framework/zigbee-device-common.h" //sl_zigbee_bind_request

#define PLUGIN_NAME "BDB 3.1 TC Extension"
#define R23_STACK_REVISION  23

static uint8_t local_endpoint_with_poll_control_client = 0xFF;

void sli_zigbee_af_bdb_3dot1_trust_center_extension_init_callback(uint8_t init_level)
{
  (void)init_level;

  uint8_t index;
  for (index = 0; index < sl_zigbee_af_endpoint_count(); index++) {
    uint8_t endpoint = sl_zigbee_af_endpoint_from_index(index);
    if (sl_zigbee_af_contains_client(endpoint, ZCL_POLL_CONTROL_CLUSTER_ID)) {
      local_endpoint_with_poll_control_client = endpoint;
      break;
    }
  }
}

void sli_zigbee_af_bdb_3dot1_trust_center_extension_key_establishment_cb(sl_802154_long_addr_t partner,
                                                                         sl_zigbee_key_status_t key_status)
{
  // This component is added to the ZC+ZR application, so ensure we only act if we are a ZC
  if (sl_zigbee_get_node_id() != SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    return;
  }

  // If the device requested an auth token, then the device is online and has performed DLK already
  if (key_status == SL_ZIGBEE_TC_AUTHENTICATION_TOKEN_ESTABLISHED) {
    (void)sl_zigbee_af_device_query_service_discover_target(partner, 0xFF);  // Node Descriptor response will update capabilities
  }
}

void sl_zigbee_af_device_database_discovery_complete_cb(const sl_zigbee_af_device_info_t* info)
{
  sl_status_t status;
  uint8_t endpoint;
  sl_802154_long_addr_t eui;
  memcpy(eui, info->eui64, EUI64_SIZE);
  sl_802154_long_addr_t my_eui;
  sl_zigbee_af_get_eui64(my_eui);
  sl_802154_short_addr_t node_id;

  // This component is added to the ZC+ZR application, so ensure we only act if we are a ZC
  if (sl_zigbee_get_node_id() != SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    return;
  }

  // Ensure that we've completed key establishment with the device, then
  // bind to the Poll Control cluster if it's an end device
  sl_zigbee_af_core_print("%s: discovery complete for device ", PLUGIN_NAME);
  sl_zigbee_af_print_big_endian_eui64(eui);
  sl_zigbee_af_core_println("");

  // Check for Poll Control cluster server, then bind if so
  status = sl_zigbee_af_device_database_does_device_have_cluster(eui,
                                                                 ZCL_POLL_CONTROL_CLUSTER_ID,
                                                                 true,
                                                                 &endpoint);
  if (status == SL_STATUS_OK) {
    const sl_zigbee_af_device_info_t* device = sl_zigbee_af_device_database_find_device_by_eui64(eui);
    if (device && device->stackRevision == R23_STACK_REVISION) {
      status = sl_zigbee_lookup_node_id_by_eui64(eui, &node_id);
      if (status == SL_STATUS_OK) {
        status = sl_zigbee_bind_request(node_id,
                                        eui,
                                        endpoint,
                                        ZCL_POLL_CONTROL_CLUSTER_ID,
                                        UNICAST_BINDING,
                                        my_eui,
                                        0, // multicast group identifier - ignored
                                        local_endpoint_with_poll_control_client,
                                        (SL_ZIGBEE_APS_OPTION_ENCRYPTION | SL_ZIGBEE_APS_OPTION_RETRY));
        sl_zigbee_af_core_println("%s sent binding to 0x%04X: 0x%02X",
                                  PLUGIN_NAME,
                                  node_id,
                                  status);
      }
    }
  }
}
