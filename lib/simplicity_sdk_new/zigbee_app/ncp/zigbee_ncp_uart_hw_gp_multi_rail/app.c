/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
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

#include PLATFORM_HEADER

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "hal.h"
#include "stack/include/sl_zigbee.h"
#include "stack/include/gp-types.h"
// This special GP NCP app is allowed to make internal calls
#include "stack/internal/inc/stack-info-internal-def.h"
#include "app/util/ezsp/ezsp-enum.h"
#include "app/xncp/xncp-sample-custom-ezsp-protocol.h"
#include "sl_zigbee_system_common.h"
#include "stack/config/sl_zigbee_token_defines.h"

#ifdef SL_CATALOG_ZIGBEE_REAL_IPC_PRESENT
#define MAX_BUFFER_SIZE 127
#else
#define MAX_BUFFER_SIZE 128
#endif // SL_CATALOG_ZIGBEE_REAL_IPC_PRESENT

#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_PRESENT
#include "green-power-server.h"
#include "green-power-common.h"
#include "app/framework/include/af.h"
// Finds and returns the Gp Controlable application endpoint in the APS group
static uint16_t findAppEndpointGroupId(uint8_t endpoint)
{
  for (uint8_t i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    sl_zigbee_binding_table_entry_t binding;
    if (sl_zigbee_get_binding(i, &binding) == SL_STATUS_OK
        && binding.type == SL_ZIGBEE_MULTICAST_BINDING
        && binding.local == endpoint) {
      uint16_t groupId = (binding.identifier[1] << 8) | binding.identifier[0];
      return groupId;
    }
  }
  return 0;
}

// This implementation is targeting the test spec (4.3.4.1)
// where it uses the same group for Gp endpoint and application endpoints.
// to read the groupId of the operational endpoint and use that groupId
// to add the GP endpoint for groupcast notifications.
void sl_zigbee_af_green_power_server_pre_sink_pairing_cb(sli_zigbee_gp_comm_data_saved_t *commissioningGpd)
{
  sl_zigbee_app_debug_println("PreSinkPairingCallback is called from %s",
                              (commissioningGpd->preSinkCbSource == GP_PRE_SINK_PAIRING_CALLBACK_PAIRING_CONFIGURATION)
                              ? "pairing configuration" : "native commissioning");

  if (commissioningGpd->preSinkCbSource == GP_PRE_SINK_PAIRING_CALLBACK_PAIRING_CONFIGURATION) {
    return;
  }
  // Set up the grouplist incase it did not have one, supply default values.
  uint8_t *count = commissioningGpd->groupList;
  uint8_t *grouplist = commissioningGpd->groupList + 1;
  *count = 0;
  uint8_t totalEndpointCount = sl_zigbee_get_endpoint_count();
  for (uint8_t i = 0; i < totalEndpointCount && grouplist < (commissioningGpd->groupList + GP_SIZE_OF_SINK_LIST_ENTRIES_OCTET_STRING); i++) {
    uint8_t endpoint = sl_zigbee_get_endpoint(i);
    if (endpoint != 0xff // A valid application endpoint value.
        && isCommissioningAppEndpoint(endpoint)) {
      uint16_t groupId = findAppEndpointGroupId(endpoint);
      if (0 != groupId) {
        (*count)++;
        memcpy(grouplist, &groupId, sizeof(groupId));
        grouplist += sizeof(groupId);
        uint16_t alias = sli_zigbee_gpd_alias(&(commissioningGpd->addr));
        memcpy(grouplist, &alias, sizeof(alias));
        grouplist += sizeof(alias);
      }
    }
  }
}

