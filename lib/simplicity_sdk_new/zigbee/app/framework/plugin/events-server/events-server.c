/***************************************************************************//**
 * @file
 * @brief Routines for the Events Server plugin, which implements the server
 *        side of the Events cluster.
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
#include "app/framework/util/common.h"
#include "events-server.h"

#include "zap-cluster-command-parser.h"

#if (SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE != 0)
#define SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
#endif // SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE != 0

#if (SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE != 0)
#define SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
#endif // SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE != 0

#if (SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE != 0)
#define SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
#endif // SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE != 0

#if (SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE != 0)
#define SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
#endif // SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE != 0

#if (SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE != 0)
#define SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
#endif // SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE != 0

// If additional log types are defined in the future this is what will need to
// be modified:
//   update the ami.xml file appropriately
//   add the definition of the new log(s). i.e. static EmberAfEventLog fooLog...
//   add the new enum(s) to the allLogIds array
//   allocate the space for the new events. i.e. static sl_zigbee_af_zcl_event_t fooEvents...
//   add the new log size enum(s) to the SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_EVENT_LOG_SIZE
//   update the sl_zigbee_af_events_cluster_server_init_cb and getEventLog functions

typedef struct {
  sl_zigbee_af_zcl_event_t event;
  uint8_t flags;
} EmberAfEventMetadata;

typedef struct {
  uint8_t nextEntry;
  uint8_t maxEntries;
  EmberAfEventMetadata *entries;
} EmberAfEventLog;

#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
static EmberAfEventLog tamperLog[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
static EmberAfEventLog faultLog[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
static EmberAfEventLog generalLog[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE

static EmberAfEventLog securityLog[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
static EmberAfEventLog networkLog[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT];
#endif

static sl_zigbee_af_event_log_id_t allLogIds[] = {
  SL_ZIGBEE_ZCL_EVENT_LOG_ID_TAMPER_LOG,
  SL_ZIGBEE_ZCL_EVENT_LOG_ID_FAULT_LOG,
  SL_ZIGBEE_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG,
  SL_ZIGBEE_ZCL_EVENT_LOG_ID_SECURITY_EVENT_LOG,
  SL_ZIGBEE_ZCL_EVENT_LOG_ID_NETWORK_EVENT_LOG,
#if (defined(SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT) && defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT))
  SL_ZIGBEE_ZCL_EVENT_LOG_ID_GBCS_GENERAL_EVENT_LOG,
  SL_ZIGBEE_ZCL_EVENT_LOG_ID_GBCS_SECURITY_EVENT_LOG,
#endif
};
#define NUM_EVENT_LOGS (sizeof(allLogIds) / sizeof(allLogIds[0]))

// The following arrays are defined to allocate the space the various event logs.
// The only time they are referenced is in the init function where the address
// of each log is stored in the entries field of the event logs.
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
static EmberAfEventMetadata tamperEvents[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE];
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
static EmberAfEventMetadata faultEvents[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE];
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
static EmberAfEventMetadata generalEvents[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE];
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
static EmberAfEventMetadata securityEvents[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE];
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
static EmberAfEventMetadata networkEvents[SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT]
[SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE];
#endif

#define SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_EVENT_LOG_SIZE \
  (SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE     \
   + SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE    \
   + SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE  \
   + SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE \
   + SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE)
#if (SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_EVENT_LOG_SIZE == 0)
#error The Events Server plugin requires at least one log be enabled (i.e. log size plugin option > 0).
#endif

#define ZCL_EVENTS_VALID                      BIT(1)
#define eventIsValid(emd)                     ((emd)->flags & ZCL_EVENTS_VALID)

#define ZCL_EVENTS_SEARCH_CRITERIA_ALL_EVENT_IDS        0x0000
#define ZCL_EVENTS_SEARCH_CRITERIA_PUBLISH_ALL_EVENTS   0x00

#define GET_EVENT_LOG_LOGID_MASK          0x0F
#define GET_EVENT_LOG_EVENT_CONTROL_MASK  0xF0
#define requestedLogId(ecli)              ((ecli) & GET_EVENT_LOG_LOGID_MASK)
#define retrieveFullInformation(ecli)     ((ecli) & SL_ZIGBEE_ZCL_EVENT_CONTROL_RETRIEVE_FULL_INFORMATION)

#define PUBLISH_EVENT_LOG_PAYLOAD_CONTROL_MASK   0x0F
#define PUBLISH_EVENT_LOG_NUMBER_OF_EVENTS_MASK  0xF0
#define eventCrossesFrameBoundary(lpc)           ((lpc) & EventCrossesFrameBoundary)
#define numberOfEvents(lpc)                      (((lpc) & PUBLISH_EVENT_LOG_NUMBER_OF_EVENTS_MASK) >> 4)

// Publish Event Log: ZCL Header
//   Frame Control (1 byte)
//   Sequence Number (1 byte)
//   Command ID (1 byte)
#define PUBLISH_EVENT_ZCL_HEADER_LENGTH               3

// Publish Event Log: Payload Header
//   Total Number of Matching Events (2 byte)
//   Command Index (1 byte)
//   Total Commands (1 byte)
//   Log Payload Control (1 byte)
#define PUBLISH_EVENT_LOG_PAYLOAD_HEADER_LENGTH       5

// Publish Event Log: Event Info
//   Log ID (1 byte)
//   Event ID (2 bytes)
//   Event Time (4 bytes)
//   Event Data Length (1 byte)
#define PUBLISH_EVENT_LOG_EVENT_INFO_LENGTH           8

// The sli_zigbee_publish_event_log_state_t and sli_zigbee_publish_event_log_partner_t structures are used to
// manage the multiple PublishEventLog responses that may be required when a
// GetEventLog command is received.
typedef struct {
  uint8_t eventsToPublishIndex;
  uint8_t *remainingEventData;
  uint8_t remainingEventDataLen;
} sli_zigbee_publish_event_log_state_t;

typedef struct {
  bool isIntraPan;
  union {
    struct {
      sl_802154_short_addr_t nodeId;
      uint8_t       clientEndpoint;
      uint8_t       serverEndpoint;
    } intra;
    struct {
      sl_802154_long_addr_t eui64;
      sl_802154_pan_id_t panId;
    } inter;
  } pan;
  uint8_t sequence;
  bool publishFullInformation;
  uint16_t maxPayloadLength;
  uint16_t totalMatchingEvents;
  uint16_t eventsToPublishCount;
  uint16_t eventsToPublish[SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_EVENT_LOG_SIZE];
  uint8_t totalCommands;
  uint8_t commandIndex;
  sli_zigbee_publish_event_log_state_t state;
} sli_zigbee_publish_event_log_partner_t;
static sli_zigbee_publish_event_log_partner_t partner;

//------------------------------------------------------------------------------
// Forward Declarations

static EmberAfEventLog *getEventLog(uint8_t endpoint, sl_zigbee_af_event_log_id_t logId);
static void clearEventLog(EmberAfEventLog *eventLog);
static void printEventLog(uint8_t endpoint, sl_zigbee_af_event_log_id_t logId);

static uint16_t findMatchingEvents(uint8_t endpoint,
                                   uint8_t logId,
                                   uint16_t eventId,
                                   uint32_t startTime,
                                   uint32_t endTime,
                                   uint8_t maxEvents,
                                   uint16_t eventOffset,
                                   uint16_t *eventToPublishCount,
                                   uint16_t *eventsToPublish);
static uint8_t getPublishEventLogTotalCommands(uint8_t endpoint,
                                               uint16_t payloadSize,
                                               bool publishFullInformation,
                                               uint16_t eventToPublishCount,
                                               uint16_t *eventsToPublish);
static uint8_t fillPublishEventLogSubPayload(uint8_t endpoint,
                                             sli_zigbee_publish_event_log_state_t *state,
                                             bool publishFullInformation,
                                             uint8_t *subPayload,
                                             uint16_t subPayloadMaxSpace,
                                             uint16_t *subPayloadLen,
                                             uint16_t eventToPublishCount,
                                             uint16_t *eventsToPublish,
                                             uint8_t *logPayloadControl);
static uint8_t findNextEvent(EmberAfEventLog *eventLog,
                             uint8_t prevIndex,
                             uint16_t eventId,
                             uint32_t startTime,
                             uint32_t endTime);

static void mangleCommandForGBCSNonTOMCmd(uint8_t * ep,
                                          uint8_t * logId);
static void initEventLog();

//------------------------------------------------------------------------------
// Callbacks

// Initialize the various event logs.
static bool clusterInitialized = false;
void sl_zigbee_af_events_cluster_server_init_cb(uint8_t endpoint)
{
  UNUSED_VAR(endpoint);
  initEventLog();
}

static void initEventLog()
{
  if (!clusterInitialized) {
    uint8_t i;
    for (i = 0; i < SL_ZIGBEE_ZCL_EVENTS_CLUSTER_SERVER_ENDPOINT_COUNT; i++) {
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
      tamperLog[i].maxEntries = SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_SIZE;
      tamperLog[i].entries = &tamperEvents[i][0];
      clearEventLog(&tamperLog[i]);
#endif

#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
      faultLog[i].maxEntries = SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_SIZE;
      faultLog[i].entries = &faultEvents[i][0];
      clearEventLog(&faultLog[i]);
#endif

#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
      generalLog[i].maxEntries = SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_SIZE;
      generalLog[i].entries = &generalEvents[i][0];
      clearEventLog(&generalLog[i]);
#endif

#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
      securityLog[i].maxEntries = SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_SIZE;
      securityLog[i].entries = &securityEvents[i][0];
      clearEventLog(&securityLog[i]);
#endif

#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
      networkLog[i].maxEntries = SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_SIZE;
      networkLog[i].entries = &networkEvents[i][0];
      clearEventLog(&networkLog[i]);
#endif
    }
    partner.commandIndex = ZCL_EVENTS_INVALID_INDEX;
    clusterInitialized = true;
  }
}

// This function is scheduled upon receipt of a GetEventLog command. Every tick
// we'll send a PublishEventLog command back to the requestor until all responses
// are sent.
void sl_zigbee_af_events_cluster_server_tick_cb(uint8_t endpoint)
{
  // Allocate for the largest possible size, unfortunately
  uint8_t subPayload[SL_ZIGBEE_AF_MAXIMUM_SEND_PAYLOAD_LENGTH];
  uint16_t subPayloadLen = 0;
  uint8_t logPayloadControl;
  uint16_t maxSubPayloadLen = partner.maxPayloadLength - PUBLISH_EVENT_ZCL_HEADER_LENGTH - PUBLISH_EVENT_LOG_PAYLOAD_HEADER_LENGTH;
  uint8_t numberOfEvents = 0;

  numberOfEvents = fillPublishEventLogSubPayload(endpoint,
                                                 &partner.state,
                                                 partner.publishFullInformation,
                                                 subPayload,
                                                 maxSubPayloadLen,
                                                 &subPayloadLen,
                                                 partner.eventsToPublishCount,
                                                 partner.eventsToPublish,
                                                 &logPayloadControl);
  if (numberOfEvents > 0) {
    sl_zigbee_af_events_cluster_println("TX PublishEventLog at command index 0x%02X of 0x%02X, number of events 0x%02X",
                                        partner.commandIndex,
                                        partner.totalCommands,
                                        numberOfEvents);
//IRP282 & Jira EMAPPFWKV2-1376: clear most significant nibble of logPayloadControl when the PublishEventLog
//is part of a GBZ message i.e. WAN only commands and when the command is meant for a device on the HAN, logPayloadControl
//field should be as defined per ZSE specs.
#if defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT)
    if (partner.pan.intra.clientEndpoint == SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_HIDDEN_CLIENT_SIDE_ENDPOINT) {
      CLEARBITS(logPayloadControl, PUBLISH_EVENT_LOG_NUMBER_OF_EVENTS_MASK);
    }
#endif

#ifdef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
    // GBCS explicitly lists some commands that need to be sent with "disable
    // default response" flag set. This is one of them.
    // We make it conditional on GBCS so it does not affect standard SE apps.
    sl_zigbee_af_set_disable_default_response(SL_ZIGBEE_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT);
#endif

    sl_zigbee_af_fill_command_events_cluster_publish_event_log(partner.totalMatchingEvents,
                                                               partner.commandIndex,
                                                               partner.totalCommands,
                                                               logPayloadControl,
                                                               subPayload,
                                                               subPayloadLen);
    // Rewrite the sequence number of the response so it matches the request.
    appResponseData[1] = partner.sequence;
    if (partner.isIntraPan) {
      sl_zigbee_af_set_command_endpoints(partner.pan.intra.serverEndpoint,
                                         partner.pan.intra.clientEndpoint);
      sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, partner.pan.intra.nodeId);
    } else {
      sl_zigbee_af_send_command_inter_pan(partner.pan.inter.panId,
                                          partner.pan.inter.eui64,
                                          SL_ZIGBEE_NULL_NODE_ID,
                                          0, // multicast id - unused
                                          SE_PROFILE_ID);
    }
    partner.commandIndex++;
  }

  if (numberOfEvents != 0
      && partner.commandIndex < partner.totalCommands) {
    sl_zigbee_zcl_schedule_server_tick(endpoint,
                                       ZCL_EVENTS_CLUSTER_ID,
                                       MILLISECOND_TICKS_PER_QUARTERSECOND);
  } else {
    partner.commandIndex = ZCL_EVENTS_INVALID_INDEX;
  }
}

// The GetEventLog command allows a client to request events from a server's
// event logs. One or more PublishEventLog commands are returned on receipt
// of this command. A ZCL Default Response with status NOT_FOUND shall be
// returned if no events match the given search criteria.
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_events_cluster_get_event_log_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_events_cluster_get_event_log_command_t cmd_data;

  if (zcl_decode_events_cluster_get_event_log_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint8_t endpoint = sl_zigbee_af_current_endpoint();

  sl_zigbee_af_events_cluster_println("RX: GetEventLog 0x%02X, 0x%04X, 0x%08X, 0x%08X, 0x%02X, 0x%04X",
                                      cmd_data.eventControlLogId,
                                      cmd_data.eventId,
                                      cmd_data.startTime,
                                      cmd_data.endTime,
                                      cmd_data.numberOfEvents,
                                      cmd_data.eventOffset);

  mangleCommandForGBCSNonTOMCmd(&endpoint,
                                &cmd_data.eventControlLogId);

  // Only one GetEventLog can be processed at a time.
  if (partner.commandIndex != ZCL_EVENTS_INVALID_INDEX) {
    sl_zigbee_af_events_cluster_println("%s%s%s",
                                        "Error: ",
                                        "Cannot get event log: ",
                                        "only one GetEventLog command can be processed at a time");
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }

  partner.totalMatchingEvents = findMatchingEvents(endpoint,
                                                   requestedLogId(cmd_data.eventControlLogId),
                                                   cmd_data.eventId,
                                                   cmd_data.startTime,
                                                   cmd_data.endTime,
                                                   cmd_data.numberOfEvents,
                                                   cmd_data.eventOffset,
                                                   &partner.eventsToPublishCount,
                                                   partner.eventsToPublish);
  if (partner.totalMatchingEvents == 0 || partner.totalMatchingEvents <= cmd_data.eventOffset) {
    sl_zigbee_af_events_cluster_println("No matching events to return!");
    return SL_ZIGBEE_ZCL_STATUS_NOT_FOUND;
  }
  partner.isIntraPan = (cmd->interPanHeader == NULL);
  if (partner.isIntraPan) {
    partner.pan.intra.nodeId = cmd->source;
    partner.pan.intra.clientEndpoint = cmd->apsFrame->sourceEndpoint;
    partner.pan.intra.serverEndpoint = cmd->apsFrame->destinationEndpoint;
  } else {
    partner.pan.inter.panId = cmd->interPanHeader->panId;
    memcpy(partner.pan.inter.eui64, cmd->interPanHeader->longAddress, EUI64_SIZE);
  }
  partner.sequence = cmd->seqNum;
  partner.maxPayloadLength = SL_ZIGBEE_AF_MAXIMUM_SEND_PAYLOAD_LENGTH;
  partner.publishFullInformation = retrieveFullInformation(cmd_data.eventControlLogId);
  partner.totalCommands = getPublishEventLogTotalCommands(endpoint,
                                                          partner.maxPayloadLength,
                                                          partner.publishFullInformation,
                                                          partner.eventsToPublishCount,
                                                          partner.eventsToPublish);
  partner.commandIndex = 0;
  partner.state.eventsToPublishIndex = 0;
  partner.state.remainingEventData = NULL;
  partner.state.remainingEventDataLen = 0;
  sl_zigbee_zcl_schedule_server_tick(endpoint,
                                     ZCL_EVENTS_CLUSTER_ID,
                                     MILLISECOND_TICKS_PER_QUARTERSECOND);
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

// The ClearEventLog command requests that an Events server device clear the
// specified event log(s). The Events server device SHOULD clear the requested
// events logs, however it is understood that market specific restrictions may
// be applied to prevent this.
//
// To determine whether or not we should clear the requested log we will
// callback to the application with the requested logId and let the application
// tell us which logs are OK to be cleared.
//
// Note: that when setting the clearedEventLogs bitmap included within the
// ClearEventLogResponse we take advantage of the fact that the logId value
// currently indicates the the bit number in the bitmap to be set.  If this ever
// changes we'll need to modify the way we set the clearedEventLogs.
sl_zigbee_af_zcl_request_status_t sl_zigbee_af_events_cluster_clear_event_log_request_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_events_cluster_clear_event_log_request_command_t cmd_data;
  uint8_t i, clearedEventLogs = 0;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();

  if (zcl_decode_events_cluster_clear_event_log_request_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_events_cluster_println("RX: ClearEventLogRequest 0x%02X", cmd_data.logId);

  mangleCommandForGBCSNonTOMCmd(&endpoint,
                                &cmd_data.logId);

#if defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT)
  if ((endpoint == SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT)
      && (cmd_data.logId == SL_ZIGBEE_ZCL_EVENT_LOG_ID_SECURITY_EVENT_LOG)) {
    sl_zigbee_af_events_cluster_println("ERR: Modifying or deleting entries from the GPF Security Log is not allowed.");
    return SL_ZIGBEE_ZCL_STATUS_FAILURE;
  }
#endif

  if (sl_zigbee_af_events_server_ok_to_clear_log_cb((sl_zigbee_af_event_log_id_t)cmd_data.logId)) {
    if (!sl_zigbee_af_events_server_clear_event_log(endpoint, (sl_zigbee_af_event_log_id_t)cmd_data.logId)) {
      sl_zigbee_af_events_cluster_println("%s%s%s",
                                          "Error: ",
                                          "Cannot clear event log: ",
                                          "invalid endpoint and/or logId");
      return SL_ZIGBEE_ZCL_STATUS_FAILURE;
    }
    clearedEventLogs |= BIT(cmd_data.logId);
  } else if (SL_ZIGBEE_ZCL_EVENT_LOG_ID_ALL_LOGS == cmd_data.logId) {
    for (i = 0; i < NUM_EVENT_LOGS; i++) {
      if (sl_zigbee_af_events_server_ok_to_clear_log_cb(allLogIds[i])) {
        if (!sl_zigbee_af_events_server_clear_event_log(endpoint, allLogIds[i])) {
          sl_zigbee_af_events_cluster_println("%s%s%s",
                                              "Error: ",
                                              "Cannot clear event log: ",
                                              "invalid endpoint");
          return SL_ZIGBEE_ZCL_STATUS_FAILURE;
        }
        clearedEventLogs |= BIT(allLogIds[i]);
      }
    }
  }

  sl_zigbee_af_fill_command_events_cluster_clear_event_log_response(clearedEventLogs);
  sl_zigbee_af_send_response();
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

//------------------------------------------------------------------------------
// Other Miscellaneous functions

// Clear the specified event log by marking all entries invalid. Returns false
// if endpoint or logId is invalid
bool sl_zigbee_af_events_server_clear_event_log(uint8_t endpoint, sl_zigbee_af_event_log_id_t logId)
{
  uint8_t i;
  EmberAfEventLog *eventLog;
  bool status = false;

  if (SL_ZIGBEE_ZCL_EVENT_LOG_ID_ALL_LOGS == logId) {
    for (i = 0; i < NUM_EVENT_LOGS; i++) {
      eventLog = getEventLog(endpoint, allLogIds[i]);
      // we know that logId is valid so if getEventLog returns NULL it must
      // mean that endpoint is invalid so we'll break out of the loop and
      // return false immediately.
      if (NULL == eventLog) {
        break;
      }
      clearEventLog(eventLog);
    }

    status = true;
    goto kickout;
  } else {
    eventLog = getEventLog(endpoint, logId);
    if (NULL != eventLog) {
      clearEventLog(eventLog);
      status = true;
      goto kickout;
    }
  }

  kickout:

#if (defined(SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT) && defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT))
  /* Per GBCS req via CHTS v1.46, Section 4.5.13. We should log an event in
   * the security log when a command to clear all event log entries in the GPF ESI / CHF Event Log
   * is executed.*/
  if (status
      && ((SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT == endpoint)
          || (SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT == endpoint))
      && ((SL_ZIGBEE_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG == logId)
          || (SL_ZIGBEE_ZCL_EVENT_LOG_ID_GBCS_GENERAL_EVENT_LOG == logId))) {
    uint8_t index;
    sl_zigbee_af_zcl_event_t event;

    event.eventId = GBCS_EVENT_ID_EVENT_LOG_CLEARED;
    event.eventTime = sl_zigbee_af_get_current_time();
    event.eventData[0] = 0x00;

    index = sl_zigbee_af_events_server_add_event(endpoint,
                                                 SL_ZIGBEE_ZCL_EVENT_LOG_ID_SECURITY_EVENT_LOG,
                                                 &event);
    if (index == ZCL_EVENTS_INVALID_INDEX) {
      sl_zigbee_af_events_cluster_println("ERR: Failed to log event (Event Log Cleared) to Security Event Log on ep(0x%02X)",
                                          endpoint);
    } else {
      sl_zigbee_af_events_cluster_println("INFO: Clear Event Log event added to Security Event Log on ep(0x%02X) at index(0x%02X)",
                                          endpoint,
                                          index);
    }
  }
