/***************************************************************************//**
 * @file
 * @brief ZigBee Device Object (ZDO) functions not provided by the stack.
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

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee.h"
#include "hal/hal.h"
#include "zigbee-device-common.h"
#include "zigbee-device-library.h"

//aligns SoC with the host implementation of sl_zigbee_match_descriptors_request
#define MAX_ZIG_DEV_REQUEST_LENGTH 202

static sl_status_t sendZigDevRequestBuffer(sl_802154_short_addr_t destination,
                                           uint16_t clusterId,
                                           sl_zigbee_aps_option_t options,
                                           uint8_t requestLength,
                                           uint8_t *requestContents);

sl_status_t sl_zigbee_match_descriptors_request(sl_802154_short_addr_t target,
                                                uint16_t profile,
                                                uint8_t inCount,
                                                uint16_t* inClusters,
                                                uint8_t outCount,
                                                uint16_t* outClusters,
                                                sl_zigbee_aps_option_t options)
{
  uint8_t zig_dev_request_buffer[MAX_ZIG_DEV_REQUEST_LENGTH];
  uint8_t offset = ZDO_MESSAGE_OVERHEAD + 5;
  uint8_t contents[ZDO_MESSAGE_OVERHEAD + 5];  // Add 2 bytes for NWK Address
                                               // Add 2 bytes for Profile Id
                                               // Add 1 byte for in Cluster Count

  sl_status_t result;
  uint8_t i;

  contents[0] = sl_zigbee_next_zig_dev_request_sequence();
  contents[1] = LOW_BYTE(target);
  contents[2] = HIGH_BYTE(target);
  contents[3] = LOW_BYTE(profile);
  contents[4] = HIGH_BYTE(profile);
  contents[5] = inCount;

  memmove(&zig_dev_request_buffer, contents, offset);

  for (i = 0; i < inCount; i++) {
    zig_dev_request_buffer[(i * 2) + offset] = LOW_BYTE(inClusters[i]);
    zig_dev_request_buffer[(i * 2) + offset + 1] = HIGH_BYTE(inClusters[i]);
  }
  offset += (inCount * 2);
  zig_dev_request_buffer[offset] = outCount;
  offset++;
  for (i = 0; i < outCount; i++) {
    zig_dev_request_buffer[(i * 2) + offset] = LOW_BYTE(outClusters[i]);
    zig_dev_request_buffer[(i * 2) + offset + 1] = HIGH_BYTE(outClusters[i]);
  }
  offset += (outCount * 2);

  result = sendZigDevRequestBuffer(target,
                                   MATCH_DESCRIPTORS_REQUEST,
                                   options,
                                   offset,
                                   (uint8_t*)zig_dev_request_buffer);
  return result;
}

sl_status_t sl_zigbee_send_zig_dev_request(sl_802154_short_addr_t destination,
                                           uint16_t clusterId,
                                           sl_zigbee_aps_option_t options,
                                           uint8_t *contents,
                                           uint8_t length)
{
  contents[0] = sl_zigbee_next_zig_dev_request_sequence();

  return sendZigDevRequestBuffer(destination,
                                 clusterId,
                                 options,
                                 length,
                                 contents);
}

static sl_status_t sendZigDevRequestBuffer(sl_802154_short_addr_t destination,
                                           uint16_t clusterId,
                                           sl_zigbee_aps_option_t options,
                                           uint8_t requestLength,
                                           uint8_t *requestContents)
{
  sl_zigbee_aps_frame_t apsFrame;

  apsFrame.sourceEndpoint = SL_ZIGBEE_ZDO_ENDPOINT;
  apsFrame.destinationEndpoint = SL_ZIGBEE_ZDO_ENDPOINT;
  apsFrame.clusterId = clusterId;
  apsFrame.profileId = SL_ZIGBEE_ZDO_PROFILE_ID;
  apsFrame.options = options;

  if (destination == SL_ZIGBEE_BROADCAST_ADDRESS
      || destination == SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS
      || destination == SL_ZIGBEE_SLEEPY_BROADCAST_ADDRESS) {
    return sl_zigbee_send_broadcast(SL_ZIGBEE_NULL_NODE_ID,
                                    destination,
                                    0,
                                    &apsFrame,
                                    sl_zigbee_get_zig_dev_request_radius(),
                                    0x00, // tag
                                    requestLength,
                                    requestContents,
                                    NULL);
  } else {
    return sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                  destination,
                                  &apsFrame,
                                  0x00, // tag
                                  requestLength,
                                  requestContents,
                                  NULL);
  }
}
