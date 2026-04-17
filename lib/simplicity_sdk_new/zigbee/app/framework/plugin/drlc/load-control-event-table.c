/***************************************************************************//**
 * @file
 * @brief Host specific code related to the event table.
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

// clusters specific header
#include "app/framework/include/af.h"
#include "../../util/common.h"
#include "load-control-event-table.h"

#include "drlc-config.h"

#include "app/framework/plugin/esi-management/esi-management.h"

static void sli_zigbee_af_call_event_action(sl_zigbee_af_load_control_event_t *event,
                                            uint8_t eventStatus,
                                            uint8_t sequenceNumber,
                                            bool replyToSingleEsi,
                                            uint8_t esiIndex);

// This assumes that the Option Flag Enum uses only two bits
const uint8_t controlValueToStatusEnum[] = {
  SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_EVENT_COMPLETED_NO_USER_PARTICIPATION_PREVIOUS_OPT_OUT, // ! EVENT_OPT_FLAG_PARTIAL
  // && ! EVENT_OPT_FLAG_OPT_IN
  SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_EVENT_COMPLETED,                                        // ! EVENT_OPT_FLAG_PARTIAL
  // && EVENT_OPT_FLAG_OPT_IN
  SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_EVENT_PARTIALLY_COMPLETED_WITH_USER_OPT_OUT,   // EVENT_OPT_FLAG_PARTIAL
  // && ! EVENT_OPT_FLAG_OPT_IN
  SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_EVENT_PARTIALLY_COMPLETED_DUE_TO_USER_OPT_IN   // EVENT_OPT_FLAG_PARTIAL
  // && EVENT_OPT_FLAG_OPT_IN
};

typedef struct {
  sl_zigbee_af_load_control_event_t event;
  uint8_t entryStatus;
} sli_zigbee_load_control_event_table_entry_t;

static sli_zigbee_load_control_event_table_entry_t loadControlEventTable[SL_ZIGBEE_ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT][SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE];

static bool overlapFound(sl_zigbee_af_load_control_event_t *newEvent,
                         sl_zigbee_af_load_control_event_t *existingEvent)
{
  // CCB 1291: overlap exists if deviceClass or UEG overlaps (in addition to
  // event time check)
  bool deviceClassOverlaps = (newEvent->deviceClass
                              & existingEvent->deviceClass);
  bool uegOverlaps = ((0 == newEvent->utilityEnrollmentGroup)
                      || (0 == existingEvent->utilityEnrollmentGroup)
                      || (newEvent->utilityEnrollmentGroup
                          & existingEvent->utilityEnrollmentGroup));

  if (deviceClassOverlaps || uegOverlaps) {
    if (newEvent->startTime < (existingEvent->startTime + ((uint32_t)existingEvent->duration * 60))
        && existingEvent->startTime < (newEvent->startTime + ((uint32_t)newEvent->duration * 60))) {
      return true;
    }
  }

  return false;
}

static bool entryIsScheduledOrStarted(uint8_t entryStatus)
{
  if (entryStatus == ENTRY_SCHEDULED || entryStatus == ENTRY_STARTED) {
    return true;
  }

  return false;
}

static void initEventData(sl_zigbee_af_load_control_event_t *event)
{
  memset(event, 0, sizeof(sl_zigbee_af_load_control_event_t));
}

/**
 * Clears the table of any entries which pertain to a
 * specific eventId.
 **/
static void voidAllEntriesWithEventId(uint8_t endpoint,
                                      uint32_t eventId)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  sli_zigbee_load_control_event_table_entry_t *e;

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (e->event.eventId == eventId) {
      e->entryStatus = ENTRY_VOID;
    }
  }
}

/**
 * The callback function passed to the ESI management plugin. It handles
 * ESI entry deletions.
 */
static void esiDeletionCallback(uint8_t esiIndex)
{
  uint8_t i, j;
  for (i = 0; i < SL_ZIGBEE_ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT; i++) {
    for (j = 0; j < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; j++) {
      loadControlEventTable[i][j].event.esiBitmask &= ~BIT(esiIndex);
    }
  }
}