#endif

  return status;
}

// Print the specified event log.
void sl_zigbee_af_events_server_print_event_log(uint8_t endpoint, sl_zigbee_af_event_log_id_t logId)
{
  uint8_t i;

  if (SL_ZIGBEE_ZCL_EVENT_LOG_ID_ALL_LOGS == logId) {
    for (i = 0; i < NUM_EVENT_LOGS; i++) {
      printEventLog(endpoint, allLogIds[i]);
    }
  } else {
    printEventLog(endpoint, logId);
  }
}

// Print an event
void sl_zigbee_af_events_server_print_event(const sl_zigbee_af_zcl_event_t *event)
{
  sl_zigbee_af_events_cluster_println("       eventId: 0x%04X", event->eventId);
  sl_zigbee_af_events_cluster_println("     eventTime: 0x%08X", event->eventTime);
  sl_zigbee_af_events_cluster_println("  eventDataLen: 0x%02X", sl_zigbee_af_string_length(event->eventData));
  sl_zigbee_af_events_cluster_print("     eventData: ");
  sl_zigbee_af_events_cluster_print_string(event->eventData);
  sl_zigbee_af_events_cluster_println("");
}

// Retrieves the event at the index.  Returns false if logId or index is invalid.
bool sl_zigbee_af_events_server_get_event(uint8_t endpoint,
                                          sl_zigbee_af_event_log_id_t logId,
                                          uint8_t index,
                                          sl_zigbee_af_zcl_event_t *event)
{
  EmberAfEventLog *eventLog = getEventLog(endpoint, logId);
  if (NULL != eventLog  && index < eventLog->maxEntries) {
    memcpy(event, &eventLog->entries[index].event, sizeof(sl_zigbee_af_zcl_event_t));
    return true;
  }
  return false;
}

