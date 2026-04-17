/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "green-power-af-wrapper.h"

WEAK(uint16_t sl_zigbee_af_print_active_area) = 0;

#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_ATTRIBUTE_SYSTEM == 1)
WEAK(sl_zigbee_af_status_t sl_zigbee_af_read_attribute_wrapper(uint8_t endpoint,
                                                               sl_zigbee_af_cluster_id_t cluster,
                                                               sl_zigbee_af_attribute_id_t attributeID,
                                                               uint8_t mask,
                                                               uint8_t *dataPtr,
                                                               uint8_t readLength,
                                                               sl_zigbee_af_attribute_type_t *dataType))
{
  (void)endpoint;
  (void)cluster;
  (void)attributeID;
  (void)mask;
  (void)dataPtr;
  (void)readLength;
  (void)dataType;
  return SL_ZIGBEE_ZCL_STATUS_FAILURE;
}

WEAK(sl_zigbee_af_status_t sl_zigbee_af_write_attribute_wrapper(uint8_t endpoint,
                                                                sl_zigbee_af_cluster_id_t cluster,
                                                                sl_zigbee_af_attribute_id_t attributeID,
                                                                uint8_t mask,
                                                                uint8_t* dataPtr,
                                                                sl_zigbee_af_attribute_type_t dataType))
{
  (void)endpoint;
  (void)cluster;
  (void)attributeID;
  (void)mask;
  (void)dataPtr;
  (void)dataType;
  return SL_ZIGBEE_ZCL_STATUS_FAILURE;
}

WEAK(sl_zigbee_af_status_t sl_zigbee_af_write_client_attribute_wrapper(uint8_t endpoint,
                                                                       sl_zigbee_af_cluster_id_t cluster,
                                                                       sl_zigbee_af_attribute_id_t attributeID,
                                                                       uint8_t* dataPtr,
                                                                       sl_zigbee_af_attribute_type_t dataType))
{
  (void)endpoint;
  (void)cluster;
  (void)attributeID;
  (void)dataPtr;
  (void)dataType;
  return SL_ZIGBEE_ZCL_STATUS_FAILURE;
}

WEAK(bool sl_zigbee_af_contains_server_wrapper(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId))
{
  (void)endpoint;
  (void)clusterId;
  return false;
}

WEAK(bool sl_zigbee_af_contains_client_wrapper(uint8_t endpoint, sl_zigbee_af_cluster_id_t clusterId))
{
  (void)endpoint;
  (void)clusterId;
  return false;
}
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_ATTRIBUTE_SYSTEM == 1)

WEAK(uint8_t sl_zigbee_af_get_radio_channel_wrapper(void))
{
  return sl_zigbee_get_radio_channel();
}

#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_AF_INTERFACE == 1)
WEAK(sl_zigbee_aps_frame_t *sl_zigbee_af_get_command_aps_frame_wrapper(void))
{
  return NULL;
}

WEAK(sl_status_t sl_zigbee_af_send_command_unicast_wrapper(sl_zigbee_outgoing_message_type_t type,
                                                           uint16_t indexOrDestination))
{
  (void)type;
  (void)indexOrDestination;
  return SL_STATUS_NOT_AVAILABLE;
}

WEAK(sl_status_t sl_zigbee_af_send_command_broadcast_wrapper(sl_802154_short_addr_t destination,
                                                             sl_802154_short_addr_t alias,
                                                             uint8_t sequence))
{
  (void)destination;
  (void)alias;
  (void)sequence;
  return SL_STATUS_NOT_AVAILABLE;
}

WEAK(sl_status_t sl_zigbee_af_send_command_multicast_wrapper(sl_zigbee_multicast_id_t multicastId, sl_802154_short_addr_t alias, uint8_t sequence))
{
  (void)multicastId;
  (void)alias;
  (void)sequence;
  return SL_STATUS_NOT_AVAILABLE;
}

WEAK(uint16_t sl_zigbee_af_fill_external_buffer_wrapper(uint8_t frameControl,
                                                        sl_zigbee_af_cluster_id_t clusterId,
                                                        uint8_t commandId,
                                                        const char *format,
                                                        ...))
{
  (void)frameControl;
  (void)clusterId;
  (void)commandId;
  (void)format;
  return 0xFFFF;
}
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_ADAPTER_USE_CUSTOM_AF_INTERFACE == 1)

