/***************************************************************************//**
 * @file
 * @brief Routines for the Green Power Client plugin.
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "zap-cluster-command-parser.h"
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-adapter.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

#include "green-power-client.h"
#include "stack/include/gp-types.h"
#include "stack/gp/sli-gp-proxy-table.h"
#include "stack/include/gp-proxy-table.h"
#include "app/framework/plugin/green-power-common/green-power-common.h"

#ifdef EZSP_HOST
  #define sl_zigbee_gpep_incoming_message_handler sl_zigbee_ezsp_gpep_incoming_message_handler
  #define sli_zigbee_stack_d_gp_sent_handler sl_zigbee_ezsp_d_gp_sent_handler
#ifdef SL_ZIGBEE_GP_PROXY_TABLE_SIZE
#warn "SL_ZIGBEE_GP_PROXY_TABLE_SIZE must be configured on the NCP using the zigbee_gp component. The user-supplied SL_ZIGBEE_GP_PROXY_TABLE_SIZE value is being ignored."
#endif //SL_ZIGBEE_GP_PROXY_TABLE_SIZE
static uint8_t gpProxyTableSize = 0;
  #define SL_ZIGBEE_GP_PROXY_TABLE_SIZE gpProxyTableSize
#endif

#define MIN_RESERVED_GPD_COMMAND_ID 0xE4
#define MAX_RESERVED_GPD_COMMAND_ID 0xEF
#define SL_ZIGBEE_ZCL_GP_GPDF_RESERVED_MIN 0xF7
#define SL_ZIGBEE_ZCL_GP_GPDF_RESERVED_MAX  0xFF

#define GP_PROXY_TABLE_RESPONSE_ENTRIES_OFFSET 3
#define GP_NON_MANUFACTURER_ZCL_HEADER_LENGTH 3

// Channel event timeout is 5 seconds
#define GP_CHANNEL_EVENT_TIMEOUT_IN_MSEC  (5 * MILLISECOND_TICKS_PER_SECOND)

#define isNodeIdValid(nodeId) ((nodeId) < SL_ZIGBEE_DISCOVERY_ACTIVE_NODE_ID)

#ifdef EZSP_HOST
#define sli_zigbee_af_set_add_delay(delay) (void)0
#else
void sli_zigbee_af_set_add_delay(uint16_t delay);
#endif // EZSP_HOST

static sl_zigbee_af_green_power_client_commissioning_state_t commissioningState;
static sl_zigbee_af_green_power_duplicate_filter_t duplicateFilter;

static sl_zigbee_af_event_t exitCommissioningEvent;
static sl_zigbee_af_event_t channelEvent;
void channelEventHandler(sl_zigbee_af_event_t * event);
void exitCommissioningEventHandler(sl_zigbee_af_event_t * event);
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_ENABLE_BIDIRECTIONAL_OPERATION == 1)
#define ENABLE_BIDIRECTIONAL_OPERATION
#endif

static void flushGpTxQueue(void)
{
  sl_zigbee_gp_address_t addr;
  // Use 0xFFFFFFFF wildcard to clear out the gpTxQueue
  addr.applicationId = SL_ZIGBEE_GP_APPLICATION_SOURCE_ID;
  addr.id.sourceId = GP_GPD_SRC_ID_WILDCARD;
  sl_zigbee_d_gp_send(false, false, &addr, 0, 0, NULL, 0, 0);
}

static void sendChannelConfigToGpStub(void)
{
  uint8_t payload = (sl_zigbee_af_get_radio_channel() - 11) // The channel is coded as : Operational Channel - 11
                    | 0x10; //basic
  sl_zigbee_gp_address_t addr;
  addr.id.sourceId = GP_GPD_SRC_ID_FOR_CAHNNEL_CONFIG;
  addr.applicationId = SL_ZIGBEE_GP_APPLICATION_SOURCE_ID;
  sl_zigbee_d_gp_send(true, //add
                      false,//no CCA
                      &addr,
                      SL_ZIGBEE_ZCL_GP_GPDF_CHANNEL_CONFIGURATION,
                      sizeof(payload), //length
                      &(payload),
                      0, //gpepHandle
                      0); //entryLifetimeMs
}

void sl_zigbee_af_green_power_client_init_cb(uint8_t init_level)
{
  switch (init_level) {
    case SL_ZIGBEE_INIT_LEVEL_EVENT:
    {
      sl_zigbee_af_event_init(&exitCommissioningEvent,
                              exitCommissioningEventHandler);
      sl_zigbee_af_event_init(&channelEvent,
                              channelEventHandler);
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA:
    {
      memset(&commissioningState,
             0x00,
             sizeof(sl_zigbee_af_green_power_client_commissioning_state_t));
      memset(&duplicateFilter, 0x00, sizeof(sl_zigbee_af_green_power_duplicate_filter_t));
      commissioningState.gppCommissioningWindow = SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_GPP_COMMISSIONING_WINDOW;
      break;
    }

    case SL_ZIGBEE_INIT_LEVEL_DONE:
    {
      // Write the max proxy table entries attribute, since we currently have no
      // way to configure it at compile time.
#ifdef EZSP_HOST
      uint16_t stackProxyTableSize = 0;
      if (SL_ZIGBEE_EZSP_SUCCESS != sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_GP_PROXY_TABLE_SIZE, (uint16_t*)&stackProxyTableSize)) {
        sl_zigbee_af_green_power_cluster_println("ERR: Cannot get the proxy table size from GP stack.");
      }
      gpProxyTableSize = (uint8_t)stackProxyTableSize;
#endif //EZSP_HOST
      uint8_t proxyTableSize = SL_ZIGBEE_GP_PROXY_TABLE_SIZE;
      sl_zigbee_af_status_t UNUSED status
        = sl_zigbee_af_write_client_attribute(SL_ZIGBEE_GP_ENDPOINT,
                                              ZCL_GREEN_POWER_CLUSTER_ID,
                                              ZCL_GP_CLIENT_GPP_MAX_PROXY_TABLE_ENTRIES_ATTRIBUTE_ID,
                                              &proxyTableSize,
                                              ZCL_INT8U_ATTRIBUTE_TYPE);
      break;
    }
  }
}

static void exitCommissioningMode(void)
{
  commissioningState.inCommissioningMode = false;
  sl_zigbee_af_green_power_cluster_println("Exit comm for sink %04X", commissioningState.commissioningSink);

  flushGpTxQueue();
}

void channelEventHandler(sl_zigbee_af_event_t * event)
{
  (void)event;
  if (commissioningState.channelStatus & GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK) {
    if (commissioningState.channelStatus & GP_CLIENT_ADDITIONAL_CHANNEL_REQUEST_PENDING) {
      commissioningState.channelStatus &= ~GP_CLIENT_ADDITIONAL_CHANNEL_REQUEST_PENDING;
      sendChannelConfigToGpStub();
      sl_zigbee_af_event_set_delay_ms(&channelEvent,
                                      GP_CHANNEL_EVENT_TIMEOUT_IN_MSEC);
      return;
    }
    sl_zigbee_af_green_power_cluster_println("return to chan %d", sl_zigbee_af_get_radio_channel());
    sl_zigbee_stop_scan();
    commissioningState.channelStatus = 0;
    flushGpTxQueue();
  }

  sl_zigbee_af_event_set_inactive(&channelEvent);
}

void exitCommissioningEventHandler(sl_zigbee_af_event_t * event)
{
  (void)event;
  exitCommissioningMode();

  sl_zigbee_af_event_set_inactive(&exitCommissioningEvent);
}

static uint16_t storeProxyTableEntry(sl_zigbee_gp_proxy_table_entry_t *entry,
                                     uint8_t *buffer)
{
  uint8_t *finger = buffer;
  //uint8_t securityLevel = entry->securityOptions & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_SECURITY_OPTIONS_SECURITY_LEVEL;
  //uint8_t securityKeyType = entry->securityOptions & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_SECURITY_OPTIONS_SECURITY_KEY_TYPE;

  sl_zigbee_af_copy_int16u(finger, 0, (uint16_t)(entry->options & 0xFFFF));
  finger += sizeof(uint16_t);
  if (entry->gpd.applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
    sl_zigbee_af_copy_int32u(finger, 0, entry->gpd.id.sourceId);
    finger += sizeof(uint32_t);
  } else if (entry->gpd.applicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    memmove(finger, entry->gpd.id.gpdIeeeAddress, EUI64_SIZE);
    finger += EUI64_SIZE;
  }

  if (entry->gpd.applicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_af_copy_int8u(finger, 0, entry->gpd.endpoint);
    finger += sizeof(uint8_t);
  }

  if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_ASSIGNED_ALIAS) {
    sl_zigbee_af_copy_int16u(finger, 0, entry->assignedAlias);
    finger += sizeof(uint16_t);
  }

  if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_SECURITY_USE) {
    sl_zigbee_af_copy_int8u(finger, 0, entry->securityOptions);
    finger += sizeof(uint8_t);
    sl_zigbee_af_copy_int32u(finger, 0, entry->gpdSecurityFrameCounter);
    finger += sizeof(uint32_t);
    memmove(finger, entry->gpdKey.contents, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
    finger += SL_ZIGBEE_ENCRYPTION_KEY_SIZE;
  } else if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_SEQUENCE_NUMBER_CAP) {
    sl_zigbee_af_copy_int32u(finger, 0, entry->gpdSecurityFrameCounter);
    finger += sizeof(uint32_t);
  }

  // Lightweight sink address list
  if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_LIGHTWEIGHT_UNICAST_GPS) {
    // let's count
    uint8_t index = 0;
    uint8_t *entryCount = finger;
    sl_zigbee_af_copy_int8u(finger, 0, 0x00);
    finger += sizeof(uint8_t);
    for (index = 0; index < GP_SINK_LIST_ENTRIES; index++) {
      sl_zigbee_gp_sink_list_entry_t * sinkEntry = &entry->sinkList[index];

      if (sinkEntry->type == SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST) {
        memmove(finger, sinkEntry->target.unicast.sinkEUI, EUI64_SIZE);
        finger += EUI64_SIZE;
        sl_zigbee_af_copy_int16u(finger, 0, sinkEntry->target.unicast.sinkNodeId);
        finger += sizeof(uint16_t);
        (*entryCount)++;
      } else {
        continue;
      }
    }
  }

  // Sink group list
  if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_COMMISIONED_GROUP_GPS) {
    // let's count
    uint8_t index = 0;
    uint8_t *entryCount = finger;
    sl_zigbee_af_copy_int8u(finger, 0, 0x00);
    finger += sizeof(uint8_t);
    for (index = 0; index < GP_SINK_LIST_ENTRIES; index++) {
      sl_zigbee_gp_sink_list_entry_t * sinkEntry = &entry->sinkList[index];

      if (sinkEntry->type == SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST) {
        sl_zigbee_af_copy_int16u(finger, 0, sinkEntry->target.groupcast.groupID);
        finger += sizeof(uint16_t);
        sl_zigbee_af_copy_int16u(finger, 0, sinkEntry->target.groupcast.alias);
        finger += sizeof(uint16_t);
        (*entryCount)++;
      } else {
        continue;
      }
    }
  }

  sl_zigbee_af_copy_int8u(finger, 0, entry->groupcastRadius);
  finger += sizeof(uint8_t);

  if ((entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_ENTRY_ACTIVE) == 0
      || (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_ENTRY_VALID) == 0) {
    sl_zigbee_af_copy_int8u(finger, 0, entry->searchCounter);
    finger += sizeof(uint8_t);
  }

  if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_EXTENSION) {
    sl_zigbee_af_copy_int16u(finger, 0, (entry->options >> 16) & 0xFFFF);
    finger += sizeof(uint16_t);
  }

  // full unicast sink address list
  if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_FULL_UNICAST_GPS) {
    // let's count
    uint8_t index = 0;
    uint8_t *len = finger;
    sl_zigbee_af_copy_int8u(finger, 0, 0); // Add zero length
    finger += sizeof(uint8_t);
    for (index = 0; index < GP_SINK_LIST_ENTRIES; index++) {
      sl_zigbee_gp_sink_list_entry_t * sinkEntry = &entry->sinkList[index];

      if (sinkEntry->type == SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST) {
        memmove(finger, sinkEntry->target.unicast.sinkEUI, EUI64_SIZE);
        finger += EUI64_SIZE;
        sl_zigbee_af_copy_int16u(finger, 0, sinkEntry->target.unicast.sinkNodeId);
        finger += sizeof(uint16_t);
        *len += EUI64_SIZE + sizeof(uint16_t);
      } else {
        continue;
      }
    }
  }

  return (uint16_t)(finger - buffer);
}

//---------------------------
// Command callbacks

sl_zigbee_af_zcl_request_status_t sli_zigbee_af_green_power_client_gp_proxy_commissioning_mode_command_handler(uint8_t options,
                                                                                                               uint16_t commissioningWindow,
                                                                                                               uint8_t channel,
                                                                                                               bool localCommandLoopback)
{
  bool enterCommissioningMode = (options
                                 & SL_ZIGBEE_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_ACTION);
  // The exitMode enumeration type is mapped as [b3b2b1] = [b3b2] | [b1] of `options` fields in the Gp Proxy Commissioning Mode
  // command where (b3-b2) for `ExitMode` fields and b1 is the exit on commissioning window time expiry.
  uint8_t exitMode = (options                                                                             \
                      & (SL_ZIGBEE_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_EXIT_MODE                        \
                         | SL_ZIGBEE_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_COMMISSIONING_WINDOW_PRESENT)) \
                     >> 1;

  bool chanPresent = (options
                      & SL_ZIGBEE_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_CHANNEL_PRESENT); // should always be 0
  bool unicastCommunication = (options
                               & SL_ZIGBEE_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_UNICAST_COMMUNICATION);

  if (!localCommandLoopback && sl_zigbee_af_current_endpoint() != SL_ZIGBEE_GP_ENDPOINT) {
  } else if (!localCommandLoopback
             && commissioningState.inCommissioningMode
             && (sli_zigbee_af_get_sender() != commissioningState.commissioningSink)) {
    // check if current message sender is same as sender that put us in
    // commissioning mode.
    // if not, drop message silently.
  } else if (enterCommissioningMode) {
    commissioningState.commissioningSink = (localCommandLoopback ? sl_zigbee_get_node_id() : sli_zigbee_af_get_sender());
    commissioningState.inCommissioningMode = true;
    commissioningState.gppCommissioningWindow = SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_GPP_COMMISSIONING_WINDOW;
    commissioningState.exitMode = (sl_zigbee_af_green_power_client_commissioning_exit_mode_t) exitMode;
    commissioningState.unicastCommunication = unicastCommunication;
    sl_zigbee_af_green_power_cluster_println("Enter comm for sink %04X", commissioningState.commissioningSink);
    if (commissioningState.exitMode & SL_ZIGBEE_AF_GPC_COMMISSIONING_EXIT_ON_COMMISSIONING_WINDOW_EXP) {
      // store the new commissioning window value and update our local default
      // one.
      sl_zigbee_af_event_set_delay_ms(&exitCommissioningEvent,
                                      (commissioningWindow * MILLISECOND_TICKS_PER_SECOND));
    } else {
      sl_zigbee_af_event_set_delay_ms(&exitCommissioningEvent,
                                      (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_GPP_COMMISSIONING_WINDOW * MILLISECOND_TICKS_PER_SECOND));
    }
    if (chanPresent) {
      commissioningState.channel = channel;
      // TODO: goto channel
    } else {
      // TODO: goto operational channel
    }
  } else {
    // exit commissioning mode.
    exitCommissioningMode();
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

/*
 * Disable default response bit should be set per GP Spec 14-0563-08
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_green_power_cluster_gp_proxy_commissioning_mode_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_green_power_cluster_gp_proxy_commissioning_mode_command_t cmd_data;

  if (zcl_decode_green_power_cluster_gp_proxy_commissioning_mode_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  return sli_zigbee_af_green_power_client_gp_proxy_commissioning_mode_command_handler(cmd_data.options,
                                                                                      cmd_data.commissioningWindow,
                                                                                      cmd_data.channel,
                                                                                      false);
}

/*
 * Disable default response should be enabled in the sending message.
 */
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_green_power_cluster_gp_pairing_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_green_power_cluster_gp_pairing_command_t cmd_data;
  uint8_t securityLevel;
  uint8_t commMode;
  uint8_t securityKeyType;
  sl_zigbee_gp_address_t addr;
  bool createdPairing = false;
  bool broadcast = false;

  if (zcl_decode_green_power_cluster_gp_pairing_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  securityLevel = ((cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_SECURITY_LEVEL)
                   >> SL_ZIGBEE_AF_GP_PAIRING_OPTION_SECURITY_LEVEL_OFFSET);
  commMode = ((cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_COMMUNICATION_MODE)
              >> SL_ZIGBEE_AF_GP_PAIRING_OPTION_COMMUNICATION_MODE_OFFSET);
  securityKeyType = ((cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_SECURITY_KEY_TYPE)
                     >> SL_ZIGBEE_AF_GP_PAIRING_OPTION_SECURITY_KEY_TYPE_OFFSET);

  // If "option & addSinkBit(3) == 1" AND "groupcastRadiusBit(17)==0"
  // then groupcastRadius = 0 not 0xFF that was passed by call-command-handler
  if ((cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_ADD_SINK)
      && !(cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_GROUPCAST_RADIUS_PRESENT)) {
    cmd_data.groupcastRadius = 0;
  }

  if (cmd->type == SL_ZIGBEE_INCOMING_BROADCAST) {
    broadcast = true;
  }
  sl_zigbee_gp_application_id_t appId = (cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_APPLICATION_ID);
  if (!sli_zigbee_af_gp_make_addr(&addr,
                                  appId,
                                  cmd_data.gpdSrcId,
                                  cmd_data.gpdIeee,
                                  cmd_data.endpoint)
      || IS_GPD_SRC_ID_ZERO(appId, cmd_data.gpdSrcId)) {
    // GPD Address or App Id are invalid, so drop the command
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  sl_zigbee_af_core_print("GP Pairing\n");

  // Commissioning: Step 21a: Proxy finalizes commissioning
  if (sl_zigbee_af_current_endpoint() != SL_ZIGBEE_GP_ENDPOINT) {
  } else if (securityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_RESERVED) {
  } else if ((cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_ADD_SINK)
             && (cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_REMOVE_GPD)) {
  } else if ((securityKeyType > SL_ZIGBEE_GP_SECURITY_KEY_GPD_OOB)
             &&  (securityKeyType < SL_ZIGBEE_GP_SECURITY_KEY_GPD_DERIVED)) {
  } else if ((commMode == SL_ZIGBEE_ZCL_GP_PAIRING_OPTIONS_COMMUNICATION_MODE_FULL_UNICAST_FORWARDING)
             && (!(cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_REMOVE_GPD))) {
    // CCB #2278 -In case of unicast GP Pairing, the proxy SHALL respond with ZCL Default
    //  Response with relevant error code: INVALID_FIELD for Full unicast;
    //  -In case of broadcast GP Pairing, the proxy SHALL silently drop the frame.
    //  If RemoveGPD options bit is set the comms mode is not valid so ignore this test
    if (broadcast == true) {
      return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
      // Silently drop it as its a broadcast
    } else {
      //send default response for unicast pairing
      sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD);
    }
  } else {
    // Step b:

    sl_zigbee_af_green_power_cluster_println("GP Pairing SRCID = %08X, Opt= %u Sink = %04X", addr.id.sourceId, cmd_data.options, cmd_data.sinkNwkAddress);

    createdPairing = sl_zigbee_gp_proxy_table_process_gp_pairing(cmd_data.options,
                                                                 &addr,
                                                                 commMode,
                                                                 cmd_data.sinkNwkAddress,
                                                                 cmd_data.sinkGroupId,
                                                                 cmd_data.assignedAlias,
                                                                 cmd_data.sinkIeeeAddress,
                                                                 (sl_zigbee_key_data_t *) cmd_data.gpdKey,
                                                                 cmd_data.gpdSecurityFrameCounter,
                                                                 cmd_data.groupcastRadius);

    // step c: optionally, exit commissioning mode
    if (createdPairing
        && (commissioningState.exitMode & SL_ZIGBEE_AF_GPC_COMMISSIONING_EXIT_ON_FIRST_PAIRING_SUCCESS)) {
      exitCommissioningMode();
    } else if ((cmd_data.options & SL_ZIGBEE_AF_GP_PAIRING_OPTION_ADD_SINK) && !createdPairing) {
#ifndef EZSP_HOST
      uint8_t index = sli_zigbee_gp_proxy_table_get_free_entry_index();  //sli_zigbee_gp_proxy_table_find_or_allocate_entry(&addr);   //checking if pairing failed because of no
      // space
      if (index == 0xFF) {
        sl_zigbee_af_green_power_cluster_println("ERR PROXY TABLE FULL");
        if (broadcast != true) {
          // CCB # 2279 - only send default response if not broadcast - broadcasts are dropped
          sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE);
        }
      }
    }
#else
      // Checking if the proxy table is full is more of a pain on Host
      // We have to search through the whole table.
      // Because of this, we only do it for the non-broadcast case
      // Next time we bump the EZSP version, we should change the
      // return of Process GP Pairing to a status instead of a bool
      if (!broadcast) {
        sl_zigbee_gp_proxy_table_entry_t entry;
        for (uint8_t entryIndex = 0; entryIndex < SL_ZIGBEE_GP_PROXY_TABLE_SIZE; entryIndex++) {
          sl_zigbee_gp_proxy_table_get_entry(entryIndex, &entry);
          if (entry.status == SL_ZIGBEE_GP_PROXY_TABLE_ENTRY_STATUS_UNUSED) {
            return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED; // We have an unused entry so we don't need to send a DR for full
          }
        }
        sl_zigbee_af_green_power_cluster_println("ERR PROXY TABLE FULL");
        sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE);
      }
    }
