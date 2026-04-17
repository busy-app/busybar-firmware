/***************************************************************************//**
 * @file
 * @brief Common definitions and APIs used throughout the zigbee stack.
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

#ifndef SL_ZIGBEE_STACK_H
#define SL_ZIGBEE_STACK_H

// Get the stack configuration from the application first
#include PLATFORM_HEADER  //DOLATER: macro-ize this
// Also include the public definitions
#include "include/sl_zigbee.h"
#include "internal/inc/internal-defs-patch.h"
#include "stack/include/sl_zigbee_types_internal.h"

// Include events because we use the event type in a lot of .h files.
#include "event_queue/event-queue.h"

#ifndef BOOTLOADER
// Needed for the stack profile ID definitions.
#include "routing/zigbee/zigbee.h"
#endif
#include "mac-types.h"

// These are provided by the application in config/sl_zigbee_configuration.c.
extern uint16_t sl_zigbee_mac_indirect_timeout;
extern uint8_t sl_zigbee_end_devicekeep_alive_support_mode;
extern uint8_t sli_zigbee_stack_end_device_poll_timeout;
extern uint16_t sl_zigbee_link_power_delta_interval;

//----------------------------------------------------------------
// Handy macros for exposing things only for testing.
// HIDDEN expands to 'static' in platform builds and disappears in test builds.
// ONLY_IF_EMBER_TEST(x) is x in test builds and disappears in platform builds.

#ifdef SL_ZIGBEE_TEST
#define HIDDEN
#define ONLY_IF_EMBER_TEST(x) x
#else
#define HIDDEN static
#define ONLY_IF_EMBER_TEST(x)
#endif

#ifdef SL_ZIGBEE_LEAF_STACK
  #define NOT_IN_LEAF_STACK(x)
#else
  #define NOT_IN_LEAF_STACK(x) x
#endif

//**************************************************
// Stack private types

#define BROADCAST_ADDRESS     0xFFFF
#define EM_BROADCAST_PAN_ID   SL_ZIGBEE_BROADCAST_PAN_ID
#define EM_USE_LONG_ADDRESS   SL_ZIGBEE_USE_LONG_ADDRESS

// Value that represents the cost of sending a message, taking both hop count
// Less reliable links have a higher cost.
typedef uint8_t sl_zigbee_cost_t;

// The link quality (aka reliability) is a value obtained from the hardware
// which gives the probability of receiving a packet on the link, based on
// how well the packet was heard.  Link quality is computed by the phy in
// a variety of ways (RSSI, spread spectrum correlator output...)
//
// This is encoded as required by 802.15.4, which says that 0 is worst,
// 255 is best.
typedef uint8_t sli_link_quality_t;

// For use by the broadcast table.
typedef uint32_t sli_neighbor_bitmask_t;

// Power management status
enum {
  AWAKE,
  SLEEPY,
  ASLEEP
};

typedef uint8_t sli_power_status_t;

typedef uint16_t sli_child_status_flags_t;

void sli_zigbee_set_network_state(uint8_t newZigbeeState);
#define sli_zigbee_network_up() (sli_zigbee_set_network_state(NETWORK_JOINED))
#define sli_zigbee_network_down(state) (sli_zigbee_set_network_state((state)))

//**************************************************
// Stack private static-params

//**************************************************
// Other Stack private stuff

// modulo increment and decrement
// useful for circular buffers
#define MOD_INC(num, modulus) \
  (((num) == ((modulus) - 1)) ? 0 : ((num) + 1))

#define MOD_DEC(num, modulus) \
  (((num) == 0) ? ((modulus) - 1) : ((num) - 1))

// Returns true if the first 'count' bytes pointed to by 'bytes' are
// all 'target'.
bool sli_zigbee_af_memory_byte_compare(const uint8_t *bytes, uint8_t count, uint8_t target);
#define sli_zigbee_is_memory_zero(bytes, count) (sli_zigbee_af_memory_byte_compare((bytes), (count), 0))
#define isNullEui64(eui64) (sli_zigbee_is_memory_zero((eui64), EUI64_SIZE))

// Decrements the contents of delayTicksLoc by ticksPassed and, if the
// result is greater than zero, sets the contents of minTicksNeededLoc
// to be the minimum of its current contents and the new contents of
// delayTicksLoc.

void sli_zigbee_decrement_ticks(uint16_t *delayTicksLoc,
                                uint16_t ticksPassed,
                                uint16_t *minTicksNeededLoc);

extern sl_zigbee_task_id_t sli_zigbee_stack_task;
#ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
extern sl_zigbee_task_id_t sli_zigbee_multi_pan_event_task;
#endif // SL_ZIGBEE_MULTI_NETWORK_STRIPPED

extern sli_zigbee_event_queue_t sli_zigbee_stack_event_queue;

// The LQI and RSSI of the packet currently being processed.
extern uint8_t sli_zigbee_current_lqi;
extern int8_t sli_zigbee_current_rssi;
// The channel on which the packet currently being processed was received.
extern uint8_t sli_zigbee_current_channel;
// The value of the MAC timer when the SFD was received for the packet currently
// being processed.
extern uint32_t sli_zigbee_current_mac_timestamp;

typedef uint8_t sli_pakcet_dispatch_type_t;
typedef uint8_t sli_802154_mac_frame_type_t;
typedef uint8_t sli_802154_mac_address_mode_t;
typedef uint8_t sli_header_option_type_t;

// MAC scan types.  Note:  this is an expanded list that includes internal as
// well as external scan types.
//
// The RADIO_OFF scan is not really a scan at all.  It is used to
// temporarilly turn off the radio in order to use the TX and RX
// buffers for other purposes.
// Start the scan with:
//   sli_zigbee_stack_start_scan(SL_ZIGBEE_START_RADIO_OFF_SCAN, 0, 0);
// Then wait for sli_802154mac_scan_type to be set to SLI_ZIGBEE_RADIO_OFF_SCAN,
// at which point the radio is off.  There may be a brief
// delay while the radio finishes transmitting.
//
// Call sli_zigbee_stack_stop_scan() to restart the radio and the MAC.

// sl_zigbee_network_scan_type_t
// Continuation of publicly visible scan types
// These scan types do not generate application callbacks

enum {
  SLI_ZIGBEE_STACK_ENERGY_SCAN = SL_ZIGBEE_LAST_SCAN_TYPE + 1,
  SLI_ZIGBEE_STACK_ACTIVE_SCAN,
  SLI_ZIGBEE_PASSIVE_SCAN,
  SLI_ZIGBEE_ORPHAN_SCAN,
  SLI_ZIGBEE_RADIO_OFF_SCAN,          // Indicates that the radio is off.
  SLI_ZIGBEE_STACK_ZLL_ACTIVE_SCAN,
  SLI_ZIGBEE_SCAN_STOPPING,
  SLI_ZIGBEE_SCAN_IDLE,
};

// An enumeration in zigbee/aps-security.h.
typedef uint8_t sli_zigbee_key_type_t;

// Security key and key sequence number are closely related
// under Zigbee.

typedef struct {
  uint8_t key[SECURITY_BLOCK_SIZE];
  uint8_t keySequenceNumber;
} sl_zigbee_security_key_t;

// The fields are ordered to avoid alignment issues on the 250.
typedef struct {
  sl_802154_long_addr_t source;
  sl_802154_long_addr_t destination;
  uint16_t clusterId;
  uint8_t type;           // SL_ZIGBEE_UNICAST_BINDING or SL_ZIGBEE_MULTICAST_BINDING
  uint8_t sourceEndpoint;
  uint8_t destinationEndpoint;
} sli_aps_indirect_binding_t;

// This returns phy interface for given neighbor/child nodeId on simultaneous dual
// mac/phy device, 0xFF if nodeId not found in neighbor/child table respectively.
// For non-dual mac/phy device, it returns phyInterface based on radio channel and ignores
// nodeId argument.
uint8_t sli_zigbee_get_phy_interface_by_node_id(sl_802154_short_addr_t nodeId);
//----------------------------------------------------------------
// Zigbee Pro Node Type and network type macros and functions.

#if defined(SL_ZIGBEE_LEAF_STACK) && !defined(CSL_SUPPORT)
#define sli_zigbee_node_type_is_end_device() (true)
#elif (defined(SL_ZIGBEE_ROUTER_STACK))
#define sli_zigbee_node_type_is_end_device() (false)
#else
#define sli_zigbee_node_type_is_end_device() \
  (sli_zigbee_node_type == SL_ZIGBEE_END_DEVICE || sli_zigbee_node_type == SL_ZIGBEE_SLEEPY_END_DEVICE)
#endif

#if (defined(SL_ZIGBEE_ROUTER_STACK))
#define sli_zigbee_node_type_is_joined_end_device() \
  (false)

#define sli_zigbee_node_type_is_sleepy() \
  (false)

#define sli_zigbee_node_type_is_sleepy_to_sleepy_initiator() \
  (false)

#define sli_zigbee_node_type_is_sleepy_to_sleepy_target() \
  (false)

#define sli_zigbee_node_type_is_sleepy_to_sleepy() \
  (false)

#else
// sli_zigbee_node_type can be SL_ZIGBEE_UNKNOWN_DEVICE, SL_ZIGBEE_COORDINATOR, SL_ZIGBEE_ROUTER, SL_ZIGBEE_END_DEVICE, or SL_ZIGBEE_SLEEPY_END_DEVICE
// on the full stack, sli_zigbee_node_type_is_end_device returns true for SL_ZIGBEE_END_DEVICE and SL_ZIGBEE_SLEEPY_END_DEVICE and false for all others
// on the leaf stack, sli_zigbee_node_type_is_end_device always returns true
// for cases where we originally called sli_zigbee_node_type_is_end_device where the sli_zigbee_node_type can be SL_ZIGBEE_UNKNOWN_DEVICE, we need this condition which returns false in that case
#define sli_zigbee_node_type_is_joined_end_device() \
  (sli_zigbee_node_type == SL_ZIGBEE_END_DEVICE || sli_zigbee_node_type == SL_ZIGBEE_SLEEPY_END_DEVICE)

#define sli_zigbee_node_type_is_sleepy() \
  (sli_zigbee_node_type == SL_ZIGBEE_SLEEPY_END_DEVICE)

#define sli_zigbee_node_type_is_sleepy_to_sleepy_initiator() \
  (sli_zigbee_node_type == SL_ZIGBEE_S2S_INITIATOR_DEVICE)

#define sli_zigbee_node_type_is_sleepy_to_sleepy_target() \
  (sli_zigbee_node_type == SL_ZIGBEE_S2S_TARGET_DEVICE)

#define sli_zigbee_node_type_is_sleepy_to_sleepy() \
  (sli_zigbee_node_type_is_sleepy_to_sleepy_initiator() || sli_zigbee_node_type_is_sleepy_to_sleepy_target())

#endif

// Max Zigbee Pro value
#define sli_zigbee_node_type_max_value() \
  (SL_ZIGBEE_S2S_TARGET_DEVICE)

#ifdef SL_ZIGBEE_LEAF_STACK
#define sli_zigbee_node_type_is_coordinator() (false)
#define sli_zigbee_node_type_is_router() (false)
#elif (defined(SL_ZIGBEE_ROUTER_STACK))
#define sli_zigbee_node_type_is_coordinator() (false)
#define sli_zigbee_node_type_is_router() (true)
#else
#define sli_zigbee_node_type_is_coordinator() \
  (SL_ZIGBEE_COORDINATOR == sli_zigbee_node_type)
#define sli_zigbee_node_type_is_router() \
  (SL_ZIGBEE_ROUTER == sli_zigbee_node_type)
#endif

#define sli_zigbee_node_type_is_coordinator_or_router() \
  (sli_zigbee_node_type_is_coordinator() || sli_zigbee_node_type_is_router())

bool sli_zigbee_is_state_joining_or_rejoining(void);

//----------------------------------------------------------------
// Node ID and Pan ID setters
void sli_zigbee_set_node_id(uint16_t panId);
void sli_zigbee_set_pan_id(uint16_t panId);

//----------------------------------------------------------------
// Internal buffer utilities.

uint8_t sli_zigbee_normalize_buffer_index(sli_buffer_manager_buffer_t *bufferLoc, uint16_t index);
void sli_zigbee_copy_to_normalized_buffers(const uint8_t * contents,
                                           sli_buffer_manager_buffer_t buffer,
                                           uint16_t startIndex,
                                           uint16_t length,
                                           uint8_t direction);
bool sli_zigbee_set_linked_buffers_length(sli_buffer_manager_buffer_t buffer,
                                          uint16_t oldLength,
                                          uint16_t newLength);

//----------------------------------------------------------------
// A simple printf()-like facility for creating messages.  For now this only
// works for messages that don't cross buffer boundaries.
//
// This has not been released to customers, mostly because of the restriction
// about crossing buffer boundaries.
//
// The code here uses format strings to specify how values are to be encoded.
// Each character in a format string gives the format for one
// value.  The formats are case-insenstive.  The current format characters are:
// '1'      A one-byte unsigned value.
// '2'      A two-byte unsigned value encoded as the less-significant byte
//          followed by the more-significant byte.
// '4'      A 4-byte unsigned value encoded as the least-significant byte
//          to the most significant byte.  This use takes precedence over the
//          formatting ranges described below.
// '3'-'9'  A pointer to three to nine bytes (usually 8 for an EUI64) that
//          is to be copied into the packet.
// 'A'-'G'  A pointer to ten to sixteen bytes that is to be copied into the
//          packet.
// 's'      A sequence of unsigned bytes.  When encoding the first supplied
//          value is a pointer to the data and the second value is the number
//          of bytes.
// [Not yet implemented:
// 'p'      Same as 's' except that the bytes are in program space.
// ]

sli_buffer_manager_buffer_t sli_legacy_packet_buffer_make_message(uint16_t startIndex, const char * format, ...);

sli_buffer_manager_buffer_t sli_legacy_packet_buffer_make_message_using_va_list(uint16_t startIndex,
                                                                                const char * format,
                                                                                va_list argPointer);

// Similar to sli_legacy_packet_buffer_make_message_using_va_list(), except that it operates
// on a flat buffer (up to maxLength).  The buffer is assumed to be
// big enough to hold the message
uint16_t sli_legacy_packet_buffer_write_message(uint8_t* buffer,
                                                uint16_t maxLength,
                                                uint16_t startIndex,
                                                const char * format,
                                                ...);
uint16_t sli_legacy_packet_buffer_write_message_using_va_list(uint8_t* buffer,
                                                              uint16_t maxLength,
                                                              uint16_t startIndex,
                                                              const char * format,
                                                              va_list argPointer);

uint8_t sli_zigbee_parse_buffer(sli_buffer_manager_buffer_t message,
                                uint16_t startIndex,
                                const char * format,
                                ...);

uint8_t sli_zigbee_parse_buffer_from_va_list(sli_buffer_manager_buffer_t message,
                                             uint16_t startIndex,
                                             const char * format,
                                             va_list elements);

//----------------------------------------------------------------
// Message buffer queue functions.
//
// The link fields point from head to tail, except that the final tail's link
// points back to the head, forming a loop.  This allows us to add to the tail
// and remove from the head in constant time.

 #define sli_zigbee_message_buffer_queue_head(queue) \
  sli_legacy_buffer_manager_buffer_queue_head((queue))

#define sli_zigbee_message_buffer_queue_purge(queueAddr) \
  *(queueAddr) = SL_ZIGBEE_NULL_MESSAGE_BUFFER

 #define sli_legacy_packet_buffer_queue_removeTail(queue) \
  (sli_legacy_packet_buffer_queue_remove((queue), *(queue)))
#define sli_zigbee_message_buffer_queue_length(queue) \
  (sli_legacy_buffer_manager_buffer_queue_length((queue)))
// This function inserts a buffer before a buffer in a buffer queue
void sli_legacy_buffer_manager_buffer_queue_pre_insert_buffer(sli_zigbee_message_buffer_queue_t *queue,
                                                              sli_buffer_manager_buffer_t bufferToInsert,
                                                              sli_buffer_manager_buffer_t bufferToInsertBefore);

#define sli_zigbee_message_buffer_queue_pre_insert_buffer sli_legacy_buffer_manager_buffer_queue_pre_insert_buffer

extern sli_zigbee_message_buffer_queue_t sli_802154mac_to_network_queue;
extern sli_zigbee_message_buffer_queue_t sli_zigbee_network_to_application_queue;

// Looping through the buffers in a queue is a little tricky with the
// representation that we use, so here is a handy macro for doing so.
// 'body' gets executed with 'finger' set to the successive queue
// elements, from head to tail.
//
// Notes:
//  - 'queue' should have the type 'sli_zigbee_message_buffer_queue_t *'
//  - Do not declare the 'finger' variable.
//  - Use 'return' or 'goto' to do an early exit (do not use 'break').
//
// For example, the following sums up the lengths of the buffers in a queue.
//
// {
//   uint8_t count = 0;
//   WALK_QUEUE(theQueue, finger,
//     {
//       count += sl_legacy_buffer_manager_message_buffer_length(finger);
//     });
// }

#define WALK_QUEUE(queue, finger, body)                                                      \
  do {                                                                                       \
    sli_buffer_manager_buffer_t finger = sli_legacy_buffer_manager_buffer_queue_head(queue); \
    while (finger != NULL_BUFFER) {                                                          \
      do { body } while (0);                                                                 \
      finger = sli_legacy_buffer_manager_buffer_queue_next(queue, finger);                   \
    }                                                                                        \
  } while (0)

//----------------------------------------------------------------

// A quick & dirty mechansim for debugging precise timing in the stack.
// Written to debug the secure association process.  I've left the calls
// to sli_zigbee_note_time() sprinkled in the code.  Also see app/test/traffic.c
// which implements the callback to record the notes so they can
// be printed at a later time. -MNP
void sli_zigbee_note_time_handler(const char * filename, int linenumber, uint8_t key);
#define sli_zigbee_note_time(key)
// #define sli_zigbee_note_time(key) sli_zigbee_note_time_handler(__SOURCEFILE__, __LINE__, key)

#if defined(SL_ZIGBEE_TEST)
extern uint16_t simulatorId;
extern uint16_t rebootCount;
void simPrint(char *format, ...);
void debugSimPrint(char *format, ...);
void printBuffer(const char* label, const uint8_t *buffer, uint16_t length);
void debugPrintTextAndHex(const char* text,
                          const uint8_t* hexData,
                          uint8_t length,
                          uint8_t spaceEveryXChars,
                          bool finalCr);

void scriptTestCheckpoint(char* formatString, ...);

#elif !defined(SL_ZIGBEE_TEST) && !defined(SL_ZIGBEE_SCRIPTED_TEST)
  #define simPrint(...)
  #define debugSimPrint(...)
  #define scriptTestCheckpoint(string, ...)
  #define debugPrintTextAndHex(...)
  #define printBuffer(...)
#endif

#if defined(SL_ZIGBEE_TEST) || defined(ZIGBEE_STACK_ON_HOST)
typedef struct sli_parcel_s_t {
  uint32_t tag;           // for safety
  int length;
  uint8_t contents[0];
} sli_parcel_t;
#endif

#ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
#define NWK_INDEX nwk_index
#else
#define NWK_INDEX 0
#endif

#if (defined(MAC_DUAL_PRESENT) || defined(ALT_MAC))
#define MAC_INDEX mac_index
#else
#define MAC_INDEX 0
#endif

#endif // SL_ZIGBEE_STACK_H
