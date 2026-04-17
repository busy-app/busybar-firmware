/***************************************************************************//**
 * @file
 * @brief Attribute definitions for the Test Harness plugin.
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

#include "test-harness.h"

#include "test-harness-config.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

//------------------------------------------------------------------------------
// Globals
sl_zigbee_af_event_t sl_zigbee_af_test_harness_read_write_attributes_timeout_event;
#define readWriteAttributesTimeoutEventControl (&sl_zigbee_af_test_harness_read_write_attributes_timeout_event)
void sl_zigbee_af_test_harness_read_write_attributes_timeout_event_handler(sl_zigbee_af_event_t * event);

typedef enum {
  READ_WRITE_TEST_STATE_NONE,
  READ_WRITE_TEST_STATE_READ_SENT,
  READ_WRITE_TEST_STATE_WRITE_SENT,
} sli_zigbee_read_write_state_t;

typedef enum {
  TEST_TYPE_READ_WRITE_ATTRIBUTES = 0,
  TEST_TYPE_BIG_ATTRIBUTE_READ    = 1,
} sli_zigbee_test_type_t;

#define TEST_TYPE_MAX TEST_TYPE_BIG_ATTRIBUTE_READ

static const char* testTypeStrings[] = {
  "Read Write Multiple attributes",
  "Big Attribute Read (requiring fragmentation)",
  NULL,
};

sli_zigbee_test_type_t testType = TEST_TYPE_READ_WRITE_ATTRIBUTES;

static sli_zigbee_read_write_state_t readWriteState = READ_WRITE_TEST_STATE_NONE;

static sl_802154_short_addr_t destAddress = SL_ZIGBEE_NULL_NODE_ID;
static uint8_t sourceEndpoint = 1;
static uint8_t destEndpoint;
static uint16_t testClusterId;
static uint16_t attributeIdStart = 0;
static uint16_t attributeIdEnd = 0;
static bool clientToServer = true;

static uint16_t currentIndex;

void setDestinationCommand(SL_CLI_COMMAND_ARG);

const char* setDestCommandArguments[] = {
  "dest-address",
  "dest-ep",
  NULL,
};

const char* startTestArguments[] = {
  "cluster-id",
  "attribute-id-start",
  "attribute-id-end",
  "client-to-server",
  NULL,
};

const char* optionCommandArguments[] = {
  "test-type",
  "timeout-in-sec",
  NULL,
};

static uint16_t timeoutSeconds = 3;

void startTestCommand(SL_CLI_COMMAND_ARG);
void setOptionsCommand(SL_CLI_COMMAND_ARG);

#define UNKNOWN_RESULT 0xFF
typedef struct  {
  uint8_t readResult;
  uint8_t writeResult;
  uint8_t attributeType;
} sli_zigbee_single_attribute_test_results_t;

#define MAX_ATTRIBUTE_IDS 100
static sli_zigbee_single_attribute_test_results_t attributeResults[MAX_ATTRIBUTE_IDS];

typedef struct {
  uint8_t status;
  const char* string;
} sli_zigbee_zcl_status_to_string_map_t;

const sli_zigbee_zcl_status_to_string_map_t zclStatusToStringMap[] = {
  { SL_ZIGBEE_ZCL_STATUS_SUCCESS, "Success         " },
  { SL_ZIGBEE_ZCL_STATUS_FAILURE, "Failure         " },
  { SL_ZIGBEE_ZCL_STATUS_NOT_AUTHORIZED, "Not Authorized  " },
  { SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND, "Malformed Comm  " },
  { SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND, "Unsup Cmd       " },
  { SL_ZIGBEE_ZCL_STATUS_UNSUP_GENERAL_COMMAND, "Unsup Gen Cmd   " },
  { SL_ZIGBEE_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND, "Uns MFG Clus Cmd" },
  { SL_ZIGBEE_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND, "Uns MFG Gen Cmd " },
  { SL_ZIGBEE_ZCL_STATUS_INVALID_FIELD, "Invalid Field   " },
  { SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE, "Unsupported Attr" },
  { SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE, "Invalid Value   " },
  { SL_ZIGBEE_ZCL_STATUS_READ_ONLY, "Read Only       " },
  { SL_ZIGBEE_ZCL_STATUS_INSUFFICIENT_SPACE, "Insuff Space    " },
  { SL_ZIGBEE_ZCL_STATUS_DUPLICATE_EXISTS, "Duplicate Exists" },
  { SL_ZIGBEE_ZCL_STATUS_NOT_FOUND, "Not found       " },
  { SL_ZIGBEE_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE, "Unreportable Att" },
  { SL_ZIGBEE_ZCL_STATUS_INVALID_DATA_TYPE, "Invalid Data Typ" },
  { SL_ZIGBEE_ZCL_STATUS_INVALID_SELECTOR, "Invalid selector" },
  { SL_ZIGBEE_ZCL_STATUS_WRITE_ONLY, "Write Only      " },
  { SL_ZIGBEE_ZCL_STATUS_INCONSISTENT_STARTUP_STATE, "Inconsis Startup" },
  { SL_ZIGBEE_ZCL_STATUS_DEFINED_OUT_OF_BAND, "Def Out of Band " },
  { SL_ZIGBEE_ZCL_STATUS_ACTION_DENIED, "Action Denied   " },
  { SL_ZIGBEE_ZCL_STATUS_TIMEOUT, "Timeout         " },
  { SL_ZIGBEE_ZCL_STATUS_ABORT, "Abort           " },
  { SL_ZIGBEE_ZCL_STATUS_INVALID_IMAGE, "Invalid Image   " },
  { SL_ZIGBEE_ZCL_STATUS_WAIT_FOR_DATA, "Wait for data   " },
  { SL_ZIGBEE_ZCL_STATUS_NO_IMAGE_AVAILABLE, "No image avail  " },
  { SL_ZIGBEE_ZCL_STATUS_REQUIRE_MORE_IMAGE, "Require more img" },
  { SL_ZIGBEE_ZCL_STATUS_NOTIFICATION_PENDING, "Notif Pending   " },
  { SL_ZIGBEE_ZCL_STATUS_HARDWARE_FAILURE, "Hardware failure" },
  { SL_ZIGBEE_ZCL_STATUS_SOFTWARE_FAILURE, "Software failure" },
  { SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_CLUSTER, "Unsup Cluster   " },
  { SL_ZIGBEE_ZCL_STATUS_LIMIT_REACHED, "Limit Reached   " },

  { 0, NULL },  // terminator
};
static const char unknownErrorString[] =           "???             ";

typedef struct {
  uint8_t type;
  const char* string;
} sli_zigbee_zcl_attribute_type_to_string_map_t;

sli_zigbee_zcl_attribute_type_to_string_map_t zclAttributeTypeToStringMap[] = {
  { ZCL_NO_DATA_ATTRIBUTE_TYPE, "No Data        " },
  { ZCL_DATA8_ATTRIBUTE_TYPE, "Data 8         " },
  { ZCL_DATA16_ATTRIBUTE_TYPE, "Data 16        " },
  { ZCL_DATA24_ATTRIBUTE_TYPE, "Data 24        " },
  { ZCL_DATA32_ATTRIBUTE_TYPE, "Data 32        " },
  { ZCL_DATA40_ATTRIBUTE_TYPE, "Data 40        " },
  { ZCL_DATA48_ATTRIBUTE_TYPE, "Data 48        " },
  { ZCL_DATA56_ATTRIBUTE_TYPE, "Data 56        " },
  { ZCL_DATA64_ATTRIBUTE_TYPE, "Data 64        " },
  { ZCL_BOOLEAN_ATTRIBUTE_TYPE, "Boolean        " },
  { ZCL_BITMAP8_ATTRIBUTE_TYPE, "Bitmap 8       " },
  { ZCL_BITMAP16_ATTRIBUTE_TYPE, "Bitmap 16      " },
  { ZCL_BITMAP24_ATTRIBUTE_TYPE, "Bitmap 24      " },
  { ZCL_BITMAP32_ATTRIBUTE_TYPE, "Bitmap 32      " },
  { ZCL_BITMAP40_ATTRIBUTE_TYPE, "Bitmap 40      " },
  { ZCL_BITMAP48_ATTRIBUTE_TYPE, "Bitmap 48      " },
  { ZCL_BITMAP56_ATTRIBUTE_TYPE, "Bitmap 56      " },
  { ZCL_BITMAP64_ATTRIBUTE_TYPE, "Bitmap 64      " },
  { ZCL_INT8U_ATTRIBUTE_TYPE, "Unsigned Int 8 " },
  { ZCL_INT16U_ATTRIBUTE_TYPE, "Unsigned Int 16" },
  { ZCL_INT24U_ATTRIBUTE_TYPE, "Unsigned Int 24" },
  { ZCL_INT32U_ATTRIBUTE_TYPE, "Unsigned Int 32" },
  { ZCL_INT40U_ATTRIBUTE_TYPE, "Unsigned Int 40" },
  { ZCL_INT48U_ATTRIBUTE_TYPE, "Unsigned Int 48" },
  { ZCL_INT56U_ATTRIBUTE_TYPE, "Unsigned Int 56" },
  { ZCL_INT64U_ATTRIBUTE_TYPE, "Unsigned Int 64" },
  { ZCL_INT8S_ATTRIBUTE_TYPE, "Signed Int 8   " },
  { ZCL_INT16S_ATTRIBUTE_TYPE, "Signed Int 16  " },
  { ZCL_INT24S_ATTRIBUTE_TYPE, "Signed Int 24  " },
  { ZCL_INT32S_ATTRIBUTE_TYPE, "Signed Int 32  " },
  { ZCL_INT40S_ATTRIBUTE_TYPE, "Signed Int 40  " },
  { ZCL_INT48S_ATTRIBUTE_TYPE, "Signed Int 48  " },
  { ZCL_INT56S_ATTRIBUTE_TYPE, "Signed Int 56  " },
  { ZCL_INT64S_ATTRIBUTE_TYPE, "Signed Int 64  " },
  { ZCL_ENUM8_ATTRIBUTE_TYPE, "Enum 8         " },
  { ZCL_ENUM16_ATTRIBUTE_TYPE, "Enum 16        " },
  { ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE, "Float semi     " },
  { ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE, "Float Single   " },
  { ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE, "Float Double   " },
  { ZCL_OCTET_STRING_ATTRIBUTE_TYPE, "Octet String   " },
  { ZCL_CHAR_STRING_ATTRIBUTE_TYPE, "Char String    " },
  { ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE, "Octet string   " },
  { ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE, "Long Char Str  " },
  { ZCL_ARRAY_ATTRIBUTE_TYPE, "Array Attribute" },
  { ZCL_STRUCT_ATTRIBUTE_TYPE, "Struct Attrib  " },
  { ZCL_SET_ATTRIBUTE_TYPE, "Set Attribute  " },
  { ZCL_BAG_ATTRIBUTE_TYPE, "Bag Attribute  " },
  { ZCL_TIME_OF_DAY_ATTRIBUTE_TYPE, "Time of Day Att" },
  { ZCL_DATE_ATTRIBUTE_TYPE, "Date Attribute " },
  { ZCL_UTC_TIME_ATTRIBUTE_TYPE, "UTC Time Attr  " },
  { ZCL_CLUSTER_ID_ATTRIBUTE_TYPE, "Cluster ID Attr" },
  { ZCL_ATTRIBUTE_ID_ATTRIBUTE_TYPE, "Attribute ID   " },
  { ZCL_BACNET_OID_ATTRIBUTE_TYPE, "Bacnet OID     " },
  { ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE, "Ieee Address   " },
  { ZCL_SECURITY_KEY_ATTRIBUTE_TYPE, "Security Key   " },
  { ZCL_UNKNOWN_ATTRIBUTE_TYPE, "Unknown        " },
  { 0, NULL }  // terminator
};
static const char unknownAttributeTypeString[] = "???            ";

#define SOURCE_ENDPOINT 1

//------------------------------------------------------------------------------
// Forward Declarations

static void sendReadAttribute(void);
static void bigReadCommand(void);

//------------------------------------------------------------------------------
// Functions
#if (defined(SL_CATALOG_CLI_PRESENT))
void setOptionsCommand(SL_CLI_COMMAND_ARG)
{
  uint8_t temp = sl_cli_get_argument_uint8(arguments, 0);
  if (temp > TEST_TYPE_MAX) {
    uint8_t i;
    sl_zigbee_af_core_println("Error: Invalid test type number.  Valid numbers are:");
    for (i = 0; i <= TEST_TYPE_MAX; i++) {
      sl_zigbee_af_core_println("%d: %s", i, testTypeStrings[i]);
    }
    return;
  }

  timeoutSeconds = sl_cli_get_argument_uint16(arguments, 1);
  testType = (sli_zigbee_test_type_t)temp;
}

void setDestinationCommand(SL_CLI_COMMAND_ARG)
{
  destAddress = sl_cli_get_argument_uint16(arguments, 0);
  destEndpoint = sl_cli_get_argument_uint8(arguments, 1);
}

static void readWriteAttributeTest(void)
{
  uint16_t count = attributeIdEnd - attributeIdStart + 1;
  if (count > MAX_ATTRIBUTE_IDS) {
    sl_zigbee_af_core_println("Error: Too many attributes to test (%d > %d).",
                              count,
                              MAX_ATTRIBUTE_IDS);
    return;
  }
  memset(attributeResults,
         UNKNOWN_RESULT,
         sizeof(sli_zigbee_single_attribute_test_results_t) * MAX_ATTRIBUTE_IDS);
  currentIndex = 0;
  sl_zigbee_af_core_println("Testing cluster 0x%04X, attributes 0x%04X -> 0x%04X",
                            testClusterId,
                            attributeIdStart,
                            attributeIdEnd);
  sendReadAttribute();
}

void startTestCommand(SL_CLI_COMMAND_ARG)
{
  testClusterId = sl_cli_get_argument_uint16(arguments, 0);
  attributeIdStart = sl_cli_get_argument_uint16(arguments, 1);
  attributeIdEnd = sl_cli_get_argument_uint16(arguments, 2);
  clientToServer = (bool)sl_cli_get_argument_uint32(arguments, 3);
  if (attributeIdStart > attributeIdEnd) {
    sl_zigbee_af_core_println("Error:  Start ID must be less than or equal to end ID.");
    return;
  }

  if (testType == TEST_TYPE_READ_WRITE_ATTRIBUTES) {
    readWriteAttributeTest();
  } else if (testType == TEST_TYPE_BIG_ATTRIBUTE_READ) {
    bigReadCommand();
  } else {
    sl_zigbee_af_core_println("Error: Invalid test type %d", testType);
  }
}

#endif

static const char* gettAttributeTypeString(uint8_t attributeType)
{
  uint8_t i = 0;
  while (zclAttributeTypeToStringMap[i].string != NULL) {
    if (zclAttributeTypeToStringMap[i].type == attributeType) {
      return zclAttributeTypeToStringMap[i].string;
    }
    i++;
  }
  return unknownAttributeTypeString;
}

static const char* getErrorString(uint8_t status)
{
  uint8_t i = 0;
  while (zclStatusToStringMap[i].string != NULL) {
    if (zclStatusToStringMap[i].status == status) {
      return zclStatusToStringMap[i].string;
    }
    i++;
  }
  return unknownErrorString;
}

static void printResults(void)
{
  uint16_t i;
  uint16_t index = 0;
  sl_zigbee_af_core_println("Cluster: 0x%04X\n", testClusterId);
  sl_zigbee_af_core_println("Attr    Read                     Type                    Write");
  sl_zigbee_af_core_println("-------------------------------------------------------------------------------");
  for (i = attributeIdStart; i <= attributeIdEnd; i++) {
    sl_zigbee_af_core_println("0x%04X: 0x%02X [%s]  0x%02X [%s]  0x%02X [%s]",
                              i,
                              attributeResults[index].readResult,
                              getErrorString(attributeResults[index].readResult),
                              attributeResults[index].attributeType,
                              gettAttributeTypeString(attributeResults[index].attributeType),
                              attributeResults[index].writeResult,
                              getErrorString(attributeResults[index].writeResult));
    index++;
  }
}

static bool checkTestComplete(void)
{
  if ((attributeIdStart + currentIndex) > attributeIdEnd) {
    sl_zigbee_af_core_println("Done.\n");
    printResults();
    sl_zigbee_af_event_set_inactive(readWriteAttributesTimeoutEventControl);
    return true;
  }
  return false;
}

static void sendCommand(bool read)
{
  sl_zigbee_af_set_command_endpoints(sourceEndpoint, destEndpoint);
  sl_status_t status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, destAddress);
  sl_zigbee_af_core_println("%s Attr 0x%04X",
                            (read
                             ? "Read "
                             : "Write"),
                            attributeIdStart + currentIndex);
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_event_set_delay_qs(readWriteAttributesTimeoutEventControl,
                                    timeoutSeconds << 2);
  } else {
    sl_zigbee_af_core_println("Error: Failed to send command (0x%02X)", status);
  }
}

static void sendReadAttribute(void)
{
  uint8_t temp[2];
  temp[0] = LOW_BYTE(attributeIdStart + currentIndex);
  temp[1] = HIGH_BYTE(attributeIdStart + currentIndex);
  (void) sl_zigbee_af_fill_external_buffer((ZCL_GLOBAL_COMMAND
                                            | (clientToServer
                                               ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                               : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)),
                                           testClusterId,
                                           ZCL_READ_ATTRIBUTES_COMMAND_ID,
                                           "b",
                                           temp,
                                           2);
  sendCommand(true);
  readWriteState = READ_WRITE_TEST_STATE_READ_SENT;
}

static void sendWriteAttribute(uint8_t type,
                               uint8_t* data,
                               uint16_t dataLen)
{
  UNUSED_VAR(type);
  (void) sl_zigbee_af_fill_external_buffer((ZCL_GLOBAL_COMMAND
                                            | (clientToServer
                                               ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                               : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)),
                                           testClusterId, \
                                           ZCL_WRITE_ATTRIBUTES_COMMAND_ID,
                                           "b",
                                           data,
                                           dataLen);

  sendCommand(false);  // read?
  readWriteState = READ_WRITE_TEST_STATE_WRITE_SENT;
}

void sl_zigbee_af_test_harness_read_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                           uint8_t * buffer,
                                                           uint16_t bufLen)
{
  if (clusterId == testClusterId
      && readWriteState == READ_WRITE_TEST_STATE_READ_SENT) {
    uint8_t result = buffer[2];
    uint16_t attributeId = (buffer[0] + (buffer[1] << 8));
    if (attributeId != (currentIndex + attributeIdStart)) {
      // Might be an APS retry, ignore.
      return;
    }
    sl_zigbee_af_event_set_inactive(readWriteAttributesTimeoutEventControl);
    attributeResults[currentIndex].readResult = result;
    if (result == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      uint8_t type = buffer[3];
      attributeResults[currentIndex].attributeType = type;
      // A very crude way to change the attribute data without knowing
      // what we are writing.  We change the 0th bit of the first byte.
      // This is unlikely to be outside the valid range for the attribute
      // and yet is different than the current value.

      // The read attributes response is:
      //   Attribute ID (2-bytes)
      //   Status (1-byte)
      //   Type (1-byte)
      //   Value (variable)

      // We need to change it into a write attributes request:
      //   Attribute ID (2-bytes)
      //   Type (1-byte)
      //   Value (variable)

      // So we just shift the Attribute ID forward 1-byte to
      // avoid an overlapping memmove().  The size of the value
      // may be quite large, so better to let it stay put.

      buffer[1] = LOW_BYTE(attributeId);
      buffer[2] = HIGH_BYTE(attributeId);
      buffer[4] = (buffer[4] & 1
                   ? (buffer[4] & 0xFE)
                   : (buffer[4] | 0x01));
      sendWriteAttribute(type, &buffer[1], bufLen - 1);
    } else {
      currentIndex++;
      if (!checkTestComplete()) {
        sendReadAttribute();
      }
    }
  }
}

void sl_zigbee_af_test_harness_write_attributes_response_cb(sl_zigbee_af_cluster_id_t clusterId,
                                                            uint8_t * buffer,
                                                            uint16_t bufLen)
{
  UNUSED_VAR(bufLen);
  if (clusterId == testClusterId
      && readWriteState == READ_WRITE_TEST_STATE_WRITE_SENT) {
    uint16_t attributeId = (buffer[1] + (buffer[2] << 8));
    if (attributeId != (currentIndex + attributeIdStart)) {
      return;
    }
    sl_zigbee_af_event_set_inactive(readWriteAttributesTimeoutEventControl);
    attributeResults[currentIndex].writeResult = buffer[0];
    currentIndex++;
    if (!checkTestComplete()) {
      sendReadAttribute();
    }
  }
}

void sl_zigbee_af_test_harness_read_write_attributes_timeout_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  if (readWriteState == READ_WRITE_TEST_STATE_WRITE_SENT
      || readWriteState == READ_WRITE_TEST_STATE_READ_SENT) {
    sl_zigbee_af_core_println("Timeout %s attribute",
                              (readWriteState == READ_WRITE_TEST_STATE_WRITE_SENT
                               ? "writing"
                               : "reading"));
    currentIndex++;
  }

  if (!checkTestComplete()) {
    sendReadAttribute();
  }
}

static void bigReadCommand(void)
{
  uint16_t currentAttributeId;
  sl_status_t status;

  #if !defined(SL_CATALOG_ZIGBEE_FRAGMENTATION_PRESENT)
  sl_zigbee_af_core_println("Warning:  Fragmentation plugin not enabled.  Will not be able to fit many attributes.");
  #endif

  if (sli_zigbee_af_zcl_buffer_len < (SL_ZIGBEE_AF_ZCL_OVERHEAD
                                      + ((attributeIdEnd - attributeIdStart) << 1))) {
    sl_zigbee_af_core_println("Error:  Too many attributes (%d) to fit into super buffer (max attributes: %d)",
                              (attributeIdEnd + 1 - attributeIdStart),
                              // 3 bytes = Overhead
                              //   ZCL Frame control (1-byte)
                              //   ZCL Command ID (1-byte)
                              //   Sequence Number
                              (sli_zigbee_af_zcl_buffer_len - SL_ZIGBEE_AF_ZCL_OVERHEAD) >> 1); // each attribute is 2-bytes
    return;
  }

  (void) sl_zigbee_af_fill_external_buffer((ZCL_GLOBAL_COMMAND
                                            | (clientToServer
                                               ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                               : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)),
                                           testClusterId,
                                           ZCL_READ_ATTRIBUTES_COMMAND_ID,
                                           "");
  for (currentAttributeId = attributeIdStart;
       currentAttributeId <= attributeIdEnd
       && *sli_zigbee_af_response_length_ptr < sli_zigbee_af_zcl_buffer_len;
       currentAttributeId++) {
    sli_zigbee_af_zcl_buffer[(*sli_zigbee_af_response_length_ptr)++] = LOW_BYTE(currentAttributeId);
    sli_zigbee_af_zcl_buffer[(*sli_zigbee_af_response_length_ptr)++] = HIGH_BYTE(currentAttributeId);
  }

  sl_zigbee_af_set_command_endpoints(SOURCE_ENDPOINT, destEndpoint);
  status = sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                             destAddress);
  sl_zigbee_af_core_println("Sent read attributes for cluster 0x%04X, attributes: %d, status: 0x%02X",
                            testClusterId,
                            attributeIdEnd - attributeIdStart,
                            status);
}