#endif
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_green_power_cluster_gp_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_green_power_cluster_gp_response_command_t cmd_data;

  if (zcl_decode_green_power_cluster_gp_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_gp_application_id_t appId = (cmd_data.options & SL_ZIGBEE_AF_GP_RESPONSE_OPTION_APPLICATION_ID);

  if (sl_zigbee_af_current_endpoint() != SL_ZIGBEE_GP_ENDPOINT) {
    goto kickout;
  }
  sl_zigbee_af_green_power_cluster_println("GpResponse cmd: %1x", cmd_data.gpdCommandId);

  sl_zigbee_gp_address_t addr;
  if (!sli_zigbee_af_gp_make_addr(&addr,
                                  appId,
                                  cmd_data.gpdSrcId,
                                  cmd_data.gpdIeee,
                                  cmd_data.endpoint)
      || (IS_GPD_SRC_ID_ZERO(appId, cmd_data.gpdSrcId)
          && cmd_data.gpdCommandId != SL_ZIGBEE_ZCL_GP_GPDF_CHANNEL_CONFIGURATION)
      || (!IS_GPD_SRC_ID_ZERO(appId, cmd_data.gpdSrcId)
          && cmd_data.gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_CHANNEL_CONFIGURATION)) {
    // GPD Address or App Id are invalid and
    // SrcId is only 0 for Channel configuration, else drop
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }
  sl_status_t result;
  switch (cmd_data.gpdCommandId) {
    case SL_ZIGBEE_ZCL_GP_GPDF_CHANNEL_CONFIGURATION:
    {
      if (cmd_data.tempMasterShortAddress == sl_zigbee_get_node_id()) {
        if (!sl_zigbee_af_event_is_scheduled(&channelEvent)
            || (commissioningState.channelStatus & GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK)) {
          sendChannelConfigToGpStub();
          commissioningState.channelStatus |= GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK;
          // based on figure 88 of GP proxy basic spec V1.0 page 191
          // the following condition seems neccessary
          if (sl_zigbee_af_get_radio_channel() != (cmd_data.tempMasterTxChannel + 11) ) {
            sl_zigbee_af_green_power_cluster_println("GP switch to chan %d", cmd_data.tempMasterTxChannel + 11);
            sl_zigbee_start_scan(SL_ZIGBEE_STACK_GP_CHANNEL_DELIVERY_SCAN, (1 << (cmd_data.tempMasterTxChannel + 11)), 0 /*duration*/);
            commissioningState.channelStatus &= ~GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK;
          } else {
            if (commissioningState.channelStatus & GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK) {
              commissioningState.channelStatus |= GP_CLIENT_ADDITIONAL_CHANNEL_REQUEST_PENDING;
            }
            commissioningState.channelStatus |= GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK;
          }
          // Stared timeout to clear the Tx Queue if the packet is not delivered,

          sl_zigbee_af_event_set_delay_ms(&channelEvent,
                                          GP_CHANNEL_EVENT_TIMEOUT_IN_MSEC);
          return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
        } else {
          return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
        }
      } else {
        //TODO clear firstToForward
        channelEventHandler(&channelEvent);
        //sl_zigbee_d_gp_send(false, false, &addr, 0, 0, NULL, 0, 0);
      }
      break;
    }
    case SL_ZIGBEE_ZCL_GP_GPDF_COMMISSIONING_REPLY:
      //TODO IEEE GPD
    {
      if (cmd_data.tempMasterShortAddress == sl_zigbee_get_node_id()) {
        sl_zigbee_af_green_power_cluster_println("dgpsend comm reply %s", "A");
        result = sl_zigbee_d_gp_send(true, //add
                                     false, //skip cca
                                     &addr,
                                     SL_ZIGBEE_ZCL_GP_GPDF_COMMISSIONING_REPLY,
                                     cmd_data.gpdCommandPayload[0],
                                     cmd_data.gpdCommandPayload + 1,
                                     0, 0
                                     );
        if (result == SL_STATUS_OK) {
          sl_zigbee_af_green_power_cluster_println("success");
        } else {
          sl_zigbee_af_green_power_cluster_println("fail %02X", result);
        }
      } else {
        sl_zigbee_af_green_power_cluster_println("Flush GP TX");
        sl_zigbee_d_gp_send(false, false, &addr, 0, 0, NULL, 0, 0);
      }
      break;
    }
    default:
    {
      if ((cmd_data.gpdCommandId >= SL_ZIGBEE_ZCL_GP_GPDF_RESERVED_MIN) //and implicitly check gpdCommandId <= SL_ZIGBEE_ZCL_GP_GPDF_RESERVED_MAX
          || (cmd_data.gpdCommandId >= SL_ZIGBEE_ZCL_GP_GPDF_MFR_DEF_GPD_CMD0
              && cmd_data.gpdCommandId <= SL_ZIGBEE_ZCL_GP_GPDF_MFR_DEF_GPD_CMD_F)) {
        sl_zigbee_af_green_power_cluster_println("dgpsend comm reply %s", "B");
        result = sl_zigbee_d_gp_send(true,   //add
                                     false, //skip cca
                                     &addr,
                                     cmd_data.gpdCommandId,
                                     cmd_data.gpdCommandPayload[0],
                                     cmd_data.gpdCommandPayload + 1,
                                     0, 0
                                     );
        if (result == SL_STATUS_OK) {
          sl_zigbee_af_green_power_cluster_println("success");
        } else {
          sl_zigbee_af_green_power_cluster_println("fail %02X", result);
        }
      }
      break;
    }
  }
  kickout: return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_green_power_cluster_gp_proxy_table_request_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_green_power_cluster_gp_proxy_table_request_command_t cmd_data;
  uint8_t validEntriesCount = 0;
  uint8_t entryIndex = 0;
  uint8_t appId;
  uint8_t requestType;
  sl_zigbee_gp_proxy_table_entry_t entry;

  if (zcl_decode_green_power_cluster_gp_proxy_table_request_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  appId = cmd_data.options & SL_ZIGBEE_AF_GP_PROXY_TABLE_REQUEST_OPTIONS_APPLICATION_ID;
  requestType = (cmd_data.options & SL_ZIGBEE_AF_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE)
                >> SL_ZIGBEE_AF_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_OFFSET;

  // only respond to unicast messages.
  if (sl_zigbee_af_current_command()->type != SL_ZIGBEE_INCOMING_UNICAST) {
    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  if (SL_ZIGBEE_GP_PROXY_TABLE_SIZE == 0) {
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND);
    goto kickout;
  }

  if (sl_zigbee_af_current_endpoint() != SL_ZIGBEE_GP_ENDPOINT) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  for (entryIndex = 0; entryIndex < SL_ZIGBEE_GP_PROXY_TABLE_SIZE; entryIndex++) {
    if (sl_zigbee_gp_proxy_table_get_entry(entryIndex, &entry) != SL_STATUS_OK) {
      return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
    }

    if (entry.status != SL_ZIGBEE_GP_PROXY_TABLE_ENTRY_STATUS_UNUSED) {
      validEntriesCount++;
    }
  }

  if (requestType == SL_ZIGBEE_ZCL_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_BY_GPD_ID) {
    sl_zigbee_gp_address_t addr;
    if (!sli_zigbee_af_gp_make_addr(&addr,
                                    appId,
                                    cmd_data.gpdSrcId,
                                    cmd_data.gpdIeee,
                                    cmd_data.endpoint)
        || IS_GPD_SRC_ID_ZERO(appId, cmd_data.gpdSrcId)) {
      // Invalid app Id or reserved address, just respond back as not found.
      sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_response_smart(SL_ZIGBEE_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                                                  validEntriesCount,
                                                                                  0xff,
                                                                                  0);
      sli_zigbee_af_gp_send_response_unicast();
      goto kickout;
    }
    entryIndex = sl_zigbee_gp_proxy_table_lookup(&addr);
    if (entryIndex == 0xFF) {
      sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_response_smart(SL_ZIGBEE_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                                                  validEntriesCount,
                                                                                  0xff,
                                                                                  0);
      sli_zigbee_af_gp_send_response_unicast();
      goto kickout;
    } else {
      sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_response_smart(SL_ZIGBEE_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_SUCCESS,
                                                                                  validEntriesCount,
                                                                                  0xff,
                                                                                  1);
      if (sl_zigbee_gp_proxy_table_get_entry(entryIndex, &entry) != SL_STATUS_OK) {
        return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
      }

      appResponseLength
        += storeProxyTableEntry(&entry,
                                appResponseData + appResponseLength);
      sli_zigbee_af_gp_send_response_unicast();
      goto kickout;
    }
  } else if (requestType == SL_ZIGBEE_ZCL_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_BY_INDEX) {
    if (cmd_data.index >= validEntriesCount) {
      //Nothing to send
      sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_response_smart(SL_ZIGBEE_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                                                  validEntriesCount,
                                                                                  cmd_data.index,
                                                                                  0);
      sli_zigbee_af_gp_send_response_unicast();
      goto kickout;
    } else {
      sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_response_smart(SL_ZIGBEE_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_SUCCESS,
                                                                                  validEntriesCount,
                                                                                  cmd_data.index,
                                                                                  0);

      validEntriesCount = 0;
      uint16_t entriesCount = 0;
      for (entryIndex = 0; entryIndex < SL_ZIGBEE_GP_PROXY_TABLE_SIZE; entryIndex++) {
        if (sl_zigbee_gp_proxy_table_get_entry(entryIndex, &entry) != SL_STATUS_OK) {
          return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
        }

        uint8_t  tempDatabuffer[SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN];
        uint16_t tempDataLength = 0;

        if (entry.status != SL_ZIGBEE_GP_PROXY_TABLE_ENTRY_STATUS_UNUSED) {
          validEntriesCount++;
          if (validEntriesCount > cmd_data.index) {
            // Copy to a temp buffer and add if there is spoce
            tempDataLength = storeProxyTableEntry(&entry, tempDatabuffer);

            // If space add to buffer
            if ( sizeof(appResponseData) > (appResponseLength + tempDataLength)) {
              memmove(&appResponseData[appResponseLength], tempDatabuffer, tempDataLength);
              appResponseLength +=  tempDataLength;
              entriesCount++;
            } else {
              break;
            }
          }
        }
      }

      //Insert the number of entries actually included
      appResponseData[GP_PROXY_TABLE_RESPONSE_ENTRIES_OFFSET + GP_NON_MANUFACTURER_ZCL_HEADER_LENGTH] = entriesCount;

      sl_status_t status = sli_zigbee_af_gp_send_response_unicast();

      if (status == SL_STATUS_MESSAGE_TOO_LONG) {
        sl_zigbee_af_fill_command_green_power_cluster_gp_proxy_table_response_smart(SL_ZIGBEE_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_SUCCESS,
                                                                                    validEntriesCount,
                                                                                    cmd_data.index,
                                                                                    0);
        sli_zigbee_af_gp_send_response_unicast();
      }
      goto kickout;
    }
  }

  kickout:  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

bool sli_zigbee_af_green_power_client_retrieve_attribute_and_craft_response(uint8_t endpoint,
                                                                            sl_zigbee_af_cluster_id_t clusterId,
                                                                            sl_zigbee_af_attribute_id_t attrId,
                                                                            uint8_t mask,
                                                                            uint16_t manufacturerCode,
                                                                            uint16_t readLength)
{
  uint8_t proxyTableEntryAppResponseData[SL_ZIGBEE_AF_RESPONSE_BUFFER_LEN];
  uint8_t zclStatus = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  uint16_t stringDataOffsetStart = 0;
  uint16_t stringLength = 0;
  bool    status = false;

  if (endpoint != SL_ZIGBEE_GP_ENDPOINT
      || clusterId != ZCL_GREEN_POWER_CLUSTER_ID
      || attrId != ZCL_GP_CLIENT_PROXY_TABLE_ATTRIBUTE_ID
      || mask != CLUSTER_MASK_CLIENT
      || manufacturerCode != SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) {
    // do nothing but return false
  } else if (readLength < 6) {
    // Can't fit the ZCL header in available length so exit with error
    // do nothing but return true as we are processing the correct type of packet
  } else {
    (void) sl_zigbee_af_put_int16u_in_resp(attrId);

    // The proxy table attribute is a long string ZCL attribute type, which means
    // it is encoded starting with a 2-byte length. We fill in the real length
    // after we have encoded the whole proxy table.
    // Fopur bytes extra = 2byte length + 1 byte ZCl Status + 1 byte Attribute Type
    uint16_t stringDataOffset =  appResponseLength + 4;
    stringDataOffsetStart = stringDataOffset;

    // Search the proxy table and respond with entries
    for (uint8_t i = 0; i < SL_ZIGBEE_GP_PROXY_TABLE_SIZE; i++) {
      sl_zigbee_gp_proxy_table_entry_t entry;

      if (sl_zigbee_gp_proxy_table_get_entry(i, &entry) != SL_STATUS_OK) {
        return false;
      } else if (entry.status != SL_ZIGBEE_GP_PROXY_TABLE_ENTRY_STATUS_UNUSED) {
        // Have a valid entry so encode response in temp buffer and add if it fits
        uint16_t proxyTableEntryLength = storeProxyTableEntry(&entry, proxyTableEntryAppResponseData);

        if ( (proxyTableEntryLength + stringDataOffset) > readLength) {
          // String is too big so
          zclStatus = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
          break;
        } else {
          memmove(&appResponseData[stringDataOffset], proxyTableEntryAppResponseData, proxyTableEntryLength);
          stringDataOffset += proxyTableEntryLength;
        }
      }
    }

    // calculate string length
    stringLength = stringDataOffset - stringDataOffsetStart;

    if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      (void) sl_zigbee_af_put_int8u_in_resp(zclStatus);
      (void) sl_zigbee_af_put_int8u_in_resp(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE);
      (void) sl_zigbee_af_put_int16u_in_resp(stringLength);
    } else {
      (void) sl_zigbee_af_put_int8u_in_resp(zclStatus);
    }

    appResponseLength += stringLength;
    status = true;
  }

  return status;
}

static UNUSED uint8_t qualityBasedDelay(uint8_t gpdLink)
{
  // The top 2 bits of gpdlink is operated with the mask 0xC0 (b11000000), hence needs to be shifted by 6.
  uint8_t ourLqi = (gpdLink & 0xC0) >> 6;
  // A.3.6.3.1 : QualityBasedDelay is calculated as follows:
  //  For Link quality = 0b11: 0 ms;
  //  For Link quality = 0b10: 32ms;
  //  For Link quality = 0b01: 64ms;
  //  For Link quality = 0b00: 96ms;
  // So, the return from this function is a delay expressed as b0XX00000 ms, where bXX is (3 - ourLqi) in binary
  return ((3 - ourLqi) << 5);
}

static UNUSED void gppTunnelingDelay(uint8_t bidirectionalInfo, uint8_t gpdLink)
{
#ifdef EZSP_HOST
  UNUSED_VAR(gpdLink);
#endif
  if (bidirectionalInfo & SL_ZIGBEE_GP_BIDIRECTION_INFO_RX_AFTER_TX_MASK) {
    sli_zigbee_af_set_add_delay(GP_DMIN_B + qualityBasedDelay(gpdLink));
  } else {
    sli_zigbee_af_set_add_delay(GP_DMIN_U + qualityBasedDelay(gpdLink));
  }
}

static void sendUnicastCommissioningNotification(sl_zigbee_gp_params_t *params, uint16_t options)
{
  if (sl_zigbee_af_fill_command_green_power_cluster_gp_commissioning_notification_smart(
        options,
        params->addr.id.sourceId,
        params->addr.id.gpdIeeeAddress,
        params->addr.endpoint,
        params->sequenceNumber,
        params->gpdfSecurityLevel,
        params->gpdSecurityFrameCounter,
        params->gpdCommandId,
        params->gpdCommandPayloadLength,
        params->gpdCommandPayload,
        sl_zigbee_get_node_id(),
        params->gpdLink,
        params->mic) == 0) {
    return;
  }

  sl_zigbee_aps_frame_t *apsFrame = sl_zigbee_af_get_command_aps_frame();
  apsFrame->sourceEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  apsFrame->destinationEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  gppTunnelingDelay(params->bidirectionalInfo, params->gpdLink);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, commissioningState.commissioningSink);
  sli_zigbee_af_set_add_delay(0);
}

