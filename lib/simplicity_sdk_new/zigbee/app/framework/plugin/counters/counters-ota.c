/***************************************************************************//**
 * @file
 * @brief A library for retrieving Ember stack counters over the air.
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
#include "counters.h"
#include "counters-ota.h"

//Include counters-ota if enabled by the plugin
#if defined(OTA_COUNTERS_ENABLED)

static sl_status_t sendCluster(sl_802154_short_addr_t destination,
                               uint16_t clusterId,
                               uint8_t payloadLength,
                               uint8_t *payload)
{
  sl_zigbee_aps_frame_t apsFrame;
  apsFrame.profileId = SL_ZIGBEE_PRIVATE_PROFILE_ID;
  apsFrame.clusterId = clusterId;
  apsFrame.sourceEndpoint = 0;
  apsFrame.destinationEndpoint = 0;
  apsFrame.options = (SL_ZIGBEE_APS_OPTION_RETRY
                      | SL_ZIGBEE_APS_OPTION_ENABLE_ROUTE_DISCOVERY);
  return sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                destination,
                                &apsFrame,
                                0x00, // tag
                                payloadLength,
                                payload,
                                NULL);
}

sl_status_t sl_zigbee_af_counters_send_request(sl_802154_short_addr_t destination,
                                               bool clearCounters)
{
  return sendCluster(destination,
                     (clearCounters
                      ? SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_REQUEST
                      : SL_ZIGBEE_REPORT_COUNTERS_REQUEST),
                     0,
                     NULL);
}

bool sl_zigbee_af_counters_is_incoming_request(sl_zigbee_aps_frame_t *apsFrame, sl_802154_short_addr_t sender)
{
  uint8_t reply[MAX_PAYLOAD_LENGTH];
  uint8_t length = 0;
  uint8_t i;

  if (apsFrame->profileId != SL_ZIGBEE_PRIVATE_PROFILE_ID
      || (apsFrame->clusterId != SL_ZIGBEE_REPORT_COUNTERS_REQUEST
          && apsFrame->clusterId != SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_REQUEST)) {
    return false;
  }

  for (i = 0; i < SL_ZIGBEE_COUNTER_TYPE_COUNT; i++) {
    uint16_t val = sl_zigbee_counters[i];

    if (val != 0) {
      reply[length] = i;
      reply[length + 1] = LOW_BYTE(val);
      reply[length + 2] = HIGH_BYTE(val);
      length += 3;
    }

    if (length + 3 > MAX_PAYLOAD_LENGTH
        || (length && (i + 1 == SL_ZIGBEE_COUNTER_TYPE_COUNT))) {
      // The response cluster is the request id with the high bit set.
      sendCluster(sender, apsFrame->clusterId | 0x8000, length, reply);
      length = 0;
    }
  }

  return true;
}

bool sl_zigbee_af_counters_is_incoming_response(sl_zigbee_aps_frame_t *apsFrame)
{
  return (apsFrame->profileId == SL_ZIGBEE_PRIVATE_PROFILE_ID
          && (apsFrame->clusterId == SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_RESPONSE
              || apsFrame->clusterId == SL_ZIGBEE_REPORT_COUNTERS_RESPONSE));
}

bool sl_zigbee_af_counters_is_outgoing_response(sl_zigbee_aps_frame_t *apsFrame,
                                                sl_status_t status)
{
  bool isResponse = sl_zigbee_is_incoming_counters_response(apsFrame);
  if (isResponse
      && apsFrame->clusterId == SL_ZIGBEE_REPORT_AND_CLEAR_COUNTERS_RESPONSE
      && status == SL_STATUS_OK) {
    sl_zigbee_clear_counters();
  }
  return isResponse;
}

#endif //OTA_COUNTERS_ENABLED
