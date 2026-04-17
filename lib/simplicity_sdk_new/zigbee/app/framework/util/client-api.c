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

#include PLATFORM_HEADER
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"
#include "client-api.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
uint8_t *sli_zigbee_af_zcl_buffer = NULL;
uint16_t sli_zigbee_af_zcl_buffer_len = 0;

// Pointer to where this API should put the length
uint16_t *sli_zigbee_af_response_length_ptr = NULL;

// Pointer to where the API should put the cluster ID
sl_zigbee_aps_frame_t *sli_zigbee_af_command_aps_frame = NULL;

/////////////////

// Method that fills in the buffer.
// It returns number of bytes filled
// and 0 on error.
static uint16_t vFillBuffer(uint8_t *buffer,
                            uint16_t bufferLen,
                            uint8_t frameControl,
                            uint16_t manufacturerCode,
                            uint8_t commandId,
                            const char * format,
                            va_list argPointer)
{
  uint32_t value;
  uint8_t valueLen;
  uint8_t *data;
  uint16_t dataLen;
  uint8_t i;
  uint16_t bytes = 0;

  // The destination buffer must be at least large enough to hold the ZCL
  // overhead: frame control, manufacturer code (if applicable), sequence
  // number, and command id.  If it is, add these in order.
  if (bufferLen < SL_ZIGBEE_AF_ZCL_OVERHEAD
      || (manufacturerCode != SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE
          && bufferLen < SL_ZIGBEE_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD)) {
    sl_zigbee_af_debug_println("ERR: Buffer too short for ZCL header");
    return 0;
  }
  if (manufacturerCode != SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) {
    frameControl |= ZCL_MANUFACTURER_SPECIFIC_MASK;
  }
  buffer[bytes++] = frameControl;
  if (manufacturerCode != SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) {
    buffer[bytes++] = LOW_BYTE(manufacturerCode);
    buffer[bytes++] = HIGH_BYTE(manufacturerCode);
  }
  buffer[bytes++] = sl_zigbee_af_next_sequence();
  buffer[bytes++] = commandId;

  // Each argument comes in as an integer value, a pointer to a buffer, or a
  // pointer to a buffer followed by an integer length.
  for (i = 0; format[i] != 0; i++) {
    char cmd;
    value = 0;
    valueLen = 0;
    data = 0;
    cmd = format[i];
    if (cmd <= 's') {
      //  0--9, A--G, L, S, b, l, and s all have a pointer to a buffer.  The
      // length of that buffer is implied by 0--9 and A--G (0 to 16 bytes).
      // For L, S, and b, a separate integer specifies the length.  That length
      // will precede the data in the destination buffer for L and S, which
      // turns them into regular ZigBee strings.  In this case, care must be
      // taken to account for invalid strings, which have length 0xFFFF or 0xFF
      // for L and S respectively.  In the case of invalid strings, the length
      // byte(s) are copied to the destination buffer but the string itself is
      // not.  Finally, l and s are just ZigBee strings and the length of the
      // string data is contained within the buffer itself and the entire
      // buffer is copied as is to the destination buffer.  Note that
      // emberAf(Long)StringLength handles invalid strings, but it does not
      // include the length byte(s) in the total length, so the overhead
      // must be maually accounted for when copying.
      data = (uint8_t *)va_arg(argPointer, uint8_t *);
      valueLen = 0;
      if (cmd == 'L' || cmd == 'S' || cmd == 'b') {
        dataLen = (uint16_t)va_arg(argPointer, int);
        if (cmd == 'L') {
          value = dataLen;
          valueLen = (value == 0xFFFF ? 0 : 2);
        } else if (cmd == 'S') {
          value = (uint8_t)dataLen;
          valueLen = (value == 0xFF ? 0 : 1);
        } else {
          // MISRA requires ..else if.. to have terminating else.
        }
      } else if (cmd == 'l') {
        dataLen = sl_zigbee_af_long_string_length(data) + 2;
      } else if (cmd == 's') {
        dataLen = sl_zigbee_af_string_length(data) + 1;
      } else if ('0' <= cmd && cmd <= '9') {
        dataLen = cmd - '0';
      } else if ('A' <= cmd && cmd <= 'G') {
        dataLen = cmd - 'A' + 10;
      } else {
        sl_zigbee_af_debug_println("ERR: Unknown format '%c'", cmd);
        return 0;
      }
    } else {
      // u, v, x, and w are one-, two-, three-, or four-byte integers.  u and v
      // must be extracted as an int while x and w come through as an uint32_t.
      // In all cases, the value is copied to the destination buffer in little-
      // endian format.
      dataLen = 0;
      if (cmd == 'u') {
        valueLen = 1;
      } else if (cmd == 'v') {
        valueLen = 2;
      } else if (cmd == 'x') {
        valueLen = 3;
      } else if (cmd == 'w') {
        valueLen = 4;
      } else {
        sl_zigbee_af_debug_println("ERR: Unknown format '%c'", cmd);
        return 0;
      }
      value = (uint32_t)(valueLen <= 2
                         ? (uint32_t)va_arg(argPointer, int)
                         : va_arg(argPointer, uint32_t));
    }

    // The destination buffer must be at least as large as the running total
    // plus the length of the integer value (if applicable) plus the length of
    // the data (if applicable).
    if (bufferLen < bytes + dataLen + valueLen) {
      sl_zigbee_af_debug_println("ERR: Buffer too short for %d bytes for format '%c'",
                                 dataLen + valueLen,
                                 cmd);
      return 0;
    }

    // If there is an integer value, add it to destination buffer in little-
    // endian format.
    for (; 0 < valueLen; valueLen--) {
      buffer[bytes++] = LOW_BYTE(value);
      value = value >> 8;
    }

    // Finally, if there is data, add it to the destination buffer as is.  If
    // the data length is zero, data may actually be NULL.  Even if the length
    // argument is zero, passing NULL as either the source or destination to
    // memcpy is invalid and the behavior is undefined.  We avoid that with an
    // explicit check.
    if (dataLen != 0) {
      if (data == NULL) {
        sl_zigbee_af_debug_println("ERR: Missing data for %d bytes for format '%c'",
                                   dataLen,
                                   cmd);
        return 0;
      }
      memcpy(buffer + bytes, data, dataLen);
      bytes += dataLen;
    }
  }

  return bytes;
}

