/***************************************************************************//**
 * @file
 * @brief  *  Binary debug message format:
 *   Byte  Description
 *   ----- ---------------------------------------------------------------------
 *    0    initial framing: '['
 *    1    length of data, doesn't count itself, framing, msg type, timestamp
 *    2    timestamp low byte
 *    3    timestamp high byte
 *    4    Debug Message Type
 *    ...  Data dependant on the Message Type
 *    Nth byte, end framing: ']'
 *
 *  If a debug packet as described above is "n" bytes long,
 *   the length byte will be equal to n-6.
 *
 *  Message Types and format:
 *  DebugPrintf
 *    ... ascii debug string
 *
 *  API Trace
 *    Byte 5, API subtype
 *    ... concatenated API parameters, the parser of the protocol needs
 *        to decipher based on API subtype; pointer parameters are not
 *        included in message.
 *
 *  Time sync
 *    No data, length is zero.  All data needed is in timestamp.
 *
 *  Assert
 *    Byte 5, line number low byte
 *    Byte 6, line number high byte
 *    ... ascii filename
 *
 *  Core Dump
 *    Byte 5, address low byte
 *    Byte 6, address high byte
 *    ... memory dump from given address
 *        (Note: if 5th and 6th bytes are 0xFFFF then data is processor state)
 *
 *  Reset & init info
 *    Byte 5, reset type
 *    Byte 6, reserved
 *    Bytes 7-14, EUI64
 *    Byte 15, stack configuration
 *    Byte 16, phy type
 *    Byte 17, stack version (high nibble major, low nibble minor)
 *    Byte 18, build number
 *    Byte 19, Perforce change number low byte
 *    Byte 20, Perforce change number high byte
 *    Byte 21, initial channel number
 *    Byte 22, initial radio power
 *
 *  Packet Trace
 *    Byte 5, packet trace subtype
 *    ... format depends on subtype.  Details below.
 *
 *  Error
 *    Byte 5: error code.
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

#ifndef SILABS_DEBUG_H
#define SILABS_DEBUG_H
//******************************************************************************

// Include generated debug message types
#include "debug-message-type-gen.h"

// Include generate apitrace calls.
// This include includes the enum for the api trace types
// as well as macros for all apitrace calls.
#ifdef ZIGBEE_STACK_ON_HOST
#include "zigbee_debug_stack_on_host.h"
#else
#include "debug-gen.h"
#define HOST_DEBUG_LOG(...)  do {} while (false)
#endif // ZIGBEE_STACK_ON_HOST

// Subtypes for use with EM_DEBUG_STATS type.
enum {
  EM_STATS_MAC_CSMA_FAILURE,
  EM_STATS_MAC_SEND_SUCCESS,
  EM_STATS_MAC_SEND_FAILURE,
  EM_STATS_MAC_RECEIVE,        // in mac-arbiter.c:
  EM_STATS_DROP_DEBUG_MESSAGE, // in debug.c
  EM_STATS_OUT_OF_BUFFERS,     // in packet-buffer.c
  EM_STATS_ARRAY_SIZE          // this entry must be last
};

#if !defined(SL_ZIGBEE_TEST_ASSERT)
 #define SL_ZIGBEE_TEST_ASSERT(x) assert(x)
#endif

//******************************************************************************

#if defined(DEBUG_LEVEL) && defined(BASIC_DEBUG) && (DEBUG_LEVEL >= BASIC_DEBUG)
void sli_zigbee_debug_queue_add(sli_buffer_manager_buffer_t buffer);

void sl_zigbee_debug_handler(uint8_t messageLength, uint8_t *messageContents);
void sli_zigbee_debug_process_incoming(uint16_t debugType, uint8_t *data, uint8_t length);
void sli_zigbee_debug_reset_info(void);
void sli_zigbee_debug_binary_format(uint16_t debugType, const char * formatString, ...);
void sli_zigbee_debug_receive_tick(void);
bool sli_zigbee_debug_should_ignore_trace_for_type(uint16_t debugType);
void sli_zigbee_debug_memory_dump(uint16_t debugType, uint8_t *start, uint8_t *end);
void sli_zigbee_debug_internal_binary_printf(uint16_t debugType,
                                             const char * formatString,
                                             va_list args);
//#if DEBUG_LEVEL == FULL_DEBUG
bool sli_zigbee_debug_process_incoming_full_debug_messages(uint16_t debugType, uint8_t *data, uint8_t length);
void sli_zigbee_debug_api_trace(uint16_t debugType, const char * formatString, ...);
void sli_zigbee_debug_clear_stats(void);
void sli_zigbee_debug_print_stats(void);
void sli_zigbee_debug_stats(uint8_t index);
  #define IF_DEBUG(foo) do { foo; } while (false)
  #define EXEC_AND_ASSERT_IF_DEBUG(foo) assert(foo)
//#endif
#else   // DEBUG_LEVEL < BASIC_DEBUG
  #define sli_zigbee_debug_queue_add(buffer) do {} while (false)
  #define sli_zigbee_debug_process_incoming(debugType, data, length) do {} while (false)
  #define sli_zigbee_debug_reset_info() do {} while (false)
  #define sli_zigbee_debug_binary_format(debugType, formatString, ...) do {} while (false)
  #define sli_zigbee_debug_receive_tick() do {} while (false)
  #define sli_zigbee_debug_process_incoming_full_debug_messages(debugType, data, length) do {} while (false)
  #define sli_zigbee_debug_api_trace(debugType, message) do {} while (false)
  #define sli_zigbee_debug_clear_stats() do {} while (false)
  #define sli_zigbee_debug_print_stats() do {} while (false)
  #define sli_zigbee_debug_stats(index) do {} while (false)
  #define IF_DEBUG(foo) do {} while (false)
  #define EXEC_AND_ASSERT_IF_DEBUG(foo) do { foo; } while (false)
#endif

#endif //__DEBUG_H__