/**
 * The tick function simply checks entries in the table
 * and sends informational messages about event start
 * and event complete.
 */
void sli_zigbee_af_load_control_event_table_tick(uint8_t endpoint)
{
  uint32_t ct = sl_zigbee_af_get_current_time();
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  uint8_t i;
  sli_zigbee_load_control_event_table_entry_t *e;
  bool eventCurrentlyRunning = false;

  if (ep == 0xFF) {
    return;
  }

  // First, go through any running events and end the ones that are complete
  // An event that is extended by a random delay (start or duration) must keep
  // other events from starting; we can't just check a scheduled event's
  // start time and start delay against the current time because a previously
  // running event may be delayed
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++ ) {
    e = &loadControlEventTable[ep][i];
    if (e->entryStatus == ENTRY_STARTED) {
      if ((e->event.startTime
           + e->event.startRand   // CCB 1513: startRand does affect end time
           + ((uint32_t)e->event.duration * 60)
           + e->event.durationRand) <= ct) {
        sli_zigbee_af_call_event_action(&(e->event),
                                        controlValueToStatusEnum[e->event.optionControl],
                                        sl_zigbee_af_next_sequence(),
                                        false,
                                        0);
        voidAllEntriesWithEventId(endpoint,
                                  e->event.eventId);
        return;
      } else {
        eventCurrentlyRunning = true;
      }
    }
  }

  // Now check for scheduled, superseded, and canceled events
  for ( i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++ ) {
    e = &loadControlEventTable[ep][i];
    if (e->entryStatus == ENTRY_SCHEDULED) {
      if ((e->event.startTime + e->event.startRand <= ct)
          && (false == eventCurrentlyRunning)) { // 15-0132-03 test 10.38
        // Bug: 13546
        // When the event starts always send a Report Event status message.
        // If user opted-out, then send that status instead of event started.
        sli_zigbee_af_call_event_action(&(e->event),
                                        ((e->event.optionControl & EVENT_OPT_FLAG_OPT_IN)
                                         ? SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_EVENT_STARTED
                                         : SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_USER_HAS_CHOOSE_TO_OPT_OUT),
                                        sl_zigbee_af_next_sequence(),
                                        false,
                                        0);
        e->entryStatus = ENTRY_STARTED;
        // We may have waited for a previous event to finish due to a delay
        // In that case, we need to keep track of our current start time so that
        // we end on time and not sooner
        // We account for this by incrementing the startRand by the difference
        // in scheduled start versus actual start. Do not increment startTime
        // as the events' order are determined by that value
        e->event.startRand = ct - e->event.startTime;
        return;
      }
    } else if (e->entryStatus == ENTRY_IS_SUPERSEDED_EVENT) {
      if (e->event.startTime <= ct) {
        sli_zigbee_af_call_event_action(&(e->event),
                                        SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_THE_EVENT_HAS_BEEN_SUPERSEDED,
                                        sl_zigbee_af_next_sequence(),
                                        false,
                                        0);
        voidAllEntriesWithEventId(endpoint,
                                  e->event.eventId);
        return;
      }
    } else if (e->entryStatus == ENTRY_IS_CANCELLED_EVENT) {
      if (e->event.startTime <= ct) {
        sli_zigbee_af_call_event_action(&(e->event),
                                        SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_THE_EVENT_HAS_BEEN_CANCELED,
                                        sl_zigbee_af_next_sequence(),
                                        false,
                                        0);
        voidAllEntriesWithEventId(endpoint,
                                  e->event.eventId);
        return;
      }
    }
  }
}

/**
 * This function is used to schedule events in the
 * load control event table.
 */
