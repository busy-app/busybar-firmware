/***************************************************************************//**
 * @file
 * @brief Routines for the Poll Control Server plugin, which implement the
 *        server side of the Poll Control cluster. The Poll Control cluster
 *        provides a means to communicate with an end device with a sleep
 *        schedule.
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

// This plugin does not synchronize attributes between endpoints and does not
// handle multi-network issues with regard to polling.  Because of this, it is
// limited to exactly one endpoint that implements the Poll Control cluster
// server.
#if SL_ZIGBEE_ZCL_POLL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT != 1
  #error "The Poll Control Server plugin only supports one endpoint."
#endif

// The built-in cluster tick has hooks into the polling code and is therefore
// used to control both the temporary fast poll mode while waiting for
// CheckInResponse commands and the actual fast poll mode that follows one or
// more positive CheckInResponse commands.  The endpoint event is used to send
// the periodic CheckIn commands.
//
// When it is time to check in, a new fast poll period begins with the
// selection of clients.  The clients are determined by scanning the binding
// table for nodes bound to the local endpoint for the Poll Control server.
// A CheckIn command is sent to each client, up to the limit set in the plugin
// options.  After the CheckIn commands are sent, the plugin enters a temporary
// fast poll mode until either all clients send a CheckInResponse command or
// the check-in timeout expires.  If one or more clients requests fast polling,
// the plugin continues fast polling for the maximum requested duration.  If
// FastPollStop commands are received from any clients, the fast poll duration
// is adjusted so that it reflects the maximum duration requested by all active
// clients.  Once the requested duration for all clients is satisfied, fast
// polling ends.
//
// Note that if a required check in happens to coincide with an existing fast
// poll period, the current fast poll period is terminated, all existing
// clients are forgetten, and a new fast poll period begins with the selection
// of new clients and the sending of new CheckIn commands.

#include "zap-cluster-command-parser.h"
#include "poll-control-server-config.h"
#include "poll-control-server.h"
#if (SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_ACCEPT_SET_LONG_POLL_INTERVAL_COMMAND == 1)
#define ACCEPT_SET_LONG_POLL_INTERVAL_COMMAND
#endif
#if (SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_ACCEPT_SET_SHORT_POLL_INTERVAL_COMMAND == 1)
#define ACCEPT_SET_SHORT_POLL_INTERVAL_COMMAND
#endif
static sl_zigbee_af_event_t checkInEndpointEvents[ZCL_FIXED_ENDPOINT_COUNT];
typedef struct {
  uint8_t bindingIndex;
  uint16_t fastPollTimeoutQs;
} sli_zcl_poll_control_server_client_t;
static sli_zcl_poll_control_server_client_t clients[SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS];
static bool ignoreNonTrustCenter = false;

enum {
  INITIAL = 0,
  WAITING = 1,
  POLLING = 2,
};
static uint8_t state = INITIAL;
static uint32_t fastPollStartTimeMs;

// Flags used to track trust center check in failures
static bool trustCenterCheckInRequestSent;
static bool trustCenterCheckInResponseReceived;
static uint8_t trustCenterCheckInFailureCount;

// The timeout option is in quarter seconds, but we use it in milliseconds.
#define CHECK_IN_TIMEOUT_DURATION_MS                                 \
  (SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_CHECK_IN_RESPONSE_TIMEOUT \
   * MILLISECOND_TICKS_PER_QUARTERSECOND)
#define NULL_INDEX 0xFF

static sl_zigbee_af_status_t readServerAttribute(uint8_t endpoint,
                                                 sl_zigbee_af_attribute_id_t attributeId,
                                                 const char * name,
                                                 uint8_t *data,
                                                 uint8_t size)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_read_server_attribute(endpoint,
                                                                    ZCL_POLL_CONTROL_CLUSTER_ID,
                                                                    attributeId,
                                                                    data,
                                                                    size);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_poll_control_cluster_println("ERR: %sing %s 0x%02X", "read", name, status);
  }
  return status;
}

static sl_zigbee_af_status_t writeServerAttribute(uint8_t endpoint,
                                                  sl_zigbee_af_attribute_id_t attributeId,
                                                  const char * name,
                                                  uint8_t *data,
                                                  sl_zigbee_af_attribute_type_t type)
{
  sl_zigbee_af_status_t status = sl_zigbee_af_write_server_attribute(endpoint,
                                                                     ZCL_POLL_CONTROL_CLUSTER_ID,
                                                                     attributeId,
                                                                     data,
                                                                     type);
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_poll_control_cluster_println("ERR: %sing %s 0x%02X", "writ", name, status);
  }
  return status;
}

static sl_status_t scheduleServerTick(uint8_t endpoint, uint32_t delayMs)
{
  return sl_zigbee_zcl_schedule_server_tick_extended(endpoint,
                                                     ZCL_POLL_CONTROL_CLUSTER_ID,
                                                     delayMs,
                                                     SL_ZIGBEE_AF_SHORT_POLL,
                                                     SL_ZIGBEE_AF_OK_TO_SLEEP);
}

static sl_status_t deactivateServerTick(uint8_t endpoint)
{
  return sl_zigbee_zcl_deactivate_server_tick(endpoint, ZCL_POLL_CONTROL_CLUSTER_ID);
}

static void scheduleCheckIn(uint8_t endpoint)
{
  sl_zigbee_af_status_t status;
  uint32_t checkInIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (uint8_t *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS && checkInIntervalQs != 0) {
    sl_zigbee_af_endpoint_event_set_delay_ms(checkInEndpointEvents,
                                             endpoint,
                                             (checkInIntervalQs
                                              * MILLISECOND_TICKS_PER_QUARTERSECOND));
  } else {
    sl_zigbee_af_endpoint_event_set_inactive(checkInEndpointEvents,
                                             endpoint);
  }
}

static uint8_t findClientIndex(void)
{
  sl_zigbee_binding_table_entry_t incomingBinding;
  uint8_t incomingBindingIndex = sli_zigbee_af_get_binding_index();
  if (sl_zigbee_get_binding(incomingBindingIndex, &incomingBinding)
      == SL_STATUS_OK) {
    uint8_t clientIndex;
    for (clientIndex = 0;
         clientIndex < SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS;
         clientIndex++) {
      sl_zigbee_binding_table_entry_t clientBinding;
      if ((sl_zigbee_get_binding(clients[clientIndex].bindingIndex, &clientBinding)
           == SL_STATUS_OK)
          && incomingBinding.type == clientBinding.type
          && incomingBinding.local == clientBinding.local
          && incomingBinding.remote == clientBinding.remote
          && (memcmp(incomingBinding.identifier,
                     clientBinding.identifier,
                     EUI64_SIZE)
              == 0)) {
        return clientIndex;
      }
    }
  }
  return NULL_INDEX;
}

static bool pendingCheckInResponses(void)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    if (clients[i].bindingIndex != NULL_INDEX
        && clients[i].fastPollTimeoutQs == 0) {
      return true;
    }
  }
  return false;
}

static bool isPollControlBindingTrustCenter(uint8_t endpoint, uint8_t bindingIndex)
{
  sl_zigbee_binding_table_entry_t binding;
  if (sl_zigbee_get_binding(bindingIndex, &binding) == SL_STATUS_OK
      && binding.type == SL_ZIGBEE_UNICAST_BINDING
      && binding.local == endpoint
      && binding.clusterId == ZCL_POLL_CONTROL_CLUSTER_ID) {
    sl_802154_long_addr_t trustCenterEui64;
    if (sl_zigbee_lookup_eui64_by_node_id(SL_ZIGBEE_TRUST_CENTER_NODE_ID, trustCenterEui64) != SL_STATUS_OK) {
      return false;
    }

    if (0 == memcmp(binding.identifier, trustCenterEui64, EUI64_SIZE)) {
      return true;
    }
  }
  return false;
}

static bool outstandingFastPollRequests(uint8_t endpoint)
{
  uint32_t currentTimeMs = halCommonGetInt32uMillisecondTick();
  uint32_t elapsedFastPollTimeMs = elapsedTimeInt32u(fastPollStartTimeMs,
                                                     currentTimeMs);
  uint16_t fastPollTimeoutQs = 0;
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
    if (clients[i].bindingIndex != NULL_INDEX) {
      if (clients[i].fastPollTimeoutQs * MILLISECOND_TICKS_PER_QUARTERSECOND
          < elapsedFastPollTimeMs) {
        clients[i].bindingIndex = NULL_INDEX;
      } else {
        if (fastPollTimeoutQs < clients[i].fastPollTimeoutQs) {
          fastPollTimeoutQs = clients[i].fastPollTimeoutQs;
        }
      }
    }
  }

  if (fastPollTimeoutQs == 0) {
    return false;
  } else {
    uint32_t newFastPollEndTimeMs = (fastPollStartTimeMs
                                     + (fastPollTimeoutQs
                                        * MILLISECOND_TICKS_PER_QUARTERSECOND));
    uint32_t remainingFastPollTimeMs = elapsedTimeInt32u(currentTimeMs,
                                                         newFastPollEndTimeMs);
    scheduleServerTick(endpoint, remainingFastPollTimeMs);
    return true;
  }
}

static sl_zigbee_af_status_t validateCheckInInterval(uint8_t endpoint,
                                                     uint32_t newCheckInIntervalQs)
{
  sl_zigbee_af_status_t status;
  uint32_t longPollIntervalQs;

  if (newCheckInIntervalQs == 0) {
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (uint8_t *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
  if (newCheckInIntervalQs < longPollIntervalQs) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }

#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_CHECK_IN_INTERVAL_MIN_ATTRIBUTE
  {
    uint32_t checkInIntervalMinQs;
    status = readServerAttribute(endpoint,
                                 ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID,
                                 "check in interval min",
                                 (uint8_t *)&checkInIntervalMinQs,
                                 sizeof(checkInIntervalMinQs));
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      return status;
    }
    if (newCheckInIntervalQs < checkInIntervalMinQs) {
      return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    }
  }
#endif

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

static sl_zigbee_af_status_t validateLongPollInterval(uint8_t endpoint,
                                                      uint32_t newLongPollIntervalQs)
{
  sl_zigbee_af_status_t status;
  uint32_t checkInIntervalQs;
  uint16_t shortPollIntervalQs;

  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (uint8_t *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
  if (checkInIntervalQs < newLongPollIntervalQs) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }

  status = readServerAttribute(endpoint,
                               ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                               "short poll interval",
                               (uint8_t *)&shortPollIntervalQs,
                               sizeof(shortPollIntervalQs));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
  if (newLongPollIntervalQs < shortPollIntervalQs) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }

#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_LONG_POLL_INTERVAL_MIN_ATTRIBUTE
  {
    uint32_t longPollIntervalMinQs;
    status = readServerAttribute(endpoint,
                                 ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID,
                                 "long poll interval min",
                                 (uint8_t *)&longPollIntervalMinQs,
                                 sizeof(longPollIntervalMinQs));
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      return status;
    }
    if (newLongPollIntervalQs < longPollIntervalMinQs) {
      return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
    }
  }
#endif

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

static sl_zigbee_af_status_t validateShortPollInterval(uint8_t endpoint,
                                                       uint16_t newShortPollIntervalQs)
{
  sl_zigbee_af_status_t status;
  uint32_t longPollIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (uint8_t *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
  if (longPollIntervalQs < newShortPollIntervalQs) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

static sl_zigbee_af_status_t validateFastPollInterval(uint8_t endpoint,
                                                      uint16_t newFastPollIntervalQs)
{
#ifdef ZCL_USING_POLL_CONTROL_CLUSTER_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE
  sl_zigbee_af_status_t status;
  uint16_t fastPollTimeoutMaxQs;
  status = readServerAttribute(endpoint,
                               ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID,
                               "fast poll timeout max",
                               (uint8_t *)&fastPollTimeoutMaxQs,
                               sizeof(fastPollTimeoutMaxQs));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
  if (fastPollTimeoutMaxQs < newFastPollIntervalQs) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
#else
  UNUSED_VAR(endpoint);
  UNUSED_VAR(newFastPollIntervalQs);
#endif

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

static sl_zigbee_af_status_t validateCheckInIntervalMin(uint8_t endpoint,
                                                        uint32_t newCheckInIntervalMinQs)
{
  sl_zigbee_af_status_t status;
  uint32_t checkInIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID,
                               "check in interval",
                               (uint8_t *)&checkInIntervalQs,
                               sizeof(checkInIntervalQs));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
  if (checkInIntervalQs < newCheckInIntervalMinQs) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

static sl_zigbee_af_status_t validateLongPollIntervalMin(uint8_t endpoint,
                                                         uint32_t newLongPollIntervalMinQs)
{
  sl_zigbee_af_status_t status;
  uint32_t longPollIntervalQs;
  status = readServerAttribute(endpoint,
                               ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                               "long poll interval",
                               (uint8_t *)&longPollIntervalQs,
                               sizeof(longPollIntervalQs));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
  if (longPollIntervalQs < newLongPollIntervalMinQs) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

static sl_zigbee_af_status_t validateFastPollTimeoutMax(uint8_t endpoint,
                                                        uint16_t newFastPollTimeoutMaxQs)
{
  sl_zigbee_af_status_t status;
  uint16_t fastPollTimeoutQs;
  status = readServerAttribute(endpoint,
                               ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID,
                               "fast poll timeout",
                               (uint8_t *)&fastPollTimeoutQs,
                               sizeof(fastPollTimeoutQs));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return status;
  }
  if (newFastPollTimeoutMaxQs < fastPollTimeoutQs) {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

void sl_zigbee_af_poll_control_cluster_server_init_cb(uint8_t endpoint)
{
  uint32_t longPollIntervalQs;
  uint16_t shortPollIntervalQs;

  if (readServerAttribute(endpoint,
                          ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                          "long poll interval",
                          (uint8_t *)&longPollIntervalQs,
                          sizeof(longPollIntervalQs))
      == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_set_long_poll_interval_qs_cb(longPollIntervalQs);
  }

  if (readServerAttribute(endpoint,
                          ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                          "short poll interval",
                          (uint8_t *)&shortPollIntervalQs,
                          sizeof(shortPollIntervalQs))
      == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_set_short_poll_interval_qs_cb(shortPollIntervalQs);
  }

  // TODO: Begin checking in after the network comes up instead of at startup.
  scheduleCheckIn(endpoint);
}

void sl_zigbee_af_poll_control_cluster_server_tick_cb(uint8_t endpoint)
{
  if (state == WAITING) {
    uint16_t fastPollTimeoutQs = 0;
    uint8_t i;

    if (trustCenterCheckInRequestSent && !trustCenterCheckInResponseReceived) {
      trustCenterCheckInFailureCount++;
      sl_zigbee_af_poll_control_cluster_println("ERR: Poll control check in failure (%d of %d)",
                                                trustCenterCheckInFailureCount,
                                                SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_TRUST_CENTER_CHECK_IN_FAILURE_THRESHOLD);
      if (trustCenterCheckInFailureCount >= SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_TRUST_CENTER_CHECK_IN_FAILURE_THRESHOLD) {
        sl_zigbee_af_poll_control_cluster_println("ERR: Trust Center check in failure threshold reached");
        sl_zigbee_af_poll_control_server_check_in_timeout_cb();
        trustCenterCheckInFailureCount = 0;
      }
    }

    for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS; i++) {
      if (clients[i].bindingIndex != NULL_INDEX
          && fastPollTimeoutQs < clients[i].fastPollTimeoutQs) {
        fastPollTimeoutQs = clients[i].fastPollTimeoutQs;
      }
    }

    if (fastPollTimeoutQs != 0) {
      state = POLLING;
      fastPollStartTimeMs = halCommonGetInt32uMillisecondTick();
      scheduleServerTick(endpoint,
                         (fastPollTimeoutQs
                          * MILLISECOND_TICKS_PER_QUARTERSECOND));
      return;
    }
  }

  state = INITIAL;
  deactivateServerTick(endpoint);
}

static void checkInEndpointEventHandler(uint8_t endpoint)
{
  uint8_t bindingIndex, clientIndex;

  trustCenterCheckInRequestSent = false;
  trustCenterCheckInResponseReceived = false;
  for (clientIndex = 0;
       clientIndex < SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS;
       clientIndex++) {
    clients[clientIndex].bindingIndex = SL_ZIGBEE_NULL_BINDING;
  }

  for (bindingIndex = 0, clientIndex = 0;
       (bindingIndex < SL_ZIGBEE_BINDING_TABLE_SIZE
        && clientIndex < SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS);
       bindingIndex++) {
    sl_zigbee_binding_table_entry_t binding;
    if (sl_zigbee_get_binding(bindingIndex, &binding) == SL_STATUS_OK
        && binding.type == SL_ZIGBEE_UNICAST_BINDING
        && binding.local == endpoint
        && binding.clusterId == ZCL_POLL_CONTROL_CLUSTER_ID) {
      // If ignoreNonTrustCenter is true, then we only add
      // the trust center as a client.
      if (ignoreNonTrustCenter
          && (!isPollControlBindingTrustCenter(endpoint, bindingIndex))) {
        sl_zigbee_af_poll_control_cluster_println("Ignoring poll control client with bindingIndex %d in search of TC",
                                                  bindingIndex);
        continue;
      }

      sl_zigbee_af_fill_command_poll_control_cluster_check_in();
      if (sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_VIA_BINDING, bindingIndex)
          == SL_STATUS_OK) {
        clients[clientIndex].bindingIndex = bindingIndex;
        clients[clientIndex].fastPollTimeoutQs = 0;
        clientIndex++;
        if (isPollControlBindingTrustCenter(endpoint, bindingIndex)) {
          trustCenterCheckInRequestSent = true;
        }
      }
    }
  }

  if (clientIndex == 0) {
    state = INITIAL;
    deactivateServerTick(endpoint);
  } else {
    state = WAITING;
    scheduleServerTick(endpoint, CHECK_IN_TIMEOUT_DURATION_MS);
  }

  scheduleCheckIn(endpoint);
}

// Templated to event_init
void sli_zigbee_af_poll_control_server_init_callback(uint8_t init_level)
{
  (void)init_level;

  uint8_t endpoint_array[ZCL_FIXED_ENDPOINT_COUNT] = ZCL_FIXED_ENDPOINT_ARRAY;
  uint8_t i;

  for (i = 0; i < ZCL_FIXED_ENDPOINT_COUNT; i++) {
    sl_zigbee_af_endpoint_event_init(&checkInEndpointEvents[i],
                                     checkInEndpointEventHandler,
                                     endpoint_array[i]);
  }
}

sl_zigbee_af_status_t sl_zigbee_af_poll_control_cluster_server_pre_attribute_changed_cb(uint8_t endpoint,
                                                                                        sl_zigbee_af_attribute_id_t attributeId,
                                                                                        sl_zigbee_af_attribute_type_t attributeType,
                                                                                        uint8_t size,
                                                                                        uint8_t *value)
{
  UNUSED_VAR(attributeType);

  switch (attributeId) {
    case ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID:
    {
      uint32_t newCheckInIntervalQs;
      memmove(&newCheckInIntervalQs, value, size);
      return validateCheckInInterval(endpoint, newCheckInIntervalQs);
    }
    case ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      uint32_t newLongPollIntervalQs;
      memmove(&newLongPollIntervalQs, value, size);
      return validateLongPollInterval(endpoint, newLongPollIntervalQs);
    }
    case ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      uint16_t newShortPollIntervalQs;
      memmove(&newShortPollIntervalQs, value, size);
      return validateShortPollInterval(endpoint, newShortPollIntervalQs);
    }
    case ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID:
    {
      uint16_t newFastPollIntervalQs;
      memmove(&newFastPollIntervalQs, value, size);
      return validateFastPollInterval(endpoint, newFastPollIntervalQs);
    }
    case ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID:
    {
      uint32_t newCheckInIntervalMinQs;
      memmove(&newCheckInIntervalMinQs, value, size);
      return validateCheckInIntervalMin(endpoint, newCheckInIntervalMinQs);
    }
    case ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID:
    {
      uint32_t newLongPollIntervalMinQs;
      memmove(&newLongPollIntervalMinQs, value, size);
      return validateLongPollIntervalMin(endpoint, newLongPollIntervalMinQs);
    }
    case ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID:
    {
      uint32_t newFastPollTimeoutMaxQs;
      memmove(&newFastPollTimeoutMaxQs, value, size);
      return validateFastPollTimeoutMax(endpoint, newFastPollTimeoutMaxQs);
    }
    default:
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
}

void sl_zigbee_af_poll_control_cluster_server_attribute_changed_cb(uint8_t endpoint,
                                                                   sl_zigbee_af_attribute_id_t attributeId)
{
  switch (attributeId) {
    case ZCL_CHECK_IN_INTERVAL_ATTRIBUTE_ID:
      scheduleCheckIn(endpoint);
      break;
    case ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      sl_zigbee_af_status_t status;
      uint32_t longPollIntervalQs;
      status = readServerAttribute(endpoint,
                                   ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                                   "long poll interval",
                                   (uint8_t *)&longPollIntervalQs,
                                   sizeof(longPollIntervalQs));
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        sl_zigbee_af_set_long_poll_interval_qs_cb(longPollIntervalQs);
      }
      break;
    }
    case ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID:
    {
      sl_zigbee_af_status_t status;
      uint16_t shortPollIntervalQs;
      status = readServerAttribute(endpoint,
                                   ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                                   "short poll interval",
                                   (uint8_t *)&shortPollIntervalQs,
                                   sizeof(shortPollIntervalQs));
      if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        sl_zigbee_af_set_short_poll_interval_qs_cb(shortPollIntervalQs);
      }
      break;
    }
    case ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID:
    case ZCL_CHECK_IN_INTERVAL_MIN_ATTRIBUTE_ID:
    case ZCL_LONG_POLL_INTERVAL_MIN_ATTRIBUTE_ID:
    case ZCL_FAST_POLL_TIMEOUT_MAX_ATTRIBUTE_ID:
    default:
      break;
  }
}

//-----------------------
// ZCL commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_poll_control_cluster_fast_poll_stop_cb(void)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  uint8_t clientIndex = findClientIndex();

  sl_zigbee_af_poll_control_cluster_println("RX: FastPollStop");

  // clientIndex will always be less than SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS
  if (clientIndex != NULL_INDEX
      && clientIndex < SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS) {
    if (state == POLLING) {
      uint8_t endpoint = sl_zigbee_af_current_endpoint();
      status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      clients[clientIndex].bindingIndex = NULL_INDEX;

      // Calling the tick directly in the polling state will cause the fast
      // poll mode to stop.
      if (!outstandingFastPollRequests(endpoint)) {
        sl_zigbee_af_poll_control_cluster_server_tick_cb(endpoint);
      }
    }
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_poll_control_cluster_check_in_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_poll_control_cluster_check_in_response_command_t cmd_data;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
  uint8_t clientIndex = findClientIndex();

  if (zcl_decode_poll_control_cluster_check_in_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_poll_control_cluster_println("RX: CheckInResponse 0x%02X, 0x%04X",
                                            cmd_data.startFastPolling,
                                            cmd_data.fastPollTimeout);

  // clientIndex will always be less than SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS
  if (clientIndex != NULL_INDEX
      && clientIndex < SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_SERVER_MAX_CLIENTS) {
    if (state == WAITING) {
      uint8_t endpoint = sl_zigbee_af_current_endpoint();

      if (isPollControlBindingTrustCenter(endpoint, clients[clientIndex].bindingIndex)) {
        trustCenterCheckInResponseReceived = true;
        if (trustCenterCheckInFailureCount > 0) {
          sl_zigbee_af_poll_control_cluster_println("Poll Control: trust center "
                                                    "responding to checkins after %d"
                                                    "failure%s",
                                                    trustCenterCheckInFailureCount,
                                                    trustCenterCheckInFailureCount == 1
                                                    ? "" : "s");
        }
        trustCenterCheckInFailureCount = 0;
      }

      if (cmd_data.startFastPolling) {
        if (cmd_data.fastPollTimeout == 0) {
          status = readServerAttribute(endpoint,
                                       ZCL_FAST_POLL_TIMEOUT_ATTRIBUTE_ID,
                                       "fast poll timeout",
                                       (uint8_t *)&cmd_data.fastPollTimeout,
                                       sizeof(cmd_data.fastPollTimeout));
        } else {
          status = validateFastPollInterval(endpoint, cmd_data.fastPollTimeout);
        }
        if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
          clients[clientIndex].fastPollTimeoutQs = cmd_data.fastPollTimeout;
        } else {
          clients[clientIndex].bindingIndex = NULL_INDEX;
        }
      } else {
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
        clients[clientIndex].bindingIndex = NULL_INDEX;
      }

      // Calling the tick directly when in the waiting state will cause the
      // temporarily fast poll mode to stop and will begin the actual fast poll
      // mode if applicable.
      if (!pendingCheckInResponses()) {
        sl_zigbee_af_poll_control_cluster_server_tick_cb(endpoint);
      }
    }
  }

  return status;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_poll_control_cluster_set_long_poll_interval_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef ACCEPT_SET_LONG_POLL_INTERVAL_COMMAND
  sl_zcl_poll_control_cluster_set_long_poll_interval_command_t cmd_data;
  sl_zigbee_af_status_t status;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();

  if (zcl_decode_poll_control_cluster_set_long_poll_interval_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_poll_control_cluster_println("RX: SetLongPollInterval 0x%08X",
                                            cmd_data.newLongPollInterval);

  // Trying to write the attribute will trigger the PreAttributeChanged
  // callback, which will handle validation.  If the write is successful, the
  // AttributeChanged callback will fire, which will handle setting the new
  // long poll interval.
  status = writeServerAttribute(endpoint,
                                ZCL_LONG_POLL_INTERVAL_ATTRIBUTE_ID,
                                "long poll interval",
                                (uint8_t *)&cmd_data.newLongPollInterval,
                                ZCL_INT32U_ATTRIBUTE_TYPE);

  return status;
#else // !ACCEPT_SET_LONG_POLL_INTERVAL_COMMAND
  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
#endif // ACCEPT_SET_LONG_POLL_INTERVAL_COMMAND
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_poll_control_cluster_set_short_poll_interval_cb(sl_zigbee_af_cluster_command_t *cmd)
{
#ifdef ACCEPT_SET_SHORT_POLL_INTERVAL_COMMAND
  sl_zcl_poll_control_cluster_set_short_poll_interval_command_t cmd_data;
  sl_zigbee_af_status_t status;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();

  if (zcl_decode_poll_control_cluster_set_short_poll_interval_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  sl_zigbee_af_poll_control_cluster_println("RX: SetShortPollInterval 0x%04X",
                                            cmd_data.newShortPollInterval);

  // Trying to write the attribute will trigger the PreAttributeChanged
  // callback, which will handle validation.  If the write is successful, the
  // AttributeChanged callback will fire, which will handle setting the new
  // short poll interval.
  status = writeServerAttribute(endpoint,
                                ZCL_SHORT_POLL_INTERVAL_ATTRIBUTE_ID,
                                "short poll interval",
                                (uint8_t *)&cmd_data.newShortPollInterval,
                                ZCL_INT16U_ATTRIBUTE_TYPE);

  return status;
#else // !ACCEPT_SET_SHORT_POLL_INTERVAL_COMMAND
  return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
#endif // ACCEPT_SET_SHORT_POLL_INTERVAL_COMMAND
}

void sli_zigbee_af_poll_control_server_reset_attributes_callback(uint8_t endpointId)
{
  // EMAPPFWKV2-1437: when we reset our attributes, we need to re-sync with
  // the consumers of our attribute values. For example, the consumers of
  // sl_zigbee_af_set_long_poll_interval_qs_cb and
  // sl_zigbee_af_set_short_poll_interval_qs_cb will want to know that the poll
  // intervals might have changed. Therefore, we simply call the init function
  // to read the attribute values and notify interested parties.
  sl_zigbee_af_poll_control_cluster_server_init_cb(endpointId);
}

void sl_zigbee_af_poll_control_server_stack_status_cb(sl_status_t status)
{
  // Reset failure count if just joining network
  if (status == SL_STATUS_NETWORK_UP) {
    trustCenterCheckInFailureCount = 0;
  }
}

void sl_zigbee_af_poll_control_server_set_ignore_non_trust_center(bool ignoreNonTc)
{
  ignoreNonTrustCenter = ignoreNonTc;
}

bool sl_zigbee_af_poll_control_server_get_ignore_non_trust_center(void)
{
  return ignoreNonTrustCenter;
}

uint32_t sl_zigbee_af_poll_control_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_CHECK_IN_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_poll_control_cluster_check_in_response_cb(cmd);
        break;
      }
      case ZCL_FAST_POLL_STOP_COMMAND_ID:
      {
        status = sl_zigbee_af_poll_control_cluster_fast_poll_stop_cb();
        break;
      }
      case ZCL_SET_LONG_POLL_INTERVAL_COMMAND_ID:
      {
        status = sl_zigbee_af_poll_control_cluster_set_long_poll_interval_cb(cmd);
        break;
      }
      case ZCL_SET_SHORT_POLL_INTERVAL_COMMAND_ID:
      {
        status = sl_zigbee_af_poll_control_cluster_set_short_poll_interval_cb(cmd);
        break;
      }
    }
  }

  return status;
}