static void sendBroadcastCommissioningNotification(sl_zigbee_gp_params_t *params, uint16_t options)
{
  if (sl_zigbee_af_fill_command_green_power_cluster_gp_commissioning_notification_smart(
        options,
        params->addr.id.sourceId,
        params->addr.id.gpdIeeeAddress,
        params->addr.endpoint,
        params->sequenceNumber,
        params->gpdfSecurityLevel,
        params->gpdSecurityFrameCounter,
        params->gpdCommandId,
        params->gpdCommandPayloadLength,
        params->gpdCommandPayload,
        sl_zigbee_get_node_id(),
        params->gpdLink,
        params->mic) == 0) {
    return;
  }

  sl_zigbee_aps_frame_t *apsFrame = sl_zigbee_af_get_command_aps_frame();
  apsFrame->sourceEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  apsFrame->destinationEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  apsFrame->sequence = params->sequenceNumber - SL_ZIGBEE_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET;
  apsFrame->options |= SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
  gppTunnelingDelay(params->bidirectionalInfo, params->gpdLink);
  sl_zigbee_af_send_command_broadcast(SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                      sli_zigbee_gpd_alias(&(params->addr)),
                                      params->sequenceNumber - SL_ZIGBEE_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET);
  sli_zigbee_af_set_add_delay(0);
}

