/***************************************************************************//**
 * @file slx_zigbee_insecure_debug_key_trace.c
 * @brief provides dumps of security information via several interfaces
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

#include "slx_zigbee_insecure_debug_key_trace.h"
#include "sl_zigbee_types.h"
#include "sl_zigbee_debug_print.h"
#include "sl_zigbee_stack.h"
#include "zigbee-packet-header.h"
#include "stack/routing/util/retry.h"
#include "stack/zigbee/aps-security.h"
#include "sl_cli.h"
#include "zigbee_packet_types.h"

#include "slx_zigbee_insecure_debug_key_trace_config.h"
static uint8_t insecure_debug_trace_enabled = SLX_ZIGBEE_INSECURE_DEBUG_DEFAULT_STATE;

#define WIRESHARK_NWK_REPORT_FRAME_LENGTH (2 + EXTENDED_PAN_ID_SIZE + SL_ZIGBEE_ENCRYPTION_KEY_SIZE)
void slx_zigbee_insecure_debug_generate_trace(slx_zigbee_insecure_debug_message_type msg_type,
                                              void *debug_data)
{
  if (debug_data == NULL || !insecure_debug_trace_enabled) {
    return;
  }
  switch (msg_type) {
#if SLX_ZIGBEE_INSECURE_DEBUG_NWK_REPORT_KEY_PACKET_ENABLED == 1
    case SLX_ZIGBEE_INSECURE_DEBUG_NWK_REPORT_KEY_PACKET: {
      sli_buffer_manager_buffer_t payload = sli_legacy_buffer_manager_allocate_buffer(WIRESHARK_NWK_REPORT_FRAME_LENGTH);
      if (payload == NULL_BUFFER) {
        return;
      }
      uint8_t *frame = sli_legacy_buffer_manager_get_buffer_pointer(payload);
      frame[0] = ZIGBEE_NETWORK_REPORT;
      // Bit "Request" set.
      frame[1] = APS_KEY_DEBUG_WIRESHARK_NWK_REPORT_MAGIC_BYTE;
      sli_zigbee_stack_get_extended_pan_id(&frame[2]);
      // data is a key
      memmove(&frame[2 + EXTENDED_PAN_ID_SIZE], (uint8_t *) debug_data, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

      uint16_t nwk_frame_control
        = (ZIGBEE_FRAME_CONTROL_FRAME_TYPE_COMMAND
           | ZIGBEE_FRAME_CONTROL_PROTOCOL_VERSION
           | ZIGBEE_FRAME_CONTROL_SOURCE_IEEE_ADDRESS);
      sli_zigbee_packet_header_t header = sli_zigbee_make_zigbee_header(SL_ZIGBEE_NULL_MESSAGE_BUFFER,
                                                                        nwk_frame_control,
                                                                        SL_ZIGBEE_BROADCAST_ADDRESS,
                                                                        NULL,
                                                                        1);

      if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
        return;
      }

      if (sl_legacy_buffer_manager_append_to_linked_buffers(header, sli_legacy_buffer_manager_get_buffer_pointer(payload), sli_legacy_buffer_manager_get_buffer_length(payload)) != SL_STATUS_OK) {
        return;
      }

      (void) sli_zigbee_retry_submit(header, 1, 0, SLI_ZIGBEE_RETRY_FLAG_NONE);
    } break;
#endif // SLX_ZIGBEE_INSECURE_DEBUG_NWK_REPORT_KEY_PACKET_ENABLED
#if SLX_ZIGBEE_INSECURE_DEBUG_TRANSPORT_KEY_PACKET_ENABLED == 1
    case SLX_ZIGBEE_INSECURE_DEBUG_TRANSPORT_KEY_PACKET: {
      // data is a key
      sl_802154_long_addr_t dummy_long = { 0xFF, 0xEE, 0xFF, 0x11, 0xFF, 0x00, 0xFF, 0x6d };
      (void) sli_zigbee_send_key(SL_ZIGBEE_BROADCAST_ADDRESS,
                                 dummy_long,
                                 dummy_long,
                                 KEY_TRANSPORT_INVALID_KEY,
                                 (sl_zigbee_key_data_t *) debug_data,
                                 SL_ZIGBEE_APS_OPTION_NONE);
    } break;
#endif // SLX_ZIGBEE_INSECURE_DEBUG_TRANSPORT_KEY_PACKET_ENABLED
    default:
      sl_zigbee_core_debug_println("unsupported msg type %d", msg_type);
  }
}

void insecure_debug_key_enable(sl_cli_command_arg_t *arguments)
{
  insecure_debug_trace_enabled = true;
  if (sl_cli_get_argument_count(arguments) >= 1) {
    insecure_debug_trace_enabled = sl_cli_get_argument_uint16(arguments, 0);
  }
  sl_zigbee_core_debug_println("enable insecure debug key trace? %c", insecure_debug_trace_enabled ? 'y' : 'n');
}
