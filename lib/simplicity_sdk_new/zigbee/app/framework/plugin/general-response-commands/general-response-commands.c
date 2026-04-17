/***************************************************************************//**
 * @file
 * @brief Routines for the General Response Commands plugin.
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

#include "../../include/af.h"
#include "../../util/util.h"

bool sli_zigbee_af_general_response_commands_read_attributes_response_callback(sl_zigbee_af_cluster_id_t clusterId,
                                                                               uint8_t *buffer,
                                                                               uint16_t bufLen)
{
#if !(defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && (SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED == 1) && (SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED == 1))
  UNUSED_VAR(clusterId);
#endif

  uint16_t bufIndex = 0;
  sl_zigbee_af_debug_print("%s_RESP: ", "READ_ATTR");
  sl_zigbee_af_debug_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(clusterId, sl_zigbee_af_get_mfg_code_from_current_command()));
  sl_zigbee_af_debug_println("");
  sl_zigbee_af_debug_flush();

  // Each record in the response has a two-byte attribute id and a one-byte
  // status.  If the status is SUCCESS, there will also be a one-byte type and
  // variable-length data.
  while (bufIndex + 3 <= bufLen) {
    sl_zigbee_af_attribute_id_t attributeId;
    (void) attributeId;
    sl_zigbee_af_status_t status;
    attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                       bufIndex,
                                                                       bufLen);
    bufIndex += 2;
    status = (sl_zigbee_af_status_t)sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
    bufIndex++;
    sl_zigbee_af_debug_println(" - attr:%04X, status:%02X", attributeId, status);
    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      uint8_t dataType;
      uint16_t dataSize;
      if (bufLen - bufIndex < 1) {
        sl_zigbee_af_debug_println("ERR: attr:%04X premature end of buffer after success status", attributeId);
        break;
      }
      dataType = sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
      bufIndex++;

      dataSize = sl_zigbee_af_attribute_value_size(dataType,
                                                   buffer + bufIndex,
                                                   bufLen - bufIndex);

      sl_zigbee_af_debug_print("   type:%02X, val:", dataType);
      if (dataSize != 0) {
        if (sl_zigbee_af_is_string_attribute_type(dataType)) {
          sl_zigbee_af_debug_print_string(buffer + bufIndex);
        } else if (sl_zigbee_af_is_long_string_attribute_type(dataType)) {
          sl_zigbee_af_debug_debug_exec(sl_zigbee_af_print_long_string(SL_ZIGBEE_AF_PRINT_CORE, buffer + bufIndex));
        } else {
          sl_zigbee_af_debug_print_buffer(buffer + bufIndex, dataSize, false);
        }
        sl_zigbee_af_debug_println("");
        sl_zigbee_af_debug_flush();
        bufIndex += dataSize;
      } else {
        // dataSize exceeds buffer length, terminate loop
        sl_zigbee_af_debug_println("ERR: attr:%04X size exceeds buffer size %d", attributeId, dataSize);
        sl_zigbee_af_debug_flush();
        break; // while
      }
    }
  }
  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  return true;
}

bool sl_zigbee_af_write_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                               uint8_t *buffer,
                                               uint16_t bufLen)
{
#if !(defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && (SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED == 1) && (SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED == 1))
  UNUSED_VAR(clusterId);
#endif

  uint16_t bufIndex = 0;
  sl_zigbee_af_debug_print("%s_RESP: ", "WRITE_ATTR");
  sl_zigbee_af_debug_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(clusterId, sl_zigbee_af_get_mfg_code_from_current_command()));
  sl_zigbee_af_debug_println("");
  sl_zigbee_af_debug_flush();

  // Each record in the response has a one-byte status.  If the status is not
  // SUCCESS, the record will also contain a two-byte attribute id.
  while (bufIndex + 1 <= bufLen) {
    sl_zigbee_af_status_t status = (sl_zigbee_af_status_t)sl_zigbee_af_get_int8u(buffer,
                                                                                 bufIndex,
                                                                                 bufLen);
    bufIndex++;
    sl_zigbee_af_debug_println(" - status:%02X", status);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_attribute_id_t attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                                                     bufIndex,
                                                                                                     bufLen);
      // Remove an "unused variable" warning for the case the print call below
      // is a no-op.
      (void)attributeId;

      bufIndex += 2;
      sl_zigbee_af_debug_println("   attr:%04X", attributeId);
    }
    sl_zigbee_af_debug_flush();
  }
  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  return true;
}

bool sl_zigbee_af_configure_reporting_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                  uint8_t *buffer,
                                                  uint16_t bufLen)
{
#if !(defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && (SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED == 1) && (SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED == 1))
  UNUSED_VAR(clusterId);
#endif

  uint16_t bufIndex = 0;
  sl_zigbee_af_reporting_print("%s_RESP: ", "CFG_RPT");
  sl_zigbee_af_reporting_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(clusterId, sl_zigbee_af_get_mfg_code_from_current_command()));
  sl_zigbee_af_reporting_println("");
  sl_zigbee_af_reporting_flush();

  // Each record in the response has a one-byte status.  If the status is not
  // SUCCESS, the record will also contain a one-byte direction and a two-byte
  // attribute id.
  while (bufIndex + 1 <= bufLen) {
    sl_zigbee_af_status_t status = (sl_zigbee_af_status_t)sl_zigbee_af_get_int8u(buffer,
                                                                                 bufIndex,
                                                                                 bufLen);
    bufIndex++;
    sl_zigbee_af_reporting_println(" - status:%02X", status);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_reporting_direction_t direction;
      sl_zigbee_af_attribute_id_t attributeId;
      direction =  (sl_zigbee_af_reporting_direction_t)sl_zigbee_af_get_int8u(buffer,
                                                                              bufIndex,
                                                                              bufLen);
      bufIndex++;
      attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                         bufIndex,
                                                                         bufLen);
      bufIndex += 2;
      sl_zigbee_af_reporting_println("   direction:%02X, attr:%04X",
                                     direction,
                                     attributeId);
    }
    sl_zigbee_af_reporting_flush();
  }
  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  return true;
}

bool sl_zigbee_af_read_reporting_configuration_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                           uint8_t *buffer,
                                                           uint16_t bufLen)
{
#if !(defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && (SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED == 1) && (SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED == 1))
  UNUSED_VAR(clusterId);
#endif

  uint16_t bufIndex = 0;
  sl_zigbee_af_reporting_print("%s_RESP: ", "READ_RPT_CFG");
  sl_zigbee_af_reporting_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(clusterId, sl_zigbee_af_get_mfg_code_from_current_command()));
  sl_zigbee_af_reporting_println("");
  sl_zigbee_af_reporting_flush();

  // Each record in the response has a one-byte status, a one-byte direction,
  // and a two-byte attribute id.  If the status is SUCCESS, the record will
  // contain additional fields.
  while (bufIndex + 4 <= bufLen) {
    sl_zigbee_af_attribute_id_t attributeId;
    sl_zigbee_af_status_t status;
    sl_zigbee_af_reporting_direction_t direction;
    status = (sl_zigbee_af_status_t)sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
    bufIndex++;
    direction = (sl_zigbee_af_reporting_direction_t)sl_zigbee_af_get_int8u(buffer,
                                                                           bufIndex,
                                                                           bufLen);
    bufIndex++;
    attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                       bufIndex,
                                                                       bufLen);
    bufIndex += 2;
    sl_zigbee_af_reporting_println(" - status:%02X, direction:%02X, attr:%04X",
                                   status,
                                   direction,
                                   attributeId);
    if (status == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      // If the direction indicates the attribute is reported, the record will
      // contain a one-byte type and two two-byte intervals.  If the type is
      // analog, the record will contain a reportable change of the same data
      // type.  If the direction indicates reports of the attribute are
      // received, the record will contain a two-byte timeout.
      switch (direction) {
        case SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED:
        {
          uint16_t minInterval, maxInterval;
          uint8_t dataType;
          dataType = sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
          bufIndex++;
          minInterval = sl_zigbee_af_get_int16u(buffer, bufIndex, bufLen);
          bufIndex += 2;
          maxInterval = sl_zigbee_af_get_int16u(buffer, bufIndex, bufLen);
          bufIndex += 2;
          sl_zigbee_af_reporting_println("   type:%02X, min:%04X, max:%04X",
                                         dataType,
                                         minInterval,
                                         maxInterval);
          if (sl_zigbee_af_get_attribute_analog_or_discrete_type(dataType)
              == SL_ZIGBEE_AF_DATA_TYPE_ANALOG) {
            uint8_t dataSize = sl_zigbee_af_get_data_size(dataType);
            sl_zigbee_af_reporting_print("   change:");
            sl_zigbee_af_reporting_print_buffer(buffer + bufIndex, dataSize, false);
            sl_zigbee_af_reporting_println("");
            bufIndex += dataSize;
          }
          break;
        }
        case SL_ZIGBEE_ZCL_REPORTING_DIRECTION_RECEIVED:
        {
          uint16_t timeout = sl_zigbee_af_get_int16u(buffer, bufIndex, bufLen);
          bufIndex += 2;
          sl_zigbee_af_reporting_println("   timeout:%04X", timeout);
          break;
        }
        default:
          sl_zigbee_af_reporting_println("ERR: unknown direction %02X", direction);
          sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD);
          return true;
      }
    }
    sl_zigbee_af_reporting_flush();
  }
  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  return true;
}

bool sli_zigbee_af_general_response_commands_report_attributes_callback(sl_zigbee_af_cluster_id_t clusterId,
                                                                        uint8_t *buffer,
                                                                        uint16_t bufLen)
{
#if !(defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && (SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED == 1) && (SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED == 1))
  UNUSED_VAR(clusterId);
#endif

  uint16_t bufIndex = 0;

  sl_zigbee_af_reporting_print("RPT_ATTR: ");
  sl_zigbee_af_reporting_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(clusterId, sl_zigbee_af_get_mfg_code_from_current_command()));
  sl_zigbee_af_reporting_println("");

  // Each record in the response has a two-byte attribute id, a one-byte
  // type, and variable-length data.
  while (bufIndex + 3 < bufLen) {
    sl_zigbee_af_attribute_id_t attributeId;
    uint8_t dataType;
    uint16_t dataSize;
    attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                       bufIndex,
                                                                       bufLen);
    bufIndex += 2;
    dataType = sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
    bufIndex++;

    dataSize = sl_zigbee_af_attribute_value_size(dataType,
                                                 buffer + bufIndex,
                                                 bufLen - bufIndex);

    sl_zigbee_af_reporting_println(" - attr:%04X", attributeId);
    sl_zigbee_af_reporting_print("   type:%02X, val:", dataType);
    if (dataSize != 0) {
      if (sl_zigbee_af_is_string_attribute_type(dataType)) {
        sl_zigbee_af_reporting_print_string(buffer + bufIndex);
      } else if (sl_zigbee_af_is_long_string_attribute_type(dataType)) {
        sl_zigbee_af_reporting_debug_exec(sl_zigbee_af_print_long_string(SL_ZIGBEE_AF_PRINT_GENERIC, buffer + bufIndex));
      } else {
        sl_zigbee_af_reporting_print_buffer(buffer + bufIndex, dataSize, false);
      }
      sl_zigbee_af_reporting_println("");
      sl_zigbee_af_reporting_flush();
      bufIndex += dataSize;
    } else {
      // dataSize exceeds buffer length, terminate loop
      sl_zigbee_af_debug_println("ERR: attr:%04X size %d exceeds buffer size", attributeId, dataSize);
      sl_zigbee_af_reporting_flush();
      break; // while
    }
  }
  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  return true;
}

bool sl_zigbee_af_default_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                      uint8_t commandId,
                                      sl_zigbee_af_status_t status)
{
#if !(defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && (SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED == 1) && (SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED == 1))
  UNUSED_VAR(clusterId);
  UNUSED_VAR(commandId);
  UNUSED_VAR(status);
#endif

  sl_zigbee_af_debug_print("%s_RESP: ", "DEFAULT");
  sl_zigbee_af_debug_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(clusterId, sl_zigbee_af_get_mfg_code_from_current_command()));
  sl_zigbee_af_debug_println(" cmd %02X status %02X", commandId, status);
  sl_zigbee_af_debug_flush();
  return true;
}

bool sl_zigbee_af_discover_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                  bool discoveryComplete,
                                                  uint8_t *buffer,
                                                  uint16_t bufLen,
                                                  bool extended)
{
#if !(defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && (SL_ZIGBEE_DEBUG_ZCL_GROUP_ENABLED == 1) && (SL_ZIGBEE_DEBUG_PRINTS_ZCL_LEGACY_AF_DEBUG_ENABLED == 1))
  UNUSED_VAR(clusterId);
  UNUSED_VAR(discoveryComplete);
#endif

  uint16_t bufIndex = 0;
  sl_zigbee_af_debug_print("%s%s_RESP: ", "DISC_ATTR", (extended ? "_EXT" : ""));
  sl_zigbee_af_debug_debug_exec(sl_zigbee_af_decode_and_print_cluster_with_mfg_code(clusterId, sl_zigbee_af_get_mfg_code_from_current_command()));
  sl_zigbee_af_debug_println(" comp %sDONE", discoveryComplete ? "" : "NOT_");
  sl_zigbee_af_debug_flush();

  // Each record in the response has a two-byte attribute id and a one-byte
  // type.
  // NOTE if printing is not enabled then this loop doesn't do anything
  while (bufIndex + 3 <= bufLen) {
    sl_zigbee_af_attribute_id_t attributeId;
    uint8_t dataType;
    uint8_t accessControl;
    // NOTE silence unused but set variable (when printing is not enabled)
    (void) attributeId;
    (void) dataType;
    (void) accessControl;
    attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                       bufIndex,
                                                                       bufLen);
    bufIndex += 2;
    dataType = sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
    bufIndex++;
    if (extended) {
      accessControl = sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
      bufIndex++;
      sl_zigbee_af_debug_println(" - attr:%04X, type:%02X ac:%02X", attributeId, dataType, accessControl);
    } else {
      sl_zigbee_af_debug_println(" - attr:%04X, type:%02X", attributeId, dataType);
    }
    sl_zigbee_af_debug_flush();
  }
  sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  return true;
}