// Sets the event at the index.  Returns false if logId or index is invalid.
bool sl_zigbee_af_events_server_set_event(uint8_t endpoint,
                                          sl_zigbee_af_event_log_id_t logId,
                                          uint8_t index,
                                          const sl_zigbee_af_zcl_event_t *event)
{
  bool status = false;
  EmberAfEventLog *eventLog = getEventLog(endpoint, logId);
  if (NULL != eventLog  && index < eventLog->maxEntries) {
    if (NULL == event) {
      eventLog->entries[index].flags &= ~ZCL_EVENTS_VALID;
      status = true;
      goto kickout;
    }

    memcpy(&eventLog->entries[index].event, event, sizeof(sl_zigbee_af_zcl_event_t));
    eventLog->entries[index].flags |= ZCL_EVENTS_VALID;
    // ensure we don't overwrite this entry
    if (index > eventLog->nextEntry) {
      eventLog->nextEntry  = index + 1;
    }
    status = true;
    goto kickout;
  }
  kickout:

  if (status) {
    sl_zigbee_af_events_server_log_data_updated_cb(sl_zigbee_af_current_command());
  }
  return status;
}

// Add an event to the specified event log.
uint8_t sl_zigbee_af_events_server_add_event(uint8_t endpoint,
                                             sl_zigbee_af_event_log_id_t logId,
                                             const sl_zigbee_af_zcl_event_t *event)
{
  EmberAfEventLog *eventLog = getEventLog(endpoint, logId);
  uint8_t index;
  if (NULL != eventLog) {
    uint8_t nextEntry = (eventLog->nextEntry)++;
    if (eventLog->nextEntry >= eventLog->maxEntries) {
      eventLog->nextEntry = 0;
    }
    memcpy(&eventLog->entries[nextEntry].event, event, sizeof(sl_zigbee_af_zcl_event_t));
    eventLog->entries[nextEntry].flags |= ZCL_EVENTS_VALID;
    index = nextEntry;
    goto kickout;
  }

  index = ZCL_EVENTS_INVALID_INDEX;
  kickout:

  if (index == ZCL_EVENTS_INVALID_INDEX) {
    sl_zigbee_af_events_cluster_println("Cannot add Event to log 0x%02X", logId);
  } else {
    sl_zigbee_af_events_cluster_println("Event added to log 0x%02X at index 0x%02X", logId, index);
    sl_zigbee_af_events_server_log_data_updated_cb(sl_zigbee_af_current_command());
  }

  return index;
}