void sli_zigbee_af_schedule_load_control_event(uint8_t endpoint,
                                               sl_zigbee_af_load_control_event_t *newEvent)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  sli_zigbee_load_control_event_table_entry_t *e;
  uint32_t ct = sl_zigbee_af_get_current_time();
  sl_zigbee_af_cluster_command_t *curCommand = sl_zigbee_af_current_command();
  uint8_t esiIndex =
    sl_zigbee_af_esi_management_update_esi_and_get_index(curCommand);
  //sl_zigbee_af_esi_management_update_esi_and_get_index(sl_zigbee_af_current_command());

  if (ep == 0xFF) {
    return;
  }

  //validate starttime + duration
  if (newEvent->startTime == 0xffffffffUL
      || newEvent->duration > 0x5a0) {
    sli_zigbee_af_call_event_action(newEvent,
                                    SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_REJECTED,
                                    sl_zigbee_af_current_command()->seqNum,
                                    true,
                                    esiIndex);
    return;
  }

  //validate expiration
  if (ct > (newEvent->startTime + ((uint32_t)newEvent->duration * 60))) {
    sli_zigbee_af_call_event_action(newEvent,
                                    SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_REJECTED_EVENT_EXPIRED,
                                    sl_zigbee_af_current_command()->seqNum,
                                    true,
                                    esiIndex);
    return;
  }

  //validate event id
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (entryIsScheduledOrStarted(e->entryStatus)
        && (e->event.eventId == newEvent->eventId)) {
      // Bug 13805: from multi-ESI specs (5.7.3.5): When a device receives
      // duplicate events (same event ID) from multiple ESIs, it shall send an
      // event response to each ESI. Future duplicate events from the same
      // ESI(s) shall be either ignored by sending no response at all or with a
      // default response containing a success status code.
      //sl_zigbee_af_send_default_response(sl_zigbee_af_current_command(),
      //                           SL_ZIGBEE_ZCL_STATUS_DUPLICATE_EXISTS);

      // First time hearing this event from this ESI. If the ESI is present in
      // the table add the ESI to the event ESI bitmask and respond. If it is
      // a duplicate from the same ESI, we just ingore it.
      if ((e->event.esiBitmask & BIT(esiIndex)) == 0
          && esiIndex < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE) {
        e->event.esiBitmask |= BIT(esiIndex);
        sli_zigbee_af_call_event_action(&(e->event),
                                        SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_RX,
                                        sl_zigbee_af_current_command()->seqNum,
                                        true,
                                        esiIndex);
      }

      return;
    }
  }

  //locate empty table entry
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (e->entryStatus == ENTRY_VOID) {
      memmove(&(e->event), newEvent, sizeof(sl_zigbee_af_load_control_event_t));

      //check for supercession
      for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
        sli_zigbee_load_control_event_table_entry_t *currentEntry = &loadControlEventTable[ep][i];
        if (currentEntry->entryStatus == ENTRY_SCHEDULED
            || currentEntry->entryStatus == ENTRY_STARTED) {
          // If the event is superseded we need to let the application know
          // according to the following conditions interpreted from 075356r15
          // with help from NTS.
          //    1. If superseded event has not started, send superseded
          //       notification to application immediately.
          //    2. If superseded event HAS started, allow to run and send
          //       superseded message 1 second before new event starts.
          //       (to do this we subtract 1 from new event start time to know
          //        when to notify the application that the current running
          //        event has been superseded.)
          if (overlapFound(newEvent, &(currentEntry->event))) {
            if (currentEntry->entryStatus != ENTRY_STARTED) {
              currentEntry->event.startTime = ct;
            } else {
              currentEntry->event.startTime = (newEvent->startTime + newEvent->startRand - 1);
            }
            currentEntry->entryStatus = ENTRY_IS_SUPERSEDED_EVENT;
          }
        }
      }

      e->entryStatus = ENTRY_SCHEDULED;

      // If the ESI is in the table, we add it to the ESI bitmask of this event
      // and we respond.
      if (esiIndex < SL_ZIGBEE_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE) {
        e->event.esiBitmask = BIT(esiIndex);
        sli_zigbee_af_call_event_action(&(e->event),
                                        SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_RX,
                                        sl_zigbee_af_current_command()->seqNum,
                                        true,
                                        esiIndex);
      }

      return;
    }
  }

  // If we get here we have failed to schedule the event because we probably
  // don't have any room in the table and must reject scheduling. We reject but
  // others have chosen to bump the earliest event. There is an ongoing
  // discussion on this issue will be discussed for possible CCB.
  sli_zigbee_af_call_event_action(newEvent,
                                  SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_REJECTED,
                                  sl_zigbee_af_current_command()->seqNum,
                                  true,
                                  esiIndex);
}