WEAK(void sl_zigbee_af_copy_int16u(uint8_t *data, uint16_t index, uint16_t x))
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFFu);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFFu);
}

WEAK(void sl_zigbee_af_copy_int24u(uint8_t *data, uint16_t index, uint32_t x))
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFFu);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFFu);
  data[index + 2] = (uint8_t) ( ((x) >> 16) & 0xFFu);
}

WEAK(void sl_zigbee_af_copy_int32u(uint8_t *data, uint16_t index, uint32_t x))
{
  data[index]   = (uint8_t) ( ((x)    ) & 0xFFu);
  data[index + 1] = (uint8_t) ( ((x) >> 8) & 0xFFu);
  data[index + 2] = (uint8_t) ( ((x) >> 16) & 0xFFu);
  data[index + 3] = (uint8_t) ( ((x) >> 24) & 0xFFu);
}

WEAK(void sl_zigbee_af_copy_string(uint8_t *dest, uint8_t *src, uint8_t size))
{
  if ( src == NULL ) {
    dest[0] = 0u; // Zero out the length of string
  } else if (src[0] == 0xFFu) {
    dest[0] = src[0];
  } else {
    uint8_t length = sl_zigbee_af_string_length(src);
    if (size < length) {
      length = size;
    }
    memmove(dest + 1, src + 1, length);
    dest[0] = length;
  }
}

WEAK(void sl_zigbee_af_copy_long_string(uint8_t *dest, uint8_t *src, uint16_t size))
{
  if ( src == NULL ) {
    dest[0] = dest[1] = 0u; // Zero out the length of string
  } else if ((src[0] == 0xFFu)
             && (src[1] == 0xFFu)) {
    dest[0] = 0xFFu;
    dest[1] = 0xFFu;
  } else {
    uint16_t length = sl_zigbee_af_long_string_length(src);
    if (size < length) {
      length = size;
    }
    memmove(dest + 2, src + 2, length);
    dest[0] = LOW_BYTE(length);
    dest[1] = HIGH_BYTE(length);
  }
}

#if (BIGENDIAN_CPU)
  #define EM_BIG_ENDIAN true
#else
  #define EM_BIG_ENDIAN false
#endif

// You can pass in val1 as NULL, which will assume that it is
// pointing to an array of all zeroes. This is used so that
// default value of NULL is treated as all zeroes.
WEAK(int8_t sl_zigbee_af_compare_values(uint8_t* val1,
                                        uint8_t* val2,
                                        uint8_t len,
                                        bool signedNumber))
{
  uint8_t i, j, k;

  SL_ZIGBEE_TEST_ASSERT(len > 0);

  if (signedNumber) { // signed number comparison
    if (len <= 4) { // only number with 32-bits or less is supported
      int32_t accum1 = 0x0;
      int32_t accum2 = 0x0;
      int32_t all1s = -1;

      for (i = 0; i < len; i++) {
        j = (val1 == NULL
             ? 0
             : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i])
             );
        accum1 |= j << (8 * (len - 1 - i));

        k = (EM_BIG_ENDIAN
             ? val2[i]
             : val2[(len - 1) - i]);
        accum2 |= k << (8 * (len - 1 - i));
      }

      // sign extending, no need for 32-bits numbers
      if (len < 4) {
        if ((accum1 & (1 << (8 * len - 1))) != 0) { // check sign
          accum1 |= all1s - ((1 << (len * 8)) - 1);
        }
        if ((accum2 & (1 << (8 * len - 1))) != 0) { // check sign
          accum2 |= all1s - ((1 << (len * 8)) - 1);
        }
      }

      if (accum1 > accum2) {
        return 1;
      } else if (accum1 < accum2) {
        return -1;
      } else {
        return 0;
      }
    } else { // not supported
      return 0;
    }
  } else { // regular unsigned number comparison
    for (i = 0; i < len; i++) {
      j = (val1 == NULL
           ? 0
           : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i])
           );
      k = (EM_BIG_ENDIAN
           ? val2[i]
           : val2[(len - 1) - i]);

      if (j > k) {
        return 1;
      } else if (k > j) {
        return -1;
      } else {
        // MISRA requires ..else if.. to have terminating else.
      }
    }
    return 0;
  }
}

