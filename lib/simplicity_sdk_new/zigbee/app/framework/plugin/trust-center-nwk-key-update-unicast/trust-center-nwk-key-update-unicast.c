/***************************************************************************//**
 * @file
 * @brief Functionality for rolling the network key by unicasting the key update
 * to all devices in the Trust Center's link key table with authorized keys.
 * This is the preferred mechanism for Smart Energy.
 * This does not work for devices using a global link key.
 *
 * The process is a little more complex than a broadcast key update:
 *   1. Broadcast a Many-to-one Route Record (advertisement) to insure
 *      devices will send route record messages to the TC so that we have
 *      the latest routes.
 *   2. For each ROUTER entry in the key-table (sleepies are assumed to
 *      always miss key updates, so we don't bother):
 *      A. Broadcast a ZDO address discovery for the target device's short
 *         address.
 *      B. If we receive a response, they will also send a Route Record
 *         containing route information AND long->short ID mapping.
 *         In that case send a unicast NWK key update to the device.
 *         If we don't get any response, skip them.
 *   3. Broadcast NWK key switch
 *
 * Note: This does not PERIODICALLY update the NWK key.  It just manages the
 * process when it is told to initate the key change.  Another software
 * module must determine the policy for periodically initiating this process
 * (such as the 'Trust Center NWK Key Update Periodic' App Framework Plugin).
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
#include "app/framework/util/util.h"

#include "app/framework/util/af-main.h"

#include "trust-center-nwk-key-update-unicast.h"
#include "stack/include/zigbee-security-manager.h"

#include "app/framework/plugin/concentrator/concentrator-support.h"
#include "stack/include/zigbee-security-manager.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

// *****************************************************************************
// Globals

enum {
  KEY_UPDATE_NONE,
  BROADCAST_MTORR,
  TRAVERSE_KEY_TABLE,
  DISCOVERING_NODE_ID,
  SENDING_KEY_UPDATES,
  BROADCAST_KEY_SWITCH,
};
typedef uint8_t sli_zigbee_key_update_state_id_t;

#define KEY_UPDATE_STATE_STRINGS { \
    "None",                        \
    "Broadcast MTORR",             \
    "Traverse Key Table",          \
    "Discovering Node Id",         \
    "Unicasting Key Updates",      \
    "Broadcast Key Switch",        \
}

const char * keyUpdateStateStrings[] = KEY_UPDATE_STATE_STRINGS;

#define KEY_MASK (SL_ZIGBEE_KEY_IS_AUTHORIZED \
                  | SL_ZIGBEE_KEY_PARTNER_IS_SLEEPY)

enum {
  OPERATION_START   = 0,
  OPERATION_FAILED  = 1,
  TIMER_EXPIRED     = 2,
};
typedef uint8_t sli_zigbee_key_update_result_t;

typedef void (KeyUpdateFunction)(sli_zigbee_key_update_result_t);

typedef struct {
  KeyUpdateFunction* function;
  sli_zigbee_key_update_state_id_t stateId;
} sli_zigbee_key_update_state_t;

sl_zigbee_af_event_t sl_zigbee_af_trust_center_nwk_key_update_unicast_my_event;
#define myEvent (&sl_zigbee_af_trust_center_nwk_key_update_unicast_my_event)
void sl_zigbee_af_trust_center_nwk_key_update_unicast_my_event_handler(sl_zigbee_af_event_t * event);

static sli_zigbee_key_update_state_id_t currentStateId = KEY_UPDATE_NONE;

#define EXTRA_MTORR_DELAY_QS 4

static int16_t keyTableIndex = -1;
static sl_802154_short_addr_t discoveredNodeId;

// This is used as a per-device failure count for ZDO discovery, and a single
// failure count for broadcasting the key-switch
static uint8_t failureCount = 0;

#define FAILURE_COUNT_THRESHOLD 3
#define ZDO_DELAY_AFTER_FAILURE_QS 4
#define SEND_KEY_FAILURE_DELAY_QS  4
#define KEY_UPDATE_DELAY_QS        4
#define BROADCAST_KEY_SWITCH_DELAY_QS 4
#define BROADCAST_KEY_SWITCH_DELAY_AFTER_FAILURE_QS 4

#if defined SL_ZIGBEE_TEST
  #define SL_ZIGBEE_TEST_PUBLIC
#else
  #define SL_ZIGBEE_TEST_PUBLIC static
#endif

#if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT)
// For testing, we need to support a single application that can do
// unicast AND broadcast key updates.  So we re-map the function name
// so both can be compiled in without conflict.
  #define sl_zigbee_af_trust_center_start_network_key_update sl_zigbee_af_trust_center_start_unicast_network_key_update
#endif

// *****************************************************************************
// State Machine

static void broadcastMtorr(sli_zigbee_key_update_result_t status);
static void traverseKeyTable(sli_zigbee_key_update_result_t status);
static void zdoDiscovery(sli_zigbee_key_update_result_t status);
static void sendKeyUpdate(sli_zigbee_key_update_result_t status);
static void broadcastKeySwitch(sli_zigbee_key_update_result_t status);

static const sli_zigbee_key_update_state_t stateTable[] = {
  { NULL, KEY_UPDATE_NONE },
  { broadcastMtorr, BROADCAST_MTORR },
  { traverseKeyTable, TRAVERSE_KEY_TABLE },
  { zdoDiscovery, DISCOVERING_NODE_ID },
  { sendKeyUpdate, SENDING_KEY_UPDATES },
  { broadcastKeySwitch, BROADCAST_KEY_SWITCH },
};

// *****************************************************************************
// Forward Declarations

static void keyUpdateGotoState(sli_zigbee_key_update_state_id_t state);
#define keyUpdateGotoNextState() keyUpdateGotoState(currentStateId + 1)

//-------internal callback
void sli_zigbee_af_trust_center_nwk_key_update_unicast_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(myEvent,
                          sl_zigbee_af_trust_center_nwk_key_update_unicast_my_event_handler);
}

// *****************************************************************************
// Functions
void sl_zigbee_af_trust_center_nwk_key_update_unicast_my_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(myEvent);
  if (stateTable[currentStateId].function != NULL) {
    stateTable[currentStateId].function(TIMER_EXPIRED);
  }
}

static void keyUpdateGotoStateAfterDelay(sli_zigbee_key_update_state_id_t stateId, uint32_t delayQs)
{
  currentStateId = stateId;
  sl_zigbee_af_security_flush();
  sl_zigbee_af_security_println("Delaying %ld sec. before going to state: %s",
                                delayQs >> 2,
                                keyUpdateStateStrings[currentStateId]);
  sl_zigbee_af_security_flush();
  sl_zigbee_af_event_set_delay_qs(myEvent, delayQs);
}

static void keyUpdateGotoState(sli_zigbee_key_update_state_id_t stateId)
{
  currentStateId = stateId;
  sl_zigbee_af_security_println("Key Update State: %s",
                                keyUpdateStateStrings[currentStateId]);
  stateTable[currentStateId].function(OPERATION_START);
}

static void broadcastMtorr(sli_zigbee_key_update_result_t result)
{
  uint16_t delayQs;

  if (result == TIMER_EXPIRED) {
    keyUpdateGotoNextState();
    return;
  }

  delayQs = sl_zigbee_af_concentrator_queue_discovery() + EXTRA_MTORR_DELAY_QS;
  sl_zigbee_af_security_println("NWK Key Update: Sending MTORR in %d sec.",
                                delayQs >> 2);
  sl_zigbee_af_event_set_delay_qs(myEvent, delayQs);
}

static void traverseKeyTable(sli_zigbee_key_update_result_t result)
{
  UNUSED_VAR(result);
  keyTableIndex++;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;

  for (; keyTableIndex < sl_zigbee_af_get_key_table_size(); keyTableIndex++) {
    sl_zigbee_sec_man_aps_key_metadata_t key_data;
    context.key_index = keyTableIndex;
    context.flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;
    sl_status_t status = sl_zigbee_sec_man_get_aps_key_info(&context,
                                                            &key_data);
    if (status == SL_STATUS_OK) {
      if ((key_data.bitmask & KEY_MASK)
          == SL_ZIGBEE_KEY_IS_AUTHORIZED) {
        sl_zigbee_af_security_println("Updating NWK key at key table index %d",
                                      keyTableIndex);
        keyUpdateGotoNextState();
        return;
      } else {
        sl_zigbee_af_security_println("Skipping key table index %d (unauthorized key or sleepy child)",
                                      keyTableIndex);
      }
    }
  }
  sl_zigbee_af_security_println("Finishing traversing key table.");
  currentStateId = BROADCAST_KEY_SWITCH;
  sl_zigbee_af_event_set_delay_qs(myEvent,
                                  BROADCAST_KEY_SWITCH_DELAY_QS);
}

SL_ZIGBEE_TEST_PUBLIC void zdoDiscoveryCallback(const sl_zigbee_af_service_discovery_result_t* result)
{
  if (result->status == SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED
      || result->status == SL_ZIGBEE_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    discoveredNodeId = result->matchAddress;
    sl_zigbee_af_security_println("Key Table index %d is node ID 0x%04X",
                                  keyTableIndex,
                                  discoveredNodeId);
    keyUpdateGotoNextState();
  } else if (discoveredNodeId != SL_ZIGBEE_NULL_NODE_ID) {
    // Do nothing, this is just a callback telling us this is the end of the discovery.
    // Since we have received the node ID by now, we can safely ignore this callback.
  } else {
    sl_zigbee_af_security_println("Could not find node ID for key table entry %d",
                                  keyTableIndex);
    zdoDiscovery(OPERATION_FAILED);
  }
}

static sl_status_t getEui64OfCurrentKeyTableIndex(sl_802154_long_addr_t returnEui64)
{
  sl_zigbee_sec_man_aps_key_metadata_t key_data;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  context.key_index = keyTableIndex;
  context.flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;

  sl_status_t status = sl_zigbee_sec_man_get_aps_key_info(&context, &key_data);

  if (status == SL_STATUS_OK) {
    memmove(returnEui64, context.eui64, EUI64_SIZE);
  }
  return ((status == SL_STATUS_OK) ? SL_STATUS_OK : SL_STATUS_NOT_FOUND);
}

static void zdoDiscovery(sli_zigbee_key_update_result_t result)
{
  sl_802154_long_addr_t longId;
  sl_status_t status;

  discoveredNodeId = SL_ZIGBEE_NULL_NODE_ID;

  if (result == OPERATION_FAILED) {
    failureCount++;
    if (failureCount >= FAILURE_COUNT_THRESHOLD) {
      goto zdoDiscoveryFail;
    }
  }

  if (SL_STATUS_OK == getEui64OfCurrentKeyTableIndex(longId)) {
    status = sl_zigbee_af_find_node_id(longId, zdoDiscoveryCallback);
    sl_zigbee_af_security_println("Discovering node ID for key table %d",
                                  keyTableIndex);
    if (status != SL_STATUS_OK) {
      failureCount++;
      sl_zigbee_af_security_println("Failed to start Node ID dsc.  Failure count: %d",
                                    failureCount);
    } else {
      return;
    }
  } else {
    sl_zigbee_af_security_println("Failed to get EUI64 of current key table entry (%d).  Skipping.",
                                  keyTableIndex);
    failureCount = FAILURE_COUNT_THRESHOLD;
  }

  zdoDiscoveryFail:
  if (failureCount >= FAILURE_COUNT_THRESHOLD) {
    sl_zigbee_af_security_println("Maximum error count reached (%d), skipping key table entry %d",
                                  FAILURE_COUNT_THRESHOLD,
                                  keyTableIndex);
    traverseKeyTable(OPERATION_FAILED);
  } else {
    sl_zigbee_af_event_set_delay_qs(myEvent,
                                    ZDO_DELAY_AFTER_FAILURE_QS);
  }
}

static bool nextNetworkKeyIsNewer(sl_zigbee_key_struct_t* nextNwkKey)
{
  sl_status_t status;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_network_key_info_t key_info;

  sl_zigbee_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_NETWORK;
  context.key_index = 1;

  status = sl_zigbee_sec_man_export_key(&context, &plaintext_key);
  memmove(&(nextNwkKey->key),
          &plaintext_key.key,
          SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

  // It is assumed that the current nwk key has valid data.
  (void)sl_zigbee_sec_man_get_network_key_info(&key_info);
  if (status != SL_STATUS_OK
      || (timeGTorEqualInt8u(key_info.network_key_sequence_number,
                             key_info.alt_network_key_sequence_number))) {
    return false;
  }

  return true;
}

static void abortKeyUpdate(sl_status_t status)
{
  sl_zigbee_af_security_println("Key Update %s (0x%02X)",
                                (status == SL_STATUS_OK
                                 ? "complete"
                                 : "aborted"),
                                status);
  currentStateId = KEY_UPDATE_NONE;
  keyTableIndex = -1;
  failureCount = 0;
  sl_zigbee_af_network_key_update_complete_cb(status);
}

static void sendKeyUpdate(sli_zigbee_key_update_result_t result)
{
  sl_zigbee_key_struct_t nextNwkKey;
  sl_status_t status;

  if (result == OPERATION_START) {
    failureCount = 0;
  }

  if (!nextNetworkKeyIsNewer(&nextNwkKey)) {
    // Setting the key to all zeroes tells the stack
    // to randomly generate a new key and use that.
    memset(sl_zigbee_key_contents(&(nextNwkKey.key)),
           0,
           SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  }

  {
    sl_802154_long_addr_t eui64;
    if (SL_STATUS_OK != getEui64OfCurrentKeyTableIndex(eui64)) {
      sl_zigbee_af_security_println("Failed to get EUI64 of index %d",
                                    keyTableIndex);
      return;
    }
    sl_zigbee_af_security_println("Sending NWK Key update to 0x%04X",
                                  discoveredNodeId);

    status = sl_zigbee_send_unicast_network_key_update(discoveredNodeId,
                                                       eui64,
                                                       &(nextNwkKey.key));
  }

  if (status != SL_STATUS_OK) {
    failureCount++;
    sl_zigbee_af_security_println("Failed to unicast NWK key update (%d).  Failure count: %d",
                                  status,
                                  failureCount);
    if (failureCount >= FAILURE_COUNT_THRESHOLD) {
      sl_zigbee_af_security_println("Maximum failure count hit (%d) for sending key update, aborting.",
                                    FAILURE_COUNT_THRESHOLD);
      abortKeyUpdate(status);
      return;
    }

    sl_zigbee_af_event_set_delay_qs(myEvent,
                                    SEND_KEY_FAILURE_DELAY_QS);
  } else {
    keyUpdateGotoStateAfterDelay(TRAVERSE_KEY_TABLE,
                                 KEY_UPDATE_DELAY_QS);
  }
}

static void broadcastKeySwitch(sli_zigbee_key_update_result_t result)
{
  sl_status_t status;
  if (result != TIMER_EXPIRED) {
    failureCount = 0;
  }
  status = sl_zigbee_broadcast_network_key_switch();
  if (SL_STATUS_OK != status) {
    failureCount++;
    sl_zigbee_af_security_println("Failed to broadcast key switch, failures: %d.  Will retry.",
                                  failureCount);
    if (failureCount >= FAILURE_COUNT_THRESHOLD) {
      sl_zigbee_af_security_println("Max fail count hit (%d), aborting key update.");
    } else {
      sl_zigbee_af_event_set_delay_qs(myEvent,
                                      BROADCAST_KEY_SWITCH_DELAY_AFTER_FAILURE_QS);
    }
  }
  sl_zigbee_af_security_println("Sent NWK key switch.");

  abortKeyUpdate(status);
}

sl_status_t sl_zigbee_af_trust_center_start_network_key_update(void)
{
  if (currentStateId != KEY_UPDATE_NONE
      || sl_zigbee_af_network_state() != SL_ZIGBEE_JOINED_NETWORK
      || sl_zigbee_af_get_node_id() != SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    return SL_STATUS_INVALID_STATE;
  }

  keyUpdateGotoNextState();
  return SL_STATUS_OK;
}