// Publish an event.
void sl_zigbee_af_events_server_publish_event_message(sl_802154_short_addr_t nodeId,
                                                      uint8_t srcEndpoint,
                                                      uint8_t dstEndpoint,
                                                      sl_zigbee_af_event_log_id_t logId,
                                                      uint8_t index,
                                                      uint8_t eventControl)
{
  sl_status_t status;
  sl_zigbee_af_zcl_event_t event;

  if (!sl_zigbee_af_events_server_get_event(srcEndpoint, logId, index, &event)) {
    sl_zigbee_af_events_cluster_println("%s%s%s",
                                        "Error: ",
                                        "Cannot publish event: ",
                                        "invalid logId and/or index");
    return;
  }

  sl_zigbee_af_fill_command_events_cluster_publish_event(logId,
                                                         event.eventId,
                                                         event.eventTime,
                                                         eventControl,
                                                         event.eventData);
  sl_zigbee_af_set_command_endpoints(srcEndpoint, dstEndpoint);
  sl_zigbee_af_get_command_aps_frame()->options |= SL_ZIGBEE_APS_OPTION_SOURCE_EUI64;
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, nodeId);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_events_cluster_println("%s%s0x%02X",
                                        "Error: ",
                                        "Cannot publish event: ",
                                        status);
  }
}

