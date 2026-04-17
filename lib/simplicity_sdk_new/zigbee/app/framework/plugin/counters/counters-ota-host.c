/***************************************************************************//**
 * @file
 * @brief An EZSP host application library for retrieving Ember stack
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
#include "app/util/common/common.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#include "counters-ota.h"

#if defined(OTA_COUNTERS_ENABLED)

static sl_status_t sendCluster(sl_802154_short_addr_t destination,
                               uint16_t clusterId,
                               uint8_t length,
                               uint8_t *contents)
{
  sl_zigbee_aps_frame_t apsFrame;
  uint8_t sequence;
  apsFrame.profileId = SL_ZIGBEE_PRIVATE_PROFILE_ID;
  apsFrame.clusterId = clusterId;
  apsFrame.sourceEndpoint = 0;
  apsFrame.destinationEndpoint = 0;
  apsFrame.options = (SL_ZIGBEE_APS_OPTION_RETRY
                      | SL_ZIGBEE_APS_OPTION_ENABLE_ROUTE_DISCOVERY);
  return sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                destination,
                                &apsFrame,
                                0, // message tag
                                length, // mesage length
                                contents,
                                &sequence);
}

sl_status_t sl_zigbee_send_counters_request(sl_802154_short_addr_t destination,
                                            bool clearCounters)
{
  return sendCluster(destination,
                     (clearCounters
                      ? SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_REQUEST
                      : SL_ZIGBEE_REPORT_COUNTERS_REQUEST),
                     0,
                     NULL);
}

bool sl_zigbee_is_incoming_counters_request(sl_zigbee_aps_frame_t *apsFrame, sl_802154_short_addr_t sender)
{
  uint8_t i;
  uint8_t length = 0;
  uint16_t counters[SL_ZIGBEE_COUNTER_TYPE_COUNT];
  uint8_t payload[MAX_PAYLOAD_LENGTH];

  if (apsFrame->profileId != SL_ZIGBEE_PRIVATE_PROFILE_ID
      || (apsFrame->clusterId != SL_ZIGBEE_REPORT_COUNTERS_REQUEST
          && apsFrame->clusterId != SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_REQUEST)) {
    return false;
  }

  sl_zigbee_ezsp_read_and_clear_counters(counters);

  for (i = 0; i < SL_ZIGBEE_COUNTER_TYPE_COUNT; i++) {
    uint16_t val = counters[i];
    if (val != 0) {
      payload[length] = i;
      payload[length + 1] = LOW_BYTE(val);
      payload[length + 2] = HIGH_BYTE(val);
      length += 3;
    }
    if (length + 3 > MAX_PAYLOAD_LENGTH
        || (length
            && (i + 1 == SL_ZIGBEE_COUNTER_TYPE_COUNT))) {
      // The response cluster is the request id with the high bit set.
      sendCluster(sender, apsFrame->clusterId | 0x8000, length, payload);
      length = 0;
    }
  }

  return true;
}

bool sl_zigbee_is_incoming_counters_response(sl_zigbee_aps_frame_t *apsFrame)
{
  return (apsFrame->profileId == SL_ZIGBEE_PRIVATE_PROFILE_ID
          && (apsFrame->clusterId == SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_RESPONSE
              || apsFrame->clusterId == SL_ZIGBEE_REPORT_COUNTERS_RESPONSE));
}

bool sl_zigbee_is_outgoing_counters_response(sl_zigbee_aps_frame_t *apsFrame,
                                             sl_status_t status)
{
  return sl_zigbee_is_incoming_counters_response(apsFrame);
}

#endif