////////////////////// Public API ////////////////////////

void sl_zigbee_af_set_external_buffer(uint8_t *buffer,
                                      uint16_t bufferLen,
                                      uint16_t *lenPtr,
                                      sl_zigbee_aps_frame_t *apsFrame)
{
#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_acquire_lock();
#endif
  sli_zigbee_af_zcl_buffer = buffer;
  sli_zigbee_af_zcl_buffer_len = bufferLen;
  sli_zigbee_af_response_length_ptr = lenPtr;
  sli_zigbee_af_command_aps_frame = apsFrame;
#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_release_lock();
#endif
}

uint16_t sl_zigbee_af_fill_external_manufacturer_specific_buffer(uint8_t frameControl,
                                                                 sl_zigbee_af_cluster_id_t clusterId,
                                                                 uint16_t manufacturerCode,
                                                                 uint8_t commandId,
                                                                 const char * format,
                                                                 ...)
{
  uint16_t returnValue;
  va_list argPointer = { 0 };

#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_acquire_lock();
#endif
  va_start(argPointer, format);
  returnValue = vFillBuffer(sli_zigbee_af_zcl_buffer,
                            sli_zigbee_af_zcl_buffer_len,
                            frameControl,
                            manufacturerCode,
                            commandId,
                            format,
                            argPointer);
  va_end(argPointer);
  *sli_zigbee_af_response_length_ptr = returnValue;
  sli_zigbee_af_command_aps_frame->clusterId = clusterId;
  sli_zigbee_af_command_aps_frame->options = SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS;
#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_release_lock();
#endif
  return returnValue;
}

uint16_t sl_zigbee_af_fill_external_buffer(uint8_t frameControl,
                                           sl_zigbee_af_cluster_id_t clusterId,
                                           uint8_t commandId,
                                           const char * format,
                                           ...)
{
  uint16_t returnValue;
  va_list argPointer = { 0 };

#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_acquire_lock();
#endif
  va_start(argPointer, format);
  returnValue = vFillBuffer(sli_zigbee_af_zcl_buffer,
                            sli_zigbee_af_zcl_buffer_len,
                            frameControl,
                            SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                            commandId,
                            format,
                            argPointer);
  va_end(argPointer);
  *sli_zigbee_af_response_length_ptr = returnValue;
  sli_zigbee_af_command_aps_frame->clusterId = clusterId;
  sli_zigbee_af_command_aps_frame->options = SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS;
#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_release_lock();
#endif
  return returnValue;
}

uint16_t sl_zigbee_af_fill_buffer(uint8_t *buffer,
                                  uint16_t bufferLen,
                                  uint8_t frameControl,
                                  uint8_t commandId,
                                  const char * format,
                                  ...)
{
  uint16_t returnValue;
  va_list argPointer = { 0 };
  va_start(argPointer, format);
  returnValue = vFillBuffer(buffer,
                            bufferLen,
                            frameControl,
                            SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
                            commandId,
                            format,
                            argPointer);
  va_end(argPointer);
  return returnValue;
}

