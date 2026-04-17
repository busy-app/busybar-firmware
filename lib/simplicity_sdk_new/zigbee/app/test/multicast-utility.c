/***************************************************************************//**
 * @file
 * @brief
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

// File: multicast-utility.c
//
// Description:  Implements some utilities for multicast testing.
//
// Author(s): Maurizio Nanni <maurizio.nanni@ember.com>
//
// Copyright 2011 by Ember Corporation.  All rights reserved.               *80*

#include PLATFORM_HEADER

// Ember stack and related utilities.
#include "stack/include/sl_zigbee.h"               // Main stack definitions.
#include "stack/include/zigbee-device-stack.h" // ZigBee Device Object.

#include "stack/core/sl_zigbee_stack.h"
#include "stack/core/sl_zigbee_multi_network.h"
#include "stack/include/sl_zigbee_types_internal.h"
#include "stack/framework/packet-header.h"
#include "stack/framework/zigbee-packet-header.h"
#include "stack/routing/zigbee/association.h"
#include "stack/routing/zigbee/network.h"
#include "stack/zigbee/aps-security.h"
#include "stack/include/zigbee_packet_types.h"

sl_status_t sendMulticast(sl_zigbee_aps_frame_t *apsFrame,
                          uint8_t radius,
                          uint16_t broadcastAddr,
                          sli_buffer_manager_buffer_t message)
{
  if (sli_zigbee_state != NETWORK_JOINED) {
    return SL_STATUS_NETWORK_DOWN;
  } else if (apsFrame->options & SL_ZIGBEE_APS_OPTION_SECURITY) {
    return SL_STATUS_FAIL;
  } else {
    sli_zigbee_packet_header_t header;
    bool success;
    if (message != SL_ZIGBEE_NULL_MESSAGE_BUFFER
        && (sl_legacy_buffer_manager_message_buffer_length(message)
            > (sl_zigbee_maximum_aps_payload_length()
               - 1))) { // NWK multicast: Add multicast control.
      return SL_STATUS_MESSAGE_TOO_LONG;
    }

    header = sli_zigbee_make_zigbee_multicast_header(message,
                                                     ((apsFrame->options
                                                       & SL_ZIGBEE_APS_OPTION_SOURCE_EUI64)
                                                      ? ZIGBEE_FRAME_CONTROL_SOURCE_IEEE_ADDRESS
                                                      : 0),
                                                     apsFrame->groupId,
                                                     radius);

    if (header == SL_ZIGBEE_NULL_MESSAGE_BUFFER) {
      return SL_STATUS_ALLOCATION_FAILED;
    }

    {
      uint8_t *frame = sli_zigbee_header_routing_frame(header);

      if (7 < broadcastAddr) {
        broadcastAddr = 7;      // 7 is infinity
      }
      sli_zigbee_multicast_control(frame) = sli_zigbee_multicast_control_func(broadcastAddr,
                                                                              broadcastAddr);
    }

    if (!sli_zigbee_add_aps_frame(&header,
                                  ZIGBEE_APS_FRAME_CONTROL_MODE_BROADCAST,
                                  apsFrame)) {
      sl_legacy_buffer_manager_release_message_buffer(header);
      return SL_STATUS_ALLOCATION_FAILED;
    }

    success = sli_zigbee_network_send_broadcast(header);
    sl_legacy_buffer_manager_release_message_buffer(header);

    return (success ? SL_STATUS_OK : SL_STATUS_BUSY);
  }
}