//------------------------------------------------------------------------------
// Internal Functions

// Given an endpoint and a logId return the pointer to the corresponding event log.
static EmberAfEventLog *getEventLog(uint8_t endpoint, sl_zigbee_af_event_log_id_t logId)
{
  EmberAfEventLog *eventLog = NULL;
  uint8_t ep = sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, ZCL_EVENTS_CLUSTER_ID);

  if (ep == SL_ZIGBEE_AF_INVALID_ENDPOINT_INDEX) {
    return NULL;
  }

  switch (logId) {
    case SL_ZIGBEE_ZCL_EVENT_LOG_ID_TAMPER_LOG:
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_TAMPER_LOG_ENABLE
      eventLog = &tamperLog[ep];
#else
      sl_zigbee_af_events_cluster_println("%s%s",
                                          "Error: ",
                                          "Tamper log disabled");
#endif
      break;

    case SL_ZIGBEE_ZCL_EVENT_LOG_ID_FAULT_LOG:
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_FAULT_LOG_ENABLE
      eventLog = &faultLog[ep];
#else
      sl_zigbee_af_events_cluster_println("%s%s",
                                          "Error: ",
                                          "Fault log disabled");
#endif
      break;

    case SL_ZIGBEE_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG:
#if (defined(SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT) && defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT))
    case SL_ZIGBEE_ZCL_EVENT_LOG_ID_GBCS_GENERAL_EVENT_LOG:
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_GENERAL_LOG_ENABLE
      eventLog = &generalLog[ep];
#else
      sl_zigbee_af_events_cluster_println("%s%s",
                                          "Error: ",
                                          "General log disabled");
#endif
      break;

    case SL_ZIGBEE_ZCL_EVENT_LOG_ID_SECURITY_EVENT_LOG:
#if (defined(SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT) && defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT))
    case SL_ZIGBEE_ZCL_EVENT_LOG_ID_GBCS_SECURITY_EVENT_LOG:
#endif
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_SECURITY_LOG_ENABLE
      eventLog = &securityLog[ep];
#else
      sl_zigbee_af_events_cluster_println("%s%s",
                                          "Error: ",
                                          "Security log disabled");
#endif
      break;

    case SL_ZIGBEE_ZCL_EVENT_LOG_ID_NETWORK_EVENT_LOG:
#ifdef SL_ZIGBEE_AF_PLUGIN_EVENTS_SERVER_NETWORK_LOG_ENABLE
      eventLog = &networkLog[ep];
#else
      sl_zigbee_af_events_cluster_println("%s%s",
                                          "Error: ",
                                          "Network log disabled");
#endif
      break;

    default:
      sl_zigbee_af_events_cluster_println("%s%s0x%02X",
                                          "Error: ",
                                          "Unknown log: ",
                                          logId);
      break;
  }

  return eventLog;
}