void sl_zigbee_af_green_power_server_pairing_status_cb(sl_zigbee_sink_pairing_status_t status,
                                                       sl_zigbee_commissioning_gpd_t *commissioningGpd)
{
  sl_zigbee_app_debug_println("server_pairing_status_cb");
  sl_zigbee_app_debug_println("Status: %02X", status);
  sl_zigbee_app_debug_println("App ID: %02X, App EP: %02X", commissioningGpd->addr.applicationId, commissioningGpd->addr.endpoint);
  if (commissioningGpd->addr.applicationId == SL_ZIGBEE_GP_APPLICATION_IEEE_ADDRESS) {
    sl_zigbee_app_debug_println("IEEE address: %.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X",
                                commissioningGpd->addr.id.gpdIeeeAddress[0],
                                commissioningGpd->addr.id.gpdIeeeAddress[1],
                                commissioningGpd->addr.id.gpdIeeeAddress[2],
                                commissioningGpd->addr.id.gpdIeeeAddress[3],
                                commissioningGpd->addr.id.gpdIeeeAddress[4],
                                commissioningGpd->addr.id.gpdIeeeAddress[5],
                                commissioningGpd->addr.id.gpdIeeeAddress[6],
                                commissioningGpd->addr.id.gpdIeeeAddress[7]);
  } else {
    sl_zigbee_app_debug_println("Source ID: %08X", commissioningGpd->addr.id.sourceId);
  }
  sl_zigbee_app_debug_println("Option: %02X, Extended Option: %02X", commissioningGpd->gpdfOptions, commissioningGpd->gpdfExtendedOptions);
  sl_zigbee_app_debug_println("Communication mode: %02X", commissioningGpd->communicationMode);
  sl_zigbee_app_debug_println("Security level: %02X", commissioningGpd->securityLevel);

  // Key
  sl_zigbee_app_debug_print("Key: ");
  for (uint8_t i = 0; i < SL_ZIGBEE_ENCRYPTION_KEY_SIZE; i++) {
    sl_zigbee_app_debug_print("%.2X ", commissioningGpd->key.contents[i]);
  }

  // Alias
  sl_zigbee_app_debug_println("\nUse given alias: %d", commissioningGpd->useGivenAssignedAlias);
  if (commissioningGpd->useGivenAssignedAlias) {
    sl_zigbee_app_debug_print("%04X ", commissioningGpd->givenAlias);
  }

  // Group
  sl_zigbee_app_debug_println("Group count: %d", commissioningGpd->groupList[0]);
  for (uint8_t i = 0; i < commissioningGpd->groupList[0]; i++) {
    uint8_t pos = i * sizeof(sl_zigbee_gp_sink_group_t) + 1;
    sl_zigbee_gp_sink_group_t sinkGroup = { 0 };
    memcpy(&sinkGroup, &commissioningGpd->groupList[pos], sizeof(sl_zigbee_gp_sink_group_t));
    sl_zigbee_app_debug_println("Idx: %d, Group ID: %04X, Alias: %04X", i, sinkGroup.groupID, sinkGroup.alias);
  }
}

void sl_zigbee_af_green_power_cluster_commissioning_message_status_notification_cb(sl_zigbee_af_green_power_server_commissioning_state_t *commissioningState,
                                                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                                                   sl_zigbee_outgoing_message_type_t messageType,
                                                                                   uint16_t destination,
                                                                                   sl_status_t status)
{
  UNUSED_VAR(apsFrame);
  UNUSED_VAR(messageType);
  UNUSED_VAR(destination);
  UNUSED_VAR(status);
  sl_zigbee_app_debug_println("CommissioningMessageStatusNotificationCallback. Endpoint: %d, Proxies Involved: %d",
                              commissioningState->endpoint, commissioningState->proxiesInvolved);
}

#endif //SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_PRESENT

#if defined(SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT)
#include "multirail-demo.h"

static void gp_transmit_complete_event_handler(sli_zigbee_event_t *event);
extern sli_zigbee_event_queue_t sli_zigbee_stack_event_queue;
sli_zigbee_event_t gp_transmit_complete_event = { {
                                                    &sli_zigbee_stack_event_queue,
                                                    gp_transmit_complete_event_handler,
                                                    sli_zigbee_isr_event_marker,
                                                    ""
                                                  },
                                                  NULL, 0, 0, NULL };