// Zigbee spec says types between signed 8 bit and signed 64 bit
WEAK(bool sl_zigbee_af_is_type_signed(sl_zigbee_af_attribute_type_t dataType))
{
  return (dataType >= ZCL_INT8S_ATTRIBUTE_TYPE
          && dataType <= ZCL_INT64S_ATTRIBUTE_TYPE);
}

WEAK(bool sl_zigbee_af_groups_cluster_endpoint_in_group_cb(uint8_t endpoint,
                                                           uint16_t groupId))
{
  (void)endpoint;
  (void)groupId;
  return false;
}

WEAK(sl_802154_short_addr_t sl_zigbee_af_get_node_id(void))
{
  return sl_zigbee_get_node_id();
}

WEAK(void sl_zigbee_af_get_eui64(sl_802154_long_addr_t returnEui64))
{
  memmove(returnEui64, sl_zigbee_get_eui64(), EUI64_SIZE);
}

WEAK(sl_802154_pan_id_t sl_zigbee_af_get_pan_id(void))
{
  return sl_zigbee_get_pan_id();
}

bool sl_zigbee_af_is_current_security_profile_smart_energy(void)
{
  return false;
}

WEAK(uint8_t sl_zigbee_af_get_open_network_duration_sec(void))
{
  return 0;
}

WEAK(bool sli_zigbee_af_service_discovery_incoming(sl_802154_short_addr_t sender,
                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                   const uint8_t *message,
                                                   uint16_t length))
{
  (void)sender;
  (void)apsFrame;
  (void)message;
  (void)length;
  return false;
}
WEAK(sl_status_t zaTrustCenterSecurityInit(bool centralizedNetwork))
{
  (void)centralizedNetwork;
  return 0;
}
WEAK(sl_status_t zaNodeSecurityInit(bool centralizedNetwork))
{
  (void)centralizedNetwork;
  return 0;
}

WEAK(sl_status_t zaTrustCenterSecurityPolicyInit(void))
{
  return 0;
}
WEAK(void sl_zigbee_af_get_mfg_string(uint8_t* returnData))
{
  (void)returnData;
}

WEAK(sl_status_t sl_zigbee_af_get_node_type(sl_zigbee_node_type_t *nodeType))
{
  return sl_zigbee_get_network_parameters(nodeType, NULL);
}

WEAK(sl_status_t sl_zigbee_send_zig_dev_request(sl_802154_short_addr_t destination,
                                                uint16_t clusterId,
                                                sl_zigbee_aps_option_t options,
                                                uint8_t *contents,
                                                uint8_t length))
{
  (void)destination;
  (void)clusterId;
  (void)options;
  (void)contents;
  (void)length;
  return 0;
}

WEAK(sl_status_t sli_zigbee_af_send(sl_zigbee_outgoing_message_type_t type,
                                    uint16_t indexOrDestination,
                                    sl_zigbee_aps_frame_t *apsFrame,
                                    uint8_t messageLength,
                                    uint8_t *message,
                                    uint16_t *messageTag,
                                    sl_802154_short_addr_t alias,
                                    uint8_t sequence))
{
  (void)type;
  (void)indexOrDestination;
  (void)apsFrame;
  (void)messageLength;
  (void)message;
  (void)messageTag;
  (void)alias;
  (void)sequence;
  return 0;
}

WEAK(void sl_zigbee_af_print(uint16_t area, const char * formatString, ...))
{
  (void)area;
  (void)formatString;
}

WEAK(bool sl_zigbee_af_print_enabled(uint16_t area))
{
  (void)area;
  return false;
}

WEAK(void sl_zigbee_af_print_buffer(uint16_t area,
                                    const uint8_t *buffer,
                                    uint16_t bufferLen,
                                    bool withSpace))
{
  (void)area;
  (void)buffer;
  (void)bufferLen;
  (void)withSpace;
}
WEAK(void sl_zigbee_af_println(uint16_t functionality, const char * formatString, ...))
{
  (void)functionality;
  (void)formatString;
}

WEAK(bool sl_zigbee_af_memory_byte_compare(const uint8_t *bytes, uint8_t count, uint8_t target))
{
  uint8_t i;
  for (i = 0; i < count; i++) {
    if (*bytes != target) {
      return false;
    }
    bytes++;
  }
  return true;
}