// Clear the given event log by marking all entries invalid
static void clearEventLog(EmberAfEventLog *eventLog)
{
  uint8_t i;

  for (i = 0; i < eventLog->maxEntries; i++) {
    eventLog->entries[i].flags &= ~ZCL_EVENTS_VALID;
  }
  eventLog->nextEntry = 0;
  sl_zigbee_af_events_server_log_data_updated_cb(sl_zigbee_af_current_command());
}

// Print all valid entries within the given event log
static void printEventLog(uint8_t endpoint, sl_zigbee_af_event_log_id_t logId)
{
  const EmberAfEventLog *eventLog;
  uint8_t i, count = 0;

  eventLog = getEventLog(endpoint, logId);
  if (NULL == eventLog) {
    return;
  }

  // Count the number of valid entries in the given event log. We could use
  // the nextEntry value within the eventLog for this if we never call
  // sl_zigbee_af_events_server_set_event will a NULL event pointer thus potentially creating
  // holes in the log
  for (i = 0; i < eventLog->maxEntries; i++) {
    if (eventIsValid(&eventLog->entries[i])) {
      count++;
    }
  }

  sl_zigbee_af_events_cluster_println("Log: 0x%02X Total 0x%02X", logId, count);
  sl_zigbee_af_events_cluster_println("");
  if (count > 0) {
    for (i = 0; i < eventLog->maxEntries; i++) {
      if (!eventIsValid(&eventLog->entries[i])) {
        continue;
      }
      sl_zigbee_af_events_cluster_println("Index: 0x%02X", i);
      sl_zigbee_af_events_server_print_event(&eventLog->entries[i].event);
    }
  }
}

// find events needing to be published that match the given criteria
static uint16_t findMatchingEvents(uint8_t endpoint,
                                   uint8_t logId,
                                   uint16_t eventId,
                                   uint32_t startTime,
                                   uint32_t endTime,
                                   uint8_t maxEvents,
                                   uint16_t eventOffset,
                                   uint16_t *eventsToPublishCount,
                                   uint16_t *eventsToPublish)
{
  uint8_t i;
  EmberAfEventLog *eventLogs[NUM_EVENT_LOGS];
  uint8_t candidateEventIndex[NUM_EVENT_LOGS];
  bool eventsFound = false;
  sl_zigbee_af_zcl_event_t *nextEvent, *eventCandidate;
  uint8_t nextEventLogIndex = 0;
  uint16_t totalMatchingEvents = 0, skippedEvents = 0;

  *eventsToPublishCount = 0;

  // Find all the potential first events. These are the most recent events
  // matching the given criteria in each of the event logs.
  for (i = 0; i < NUM_EVENT_LOGS; i++) {
    eventLogs[i] = NULL;
    candidateEventIndex[i] = ZCL_EVENTS_INVALID_INDEX;

    if (logId != SL_ZIGBEE_ZCL_EVENT_LOG_ID_ALL_LOGS
        && logId != allLogIds[i]) {
      continue;
    }

    eventLogs[i] = getEventLog(endpoint, allLogIds[i]);
    if (eventLogs[i] == NULL) {
      continue;
    }

    candidateEventIndex[i] = findNextEvent(eventLogs[i],
                                           ZCL_EVENTS_INVALID_INDEX,
                                           eventId,
                                           startTime,
                                           endTime);
    if (!eventsFound && candidateEventIndex[i] != ZCL_EVENTS_INVALID_INDEX) {
      eventsFound = true;
    }
  }

  // Now compare each potential next event to each other and choose
  // the appropriate next event.  Once chosen, process it, and find
  // the next potential event from the log from which the chosen event
  // is located. Continue this process until all events are processed.
  while (eventsFound) {
    nextEvent = NULL;
    for (i = 0; i < NUM_EVENT_LOGS; i++) {
      if (candidateEventIndex[i] == ZCL_EVENTS_INVALID_INDEX) {
        continue;
      }
      eventCandidate = &(eventLogs[i]->entries[candidateEventIndex[i]].event);
      if (nextEvent == NULL
          || eventCandidate->eventTime > nextEvent->eventTime) {
        nextEvent = eventCandidate;
        nextEventLogIndex = i;
      }
    }
    if (nextEvent != NULL) {
      // If we get here then we found the next most recent matching event.
      // We need to find the next event in the log from which this event is
      // located then bump the total number of matching events but then decide
      // if we should add the event to the given eventsToPublish array.  This
      // will be determined based on the number of events requested and the
      // offset given.
      uint8_t nextEventIndex = candidateEventIndex[nextEventLogIndex];
      candidateEventIndex[nextEventLogIndex] = findNextEvent(eventLogs[nextEventLogIndex],
                                                             candidateEventIndex[nextEventLogIndex],
                                                             eventId,
                                                             startTime,
                                                             endTime);
      totalMatchingEvents++;

      if (skippedEvents < eventOffset) {
        skippedEvents++;
        continue;
      }

      if (maxEvents == ZCL_EVENTS_SEARCH_CRITERIA_PUBLISH_ALL_EVENTS
          || *eventsToPublishCount < maxEvents) {
        *eventsToPublish++ = ((uint16_t)allLogIds[nextEventLogIndex]) << 8 | (uint16_t)nextEventIndex;
        (*eventsToPublishCount)++;
      }
    } else {
      eventsFound = false;
    }
  }

  return totalMatchingEvents;
}