static bool autoCommissioningCallback(sl_zigbee_gp_params_t *params)
{
  uint16_t options = 0;
  sl_zigbee_aps_frame_t *apsFrame = NULL;

  if (!commissioningState.inCommissioningMode) {
    return true;
  }

  if (params->addr.applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID
      && params->addr.id.sourceId == 0x00000000) {
    return true;
  }
  if (params->gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_COMMISSIONING || params->gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_SUCCESS) {
    return true;
  }

  if (params->bidirectionalInfo & SL_ZIGBEE_GP_BIDIRECTION_INFO_RX_AFTER_TX_MASK) {
    return true;
  }

  /* TODO
     uint8_t UNUSED proxyIndex = 0xFF;
     proxyIndex = sl_zigbee_gp_proxy_table_lookup(addr);
     if (proxyIndex != 0xFF) {
     //we have a proxy table entry for this GPD
     sli_zigbee_gp_proxy_table_set_in_range(proxyIndex);
     } else {
     //we don't have a proxy table entry
     //optionally create an active invalid entry here
     //not required for proxy basic
     }
   */

  if (params->status == SL_ZIGBEE_GP_STATUS_NO_SECURITY) {
    options = 0;
  }
  //TODO handle other cases

  options |= params->addr.applicationId;
  options |= params->gpdfSecurityLevel
             << GP_COMMISSIONING_SECURITY_LEVEL_TO_OPTIONS_SHIFT;
  options |= params->gpdfSecurityKeyType
             << GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT; //security key type
  options |= SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_PROXY_INFO_PRESENT;
  if (sl_zigbee_af_fill_command_green_power_cluster_gp_commissioning_notification_smart(options,
                                                                                        params->addr.id.sourceId,
                                                                                        params->addr.id.gpdIeeeAddress,
                                                                                        params->addr.endpoint,
                                                                                        params->sequenceNumber,
                                                                                        params->gpdfSecurityLevel,
                                                                                        params->gpdSecurityFrameCounter,
                                                                                        params->gpdCommandId,
                                                                                        params->gpdCommandPayloadLength,
                                                                                        params->gpdCommandPayload,
                                                                                        sl_zigbee_get_node_id(),
                                                                                        params->gpdLink,
                                                                                        params->mic) == 0) {
    return true;
  }

  apsFrame = sl_zigbee_af_get_command_aps_frame();
  apsFrame->sourceEndpoint = SL_ZIGBEE_GP_ENDPOINT; //sl_zigbee_af_current_endpoint();
  apsFrame->destinationEndpoint = SL_ZIGBEE_GP_ENDPOINT; //sl_zigbee_af_current_endpoint();
  gppTunnelingDelay(params->bidirectionalInfo, params->gpdLink);
  if (commissioningState.unicastCommunication) {
    sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                      commissioningState.commissioningSink);
  } else {
    apsFrame->sequence = params->sequenceNumber - SL_ZIGBEE_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET;
    apsFrame->options |= SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
    sl_zigbee_af_send_command_broadcast(SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                        sli_zigbee_gpd_alias(&(params->addr)),
                                        params->sequenceNumber - SL_ZIGBEE_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET);
  }
  sli_zigbee_af_set_add_delay(0);
  return true;
}