// The time between receiving a GP packet and sending a scheduled response
#ifndef GP_RX_OFFSET_USEC
#define GP_RX_OFFSET_USEC 20500
#endif

//--------------------------------
// Extern and forward declarations

extern uint8_t sli_mac_lower_mac_get_radio_channel(uint8_t mac_index);
extern sli_buffer_manager_buffer_t sli_zigbee_gpdf_make_header(bool useCca,
                                                               sl_zigbee_gp_address_t *src,
                                                               sl_zigbee_gp_address_t *dst);
extern void sli_zigbee_stack_d_gp_sent_handler(sl_status_t status,
                                               uint8_t gpepHandle);
extern sl_status_t sl_zigbee_af_xncp_send_custom_ezsp_message(uint8_t length, uint8_t *payload);

static void appGpScheduleOutgoingGpdf(sl_zigbee_zigbee_packet_type_t packetType,
                                      int8u* packetData,
                                      int8u size_p);

//---------------
// Event handlers

static void gp_transmit_complete_event_handler(sli_zigbee_event_t *event)
{
  UNUSED_VAR(event);
  sli_zigbee_stack_d_gp_sent_handler(SL_STATUS_OK, 0);
}

//----------------------
// Implemented Callbacks

/** @brief Init
 * Application init function
 */
void sl_zigbee_af_main_init_cb(void)
{
}

/** @brief Incoming Custom EZSP Message Callback
 *
 * This function is called when the NCP receives a custom EZSP message from the
 * HOST.  The message length and payload is passed to the callback in the first
 * two arguments.  The implementation can then fill in the replyPayload and set
 * the replayPayloadLength to the number of bytes in the replyPayload.
 * See documentation for the function sl_zigbee_ezsp_custom_frame on sending these messages
 * from the HOST.
 *
 * @param messageLength The length of the messagePayload.
 * @param messagePayload The custom message that was sent from the HOST.
 * Ver.: always
 * @param replyPayloadLength The length of the replyPayload.  This needs to be
 * set by the implementation in order for a properly formed respose to be sent
 * back to the HOST. Ver.: always
 * @param replyPayload The custom message to send back to the HOST in respose
 * to the custom message. Ver.: always
 *
 * @return An ::sl_status_t indicating the result of the custom message
 * handling.  This returned status is always the first byte of the EZSP
 * response.
 */