// calculate how many Publish Event Log commands it will take to publish all
// the events in the given eventsToPublish array..
static uint8_t getPublishEventLogTotalCommands(uint8_t endpoint,
                                               uint16_t maxPayloadLength,
                                               bool publishFullInformation,
                                               uint16_t eventsToPublishCount,
                                               uint16_t *eventsToPublish)
{
  uint8_t totalCommands = 0;
  uint8_t numberOfEvents;
  sli_zigbee_publish_event_log_state_t state;
  uint16_t subPayloadLen = 0;
  uint16_t maxSubPayloadLen = maxPayloadLength - PUBLISH_EVENT_ZCL_HEADER_LENGTH - PUBLISH_EVENT_LOG_PAYLOAD_HEADER_LENGTH;

  state.eventsToPublishIndex = 0;
  state.remainingEventData = NULL;
  state.remainingEventDataLen = 0;
  numberOfEvents = fillPublishEventLogSubPayload(endpoint,
                                                 &state,
                                                 publishFullInformation,
                                                 NULL,
                                                 maxSubPayloadLen,
                                                 &subPayloadLen,
                                                 eventsToPublishCount,
                                                 eventsToPublish,
                                                 NULL); // logPayloadControl
  while (numberOfEvents > 0) {
    totalCommands++;
    numberOfEvents = fillPublishEventLogSubPayload(endpoint,
                                                   &state,
                                                   publishFullInformation,
                                                   NULL,
                                                   maxSubPayloadLen,
                                                   &subPayloadLen,
                                                   eventsToPublishCount,
                                                   eventsToPublish,
                                                   NULL); // logPayloadControl
  }
  return totalCommands;
}

// This function will loop through the events in the given eventsToPublish list
// packing as many events into the given Publish Event Log subPayload buffer
// as will fit. The format of the contents of the Publish Event Log subPayload
// is a repeated set of the following fields:
//
// Octets:         1    |      2    |      4     |    1..255   |
// Field Name:  Log ID  | Event ID  | Event Time | Event Data  |
//
// Because there are several logs and many events per log event traversal is
// maintained in the given sli_zigbee_publish_event_log_state_t structure.
//
// If the given subPayload pointer is NULL this means that the caller is not
// building the command to send at this time but rather just trying to
// calculate how many commands it will take to send all the events (see
// sl_zigbee_af_events_cluster_get_event_log_cb).
//
// The return value is the number of events packed in the subPayload buffer.
// This function will also fill a reference to a passed byte with the
// log payload control. If the caller does not care about the logPayloadControl,
// they can set this value to NULL. This byte includes the number of events in
// the upper nibble and whether or not an event in the subPayload crosses over
// the frame boundary in the lower nibble. Remember, GBCS wants this upper
// nibble to be set to 0x0, since the number of events could total 100 which
// cannot fit in 4 bits.
static uint8_t fillPublishEventLogSubPayload(uint8_t endpoint,
                                             sli_zigbee_publish_event_log_state_t *state,
                                             bool publishFullInformation,
                                             uint8_t *subPayload,
                                             uint16_t subPayloadMaxSpace,
                                             uint16_t *subPayloadLen,
                                             uint16_t eventsToPublishCount,
                                             uint16_t *eventsToPublish,
                                             uint8_t *logPayloadControl)
{
  uint16_t subPayloadRemainingSpace = subPayloadMaxSpace;
  EmberAfEventLog *eventLog;
  sl_zigbee_af_zcl_event_t *event;
  uint8_t eventDataLen;
  uint8_t *eventData;
  uint8_t numberOfEvents = 0;
  bool eventCrossesFrameBoundary = false;

  *subPayloadLen = 0;

  // Before finding the next event to publish first check to see if there is
  // remaining event data from a previously published event that needs to be
  // sent.
  if (state->remainingEventDataLen > 0) {
    numberOfEvents++;
    eventCrossesFrameBoundary = true;
    if (state->remainingEventDataLen > subPayloadRemainingSpace) {
      if (subPayload != NULL) {
        memcpy(subPayload, state->remainingEventData, subPayloadRemainingSpace);
        *subPayloadLen += subPayloadRemainingSpace;
      }
      state->remainingEventData += subPayloadRemainingSpace;
      state->remainingEventDataLen -= subPayloadRemainingSpace;
      subPayloadRemainingSpace = 0;
    } else {
      if (subPayload != NULL) {
        memcpy(subPayload, state->remainingEventData, state->remainingEventDataLen);
        *subPayloadLen += state->remainingEventDataLen;
      }
      state->remainingEventData = NULL;
      state->remainingEventDataLen = 0;
      subPayloadRemainingSpace -= state->remainingEventDataLen;
    }
  }

  // Loop through packing as many events as possible into the subPayload buffer.
  while (subPayloadRemainingSpace > PUBLISH_EVENT_LOG_EVENT_INFO_LENGTH
         && state->eventsToPublishIndex < eventsToPublishCount) {
    uint8_t logId = (eventsToPublish[state->eventsToPublishIndex] & 0xFF00) >> 8;
    uint8_t eventIndex = (eventsToPublish[state->eventsToPublishIndex] & 0x00FF);

    state->eventsToPublishIndex++;
    eventLog = getEventLog(endpoint, (sl_zigbee_af_event_log_id_t)logId);
    if (eventLog == NULL) {
      continue;
    }

    event = &(eventLog->entries[eventIndex].event);
    numberOfEvents++;
    eventDataLen = (publishFullInformation) ? sl_zigbee_af_string_length(event->eventData) : 0;
    if (subPayload != NULL) {
      sl_zigbee_af_copy_int8u(subPayload, *subPayloadLen, logId);
      (*subPayloadLen)++;
      sl_zigbee_af_copy_int16u(subPayload, *subPayloadLen, event->eventId);
      (*subPayloadLen) += 2;
      sl_zigbee_af_copy_int32u(subPayload, *subPayloadLen, event->eventTime);
      (*subPayloadLen) += 4;
      sl_zigbee_af_copy_int8u(subPayload, *subPayloadLen, eventDataLen);
      (*subPayloadLen)++;
    }
    subPayloadRemainingSpace -= PUBLISH_EVENT_LOG_EVENT_INFO_LENGTH;

    if (eventDataLen != 0) {
      eventData = &event->eventData[1];
      if (eventDataLen <= subPayloadRemainingSpace) {
        // The data associated with this event fits in the remaining space so
        // copy it in and move on to the next event.
        if (subPayload != NULL) {
          memcpy(&subPayload[*subPayloadLen], eventData, eventDataLen);
          *subPayloadLen += eventDataLen;
        }
        subPayloadRemainingSpace -= eventDataLen;
      } else {
        // The data associated with this event won't fit in the remaining space.
        // We could split the event data so that part of the event is in this frame
        // and the remaining data is in the next but the spec says "Wherever
        // possible events SHOULD NOT be sent across payload boundaries". So the
        // only case where we should split the event is when the event is too big
        // for a single frame.
        if (eventDataLen > subPayloadMaxSpace) {
          // Event data is too big for a single frame so split it up.
          eventCrossesFrameBoundary = true;
          if (subPayload != NULL) {
            memcpy(&subPayload[*subPayloadLen], eventData, subPayloadRemainingSpace);
            *subPayloadLen += subPayloadRemainingSpace;
          }
          state->remainingEventData = eventData + subPayloadRemainingSpace;;
          state->remainingEventDataLen = eventDataLen - subPayloadRemainingSpace;
          subPayloadRemainingSpace = 0;
        } else {
          // The event will fit into a single frame just not this one so
          // backout the event from this buffer by decrementing the subPayload
          // length and number of events. Also, decrement the eventsToPublishIndex in the
          // state struct as we want to start with this event in the next command.
          if (subPayload != NULL) {
            *subPayloadLen -= PUBLISH_EVENT_LOG_EVENT_INFO_LENGTH;
          }
          numberOfEvents--;
          state->eventsToPublishIndex--;
          break;
        }
      }
    }
  }

  if (logPayloadControl) {
    *logPayloadControl = (numberOfEvents << 4);
    if (eventCrossesFrameBoundary) {
      *logPayloadControl |= SL_ZIGBEE_ZCL_EVENT_LOG_PAYLOAD_CONTROL_EVENT_CROSSES_FRAME_BOUNDARY;
    }
  }

  return numberOfEvents;
}

