/***************************************************************************//**
 * @file
 * @brief ZigBee Device Object (ZDO) functions available on all platforms.
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
#include "zigbee-device-common.h"
#include "stack/include/stack-info.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT
#include "sl_zigbee_stack_specific_tlv.h"
#endif // SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT

static uint8_t zigDevRequestSequence = 0;
uint8_t zigDevRequestRadius = 255;

// Application generated ZDO messages use sequence numbers 0-127, and the stack
// uses sequence numbers 128-255.  This simplifies life by eliminating the need
// for coordination between the two entities, and allows both to send ZDO
// messages with non-conflicting sequence numbers.
#define APPLICATION_ZDO_SEQUENCE_MASK (0x7F)

uint8_t sl_zigbee_get_last_app_zig_dev_request_sequence(void)
{
  return (zigDevRequestSequence & APPLICATION_ZDO_SEQUENCE_MASK);
}

uint8_t sl_zigbee_next_zig_dev_request_sequence(void)
{
  return ((++zigDevRequestSequence) & APPLICATION_ZDO_SEQUENCE_MASK);
}

sl_status_t sl_zigbee_send_zig_dev_request_target(sl_802154_short_addr_t target,
                                                  uint16_t clusterId,
                                                  sl_zigbee_aps_option_t options)
{
  uint8_t contents[ZDO_MESSAGE_OVERHEAD + 2];
  uint8_t *payload = contents + ZDO_MESSAGE_OVERHEAD;
  payload[0] = LOW_BYTE(target);
  payload[1] = HIGH_BYTE(target);
  return sl_zigbee_send_zig_dev_request(target,
                                        clusterId,
                                        options,
                                        contents,
                                        sizeof(contents));
}

sl_status_t sl_zigbee_simple_descriptor_request(sl_802154_short_addr_t target,
                                                uint8_t targetEndpoint,
                                                sl_zigbee_aps_option_t options)
{
  uint8_t contents[ZDO_MESSAGE_OVERHEAD + 3];
  uint8_t *payload = contents + ZDO_MESSAGE_OVERHEAD;
  payload[0] = LOW_BYTE(target);
  payload[1] = HIGH_BYTE(target);
  payload[2] = targetEndpoint;
  return sl_zigbee_send_zig_dev_request(target,
                                        SIMPLE_DESCRIPTOR_REQUEST,
                                        options,
                                        contents,
                                        sizeof(contents));
}

sl_status_t sl_zigbee_send_zig_dev_bind_request(sl_802154_short_addr_t target,
                                                uint16_t bindClusterId,
                                                sl_802154_long_addr_t source,
                                                uint8_t sourceEndpoint,
                                                uint16_t clusterId,
                                                uint8_t type,
                                                sl_802154_long_addr_t destination,
                                                sl_zigbee_multicast_id_t groupAddress,
                                                uint8_t destinationEndpoint,
                                                sl_zigbee_aps_option_t options)
{
  uint8_t contents[ZDO_MESSAGE_OVERHEAD + 21];
  uint8_t *payload = contents + ZDO_MESSAGE_OVERHEAD;
  uint8_t length;
  memmove(payload, source, 8);
  payload[8] = sourceEndpoint;
  payload[9] = LOW_BYTE(clusterId);
  payload[10] = HIGH_BYTE(clusterId);
  payload[11] = type;
  switch (type) {
    case UNICAST_BINDING:
      memmove(payload + 12, destination, 8);
      payload[20] = destinationEndpoint;
      length = ZDO_MESSAGE_OVERHEAD + 21;
      break;
    case MULTICAST_BINDING:
      payload[12] = LOW_BYTE(groupAddress);
      payload[13] = HIGH_BYTE(groupAddress);
      length = ZDO_MESSAGE_OVERHEAD + 14;
      break;
    default:
      return SL_STATUS_FAIL;
  }
  return sl_zigbee_send_zig_dev_request(target,
                                        bindClusterId,
                                        options,
                                        contents,
                                        length);
}

#define sl_zigbee_lqi_table_request(target, startIndex, options) \
  (sl_zigbee_table_request(LQI_TABLE_REQUEST, (target), (startIndex), (options)))
#define sl_zigbee_routing_table_request(target, startIndex, options) \
  (sl_zigbee_table_request(ROUTING_TABLE_REQUEST, (target), (startIndex), (options)))
#define sl_zigbee_binding_table_request(target, startIndex, options) \
  (sl_zigbee_table_request(BINDING_TABLE_REQUEST, (target), (startIndex), (options)))

sl_status_t sl_zigbee_table_request(uint16_t clusterId,
                                    sl_802154_short_addr_t target,
                                    uint8_t startIndex,
                                    sl_zigbee_aps_option_t options)
{
  uint8_t contents[ZDO_MESSAGE_OVERHEAD + 1];
  contents[ZDO_MESSAGE_OVERHEAD] = startIndex;
  return sl_zigbee_send_zig_dev_request(target,
                                        clusterId,
                                        options,
                                        contents,
                                        sizeof(contents));
}

sl_status_t sl_zigbee_leave_request(sl_802154_short_addr_t target,
                                    sl_802154_long_addr_t deviceAddress,
                                    uint8_t leaveRequestFlags,
                                    sl_zigbee_aps_option_t options)
{
  uint8_t contents[ZDO_MESSAGE_OVERHEAD + 9];
  memmove(contents + ZDO_MESSAGE_OVERHEAD, deviceAddress, 8);
  contents[ZDO_MESSAGE_OVERHEAD + 8] = leaveRequestFlags;
  return sl_zigbee_send_zig_dev_request(target,
                                        LEAVE_REQUEST,
                                        options,
                                        contents,
                                        sizeof(contents));
}

#define PERMIT_JOIN_REQUEST_MIN_LENGTH ZDO_MESSAGE_OVERHEAD + 2 // sequence number, duration, tc significance
#ifdef SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT
#define PERMIT_JOIN_REQUEST_SIZE (PERMIT_JOIN_REQUEST_MIN_LENGTH + SL_ZIGBEE_GLOBAL_TLV_BEACON_APPENDIX_ENCAP_MAX_LEN) // sequence number, duration, tc significance, beacon appendix
#else
#define PERMIT_JOIN_REQUEST_SIZE (PERMIT_JOIN_REQUEST_MIN_LENGTH) // sequence number, duration, tc significance
#endif // SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT

sl_status_t sl_zigbee_permit_joining_request(sl_802154_short_addr_t target,
                                             uint8_t duration,
                                             uint8_t authentication,
                                             sl_zigbee_aps_option_t options)
{
  uint8_t contents[PERMIT_JOIN_REQUEST_SIZE] = { 0, };
  uint8_t length = PERMIT_JOIN_REQUEST_MIN_LENGTH;
  contents[ZDO_MESSAGE_OVERHEAD] = duration;
  contents[ZDO_MESSAGE_OVERHEAD + 1] = authentication;
  #ifdef SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT
  if (sl_zigbee_get_stack_compliance_revision() >= R23_COMPLIANCE_REVISION) {
    // add the beacon appendix
    sl_zigbee_global_tlv_beacon_appendix_encap_t appendix_tlv = { 0 };
    sl_status_t status = sl_zigbee_global_tlv_beacon_appendix_encapsulation(&appendix_tlv);
    if (status != SL_STATUS_OK) {
      return status;
    }
    // update the length
    sl_zigbee_tlv_chain chain = { 0, };
    sl_zigbee_tlv_initialize_empty_chain(&chain, contents + length, sizeof(contents) - length);
    status = sl_zigbee_tlv_chain_add_tlv(&chain, (sl_zigbee_tlv_t *) &appendix_tlv);
    if (status != SL_STATUS_OK) {
      return status;
    }
    length += chain.length;
  }
  #endif // SL_CATALOG_ZIGBEE_R23_SUPPORT_PRESENT
  return sl_zigbee_send_zig_dev_request(target,
                                        PERMIT_JOINING_REQUEST,
                                        options,
                                        contents,
                                        length);
}

sl_802154_short_addr_t sl_zigbee_decode_address_response(uint8_t responseLength,
                                                         uint8_t *response,
                                                         sl_802154_long_addr_t eui64)
{
  if (responseLength < ZDO_MESSAGE_OVERHEAD + 11) {
    return SL_ZIGBEE_NULL_NODE_ID;
  }

  if (response[ZDO_MESSAGE_OVERHEAD] == SL_ZIGBEE_ZDP_SUCCESS) {
    memmove(eui64, response + ZDO_MESSAGE_OVERHEAD + 1, EUI64_SIZE);
    return HIGH_LOW_TO_INT(response[ZDO_MESSAGE_OVERHEAD + 10], response[ZDO_MESSAGE_OVERHEAD + 9]);
  } else {
    return SL_ZIGBEE_NULL_NODE_ID;
  }
}