sl_status_t sl_zigbee_af_xncp_incoming_custom_frame_cb(uint8_t messageLength,
                                                       uint8_t *messagePayload,
                                                       uint8_t *replyPayloadLength,
                                                       uint8_t *replyPayload)
{
  // In the sample custom EZSP protocol, the command ID is always the first byte
  // of a custom EZSP command.
  uint8_t index = 0;
  uint8_t commandId = messagePayload[index++];

  switch (commandId) {
    case SL_ZIGBEE_CUSTOM_EZSP_COMMAND_INIT_APP_GP_TX_QUEUE: {
      sl_zigbee_gp_clear_tx_queue();
      sl_rail_handle_t h =  sl_zigbee_af_multirail_demo_init(NULL,
                                                             NULL,
                                                             true,
                                                             sl_rail_get_tx_power_dbm(sl_zigbee_get_rail_handle()),
                                                             NULL,
                                                             0,
                                                             0xFFFF,
                                                             NULL);
      return ((h == NULL) ? SL_STATUS_FAIL : SL_STATUS_OK);
    }
    break;
    case SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_APP_GP_TX_QUEUE: {
      sl_zigbee_gp_address_t addr = { 0 };
      addr.applicationId = messagePayload[index++];
      if (addr.applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
        addr.id.sourceId = (uint32_t)messagePayload[index++];
        addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 8);
        addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 16);
        addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 24);
      } else {
        // Initial support only for GPD Src ID
        return SL_STATUS_INVALID_STATE;
      }
      uint8_t gpdCommandId = messagePayload[index++];

      if (messageLength >= index) {
        if (sl_zigbee_d_gp_send(true,//bool action,
                                false,//bool useCca,
                                &addr,
                                gpdCommandId,
                                (messageLength - index),
                                &messagePayload[index],
                                0,
                                0) == SL_STATUS_OK) {
          return SL_STATUS_OK;
        }
      }
      return SL_STATUS_INVALID_STATE;
    }
    break;
    case SL_ZIGBEE_CUSTOM_EZSP_COMMAND_REMOVE_APP_GP_TX_QUEUE: {
      sl_zigbee_gp_address_t addr = { 0 };
      addr.applicationId = messagePayload[index++];
      if (addr.applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
        addr.id.sourceId = (uint32_t)messagePayload[index++];
        addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 8);
        addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 16);
        addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 24);
      } else {
        // Initial support only for GPD Src ID
        return SL_STATUS_INVALID_STATE;
      }

      if (messageLength >= index) {
        if (sl_zigbee_d_gp_send(false,//bool action,
                                false,//bool useCca,
                                &addr,
                                0,
                                0,
                                NULL,
                                0,
                                0) == SL_STATUS_OK) {
          return SL_STATUS_OK;
        }
      }
      return SL_STATUS_INVALID_STATE;
    }
    break;
    case SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_APP_GP_TX_QUEUE: {
      sl_zigbee_gp_tx_queue_entry_t txQueue;
      txQueue.addr.applicationId = messagePayload[index++];
      if (txQueue.addr.applicationId == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID) {
        txQueue.addr.id.sourceId = (uint32_t)messagePayload[index++];
        txQueue.addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 8);
        txQueue.addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 16);
        txQueue.addr.id.sourceId += (((uint32_t)messagePayload[index++]) << 24);
      } else {
        // Initial support only for GPD Src ID
        return SL_STATUS_INVALID_STATE;
      }
      uint8_t buffer[MAX_BUFFER_SIZE];
      uint16_t outPktLength = 0;

      if (sl_zigbee_gp_get_tx_queue_entry_from_queue(&txQueue,
                                                     buffer,
                                                     &outPktLength,
                                                     sizeof(buffer)) != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
        replyPayload[0] = txQueue.addr.applicationId;
        replyPayload[1] = txQueue.addr.id.sourceId >> 0;
        replyPayload[2] = txQueue.addr.id.sourceId >> 8;
        replyPayload[3] = txQueue.addr.id.sourceId >> 16;
        replyPayload[4] = txQueue.addr.id.sourceId >> 24;
        memcpy(&replyPayload[5], buffer, outPktLength);
        *replyPayloadLength = (outPktLength + 5);
        return SL_STATUS_OK;
      }
      return SL_STATUS_INVALID_STATE;
    }
    break;
    case SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SET_APP_GP_TX_QUEUE_MAX_SIZE: {
      uint16_t maxTxGpQueueSize = (uint32_t)messagePayload[index++];
      maxTxGpQueueSize += (((uint32_t)messagePayload[index]) << 8);
      sl_zigbee_gp_set_max_tx_queue_entry(maxTxGpQueueSize);
      return SL_STATUS_OK;
    }
    break;
    case SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_APP_GP_TX_QUEUE_MAX_SIZE: {
      uint16_t maxTxGpQueueSize = sl_zigbee_get_gp_max_tx_q_list_count();
      replyPayload[0] = maxTxGpQueueSize >> 0;
      replyPayload[1] = maxTxGpQueueSize >> 8;
      *replyPayloadLength = sizeof(maxTxGpQueueSize);
      return SL_STATUS_OK;
    }
    break;
    case SL_ZIGBEE_CUSTOM_EZSP_COMMAND_GET_APP_GP_TX_QUEUE_COUNT: {
      uint16_t txGpQueueCount = sl_zigbee_get_gp_tx_q_list_count();
      replyPayload[0] = txGpQueueCount >> 0;
      replyPayload[1] = txGpQueueCount >> 8;
      *replyPayloadLength = sizeof(txGpQueueCount);
      return SL_STATUS_OK;
    }
    break;
    case SL_ZIGBEE_CUSTOM_EZSP_COMMAND_SEND_APP_GP_RAW: {
      uint8_t channel = messagePayload[index++];
      uint8_t delayInMsec = messagePayload[index++];
      uint8_t* buff = &messagePayload[index];
      uint8_t size = messageLength - index;
      sl_rail_scheduled_tx_config_t scheduledTxConfig = {
        .mode = SL_RAIL_TIME_DELAY,
        .when = (delayInMsec * 1000)
      };
      sl_rail_scheduler_info_t schedulerInfo = {
        .priority = 50,
        .slip_time = 2000,
        .transaction_time = 5000
      };
      sl_rail_status_t s = sl_zigbee_af_multirail_demo_send(buff,
                                                            size,
                                                            ((channel == 0) ? sli_mac_lower_mac_get_radio_channel(0) : channel),
                                                            ((delayInMsec == 0) ? NULL : &scheduledTxConfig),
                                                            &schedulerInfo);
      return ((s == SL_RAIL_STATUS_NO_ERROR) ? SL_STATUS_OK : SL_STATUS_INVALID_STATE);
    }
    break;
    default:
      return SL_STATUS_INVALID_STATE;
      break;
  }
}

