/***************************************************************************//**
 * @file af-soc.c
 * @brief SoC-specific APIs and infrastructure code.
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#include "app/framework/util/af-main.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/util/common.h"
#include "app/util/security/security.h"
#include "app/util/zigbee-framework/zigbee-device-library.h"
#include "stack/include/source-route.h"
#include "stack/config/sl_zigbee_configuration_defaults.h"
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"
#include "stack/include/sl_zigbee_token.h"
#include "stack/config/token-cache.h"

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
#include "fragmentation.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_FORM_AND_JOIN_PRESENT
#include "app/util/common/form-and-join.h"
#endif

#if (defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && !defined(SL_ZIGBEE_TEST))
#define EXTENDED_RESET_INFO
#include "cortexm3/diagnostic.h"
#endif // !defined(SL_ZIGBEE_TEST)

#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  #include "app/test/pro-compliance.h"
#endif

//------------------------------------------------------------------------------
// Internal callbacks

void sli_zigbee_af_init_cb(uint8_t init_level)
{
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  if (isZCPContext() == true) {
    // If this is running as a pro-compliance app, we should return here
    // to avoid setting up network (e.g init network state, neighbor exchange)
    // Leave that to the ZCP test framework (java files)
    return;
  }
#endif
  (void)init_level;

  sl_zigbee_af_core_println("Reset info: 0x%02X (%s)",
                            halGetResetInfo(),
                            halGetResetString());

  // This will initialize the stack of networks maintained by the framework,
  // including setting the default network.
  sli_zigbee_af_initialize_network_index_stack();

  // Initialize messageSentCallbacks table
  sli_zigbee_af_initialize_message_sent_callback_array();

  sl_zigbee_af_endpoint_configure();

  // The address cache needs to be initialized and used with the source routing
  // code for the trust center to operate properly.
  securityAddressCacheInit(SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_SIZE,                     // offset
                           SL_ZIGBEE_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE); // size

  sli_zigbee_af_network_security_init();

  // Set the manufacturing code. This is defined by ZigBee document 053874r10
  // Ember's ID is 0x1002 and is the default, but this can be overridden in App Builder.
  sl_zigbee_set_manufacturer_code(SL_ZIGBEE_ZCL_MANUFACTURER_CODE);

  sl_zigbee_node_descriptor_info_t descriptor;
  descriptor.max_incoming_transfer_size = SL_ZIGBEE_AF_INCOMING_BUFFER_LENGTH;
  descriptor.max_outgoing_transfer_size = SL_ZIGBEE_AF_MAXIMUM_SEND_PAYLOAD_LENGTH;
  descriptor.capability = 0xFF;
  (void) sl_zigbee_set_node_descriptor(&descriptor);

  sl_802154_set_tx_power_mode(SL_ZIGBEE_AF_TX_POWER_MODE);

  (void)sl_zigbee_set_radio_ieee802154_cca_mode(SL_ZIGBEE_RADIO_802154_CCA_MODE);
}

//privileged callback / runs on vNCP
sl_zigbee_zdo_status_t sli_zigbee_af_remote_set_binding_callback(sl_zigbee_binding_table_entry_t *entry)
{
  sl_zigbee_zdo_status_t status = SL_ZIGBEE_ZDP_TABLE_FULL;
#if (SL_ZIGBEE_BINDING_TABLE_SIZE > 0)
  sl_status_t setStatus;
  sl_zigbee_binding_table_entry_t candidate;
  uint8_t i;

  // If we receive a bind request for the Key Establishment cluster and we are
  // not the trust center, then we are doing partner link key exchange.  We
  // don't actually have to create a binding.
  if (sl_zigbee_af_get_node_id() != SL_ZIGBEE_TRUST_CENTER_NODE_ID
      && entry->clusterId == ZCL_KEY_ESTABLISHMENT_CLUSTER_ID) {
    status = sl_zigbee_af_partner_link_key_exchange_request_cb(entry->identifier);
    goto kickout;
  }

  // ask the application if current binding request is allowed or not
  status = sl_zigbee_af_remote_set_binding_permission_cb(entry) == SL_STATUS_OK ? SL_ZIGBEE_ZDP_SUCCESS : SL_ZIGBEE_ZDP_NOT_AUTHORIZED;
  if (status == SL_ZIGBEE_ZDP_SUCCESS) {
    // For all other requests, we search the binding table for an unused entry
    // and store the new entry there if we find one.
    for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
      if (sl_zigbee_get_binding(i, &candidate) == SL_STATUS_OK
          && candidate.type == SL_ZIGBEE_UNUSED_BINDING) {
        setStatus = sl_zigbee_set_binding(i, entry);
        status = setStatus == SL_STATUS_OK ? SL_ZIGBEE_ZDP_SUCCESS // binding set
                 : (setStatus == SL_STATUS_ZIGBEE_BINDING_IS_ACTIVE ? SL_ZIGBEE_ZDP_NOT_AUTHORIZED // selected index is active
                    : SL_ZIGBEE_ZDP_TABLE_FULL); // report full for any other failure
        goto kickout;
      }
    }
    // If we get here, we didn't find an empty table slot, so table is full.
    status = SL_ZIGBEE_ZDP_TABLE_FULL;
  }

  kickout:
#endif
  return status;
}

sl_zigbee_zdo_status_t sli_zigbee_af_remote_delete_binding_callback(uint8_t index)
{
  sl_status_t deleteStatus;
  sl_zigbee_zdo_status_t status;

  // ask the application if current binding request is allowed or not
  status = sl_zigbee_af_remote_delete_binding_permission_cb(index) == SL_STATUS_OK
           ? SL_ZIGBEE_ZDP_SUCCESS
           : SL_ZIGBEE_ZDP_NOT_AUTHORIZED;
  if (status == SL_STATUS_OK) {
    deleteStatus = sl_zigbee_delete_binding(index);
    status = deleteStatus == SL_STATUS_OK ? SL_ZIGBEE_ZDP_SUCCESS // binding deleted
             : (deleteStatus == SL_STATUS_ZIGBEE_BINDING_IS_ACTIVE ? SL_ZIGBEE_ZDP_NOT_AUTHORIZED //selected index is active
                : SL_ZIGBEE_ZDP_NO_ENTRY); // report no entry for any other failure
    sl_zigbee_af_zdo_println("delete binding: %02X %02X", index, status);
  }
  return status;
}

void sli_zigbee_af_incoming_message_callback(sl_zigbee_incoming_message_type_t type,
                                             sl_zigbee_aps_frame_t *apsFrame,
                                             sl_zigbee_rx_packet_info_t *packetInfo,
                                             uint8_t messageLength,
                                             uint8_t *message)
{
  sli_zigbee_af_incoming_message_handler(type,
                                         apsFrame,
                                         packetInfo,
                                         messageLength,
                                         message);
}

extern void sli_zigbee_af_message_sent(sl_status_t status,
                                       sl_zigbee_outgoing_message_type_t type,
                                       uint16_t indexOrDestination,
                                       sl_zigbee_aps_frame_t *apsFrame,
                                       uint16_t messageTag,
                                       uint8_t messageLength,
                                       uint8_t *message);

// Called when a message we sent is acked by the destination or when an
// ack fails to arrive after several retransmissions.
void sli_zigbee_af_message_sent_callback(sl_status_t status,
                                         sl_zigbee_outgoing_message_type_t type,
                                         uint16_t indexOrDestination,
                                         sl_zigbee_aps_frame_t *apsFrame,
                                         uint16_t messageTag,
                                         uint8_t messageLength,
                                         uint8_t *messageContents)
{
#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
  // This call if returns true, ends up calling
  // sli_zigbee_af_fragmentation_message_sent_handler() when the last fragment was received.
  if (sli_zigbee_af_fragmentation_message_sent(apsFrame, status)) {
    return;
  }
#endif //SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT

  sl_zigbee_concentrator_note_delivery_failure(type, status);

  sli_zigbee_af_message_sent_handler(status,
                                     type,
                                     indexOrDestination,
                                     apsFrame,
                                     messageTag,
                                     messageLength,
                                     messageContents);

  // Generated dispatcher
  sli_zigbee_af_message_sent(status,
                             type,
                             indexOrDestination,
                             apsFrame,
                             messageTag,
                             messageLength,
                             messageContents);
}

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
void sli_zigbee_af_fragmentation_message_sent_handler(sl_status_t status,
                                                      sl_zigbee_outgoing_message_type_t type,
                                                      uint16_t indexOrDestination,
                                                      sl_zigbee_aps_frame_t *apsFrame,
                                                      uint16_t messageTag,
                                                      uint8_t *buffer,
                                                      uint16_t bufLen)
{
#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  if (isZCPContext() == true) {
    sli_zigbee_af_fragmentation_message_sent_handler_pro_compliance(status, type, indexOrDestination, apsFrame, messageTag, buffer, bufLen);
    return;
  }
#endif
  // the fragmented message is no longer in process
  sl_zigbee_af_debug_println("%send.", "Fragmentation:");
  sli_zigbee_af_message_sent_handler(status,
                                     type,
                                     indexOrDestination,
                                     apsFrame,
                                     messageTag,
                                     bufLen,
                                     buffer);

  // Generated dispatcher
  sli_zigbee_af_message_sent(status,
                             type,
                             indexOrDestination,
                             apsFrame,
                             messageTag,
                             bufLen,
                             buffer);

  // EMZIGBEE-4437: setting back the buffers to the original in case someone set
  // that to something else.
  sl_zigbee_af_set_external_buffer(appResponseData,
                                   SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN,
                                   &appResponseLength,
                                   &sl_zigbee_af_response_aps_frame);
}
#endif // SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT

//------------------------------------------------------------------------------
// Public APIs

void sl_zigbee_af_get_mfg_string(uint8_t* returnData)
{
  (void)sl_token_manager_get_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(TOKEN_MFG_STRING), (void *)returnData, sizeof(tokTypeMfgString));
}

uint8_t sl_zigbee_af_get_stack_profile(void)
{
  return sl_zigbee_stack_profile();
}

uint8_t sli_zigbee_stack_get_endpoint(uint8_t index)
{
  uint8_t ep;
  ep = sli_zigbee_af_get_endpoint(index);
  return ep;
}

// must return the endpoint desc of the endpoint specified
bool sli_zigbee_stack_get_endpoint_description(uint8_t endpoint,
                                               sl_zigbee_endpoint_description_t *result)
{
  bool status;
  status = sli_zigbee_af_get_endpoint_description(endpoint, result);
  return status;
}

// must return the clusterId at listIndex in the list specified for the
// endpoint specified
uint16_t sli_zigbee_stack_get_endpoint_cluster(uint8_t endpoint,
                                               sl_zigbee_cluster_list_id_t listId,
                                               uint8_t listIndex)
{
  uint16_t cluster;
  cluster = sli_zigbee_af_get_endpoint_cluster(endpoint, listId, listIndex);
  return cluster;
}

void sl_zigbee_af_get_eui64(sl_802154_long_addr_t returnEui64)
{
  memmove(returnEui64, sl_zigbee_get_eui64(), EUI64_SIZE);
}

sl_status_t sl_zigbee_af_get_network_parameters(sl_zigbee_node_type_t* nodeType,
                                                sl_zigbee_network_parameters_t* parameters)
{
  return sl_zigbee_get_network_parameters(nodeType, parameters);
}

sl_status_t sl_zigbee_af_get_node_type(sl_zigbee_node_type_t *nodeType)
{
  return sl_zigbee_get_network_parameters(nodeType, NULL);
}

uint8_t sl_zigbee_af_get_security_level(void)
{
  return sl_zigbee_security_level();
}

uint8_t sl_zigbee_af_get_key_table_size(void)
{
  return SL_ZIGBEE_KEY_TABLE_SIZE;
}

uint8_t sl_zigbee_af_get_binding_table_size(void)
{
  return SL_ZIGBEE_BINDING_TABLE_SIZE;
}

uint8_t sl_zigbee_af_get_address_table_size(void)
{
  return SL_ZIGBEE_ADDRESS_TABLE_SIZE;
}

uint8_t sl_zigbee_af_get_child_table_size(void)
{
  return SL_ZIGBEE_CHILD_TABLE_SIZE;
}

uint8_t sl_zigbee_af_get_neighbor_table_size(void)
{
  return SL_ZIGBEE_NEIGHBOR_TABLE_SIZE;
}

uint8_t sl_zigbee_af_get_route_table_size(void)
{
  return SL_ZIGBEE_ROUTE_TABLE_SIZE;
}

uint8_t sl_zigbee_af_get_sleepy_multicast_config(void)
{
  return SL_ZIGBEE_SEND_MULTICASTS_TO_SLEEPY_ADDRESS;
}

sl_status_t sl_zigbee_af_get_source_route_table_entry(
  uint8_t index,
  sl_802154_short_addr_t *destination,
  uint8_t *closerIndex)
{
  return sl_zigbee_get_source_route_table_entry(index,
                                                destination,
                                                closerIndex);
}

uint8_t sl_zigbee_af_get_source_route_table_total_size(void)
{
  return sl_zigbee_get_source_route_table_total_size();
}

uint8_t sl_zigbee_af_get_source_route_table_filled_size(void)
{
  return sl_zigbee_get_source_route_table_filled_size();
}

sl_status_t sl_zigbee_af_get_child_data(uint8_t index,
                                        sl_zigbee_child_data_t *childData)
{
  return sl_zigbee_get_child_data(index,
                                  childData);
}

//------------------------------------------------------------------------------
// Internal APIs

uint16_t sli_zigbee_af_get_packet_buffer_free_space(void)

{
  return sli_legacy_buffer_manager_buffer_bytes_remaining();
}

uint16_t sli_zigbee_af_get_packet_buffer_total_space(void)
{
  return sli_legacy_buffer_manager_buffer_bytes_total();
}

#if !(defined(SL_ZIGBEE_LEAF_STACK))
uint8_t sl_zigbee_af_get_open_network_duration_sec(void)
{
  return sli_zigbee_get_permit_joining_remaining_duration_sec();
}
#endif

void sli_zigbee_af_cli_version_command(void)
{
  sli_zigbee_af_parse_and_print_version(sl_zigbee_version);
}

void sli_zigbee_af_print_ezsp_endpoint_flags(uint8_t endpoint)
{
  (void)endpoint;
  // Not applicable for SOC
}

uint8_t sli_zigbee_stack_get_endpoint_count(void)
{
  uint8_t numberOfEndpoints;
  numberOfEndpoints = sl_zigbee_af_endpoint_count();
  return numberOfEndpoints;
}