sl_status_t sl_zigbee_af_send_command_unicast_to_bindings_with_cb(sl_zigbee_af_message_sent_function_t callback)
{
  return sl_zigbee_af_send_unicast_to_bindings_with_cb(sli_zigbee_af_command_aps_frame,
                                                       *sli_zigbee_af_response_length_ptr,
                                                       sli_zigbee_af_zcl_buffer,
                                                       callback);
}

sl_status_t sl_zigbee_af_send_command_unicast_to_bindings(void)
{
  return sl_zigbee_af_send_command_unicast_to_bindings_with_cb(NULL);
}

sl_status_t sl_zigbee_af_send_command_multicast_with_cb(sl_zigbee_multicast_id_t multicastId, sl_802154_short_addr_t alias, uint8_t sequence, sl_zigbee_af_message_sent_function_t callback)
{
  return sl_zigbee_af_send_multicast_with_cb(multicastId,
                                             alias,
                                             sequence,
                                             sli_zigbee_af_command_aps_frame,
                                             *sli_zigbee_af_response_length_ptr,
                                             sli_zigbee_af_zcl_buffer,
                                             callback);
}

sl_status_t sl_zigbee_af_send_command_multicast(sl_zigbee_multicast_id_t multicastId, sl_802154_short_addr_t alias, uint8_t sequence)
{
  return sl_zigbee_af_send_command_multicast_with_cb(multicastId, alias, sequence, NULL);
}

sl_status_t sl_zigbee_af_send_command_multicast_to_bindings(void)
{
  return sl_zigbee_af_send_multicast_to_bindings(sli_zigbee_af_command_aps_frame,
                                                 *sli_zigbee_af_response_length_ptr,
                                                 sli_zigbee_af_zcl_buffer);
}

sl_status_t sl_zigbee_af_send_command_unicast_with_cb(sl_zigbee_outgoing_message_type_t type,
                                                      uint16_t indexOrDestination,
                                                      sl_zigbee_af_message_sent_function_t callback)
{
  return sl_zigbee_af_send_unicast_with_cb(type,
                                           indexOrDestination,
                                           sli_zigbee_af_command_aps_frame,
                                           *sli_zigbee_af_response_length_ptr,
                                           sli_zigbee_af_zcl_buffer,
                                           callback);
}

sl_status_t sl_zigbee_af_send_command_unicast(sl_zigbee_outgoing_message_type_t type,
                                              uint16_t indexOrDestination)
{
  return sl_zigbee_af_send_command_unicast_with_cb(type, indexOrDestination, NULL);
}

sl_status_t sl_zigbee_af_send_command_broadcast_with_cb(sl_802154_short_addr_t destination,
                                                        sl_802154_short_addr_t alias,
                                                        uint8_t sequence,
                                                        sl_zigbee_af_message_sent_function_t callback)
{
  return sl_zigbee_af_send_broadcast_with_cb(destination,
                                             sli_zigbee_af_command_aps_frame,
                                             *sli_zigbee_af_response_length_ptr,
                                             sli_zigbee_af_zcl_buffer,
                                             alias,
                                             sequence,
                                             callback);
}

sl_status_t sl_zigbee_af_send_command_broadcast(sl_802154_short_addr_t destination,
                                                sl_802154_short_addr_t alias,
                                                uint8_t sequence)
{
  return sl_zigbee_af_send_command_broadcast_with_cb(destination, alias, sequence, NULL);
}

sl_status_t sl_zigbee_af_send_command_inter_pan(sl_802154_pan_id_t panId,
                                                const sl_802154_long_addr_t destinationLongId,
                                                sl_802154_short_addr_t destinationShortId,
                                                sl_zigbee_multicast_id_t multicastId,
                                                sl_zigbee_af_profile_id_t profileId)
{
  return sl_zigbee_af_send_inter_pan(panId,
                                     destinationLongId,
                                     destinationShortId,
                                     multicastId,
                                     sli_zigbee_af_command_aps_frame->clusterId,
                                     profileId,
                                     *sli_zigbee_af_response_length_ptr,
                                     sli_zigbee_af_zcl_buffer);
}

sl_zigbee_aps_frame_t *sl_zigbee_af_get_command_aps_frame(void)
{
  return sli_zigbee_af_command_aps_frame;
}

void sl_zigbee_af_set_command_endpoints(uint8_t sourceEndpoint, uint8_t destinationEndpoint)
{
#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_acquire_lock();
#endif
  sli_zigbee_af_command_aps_frame->sourceEndpoint = sourceEndpoint;
  sli_zigbee_af_command_aps_frame->destinationEndpoint = destinationEndpoint;
#ifdef SL_CATALOG_KERNEL_PRESENT
  sl_zigbee_af_release_lock();
#endif
}