static bool commissioningGpdfCallback(sl_zigbee_gp_params_t *params)
{
  // RxAfterTx is true
  bool frameFound = false;
  // Per GP spec 14-0563-08:
  // Commissioning: Step 12a: Proxy receives commissioning command
  // If applicationId == 0b0000 and srcId == 0 or reserved range 0xfffffff9 - 0xffffffffe
  // OR applicationId == 0b0010 and srcId == 0, drop frame.
  if (!sli_zigbee_af_gp_make_addr(&(params->addr),
                                  params->addr.applicationId,
                                  params->addr.id.sourceId,
                                  params->addr.id.gpdIeeeAddress,
                                  params->addr.endpoint)
      || IS_GPD_SRC_ID_ZERO(params->addr.applicationId, params->addr.id.sourceId)) {
    // GPD Address or App Id are invalid, so drop the command
    goto kickout;
  }

  if (params->autoCommissioning) {
    goto kickout;
  }

  // Step b
  if (((params->gpdfSecurityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC)
       || (params->gpdfSecurityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC_ENCRYPTED))
      && ((params->status == SL_ZIGBEE_GP_STATUS_AUTH_FAILURE)
          || (params->status == SL_ZIGBEE_GP_STATUS_UNPROCESSED))) {
    // fwd frame with Security processing failed bit set as part of GP
    // Commissioning Notification.
    goto send_notification;
  }

  // Step c
  if ((params->bidirectionalInfo & SL_ZIGBEE_GP_BIDIRECTION_INFO_RX_AFTER_TX_MASK)
      && (params->gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_COMMISSIONING)
      && (params->addr.applicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS)) {
    // check endpoint
  }

  if (frameFound) {
    // send at least one Commissioning Reply GPDF
    //XXX compare to where we do this now
  }

  // Step d
  /* TODO
     {
     uint8_t gpProxyTableIndex = sl_zigbee_gp_proxy_table_lookup(addr);
     if (gpProxyTableIndex != 0xFF) {
      sli_zigbee_gp_proxy_table_set_in_range(gpProxyTableIndex);
     } else {
       //basic proxy doesn't have to do anything in this situation
     }

     }
   */

  // Step e: sending GP Commissioning Notification
  send_notification: {
    uint16_t options = params->addr.applicationId;
    if (params->bidirectionalInfo & SL_ZIGBEE_GP_BIDIRECTION_INFO_RX_AFTER_TX_MASK) {
      options |= SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_RX_AFTER_TX;
    }

    if ((params->status == SL_ZIGBEE_GP_STATUS_AUTH_FAILURE)
        || (params->status == SL_ZIGBEE_GP_STATUS_UNPROCESSED)) {
      options |=
        SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_SECURITY_PROCESSING_FAILED;
      options |= (params->gpdfSecurityKeyType ? 0x4 : 0) // see 1.0 spec line 3040
                 << GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT;
    } else {
      options |= params->gpdfSecurityKeyType
                 << GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT;
    }

    options |= params->gpdfSecurityLevel
               << GP_COMMISSIONING_SECURITY_LEVEL_TO_OPTIONS_SHIFT;
    options |= SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_PROXY_INFO_PRESENT;
    if (commissioningState.unicastCommunication) {
      sendUnicastCommissioningNotification(params, options);
    } else {
      sendBroadcastCommissioningNotification(params, options);
    }
  }

  kickout: return true;
}