sl_zigbee_packet_action_t sli_zigbee_dispatch_packet_handoff_incoming_callback(sl_zigbee_zigbee_packet_type_t packetType,
                                                                               sli_buffer_manager_buffer_t packetBuffer,
                                                                               uint8_t index,
                                                                               void *data,
                                                                               uint8_t data_len)
{
  uint8_t size_p = sl_legacy_buffer_manager_message_buffer_length(packetBuffer) - index;
  uint8_t packetData[MAX_BUFFER_SIZE];
  UNUSED_VAR(data);
  UNUSED_VAR(data_len);
  // Flat packet : [<-----MAC Frame----->|<--8 bytes Appended Info-->]
  sl_legacy_buffer_manager_copy_from_linked_buffers(packetBuffer,
                                                    index,
                                                    packetData,
                                                    size_p);
  appGpScheduleOutgoingGpdf(packetType,
                            packetData,
                            size_p);
  return SL_ZIGBEE_ACCEPT_PACKET;
}

extern void sl_zigbee_wakeup_common_task(void);
/** @brief A callback called whenever a secondary instance RAIL event occurs.
 *
 * @param[in] handle A handle for a RAIL instance.
 * @param[in] events A bit mask of RAIL events (full list in rail_types.h)
 */
void sl_zigbee_af_multirail_demo_rail_event_cb(sl_rail_handle_t handle,
                                               sl_rail_events_t events)
{
  if (events & SL_RAIL_EVENT_TX_PACKET_SENT) {
    sli_zigbee_event_set_active(&gp_transmit_complete_event);
#ifdef SL_CATALOG_KERNEL_PRESENT
    sl_zigbee_wakeup_common_task();
#endif // SL_CATALOG_KERNEL_PRESENT
  }
  (void)handle; // unreferenced parameter
}

/** @brief
 *
 * Application framework equivalent of ::sl_zigbee_radio_needs_calibrating_handler
 */
void sl_zigbee_af_radio_needs_calibrating_cb(void)
{
  sl_mac_calibrate_current_channel();
}

//-----------------
// Static functions

