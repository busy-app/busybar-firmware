/***************************************************************************//**
 * @file
 * @brief This is the source for the command line interface used for the ias zone
 * server plugin.
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

#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "ias-zone-server.h"

//-----------------------------------------------------------------------------
// Globals

#define RESERVED_END 0x7FFF
#define MANUFACTURER_SPECIFIC_START 0x8000
#define MANUFACTURER_SPECIFIC_END   0xFFFE

typedef struct {
  uint16_t zoneType;
  const char* zoneTypeString;
} sli_zigbee_zone_type_to_string_map_t;

// These functions and variables are only used to generate strings used with
// sl_zigbee_af_ias_zone_cluster_println calls, so if SL_ZIGBEE_AF_PRINT_IAS_ZONE_CLUSTER is
// not defined and they are compiled in, these declarations generate "function
// declared but never referenced" warnings
static sli_zigbee_zone_type_to_string_map_t zoneTypeToStringMap[] = {
  { 0x0000, "Standard CIE" },
  { 0x000d, "Motion Sensor" },
  { 0x0015, "Contact Switch" },
  { 0x0028, "Fire Sensor" },
  { 0x002a, "Water Sensor" },
  { 0x002b, "Gas Sensor" },
  { 0x002c, "Peersonal Emergency Device" },
  { 0x002d, "Vibration / Movement Sensor" },
  { 0x010f, "Remote Control" },
  { 0x0115, "Key Fob" },
  { 0x021d, "Keypad" },
  { 0x0225, "Standard Warning Device" },
  { 0xFFFF, NULL } // terminator
};

static const char manufacturerSpecificString[] = "Manufacturer Specific";
static const char invalidZoneTypeString[] = "Invalid";
static const char reservedString[] = "Reserved";

static const char notEnrolledString[] = "NOT Enrolled";
static const char enrolledString[] = "Enrolled";
static const char unknownZoneStateString[] = "Unknown";

//-----------------------------------------------------------------------------
// Functions

static const char* getZoneTypeString(uint16_t type)
{
  uint16_t i = 0;
  while (zoneTypeToStringMap[i].zoneTypeString != NULL) {
    if (zoneTypeToStringMap[i].zoneType == type) {
      return zoneTypeToStringMap[i].zoneTypeString;
    }
    i++;
  }

  if (type <= RESERVED_END) {
    return reservedString;
  }

  if (type >= MANUFACTURER_SPECIFIC_START
      && type <= MANUFACTURER_SPECIFIC_END) {
    return manufacturerSpecificString;
  }

  return invalidZoneTypeString;
}

static const char* getZoneStateString(uint8_t zoneState)
{
  switch (zoneState) {
    case SL_ZIGBEE_ZCL_IAS_ZONE_STATE_ENROLLED:
      return enrolledString;
    case SL_ZIGBEE_ZCL_IAS_ZONE_STATE_NOT_ENROLLED:
      return notEnrolledString;
  }
  return unknownZoneStateString;
}

static void getAttributes(uint8_t*  returnCieAddress,
                          uint16_t* returnZoneStatus,
                          uint16_t* returnZoneType,
                          uint8_t*  returnZoneState,
                          uint8_t   endpoint)
{
  SL_ZIGBEE_TEST_ASSERT(endpoint != EM_AF_UNKNOWN_ENDPOINT);

  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_IAS_ZONE_CLUSTER_ID,
                                     ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID,
                                     returnCieAddress,
                                     EUI64_SIZE);

  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_IAS_ZONE_CLUSTER_ID,
                                     ZCL_ZONE_STATUS_ATTRIBUTE_ID,
                                     (uint8_t*)returnZoneStatus,
                                     2); // uint16_t size

  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_IAS_ZONE_CLUSTER_ID,
                                     ZCL_ZONE_TYPE_ATTRIBUTE_ID,
                                     (uint8_t*)returnZoneType,
                                     2); // uint16_t size

  sl_zigbee_af_read_server_attribute(endpoint,
                                     ZCL_IAS_ZONE_CLUSTER_ID,
                                     ZCL_ZONE_STATE_ATTRIBUTE_ID,
                                     (uint8_t*)returnZoneState,
                                     1); // uint8_t size
}

#include "ias-zone-server-config.h"

void sl_zigbee_af_ias_zone_server_info_command(sl_cli_command_arg_t *arguments)
{
  uint8_t cieAddress[EUI64_SIZE];
  uint16_t zoneStatus;
  uint16_t zoneType;
  uint8_t zoneState;
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);

  getAttributes(cieAddress,
                &zoneStatus,
                &zoneType,
                &zoneState,
                endpoint);
  sl_zigbee_af_ias_zone_cluster_print("CIE Address: ");
  sl_zigbee_af_print_big_endian_eui64(cieAddress);
  sl_zigbee_af_ias_zone_cluster_println("");
  sl_zigbee_af_ias_zone_cluster_println("Zone Type:   0x%04X (%s)",
                                        zoneType,
                                        getZoneTypeString(zoneType));
  sl_zigbee_af_ias_zone_cluster_println("Zone State:  0x%02X   (%s)",
                                        zoneState,
                                        getZoneStateString(zoneState));
  sl_zigbee_af_ias_zone_cluster_println("Zone Status: 0x%04X",
                                        zoneStatus);
  sl_zigbee_af_ias_zone_cluster_println("Zone ID:     0x%04X",
                                        sl_zigbee_af_ias_zone_server_get_zone_id(endpoint));
}

void sl_zigbee_af_ias_zone_server_change_status_command(sl_cli_command_arg_t *arguments)
{
  uint16_t newStatus = sl_cli_get_argument_uint16(arguments, 0);
  uint8_t  timeSinceOccurredSeconds = sl_cli_get_argument_uint8(arguments, 1);
  uint8_t  endpoint = sl_cli_get_argument_uint8(arguments, 2);
  sl_zigbee_af_ias_zone_server_update_zone_status(endpoint,
                                                  newStatus,
                                                  timeSinceOccurredSeconds << 2);
}

void sl_zigbee_af_ias_zone_server_set_enrollment_method_command(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t enrollmentMode = sl_cli_get_argument_uint8(arguments, 1);
  sl_zigbee_af_ias_zone_cluster_set_enrollment_method(endpoint, enrollmentMode);
}

void sl_zigbee_af_ias_zone_server_change_backoff_configuration(sl_cli_command_arg_t *arguments)
{
#if SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE == 1
  sli_zigbee_ias_zone_status_queue_retry_config_t retryConfig = {
    sl_cli_get_argument_uint8(arguments, 0),
    sl_cli_get_argument_uint8(arguments, 1),
    sl_cli_get_argument_uint32(arguments, 2),
    sl_cli_get_argument_uint8(arguments, 3),
    sl_cli_get_argument_uint16(arguments, 4)
  };

  if (SL_STATUS_INVALID_PARAMETER
      == sl_zigbee_af_ias_zone_server_config_status_queue_retry_params(&retryConfig)) {
    sl_zigbee_af_ias_zone_cluster_println(
      "Parameter error! Valid parameters: %s, %s, %s %d, %s.",
      "0 < firstBackoff",
      "0 < commonRatio",
      "firstBackoff < maxBackoff <", IAS_ZONE_STATUS_QUEUE_RETRY_ABS_MAX_BACKOFF_TIME_SEC,
      "0 < maxRetryAttempts");
  }
#else
  UNUSED_VAR(arguments);
  sl_zigbee_af_ias_zone_cluster_println("Command error! IAS Zone Server status queue is not enabled");
#endif
}

void sl_zigbee_af_ias_zone_server_cli_print_queue(sl_cli_command_arg_t *arguments)
{
#if (SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE == 1)
  sl_zigbee_af_ias_zone_server_print_queue();
#else
  UNUSED_VAR(arguments);
  sl_zigbee_af_ias_zone_cluster_println("Command error! IAS Zone Server status queue is not enabled");
#endif
}

void sl_zigbee_af_ias_zone_server_cli_print_queue_config(sl_cli_command_arg_t *arguments)
{
#if (SL_ZIGBEE_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE == 1)
  sl_zigbee_af_ias_zone_server_print_queue_config();
#else
  UNUSED_VAR(arguments);
  sl_zigbee_af_ias_zone_cluster_println("Command error! IAS Zone Server status queue is not enabled");
#endif
}