static bool channelRequestGpdfCallback(sl_zigbee_gp_params_t *params)
{
  if (!commissioningState.inCommissioningMode) {
    return true;
  }

  // based on figure 88 of GP proxy basic spec V1.0 page 191
  // the second condition of the following condition seems neccessary
  if ((commissioningState.channelStatus & GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK)
      && (!(commissioningState.channelStatus & GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK))) {
    //Don't forward if we're on the transmit channel, and the transmit channel
    // is not same as the operational channel
  } else {
    //TODO: check for a reason not to forward (busy, etc)

    uint16_t options = 0; //always uses appID srcID becase srcID is absent was addr.applicationId;

    //rxAfter Tx always set for channel request
    options |=
      SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_RX_AFTER_TX;

    if (params->status == SL_ZIGBEE_GP_STATUS_AUTH_FAILURE) {
      options |=
        SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_SECURITY_PROCESSING_FAILED;
    }

    options |= params->gpdfSecurityLevel
               << GP_COMMISSIONING_SECURITY_LEVEL_TO_OPTIONS_SHIFT;
    options |= params->gpdfSecurityKeyType
               << GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT;
    options |= SL_ZIGBEE_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_PROXY_INFO_PRESENT;

    sl_zigbee_aps_frame_t *apsFrame;

    if (commissioningState.unicastCommunication) {
      if (sl_zigbee_af_fill_command_green_power_cluster_gp_commissioning_notification_smart(options,
                                                                                            0x00000000, //addr.id.sourceId,
                                                                                            NULL, //addr.id.gpdIeeeAddress,
                                                                                            0, //addr.endpoint,
                                                                                            params->sequenceNumber,
                                                                                            params->gpdfSecurityLevel,
                                                                                            params->gpdSecurityFrameCounter,
                                                                                            params->gpdCommandId,
                                                                                            params->gpdCommandPayloadLength,
                                                                                            params->gpdCommandPayload,
                                                                                            sl_zigbee_get_node_id(),
                                                                                            params->gpdLink,
                                                                                            params->mic) == 0) {
        return true;
      }

      apsFrame = sl_zigbee_af_get_command_aps_frame();
      apsFrame->sourceEndpoint = SL_ZIGBEE_GP_ENDPOINT;  //sl_zigbee_af_current_endpoint();
      apsFrame->destinationEndpoint = SL_ZIGBEE_GP_ENDPOINT; //sl_zigbee_af_current_endpoint();
      gppTunnelingDelay(params->bidirectionalInfo, params->gpdLink);
      UNUSED sl_status_t retval = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                                                    commissioningState.commissioningSink);
      sli_zigbee_af_set_add_delay(0);
    } else {
      if (sl_zigbee_af_fill_command_green_power_cluster_gp_commissioning_notification_smart(options,
                                                                                            0x00000000, //addr.id.sourceId,
                                                                                            NULL, //addr.id.gpdIeeeAddress,
                                                                                            0, //addr.endpoint,
                                                                                            params->sequenceNumber,
                                                                                            params->gpdfSecurityLevel,
                                                                                            params->gpdSecurityFrameCounter,
                                                                                            params->gpdCommandId,
                                                                                            params->gpdCommandPayloadLength,
                                                                                            params->gpdCommandPayload,
                                                                                            sl_zigbee_get_node_id(),
                                                                                            params->gpdLink,
                                                                                            params->mic) == 0) {
        return true;
      }

      apsFrame = sl_zigbee_af_get_command_aps_frame();
      apsFrame->sourceEndpoint = SL_ZIGBEE_GP_ENDPOINT;  //sl_zigbee_af_current_endpoint();
      apsFrame->destinationEndpoint = SL_ZIGBEE_GP_ENDPOINT; //sl_zigbee_af_current_endpoint();
      apsFrame->sequence = params->sequenceNumber - SL_ZIGBEE_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET;
      apsFrame->options |= SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
      gppTunnelingDelay(params->bidirectionalInfo, params->gpdLink);
      UNUSED sl_status_t retval = sl_zigbee_af_send_command_broadcast(SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                                                      sli_zigbee_gpd_alias(&(params->addr)),
                                                                      params->sequenceNumber - SL_ZIGBEE_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET);
      sli_zigbee_af_set_add_delay(0);
    }
  }

  return true;
}

static void forwardDerivedGroupNotification(sl_zigbee_gp_params_t *params,
                                            sl_zigbee_gp_proxy_table_entry_t *entry)
{
  sl_zigbee_aps_frame_t *apsFrame = sl_zigbee_af_get_command_aps_frame();
  apsFrame->sourceEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  apsFrame->destinationEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  apsFrame->sequence = params->sequenceNumber;
  apsFrame->options |= SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
  apsFrame->radius = entry->groupcastRadius;
  gppTunnelingDelay(params->bidirectionalInfo, params->gpdLink);

  // get derived alias for address alias and group ID alias
  uint16_t alias = (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_ASSIGNED_ALIAS)
                   ? entry->assignedAlias
                   : sli_zigbee_gpd_alias(&(params->addr));

  sl_zigbee_af_send_command_multicast(sli_zigbee_gpd_alias(&(params->addr)),
                                      alias,
                                      params->sequenceNumber);
  sli_zigbee_af_set_add_delay(0);
}

static void forwardGroupcastNotification(sl_zigbee_gp_params_t *params,
                                         sl_zigbee_gp_sink_list_entry_t *sinkEntry,
                                         uint8_t groupcastRadius)
{
  sl_zigbee_aps_frame_t *apsFrame = sl_zigbee_af_get_command_aps_frame();
  apsFrame->sourceEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  apsFrame->destinationEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  apsFrame->sequence = params->sequenceNumber - SL_ZIGBEE_GP_NOTIFICATION_COMMISSIONED_GROUPCAST_SEQUENCE_NUMBER_OFFSET;
  apsFrame->options |= SL_ZIGBEE_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
  apsFrame->radius = groupcastRadius;
  gppTunnelingDelay(params->bidirectionalInfo, params->gpdLink);

  sl_zigbee_af_send_command_multicast(sinkEntry->target.groupcast.groupID,
                                      (sinkEntry->target.groupcast.alias == 0xFFFF)
                                      ? sli_zigbee_gpd_alias(&(params->addr))
                                      : sinkEntry->target.groupcast.alias,
                                      params->sequenceNumber - SL_ZIGBEE_GP_NOTIFICATION_COMMISSIONED_GROUPCAST_SEQUENCE_NUMBER_OFFSET);
  sli_zigbee_af_set_add_delay(0);
}

static void forwardUnicastNotification(sl_zigbee_gp_params_t *params, sl_zigbee_gp_sink_list_entry_t *sinkEntry)
{
  sl_zigbee_aps_frame_t *apsFrame = sl_zigbee_af_get_command_aps_frame();
  apsFrame->sourceEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  apsFrame->destinationEndpoint = SL_ZIGBEE_GP_ENDPOINT;
  gppTunnelingDelay(params->bidirectionalInfo, params->gpdLink);
  // The sinkNodeId is optimised to not be in the proxy table token structure because it can be looked up.
  // In case the proxy table is updated with a valid address earlier, it needs to use that.
  // So, check if the proxy table node id is not valid UCAST id before looking up the map and then forward the notification.
  if (!isNodeIdValid(sinkEntry->target.unicast.sinkNodeId)) {
    sl_802154_short_addr_t nodeId;
    (void)sl_zigbee_lookup_node_id_by_eui64(sinkEntry->target.unicast.sinkEUI, &nodeId);
    if (isNodeIdValid(nodeId)) {
      sinkEntry->target.unicast.sinkNodeId = nodeId;
    }
  }

  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, sinkEntry->target.unicast.sinkNodeId);
  sli_zigbee_af_set_add_delay(0);
}