// Gets an entry from the Gp Stub queue if available and RAIL 2 is ready
static sl_zigbee_gp_tx_queue_entry_t* get_gp_stub_tx_queue(sl_zigbee_gp_address_t* addr,
                                                           uint8_t *asduLength,
                                                           uint8_t *asdu)
{
  // Steal the GP Queue to send it through the additional RAIL handle
  // Check if RAIL 2 handle available and there is anything in GP Stub queue
  sl_zigbee_gp_tx_queue_entry_t sli_zigbee_gp_tx_queue;
  memcpy(&sli_zigbee_gp_tx_queue.addr, addr, sizeof(sl_zigbee_gp_address_t));
  uint8_t data[MAX_BUFFER_SIZE];
  uint16_t dataLength;
  if (sl_zigbee_af_multirail_demo_get_handle()
      && sl_zigbee_gp_get_tx_queue_entry_from_queue(&sli_zigbee_gp_tx_queue,
                                                    data,
                                                    &dataLength,
                                                    sizeof(data)) != SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
    // Allocate a buffer and prepare a outgoing MAC header using gpd address in the sli_zigbee_gp_tx_queue
    sli_buffer_manager_buffer_t header = sli_zigbee_gpdf_make_header(true, NULL, &(sli_zigbee_gp_tx_queue.addr));
    if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
      return NULL;
    }

    // Add the command Id from the queue to the buffer
    uint8_t len = sl_legacy_buffer_manager_message_buffer_length(header) + 1;
    if ((sl_legacy_buffer_manager_append_to_linked_buffers(header,
                                                           &(sli_zigbee_gp_tx_queue.gpdCommandId),
                                                           1) == SL_STATUS_OK)
        // Copy Command Payload from the queue to the buffer and update the length
        && (sl_legacy_buffer_manager_set_linked_buffers_length(header,
                                                               sl_legacy_buffer_manager_message_buffer_length(header)
                                                               + dataLength) == SL_STATUS_OK)
        // Add the payload
        && (sl_legacy_buffer_manager_copy_to_linked_buffers(data,
                                                            header,
                                                            len,
                                                            dataLength) == SL_STATUS_OK)) {
      // Clear the Stub queue because everything is serialised in header
      sl_zigbee_gp_remove_from_tx_queue(&sli_zigbee_gp_tx_queue);

      // Prepare a RAIL frame to be transported using the additional handle
      uint8_t outPktLength = sl_legacy_buffer_manager_message_buffer_length(header);
      uint8_t outPkt[MAX_BUFFER_SIZE];
      // RAIL Frame : [Total Length (excludes itself) | <-----MAC FRAME ---->| 2 byte CRC]
      outPkt[0] = outPktLength + 2;
      // Copy the data from the buffer
      sl_legacy_buffer_manager_copy_from_linked_buffers(header,
                                                        0,
                                                        &outPkt[1],
                                                        outPktLength);
      // Free the header as the rail frame will be submitted with a new buffer asdu
      sl_legacy_buffer_manager_release_message_buffer(header);

      static sl_zigbee_gp_tx_queue_entry_t copyOfGpStubTxQueue;
      copyOfGpStubTxQueue.inUse = true;
      *asduLength = outPkt[0] + 1;
      memmove(asdu, &outPkt, *asduLength);
      memcpy(&(copyOfGpStubTxQueue.addr), addr, sizeof(sl_zigbee_gp_address_t));
      return &copyOfGpStubTxQueue;
    }
  }
  return NULL;
}

#define macToAppDelay(macTimeStamp) ((sl_rail_get_time(SL_RAIL_EFR32_HANDLE) & 0x00FFFFFF) - (macTimeStamp))

