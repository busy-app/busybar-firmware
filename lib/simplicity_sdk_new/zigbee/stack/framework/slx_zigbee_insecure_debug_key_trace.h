/***************************************************************************//**
 * @file slx_zigbee_insecure_debug_key_trace.h
 * @brief provides dumps of security information
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

/// NOTE extreme caution is advised when enabling this component
/// it should only be used during development and testing.
/// DO NOT enable this on production.

#ifndef SLX_ZIGBEE_INSECURE_DEBUG_KEY_TRACE_H
#define SLX_ZIGBEE_INSECURE_DEBUG_KEY_TRACE_H

#include "sl_common.h"
#include "sl_enum.h"

/// NOTE Wireshark decoders understand a special network status message as
/// containing an encryption key.
#define APS_KEY_DEBUG_WIRESHARK_NWK_REPORT_MAGIC_BYTE 0xC8

/**
 * @brief defines the type of debug message to send
 */
SL_ENUM(slx_zigbee_insecure_debug_message_type) {
  SLX_ZIGBEE_INSECURE_DEBUG_NWK_REPORT_KEY_PACKET,
  SLX_ZIGBEE_INSECURE_DEBUG_TRANSPORT_KEY_PACKET,
};

/**
 * @brief generates a debug message of the given type using the data parameter
 * @param msg_type an enumeration representing which kind of debug msg to generate
 * @param debug_data an opaque pointer type. semantics are specific to each message
 */
void slx_zigbee_insecure_debug_generate_trace(slx_zigbee_insecure_debug_message_type msg_type,
                                              void *debug_data);

#endif // SLX_ZIGBEE_INSECURE_DEBUG_KEY_TRACE_H