static void forwardToSinkTable(sl_zigbee_gp_params_t *params, sl_zigbee_gp_proxy_table_entry_t *entry)
{
  for (uint8_t i = 0; i < GP_SINK_LIST_ENTRIES; i++) {
    if (entry->sinkList[i].type == SL_ZIGBEE_GP_SINK_TYPE_FULL_UNICAST
        || entry->sinkList[i].type == SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST) {
      forwardUnicastNotification(params, &entry->sinkList[i]);
    } else if (entry->sinkList[i].type == SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST) {
      forwardGroupcastNotification(params, &entry->sinkList[i], entry->groupcastRadius);
    }
  }

  if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_DERIVED_GROUP_GPS) {
    forwardDerivedGroupNotification(params, entry);
  }
}

static uint16_t prepareNotificationOptions(sl_zigbee_gp_params_t *params, sl_zigbee_gp_proxy_table_entry_t *entry)
{
  uint16_t options = params->addr.applicationId;

  //Traverse the sink list to set the options flags
  for (uint8_t i = 0; i < GP_SINK_LIST_ENTRIES; i++) {
    if (entry->sinkList[i].type == SL_ZIGBEE_GP_SINK_TYPE_FULL_UNICAST
        || entry->sinkList[i].type == SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST) {
      options |= SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_ALSO_UNICAST;
    } else if (entry->sinkList[i].type == SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST) {
      options |= SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_ALSO_COMMISSIONED_GROUP;
    }
  }

  if (entry->options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_DERIVED_GROUP_GPS) {
    options |= SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_ALSO_DERIVED_GROUP;
  }

  options |= params->gpdfSecurityLevel
             << SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_SECURITY_LEVEL_OFFSET;

  options |= params->gpdfSecurityKeyType
             << SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_SECURITY_KEY_TYPE_OFFSET;

#ifndef ENABLE_BIDIRECTIONAL_OPERATION
  //Basic proxy always sends queue full for data gpdfs (not commissioning)
  options |= SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_GP_TX_QUEUE_FULL;
#else
  // Advanced proxy uses queue availability to fill this information
  options |= (bidirectionalInfo & SL_ZIGBEE_GP_BIDIRECTION_INFO_TX_QUEUE_AVAILABLE_MASK) \
             ? 0 : SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_GP_TX_QUEUE_FULL;
#endif
  // Basic Proxy implementing the latest spec always sets this
  options |= SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_PROXY_INFO_PRESENT;

  // comm/decomm frames ignore rx after tx bit in operational mode
  if (((params->gpdCommandId != SL_ZIGBEE_ZCL_GP_GPDF_COMMISSIONING
        && params->gpdCommandId != SL_ZIGBEE_ZCL_GP_GPDF_DECOMMISSIONING)
       || commissioningState.inCommissioningMode)
      && (params->bidirectionalInfo & SL_ZIGBEE_GP_BIDIRECTION_INFO_RX_AFTER_TX_MASK)) {
    options |= SL_ZIGBEE_AF_GP_NOTIFICATION_OPTION_RX_AFTER_TX;
  }

  return options;
}

static bool gpdfForwardCallback(sl_zigbee_gp_params_t *params)
{
  uint16_t options = 0;

  if (params->proxyTableIndex == 0xFF) {
    return true;
  }
  sl_zigbee_gp_proxy_table_entry_t entry;
  if (params->proxyTableIndex != 0xFF) {
#ifndef EZSP_HOST
    sli_zigbee_gp_proxy_table_set_first_to_forward(params->proxyTableIndex);
    sli_zigbee_gp_proxy_table_set_in_range(params->proxyTableIndex);
#endif
    if (sl_zigbee_gp_proxy_table_get_entry(params->proxyTableIndex, &entry) != SL_STATUS_OK) {
      return true;
    }
  }

  options = prepareNotificationOptions(params, &entry);

  if ( params->gpdfSecurityLevel == SL_ZIGBEE_GP_SECURITY_KEY_NONE) {
    // If no security use the MAC sequence number instead
    params->gpdSecurityFrameCounter = (uint32_t)(params->sequenceNumber & 0x000000ff);
  }

  if (sl_zigbee_af_fill_command_green_power_cluster_gp_notification_smart(options,
                                                                          params->addr.id.sourceId,
                                                                          params->addr.id.gpdIeeeAddress,
                                                                          params->addr.endpoint,
                                                                          params->gpdSecurityFrameCounter,
                                                                          params->gpdCommandId,
                                                                          params->gpdCommandPayloadLength,
                                                                          params->gpdCommandPayload,
                                                                          sl_zigbee_get_node_id(),
                                                                          params->gpdLink) == 0) {
    return true;
  }

  // The Proxy table gets populated from the Gp Pairing message that does not have capability to accept the
  // sink group list. But in a combo application, that has both sink and proxy, the sink list can be supplied
  // using the Gp Pairing Configuration command from a commissioning tool and the sink table holds those
  // in its entry. And the expected outcome from the combo basic is that it should be able to forward
  // the messages to the sink table based groupcast. The following callback is used to get an updated sink
  // list entry from user. If the green power server is present it implements this callback.
  sl_zigbee_af_green_power_client_gpdf_sink_table_based_forward_cb(&(params->addr),
                                                                   entry.sinkList,
                                                                   GP_SINK_LIST_ENTRIES);

  forwardToSinkTable(params, &entry);
  return true;
}

/*
 * Check if a sl_zigbee_gp_address_t entry is being used by checking each elemental
 * sequence number's corresponding expiration time field.
 */
static bool is_gp_addr_used(sl_zigbee_af_green_power_duplicate_filter_t * filter,
                            uint8_t index)
{
  uint32_t * expirationTimes;
  uint8_t i;
  uint32_t curTime = halCommonGetInt32uMillisecondTick();

  if (index >= SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES) {
    return true;
  }
  expirationTimes = (uint32_t *) &(filter->expirationTimes[index][0]);

  for (i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR;
       i++) {
    if (expirationTimes[i] > curTime) {
      return true;
    }
  }

  return false;
}

/*
 * return index to the entry with the smallest expiration date.
 */
static uint8_t find_earliest_expiration_time_in_addr(sl_zigbee_af_green_power_duplicate_filter_t * filter,
                                                     uint8_t addrIndex)
{
  // addrIndex must be less than SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR
  // so we dont need to check it here. It is done in preceding function calls.
  uint32_t * expirationTimes = filter->expirationTimes[addrIndex];
  uint32_t earliestTimeIndex = 0;
  uint32_t earliestTime = expirationTimes[0];
  uint8_t index;

  for (index = 0;
       index < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR;
       index++) {
    if (expirationTimes[index] < earliestTime) {
      earliestTime = expirationTimes[index];
      earliestTimeIndex = index;
    }
  }

  return earliestTimeIndex;
}

static bool add_random_mac_seq_num(sl_zigbee_af_green_power_duplicate_filter_t * filter,
                                   uint8_t addrIndex,
                                   uint8_t randomSeqNum)
{
  uint8_t * seqNumList = NULL;
  uint32_t * expirationTimeList = NULL;
  uint8_t i;
  uint32_t curTime = halCommonGetInt32uMillisecondTick();
  uint8_t entryIndex;

  // addrIndex must be less than the SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR
  // so we dont need to check it here. It is done in preceding function calls.
  seqNumList = (filter->randomSeqNums[addrIndex]);
  expirationTimeList = (filter->expirationTimes[addrIndex]);

  // check if an existing sequence number with valid expiration date exists
  for (i = 0;
       i < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR;
       i++) {
    if ((randomSeqNum == seqNumList[i]) && (expirationTimeList[i] > curTime)) {
      return false;
    }
  }

  entryIndex = find_earliest_expiration_time_in_addr(filter,
                                                     addrIndex);

  seqNumList[entryIndex] = randomSeqNum;
  expirationTimeList[entryIndex] = curTime + (2 * MILLISECOND_TICKS_PER_MINUTE);

  return true;
}

/* Helper function for handling duplicateFilter */
static uint8_t find_gp_addr_index(sl_zigbee_gp_address_t * addr,
                                  sl_zigbee_gp_address_t * addrList,
                                  uint8_t sizeOfList)
{
  uint8_t index = 0;
  for (index = 0; index < sizeOfList; index++) {
    if (sli_zigbee_gp_address_match(addr, &addrList[index])) {
      return index;
    }
  }

  return 0xFF;
}