static void appGpScheduleOutgoingGpdf(sl_zigbee_zigbee_packet_type_t packetType,
                                      int8u* packetData,
                                      int8u size_p)
{
  //MAC Frame  : [<---------------MAC Header------------->||<------------------------------------NWK Frame----------------------------------->]
  //              FC(2) | Seq(1) | DstPan(2) | DstAddr(2) || FC(1) | ExtFC(0/1) | SrcId(0/4) | SecFc(0/4) | MIC(0/4) | <------GPDF(1/n)------>
  //              FC    : ExtFC Present(b7)=1| AC(b6)=0| Protocol Ver(b5-b2)=3 GP frames| Frame Type(b1-b0) = 0
  //              ExtFC :  rxAfteTX (b6) = 1 |  AppId(b2-b0) = 0
  if (packetType == SL_ZIGBEE_ZIGBEE_PACKET_TYPE_RAW_MAC
      && size_p > 9 // minimum
      && ((packetData[7] & 0x3C) == 0x0C)            // FC : GP Frames has Protocol Version = 3 ?
      && (((packetData[7] & 0xC3) == 0x01)           // FC : ExtFC(b7) = 0 | AC(b6) = 0 | Frame type(b1-b0) = 1 = Maint, Frame
          || (((packetData[7] & 0xC3) == 0x80)     // FC : ExtFC(b7) = 1 | AC(b6) = 0 | Frame type(b1-b0) = 0 = Data frame
              && ((packetData[8] & 0xC0) == 0x40)))) { // ExtFC :  Direction(b7) = 0 | rxAfteTX (b6) = 1
    // The last 3 bytes of packetData contain the MAC time stamp
    const uint32_t macTimeStamp = ((uint32_t)packetData[size_p - 3] << 16)
                                  + ((uint32_t)packetData[size_p - 2] << 8)
                                  + ((uint32_t)packetData[size_p - 1]);

    // Do we have enough time to schedule the response?
    if (macToAppDelay(macTimeStamp) < GP_RX_OFFSET_USEC) {
      sl_zigbee_gp_address_t gpdAddr;
      gpdAddr.applicationId = SL_ZIGBEE_GP_APPLICATION_SOURCE_ID;
      gpdAddr.id.sourceId = 0;
      if (((packetData[7] & 0xC3) == 0x80)   // FC : ExtFC(b7) = 1 | AC(b6) = 0 | Frame type(b1-b0) = 0 = Data frame
          && ((packetData[8] & 0x07) == SL_ZIGBEE_GP_APPLICATION_SOURCE_ID)) {// ExtFC :  App Id (b2-b0) = 0
        (void) memcpy(&gpdAddr.id.sourceId, &packetData[9], sizeof(sl_zigbee_gp_source_id_t));
      }
      // Is there a queued response for this source ID?
      uint8_t outPktLength;
      uint8_t outPkt[MAX_BUFFER_SIZE];
      sl_zigbee_gp_tx_queue_entry_t* entry = get_gp_stub_tx_queue(&gpdAddr, &outPktLength, (uint8_t*)&outPkt);
      if (entry) {
        // Schedule sending the response.
        sl_rail_scheduler_info_t schedulerInfo = {
          .priority = 50,
          .slip_time = 2000,
          .transaction_time = 5000
        };
        sl_rail_scheduled_tx_config_t scheduledTxConfig = {
          .mode = SL_RAIL_TIME_DELAY,
          // We could reuse macToAppDelay here, but recalculating the delay
          // will give us the most up-to-date timings:
          .when = GP_RX_OFFSET_USEC - macToAppDelay(macTimeStamp)
        };

        sl_rail_status_t UNUSED status = sl_zigbee_af_multirail_demo_send(outPkt,
                                                                          outPktLength,
                                                                          sli_mac_lower_mac_get_radio_channel(0),
                                                                          &scheduledTxConfig,
                                                                          &schedulerInfo);
        sl_zigbee_gp_remove_from_tx_queue(entry);
      }
    }
  }
}

void sl_zigbee_multirail_gp_tx_queue_init(void)
{
#if !defined SL_ZIGBEE_TEST
  sl_zigbee_gp_clear_tx_queue();
  sl_zigbee_af_multirail_demo_init(NULL,
                                   NULL,
                                   true,
                                   sl_rail_get_tx_power_dbm(sl_zigbee_get_rail_handle()),
                                   NULL,
                                   0,
                                   0xFFFF,
                                   NULL);
#endif // SL_ZIGBEE_TEST
}

#endif // SL_CATALOG_ZIGBEE_MULTIRAIL_DEMO_PRESENT
