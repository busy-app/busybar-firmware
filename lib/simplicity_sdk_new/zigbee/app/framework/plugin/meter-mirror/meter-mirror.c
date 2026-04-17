/***************************************************************************//**
 * @file
 * @brief Code to handle being a mirror for a sleepy meter.
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
#include "app/framework/util/util.h"
#include "meter-mirror.h"

#include "meter-mirror-config.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "zap-cluster-command-parser.h"

//----------------------------------------------------------------------------
// Globals

#define HAVE_MIRROR_CAPACITY 0x01
#ifndef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
#define NO_MIRROR_CAPACITY   0x00       // never used under GBCS
#endif

// Although technically endpoints are 8-bit, the returned value to the
// Mirror request and Mirror Remove commands are 16-bit.  Therefore
// we will use a 16-bit value to indicate an invalid endpoint.
#define INVALID_MIRROR_ENDPOINT 0xFFFF

#define INVALID_INDEX 0xFF

sl_802154_long_addr_t nullEui64 = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

#define MIRROR_NOT_FOUND_ZCL_RETURN_CODE 0xFFFF

// Each record in the report has a two-byte attribute id, a one-byte type, and
// variable-length data.
#define ATTRIBUTE_OVERHEAD 3

static bool sendMirrorReportAttributeResponse(uint8_t endpoint, uint8_t index);
static uint8_t findMirrorIndex(sl_802154_long_addr_t requestingDeviceIeeeAddress);

// This is NOT stored across reboots.  Future versions of the plugin will need
// support to persistently store this.
typedef struct {
  uint8_t eui64[EUI64_SIZE];
  uint32_t issuerEventId;
  uint32_t reportingInterval;
  uint8_t mirrorNotificationReporting;
  uint8_t notificationScheme;
} sli_zigbee_mirror_entry_t;

static sli_zigbee_mirror_entry_t mirrorList[SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS];

//-----------------------------------------------------------------------------
// Functions

uint8_t sli_zigbee_af_meter_mirror_get_mirrors_allocated(void)
{
  uint8_t mirrors = 0;
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (0 != memcmp(mirrorList[i].eui64, nullEui64, EUI64_SIZE)) {
      mirrors++;
    }
  }
  return mirrors;
}

static void updatePhysicalEnvironment(void)
{
  uint8_t physEnv =
#ifdef SL_CATALOG_ZIGBEE_GBCS_COMPATIBILITY_PRESENT
    HAVE_MIRROR_CAPACITY;
#else
    (sli_zigbee_af_meter_mirror_get_mirrors_allocated() < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS
     ? HAVE_MIRROR_CAPACITY
     : NO_MIRROR_CAPACITY);
#endif

  sl_zigbee_af_status_t status
    = sl_zigbee_af_write_server_attribute(SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_METER_CLIENT_ENDPOINT,
                                          ZCL_BASIC_CLUSTER_ID,
                                          ZCL_PHYSICAL_ENVIRONMENT_ATTRIBUTE_ID,
                                          &physEnv,
                                          ZCL_ENUM8_ATTRIBUTE_TYPE);
  if (status) {
    sl_zigbee_af_simple_metering_cluster_println("Error:  Could not update mirror capacity status %u.",
                                                 status);
  }
}

static uint8_t getIndexFromEndpoint(uint8_t endpoint)
{
  if (endpoint < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START
      ||  (endpoint >= (SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START
                        + SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS))) {
    return INVALID_INDEX;
  }
  return (endpoint - SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START);
}

static void clearMirrorByIndex(uint8_t index, bool print)
{
  memset(mirrorList[index].eui64, 0xFF, EUI64_SIZE);

  if (print) {
    sl_zigbee_af_simple_metering_cluster_println("Removed meter mirror at endpoint %d",
                                                 index + SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START);
  }
//  sl_zigbee_af_endpoint_enable_disable(endpoint,false);
  updatePhysicalEnvironment();
}

static void clearAllMirrors(bool print)
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    clearMirrorByIndex(i, print);
  }
}

bool sl_zigbee_af_meter_mirror_get_eui64_by_endpoint(uint8_t endpoint,
                                                     sl_802154_long_addr_t returnEui64)
{
  uint8_t index = getIndexFromEndpoint(endpoint);
  if (index == INVALID_INDEX) {
    return false;
  }
  memmove(returnEui64, mirrorList[index].eui64, EUI64_SIZE);
  return true;
}

bool sl_zigbee_af_meter_mirror_get_endpoint_by_eui64(sl_802154_long_addr_t eui64,
                                                     uint8_t *returnEndpoint)
{
  uint8_t index = findMirrorIndex(eui64);
  if (index == INVALID_INDEX) {
    return false;
  }
  *returnEndpoint = SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START + index;
  return true;
}

void sl_zigbee_af_meter_mirror_init_cb(uint8_t init_level)
{
  (void)init_level;

  // disable all mirror endpoints
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    sl_zigbee_af_endpoint_enable_disable(SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START + i,
                                         false);
  }

  // clear internal list of meter mirror EUIs/update phy env
  clearAllMirrors(false);
}

bool sl_zigbee_af_meter_mirror_is_mirror_used(uint8_t endpoint)
{
  uint8_t index = getIndexFromEndpoint(endpoint);
  if (INVALID_INDEX == index) {
    sl_zigbee_af_simple_metering_cluster_println("Error:  Endpoint %d is not a valid mirror endpoint.",
                                                 endpoint);
    return false;
  }
  return (0 != memcmp(mirrorList[index].eui64, nullEui64, EUI64_SIZE));
}

void sl_zigbee_af_meter_mirror_stack_status_cb(sl_status_t status)
{
  if (status == SL_STATUS_NETWORK_DOWN
      && !sl_zigbee_is_performing_rejoin()) {
    sl_zigbee_af_simple_metering_cluster_println("Re-initializing mirrors due to stack down.");
    clearAllMirrors(true);  // print?
  }
}

static uint8_t findMirrorIndex(sl_802154_long_addr_t requestingDeviceIeeeAddress)
{
  uint8_t i;

  for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS; i++) {
    if (0 == memcmp(requestingDeviceIeeeAddress, mirrorList[i].eui64, EUI64_SIZE)) {
      return i;
    }
  }
  return INVALID_INDEX;
}

uint16_t sl_zigbee_af_simple_metering_client_request_mirror_cb(sl_802154_long_addr_t requestingDeviceIeeeAddress)
{
  return sl_zigbee_af_meter_mirror_request_mirror(requestingDeviceIeeeAddress);
}

uint16_t sl_zigbee_af_meter_mirror_request_mirror(sl_802154_long_addr_t requestingDeviceIeeeAddress)
{
  uint8_t meteringDeviceTypeAttribute =  SL_ZIGBEE_ZCL_METERING_DEVICE_TYPE_UNDEFINED_MIRROR_METER;
  uint8_t index = findMirrorIndex(requestingDeviceIeeeAddress);
  uint8_t endpoint = SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
  sl_status_t status;

  if (sli_zigbee_af_meter_mirror_get_mirrors_allocated() >= SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS
      && index == INVALID_INDEX) {
    return INVALID_MIRROR_ENDPOINT;
  }

  if (index != INVALID_INDEX) {
    endpoint += index;
    sl_zigbee_af_simple_metering_cluster_println("Mirror already allocated on endpoint %d",
                                                 endpoint);
    return endpoint;
  }

  index = findMirrorIndex(nullEui64);
  if (index == INVALID_INDEX) {
    sl_zigbee_af_simple_metering_cluster_println("No free mirror endpoints for new mirror.\n");
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE);
    return INVALID_MIRROR_ENDPOINT;
  }

  endpoint += index;
  memcpy(mirrorList[index].eui64, requestingDeviceIeeeAddress, EUI64_SIZE);
  mirrorList[index].issuerEventId = 0;
  mirrorList[index].reportingInterval = 0;
  mirrorList[index].mirrorNotificationReporting = false;
  mirrorList[index].notificationScheme = SL_ZIGBEE_ZCL_NOTIFICATION_SCHEME_NO_NOTIFICATION_SCHEME_DEFINED;
  sl_zigbee_af_endpoint_enable_disable(endpoint, true);
  sl_zigbee_af_simple_metering_cluster_print("Registered new meter mirror: ");
  sl_zigbee_af_print_big_endian_eui64(requestingDeviceIeeeAddress);
  sl_zigbee_af_simple_metering_cluster_println(".  Endpoint: %d", endpoint);
  updatePhysicalEnvironment();

  status
    = sl_zigbee_af_write_server_attribute(endpoint,
                                          ZCL_SIMPLE_METERING_CLUSTER_ID,
                                          ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID,
                                          (uint8_t*) &meteringDeviceTypeAttribute,
                                          ZCL_BITMAP8_ATTRIBUTE_TYPE);
  if (status) {
    sl_zigbee_af_simple_metering_cluster_println("Failed to write Metering device type attribute on ep%d, status :  0x%02X",
                                                 endpoint,
                                                 status);
  }

  sl_zigbee_af_meter_mirror_mirror_added_cb(requestingDeviceIeeeAddress, endpoint);
  return endpoint;
}

uint16_t sl_zigbee_af_simple_metering_client_remove_mirror_cb(sl_802154_long_addr_t requestingDeviceIeeeAddress)
{
  return sl_zigbee_af_meter_mirror_remove_mirror(requestingDeviceIeeeAddress);
}

uint16_t sl_zigbee_af_meter_mirror_remove_mirror(sl_802154_long_addr_t requestingDeviceIeeeAddress)
{
  uint8_t index;
  uint8_t endpoint;

  if (0 == memcmp(nullEui64, requestingDeviceIeeeAddress, EUI64_SIZE)) {
    sl_zigbee_af_simple_metering_cluster_println("Rejecting mirror removal using NULL EUI64");
    return INVALID_MIRROR_ENDPOINT;
  }

  index = findMirrorIndex(requestingDeviceIeeeAddress);

  if (index == INVALID_INDEX) {
    sl_zigbee_af_simple_metering_cluster_print("Unknown mirror for remove: ");
    sl_zigbee_af_print_big_endian_eui64(requestingDeviceIeeeAddress);
    sl_zigbee_af_simple_metering_cluster_println(" ");
    return MIRROR_NOT_FOUND_ZCL_RETURN_CODE;
  }

  clearMirrorByIndex(index,
                     true);  // print?

  endpoint = index + SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
  sl_zigbee_af_meter_mirror_mirror_removed_cb(requestingDeviceIeeeAddress, endpoint);
  return endpoint;
}

bool sl_zigbee_af_report_attributes_cb(sl_zigbee_af_cluster_id_t clusterId,
                                       uint8_t *buffer,
                                       uint16_t bufLen)
{
  sl_802154_long_addr_t sendersEui;
  uint16_t bufIndex = 0;
  uint8_t endpoint;
  uint8_t index;
  bool attributeReportingComplete = false;

  if (SL_STATUS_OK
      != sl_zigbee_lookup_eui64_by_node_id(sl_zigbee_af_current_command()->source, sendersEui)) {
    sl_zigbee_af_simple_metering_cluster_println("Error: Meter Mirror plugin cannot determine EUI64 for node ID 0x%04X",
                                                 sl_zigbee_af_current_command()->source);
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_FAILURE);
    return true;
  }

  if (sl_zigbee_af_current_command()->direction
      == ZCL_DIRECTION_CLIENT_TO_SERVER) {
    sl_zigbee_af_simple_metering_cluster_println("Error:  Meter Mirror Plugin does not accept client to server attributes.\n",
                                                 sl_zigbee_af_current_command()->direction);
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_FAILURE);
    return true;
  }

  index = findMirrorIndex(sendersEui);
  if (index == INVALID_INDEX) {
    sl_zigbee_af_simple_metering_cluster_print("Error: Meter mirror plugin received unknown report from ");
    sl_zigbee_af_print_big_endian_eui64(sendersEui);
    sl_zigbee_af_simple_metering_cluster_println("");
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED);
    return true;
  }

  if (sl_zigbee_af_current_command()->mfgSpecific) {
    // Here is where we could handle a MFG specific Report attributes and interpret
    // it.  This code does not do that, just politely returns an error.
    sl_zigbee_af_simple_metering_cluster_println("Error: Unknown MFG Code for mirror: 0x%04X",
                                                 sl_zigbee_af_current_command()->mfgCode);
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND);
    return true;
  }

  endpoint = (index + SL_ZIGBEE_AF_PLUGIN_METER_MIRROR_ENDPOINT_START);
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  while (bufIndex + ATTRIBUTE_OVERHEAD < bufLen) {
    status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
    sl_zigbee_af_attribute_id_t attributeId;
    sl_zigbee_af_attribute_type_t dataType;
    uint16_t dataSize;

    attributeId = (sl_zigbee_af_attribute_id_t)sl_zigbee_af_get_int16u(buffer,
                                                                       bufIndex,
                                                                       bufLen);
    bufIndex += 2;
    dataType = (sl_zigbee_af_attribute_type_t)sl_zigbee_af_get_int8u(buffer, bufIndex, bufLen);
    bufIndex++;

    dataSize = sl_zigbee_af_attribute_value_size(dataType,
                                                 buffer + bufIndex,
                                                 bufLen - bufIndex);

#if (BIGENDIAN_CPU)
    if (dataSize != 0
        && dataSize <= ZCL_ATTRIBUTE_MAX_DATA_SIZE) {
      uint8_t data[ZCL_ATTRIBUTE_MAX_DATA_SIZE];
      if (isThisDataTypeSentLittleEndianOTA(dataType)) {
        sl_util_reverse_mem_copy(data, buffer + bufIndex, dataSize);
      } else {
        memmove(data, buffer + bufIndex, dataSize);
      }
#else
    if (dataSize != 0) {
      uint8_t *data = buffer + bufIndex;
#endif
      if (attributeId == ZCL_SIMPLE_METERING_CLUSTER_REPORTING_STATUS_SERVER_ATTRIBUTE_ID) {
        // From the SE 1.2a Specification within the ConfigureMirror command
        // description..
        // 1.  On powering up, the BOMD will send one or more Report Attribute
        //     commands to the Metering client on the mirror endpoint. The last
        //     attribute to be reported to the mirror shall be an Attribute
        //     Reporting Status attribute, as defined in section A.2.
        // 2.  If MirrorReportAttributeResponse is enabled, the server does not
        //     need to request an APS ACK. If the server requests an APS ACK,
        //     the Metering client on the mirror endpoint shall respond first
        //     with an APS ACK and then send the MirrorReportAttributeResponse.
        //
        // If Mirror Notification Reporting is set to false, the
        // MirrorReportAttributeResponse command shall not be enabled; the
        // Metering server may poll the Notification flags by means of a normal
        // ReadAttribute command, as shown in Figure D 29:
        attributeReportingComplete = (data[0] == SL_ZIGBEE_ZCL_ATTRIBUTE_REPORTING_STATUS_ATTRIBUTE_REPORTING_COMPLETE);
        status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
      } else {
        if (attributeId == ZCL_METERING_DEVICE_TYPE_ATTRIBUTE_ID
            && data[0] < SL_ZIGBEE_ZCL_METERING_DEVICE_TYPE_MIRRORED_ELECTRIC_METERING) {
          data[0] += 127;
        }

        status = sl_zigbee_af_write_server_attribute(endpoint,
                                                     clusterId,
                                                     attributeId,
                                                     data,
                                                     dataType);
        if (status == SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE) {
          // Unsupported attribute to mirror, terminate loop
          status = SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE;
          break;
        }
      }

      sl_zigbee_af_simple_metering_cluster_println("Mirror attribute 0x%04X: 0x%02X", attributeId, status);
      bufIndex += dataSize;
      if (status == SL_ZIGBEE_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE) {
        sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE);
        return true;
      }
    } else {
      // dataSize exceeds buffer length, terminate loop
      sl_zigbee_af_simple_metering_cluster_println("ERR: attr:%04X size %d exceeds buffer size", attributeId, dataSize);
      break;
    }
  }

  // If reporting is complete then callback to the application so that if it needs
  // to do any post processing on the reported attributes it can do it now.
  if (attributeReportingComplete) {
    sl_zigbee_af_meter_mirror_reporting_complete_cb(endpoint);
  }

  // Notification flags
  sl_zigbee_af_simple_metering_cluster_println("Mirror reporting ep: 0x%02X, reporting: 0x%02X, scheme: 0x%02X",
                                               endpoint,
                                               mirrorList[index].mirrorNotificationReporting,
                                               mirrorList[index].notificationScheme);
  if (mirrorList[index].mirrorNotificationReporting  && attributeReportingComplete) {
    if (mirrorList[index].notificationScheme == SL_ZIGBEE_ZCL_NOTIFICATION_SCHEME_PREDEFINED_NOTIFICATION_SCHEME_A
        || mirrorList[index].notificationScheme == SL_ZIGBEE_ZCL_NOTIFICATION_SCHEME_PREDEFINED_NOTIFICATION_SCHEME_B) {
      return sendMirrorReportAttributeResponse(endpoint, index);
    } else {
      // TODO: for custom notification schemes callback to application
      // return emberAfMeterMirrorSendMirrorReportAttributeResponseCallback(...)
    }
  }
  sl_zigbee_af_send_immediate_default_response(status);
  return true;
}

static bool sendMirrorReportAttributeResponse(uint8_t endpoint, uint8_t index)
{
  uint8_t status;
  uint8_t notificationFlagsBuffer[20];
  uint8_t flagsBufferLen = 0;
  uint16_t attrId;
  uint32_t readFlag;

  // Predefined Notification Scheme B requires reporting the following
  // notification flags:
  //   FUNCTIONAL_NOTIFICATION_FLAGS
  status = sl_zigbee_af_read_client_attribute(endpoint,
                                              ZCL_SIMPLE_METERING_CLUSTER_ID,
                                              ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                                              (uint8_t *)&readFlag,
                                              sizeof(uint32_t));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    memset(notificationFlagsBuffer, 0, sizeof(uint32_t));
  } else {
    sl_zigbee_af_copy_int32u(notificationFlagsBuffer, flagsBufferLen, readFlag);
  }
  flagsBufferLen += 4;

  // Predefined Notification Scheme A prescribes only reporting the functional
  // notification flags
  if (mirrorList[index].notificationScheme == SL_ZIGBEE_ZCL_NOTIFICATION_SCHEME_PREDEFINED_NOTIFICATION_SCHEME_A) {
    goto kickout;
  }

  // Predefined Notification Scheme B requires reporting the following
  // notification flags:
  //   FUNCTIONAL_NOTIFICATION_FLAGS
  //   NOTIFICATION_FLAGS_2
  //   NOTIFICATION_FLAGS_3
  //   NOTIFICATION_FLAGS_4
  //   NOTIFICATION_FLAGS_5
  for (attrId = ZCL_NOTIFICATION_FLAGS_2_ATTRIBUTE_ID;
       attrId < ZCL_NOTIFICATION_FLAGS_6_ATTRIBUTE_ID;
       attrId++) {
    status = sl_zigbee_af_read_client_attribute(endpoint,
                                                ZCL_SIMPLE_METERING_CLUSTER_ID,
                                                attrId,
                                                (uint8_t *)&readFlag,
                                                sizeof(uint32_t));
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      memset(&notificationFlagsBuffer[flagsBufferLen], 0, sizeof(uint32_t));
    } else {
      sl_zigbee_af_copy_int32u(notificationFlagsBuffer, flagsBufferLen, readFlag);
    }
    flagsBufferLen += 4;
  }

  kickout:
  // Populate the command buffer
  sl_zigbee_af_fill_command_simple_metering_cluster_mirror_report_attribute_response(mirrorList[index].notificationScheme,
                                                                                     (uint8_t *) notificationFlagsBuffer,
                                                                                     flagsBufferLen);
  sl_zigbee_af_simple_metering_cluster_println("Mirror report attribute response buffer populated");
  sl_zigbee_af_send_response();
  return true;
}

sl_zigbee_af_zcl_request_status_t sl_zigbee_af_simple_metering_cluster_configure_mirror_cb(sl_zigbee_af_cluster_command_t *cmd)
{
  sl_zcl_simple_metering_cluster_configure_mirror_command_t cmd_data;

  if (zcl_decode_simple_metering_cluster_configure_mirror_command(cmd, &cmd_data)
      != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    return SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;
  }

  uint32_t issuerEventId = cmd_data.issuerEventId;
  uint32_t reportingInterval = cmd_data.reportingInterval;
  uint8_t mirrorNotificationReporting = cmd_data.mirrorNotificationReporting;
  uint8_t notificationScheme = cmd_data.notificationScheme;
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  uint8_t endpoint = sl_zigbee_af_current_endpoint();
  sl_802154_long_addr_t sendersEui;
  uint8_t index;

  sl_zigbee_af_simple_metering_cluster_println("ConfigureMirror on endpoint 0x%02X", endpoint);

  if (SL_STATUS_OK != sl_zigbee_lookup_eui64_by_node_id(sl_zigbee_af_current_command()->source, sendersEui)) {
    sl_zigbee_af_simple_metering_cluster_println("Error: Meter Mirror plugin cannot determine EUI64 for node ID 0x%04X",
                                                 sl_zigbee_af_current_command()->source);
    status = SL_ZIGBEE_ZCL_STATUS_FAILURE;
    goto kickout;
  }

  index = findMirrorIndex(sendersEui);
  if (index == INVALID_INDEX) {
    sl_zigbee_af_simple_metering_cluster_print("Error: Meter mirror plugin received unknown report from ");
    sl_zigbee_af_print_big_endian_eui64(sendersEui);
    sl_zigbee_af_simple_metering_cluster_println("");
    status = SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED;
    goto kickout;
  }

  if (mirrorList[index].issuerEventId == 0
      || issuerEventId > mirrorList[index].issuerEventId) {
    if (notificationScheme > 0x02) {
      status = SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD;
      goto kickout;
    }

    mirrorList[index].issuerEventId = issuerEventId;
    mirrorList[index].reportingInterval = reportingInterval;
    mirrorList[index].mirrorNotificationReporting = mirrorNotificationReporting;
    mirrorList[index].notificationScheme = notificationScheme;
  }

  kickout:
  return status;
}

uint32_t sli_zigbee_af_meter_mirror_simple_metering_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                                 sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_zcl_request_status_t status = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_CONFIGURE_MIRROR_COMMAND_ID:
      {
        status = sl_zigbee_af_simple_metering_cluster_configure_mirror_cb(cmd);
        break;
      }
    }
  }
  return status;
}