static uint8_t add_gp_addr(sl_zigbee_af_green_power_duplicate_filter_t * filter,
                           sl_zigbee_gp_address_t * addr)
{
  uint8_t addrIndex;

  for (addrIndex = 0;
       addrIndex < SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES;
       addrIndex++) {
    if (!is_gp_addr_used(&duplicateFilter, addrIndex)) {
      memcpy(&filter->addrs[addrIndex], addr, sizeof(sl_zigbee_gp_address_t));
      return addrIndex;
    }
  }

  return 0xFF;
}

/*
 * Check if the incoming message is a duplicate message when the GPD's
 * Mac Sequence Number Capability indicate that random Mac Sequence Number is
 * used.
 *
 *
 * @return
 * If a duplicate mac sequence number has been found, we'll return true in
 * order to reject the message. Otherwise, we'll save the new random mac seq
 * number and return false to accept the message.
 *
 */
bool sli_zigbee_af_green_power_find_duplicate_mac_seq_num(sl_zigbee_gp_address_t * addr,
                                                          uint8_t randomSeqNum)
{
  bool added = false;
  uint8_t addrIndex = find_gp_addr_index(addr,
                                         duplicateFilter.addrs,
                                         SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES);
  if (addrIndex == 0xFF) {
    addrIndex = add_gp_addr(&duplicateFilter, addr);
  }

  if (addrIndex == 0xFF) {
    return false;
  }

  added = add_random_mac_seq_num(&duplicateFilter,
                                 addrIndex,
                                 randomSeqNum);
  return !added;
}

/*
 * This function implements the "Duplicate checking" and "Freshness check"
 * section of the GP implementation details.
 *
 * @return true if given sequenceNumber is valid and message should be
 *         accepted, false otherwise.
 */
bool sli_zigbee_af_gp_message_checking(sl_zigbee_gp_address_t *gpAddr, uint8_t sequenceNumber)
{
  (void) gpAddr;
  (void) sequenceNumber;

  return true;
}

static void processMessageInCommissioningMode(sl_zigbee_gp_params_t *params)
{
  if (params->autoCommissioning) {
    autoCommissioningCallback(params);
  }

  // If status is unprocessed and security is level 3, then gpdCommandId with its payload are encrypted
  // and can not be interpreated by proxy, hence forward it as a GP Commissioning Notification with security
  // processing failed bit set, so that sink can process the commissioning notification.
  if ((params->status == SL_ZIGBEE_GP_STATUS_UNPROCESSED || params->status == SL_ZIGBEE_GP_STATUS_AUTH_FAILURE)
      && params->gpdfSecurityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_FC_MIC_ENCRYPTED) {
    params->status = SL_ZIGBEE_GP_STATUS_AUTH_FAILURE;
    commissioningGpdfCallback(params);
    return;
  }

  // Proceed with command processing based on gpdCommandId
  switch (params->gpdCommandId) {
    case SL_ZIGBEE_ZCL_GP_GPDF_COMMISSIONING:
    case SL_ZIGBEE_ZCL_GP_GPDF_DECOMMISSIONING:
    case SL_ZIGBEE_ZCL_GP_GPDF_SUCCESS:
      commissioningGpdfCallback(params);
      break;

    case SL_ZIGBEE_ZCL_GP_GPDF_CHANNEL_REQUEST:
      channelRequestGpdfCallback(params);
      break;

    default:
      if (params->status == SL_ZIGBEE_GP_STATUS_NO_SECURITY
          && ((params->gpdCommandId >= MIN_RESERVED_GPD_COMMAND_ID
               && params->gpdCommandId <= MAX_RESERVED_GPD_COMMAND_ID)
              || (params->gpdCommandId >= SL_ZIGBEE_ZCL_GP_GPDF_MFR_DEF_GPD_CMD0
                  && params->gpdCommandId <= SL_ZIGBEE_ZCL_GP_GPDF_MFR_DEF_GPD_CMD_F))) {
        commissioningGpdfCallback(params);
      } else if (params->status == SL_ZIGBEE_GP_STATUS_UNPROCESSED) {
        params->status = SL_ZIGBEE_GP_STATUS_AUTH_FAILURE;
        commissioningGpdfCallback(params);
      } else {
        gpdfForwardCallback(params);
      }
      break;
  }
}

static void processMessageInNonCommissioningMode(sl_zigbee_gp_params_t *params)
{
  // Outside of commissioning mode, everything gets forwarded, including commissioning related messages
  // But not success messages (See A3.9.1)
  if (params->gpdCommandId == SL_ZIGBEE_ZCL_GP_GPDF_SUCCESS && !commissioningState.inCommissioningMode) {
    // Drop a success message
    return;
  }

  gpdfForwardCallback(params);
}

//GP-DATA.indication
void sli_zigbee_af_green_power_client_gpep_incoming_message_callback(sl_zigbee_gp_params_t *params)
{
  if ((commissioningState.channelStatus & GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK)
      && params->gpdCommandId != SL_ZIGBEE_ZCL_GP_GPDF_CHANNEL_REQUEST) {
    //drop everything but a channel request when we're off on a different channel
    return;
  }

  if ( params->gpdfSecurityLevel == SL_ZIGBEE_GP_SECURITY_LEVEL_NONE
       && !commissioningState.inCommissioningMode) {
    // earlier a message satisfying this condition would have been
    // dropped in gp-data, but
    // this check is needed here after we added a condition
    // in gp-data to always pass up the unsecure commissioning messages
    // up to this function (EMZIGBEE-1417)
    // when we have a security enabled proxy table entry
    // We only need to forward the unsecure commisioning messages
    // in the commisisong mode, and not in the operational mode
    // (EMZIGBEE-1578)
    bool secureEntry = false;
    uint8_t index = sl_zigbee_gp_proxy_table_lookup(&(params->addr));
    if (index != 0xFF) {
      sl_zigbee_gp_proxy_table_entry_t entry;
      sl_status_t getStatus = sl_zigbee_gp_proxy_table_get_entry(index, &entry);
      if (getStatus != SL_STATUS_OK) {
        return;
      }
      secureEntry = ((entry.options & SL_ZIGBEE_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_SECURITY_USE)
                     || (entry.securityOptions));
    }

    if (secureEntry) {
      return;
    }
  }

  if ((params->status == SL_ZIGBEE_GP_STATUS_OK
       || params->status == SL_ZIGBEE_GP_STATUS_NO_SECURITY
       || (params->status == SL_ZIGBEE_GP_STATUS_UNPROCESSED && commissioningState.inCommissioningMode)
       || (params->status == SL_ZIGBEE_GP_STATUS_AUTH_FAILURE  && commissioningState.inCommissioningMode))
      && sli_zigbee_af_gp_message_checking(&(params->addr), params->sequenceNumber)) {
    if (commissioningState.inCommissioningMode) {
      processMessageInCommissioningMode(params);
    } else {
      processMessageInNonCommissioningMode(params);
    }
  }
}

void sl_zigbee_af_green_power_client_clear_proxy_table(void)
{
  sl_zigbee_gp_clear_proxy_table();
}

void sli_zigbee_af_green_power_client_d_gp_sent_callback(sl_status_t status, uint8_t gpepHandle)
{
  (void)gpepHandle;
  sl_zigbee_af_green_power_cluster_println("sli_zigbee_stack_d_gp_sent_handler Status = %02X", status);
  if (status == SL_STATUS_OK
      && (commissioningState.channelStatus & GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK)) {
    // Here, the sent event is for channel configuration delivery.
    // Clear the 5 sec timer that started while submitted a packet to TxQueue - call handler
    channelEventHandler(&channelEvent);
  }
}

void sli_zigbee_af_green_power_client_stack_status_callback(sl_status_t status)
{
  if (status == SL_STATUS_NETWORK_DOWN
      && sl_zigbee_is_performing_rejoin() == FALSE) {
    // Clear proxy table when leaving the network.
    sl_zigbee_gp_clear_proxy_table();
    // Ensure to close any commissioning session in progress.
    exitCommissioningEventHandler(&exitCommissioningEvent);
  }
}

uint32_t sl_zigbee_af_green_power_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                               sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GP_PAIRING_COMMAND_ID:
      {
        status = sl_zigbee_af_green_power_cluster_gp_pairing_cb(cmd);
        break;
      }
      case ZCL_GP_PROXY_COMMISSIONING_MODE_COMMAND_ID:
      {
        status = sl_zigbee_af_green_power_cluster_gp_proxy_commissioning_mode_cb(cmd);
        break;
      }
      case ZCL_GP_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_green_power_cluster_gp_response_cb(cmd);
        break;
      }
      case ZCL_GP_PROXY_TABLE_REQUEST_COMMAND_ID:
      {
        status = sl_zigbee_af_green_power_cluster_gp_proxy_table_request_cb(cmd);
        break;
      }
    }
  }

  return status;
}

WEAK(void sl_zigbee_af_green_power_client_gpdf_sink_table_based_forward_cb(sl_zigbee_gp_address_t *addr,
                                                                           sl_zigbee_gp_sink_list_entry_t *sinkList,
                                                                           uint8_t maxNumberEntries))
{
  (void)addr;
  (void)sinkList;
  (void)maxNumberEntries;
}
