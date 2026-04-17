/***************************************************************************//**
 * @file af-common.c
 * @brief Code common to both the Host and SOC (system on a chip) versions
 * of the Application Framework.
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
#include "zigbee_zcl_callback_dispatcher.h"
#include "af-main.h"
#include "app/framework/util/common.h"
#include "service-discovery.h"
#include "app/framework/security/af-security.h"
#include "app/framework/security/crypto-state.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "library.h"
#ifdef SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE
  #include "stack/include/cbke-crypto-engine.h"  // sl_zigbee_get_certificate()
#endif

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_SUB_GHZ_CLIENT_PRESENT
#include "sub-ghz-client.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_SUB_GHZ_SERVER_PRESENT
#include "sub-ghz-server.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
  #include "fragmentation.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_CRITICAL_MESSAGE_QUEUE_PRESENT
 #include "critical-message-queue.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  #include "test-harness-config.h"
#endif

#ifdef SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT
  #include "app/test/pro-compliance.h"
#endif

#ifdef EZSP_HOST
#define INVALID_MESSAGE_TAG 0xFFFF
#define setStackProfile(stackProfile) \
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_STACK_PROFILE, stackProfile, "stack profile")
#define setSecurityLevel(securityLevel) \
  sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_SECURITY_LEVEL, securityLevel, "security level")
#else
#define INVALID_MESSAGE_TAG 0x0000
#define setStackProfile(stackProfile) sl_zigbee_set_stack_profile(stackProfile)
#define setSecurityLevel(securityLevel) sl_zigbee_set_security_level(securityLevel)
#endif

#define INVALID_CALLBACK_INDEX 0xFF

// flags the user can turn on or off to make the printing behave differently
bool sl_zigbee_af_print_received_messages = true;

// TODO: we hardcode these values here instead of pulling in ota-storage.h
// which define INVALID_OTA_IMAGE_ID. That would force a dependency to the
// zigbee_ota_storage_common component for all apps.
const sl_zigbee_af_ota_image_id_t sl_zigbee_af_invalid_image_id
  = { 0xFFFF, 0xFFFF, 0xFFFFFFFFUL, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

static sli_zigbee_callback_table_entry_t messageSentCallbacks[SL_ZIGBEE_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE];

// We declare this variable 'const' but NOT const.  Those functions that we may use
// this variable would also have to declare it const in order to function
// correctly, which is not the case (e.g. sl_zigbee_find_key_table_entry()).
const sl_802154_long_addr_t sl_zigbee_af_null_eui64 = { 0, 0, 0, 0, 0, 0, 0, 0 };
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
extern sl_zigbee_af_event_t sl_zigbee_af_test_harness_z3_open_network_event;
#endif

static sl_zigbee_rx_packet_info_t *current_rx_packet = NULL;

//------------------------------------------------------------------------------
// Forward declarations

static uint8_t getMessageSentCallbackIndex(void);
static void invalidateMessageSentCallbackEntry(uint16_t messageTag);
static sl_zigbee_af_message_sent_function_t getMessageSentCallback(uint16_t tag);
static sl_status_t send(sl_zigbee_outgoing_message_type_t type,
                        uint16_t indexOrDestination,
                        sl_zigbee_aps_frame_t *apsFrame,
                        uint16_t messageLength,
                        uint8_t *message,
                        bool broadcast,
                        sl_802154_short_addr_t alias,
                        uint8_t sequence,
                        sl_zigbee_af_message_sent_function_t callback);
static sl_status_t broadcastPermitJoin(uint8_t duration);
static void printMessage(sl_zigbee_incoming_message_type_t type,
                         sl_zigbee_aps_frame_t* apsFrame,
                         uint16_t messageLength,
                         uint8_t* messageContents);

#define UNKNOWN_NETWORK_STATE 0xFF

typedef struct {
  sl_802154_short_addr_t node_id;
  sl_802154_pan_id_t  pan_id;
  sl_zigbee_network_status_t network_state;
  uint8_t radio_channel;
} sli_zigbee_network_cache_t;

static sli_zigbee_network_cache_t network_cache[SL_ZIGBEE_SUPPORTED_NETWORKS];

void sli_zigbee_af_clear_network_cache(uint8_t network_index)
{
  if (network_index < SL_ZIGBEE_SUPPORTED_NETWORKS) {
    network_cache[network_index].node_id = SL_ZIGBEE_NULL_NODE_ID;
    network_cache[network_index].pan_id = 0xFFFF;
    network_cache[network_index].network_state = UNKNOWN_NETWORK_STATE;
    network_cache[network_index].radio_channel = 0xFF;
  }
}

sl_zigbee_network_status_t sl_zigbee_af_network_state(void)
{
  uint8_t network_index = sl_zigbee_get_current_network();
  if (network_index < SL_ZIGBEE_SUPPORTED_NETWORKS) {
    if (network_cache[network_index].network_state == UNKNOWN_NETWORK_STATE) {
      network_cache[network_index].network_state = sl_zigbee_network_state();
    }
    return network_cache[network_index].network_state;
  }
  return UNKNOWN_NETWORK_STATE;
}

bool sli_zigbee_af_stay_awake_network_state(void)
{
  for (uint8_t network_index = 0; network_index < SL_ZIGBEE_SUPPORTED_NETWORKS; ++network_index) {
    if (network_cache[network_index].network_state == UNKNOWN_NETWORK_STATE
        && sl_zigbee_af_push_network_index(network_index) == SL_STATUS_OK) {
      network_cache[network_index].network_state = sl_zigbee_network_state();
      (void) sl_zigbee_af_pop_network_index();
    }
    if ((network_cache[network_index].network_state != SL_ZIGBEE_JOINED_NETWORK)
        && (network_cache[network_index].network_state != SL_ZIGBEE_JOINED_NETWORK_S2S_INITIATOR)
        && (network_cache[network_index].network_state != SL_ZIGBEE_JOINED_NETWORK_S2S_TARGET)) {
      return true;
    }
  }
  return false;
}

// Because an IPC call can be expensive in terms of bandwidth,
// we cache the node ID so it can be quickly retrieved by the host.
sl_802154_short_addr_t sl_zigbee_af_get_node_id(void)
{
  uint8_t network_index = sl_zigbee_get_current_network();
  if (network_cache[network_index].node_id == SL_ZIGBEE_NULL_NODE_ID) {
    network_cache[network_index].node_id = sl_zigbee_get_node_id();
  }
  return network_cache[network_index].node_id;
}

sl_802154_pan_id_t sl_zigbee_af_get_pan_id(void)
{
  uint8_t network_index = sl_zigbee_get_current_network();
  if (network_cache[network_index].pan_id == 0xFFFF) {
    sl_zigbee_node_type_t nodeType;
    sl_zigbee_network_parameters_t parameters;
    sl_zigbee_af_get_network_parameters(&nodeType, &parameters);
    network_cache[network_index].pan_id = parameters.panId;
  }
  return network_cache[network_index].pan_id;
}

uint8_t sl_zigbee_af_get_radio_channel(void)
{
  uint8_t network_index = sl_zigbee_get_current_network();
  if (network_cache[network_index].radio_channel == 0xFF) {
    sl_zigbee_node_type_t nodeType;
    sl_zigbee_network_parameters_t parameters;
    sl_zigbee_af_get_network_parameters(&nodeType, &parameters);
    network_cache[network_index].radio_channel = parameters.radioChannel;
  }
  return network_cache[network_index].radio_channel;
}

//------------------------------------------------------------------------------
// Internal callbacks

// This is called from sli_zigbee_af_incoming_message_callback() in af-soc.c or af-host.c
void sli_zigbee_af_incoming_message_handler(sl_zigbee_incoming_message_type_t type,
                                            sl_zigbee_aps_frame_t *apsFrame,
                                            sl_zigbee_rx_packet_info_t *packetInfo,
                                            uint16_t messageLength,
                                            uint8_t *message)
{
  sl_zigbee_af_incoming_message_t im;
  sl_802154_short_addr_t current_sender = packetInfo->sender_short_id;

  current_rx_packet = packetInfo;

#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
  if (sli_zigbee_af_fragmentation_incoming_message(type,
                                                   apsFrame,
                                                   current_sender,
                                                   &message,
                                                   &messageLength)) {
    sl_zigbee_af_debug_println("%sfragment processed.", "Fragmentation:");
    return;
  }
#endif // SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT

#ifdef SL_CATALOG_ZIGBEE_SUB_GHZ_SERVER_PRESENT
  if (sli_zigbee_af_sub_ghz_server_incoming_message(type,
                                                    apsFrame,
                                                    current_sender,
                                                    messageLength,
                                                    message)) {
    sl_zigbee_af_debug_println("Sub-GHz: incoming message rejected.");
    return;
  }
#endif // SL_CATALOG_ZIGBEE_SUB_GHZ_SERVER_PRESENT

  sl_zigbee_af_debug_println("Processing message: len=%d profile=%04X cluster=%04X",
                             messageLength,
                             apsFrame->profileId,
                             apsFrame->clusterId);
  sl_zigbee_af_debug_flush();

  // Populate the incoming message struct to pass to the incoming message
  // callback.
  im.type              = type;
  im.apsFrame          = apsFrame;
  im.message           = message;
  im.msgLen            = messageLength;
  im.source            = current_sender;
  im.lastHopLqi        = packetInfo->last_hop_lqi;
  im.lastHopRssi       = packetInfo->last_hop_rssi;
  im.bindingTableIndex = packetInfo->binding_index;
  im.addressTableIndex = packetInfo->address_index;
  im.networkIndex      = sl_zigbee_get_current_network();
  if (sl_zigbee_af_pre_message_received_cb(&im)) {
    return;
  }

  // Handle service discovery responses.
  if (sli_zigbee_af_service_discovery_incoming(current_sender,
                                               apsFrame,
                                               message,
                                               messageLength)) {
    return;
  }

  // Handle ZDO messages.
  if (sli_zigbee_af_process_zdo(current_sender, apsFrame, message, messageLength)) {
    return;
  }

  // Handle ZCL messages.
  if (sl_zigbee_af_process_message(apsFrame,
                                   type,
                                   message,
                                   messageLength,
                                   current_sender,
                                   NULL)) { // no inter-pan header
    return;
  }

  current_rx_packet = NULL;
}

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
uint8_t currentSentMessageTag;
#endif

// This is called from sli_zigbee_af_message_sent_callback() in af-soc.c or af-host.c
void sli_zigbee_af_message_sent_handler(sl_status_t status,
                                        sl_zigbee_outgoing_message_type_t type,
                                        uint16_t indexOrDestination,
                                        sl_zigbee_aps_frame_t *apsFrame,
                                        uint16_t messageTag,
                                        uint16_t messageLength,
                                        uint8_t *messageContents)
{
  sl_zigbee_af_message_sent_function_t callback;
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_app_print("%stx 0x%08X, ", "ERROR: ", status); // status
    printMessage(type, apsFrame, messageLength, messageContents);
  }
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  currentSentMessageTag = messageTag;
#endif
  callback = getMessageSentCallback(messageTag);
  invalidateMessageSentCallbackEntry(messageTag);

  if (status == SL_STATUS_OK
      && apsFrame->profileId == SL_ZIGBEE_ZDO_PROFILE_ID
      && apsFrame->clusterId < CLUSTER_ID_RESPONSE_MINIMUM) {
    sl_zigbee_af_add_to_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_ZDO_RESPONSE);
  }

  sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_DATA_ACK);

  if (messageContents != NULL && messageContents[0] & ZCL_CLUSTER_SPECIFIC_COMMAND) {
    sl_zigbee_af_cluster_message_sent_with_mfg_code_cb(type,
                                                       indexOrDestination,
                                                       apsFrame,
                                                       messageLength,
                                                       messageContents,
                                                       status,
                                                       // If the manufacturer specific flag is set
                                                       // get read it as next part of message
                                                       // else use null code.
                                                       (((messageContents[0]
                                                          & ZCL_MANUFACTURER_SPECIFIC_MASK)
                                                         == ZCL_MANUFACTURER_SPECIFIC_MASK)
                                                        ? sl_zigbee_af_get_int16u(messageContents, 1, messageLength)
                                                        : SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE));
  }

  if (callback != NULL) {
    (*callback)(type, indexOrDestination, apsFrame, messageLength, messageContents, status);
  }

  sl_zigbee_af_message_sent_cb(type,
                               indexOrDestination,
                               apsFrame,
                               messageLength,
                               messageContents,
                               status);
}

WEAK(void sli_zigbee_af_stack_status_callback(sl_status_t status))
{
  sl_zigbee_af_app_flush();

  switch (status) {
    case SL_STATUS_NETWORK_UP:
    case SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_CHANGED:      // also means NETWORK_UP
    case SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_NOT_CHANGED:  // also means NETWORK_UP
    {
      // ZigBee 3.0 security configuration is handled in plugins.
#if !defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z3) && !defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4) && defined(SLI_ZIGBEE_PRIMARY_NETWORK_SECURITY_TYPE)
      // Set the runtime security settings as soon as the stack goes up.
      sl_zigbee_extended_security_bitmask_t oldExtended;
      sl_zigbee_extended_security_bitmask_t newExtended = 0;
      const sl_zigbee_af_security_profile_data_t *data = sli_zigbee_af_get_current_security_profile_data();
      bool trustCenter = (sl_zigbee_af_get_node_id() == SL_ZIGBEE_TRUST_CENTER_NODE_ID);
      if (data != NULL) {
        newExtended = (trustCenter
                       ? data->tcExtendedBitmask
                       : data->nodeExtendedBitmask);
      }

      sl_zigbee_get_extended_security_bitmask(&oldExtended);
      if ((oldExtended & SL_ZIGBEE_EXT_NO_FRAME_COUNTER_RESET) != 0U) {
        newExtended |= SL_ZIGBEE_EXT_NO_FRAME_COUNTER_RESET;
      }

      sl_zigbee_af_security_init_cb(NULL, &newExtended, trustCenter);
      sl_zigbee_set_extended_security_bitmask(newExtended);
#endif // !defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z3) && !defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_Z4) && defined(SLI_ZIGBEE_PRIMARY_NETWORK_SECURITY_TYPE)

      sl_zigbee_af_app_println("SL_STATUS_NETWORK_UP 0x%04X", sl_zigbee_af_get_node_id());
      sl_zigbee_af_app_flush();
#if defined(SL_ZIGBEE_TEST)
      simulatedTimePasses();
#endif

      if (status == SL_STATUS_NETWORK_UP) {
        sl_zigbee_start_writing_stack_tokens();
      } else {
        sl_zigbee_af_app_println("Trust Center EUI has %schanged.",
                                 (status == SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_CHANGED) ? "" : "not ");
        // We abort registration because we want to clear out any previous
        // state and force it to start anew.  One of two results will occur after
        // we restart registration later.
        // (1) It succeeds and we are on a new network with a new TC, in which
        //     case we need to kick off key establishment to re-authenticate it
        //     and also re-discover other ESIs.
        // (2) It will fail, in which case we have to reboot to forget the untrusted
        //     network and its settings.
        sl_zigbee_af_registration_abort_cb();
        sl_zigbee_af_registration_start_cb();
      }

      // This kicks off registration for newly joined devices.  If registration
      // already occurred, nothing will happen here.
      sl_zigbee_af_registration_start_cb();
      break;
    }

    case SL_STATUS_ZIGBEE_RECEIVED_KEY_IN_THE_CLEAR:
    case SL_STATUS_ZIGBEE_NO_NETWORK_KEY_RECEIVED:
    case SL_STATUS_ZIGBEE_NO_LINK_KEY_RECEIVED:
    case SL_STATUS_ZIGBEE_PRECONFIGURED_KEY_REQUIRED:
    case SL_STATUS_ZIGBEE_MOVE_FAILED:
    case SL_STATUS_NOT_JOINED:
    case SL_STATUS_NO_BEACONS:
    case SL_STATUS_NETWORK_DOWN:
      if (status == SL_STATUS_NETWORK_DOWN) {
        sl_zigbee_af_app_println("SL_STATUS_NETWORK_DOWN");
      } else {
        sl_zigbee_af_app_println("SL_STATUS_NOT_JOINED");
      }
      sl_zigbee_af_app_flush();
      sl_zigbee_af_stack_down();
      break;

    #if !(defined(SL_ZIGBEE_LEAF_STACK))
    case SL_STATUS_ZIGBEE_NETWORK_OPENED:
      sl_zigbee_af_app_println("SL_STATUS_ZIGBEE_NETWORK_OPENED: %d sec", sl_zigbee_af_get_open_network_duration_sec());
      return;

    case SL_STATUS_ZIGBEE_NETWORK_CLOSED:
      sl_zigbee_af_app_println("SL_STATUS_ZIGBEE_NETWORK_CLOSED");
      return;
    #endif

    case SL_STATUS_ZIGBEE_REJOIN_FAILED_BUT_NETWORK_RESTORED:
      sl_zigbee_af_app_println("SL_STATUS_ZIGBEE_REJOIN_FAILED_BUT_NETWORK_RESTORED");
      return;

    default:
      sl_zigbee_af_debug_println("EVENT: stackStatus 0x%08X", status);
  }

  sl_zigbee_af_app_flush();
}

#ifdef SL_ZIGBEE_TEST
void halButtonIsr(uint8_t button, uint8_t state)
{
  sl_zigbee_af_hal_button_isr_cb(button, state);
}
#endif

//------------------------------------------------------------------------------
// Public APIs

sl_status_t sl_zigbee_af_send_multicast_with_cb(sl_zigbee_multicast_id_t multicastId,

                                                sl_802154_short_addr_t alias,
                                                uint8_t sequence,
                                                sl_zigbee_aps_frame_t *apsFrame,
                                                uint16_t messageLength,
                                                uint8_t *message,
                                                sl_zigbee_af_message_sent_function_t callback)
{
  apsFrame->groupId = multicastId;
  return send(SL_ZIGBEE_OUTGOING_MULTICAST,
              multicastId,
              apsFrame,
              messageLength,
              message,
              true, // broadcast?
              alias, //alias
              sequence, //sequence
              callback);
}

sl_status_t sl_zigbee_af_send_multicast(sl_zigbee_multicast_id_t multicastId,
                                        sl_802154_short_addr_t alias,
                                        uint8_t sequence,
                                        sl_zigbee_aps_frame_t *apsFrame,
                                        uint16_t messageLength,
                                        uint8_t *message)
{
  return sl_zigbee_af_send_multicast_with_cb(multicastId,
                                             alias,
                                             sequence,
                                             apsFrame,
                                             messageLength,
                                             message,
                                             NULL);
}

sl_status_t sl_zigbee_af_send_multicast_to_bindings(sl_zigbee_aps_frame_t *apsFrame,
                                                    uint16_t messageLength,
                                                    uint8_t* message)
{
  sl_status_t status = SL_STATUS_INVALID_INDEX;
#if (SL_ZIGBEE_BINDING_TABLE_SIZE > 0)
  uint8_t i;
  sl_zigbee_binding_table_entry_t binding;
  uint16_t groupDest;

  sl_802154_short_addr_t alias = SL_ZIGBEE_NULL_NODE_ID;
  uint8_t nwkSeq = 0;

  if ((NULL == apsFrame) || (0 == messageLength) || (NULL == message)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    status = sl_zigbee_get_binding(i, &binding);
    if (status != SL_STATUS_OK) {
      return status;
    }

    if (binding.type == SL_ZIGBEE_MULTICAST_BINDING
        && binding.local == apsFrame->sourceEndpoint
        && binding.clusterId == apsFrame->clusterId) {
      groupDest = (binding.identifier[0]
                   + (((uint16_t)(binding.identifier[1])) << 8));
      apsFrame->groupId = groupDest;
      apsFrame->destinationEndpoint = binding.remote;

      status = sl_zigbee_af_send_multicast(groupDest,        // multicast ID
                                           alias,
                                           nwkSeq,
                                           apsFrame,
                                           messageLength,
                                           message);

      if (status != SL_STATUS_OK) {
        return status;
      }
    }
  }
#endif // (SL_ZIGBEE_BINDING_TABLE_SIZE > 0)

  return status;
}

sl_status_t sl_zigbee_af_send_broadcast_with_cb(sl_802154_short_addr_t destination,
                                                sl_zigbee_aps_frame_t *apsFrame,
                                                uint16_t messageLength,
                                                uint8_t* message,
                                                sl_802154_short_addr_t alias,
                                                uint8_t sequence,
                                                sl_zigbee_af_message_sent_function_t callback)

{
  return send(SL_ZIGBEE_OUTGOING_BROADCAST,
              destination,
              apsFrame,
              messageLength,
              message,
              true,   // broadcast?
              alias,   //alias
              sequence,   //sequence
              callback);
}

sl_status_t sl_zigbee_af_send_broadcast(sl_802154_short_addr_t alias,
                                        sl_802154_short_addr_t destination,
                                        uint8_t sequence,
                                        sl_zigbee_aps_frame_t *apsFrame,
                                        uint16_t messageLength,
                                        uint8_t *message)
{
  return sl_zigbee_af_send_broadcast_with_cb(destination,
                                             apsFrame,
                                             messageLength,
                                             message,
                                             alias,
                                             sequence,
                                             NULL);
}

sl_status_t sl_zigbee_af_send_unicast_with_cb(sl_zigbee_outgoing_message_type_t type,
                                              uint16_t indexOrDestination,
                                              sl_zigbee_aps_frame_t *apsFrame,
                                              uint16_t messageLength,
                                              uint8_t *message,
                                              sl_zigbee_af_message_sent_function_t callback)
{
  // The source endpoint in the APS frame MAY NOT be valid at this point if the
  // outgoing type is "via binding."
  if (type == SL_ZIGBEE_OUTGOING_VIA_BINDING) {
    // If using binding, set the endpoints based on those in the binding.  The
    // cluster in the binding is not used because bindings can be used to send
    // messages with any cluster id, not just the one set in the binding.
    sl_zigbee_binding_table_entry_t binding;
    sl_status_t status = sl_zigbee_get_binding(indexOrDestination, &binding);
    if (status != SL_STATUS_OK) {
      return status;
    }
    apsFrame->sourceEndpoint = binding.local;
    apsFrame->destinationEndpoint = binding.remote;
  }
  return send(type,
              indexOrDestination,
              apsFrame,
              messageLength,
              message,
              false,   // broadcast?
              0,   //alias
              0,   //sequence
              callback);
}

sl_status_t sl_zigbee_af_send_unicast(sl_zigbee_outgoing_message_type_t type,
                                      uint16_t indexOrDestination,
                                      sl_zigbee_aps_frame_t *apsFrame,
                                      uint16_t messageLength,
                                      uint8_t *message)
{
  return sl_zigbee_af_send_unicast_with_cb(type,
                                           indexOrDestination,
                                           apsFrame,
                                           messageLength,
                                           message,
                                           NULL);
}

sl_status_t sl_zigbee_af_send_unicast_to_bindings_with_cb(sl_zigbee_aps_frame_t *apsFrame,
                                                          uint16_t messageLength,
                                                          uint8_t* message,
                                                          sl_zigbee_af_message_sent_function_t callback)
{
  sl_status_t status = SL_STATUS_FAIL;
#if (SL_ZIGBEE_BINDING_TABLE_SIZE > 0)
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t binding;
    sl_status_t bindingStatus = sl_zigbee_get_binding(i, &binding);
    if (bindingStatus != SL_STATUS_OK) {
      return bindingStatus;
    }
    if (binding.type == SL_ZIGBEE_UNICAST_BINDING
#ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
        && binding.networkIndex == sl_zigbee_get_current_network()
#endif // SL_ZIGBEE_MULTI_NETWORK_STRIPPED
        && binding.local == apsFrame->sourceEndpoint
        && binding.clusterId == apsFrame->clusterId) {
      apsFrame->destinationEndpoint = binding.remote;

      status = send(SL_ZIGBEE_OUTGOING_VIA_BINDING,
                    i,
                    apsFrame,
                    messageLength,
                    message,
                    false,   // broadcast?
                    0,   //alias
                    0,   //sequence
                    callback);
      if (status != SL_STATUS_OK) {
        return status;
      }
    }
  }
#endif // (SL_ZIGBEE_BINDING_TABLE_SIZE > 0)

  return status;
}

sl_status_t sl_zigbee_af_send_unicast_to_bindings(sl_zigbee_aps_frame_t *apsFrame,
                                                  uint16_t messageLength,
                                                  uint8_t* message)
{
  return sl_zigbee_af_send_unicast_to_bindings_with_cb(apsFrame,
                                                       messageLength,
                                                       message,
                                                       NULL);
}

sl_status_t sl_zigbee_af_send_inter_pan(sl_802154_pan_id_t panId,
                                        const sl_802154_long_addr_t destinationLongId,
                                        sl_802154_short_addr_t destinationShortId,
                                        sl_zigbee_multicast_id_t multicastId,
                                        sl_zigbee_af_cluster_id_t clusterId,
                                        sl_zigbee_af_profile_id_t profileId,
                                        uint16_t messageLength,
                                        uint8_t* messageBytes)
{
  sl_zigbee_af_interpan_header_t header;
  memset(&header, 0, sizeof(sl_zigbee_af_interpan_header_t));
  header.panId = panId;
  header.shortAddress = destinationShortId;
  if (destinationLongId != NULL) {
    memmove(header.longAddress, destinationLongId, EUI64_SIZE);
    header.options |= SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS;
    header.messageType = SL_ZIGBEE_AF_INTER_PAN_UNICAST;
  } else if (multicastId != 0) {
    header.groupId = multicastId;
    header.messageType = SL_ZIGBEE_AF_INTER_PAN_MULTICAST;
  } else {
    header.messageType = (destinationShortId < SL_ZIGBEE_BROADCAST_ADDRESS
                          ? SL_ZIGBEE_AF_INTER_PAN_UNICAST
                          : SL_ZIGBEE_AF_INTER_PAN_BROADCAST);
  }
  header.profileId = profileId;
  header.clusterId = clusterId;
  return sl_zigbee_af_interpan_send_message_cb(&header,
                                               messageLength,
                                               messageBytes);
}

#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
void sl_zigbee_af_print_message_data(uint8_t* data, uint16_t length)
{
#ifdef SL_ZIGBEE_AF_PRINT_APP
  sl_zigbee_af_app_print(" payload (len %04X) [", length);
  sl_zigbee_af_app_print_buffer(data, length, true);
  sl_zigbee_af_app_println("]");
#else // SL_ZIGBEE_AF_PRINT_APP
  (void) data;
  (void) length;
#endif // SL_ZIGBEE_AF_PRINT_APP
}

void sli_zigbee_af_print_status(const char * task,
                                sl_status_t status)
{
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_print(sl_zigbee_af_print_active_area,
                       "%s: %s",
                       "Success",
                       task);
  } else {
    sl_zigbee_af_print(sl_zigbee_af_print_active_area,
                       "%s: %s: 0x%02X",
                       "Error",
                       task,
                       status);
  }
}
#endif //SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

sl_status_t sl_zigbee_af_permit_join(uint8_t duration,
                                     bool broadcastMgmtPermitJoin)
{
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  if (duration) {
    // The test harness needs to set up the correct link key
    sl_zigbee_af_event_set_active(&sl_zigbee_af_test_harness_z3_open_network_event);
  }
#endif
  // Permit joining forever is bad behavior, so we want to limit
  // this.  If 254 is not enough a re-broadcast should be done later.
  if (duration == SL_ZIGBEE_AF_PERMIT_JOIN_FOREVER) {
    sl_zigbee_af_app_println("Limiting duration of permit join from forever (255) to 254");
    duration = SL_ZIGBEE_AF_PERMIT_JOIN_MAX_TIMEOUT;
  }
  return sli_zigbee_af_permit_join(duration,
                                   broadcastMgmtPermitJoin);
}

sl_zigbee_af_cbke_key_establishment_suite_t sl_zigbee_af_is_full_smart_energy_security_present(void)
{
  sl_zigbee_af_cbke_key_establishment_suite_t cbkeKeyEstablishmentSuite = SL_ZIGBEE_AF_INVALID_KEY_ESTABLISHMENT_SUITE;

#if defined SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE
  sl_zigbee_certificate_data_t cert;
  sl_zigbee_certificate_283k1_data_t cert283k1;

  if ((sl_zigbee_get_library_status(SL_ZIGBEE_ECC_LIBRARY_ID)
       & SL_ZIGBEE_LIBRARY_PRESENT_MASK)
      && (SL_STATUS_OK == sl_zigbee_get_certificate(&cert)) ) {
    cbkeKeyEstablishmentSuite |= SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1;
  }

  if ((sl_zigbee_get_library_status(SL_ZIGBEE_ECC_LIBRARY_283K1_ID)
       & SL_ZIGBEE_LIBRARY_PRESENT_MASK)
      &&  (SL_STATUS_OK == sl_zigbee_get_certificate_283k1(&cert283k1))) {
    cbkeKeyEstablishmentSuite |= SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1;
  }
#endif

  return cbkeKeyEstablishmentSuite;
}

sl_status_t sl_zigbee_af_form_network(sl_zigbee_network_parameters_t *parameters)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;
  sl_zigbee_current_security_state_t securityState;
  if (sli_zigbee_af_pro_is_current_network()) {
    sl_zigbee_af_core_println("%sing on ch %d, panId 0x%04X",
                              "Form",
                              parameters->radioChannel,
                              parameters->panId);
    sl_zigbee_af_core_flush();
    if (sli_zigbee_af_current_zigbee_pro_network->nodeType == SL_ZIGBEE_COORDINATOR) {
      zaTrustCenterSecurityInit(true);   // centralized network
    }
    // ignore return value for now since it always returns SL_STATUS_OK
    (void)sl_zigbee_get_current_security_state(&securityState);
    if (sli_zigbee_af_current_zigbee_pro_network->nodeType == SL_ZIGBEE_COORDINATOR  \
        || ((sli_zigbee_af_current_zigbee_pro_network->nodeType == SL_ZIGBEE_ROUTER) \
            && (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE))) {
      status = sl_zigbee_form_network(parameters);
    } else {
      sl_zigbee_af_core_println("Error: Device does not support %s network formation",
                                (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE) ? "distributed" : "centralized");
    }
  }
  return status;
}

sl_status_t sl_zigbee_af_join_network(sl_zigbee_network_parameters_t *parameters)
{
  sl_status_t status = SL_STATUS_INVALID_STATE;
  if (sli_zigbee_af_pro_is_current_network()) {
    sl_zigbee_node_type_t nodeType = sli_zigbee_af_current_zigbee_pro_network->nodeType;
    if (nodeType == SL_ZIGBEE_COORDINATOR) {
      nodeType = SL_ZIGBEE_ROUTER;
    }
    zaNodeSecurityInit(true);   // centralized network
    sl_zigbee_af_core_println("%sing on ch %d, panId 0x%04X",
                              "Join",
                              parameters->radioChannel,
                              parameters->panId);
    status = sl_zigbee_join_network(nodeType, parameters);
  }
  return status;
}

// mfgString is expected to be +1 of MFG_STRING_MAX_LENGTH
void sl_zigbee_af_format_mfg_string(uint8_t* mfgString)
{
  uint8_t i;
  sl_zigbee_af_get_mfg_string(mfgString);

  for (i = 0; i < MFG_STRING_MAX_LENGTH; i++) {
    // The MFG string is not necessarily NULL terminated.
    // Uninitialized bytes are left at 0xFF so we make sure
    // it is NULL terminated.
    if (mfgString[i] == 0xFF) {
      mfgString[i] = '\0';
    }
  }
  mfgString[MFG_STRING_MAX_LENGTH] = '\0';
}

static const sl_zigbee_release_type_struct_t releaseTypes[] = {
  SL_ZIGBEE_RELEASE_TYPE_TO_STRING_STRUCT_DATA
};

sl_802154_short_addr_t sli_zigbee_af_get_sender(void)
{
  if (current_rx_packet != NULL) {
    return current_rx_packet->sender_short_id;
  }

  return SL_ZIGBEE_NULL_NODE_ID;
}

sl_status_t sli_zigbee_af_get_sender_eui64(sl_802154_long_addr_t sender_eui64)
{
  if (current_rx_packet != NULL
      && memcmp(current_rx_packet->sender_long_id, sl_zigbee_af_null_eui64, EUI64_SIZE) != 0) {
    memmove(sender_eui64, current_rx_packet->sender_long_id, EUI64_SIZE);
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t sli_zigbee_af_get_last_hop_lqi(uint8_t *lqi)
{
  if (current_rx_packet != NULL) {
    *lqi = current_rx_packet->last_hop_lqi;
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t sli_zigbee_af_get_last_hop_rssi(int8_t *rssi)
{
  if (current_rx_packet != NULL) {
    *rssi = current_rx_packet->last_hop_rssi;
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

uint8_t sli_zigbee_af_get_binding_index(void)
{
  if (current_rx_packet != NULL) {
    return current_rx_packet->binding_index;
  }

  return SL_ZIGBEE_NULL_BINDING;
}

uint8_t sli_zigbee_af_get_address_index(void)
{
  if (current_rx_packet != NULL) {
    return current_rx_packet->address_index;
  }

  return SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX;
}

//------------------------------------------------------------------------------
// Internal APIs

// Called from sli_zigbee_af_init_cb() in af-soc.c or af-host.c
// Initialize stack profile and security level based on security profile.
void sli_zigbee_af_network_security_init(void)
{
  // Typically, we initialize the stack profile based on SL_ZIGBEE_STACK_PROFILE
  // and initialize the security level based on SL_ZIGBEE_SECURITY_LEVEL.

  // However, such an approach cannot properly initialize the stack profile and
  // security level for coordinators (e.g. Multi-PAN devices) which can form
  // multiple networks with different stack profiles and security levels, because
  // the SL_ZIGBEE_STACK_PROFILE and SL_ZIGBEE_SECURITY_LEVEL are common for each network.

  // As each network has its own security profile, instead of using SL_ZIGBEE_STACK_PROFILE
  // and SL_ZIGBEE_SECURITY_LEVEL, we have this function to set up the stack profile and
  // security level based on the security profile of each network.
  uint8_t stackProfile;
  uint8_t securityLevel;
  for (uint8_t i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    (void) sl_zigbee_af_push_network_index(i);
    if (sli_zigbee_af_pro_is_current_network()) {
      switch (sli_zigbee_af_current_zigbee_pro_network->securityProfile) {
        case SL_ZIGBEE_AF_SECURITY_PROFILE_NONE:
          stackProfile = SL_ZIGBEE_STACK_PROFILE_NONE;
          securityLevel = SL_ZIGBEE_SECURITY_LEVEL_NONE;
          break;
        case SL_ZIGBEE_AF_SECURITY_PROFILE_HA:
        case SL_ZIGBEE_AF_SECURITY_PROFILE_HA12:
        case SL_ZIGBEE_AF_SECURITY_PROFILE_SE_TEST:
        case SL_ZIGBEE_AF_SECURITY_PROFILE_SE_FULL:
        case SL_ZIGBEE_AF_SECURITY_PROFILE_Z3:
        case SL_ZIGBEE_AF_SECURITY_PROFILE_Z4:
        case SL_ZIGBEE_AF_SECURITY_PROFILE_CUSTOM:
          stackProfile = SL_ZIGBEE_STACK_PROFILE_ZIGBEE_PRO;
          securityLevel = SL_ZIGBEE_SECURITY_LEVEL_Z3;
          break;
        default:
          sl_zigbee_af_core_println("Invalid Security Profile: 0x%02X",
                                    sli_zigbee_af_current_zigbee_pro_network->securityProfile);
          SL_ZIGBEE_TEST_ASSERT(false);
          (void) sl_zigbee_af_pop_network_index();
          return;
      }
      setStackProfile(stackProfile);
      setSecurityLevel(securityLevel);
    }
    (void) sl_zigbee_af_pop_network_index();
  }
}

// This is templated into zigbee_af_callback:init
// If possible, initialize each network.  For ZigBee PRO networks, the node
// type of the device must match the one used previously, but note that
// coordinator-capable devices are allowed to initialize as routers.
// If end-device switches from sleepy to non-sleepy,
// we also allow to initialize so that end-device can perform rejoining the network
// Same with non-sleepy end-device that switches to sleepy
#if (SL_ZIGBEE_AF_TC_SWAP_OUT_TEST == 0)
void sli_zigbee_af_network_init(uint8_t init_level)
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

  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    bool initialize = true;
    (void) sl_zigbee_af_push_network_index(i);
    sli_zigbee_af_clear_network_cache(i);
    if (sli_zigbee_af_pro_is_current_network()) {
      sl_zigbee_node_type_t nodeType;
      if (sli_zigbee_af_current_zigbee_pro_network->nodeType == SL_ZIGBEE_COORDINATOR) {
        zaTrustCenterSecurityPolicyInit();
      }
      if (sl_zigbee_af_get_node_type(&nodeType) == SL_STATUS_OK
          && (nodeType != sli_zigbee_af_current_zigbee_pro_network->nodeType
              && (nodeType != SL_ZIGBEE_END_DEVICE && sli_zigbee_af_current_zigbee_pro_network->nodeType != SL_ZIGBEE_SLEEPY_END_DEVICE)
              && (nodeType != SL_ZIGBEE_SLEEPY_END_DEVICE && sli_zigbee_af_current_zigbee_pro_network->nodeType != SL_ZIGBEE_END_DEVICE)
              && (nodeType != SL_ZIGBEE_S2S_INITIATOR_DEVICE)
              && (nodeType != SL_ZIGBEE_S2S_TARGET_DEVICE)
              && (sli_zigbee_af_current_zigbee_pro_network->nodeType != SL_ZIGBEE_COORDINATOR
                  || nodeType != SL_ZIGBEE_ROUTER))) {
        initialize = false;
      }
    }
    if (initialize) {
      sl_zigbee_network_init_struct_t networkInitStruct = { SL_ZIGBEE_AF_CUSTOM_NETWORK_INIT_OPTIONS };
      sl_zigbee_network_init(&networkInitStruct);
    }
    (void) sl_zigbee_af_pop_network_index();
  }
}
//SL_ZIGBEE_AF_TC_SWAP_OUT_TEST Prevent calling sli_zigbee_af_network_init during stack initialization
#else // SL_ZIGBEE_AF_TC_SWAP_OUT_TEST
void sli_zigbee_af_network_init(uint8_t init_level)
{
  UNUSED_VAR(init_level);
}
#endif //SL_ZIGBEE_AF_TC_SWAP_OUT_TEST
// Called from sli_zigbee_af_init_cb() in af-soc.c or af-host.c
void sli_zigbee_af_initialize_message_sent_callback_array(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++) {
    messageSentCallbacks[i].tag = INVALID_MESSAGE_TAG;
    messageSentCallbacks[i].callback = NULL;
  }
}

// Old API that doesn't restrict prevent permit joining forever (255)
sl_status_t sli_zigbee_af_permit_join(uint8_t duration,
                                      bool broadcastMgmtPermitJoin)
{
  sl_status_t status = sl_zigbee_permit_joining(duration);
  sl_zigbee_af_app_println("pJoin for %d sec: 0x%02X", duration, status);
  if (broadcastMgmtPermitJoin) {
    status = broadcastPermitJoin(duration);
  }
  return status;
}

bool sli_zigbee_af_process_zdo(sl_802154_short_addr_t sender,
                               sl_zigbee_aps_frame_t* apsFrame,
                               uint8_t* message,
                               uint16_t length)
{
  if (apsFrame->profileId != SL_ZIGBEE_ZDO_PROFILE_ID) {
    return false;
  }

  // To make the printing simpler, we assume all 'request' messages
  // have a status of 0x00.  Request messages have no status value in them
  // but saying 'success' (0x00) seems appropriate.
  // Response messages will have their status value printed appropriately.
#if !defined (SL_CATALOG_ZIGBEE_PRO_COMPLIANCE_PRESENT)
  sl_zigbee_af_zdo_println("RX: ZDO, command 0x%04X, status: 0x%02X",
                           apsFrame->clusterId,
                           (apsFrame->clusterId >= CLUSTER_ID_RESPONSE_MINIMUM
                            ? message[1]
                            : 0));
#endif
  // message[0] is the ZCL transaction number

  switch (apsFrame->clusterId) {
    case SIMPLE_DESCRIPTOR_RESPONSE:
      sl_zigbee_af_zdo_println("RX: %s Desc Resp", "Simple");
      break;
    case MATCH_DESCRIPTORS_RESPONSE:
      sl_zigbee_af_zdo_print("RX: %s Desc Resp", "Match");
      sl_zigbee_af_zdo_println(", Matches: %d", message[4]);
      break;
    case END_DEVICE_ANNOUNCE:
      sl_zigbee_af_zdo_println("Device Announce: 0x%04X",
                               length >= END_DEVICE_ANNOUNCE_LENGTH_WITH_NWK_ADDRESS ? ((uint16_t)(message[1]) + (uint16_t)(message[2] << 8)) : SL_ZIGBEE_NULL_NODE_ID);
      break;
    case IEEE_ADDRESS_RESPONSE:
      sl_zigbee_af_zdo_println("RX: IEEE Address Response");
      break;
    case ACTIVE_ENDPOINTS_RESPONSE:
      sl_zigbee_af_zdo_println("RX: Active EP Response, Count: %d", message[4]);
      break;
    case NODE_DESCRIPTOR_RESPONSE:
      sl_zigbee_af_zdo_print("RX: %s Desc Resp", "Node");
      sl_zigbee_af_zdo_println(", Matches: 0x%04X",
                               length >= NODE_DESCRIPTOR_RESPONSE_LENGTH_WITH_NWK_ADDRESS ? ((uint16_t)(message[2]) + (uint16_t)(message[3] << 8)) : SL_ZIGBEE_NULL_NODE_ID);
      break;
    default:
      break;
  }

  if (sli_zigbee_af_pre_zdo_message_received(sender, apsFrame, message, length)) {
    goto zdoProcessingDone;
  }

  sli_zigbee_af_zdo_message_received(sender, apsFrame, message, length);

  switch (apsFrame->clusterId) {
    case BIND_RESPONSE:
      sl_zigbee_af_partner_link_key_exchange_response_cb(sender, message[1]);
      break;
#ifdef SL_CATALOG_ZIGBEE_SUB_GHZ_SERVER_PRESENT
    case NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY:
      sli_zigbee_af_sub_ghz_server_zdo_message_received_callback(sender, message, length);
      break;
#endif
    default:
      break;
  }

  zdoProcessingDone:
  // if it is a zdo response we can remove the zdo waiting task
  // and let a sleepy go back into hibernation
  if (apsFrame->clusterId > CLUSTER_ID_RESPONSE_MINIMUM) {
    sl_zigbee_af_remove_from_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_ZDO_RESPONSE);
  }

  return true;
}

void sli_zigbee_af_parse_and_print_version(sl_zigbee_version_t versionStruct)
{
  uint8_t i = 0;
  const char * typeText = NULL;
  while (releaseTypes[i].typeString != NULL) {
    if (releaseTypes[i].typeNum == versionStruct.type) {
      typeText = releaseTypes[i].typeString;
    }
    i++;
  }
  sl_zigbee_af_app_print("stack ver. [%d.%d.%d",
                         versionStruct.major,
                         versionStruct.minor,
                         versionStruct.patch);
  if (versionStruct.special != 0) {
    sl_zigbee_af_app_print(".%d",
                           versionStruct.special);
  }
  UNUSED_VAR(typeText);
  sl_zigbee_af_app_println(" %s build %d]",
                           (typeText == NULL
                            ? "???"
                            : typeText),
                           versionStruct.build);
  sl_zigbee_af_app_flush();
}

//------------------------------------------------------------------------------
// Static functions

static uint8_t getMessageSentCallbackIndex(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++) {
    if (messageSentCallbacks[i].tag == INVALID_MESSAGE_TAG) {
      return i;
    }
  }

  return INVALID_CALLBACK_INDEX;
}

static void invalidateMessageSentCallbackEntry(uint16_t tag)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++) {
    if (messageSentCallbacks[i].tag == tag) {
      messageSentCallbacks[i].tag = INVALID_MESSAGE_TAG;
      messageSentCallbacks[i].callback = NULL;
      return;
    }
  }
}

static sl_zigbee_af_message_sent_function_t getMessageSentCallback(uint16_t tag)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE; i++) {
    if (messageSentCallbacks[i].tag == tag) {
      return messageSentCallbacks[i].callback;
    }
  }

  return NULL;
}

// Helper function to setup endpoint network and profile
static sl_status_t setupEndpointNetworkAndProfile(sl_zigbee_aps_frame_t *apsFrame)
{
  sl_zigbee_af_endpoint_info_struct_t endpointInfo;
  uint8_t network_index = 0;
  sl_status_t status;

  if (sl_zigbee_af_get_endpoint_info_cb(apsFrame->sourceEndpoint,
                                        &network_index,
                                        &endpointInfo)) {
    apsFrame->profileId = endpointInfo.profileId;
    return sl_zigbee_af_push_network_index(network_index);
  }

  uint8_t index = sl_zigbee_af_index_from_endpoint(apsFrame->sourceEndpoint);
  if (index == 0xFF
#if MAX_ENDPOINT_COUNT > 0
      || (index >= MAX_ENDPOINT_COUNT)
#endif
      ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  status = sl_zigbee_af_push_endpoint_network_index(apsFrame->sourceEndpoint);
  if (status != SL_STATUS_OK) {
    return status;
  }

  apsFrame->profileId = sl_zigbee_af_profile_id_from_index(index);
  return SL_STATUS_OK;
}

static sl_status_t send(sl_zigbee_outgoing_message_type_t type,
                        uint16_t indexOrDestination,
                        sl_zigbee_aps_frame_t *apsFrame,
                        uint16_t messageLength,
                        uint8_t *message,
                        bool broadcast,
                        sl_802154_short_addr_t alias,
                        uint8_t sequence,
                        sl_zigbee_af_message_sent_function_t callback)
{
  sl_status_t status;
  uint8_t commandId;
  uint8_t messageSentIndex;
  uint16_t messageTag = INVALID_MESSAGE_TAG;

  // The send APIs only deal with ZCL messages, so they must at least contain
  // the ZCL header.
  if (messageLength < SL_ZIGBEE_AF_ZCL_OVERHEAD) {
    return SL_STATUS_FAIL;
  } else if ((message[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) != 0U) {
    if (messageLength < SL_ZIGBEE_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD) {
      return SL_STATUS_FAIL;
    }
    commandId = message[4];
  } else {
    commandId = message[2];
  }

  messageSentIndex = getMessageSentCallbackIndex();
  if (callback != NULL && messageSentIndex == INVALID_CALLBACK_INDEX) {
    return SL_STATUS_FULL;
  }

  // The source endpoint in the APS frame MUST be valid at this point.  We use
  // it to set the appropriate outgoing network as well as the profile id in
  // the APS frame.
  status = setupEndpointNetworkAndProfile(apsFrame);
  if (status != SL_STATUS_OK) {
    return status;
  }

#ifdef SL_CATALOG_ZIGBEE_SUB_GHZ_CLIENT_PRESENT
  // If the Sub-GHz client is present and currently in the "suspended" state,
  // block any outgoing message unless it comes from the Sub-GHz client itself.
  if (sl_zigbee_af_sub_ghz_client_is_sending_zcl_messages_suspended()
      && apsFrame->clusterId != ZCL_SUB_GHZ_CLUSTER_ID
      && apsFrame->clusterId != ZCL_OTA_BOOTLOAD_CLUSTER_ID) {
    return SL_STATUS_SUSPENDED;
  }
#endif // SL_CATALOG_ZIGBEE_SUB_GHZ_CLIENT_PRESENT

  // Encryption is turned on if it is required, but not turned off if it isn't.
  // This allows the application to send encrypted messages in special cases
  // that aren't covered by the specs by manually setting the encryption bit
  // prior to calling the send APIs.
  if (sl_zigbee_af_determine_if_link_security_is_required(commandId,
                                                          false, // incoming?
                                                          broadcast,
                                                          apsFrame->profileId,
                                                          apsFrame->clusterId,
                                                          (type == SL_ZIGBEE_OUTGOING_DIRECT)
                                                          ? indexOrDestination
                                                          : SL_ZIGBEE_NULL_NODE_ID)) {
    apsFrame->options |= SL_ZIGBEE_APS_OPTION_ENCRYPTION;
  }

  {
    sl_zigbee_af_message_struct_t messageStruct = {
      callback,
      apsFrame,
      message,
      indexOrDestination,
      messageLength,
      type,
      broadcast,
    };
    // Called prior to fragmentation in case the mesasge does not go out over the
    // Zigbee radio, and instead goes to some other transport that does not require
    // low level ZigBee fragmentation.
    if (sl_zigbee_af_pre_message_send_cb(&messageStruct,
                                         &status)) {
      return status;
    }
  }

  // SE 1.4 requires an option to disable APS ACK and Default Response
  sli_zigbee_af_apply_disable_default_response(&message[0]);
  sli_zigbee_af_apply_retry_override(&apsFrame->options);

  if (messageLength
      <= sl_zigbee_af_maximum_aps_payload_length(type, indexOrDestination, apsFrame)) {
    apsFrame->options &= ~SL_ZIGBEE_APS_OPTION_FRAGMENT;
    status = sli_zigbee_af_send(type,
                                indexOrDestination,
                                apsFrame,
                                (uint8_t)messageLength,
                                message,
                                &messageTag,
                                alias,
                                sequence);
#ifdef SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
  } else if (!broadcast) {
    status = sli_zigbee_af_fragmentation_send_unicast(type,
                                                      indexOrDestination,
                                                      apsFrame,
                                                      message,
                                                      messageLength,
                                                      &messageTag);
    sl_zigbee_af_debug_println("%sstart:len=%d.", "Fragmentation:", messageLength);
#endif // SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT
  } else {
    status = SL_STATUS_MESSAGE_TOO_LONG;
  }

#ifdef SL_CATALOG_ZIGBEE_CRITICAL_MESSAGE_QUEUE_PRESENT
  // If this was a critical message queue entry, fire the callback
  if ((status != SL_STATUS_OK)
      && (callback == sl_zigbee_af_critical_message_queue_enqueue_cb
          || callback == sli_zigbee_af_critical_message_queue_retry_callback)) {
    callback(type,
             indexOrDestination,
             apsFrame,
             messageLength,
             message,
             status);
  }
#endif // SL_CATALOG_ZIGBEE_CRITICAL_MESSAGE_QUEUE_PRESENT

  if (callback != NULL
      && status == SL_STATUS_OK
      && messageTag != INVALID_MESSAGE_TAG
      && messageSentIndex < SL_ZIGBEE_AF_MESSAGE_SENT_CALLBACK_TABLE_SIZE) {
    messageSentCallbacks[messageSentIndex].tag = messageTag;
    messageSentCallbacks[messageSentIndex].callback = callback;
  }

  if (status == SL_STATUS_IN_PROGRESS
      && apsFrame->options & SL_ZIGBEE_APS_OPTION_DSA_SIGN) {
    // We consider "in progress" signed messages as being sent successfully.
    // The stack will send the message after signing.
    status = SL_STATUS_OK;
    sli_zigbee_af_set_crypto_operation_in_progress();
  }

  if (status == SL_STATUS_OK) {
    sl_zigbee_af_add_to_current_app_tasks(SL_ZIGBEE_AF_WAITING_FOR_DATA_ACK
                                          | SL_ZIGBEE_AF_WAITING_FOR_ZCL_RESPONSE);
  }

  (void) sl_zigbee_af_pop_network_index();
  return status;
}

static sl_status_t broadcastPermitJoin(uint8_t duration)
{
  return sl_zigbee_permit_joining_request(SL_ZIGBEE_BROADCAST_ADDRESS, duration, 1, 0);
}

static void printMessage(sl_zigbee_incoming_message_type_t type,
                         sl_zigbee_aps_frame_t* apsFrame,
                         uint16_t messageLength,
                         uint8_t* messageContents)
{
  (void) type;
  sl_zigbee_af_app_print("Profile: %s (0x%04X), Cluster: 0x%04X, %d bytes,",
                         (apsFrame->profileId == SL_ZIGBEE_ZDO_PROFILE_ID
                          ? "ZDO"
                          : (apsFrame->profileId == SE_PROFILE_ID
                             ? "SE"
                             : (apsFrame->profileId == 0x0104
                                ? "HA"
                                : "??"))),
                         apsFrame->profileId,
                         apsFrame->clusterId,
                         messageLength);
  if (apsFrame->profileId != SL_ZIGBEE_ZDO_PROFILE_ID
      && messageLength >= 3) {
    sl_zigbee_af_app_print(" ZCL %s Cmd ID: %d",
                           (messageContents[0] & ZCL_CLUSTER_SPECIFIC_COMMAND
                            ? "Cluster"
                            : "Global"),
                           messageContents[0] & ZCL_MANUFACTURER_SPECIFIC_MASK
                           ? messageContents[4] : messageContents[2]);
  }
  sl_zigbee_af_app_println("");
}

sl_zigbee_af_status_t sl_zigbee_af_status(bool wasHandled, bool clusterExists)
{
  if (wasHandled) {
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  } else if (clusterExists) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  } else {
    return SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_CLUSTER;
  }
}

WEAK(sl_status_t sl_zigbee_af_get_network_parameters(sl_zigbee_node_type_t* nodeType,
                                                     sl_zigbee_network_parameters_t* parameters))
{
  return sl_zigbee_get_network_parameters(nodeType, parameters);
}

//------------------------------------------------------------------------------
// Weak callbacks

WEAK(bool sl_zigbee_af_message_sent_cb(sl_zigbee_outgoing_message_type_t type,
                                       uint16_t indexOrDestination,
                                       sl_zigbee_aps_frame_t * apsFrame,
                                       uint16_t msgLen,
                                       uint8_t * message,
                                       sl_status_t status))
{
  (void) type;
  (void) indexOrDestination;
  (void) apsFrame;
  (void) msgLen;
  (void) message;
  (void) status;
  return false;
}
