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
#include "stack/include/sl_zigbee_types.h"
#include "hal/hal.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#include "zigbee-device-common.h"
#include "zigbee-device-host.h"

static uint8_t zigDevRequestBuffer[EZSP_MAX_FRAME_LENGTH];

static sl_status_t sendZigDevRequestBuffer(sl_802154_short_addr_t target,
                                           uint16_t clusterId,
                                           sl_zigbee_aps_option_t options,
                                           uint8_t length);

sl_status_t sl_zigbee_network_address_request(sl_802154_long_addr_t target,
                                              bool reportKids,
                                              uint8_t childStartIndex)
{
  uint8_t *payload = zigDevRequestBuffer + ZDO_MESSAGE_OVERHEAD;
  memmove(payload, target, EUI64_SIZE);
  payload[8] = reportKids ? 1 : 0;
  payload[9] = childStartIndex;
  return sendZigDevRequestBuffer(SL_ZIGBEE_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                 NETWORK_ADDRESS_REQUEST,
                                 SL_ZIGBEE_APS_OPTION_SOURCE_EUI64,
                                 ZDO_MESSAGE_OVERHEAD + 10);
}

sl_status_t sl_zigbee_ieee_address_request(sl_802154_short_addr_t target,
                                           bool reportKids,
                                           uint8_t childStartIndex,
                                           sl_zigbee_aps_option_t options)
{
  uint8_t *payload = zigDevRequestBuffer + ZDO_MESSAGE_OVERHEAD;
  payload[0] = LOW_BYTE(target);
  payload[1] = HIGH_BYTE(target);
  payload[2] = reportKids ? 1 : 0;
  payload[3] = childStartIndex;
  return sendZigDevRequestBuffer(target,
                                 IEEE_ADDRESS_REQUEST,
                                 options,
                                 ZDO_MESSAGE_OVERHEAD + 4);
}

sl_status_t sl_zigbee_ieee_address_request_to_target(sl_802154_short_addr_t discoveryNodeId,
                                                     bool reportKids,
                                                     uint8_t childStartIndex,
                                                     sl_zigbee_aps_option_t options,
                                                     sl_802154_short_addr_t targetNodeIdOfRequest)
{
  uint8_t *payload = zigDevRequestBuffer + ZDO_MESSAGE_OVERHEAD;
  payload[0] = LOW_BYTE(discoveryNodeId);
  payload[1] = HIGH_BYTE(discoveryNodeId);
  payload[2] = reportKids ? 1 : 0;
  payload[3] = childStartIndex;
  return sendZigDevRequestBuffer(targetNodeIdOfRequest,
                                 IEEE_ADDRESS_REQUEST,
                                 options,
                                 ZDO_MESSAGE_OVERHEAD + 4);
}

sl_status_t sl_zigbee_ezsp_match_descriptors_request(sl_802154_short_addr_t target,
                                                     uint16_t profile,
                                                     uint8_t inCount,
                                                     uint8_t outCount,
                                                     uint16_t *inClusters,
                                                     uint16_t *outClusters,
                                                     sl_zigbee_aps_option_t options)
{
  uint8_t i;
  uint8_t *payload = zigDevRequestBuffer + ZDO_MESSAGE_OVERHEAD;
  uint8_t offset = ZDO_MESSAGE_OVERHEAD + 5;   // Add 2 bytes for NWK Address
                                               // Add 2 bytes for Profile Id
                                               // Add 1 byte for in Cluster Count
  uint8_t length = (offset
                    + (inCount * 2) // Times 2 for 2 byte Clusters
                    + 1           // Out Cluster Count
                    + (outCount * 2)); // Times 2 for 2 byte Clusters

  if (length > EZSP_MAX_FRAME_LENGTH) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  payload[0] = LOW_BYTE(target);
  payload[1] = HIGH_BYTE(target);
  payload[2] = LOW_BYTE(profile);
  payload[3] = HIGH_BYTE(profile);
  payload[4] = inCount;

  for (i = 0; i < inCount; i++) {
    zigDevRequestBuffer[(i * 2) + offset] = LOW_BYTE(inClusters[i]);
    zigDevRequestBuffer[(i * 2) + offset + 1] = HIGH_BYTE(inClusters[i]);
  }
  offset += (inCount * 2);
  zigDevRequestBuffer[offset] = outCount;
  offset++;
  for (i = 0; i < outCount; i++) {
    zigDevRequestBuffer[(i * 2) + offset] = LOW_BYTE(outClusters[i]);
    zigDevRequestBuffer[(i * 2) + offset + 1] = HIGH_BYTE(outClusters[i]);
  }

  return sendZigDevRequestBuffer(target,
                                 MATCH_DESCRIPTORS_REQUEST,
                                 options,
                                 length);
}

sl_status_t sl_zigbee_send_zig_dev_request(sl_802154_short_addr_t destination,
                                           uint16_t clusterId,
                                           sl_zigbee_aps_option_t options,
                                           uint8_t *contents,
                                           uint8_t length)
{
  sl_status_t result;

  if (length > EZSP_MAX_FRAME_LENGTH) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  memmove(zigDevRequestBuffer, contents, length);

  result = sendZigDevRequestBuffer(destination,
                                   clusterId,
                                   options,
                                   length);
  return result;
}

static sl_status_t sendZigDevRequestBuffer(sl_802154_short_addr_t destination,
                                           uint16_t clusterId,
                                           sl_zigbee_aps_option_t options,
                                           uint8_t length)
{
  sl_zigbee_aps_frame_t apsFrame;
  uint8_t sequence = sl_zigbee_next_zig_dev_request_sequence();
  zigDevRequestBuffer[0] = sequence;

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
                                    sequence,
                                    length,
                                    zigDevRequestBuffer,
                                    &apsFrame.sequence);
  } else {
    return sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                  destination,
                                  &apsFrame,
                                  sequence,
                                  length,
                                  zigDevRequestBuffer,
                                  &apsFrame.sequence);
  }
}

uint16_t sl_zigbee_get_pseudo_random_number(void)
{
  return halCommonGetRandom();
}
