/***************************************************************************//**
 * @file
 * @brief This is the source for the plugin used to add an IAS Zone cluster server
 * to a project.  This source handles zone enrollment and storing of
 * attributes from a CIE device, and provides an API for different plugins to
 * post updated zone status values.
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

// *****************************************************************************
// * ias-zone-server.c
// *
// *
// *
// * Copyright 2015 Silicon Laboratories, Inc.                              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "ias-zone-server.h"
#include "hal/hal.h"
#include "sl_zigbee_token.h"
#include "ias-zone-server-tokens.h"

#ifdef SL_ZIGBEE_SCRIPTED_TEST
 #ifndef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
 #define SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
 #endif
 #include "app/framework/plugin/ias-zone-server/ias-zone-server-test.h"
 #include "app/zaptest/unit-test/zap-id.h"
// needed to get defines from zap-command-structs.h
 #include "zap-command-structs.h"
 #include "zap-cluster-command-parser.h"
#else // SL_ZIGBEE_SCRIPTED_TEST
 #include "ias-zone-server-config.h"
 #include "zap-cluster-command-parser.h"
#endif // SL_ZIGBEE_SCRIPTED_TEST

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_WWAH_APP_EVENT_RETRY_MANAGER_PRESENT
#include "wwah-app-event-retry-manager-config.h"
#endif

#if (SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE == 1)
#define ENABLE_QUEUE
#endif
#if (SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_UNLIMITED_RETRIES == 1)
#define UNLIMITED_RETRIES
#endif

#define UNDEFINED_ZONE_ID 0xFF
#define DELAY_TIMER_MS (1 * MILLISECOND_TICKS_PER_SECOND)
#define IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX  0x02
#define ZCL_FRAME_CONTROL_IDX 0x00

#if defined(ENABLE_QUEUE)
  #if defined(SL_CATALOG_ZIGBEE_WWAH_APP_EVENT_RETRY_MANAGER_PRESENT)
    #define NUM_QUEUE_ENTRIES SL_ZIGBEE_AF_PLUGIN_WWAH_APP_EVENT_RETRY_MANAGER_QUEUE_SIZE
  #else // !WWAH_APP_EVENT_RETRY_MANAGER_PRESENT
    #define NUM_QUEUE_ENTRIES SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_QUEUE_DEPTH
  #endif // WWAH_APP_EVENT_RETRY_MANAGER_PRESENT
#else // !ENABLE_QUEUE
  #define NUM_QUEUE_ENTRIES 0
#endif // ENABLE_QUEUE

#define DEFAULT_ENROLLMENT_METHOD  SL_ZIGBEE_ZCL_IAS_ZONE_ENROLLMENT_MODE_REQUEST

typedef struct {
  uint8_t endpoint;
  uint16_t status;
  uint32_t eventTimeMs;
} sli_zigbee_ias_zone_status_queue_entry_t;

typedef struct {
  uint8_t entriesInQueue;
  uint8_t startIdx;
  uint8_t lastIdx;
  sli_zigbee_ias_zone_status_queue_entry_t buffer[NUM_QUEUE_ENTRIES];
} sli_zigbee_ias_zone_status_queue_t;

//-----------------------------------------------------------------------------
// Globals
sl_zigbee_af_event_t sl_zigbee_af_ias_zone_server_manage_queue_event;
#define serverManageQueueEventControl (&sl_zigbee_af_ias_zone_server_manage_queue_event)
void sl_zigbee_af_ias_zone_server_manage_queue_event_handler(sl_zigbee_af_event_t * event);

static sl_zigbee_af_ias_zone_enrollment_mode_t enrollmentMethod;

#if defined(ENABLE_QUEUE)
sli_zigbee_ias_zone_status_queue_t messageQueue;

// Status queue retry parameters
typedef struct {
  sli_zigbee_ias_zone_status_queue_retry_config_t config;
  uint32_t currentBackoffTimeSec;
  uint8_t currentRetryCount;
} sli_zigbee_ias_zone_status_queue_retry_parameters_t;

// Set up status queue retry parameters.
sli_zigbee_ias_zone_status_queue_retry_parameters_t queueRetryParams = {
  .config = {
    .firstBackoffTimeSec = SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC,
    .backoffSeqCommonRatio = SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_BACKOFF_SEQUENCE_COMMON_RATIO,
    .maxBackoffTimeSec = SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_MAX_BACKOFF_TIME_SEC,
#ifdef UNLIMITED_RETRIES
    .unlimitedRetries = true,
#else // !UNLIMITED_RETRIES
    .unlimitedRetries = false,
#endif // UNLIMITED_RETRIES
    .maxRetryAttempts = SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_MAX_RETRY_ATTEMPTS
  },
  .currentBackoffTimeSec = SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC,
  .currentRetryCount = 0,
};

static void resetCurrentQueueRetryParams(void)
{
  queueRetryParams.currentRetryCount = 0;
  queueRetryParams.currentBackoffTimeSec = queueRetryParams.config.firstBackoffTimeSec;
}

#endif // ENABLE_QUEUE

//-----------------------------------------------------------------------------
// Forward declarations

static void setZoneId(uint8_t endpoint, uint8_t zoneId);
static bool areZoneServerAttributesTokenized(uint8_t endpoint);
static bool isValidEnrollmentMode(sl_zigbee_af_ias_zone_enrollment_mode_t method);
#if defined(ENABLE_QUEUE)
static uint16_t computeElapsedTimeQs(sli_zigbee_ias_zone_status_queue_entry_t *entry);
static void bufferInit(sli_zigbee_ias_zone_status_queue_t *ring);
static int16_t copyToBuffer(sli_zigbee_ias_zone_status_queue_t *ring,
                            const sli_zigbee_ias_zone_status_queue_entry_t *entry);
static int16_t popFromBuffer(sli_zigbee_ias_zone_status_queue_t *ring,
                             sli_zigbee_ias_zone_status_queue_entry_t *entry);
#endif // ENABLE_QUEUE

//-----------------------------------------------------------------------------
// Functions

static sl_status_t sendToClient(uint8_t endpoint)
{
  sl_status_t status;

  // If the device is not a network, there is no one to send to, so do nothing
  if (sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK) {
    return SL_STATUS_NETWORK_DOWN;
  }

  // Remote endpoint need not be set, since it will be provided by the call to
  // sl_zigbee_af_send_command_unicast_to_bindings()
  sl_zigbee_af_set_command_endpoints(endpoint, 0);

  // A binding table entry is created on Zone Enrollment for each endpoint, so
  // a simple call to SendCommandUnicastToBinding will handle determining the
  // destination endpoint, address, etc for us.
  status = sl_zigbee_af_send_command_unicast_to_bindings();

  if (SL_STATUS_OK != status) {
    return status;
  }

  return status;
}

static void enrollWithClient(uint8_t endpoint)
{
  sl_status_t status;
  sl_zigbee_af_fill_command_ias_zone_cluster_zone_enroll_request(
    SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE,
    SL_ZIGBEE_ZCL_MANUFACTURER_CODE);
  status = sendToClient(endpoint);
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_ias_zone_cluster_println("Sent enroll request to IAS Zone client.");
  } else {
    sl_zigbee_af_ias_zone_cluster_println("Error sending enroll request: 0x%02X\n",
                                          status);
  }
}

sl_zigbee_af_status_t sl_zigbee_af_ias_zone_cluster_server_pre_attribute_changed_cb(
  uint8_t endpoint,
  sl_zigbee_af_attribute_id_t attributeId,
  sl_zigbee_af_attribute_type_t attributeType,
  uint8_t size,
  uint8_t *value)
{
  UNUSED_VAR(attributeType);
  UNUSED_VAR(size);

  uint8_t i;
  bool zeroAddress;
  sl_zigbee_binding_table_entry_t bindingEntry;
  sl_zigbee_binding_table_entry_t currentBind;
  sl_802154_long_addr_t destEUI;
  uint8_t ieeeAddress[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  // If this is not a CIE Address write, the CIE address has already been
  // written, or the IAS Zone server is already enrolled, do nothing.
  if (attributeId != ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID
      || sl_zigbee_af_current_command() == NULL) {
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }

  memcpy(destEUI, value, EUI64_SIZE);

  // Create the binding table entry

  // This code assumes that the endpoint and device that is setting the CIE
  // address is the CIE device itself, and as such the remote endpoint to bind
  // to is the endpoint that generated the attribute change.  This
  // assumption is made based on analysis of the behavior of CIE devices
  // currently existing in the field.
  bindingEntry.type = SL_ZIGBEE_UNICAST_BINDING;
  bindingEntry.local = endpoint;
  bindingEntry.clusterId = ZCL_IAS_ZONE_CLUSTER_ID;
  bindingEntry.remote = sl_zigbee_af_current_command()->apsFrame->sourceEndpoint;
  memcpy(bindingEntry.identifier, destEUI, EUI64_SIZE);

  // Cycle through the binding table until we find a valid entry that is not
  // being used, then use the created entry to make the bind.
  for (i = 0; i < SL_ZIGBEE_BINDING_TABLE_SIZE; i++) {
    if (sl_zigbee_get_binding(i, &currentBind) != SL_STATUS_OK) {
      //break out of the loop to ensure that an error message still prints
      break;
    }
    if (currentBind.type != SL_ZIGBEE_UNUSED_BINDING) {
      // If the binding table entry created based on the response already exists
      // do nothing.
      if ((currentBind.local == bindingEntry.local)
          && (currentBind.clusterId == bindingEntry.clusterId)
          && (currentBind.remote == bindingEntry.remote)
          && (currentBind.type == bindingEntry.type)) {
        break;
      }
      // If this spot in the binding table already exists, move on to the next
      continue;
    } else {
      sl_zigbee_set_binding(i, &bindingEntry);
      break;
    }
  }

  zeroAddress = true;
  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_IAS_ZONE_CLUSTER_ID,
                                     ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                                     (uint8_t*)ieeeAddress,
                                     8);
  for (i = 0; i < 8; i++) {
    if (ieeeAddress[i] != 0) {
      zeroAddress = false;
    }
  }
  sl_zigbee_af_app_print("\nzero address: %d\n", zeroAddress);

  if ((zeroAddress == true) && (enrollmentMethod == SL_ZIGBEE_ZCL_IAS_ZONE_ENROLLMENT_MODE_REQUEST)) {
    // Only send the enrollment request if the mode is AUTO-ENROLL-REQUEST.
    // We need to delay to get around a bug where we can't send a command
    // at this point because then the Write Attributes response will not
    // be sent.  But we also delay to give the client time to configure us.
    sl_zigbee_af_ias_zone_cluster_println("Sending enrollment after %d ms",
                                          DELAY_TIMER_MS);
    sl_zigbee_zcl_schedule_server_tick_extended(endpoint,
                                                ZCL_IAS_ZONE_CLUSTER_ID,
                                                DELAY_TIMER_MS,
                                                SL_ZIGBEE_AF_SHORT_POLL,
                                                SL_ZIGBEE_AF_STAY_AWAKE);
  }

  return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
}

sl_zigbee_af_status_t sl_zigbee_af_ias_zone_cluster_set_enrollment_method(uint8_t endpoint,
                                                                          sl_zigbee_af_ias_zone_enrollment_mode_t method)
{
  sl_zigbee_af_status_t status;

  if (sl_zigbee_af_ias_zone_cluster_am_i_enrolled(endpoint)) {
    sl_zigbee_af_ias_zone_cluster_println("Error: Already enrolled");
    status = SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
  } else if (!isValidEnrollmentMode(method)) {
    sl_zigbee_af_ias_zone_cluster_println("Invalid IAS Zone Server Enrollment Mode: %d", method);
    status = SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  } else {
    enrollmentMethod = method;
#ifndef EZSP_HOST
    sl_status_t sl_status = sl_token_manager_set_data(COMMON_TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD,
                                                      (void *)&enrollmentMethod,
                                                      sizeof(uint8_t));
    SL_ZIGBEE_TEST_ASSERT(sl_status == SL_STATUS_OK);
    UNUSED_VAR(status);
#endif
    sl_zigbee_af_ias_zone_cluster_println("IAS Zone Server Enrollment Mode: %d", method);
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
  return status;
}

static bool isValidEnrollmentMode(sl_zigbee_af_ias_zone_enrollment_mode_t method)
{
  return ((method == SL_ZIGBEE_ZCL_IAS_ZONE_ENROLLMENT_MODE_TRIP_TO_PAIR)
          || (method == SL_ZIGBEE_ZCL_IAS_ZONE_ENROLLMENT_MODE_AUTO_ENROLLMENT_RESPONSE)
          || (method == SL_ZIGBEE_ZCL_IAS_ZONE_ENROLLMENT_MODE_REQUEST));
}

bool sl_zigbee_af_ias_zone_cluster_am_i_enrolled(uint8_t endpoint)
{
  sl_zigbee_af_ias_zone_state_t zoneState = 0;  // Clear this out completely.
  sl_zigbee_af_status_t status;
  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_IAS_ZONE_CLUSTER_ID,
                                              ZCL_ZONE_STATE_ATTRIBUTE_ID,
                                              (unsigned char*)&zoneState,
                                              1); // uint8_t size

  return (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS
          && zoneState == SL_ZIGBEE_ZCL_IAS_ZONE_STATE_ENROLLED);
}

static void updateEnrollState(uint8_t endpoint, bool enrolled)
{
  sl_zigbee_af_ias_zone_state_t zoneState = (enrolled
                                             ? SL_ZIGBEE_ZCL_IAS_ZONE_STATE_ENROLLED
                                             : SL_ZIGBEE_ZCL_IAS_ZONE_STATE_NOT_ENROLLED);

  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_IAS_ZONE_CLUSTER_ID,
                                      ZCL_ZONE_STATE_ATTRIBUTE_ID,
                                      (uint8_t*)&zoneState,
                                      ZCL_INT8U_ATTRIBUTE_TYPE);
  sl_zigbee_af_ias_zone_cluster_println("IAS Zone Server State: %sEnrolled",
                                        (enrolled
                                         ? ""
                                         : "NOT "));
}

//------------------------
// Commands callbacks

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_ias_zone_cluster_zone_enroll_response_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_ias_zone_cluster_zone_enroll_response_command_t cmd_data;
  uint8_t endpoint;
  uint8_t epZoneId;
  sl_zigbee_af_status_t status;

  if (zcl_decode_ias_zone_cluster_zone_enroll_response_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  endpoint = sl_zigbee_af_current_endpoint();
  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_IAS_ZONE_CLUSTER_ID,
                                              ZCL_ZONE_ID_ATTRIBUTE_ID,
                                              &epZoneId,
                                              sizeof(uint8_t));
  if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    if (cmd_data.enrollResponseCode == SL_ZIGBEE_ZCL_IAS_ENROLL_RESPONSE_CODE_SUCCESS) {
      updateEnrollState(endpoint, true);
      setZoneId(endpoint, cmd_data.zoneId);
    } else {
      updateEnrollState(endpoint, false);
      setZoneId(endpoint, UNDEFINED_ZONE_ID);
    }

    return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
  }

  sl_zigbee_af_app_println("ERROR: IAS Zone Server unable to read zone ID attribute");
  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

static sl_status_t sendZoneUpdate(uint16_t zoneStatus,
                                  uint16_t timeSinceStatusOccurredQs,
                                  uint8_t endpoint)
{
  sl_status_t status;

  if (!sl_zigbee_af_ias_zone_cluster_am_i_enrolled(endpoint)) {
    return SL_STATUS_INVALID_STATE;
  }
  sl_zigbee_af_fill_command_ias_zone_cluster_zone_status_change_notification(
    zoneStatus,
    0, // extended status, must be zero per spec
    sl_zigbee_af_ias_zone_server_get_zone_id(endpoint),
    timeSinceStatusOccurredQs); // called "delay" in the spec
  status = sendToClient(endpoint);

  return status;
}

#if defined(ENABLE_QUEUE)
static void addNewEntryToQueue(const sli_zigbee_ias_zone_status_queue_entry_t *newEntry)
{
  sl_zigbee_af_ias_zone_cluster_println("Adding new entry to queue");
  copyToBuffer(&messageQueue, newEntry);
}
#endif // ENABLE_QUEUE

sl_status_t sl_zigbee_af_ias_zone_server_update_zone_status(
  uint8_t endpoint,
  uint16_t newStatus,
  uint16_t timeSinceStatusOccurredQs)
{
#if defined(ENABLE_QUEUE)
  sli_zigbee_ias_zone_status_queue_entry_t newBufferEntry;
  newBufferEntry.endpoint = endpoint;
  newBufferEntry.status = newStatus;
  newBufferEntry.eventTimeMs = halCommonGetInt32uMillisecondTick();
#endif // ENABLE_QUEUE
  sl_status_t sendStatus;

  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_IAS_ZONE_CLUSTER_ID,
                                      ZCL_ZONE_STATUS_ATTRIBUTE_ID,
                                      (uint8_t*)&newStatus,
                                      ZCL_INT16U_ATTRIBUTE_TYPE);

  if (enrollmentMethod == SL_ZIGBEE_ZCL_IAS_ZONE_ENROLLMENT_MODE_TRIP_TO_PAIR) {
    // If unenrolled, send Zone Enroll Request command.
    if (!sl_zigbee_af_ias_zone_cluster_am_i_enrolled(endpoint)) {
      sl_zigbee_zcl_schedule_server_tick(endpoint,
                                         ZCL_IAS_ZONE_CLUSTER_ID,
                                         DELAY_TIMER_MS);
      // Don't send the zone status update since not enrolled.
      return SL_STATUS_OK;
    }
  }

#if defined(ENABLE_QUEUE)
  // If there are items in the queue waiting to send, this event should not
  // be transmitted, as that could cause the client to receive the events out
  // of order.  Instead, just add the device to the queue
  if (messageQueue.entriesInQueue == 0) {
    sendStatus = sendZoneUpdate(newStatus,
                                timeSinceStatusOccurredQs,
                                endpoint);
  } else {
    // Add a new element to the status queue and depending on the network state
    // either try to resend the first element in the queue immediately or try to
    // restart the parent research pattern.
    addNewEntryToQueue(&newBufferEntry);

    sl_zigbee_network_status_t networkState = sl_zigbee_af_network_state();

    if (networkState == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
      sl_zigbee_af_start_move_cb();
    } else if (networkState == SL_ZIGBEE_JOINED_NETWORK) {
      resetCurrentQueueRetryParams();
      sl_zigbee_af_event_set_active(serverManageQueueEventControl);
    }

    return SL_STATUS_OK;
  }

#else // !ENABLE_QUEUE
  sendStatus = sendZoneUpdate(newStatus, timeSinceStatusOccurredQs, endpoint);
#endif // ENABLE_QUEUE

  if (sendStatus == SL_STATUS_OK) {
#if defined(ENABLE_QUEUE)
    // Add a new entry to the zoneUpdate buffer
    addNewEntryToQueue(&newBufferEntry);
#endif // ENABLE_QUEUE
  } else {
    // If we're not on a network and never were, we don't need to do anything.
    // If we used to be on a network and can't talk to our parent, we should
    // try to rejoin the network and add the message to the queue
    if (sl_zigbee_af_network_state() == SL_ZIGBEE_JOINED_NETWORK_NO_PARENT) {
      sl_zigbee_af_start_move_cb();
#if defined(ENABLE_QUEUE)
      // Add a new entry to the zoneUpdate buffer
      addNewEntryToQueue(&newBufferEntry);
#endif // ENABLE_QUEUE
    }
    sl_zigbee_af_ias_zone_cluster_println("Failed to send IAS Zone update. Err 0x%02X",
                                          sendStatus);
  }
  return sendStatus;
}

void sl_zigbee_af_ias_zone_server_manage_queue_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);

#if defined(ENABLE_QUEUE)
  sli_zigbee_ias_zone_status_queue_entry_t *bufferStart;
  uint16_t status;
  uint16_t elapsedTimeQs;
  uint16_t airTimeRemainingMs;

  //If the queue was emptied without our interaction, do nothing
  if (messageQueue.entriesInQueue == 0) {
    sl_zigbee_af_event_set_inactive(serverManageQueueEventControl);
    return;
  }

  // Otherwise, pull out the first item and attempt to retransmit it.  The
  // message complete callback will handle removing items from the queue

  // To prevent an activity storm from flooding with retry requests, only
  // re-send a message if it's been at least
  // SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_MIN_OTA_TIME_MS since it was sent.
  bufferStart = &(messageQueue.buffer[messageQueue.startIdx]);
  elapsedTimeQs = computeElapsedTimeQs(bufferStart);

  if (elapsedTimeQs < (SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_MIN_OTA_TIME_MS
                       / (MILLISECOND_TICKS_PER_SECOND / 4))) {
    airTimeRemainingMs = SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_MIN_OTA_TIME_MS
                         - (elapsedTimeQs * MILLISECOND_TICKS_PER_SECOND / 4);
    sl_zigbee_af_ias_zone_cluster_println(
      "Not enough time passed for a retry, sleeping %d more mS",
      airTimeRemainingMs);
    sl_zigbee_af_event_set_delay_ms(serverManageQueueEventControl,
                                    airTimeRemainingMs);
  } else {
    status = bufferStart->status;
    sl_zigbee_af_ias_zone_cluster_println(
      "Attempting to resend a queued zone status update (status: 0x%02X, "
      "event time (s): %d) with time of %d. Retry count: %d",
      bufferStart->status,
      bufferStart->eventTimeMs / MILLISECOND_TICKS_PER_SECOND,
      elapsedTimeQs,
      queueRetryParams.currentRetryCount);
    sendZoneUpdate(status, elapsedTimeQs, bufferStart->endpoint);
    sl_zigbee_af_event_set_inactive(serverManageQueueEventControl);
  }
#else // !ENABLE_QUEUE
  sl_zigbee_af_event_set_inactive(serverManageQueueEventControl);
#endif // ENABLE_QUEUE
}

sl_status_t sl_zigbee_af_ias_zone_server_token_init(void)
{
  uint8_t tokPluginIASZoneServerEnrollmentMethodDefault = TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD_DEFAULT;
  return sl_zigbee_initialize_basic_token(COMMON_TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD, &tokPluginIASZoneServerEnrollmentMethodDefault, sizeof(uint8_t));
}

void sl_zigbee_af_ias_zone_cluster_server_init_cb(uint8_t endpoint)
{
  sl_zigbee_af_event_init(serverManageQueueEventControl,
                          sl_zigbee_af_ias_zone_server_manage_queue_event_handler);
  sl_zigbee_af_ias_zone_type_t zoneType;
  if (!areZoneServerAttributesTokenized(endpoint)) {
    sl_zigbee_af_app_print("WARNING: ATTRIBUTES ARE NOT BEING STORED IN FLASH! ");
    sl_zigbee_af_app_println("DEVICE WILL NOT FUNCTION PROPERLY AFTER REBOOTING!!");
  }
  assert(SL_STATUS_OK == sl_zigbee_af_ias_zone_server_token_init());

#ifndef EZSP_HOST
  sl_status_t status = sl_token_manager_get_data(COMMON_TOKEN_PLUGIN_IAS_ZONE_SERVER_ENROLLMENT_METHOD,
                                                 (void *)&enrollmentMethod,
                                                 sizeof(uint8_t));
  SL_ZIGBEE_TEST_ASSERT(status == SL_STATUS_OK);
  UNUSED_VAR(status);
#else
  enrollmentMethod = DEFAULT_ENROLLMENT_METHOD;
#endif
  if (!isValidEnrollmentMode(enrollmentMethod)) {
    // Default Enrollment Method to AUTO-ENROLL-REQUEST.
    enrollmentMethod = DEFAULT_ENROLLMENT_METHOD;
  }

#if defined(ENABLE_QUEUE)
  bufferInit(&messageQueue);
#endif // ENABLE_QUEUE

  zoneType = (sl_zigbee_af_ias_zone_type_t)SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE;
  (void) sl_zigbee_af_write_attribute(endpoint,
                                      ZCL_IAS_ZONE_CLUSTER_ID,
                                      ZCL_ZONE_TYPE_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      (uint8_t*)&zoneType,
                                      ZCL_INT16U_ATTRIBUTE_TYPE);

  sl_zigbee_af_ias_zone_server_update_zone_status(endpoint,
                                                  0, // status: All alarms cleared
                                                  0); // time since status occurred
}

void sl_zigbee_af_ias_zone_cluster_server_tick_cb(uint8_t endpoint)
{
  enrollWithClient(endpoint);
}

uint8_t sl_zigbee_af_ias_zone_server_get_zone_id(uint8_t endpoint)
{
  uint8_t zoneId = UNDEFINED_ZONE_ID;
  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_IAS_ZONE_CLUSTER_ID,
                                     ZCL_ZONE_ID_ATTRIBUTE_ID,
                                     &zoneId,
                                     sl_zigbee_af_get_data_size(ZCL_INT8U_ATTRIBUTE_TYPE));
  return zoneId;
}

//------------------------------------------------------------------------------
//
// This function will verify that all attributes necessary for the IAS zone
// server to properly retain functionality through a power failure are
// tokenized.
//
//------------------------------------------------------------------------------
static bool areZoneServerAttributesTokenized(uint8_t endpoint)
{
  sl_zigbee_af_attribute_metadata_t *metadata;

  metadata = sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                    ZCL_IAS_ZONE_CLUSTER_ID,
                                                    ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                                                    CLUSTER_MASK_SERVER,
                                                    SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
  if (!sl_zigbee_af_attribute_is_tokenized(metadata)) {
    return false;
  }

  metadata = sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                    ZCL_IAS_ZONE_CLUSTER_ID,
                                                    ZCL_ZONE_STATE_ATTRIBUTE_ID,
                                                    CLUSTER_MASK_SERVER,
                                                    SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
  if (!sl_zigbee_af_attribute_is_tokenized(metadata)) {
    return false;
  }

  metadata = sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                    ZCL_IAS_ZONE_CLUSTER_ID,
                                                    ZCL_ZONE_TYPE_ATTRIBUTE_ID,
                                                    CLUSTER_MASK_SERVER,
                                                    SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
  if (!sl_zigbee_af_attribute_is_tokenized(metadata)) {
    return false;
  }

  metadata = sl_zigbee_af_locate_attribute_metadata(endpoint,
                                                    ZCL_IAS_ZONE_CLUSTER_ID,
                                                    ZCL_ZONE_ID_ATTRIBUTE_ID,
                                                    CLUSTER_MASK_SERVER,
                                                    SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE);
  if (!sl_zigbee_af_attribute_is_tokenized(metadata)) {
    return false;
  }

  return true;
}

static void setZoneId(uint8_t endpoint, uint8_t zoneId)
{
  sl_zigbee_af_ias_zone_cluster_println("IAS Zone Server Zone ID: 0x%02X", zoneId);
  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_IAS_ZONE_CLUSTER_ID,
                                      ZCL_ZONE_ID_ATTRIBUTE_ID,
                                      &zoneId,
                                      ZCL_INT8U_ATTRIBUTE_TYPE);
}

static void unenrollSecurityDevice(uint8_t endpoint)
{
  uint8_t ieeeAddress[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  uint16_t zoneType = SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE;

  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_IAS_ZONE_CLUSTER_ID,
                                      ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                                      (uint8_t*)ieeeAddress,
                                      ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE);

  sl_zigbee_af_write_server_attribute(endpoint,
                                      ZCL_IAS_ZONE_CLUSTER_ID,
                                      ZCL_ZONE_TYPE_ATTRIBUTE_ID,
                                      (uint8_t*)&zoneType,
                                      ZCL_INT16U_ATTRIBUTE_TYPE);

  setZoneId(endpoint, UNDEFINED_ZONE_ID);
  // Restore the enrollment method back to its default value.
  sl_zigbee_af_ias_zone_cluster_set_enrollment_method(endpoint,
                                                      DEFAULT_ENROLLMENT_METHOD);
  updateEnrollState(endpoint, false); // enrolled?
}

// If you leave the network, unenroll yourself.
void sl_zigbee_af_ias_zone_server_stack_status_cb(sl_status_t status)
{
  uint8_t endpoint;
  uint8_t networkIndex;
  uint8_t i;

  // If the device has left the network, unenroll all endpoints on the device
  // that are servers of the IAS Zone Cluster
  if (status == SL_STATUS_NETWORK_DOWN
      && sl_zigbee_af_network_state() == SL_ZIGBEE_NO_NETWORK) {
    for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
      endpoint = sl_zigbee_af_endpoint_from_index(i);
      networkIndex = sl_zigbee_af_network_index_from_endpoint_index(i);
      if (networkIndex == sl_zigbee_get_current_network()
          && sl_zigbee_af_contains_server(endpoint, ZCL_IAS_ZONE_CLUSTER_ID)) {
        unenrollSecurityDevice(endpoint);
      }
    }
  } else if (status == SL_STATUS_NETWORK_UP) {
#if defined(ENABLE_QUEUE)
    // If we're reconnecting, send any items still in the queue
    sl_zigbee_af_ias_zone_cluster_println(
      "Rejoined network, retransmiting any queued event");
    sl_zigbee_af_event_set_active(serverManageQueueEventControl);
#endif // ENABLE_QUEUE
  }
}

#if defined(ENABLE_QUEUE)
sl_status_t sl_zigbee_af_ias_zone_server_config_status_queue_retry_params(
  sli_zigbee_ias_zone_status_queue_retry_config_t *retryConfig)
{
  if (!(retryConfig->firstBackoffTimeSec)
      || (!retryConfig->backoffSeqCommonRatio)
      || (retryConfig->maxBackoffTimeSec < retryConfig->firstBackoffTimeSec)
      || (retryConfig->maxBackoffTimeSec > IAS_ZONE_STATUS_QUEUE_RETRY_ABS_MAX_BACKOFF_TIME_SEC)
      || (!retryConfig->maxRetryAttempts) ) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  queueRetryParams.config.firstBackoffTimeSec = retryConfig->firstBackoffTimeSec;
  queueRetryParams.config.backoffSeqCommonRatio = retryConfig->backoffSeqCommonRatio;
  queueRetryParams.config.maxBackoffTimeSec = retryConfig->maxBackoffTimeSec;
  queueRetryParams.config.unlimitedRetries = retryConfig->unlimitedRetries;
  queueRetryParams.config.maxRetryAttempts = retryConfig->maxRetryAttempts;

  queueRetryParams.currentBackoffTimeSec = retryConfig->firstBackoffTimeSec;
  queueRetryParams.currentRetryCount = 0;

  return SL_STATUS_OK;
}

void sl_zigbee_af_ias_zone_server_set_status_queue_retry_params_to_default(void)
{
  queueRetryParams.config.firstBackoffTimeSec =
    SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC;
  queueRetryParams.config.backoffSeqCommonRatio =
    SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_BACKOFF_SEQUENCE_COMMON_RATIO;
  queueRetryParams.config.maxBackoffTimeSec =
    SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_MAX_BACKOFF_TIME_SEC;
#ifdef UNLIMITED_RETRIES
  queueRetryParams.config.unlimitedRetries = true;
#else // !UNLIMITED_RETRIES
  queueRetryParams.config.unlimitedRetries = false;
#endif // UNLIMITED_RETRIES
  queueRetryParams.config.maxRetryAttempts =
    SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_MAX_RETRY_ATTEMPTS;

  queueRetryParams.currentBackoffTimeSec =
    SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_FIRST_BACKOFF_TIME_SEC;
  queueRetryParams.currentRetryCount = 0;
}

void sl_zigbee_af_ias_zone_server_discard_pending_events_in_status_queue(void)
{
  sl_zigbee_af_event_set_inactive(serverManageQueueEventControl);
  bufferInit(&messageQueue);
  resetCurrentQueueRetryParams();
}

#if defined(SL_CATALOG_ZIGBEE_WWAH_APP_EVENT_RETRY_MANAGER_PRESENT)
sl_status_t sl_zigbee_af_wwah_app_event_retry_manager_config_backoff_params_cb(uint8_t firstBackoffTimeSeconds,
                                                                               uint8_t backoffSeqCommonRatio,
                                                                               uint32_t maxBackoffTimeSeconds,
                                                                               uint8_t maxRedeliveryAttempts)
{
  sli_zigbee_ias_zone_status_queue_retry_config_t retryConfig = {
    firstBackoffTimeSeconds,
    backoffSeqCommonRatio,
    maxBackoffTimeSeconds,
    (maxRedeliveryAttempts == 0xFF),
    maxRedeliveryAttempts
  };

  // Setting up retry parameters
  return sl_zigbee_af_ias_zone_server_config_status_queue_retry_params(&retryConfig);
}

void sl_zigbee_af_wwah_app_event_retry_manager_set_backoff_params_to_default(void)
{
  sl_zigbee_af_ias_zone_server_set_status_queue_retry_params_to_default();
}
#endif // WWAH_APP_EVENT_RETRY_MANAGER_PRESENT

void sl_zigbee_af_ias_zone_server_print_queue(void)
{
  sl_zigbee_af_ias_zone_cluster_println("%d/%d entries", messageQueue.entriesInQueue, NUM_QUEUE_ENTRIES);
  for (int i = 0; i < messageQueue.entriesInQueue; i++) {
    sl_zigbee_af_ias_zone_cluster_println("Entry %d: Endpoint: %d Status: %d EventTimeMs: %d",
                                          i,
                                          messageQueue.buffer[i].endpoint,
                                          messageQueue.buffer[i].status,
                                          messageQueue.buffer[i].eventTimeMs
                                          );
  }
}

void sl_zigbee_af_ias_zone_server_print_queue_config(void)
{
  sl_zigbee_af_core_println("First backoff time (sec): %d", queueRetryParams.config.firstBackoffTimeSec);
  sl_zigbee_af_core_println("Backoff sequence common ratio: %d", queueRetryParams.config.backoffSeqCommonRatio);
  sl_zigbee_af_core_println("Max backoff time (sec): %d", queueRetryParams.config.maxBackoffTimeSec);
  sl_zigbee_af_core_println("Max redelivery attempts: %d", queueRetryParams.config.maxRetryAttempts);
}

#endif // ENABLE_QUEUE

// This callback will be generated any time the node receives an ACK or a NAK
// for a message transmitted for the IAS Zone Cluster Server.  Note that this
// will not be called in the case that the message was not delivered to the
// destination when the destination is the only router the node is joined to.
// In that case, the command will never have been sent, as the device will have
// had no router by which to send the command.
void sl_zigbee_af_ias_zone_cluster_server_message_sent_cb(
  sl_zigbee_outgoing_message_type_t type,
  int16u indexOrDestination,
  sl_zigbee_aps_frame_t *apsFrame,
  int16u msgLen,
  int8u *message,
  sl_status_t status)
{
  UNUSED_VAR(type);
  UNUSED_VAR(indexOrDestination);
  UNUSED_VAR(apsFrame);

#if defined(ENABLE_QUEUE)
  uint8_t frameControl;
  uint8_t commandId;

  sli_zigbee_ias_zone_status_queue_entry_t dummyEntry;

  // Verify that this response is for a ZoneStatusChangeNotification command
  // by checking the message length, the command and direction bits of the
  // Frame Control byte, and the command ID
  if (msgLen < IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX) {
    return;
  }

  frameControl = message[ZCL_FRAME_CONTROL_IDX];
  if (!(frameControl & ZCL_CLUSTER_SPECIFIC_COMMAND)
      || !(frameControl & ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)) {
    return;
  }

  commandId = message[IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX];
  if (commandId != ZCL_ZONE_STATUS_CHANGE_NOTIFICATION_COMMAND_ID) {
    return;
  }

  // If a change status change notification command is not received by the
  // client, delay the option specified amount of time and try to resend it.
  // The event handler will perform the retransmit per the preset queue retry
  // parameteres, and the original send request will handle populating the buffer.
  // Do not try to retransmit again if the maximum number of retries attempts
  // is reached, this is however discarded if configured for unlimited retries.
  if ((status != SL_STATUS_OK)
      && (queueRetryParams.config.unlimitedRetries
          || (queueRetryParams.currentRetryCount < queueRetryParams.config.maxRetryAttempts))) {
    queueRetryParams.currentRetryCount++;

    sl_zigbee_af_ias_zone_cluster_println(
      "Status command update failed to send... Retrying in %d seconds...",
      queueRetryParams.currentBackoffTimeSec);

    // Delay according to the current retransmit backoff time.
    sl_zigbee_af_event_set_delay_ms(serverManageQueueEventControl,
                                    queueRetryParams.currentBackoffTimeSec * MILLISECOND_TICKS_PER_SECOND);

    // The backoff time needs to be increased if the maximum backoff time is not reached yet.
    if ((queueRetryParams.currentBackoffTimeSec * queueRetryParams.config.backoffSeqCommonRatio)
        <= queueRetryParams.config.maxBackoffTimeSec) {
      queueRetryParams.currentBackoffTimeSec *= queueRetryParams.config.backoffSeqCommonRatio;
    }
  } else {
    // If a command message was sent or max redelivery attempts were reached,
    // remove it from the queue and move on to the next queued message until the queue is empty.
    if (status == SL_STATUS_OK) {
      sl_zigbee_af_ias_zone_cluster_println(
        "\nZone update successful, remove entry from queue");
    } else {
      sl_zigbee_af_ias_zone_cluster_println(
        "\nZone update unsuccessful, max retry attempts reached, remove entry from queue");
    }
    popFromBuffer(&messageQueue, &dummyEntry);

    // Reset queue retry parameters.
    resetCurrentQueueRetryParams();

    if (messageQueue.entriesInQueue) {
      sl_zigbee_af_event_set_active(serverManageQueueEventControl);
    }
  }
#else
  UNUSED_VAR(msgLen);
  UNUSED_VAR(message);
  UNUSED_VAR(status);
#endif // ENABLE_QUEUE
}

#if defined(ENABLE_QUEUE)
static void bufferInit(sli_zigbee_ias_zone_status_queue_t *ring)
{
  ring->entriesInQueue = 0;
  ring->startIdx = 0;
  ring->lastIdx = NUM_QUEUE_ENTRIES - 1;
}

// Add the entry to the buffer by copying, returning the index at which it was
// added.  If the buffer is full, return -1, but still copy the entry over the
// last item of the buffer, to ensure that the last item in the buffer is
// always representative of the last known device state.
static int16_t copyToBuffer(sli_zigbee_ias_zone_status_queue_t *ring,
                            const sli_zigbee_ias_zone_status_queue_entry_t *entry)
{
  if (ring->entriesInQueue == NUM_QUEUE_ENTRIES) {
    ring->buffer[ring->lastIdx] = *entry;
    return -1;
  }

  // Increment the last pointer.  If it rolls over the size, circle it back to
  // zero.
  ring->lastIdx++;
  if (ring->lastIdx >= NUM_QUEUE_ENTRIES) {
    ring->lastIdx = 0;
  }

  ring->buffer[ring->lastIdx].endpoint = entry->endpoint;
  ring->buffer[ring->lastIdx].status = entry->status;
  ring->buffer[ring->lastIdx].eventTimeMs = entry->eventTimeMs;

  ring->entriesInQueue++;
  return ring->lastIdx;
}

// Return the idx of the popped entry, or -1 if the buffer was empty.
static int16_t popFromBuffer(sli_zigbee_ias_zone_status_queue_t *ring,
                             sli_zigbee_ias_zone_status_queue_entry_t *entry)
{
  int16_t retVal;

  if (ring->entriesInQueue == 0) {
    return -1;
  }

  // Copy out the first entry, then increment the start pointer.  If it rolls
  // over, circle it back to zero.
  *entry = ring->buffer[ring->startIdx];
  retVal = ring->startIdx;

  ring->startIdx++;
  if (ring->startIdx >= NUM_QUEUE_ENTRIES) {
    ring->startIdx = 0;
  }

  ring->entriesInQueue--;

  return retVal;
}

static uint16_t computeElapsedTimeQs(sli_zigbee_ias_zone_status_queue_entry_t *entry)
{
  uint32_t currentTimeMs = halCommonGetInt32uMillisecondTick();
  int64_t deltaTimeMs = currentTimeMs - entry->eventTimeMs;

  if (deltaTimeMs < 0) {
    deltaTimeMs = -deltaTimeMs + (0xFFFFFFFF - currentTimeMs);
  }

  return deltaTimeMs / MILLISECOND_TICKS_PER_QUARTERSECOND;
}
#endif // ENABLE_QUEUE

uint32_t sl_zigbee_af_ias_zone_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                            sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_ZONE_ENROLL_RESPONSE_COMMAND_ID:
      {
        status = sl_zigbee_af_ias_zone_cluster_zone_enroll_response_cb(cmd);
        break;
      }
    }
  }

  return status;
}