void sli_zigbee_af_load_control_event_opt_in_or_out(uint8_t endpoint,
                                                    uint32_t eventId,
                                                    bool optIn)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  sli_zigbee_load_control_event_table_entry_t *e;

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (entryIsScheduledOrStarted(e->entryStatus)
        && e->event.eventId == eventId) {
      // used to find out if we have opted in our out of a running event
      bool previousEventOption = (e->event.optionControl & EVENT_OPT_FLAG_OPT_IN);

      // set the event opt in flag
      e->event.optionControl =
        (optIn
         ? (e->event.optionControl | EVENT_OPT_FLAG_OPT_IN)
         : (e->event.optionControl & ~EVENT_OPT_FLAG_OPT_IN));

      // if we have opted in or out of a running event we need to set the
      // partial flag.
      if ((previousEventOption != optIn)
          && e->entryStatus == ENTRY_STARTED) {
        e->event.optionControl |= EVENT_OPT_FLAG_PARTIAL;
      }

      // Bug: 13546
      // SE 1.0 and 1.1 dictate that if the event has not yet started,
      // and the user opts-out then don't send a status message.
      // Effectively the event is not changing so don't bother
      // notifying the ESI.  When the event would normally start,
      // the opt-out takes effect and that is when we send the opt-out
      // message.
      if (!(e->event.optionControl & ~EVENT_OPT_FLAG_OPT_IN
            && e->entryStatus == ENTRY_SCHEDULED)) {
        sli_zigbee_af_call_event_action(
          &(e->event),
          (optIn
           ? SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_USER_HAS_CHOOSE_TO_OPT_IN
           : SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_USER_HAS_CHOOSE_TO_OPT_OUT),
          sl_zigbee_af_next_sequence(),
          false,
          0);
      }
      return;
    }
  }
}

void sli_zigbee_af_cancel_load_control_event(uint8_t endpoint,
                                             uint32_t eventId,
                                             uint8_t cancelControl,
                                             uint32_t effectiveTime)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  sli_zigbee_load_control_event_table_entry_t *e;
  sl_zigbee_af_load_control_event_t undefEvent;
  uint32_t cancelTime = 0;
  uint8_t esiIndex =
    sl_zigbee_af_esi_management_update_esi_and_get_index(sl_zigbee_af_current_command());

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (e->event.eventId == eventId
        && e->entryStatus != ENTRY_VOID) {
      // Found the event, validate effective time
      if ((effectiveTime == 0xffffffffUL)
          || (effectiveTime > (e->event.startTime
                               + (((uint32_t) e->event.duration) * 60)))) {
        sli_zigbee_af_call_event_action(&(e->event),
                                        SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_REJECTED_INVALID_CANCEL_COMMAND_INVALID_EFFECTIVE_TIME,
                                        sl_zigbee_af_current_command()->seqNum,
                                        true,
                                        esiIndex);
        return;
      }

      // We're good, Run the cancel
      if (cancelControl & CANCEL_WITH_RANDOMIZATION) {
        if (effectiveTime == 0) {
          cancelTime = sl_zigbee_af_get_current_time();
        }
        // CCB 1513: delay by durationRand if instructed, else by startRand
        cancelTime += (e->event.durationRand ? e->event.durationRand
                       : e->event.startRand);
      } else {
        cancelTime = effectiveTime;
      }
      e->entryStatus = ENTRY_IS_CANCELLED_EVENT; //will generate message on next tick
      e->event.startTime = cancelTime;
      return;
    }
  }

  // If we get here, we have failed to find the event
  // requested to cancel, send a fail message.
  initEventData(&undefEvent);
  undefEvent.destinationEndpoint =
    sl_zigbee_af_current_command()->apsFrame->destinationEndpoint;
  undefEvent.eventId = eventId;
  sli_zigbee_af_call_event_action(&undefEvent,
                                  SL_ZIGBEE_ZCL_AMI_EVENT_STATUS_REJECTED_INVALID_CANCEL_UNDEFINED_EVENT,
                                  sl_zigbee_af_current_command()->seqNum,
                                  true,
                                  esiIndex);
}