// Find the next most recent event from the given log matching the given criteria.
// and starting from the given index.
static uint8_t findNextEvent(EmberAfEventLog *eventLog,
                             uint8_t prevIndex,
                             uint16_t eventId,
                             uint32_t startTime,
                             uint32_t endTime)
{
  uint8_t i, index = ZCL_EVENTS_INVALID_INDEX;
  EmberAfEventMetadata *eventMetadata;

  prevIndex = (prevIndex == ZCL_EVENTS_INVALID_INDEX) ? eventLog->maxEntries : prevIndex;

  // Need to send events in "most recent first" order.  We assume that events
  // are added as they occur so the first element is the oldest and the last
  // element is the newest.  As such we traverse the event log in reverse order.
  for (i = prevIndex - 1; i < eventLog->maxEntries; i--) {
    eventMetadata = &eventLog->entries[i];
    // sl_zigbee_af_events_server_print_event(&eventMetadata->event);
    if (!eventIsValid(eventMetadata)) {
      continue;
    }

    if (eventId != ZCL_EVENTS_SEARCH_CRITERIA_ALL_EVENT_IDS
        && eventId != eventMetadata->event.eventId) {
      continue;
    }

    if (eventMetadata->event.eventTime < startTime
        || eventMetadata->event.eventTime >= endTime) {
      continue;
    }

    index = i;
    break;
  }

  return index;
}

/*
 * @brief Redirecting GBCS Non-TOM messages
 *
 * Events cluster cmds are all redirected to the ESI endpoint for parsing.
 * Depending on the logId, we need to redirect them accordingly.
 *
 */
static void mangleCommandForGBCSNonTOMCmd(uint8_t * endpoint,
                                          uint8_t * logId)
{
#if (defined(SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT) && defined(SL_CATALOG_ZIGBEE_GAS_PROXY_FUNCTION_PRESENT))
  uint8_t newLogId = *logId & 0x0F;
  if (*endpoint == SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_ESI_ENDPOINT) {
    // When processing a ZSE Get Event Log command or a ZSE Clear Event Log
    // command with a Log ID nibble of 0x6 (GSME Event Log) or 0x7 (GSME
    // Security Log), a GPF shall process the command using the relevant GSME
    // Proxy Log copy of the GSME Event or Security Log. Other values of Log ID
    // shall refer to the GPF's own logs.
    if (newLogId == GBCS_EVENT_LOG_ID_GSME_EVENT_LOG
        || newLogId == GBCS_EVENT_LOG_ID_GSME_SECURITY_EVENT_LOG) {
      *endpoint = SL_ZIGBEE_AF_PLUGIN_GAS_PROXY_FUNCTION_MIRROR_ENDPOINT;
      sl_zigbee_af_events_cluster_println("Redirecting to GPF meter mirror endpoint(0x%02X) with logId(0x%02X)",
                                          *endpoint, newLogId);
    } else {
      sl_zigbee_af_events_cluster_println("Redirecting to GPF esi endpoint(0x%02X) with logId(0x%02X)",
                                          *endpoint, newLogId);
    }
  }

  *logId = (*logId & 0xF0) | (newLogId & 0x0F);
#else
  UNUSED_VAR(endpoint);
  UNUSED_VAR(logId);
#endif
}

uint32_t sl_zigbee_af_events_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                          sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_GET_EVENT_LOG_COMMAND_ID:
      {
        status = sl_zigbee_af_events_cluster_get_event_log_cb(cmd);
        break;
      }
      case ZCL_CLEAR_EVENT_LOG_REQUEST_COMMAND_ID:
      {
        status = sl_zigbee_af_events_cluster_clear_event_log_request_cb(cmd);
        break;
      }
    }
  }

  return status;
}
