/***************************************************************************//**
 * @file
 * @brief Routines for the ZLL Commissioning Server plugin, which implement the
 *        server side of the ZLL protocol.
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
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/zll-commissioning-common/zll-commissioning.h"
#include  "mac-phy.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZLL_COMMISSIONING_CLIENT_PRESENT
#define ZLL_COMMISSIONING_CLIENT_PRESENT
#endif
#include "zll-commissioning-server-config.h"
#if (SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_RX_ALWAYS_ON == 1)
#define RX_ALWAYS_ON
#endif
#if (SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_STEALING_ALLOWED == 1)
#define STEALING_ALLOWED
#endif
#if (SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_REMOTE_RESET_ALLOWED == 1)
#define REMOTE_RESET_ALLOWED
#endif

// The code below assumes that there is exactly one network and that it is
// ZigBee PRO.
#if SL_ZIGBEE_SUPPORTED_NETWORKS != 1
  #error ZLL is not supported with multiple networks.
#endif

//------------------------------------------------------------------------------
// Globals

#if !defined(STEALING_ALLOWED)
  #define STEALING_ALLOWED_BOOL (false)
#else
  #define STEALING_ALLOWED_BOOL (true)
#endif

static const bool stealingAllowed = STEALING_ALLOWED_BOOL;

#if !defined (REMOTE_RESET_ALLOWED)
  #define REMOTE_RESET_ALLOWED_BOOL (false)
#else
  #define REMOTE_RESET_ALLOWED_BOOL (true)
#endif

static const bool remoteResetAllowed = REMOTE_RESET_ALLOWED_BOOL;

#ifdef PLUGIN_DEBUG
static const uint8_t sli_zigbee_af_zll_commissioning_plugin_name[] = "ZLL Commissioning Server";
#define PLUGIN_NAME sli_zigbee_af_zll_commissioning_plugin_name
#endif

#ifdef RX_ALWAYS_ON
  #define RX_ON_PERIOD_MS 0xFFFFFFFFUL
#else
  #define RX_ON_PERIOD_MS (SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_RX_ON_AT_STARTUP_PERIOD * 1000UL)
#endif // RX_ALWAYS_ON

// The target network - used here for the deviceInformationRequestHandler to
// determine if the request was from a legacy device.
extern sl_zigbee_zll_network_t sli_zigbee_af_zll_network;

// Private ZLL commissioning functions
bool sli_zigbee_af_zll_am_factory_new(void);
sl_zigbee_node_type_t sli_zigbee_af_zll_get_logical_node_type(void);
sl_zigbee_zll_policy_t sli_zigbee_af_zll_get_policy(void);

//------------------------------------------------------------------------------
// Module private functions

static void identifyRequestHandler(const sl_802154_long_addr_t source,
                                   uint32_t transaction,
                                   uint16_t identifyDurationS)
{
  (void)source;
  if ((sli_zigbee_af_zll_get_policy() & SL_ZIGBEE_ZLL_POLICY_STEALING_ENABLED)
      || sli_zigbee_af_zll_am_factory_new()) {
    sl_zigbee_af_zll_commissioning_cluster_println("RX: IdentifyRequest 0x%08X, 0x%04X",
                                                   transaction,
                                                   identifyDurationS);
    sl_zigbee_af_zll_commissioning_server_identify_cb(identifyDurationS);
  } else {
    sl_zigbee_af_zll_commissioning_cluster_println("ignoring identify request as touchlinking policy is disabled for NFN");
  }
}

static void resetToFactoryNewRequestHandler(const sl_802154_long_addr_t source,
                                            uint32_t transaction)
{
  (void)source;
  sl_zigbee_af_zll_commissioning_cluster_println("RX: ResetToFactoryNewRequest 0x%08X",
                                                 transaction);
  if (!sli_zigbee_af_zll_am_factory_new()) {
    sl_zigbee_current_security_state_t securityState;

    // Remote reset may be disabled, but only for a device on a centralized network.
    if (sl_zigbee_get_current_security_state(&securityState) != SL_STATUS_OK  // not joined
        || (securityState.bitmask & SL_ZIGBEE_DISTRIBUTED_TRUST_CENTER_MODE)
        || (sli_zigbee_af_zll_get_policy() & SL_ZIGBEE_ZLL_POLICY_REMOTE_RESET_ENABLED)) {
      sli_zigbee_af_zll_flags = TOUCH_LINK_TARGET;  // note reset via interpan
      sl_zigbee_af_zll_reset_to_factory_new();
    }
  }
}

//------------------------------------------------------------------------------
// ZLL commissioning private functions

bool sli_zigbee_af_zll_stealing_allowed(void)
{
  return stealingAllowed;
}

bool sli_zigbee_af_zll_remote_reset_allowed(void)
{
  return remoteResetAllowed;
}

void sli_zigbee_af_zll_initialize_radio(void)
{
  // Turn the receiver on for a user-specified period, in order to allow incoming.
  // scan requests, but cancel any outstanding rx on period first, in case we've
  // got here by a touchlink reset.
  sl_zigbee_zll_cancel_rx_on_when_idle(); // cancel rx on
#if RX_ON_PERIOD_MS != 0
  sl_zigbee_af_app_println("Setting rx on period to %d", RX_ON_PERIOD_MS);
  sl_zigbee_zll_set_radio_idle_mode(SL_ZIGBEE_RADIO_POWER_MODE_OFF);// UNIFIEDMAC set the initial state of radio to off, so that this stays on only for the given time
  sl_zigbee_zll_set_rx_on_when_idle(RX_ON_PERIOD_MS);
#endif

  // Tune the radio to the user-specified default channel - this will be used
  // to listen for an incoming scan request. (Note that the original ZLL specification
  // mandates a random primary channel, but the new BDB spec appears not to)
  sl_zigbee_set_logical_and_radio_channel(SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_DEFAULT_RADIO_CHANNEL);
  sl_zigbee_af_app_println("Setting default channel to %d", SL_ZIGBEE_AF_PLUGIN_ZLL_COMMISSIONING_SERVER_DEFAULT_RADIO_CHANNEL);
}

bool sli_zigbee_af_zll_commissioning_server_interpan_pre_message_received_callback(const sl_zigbee_af_interpan_header_t *header,
                                                                                   uint8_t msgLen,
                                                                                   uint8_t *message)
{
  uint32_t transaction;
  uint8_t frameControl, commandId, msgIndex;

  // If the message isn't for the ZLL Commissioning cluster, drop it with an
  // indication that we didn't handle it.
  if (header->profileId != SL_ZIGBEE_ZLL_PROFILE_ID
      || header->clusterId != ZCL_ZLL_COMMISSIONING_CLUSTER_ID) {
    return false;
  }

  if (header->messageType != SL_ZIGBEE_AF_INTER_PAN_UNICAST
      || !(header->options & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)
      || msgLen < ZLL_HEADER_OVERHEAD) {
    return false;
  }

  // Verify that the frame control byte makes sense.  Accept only the legacy
  // format or simple client-to-server or server-to-client messages (i.e., no
  // manufacturer-specific commands, etc.)  For non-legacy messages, check that
  // the frame control is correct for the command.  The check is based on
  // DeviceInformationResponse because it is the only server-to-client command
  // we care about.
  frameControl = message[ZLL_HEADER_FRAME_CONTROL_OFFSET];
  commandId = message[ZLL_HEADER_COMMAND_ID_OFFSET];
  if (frameControl != ZLL_FRAME_CONTROL_LEGACY
      && frameControl != ZLL_FRAME_CONTROL_CLIENT_TO_SERVER) {
    return false;
  }

  msgIndex = ZLL_HEADER_TRANSACTION_ID_OFFSET;
  transaction = sl_zigbee_af_get_int32u(message, msgIndex, msgLen);
  msgIndex += 4;

  switch (commandId) {
    case ZCL_IDENTIFY_REQUEST_COMMAND_ID:
    {
      if (msgIndex + 2 <= msgLen) {
        uint16_t identifyDurationS = sl_zigbee_af_get_int16u(message, msgIndex, msgLen);
        identifyRequestHandler(header->longAddress,
                               transaction,
                               identifyDurationS);
      }
      return true;
    }
    case ZCL_RESET_TO_FACTORY_NEW_REQUEST_COMMAND_ID:
    {
      // Ignore reset request if we are a co-ordinator
      if (sli_zigbee_af_zll_get_logical_node_type() != SL_ZIGBEE_COORDINATOR) {
        resetToFactoryNewRequestHandler(header->longAddress, transaction);
      }
      return true;
    }
  }

  // Return false for unhandled messages, since we are multiplexing the pre-message handler.
  return false;
}

//------------------------------------------------------------------------------
// Public functions

sl_status_t sl_zigbee_af_zll_no_touchlink_for_nfn(void)
{
  // Note, to reenable stealing, it is necessary to call sl_zigbee_af_zll_enable().
  sl_zigbee_zll_policy_t policy = sli_zigbee_af_zll_get_policy();
  sl_status_t status;
#ifndef EZSP_HOST
  status = sl_zigbee_zll_set_policy(policy & ~SL_ZIGBEE_ZLL_POLICY_STEALING_ENABLED);
#else
  status = sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_ZLL_POLICY,
                                        policy & ~SL_ZIGBEE_ZLL_POLICY_STEALING_ENABLED,
                                        "ZLL policy with no touchlink for NFN",
                                        "enable");
#endif
  return status;
}

sl_status_t sl_zigbee_af_zll_no_reset_for_nfn(void)
{
  // Note, to reenable remote reset, it is necessary to call sl_zigbee_af_zll_enable().
  // Note also that the policy is only honoured if the device is on a centralized
  // security network.
  sl_zigbee_zll_policy_t policy = sli_zigbee_af_zll_get_policy();
  sl_status_t status;
#ifndef EZSP_HOST
  status = sl_zigbee_zll_set_policy(policy & ~SL_ZIGBEE_ZLL_POLICY_REMOTE_RESET_ENABLED);
#else
  status = sl_zigbee_af_set_ezsp_policy(SL_ZIGBEE_EZSP_ZLL_POLICY,
                                        policy & ~SL_ZIGBEE_ZLL_POLICY_REMOTE_RESET_ENABLED,
                                        "ZLL policy with no remote reset for NFN",
                                        "enable");
#endif
  return status;
}

void sli_zigbee_af_zll_touch_link_target_callback(sl_zigbee_zll_network_t *networkInfo)
{
  debugPrintln("%s: touchlink target handler", PLUGIN_NAME);
  memmove(&sli_zigbee_af_zll_network, networkInfo, sizeof(sl_zigbee_zll_network_t));
#ifdef ZLL_COMMISSIONING_CLIENT_PRESENT
  sli_zigbee_af_zll_sub_device_count = 0;
#endif
  sli_zigbee_af_zll_flags = TOUCH_LINK_TARGET;
}