bool sli_zigbee_af_cancel_all_load_control_events(uint8_t endpoint,
                                                  uint8_t cancelControl)
{
  uint8_t i;
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  sli_zigbee_load_control_event_table_entry_t *e;
  bool generatedResponse = false;

  if (ep == 0xFF) {
    return false;
  }

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    if (e->entryStatus != ENTRY_VOID) {
      sli_zigbee_af_cancel_load_control_event(endpoint, e->event.eventId, cancelControl, 0);
      generatedResponse = true;
    }
  }
  return generatedResponse;
}

static void sli_zigbee_af_call_event_action(sl_zigbee_af_load_control_event_t *event,
                                            uint8_t eventStatus,
                                            uint8_t sequenceNumber,
                                            bool replyToSingleEsi,
                                            uint8_t esiIndex)
{
  if (replyToSingleEsi) {
    sl_zigbee_af_plugin_esi_management_esi_entry_t* esiEntry =
      sl_zigbee_af_esi_management_esi_look_up_by_index(esiIndex);
    if (esiEntry != NULL) {
      sl_zigbee_af_event_action(event,
                                eventStatus,
                                sequenceNumber,
                                esiIndex); // send it to a specific ESI.
    }
    // Response intended for a single ESI. If it does not appear in the table,
    // nothing to do.
  } else {
    sl_zigbee_af_event_action(event,
                              eventStatus,
                              sequenceNumber,
                              0xFF); // send it to all ESIs in the event bitmask.
  }
}

void sli_zigbee_af_note_signature_failure(void)
{
  sl_zigbee_af_demand_response_load_control_cluster_println("Failed to append signature to message.");
}

void sli_zigbee_af_load_control_event_table_print(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  sli_zigbee_load_control_event_table_entry_t *e;
  uint8_t i;

  if (ep == 0xFF) {
    return;
  }

  sl_zigbee_af_demand_response_load_control_cluster_println("ind  st id       sta      dur  ec oc");
  sl_zigbee_af_demand_response_load_control_cluster_flush();

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    e = &loadControlEventTable[ep][i];
    sl_zigbee_af_demand_response_load_control_cluster_println("[%02X] %02X %08X %08X %04X %02X %02X",
                                                              i,
                                                              e->entryStatus,
                                                              e->event.eventId,
                                                              e->event.startTime,
                                                              e->event.duration,
                                                              e->event.eventControl,
                                                              e->event.optionControl);
    sl_zigbee_af_demand_response_load_control_cluster_flush();
  }
}

void sli_zigbee_af_load_control_event_table_init(uint8_t endpoint)
{
  // Subscribe receive deletion announcements.
  sl_zigbee_af_esi_management_subscribe_to_deletion_announcements(esiDeletionCallback);

  sli_zigbee_af_load_control_event_table_clear(endpoint);
}

void sli_zigbee_af_load_control_event_table_clear(uint8_t endpoint)
{
  uint8_t ep = sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID);
  uint8_t i;

  if (ep == 0xFF) {
    return;
  }

#ifdef SL_ZIGBEE_TEST
  // If this asserts, either the ZCL configuration file is incorrect or ZAP is
  // generating incorrect output in the .c/.h files
  assert(ep < SL_ZIGBEE_ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT);
#endif // SL_ZIGBEE_TEST

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_DRLC_EVENT_TABLE_SIZE; i++) {
    memset(&loadControlEventTable[ep][i], 0, sizeof(sli_zigbee_load_control_event_table_entry_t));
  }
}
