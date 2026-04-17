/***************************************************************************//**
 * @file
 * @brief This file handles the functionality to establish a link key
 * between two devices, neither of which are the TC.   This is called
 * a "Partner Link Key Request" and is done as follows:
 *   1) Device A sends Binding Request to Device B to bind the Key
 *      Establishment Cluster.
 *   2) If Device B has the capacity to support another link key, and allows
 *      the Partner Link Key Request it returns a Binding result of
 *      ZDO Success.
 *   3) Device A receives the binding response and sends an APS Command
 *      of Request Key, type Application Link Key, to the Trust Center.
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

#include "app/framework/include/af.h"
#include "app/util/zigbee-framework/zigbee-device-common.h" //sl_zigbee_bind_request
#ifdef EZSP_HOST
  #include "app/util/zigbee-framework/zigbee-device-host.h" //sl_zigbee_network_address_request
#endif
#include "partner-link-key-exchange.h"

#include "app/framework/util/af-main.h"
#include "stack/include/zigbee-security-manager.h"

sl_zigbee_af_event_t sl_zigbee_af_partner_link_key_exchange_timeout_network_events[SL_ZIGBEE_SUPPORTED_NETWORKS];
void sl_zigbee_af_partner_link_key_exchange_timeout_network_event_handler(sl_zigbee_af_event_t * event);
typedef struct {
  bool active;
  sl_802154_short_addr_t target;
  uint8_t targetEndpoint;
  uint8_t initiatorEndpoint;
  sl_zigbee_af_partner_link_key_exchange_callback_t *callback;
} sli_zigbee_partner_link_key_exchange_state_t;
static sli_zigbee_partner_link_key_exchange_state_t states[SL_ZIGBEE_SUPPORTED_NETWORKS];

static void partnerLinkKeyExchangeComplete(bool success);
static sl_zigbee_zdo_status_t validateKeyRequest(sl_802154_long_addr_t partnerEui64);
static void addressDiscoveryCallback(const sl_zigbee_af_service_discovery_result_t* result);
static sl_status_t continuePartnerLinkKeyExchangeCallback(sl_802154_short_addr_t target,
                                                          sl_802154_long_addr_t source,
                                                          uint8_t endpoint,
                                                          uint8_t destinationEndpoint);

bool sli_zigbee_af_allow_partner_link_key = true;

//-----Internal callback

void sli_zigbee_af_partner_link_key_exchange_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_network_event_init(sl_zigbee_af_partner_link_key_exchange_timeout_network_events,
                                  sl_zigbee_af_partner_link_key_exchange_timeout_network_event_handler);
}

//-----
sl_status_t sl_zigbee_af_initiate_partner_link_key_exchange_cb(sl_802154_short_addr_t target,
                                                               uint8_t endpoint,
                                                               sl_zigbee_af_partner_link_key_exchange_callback_t *callback)
{
  sli_zigbee_partner_link_key_exchange_state_t *state = &states[sl_zigbee_get_current_network()];
  sl_802154_long_addr_t source;
  sl_status_t status;
  uint8_t destinationEndpoint;

  if (state->active) {
    sl_zigbee_af_key_establishment_cluster_println("%sPartner link key exchange in progress",
                                                   "Error: ");
    return SL_STATUS_INVALID_STATE;
  }

  destinationEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();
  if (destinationEndpoint == 0xFF) {
    return SL_STATUS_INVALID_STATE;
  }

  status = sl_zigbee_lookup_eui64_by_node_id(target, source);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_key_establishment_cluster_println("Starting %s for node 0x%04X",
                                                   "IEEE discovery",
                                                   target);
    status = sl_zigbee_af_find_ieee_address(target, addressDiscoveryCallback);
  } else {
    status = continuePartnerLinkKeyExchangeCallback(target,
                                                    source,
                                                    endpoint,
                                                    destinationEndpoint);
  }

  if (status == SL_STATUS_OK) {
    state->active = true;
    state->target = target;
    state->targetEndpoint = endpoint;
    state->initiatorEndpoint = destinationEndpoint;
    state->callback = callback;
    sl_zigbee_af_add_to_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_PARTNER_LINK_KEY_EXCHANGE);
    sl_zigbee_af_event_set_delay_ms(sl_zigbee_af_partner_link_key_exchange_timeout_network_events,
                                    SL_ZIGBEE_AF_PLUGIN_PARTNER_LINK_KEY_EXCHANGE_TIMEOUT_MILLISECONDS);
  }
  return status;
}

static void addressDiscoveryCallback(const sl_zigbee_af_service_discovery_result_t* result)
{
  if (sl_zigbee_af_have_discovery_response_status(result->status)
      && (result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST
          || result->zdoRequestClusterId == NETWORK_ADDRESS_REQUEST)) {
    // We get here under two circumstances:
    // 1. At the start of Partner Link Key Exchange (PLKE), when the initiator
    //    needs to find the target's IEEE address;
    // 2. At the end of PLKE, when the target needs to find the initiator's
    //    node ID.
    // In both cases, we need to update the address table but only in the first
    // case we need to continue the PLKE process.
    uint8_t* eui64ptr = (uint8_t*)(result->responseData);
    sl_zigbee_af_key_establishment_cluster_println("%s response: 0x%04X = %02X%02X%02X%02X%02X%02X%02X%02X",
                                                   result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST
                                                   ? "IEEE discovery"
                                                   : "NWK Address discovery",
                                                   result->matchAddress,
                                                   eui64ptr[7],
                                                   eui64ptr[6],
                                                   eui64ptr[5],
                                                   eui64ptr[4],
                                                   eui64ptr[3],
                                                   eui64ptr[2],
                                                   eui64ptr[1],
                                                   eui64ptr[0]);
#ifdef SL_ZIGBEE_AF_HAS_END_DEVICE_NETWORK
    // On a router, we rely on the stack caching the NodeId -> EUI64 mapping
    // in a routing table. But on an end device, there is no routing table and
    // we need to cache it ourselves in the address table.
    if (sl_zigbee_af_add_address_table_entry(eui64ptr, result->matchAddress) == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
      sl_zigbee_af_key_establishment_cluster_println("Cannot store IEEE address, table full.");
      partnerLinkKeyExchangeComplete(false);    // failure
    } else
#endif // SL_ZIGBEE_AF_HAS_END_DEVICE_NETWORK
    if (result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST) {
      sli_zigbee_partner_link_key_exchange_state_t *state = &states[sl_zigbee_get_current_network()];
      if (continuePartnerLinkKeyExchangeCallback(result->matchAddress,
                                                 eui64ptr,
                                                 state->initiatorEndpoint,
                                                 state->targetEndpoint) != SL_STATUS_OK) {
        partnerLinkKeyExchangeComplete(false);  // failure
      }
    }
  }
}

static sl_status_t continuePartnerLinkKeyExchangeCallback(sl_802154_short_addr_t target,
                                                          sl_802154_long_addr_t source,
                                                          uint8_t endpoint,
                                                          uint8_t destinationEndpoint)
{
  sl_802154_long_addr_t destination;
  sl_status_t status;

  status = validateKeyRequest(source);
  if (status != SL_ZIGBEE_ZDP_SUCCESS) {
    sl_zigbee_af_key_establishment_cluster_println("%s%s: 0x%02X",
                                                   "Error: ",
                                                   "Cannot perform partner link key exchange",
                                                   status);
    return status;
  }

  sl_zigbee_af_get_eui64(destination);
  status = sl_zigbee_bind_request(target,
                                  source,
                                  endpoint,
                                  ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                  UNICAST_BINDING,
                                  destination,
                                  0, // multicast group identifier - ignored
                                  destinationEndpoint,
                                  SL_ZIGBEE_APS_OPTION_NONE);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_key_establishment_cluster_println("%s%s: 0x%02X",
                                                   "Error: ",
                                                   "Failed to send bind request",
                                                   status);
  }
  return status;
}

sl_zigbee_zdo_status_t sl_zigbee_af_partner_link_key_exchange_request_cb(sl_802154_long_addr_t partner)
{
  sl_zigbee_zdo_status_t status = validateKeyRequest(partner);
  if (status != SL_ZIGBEE_ZDP_SUCCESS) {
    sl_zigbee_af_key_establishment_cluster_print("%sRejected parter link key request from ",
                                                 "Error: ");
    sl_zigbee_af_key_establishment_cluster_debug_exec(sl_zigbee_af_print_big_endian_eui64(partner));
    sl_zigbee_af_key_establishment_cluster_println(": 0x%02X", status);
    return status;
  }

  // Manually create an address table entry for the remote node so that the
  // stack will track its network address.  We can't send a network address
  // request here because we are not in the right stack context.
  if (sl_zigbee_af_add_address_table_entry(partner, SL_ZIGBEE_UNKNOWN_NODE_ID)
      == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX) {
    sl_zigbee_af_key_establishment_cluster_print("WARN: Could not create address table entry for ");
    sl_zigbee_af_key_establishment_cluster_debug_exec(sl_zigbee_af_print_big_endian_eui64(partner));
    sl_zigbee_af_key_establishment_cluster_println("");
  }

  if (!sli_zigbee_af_allow_partner_link_key) {
    sl_zigbee_af_key_establishment_cluster_println("Partner link key not allowed.");
    return SL_ZIGBEE_ZDP_NOT_PERMITTED;
  }
  return SL_ZIGBEE_ZDP_SUCCESS;
}

void sl_zigbee_af_partner_link_key_exchange_response_cb(sl_802154_short_addr_t sender,
                                                        sl_zigbee_zdo_status_t status)
{
  sli_zigbee_partner_link_key_exchange_state_t *state = &states[sl_zigbee_get_current_network()];
  if (state->active && state->target == sender) {
    sl_802154_long_addr_t partner;
    if (status != SL_ZIGBEE_ZDP_SUCCESS) {
      sl_zigbee_af_key_establishment_cluster_println("%sNode 0x%04X rejected partner link key request: 0x%02X",
                                                     "Error: ",
                                                     sender,
                                                     status);
      partnerLinkKeyExchangeComplete(false); // failure
      return;
    }
    if (sl_zigbee_lookup_eui64_by_node_id(sender, partner) != SL_STATUS_OK) {
      sl_zigbee_af_key_establishment_cluster_println("%sIEEE address of node 0x%04X is unknown",
                                                     "Error: ",
                                                     sender);
      partnerLinkKeyExchangeComplete(false); // failure
      return;
    }
    {
      sl_status_t requestLinkKeyStatus = sl_zigbee_request_link_key(partner);
      if (requestLinkKeyStatus != SL_STATUS_OK) {
        sl_zigbee_af_key_establishment_cluster_println("%s%s: 0x%02X",
                                                       "Error: ",
                                                       "Failed to request link key",
                                                       requestLinkKeyStatus);
        partnerLinkKeyExchangeComplete(false); // failure
        return;
      }
    }
  }
}

void sl_zigbee_af_partner_link_key_exchange_zigbee_key_establishment_cb(sl_802154_long_addr_t partner,
                                                                        sl_zigbee_key_status_t status)
{
  sli_zigbee_partner_link_key_exchange_state_t *state;
  sl_802154_short_addr_t nodeId;

  (void) sl_zigbee_af_push_callback_network_index();

  state = &states[sl_zigbee_get_current_network()];

  // If we can't look up the node id using the long address, we need to send a
  // network address request.  This should only occur when we did not initiate
  // the partner link key exchange, so we don't need to worry about the node id
  // check below that fires the callback.  The stack will update the address
  // table entry that we created above when the response comes back.
  sl_status_t sl_status = sl_zigbee_lookup_node_id_by_eui64(partner, &nodeId);
  if (sl_status != SL_STATUS_OK) {
    sl_zigbee_af_find_node_id(partner, addressDiscoveryCallback);
  }

  sl_zigbee_af_key_establishment_cluster_println((status <= SL_ZIGBEE_TRUST_CENTER_LINK_KEY_ESTABLISHED
                                                  ? "Key established: %d"
                                                  : ((status >= SL_ZIGBEE_TC_RESPONDED_TO_KEY_REQUEST
                                                      && status <= SL_ZIGBEE_TC_APP_KEY_SENT_TO_REQUESTER)
                                                     ? "TC answered key request: %d"
                                                     : "Failed to establish key: %d")),
                                                 status);

  if (state->active && state->target == nodeId) {
    partnerLinkKeyExchangeComplete(status == SL_ZIGBEE_APP_LINK_KEY_ESTABLISHED);
  }

  (void) sl_zigbee_af_pop_network_index();
}

void sl_zigbee_af_partner_link_key_exchange_timeout_network_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  partnerLinkKeyExchangeComplete(false); // failure
}

static void partnerLinkKeyExchangeComplete(bool success)
{
  sli_zigbee_partner_link_key_exchange_state_t *state = &states[sl_zigbee_get_current_network()];

  state->active = false;
  sl_zigbee_af_event_set_inactive(sl_zigbee_af_partner_link_key_exchange_timeout_network_events);
  sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_PARTNER_LINK_KEY_EXCHANGE);

  if (state->callback != NULL) {
    (*state->callback)(success);
  }
}

static sl_zigbee_zdo_status_t validateKeyRequest(sl_802154_long_addr_t partner)
{
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;
  sl_zigbee_sec_man_init_context(&context);

  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;

  // Partner link key requests are not valid for the trust center because it
  // already has a link key with all nodes on the network.
  if (sl_zigbee_af_get_node_id() == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    return SL_ZIGBEE_ZDP_NOT_PERMITTED;
  }

  // We must have an authorized trust center link key before we are able to
  // process link key exchanges with other nodes.
  if (SL_STATUS_OK != sl_zigbee_sec_man_get_aps_key_info(&context, &key_data)
      || !(key_data.bitmask & SL_ZIGBEE_KEY_IS_AUTHORIZED)) {
    return SL_ZIGBEE_ZDP_NOT_AUTHORIZED;
  }

  // We need an existing entry or an empty entry in the key table to process a
  // partner link key exchange.
  if (!sl_zigbee_sec_man_link_key_slot_available(partner)) {
    return SL_ZIGBEE_ZDP_TABLE_FULL;
  }

  return SL_ZIGBEE_ZDP_SUCCESS;
}
