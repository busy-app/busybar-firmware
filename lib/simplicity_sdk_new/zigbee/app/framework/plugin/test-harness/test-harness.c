/***************************************************************************//**
 * @file
 * @brief Test harness code for validating the behavior of the key establishment
 *  cluster and modifying the normal behavior of App. Framework.
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

// this file contains all the common includes for clusters in the zcl-util
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"
#ifdef SL_CATALOG_ZIGBEE_CLI_PRESENT
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#endif // SL_CATALOG_ZIGBEE_CLI_PRESENT
#include "test-harness.h"
#ifdef SL_CATALOG_CLI_PRESENT
#include "test-harness-cli.h"
#endif

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "test-harness-config.h"
#if (SL_ZIGBEE_AF_PLUGIN_TEST_HARNESS_AUTO_REGISTRATION_START == 1)
#define AUTO_REGISTRATION_START
#endif
#ifdef SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT
#include "app/framework/plugin/key-establishment/key-establishment.h"
#if defined(ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_SERVER_COMMAND_ID)
#define ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_SERVER_COMMAND_ID
#elif defined(ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_CLIENT_COMMAND_ID)
#define ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_CLIENT_COMMAND_ID
#else
#error "ZCL TerminateKeyEstablishment Command ID not defined"
#endif
#endif // ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_SERVER_COMMAND_ID || ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_CLIENT_COMMAND_ID
#ifdef SL_CATALOG_ZIGBEE_PRICE_SERVER_PRESENT
#include "price-server.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_TRUST_CENTER_KEEPALIVE_PRESENT
#include "trust-center-keepalive.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_OTA_STORAGE_SIMPLE_RAM_PRESENT
#include "ota-storage-ram.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST_PRESENT
#include "app/framework/plugin/trust-center-nwk-key-update-broadcast/trust-center-nwk-key-update-broadcast.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_UNICAST_PRESENT
#include "app/framework/plugin/trust-center-nwk-key-update-unicast/trust-center-nwk-key-update-unicast.h"
#endif

#include "stack/include/source-route.h"

#if !defined(EZSP_HOST)
  #include "stack/include/cbke-crypto-engine.h"
#endif

#if !defined(EZSP_HOST)
extern uint8_t sli_zigbee_router_neighbor_table_size;
extern uint8_t sli_mac_lower_mac_get_radio_channel(uint8_t mac_index);
  #if defined(SL_ZIGBEE_TEST) || defined(SL_ZIGBEE_STACK_TEST_HARNESS)
    #define STACK_TEST_HARNESS
  #endif
#endif

#if defined (EZSP_HOST)
extern sl_status_t sl_zigbee_set_max_end_device_children(uint8_t maxChildren);
#endif

//------------------------------------------------------------------------------
// Globals

typedef uint8_t sli_zigbee_test_harness_mode_t;

#define MODE_NORMAL           0
#define MODE_CERT_MANGLE      1
#define MODE_OUT_OF_SEQUENCE  2
#define MODE_NO_RESOURCES     3
#define MODE_TIMEOUT          4
#define MODE_DELAY_CBKE       5
#define MODE_DEFAULT_RESPONSE 6
#define MODE_KEY_MANGLE       7

static const char * modeText[] = {
  "Normal",
  "Cert Mangle",
  "Out of Sequence",
  "No Resources",
  "Cause Timeout",
  "Delay CBKE operation",
  "Default Response",
  "Key Mangle"
};

// Workaround for EMAPPFWKV2-1559.
#ifndef UNUSED
  #define UNUSED
#endif

static sli_zigbee_test_harness_mode_t testHarnessMode = MODE_NORMAL;
static int8_t certLengthMod = 0;
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
static uint8_t respondToCommandWithOutOfSequence = ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID;
static int8_t keyLengthMod  = 0;
static uint8_t certNewByteValue   = 0;
static uint8_t certIndexToChange  = 0;
static uint8_t delayedCbkeOperation = CBKE_OPERATION_GENERATE_KEYS;
#endif
static uint8_t certIndexToCorrupt = 0;

#define CERT_MANGLE_NONE    0
#define CERT_MANGLE_LENGTH  1
#define CERT_MANGLE_ISSUER  2
#define CERT_MANGLE_CORRUPT 3
#define CERT_MANGLE_SUBJECT 4
#define CERT_MANGLE_VALUE   5

static const char * certMangleText[] = {
  "None",
  "Mangle Length",
  "Rewrite Issuer",
  "Corrupt Cert",
  "Change byte",
};

#if defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_TEST)
  #define DEFAULT_POLICY true
#else
  #define DEFAULT_POLICY false
#endif
bool sli_zigbee_af_key_establishment_policy_allow_new_key_entries = DEFAULT_POLICY;
bool sli_zigbee_af_test_harness_support_for_new_price_fields = true;

typedef uint8_t sli_zigbee_cert_mangle_type_t;

static sli_zigbee_cert_mangle_type_t certMangleType = CERT_MANGLE_NONE;

// Offset from start of ZCL header
//   - ZCL overhead (3 bytes)
//   - KE suite (2 bytes)
//   - Ephemeral Data Generate Time (1 byte)
//   - Confirm Key Generate Time (1 byte)
#define CERT_OFFSET_IN_INITIATE_KEY_ESTABLISHMENT_MESSAGE (3 + 2 + 1 + 1)

static uint8_t invalidEui64[] = {
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

extern uint32_t testHarnessChannelMask;
extern const uint32_t testHarnessOriginalChannelMask;

extern sl_status_t sl_zigbee_add_child(sl_802154_short_addr_t shortId,
                                       sl_802154_long_addr_t longId,
                                       uint8_t childType);
#if !defined(EZSP_HOST)
extern void sli_zigbee_set_stack_compliance_revision(uint8_t revision);
#endif

#define TEST_HARNESS_BACK_OFF_TIME_SECONDS 30

#define CBKE_KEY_ESTABLISHMENT_SUITE         0x0001   // per the spec

#define TEST_HARNESS_PRINT_PREFIX "TEST HARNESS: "

// Holds either the ephemeral public key or the 2 SMAC values.
// The SMAC values are the biggest piece of data.
static uint8_t* delayedData[SL_ZIGBEE_SMAC_SIZE * 2];
static bool stopRecursion = false;
static uint16_t cbkeDelaySeconds = 0;

extern sl_zigbee_af_event_t sl_zigbee_af_test_harness_read_write_attributes_timeout_event;
extern void sl_zigbee_af_test_harness_read_write_attributes_timeout_event_handler(sl_zigbee_af_event_t * event);
sl_zigbee_af_event_t sli_zigbee_af_key_establishment_test_harness_event;
#define testHarnessEventControl (&sli_zigbee_af_key_establishment_test_harness_event)
void sli_zigbee_af_key_establishment_test_harness_event_handler(sl_zigbee_af_event_t * event);

#ifdef SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT
// This is what is reported to the partner of key establishment.
uint16_t sli_zigbee_af_key_establishment_test_harness_generate_key_time = DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
uint16_t sli_zigbee_af_key_establishment_test_harness_confirm_key_time = DEFAULT_GENERATE_SHARED_SECRET_TIME_SECONDS;
uint16_t sli_zigbee_af_key_establishment_test_harness_advertised_generate_key_time = DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
#endif // SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT

#define NULL_CLUSTER_ID 0xFFFF
static sl_zigbee_af_cluster_id_t clusterIdRequiringApsSecurity = NULL_CLUSTER_ID;

#if defined(AUTO_REGISTRATION_START)
  #define AUTO_REG_START true
#else
  #define AUTO_REG_START false
#endif

bool sli_zigbee_af_test_harness_allow_registration = AUTO_REG_START;

//------------------------------------------------------------------------------

static void testHarnessPrintVarArgs(const char * format,
                                    va_list vargs)
{
  sl_zigbee_af_core_flush();
  sl_zigbee_af_core_print(TEST_HARNESS_PRINT_PREFIX);
  (void) local_vprintf(format, vargs);
  sl_zigbee_af_core_flush();
}

static void testHarnessPrintln(const char * format,
                               ...)
{
  va_list vargs;
  va_start(vargs, format);
  testHarnessPrintVarArgs(format, vargs);
  va_end(vargs);
  sl_zigbee_core_debug_print("\r\n");
}

static void resetTimeouts(void)
{
#ifdef SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT
  sli_zigbee_af_key_establishment_test_harness_generate_key_time = DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
  sli_zigbee_af_key_establishment_test_harness_confirm_key_time = DEFAULT_GENERATE_SHARED_SECRET_TIME_SECONDS;
  sli_zigbee_af_key_establishment_test_harness_advertised_generate_key_time = DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
#endif // SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT
}

#if defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST_PRESENT) \
  && defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_UNICAST_PRESENT)

static bool unicastKeyUpdate = false;

extern sl_status_t sl_zigbee_af_trust_center_start_unicast_network_key_update(void);
extern sl_status_t sl_zigbee_af_trust_center_start_broadcast_network_key_update(void);
// This function is renamed in the broadcast and unicast plugins, but is still
// called directly by the periodic key update plugin, and in one or two other
// places, so we route it through here to use the appropriate method.
sl_status_t sl_zigbee_af_trust_center_start_network_key_update(void)
{
  sl_status_t status;
  sl_zigbee_app_debug_println("Using %s key update method",
                              (unicastKeyUpdate
                               ? "unicast"
                               : "broadcast"));
  status = unicastKeyUpdate ? sl_zigbee_af_trust_center_start_unicast_network_key_update()
           : sl_zigbee_af_trust_center_start_broadcast_network_key_update();

  sl_zigbee_app_debug_println("%s: %s: 0x%02X",
                              TEST_HARNESS_PRINT_NAME,
                              "Network Key Update",
                              status);
  return status;
}
#endif

//------------------------------------------------------------------------------
// Misc. forward declarations

#if defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC_PRESENT)
extern sl_zigbee_af_event_t sl_zigbee_af_trust_center_nwk_key_update_periodic_my_event;
#endif // SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC_PRESENT

#if defined(STACK_TEST_HARNESS)
void sli_zigbee_test_harness_beacon_suppress_set(bool enable);
uint8_t sli_zigbee_test_harness_beacon_suppress_get(void);
#endif

void sli_zigbee_af_test_harness_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_network_event_init(&sli_zigbee_af_key_establishment_test_harness_event,
                                  sli_zigbee_af_key_establishment_test_harness_event_handler);
  sl_zigbee_af_network_event_init(&sl_zigbee_af_test_harness_read_write_attributes_timeout_event,
                                  sl_zigbee_af_test_harness_read_write_attributes_timeout_event_handler);
}
void sli_zigbee_af_key_establishment_test_harness_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(testHarnessEventControl);
  testHarnessPrintln("Test Harness Event Handler fired, Tick: 0x%08X",
                     halCommonGetInt32uMillisecondTick());

  stopRecursion = true;
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
  testHarnessPrintln("Generating %s callback.",
                     ((delayedCbkeOperation == CBKE_OPERATION_GENERATE_KEYS
                       || delayedCbkeOperation == CBKE_OPERATION_GENERATE_KEYS_283K1)
                      ? "sl_zigbee_generate_cbke_keys_handler()"
                      : "sl_zigbee_calculate_smacs_handler()"));

  if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_KEYS) {
    sli_zigbee_af_key_establishment_generate_cbke_keys_handler(SL_STATUS_OK,
                                                               (sl_zigbee_public_key_data_t*)delayedData);
  } else if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_KEYS_283K1) {
    sli_zigbee_af_key_establishment_generate_cbke_keys_handler283k1(SL_STATUS_OK,
                                                                    (sl_zigbee_public_key_283k1_data_t*)delayedData);
  } else if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_SECRET) {
    sli_zigbee_af_key_establishment_calculate_smacs_handler(SL_STATUS_OK,
                                                            (sl_zigbee_smac_data_t*)delayedData,
                                                            (sl_zigbee_smac_data_t*)(delayedData + SL_ZIGBEE_SMAC_SIZE));
  } else if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_SECRET_283K1) {
    sli_zigbee_af_key_establishment_calculate_smacs_handler283k1(SL_STATUS_OK,
                                                                 (sl_zigbee_smac_data_t*)delayedData,
                                                                 (sl_zigbee_smac_data_t*)(delayedData + SL_ZIGBEE_SMAC_SIZE));
  } else {
    testHarnessPrintln("Test Harness Event Handler: Unknown operation 0x%08X", delayedCbkeOperation);
  }
#endif

  stopRecursion = false;
}

bool sli_zigbee_af_key_establishment_test_harness_cbke_callback(uint8_t cbkeOperation,
                                                                uint8_t* data1,
                                                                uint8_t* data2)
{
  if (testHarnessMode != MODE_DELAY_CBKE) {
    return false;
  }

  if (stopRecursion) {
    return false;
  }

  testHarnessPrintln("Delaying %s key callback for %d seconds",
                     ((cbkeOperation == CBKE_OPERATION_GENERATE_KEYS
                       || cbkeOperation == CBKE_OPERATION_GENERATE_KEYS_283K1)
                      ? "ephemeral"
                      : "confirm"),
                     cbkeDelaySeconds);

  if (cbkeOperation == CBKE_OPERATION_GENERATE_KEYS) {
    memmove(delayedData, data1, sizeof(sl_zigbee_public_key_data_t));
  } else if (cbkeOperation == CBKE_OPERATION_GENERATE_KEYS_283K1) {
    memmove(delayedData, data1, sizeof(sl_zigbee_public_key_283k1_data_t));
  } else {
    memmove(delayedData, data1, sizeof(sl_zigbee_smac_data_t));
  }

  if (data2 != NULL) {
    memmove(delayedData + sizeof(sl_zigbee_smac_data_t),
            data2,
            sizeof(sl_zigbee_smac_data_t));
  }
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
  delayedCbkeOperation = cbkeOperation;
#endif

  testHarnessPrintln("Test Harness Tick: 0x%08X",
                     halCommonGetInt32uMillisecondTick());
  uint32_t delayMs = (((uint32_t)(cbkeDelaySeconds))
                      * MILLISECOND_TICKS_PER_SECOND);
  if (delayMs <= SL_ZIGBEE_MAX_EVENT_DELAY_MS) {
    sl_zigbee_af_event_set_delay_ms(testHarnessEventControl, delayMs);
  }
  return true;
}

bool sli_zigbee_af_key_establishment_test_harness_message_send_callback(uint8_t message)
{
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
  uint8_t i;
  uint8_t* ptr = (appResponseData
                  + CERT_OFFSET_IN_INITIATE_KEY_ESTABLISHMENT_MESSAGE);
  uint8_t direction = (*appResponseData & ZCL_FRAME_CONTROL_DIRECTION_MASK);

  if (MODE_NORMAL == testHarnessMode) {
    return true;
  }

  if (testHarnessMode == MODE_CERT_MANGLE) {
    if (message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID) {
      if (certMangleType == CERT_MANGLE_LENGTH) {
        if (certLengthMod > 0) {
          ptr = appResponseData + appResponseLength;
          for (i = 0; i < certLengthMod; i++) {
            *ptr = i;
            ptr++;
          }
        }
        appResponseLength += certLengthMod;

        testHarnessPrintln("Mangling certificate length by %s%d bytes",
                           (certLengthMod > 0
                            ? "+"
                            : ""),
                           certLengthMod);
      } else if (certMangleType == CERT_MANGLE_ISSUER
                 || certMangleType == CERT_MANGLE_SUBJECT) {
        testHarnessPrintln("appRespLen=%d, certLenMod=%d", appResponseLength, certLengthMod);
        ptr += (certMangleType == CERT_MANGLE_ISSUER
                ? ((appResponseLength == 81) ? CERT_ISSUER_OFFSET_283K1 : CERT_ISSUER_OFFSET)
                : ((appResponseLength == 81) ? CERT_SUBJECT_OFFSET_283K1 : CERT_SUBJECT_OFFSET));

        memmove(ptr, invalidEui64, EUI64_SIZE);
        testHarnessPrintln("Mangling certificate %s to be (>)%02X%02X%02X%02X%02X%02X%02X%02X",
                           (certMangleType == CERT_MANGLE_ISSUER
                            ? "issuer"
                            : "subject"),
                           invalidEui64[0],
                           invalidEui64[1],
                           invalidEui64[2],
                           invalidEui64[3],
                           invalidEui64[4],
                           invalidEui64[5],
                           invalidEui64[6],
                           invalidEui64[7]);
      } else if (certMangleType == CERT_MANGLE_CORRUPT) {
        // We bit flip the byte to make sure it is different than
        // its real value.
        ptr += certIndexToCorrupt;
        *ptr = ~(*ptr);
        testHarnessPrintln("Mangling byte at index %d in certificate.",
                           certIndexToCorrupt);
      } else if (certMangleType == CERT_MANGLE_VALUE) {
        ptr += certIndexToChange;
        *ptr = certNewByteValue;
        testHarnessPrintln("Changing byte at index %u to %u in certificate.",
                           certIndexToChange, certNewByteValue);
      }
    } else if (message == ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID) {
      if (certMangleType == CERT_MANGLE_CORRUPT || certMangleType == CERT_MANGLE_VALUE) {
        // To simulate that the test harness has NOT cancelled
        // key establishment due to a problem with calculating
        // or comparing the SMAC (acting as KE server), we
        // send a confirm key message instead.  The local server
        // has really cancelled KE but we want to test that the other
        // side will cancel it and send its own terminate message.
        ptr = appResponseData + 2; // jump over frame control and sequence num
        *ptr++ = ZCL_CONFIRM_KEY_DATA_RESPONSE_COMMAND_ID;
        for (i = 0; i < SL_ZIGBEE_SMAC_SIZE; i++) {
          *ptr++ = i;
        }
        appResponseLength = (SL_ZIGBEE_AF_ZCL_OVERHEAD
                             + SL_ZIGBEE_SMAC_SIZE);
        testHarnessPrintln("Rewriting Terminate Message as Confirm Key Message");
      }
    }
  } else if (testHarnessMode == MODE_OUT_OF_SEQUENCE) {
    if (message == respondToCommandWithOutOfSequence) {
      uint8_t *ptr = appResponseData + 2;  // ZCL Frame control and sequence number

      testHarnessPrintln("Sending out-of-sequence message");

      ptr[0] = (message == ZCL_CONFIRM_KEY_DATA_REQUEST_COMMAND_ID
                ? ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID
                : (message + 1u));

      // Setting the outgoing message to the right length without really
      // filling the message with valid data means there would
      // probably be garbage or bad data.  However the receiving device should
      // check for an invalid command ID first before parsing the specific
      // fields in the message.
      appResponseLength = SL_ZIGBEE_AF_ZCL_OVERHEAD + sli_zigbee_af_key_establish_message_to_data_size[message];
    }
  } else if (testHarnessMode == MODE_NO_RESOURCES) {
    // If we are the client then we have to wait until after the first
    // message to send the 'no resources' message.
    if (!(direction == ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
          && message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID)) {
      uint8_t *ptr = appResponseData + 2;  // ZCL Frame control and sequence number
      *ptr++ = ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID;
      *ptr++ = SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES;
      *ptr++ = TEST_HARNESS_BACK_OFF_TIME_SECONDS;
      *ptr++ = LOW_BYTE(sli_zigbee_af_current_cbke_suite);
      *ptr++ = HIGH_BYTE(sli_zigbee_af_current_cbke_suite);

      appResponseLength = ptr - appResponseData;

      testHarnessPrintln("Sending Terminate: NO_RESOURCES");
    }
  } else if (testHarnessMode == MODE_DEFAULT_RESPONSE) {
    // If we are the client then we have to wait until after the first
    // message to send the Default Response message.
    if (!(direction == ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
          && message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID)) {
      uint8_t *ptr = appResponseData;
      uint8_t oldCommandId;
      *ptr++ = direction;
      ptr++; // skip sequence number, it was already written correctly.

      oldCommandId = *ptr;
      *ptr++ = ZCL_DEFAULT_RESPONSE_COMMAND_ID;

      // If we are the client, we send a response to the previous command ID.
      // If we are the server, we send a response to the current command ID.
      // This works because the client -> server and server -> client commands
      // are identical.  The client code will be sending the NEXT command ID,
      // which we are rewriting into a default response.  The server is sending
      // a command ID for the previous client command (a response ID to the
      // request), which has the same command ID as we are rewriting.
      *ptr++ = (direction == ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                ? oldCommandId - 1
                : oldCommandId);
      *ptr++ = SL_ZIGBEE_ZCL_STATUS_FAILURE;

      appResponseLength = ptr - appResponseData;

      testHarnessPrintln("Sending Default Response: FAILURE");
    }
  } else if (testHarnessMode == MODE_TIMEOUT) {
    // If we are the client then we have to wait until after the first
    // message to induce a timeout.
    if (!(direction == ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
          && message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID)) {
      testHarnessPrintln("Suppressing message to induce timeout.");
      return false;
    }
  } else if (testHarnessMode == MODE_KEY_MANGLE) {
    //Change the length of the emphemeral Key.
    if (message == ZCL_EPHEMERAL_DATA_REQUEST_COMMAND_ID) {
      if (keyLengthMod > 0) {
        ptr = appResponseData + appResponseLength;
        for (i = 0; i < keyLengthMod; i++) {
          *ptr = i;
          ptr++;
        }
      }
      appResponseLength += keyLengthMod;

      testHarnessPrintln("Mangling key length by %s%d bytes",
                         (keyLengthMod > 0
                          ? "+"
                          : ""),
                         keyLengthMod);
    }
  }

  // Send the message
  return true;
#else
  UNUSED_VAR(message);
#endif
  return false;
}

#if defined(SL_CATALOG_CLI_PRESENT)

void sl_zigbee_af_test_harness_cert_mangle_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar = sl_cli_get_command_string(arguments, position)[0];
  uint8_t secondChar  = sl_cli_get_command_string(arguments, position)[1];
  testHarnessMode = MODE_CERT_MANGLE;

  if (commandChar == 'l') {
    certMangleType = CERT_MANGLE_LENGTH;
    certLengthMod = (int8_t)sl_cli_get_argument_int8(arguments, 0);
  } else if (commandChar == 'i') {
    certMangleType = CERT_MANGLE_ISSUER;
    sl_zigbee_copy_eui64_arg(arguments, 0, invalidEui64, false);
  } else if (commandChar == 'c' ) {
    if (secondChar == 'o') {
      certMangleType = CERT_MANGLE_CORRUPT;
      certIndexToCorrupt = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
    } else {
      certMangleType = CERT_MANGLE_VALUE;
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
      certIndexToChange  = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
      certNewByteValue   = (uint8_t)sl_cli_get_argument_uint8(arguments, 1);
#endif
    }
  } else if (commandChar == 's' ) {
    certMangleType = CERT_MANGLE_SUBJECT;
    sl_zigbee_copy_eui64_arg(arguments, 0, invalidEui64, false);
  } else {
    testHarnessPrintln("Error: Unknown command.");
    return;
  }

  // Reset this value back to its normal value in case
  // it was set.
  resetTimeouts();
}

void sl_zigbee_af_test_harness_key_establishment_set_mode_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar0 = sl_cli_get_command_string(arguments, position)[0];
  uint8_t commandChar2   = sl_cli_get_command_string(arguments, position)[2];

  if (commandChar0 == 'o') {
    testHarnessMode = MODE_OUT_OF_SEQUENCE;
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
    respondToCommandWithOutOfSequence = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
#endif
  } else if (commandChar0 == 'n') {
    if (commandChar2 == '-') {
      testHarnessMode = MODE_NO_RESOURCES;
    } else if (commandChar2 == 'r') {
      testHarnessMode = MODE_NORMAL;
    } else if (commandChar2 == 'w') {
      testHarnessMode = MODE_NORMAL;
      sli_zigbee_af_key_establishment_policy_allow_new_key_entries =
        (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
    }
  } else if (commandChar0 == 't') {
    testHarnessMode = MODE_TIMEOUT;
  } else if (commandChar2 == 'l' ) {  // delay-cbke
    uint16_t delay = sl_cli_get_argument_uint16(arguments, 0);
    testHarnessMode = MODE_DELAY_CBKE;
    cbkeDelaySeconds = delay;
#ifdef SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT
    uint16_t advertisedDelay = sl_cli_get_argument_uint16(arguments, 1);
    sli_zigbee_af_key_establishment_test_harness_generate_key_time = delay;
    sli_zigbee_af_key_establishment_test_harness_confirm_key_time  = advertisedDelay;
    sli_zigbee_af_key_establishment_test_harness_advertised_generate_key_time = advertisedDelay;
#endif // SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT
    return;
  } else if (commandChar2 == 'f') { // default-resp
    testHarnessMode = MODE_DEFAULT_RESPONSE;
    return;
  } else if (commandChar0 == 'r') {
    testHarnessMode = MODE_NORMAL;
    sli_zigbee_af_test_harness_reset_aps_frame_counter();
  } else if (commandChar0 == 'a') {
    testHarnessMode = MODE_NORMAL;
    sli_zigbee_af_test_harness_advance_aps_frame_counter();
  } else {
    testHarnessPrintln("Error: Unknown command.");
    return;
  }

  resetTimeouts();
}

void sl_zigbee_af_test_harness_set_registration_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar1 = sl_cli_get_command_string(arguments, position)[1];
  if (commandChar1 == 'n') {
    sli_zigbee_af_test_harness_allow_registration = true;
  } else if (commandChar1 == 'f') {
    sli_zigbee_af_test_harness_allow_registration = false;
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}

void sl_zigbee_af_test_harness_set_aps_security_for_cluster_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar1 = sl_cli_get_command_string(arguments, position)[1];
  if (commandChar1 == 'n') {
    clusterIdRequiringApsSecurity = sl_cli_get_argument_uint16(arguments, 0);
  } else if (commandChar1 == 'f') {
    clusterIdRequiringApsSecurity = NULL_CLUSTER_ID;
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}

void sl_zigbee_af_test_harness_key_establishment_key_mangle_command(sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
  testHarnessMode = MODE_KEY_MANGLE;
  keyLengthMod = sl_cli_get_argument_int8(arguments, 0);
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_af_test_harness_key_establishment_select_suite_command(sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
  uint16_t suite = sl_cli_get_argument_uint16(arguments, 0);
  sli_zigbee_af_skip_check_supported_curves(suite);
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_af_test_harness_key_establishment_set_available_suite_command(sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT)
  uint16_t suite = sl_cli_get_argument_uint16(arguments, 0);
  sli_zigbee_af_set_available_curves(suite);
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_af_test_harness_status_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  sl_zigbee_af_key_establishment_cluster_print("Test Harness Mode: %s", modeText[testHarnessMode]);
  if (testHarnessMode == MODE_CERT_MANGLE) {
    sl_zigbee_af_key_establishment_cluster_println("");
    sl_zigbee_af_key_establishment_cluster_print("Cert Mangling Type: %s", certMangleText[certMangleType]);
    if (certMangleType == CERT_MANGLE_LENGTH) {
      sl_zigbee_af_key_establishment_cluster_print(" (%s%d bytes)",
                                                   ((certLengthMod > 0)
                                                    ? "+"
                                                    : ""),
                                                   certLengthMod);
    } else if (certMangleType == CERT_MANGLE_CORRUPT) {
      sl_zigbee_af_key_establishment_cluster_print(" (index: %d)",
                                                   certIndexToCorrupt);
    }
  } else if (testHarnessMode == MODE_DELAY_CBKE) {
    sl_zigbee_af_key_establishment_cluster_print(" (by %d seconds",
                                                 cbkeDelaySeconds);
  }
  sl_zigbee_af_key_establishment_cluster_println("");

  sl_zigbee_af_key_establishment_cluster_println("Auto SE Registration: %s",
                                                 (sli_zigbee_af_test_harness_allow_registration
                                                  ? "On"
                                                  : "Off"));
  sl_zigbee_af_key_establishment_cluster_print("Additional Cluster Security: ");
  if (clusterIdRequiringApsSecurity == NULL_CLUSTER_ID) {
    sl_zigbee_af_key_establishment_cluster_println("off");
  } else {
    sl_zigbee_af_key_establishment_cluster_println("0x%04X",
                                                   clusterIdRequiringApsSecurity);
  }

  sl_zigbee_af_key_establishment_cluster_println("Publish Price includes SE 1.1 fields: %s",
                                                 (sli_zigbee_af_test_harness_support_for_new_price_fields
                                                  ? "yes"
                                                  : "no"));
  sl_zigbee_af_key_establishment_cluster_flush();

#if defined(STACK_TEST_HARNESS)
  {
    uint8_t beaconsLeft = sli_zigbee_test_harness_beacon_suppress_get();
    sl_zigbee_af_key_establishment_cluster_print("Beacon Suppress: %s",
                                                 (beaconsLeft == 255
                                                  ? "Disabled "
                                                  : "Enabled "));
    if (beaconsLeft != 255) {
      sl_zigbee_af_key_establishment_cluster_print(" (%d left to be sent)",
                                                   beaconsLeft);
    }
    sl_zigbee_af_key_establishment_cluster_flush();
    sl_zigbee_af_key_establishment_cluster_println("");
  }
#endif

#if defined(SL_CATALOG_ZIGBEE_NETWORK_FIND_PRESENT)
  {
    sl_zigbee_af_key_establishment_cluster_print("Channel Mask: ");
    sl_zigbee_af_print_channel_list_from_mask(testHarnessChannelMask);
    sl_zigbee_af_key_establishment_cluster_println("");
  }
#endif
}

#ifndef SL_CATALOG_ZIGBEE_APS_LINK_KEY_AUTHORIZATION_PRESENT
bool sl_zigbee_af_cluster_security_custom_cb(sl_zigbee_af_profile_id_t profileId,
                                             sl_zigbee_af_cluster_id_t clusterId,
                                             bool incoming,
                                             uint8_t commandId)
{
  UNUSED_VAR(profileId);
  UNUSED_VAR(incoming);
  UNUSED_VAR(commandId);
  return (clusterIdRequiringApsSecurity != NULL_CLUSTER_ID
          && clusterId == clusterIdRequiringApsSecurity);
}
#endif // SL_CATALOG_ZIGBEE_APS_LINK_KEY_AUTHORIZATION_PRESENT

void sl_zigbee_af_test_harness_price_send_new_fields_command(sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_ZIGBEE_PRICE_SERVER_PRESENT)
  sli_zigbee_af_test_harness_support_for_new_price_fields = (bool)sl_cli_get_argument_uint32(arguments, 0);
#else
  UNUSED_VAR(arguments);
  testHarnessPrintln("No Price server plugin included.");
#endif
}

void sl_zigbee_af_test_harness_tc_keepalive_send_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
#if defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_KEEPALIVE_PRESENT)
  sli_zigbee_af_send_keepalive_signal();
#else
  testHarnessPrintln("No TC keepalive plugin included.");
#endif
}

void sl_zigbee_af_test_harness_tc_keepalive_start_stop_command(sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_KEEPALIVE_PRESENT)

  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar2 = sl_cli_get_command_string(arguments, position)[2];
  if (commandChar2 == 'o') {  // stop
    sl_zigbee_af_trust_center_keepalive_abort_cb();
  } else if (commandChar2 == 'a') { // start
    sl_zigbee_af_trust_center_keepalive_update_cb(true); // registration complete?
    // assume this is only called when device is done with that
  } else {
    testHarnessPrintln("Unknown keepalive command.");
  }

#else
  UNUSED_VAR(arguments);
  testHarnessPrintln("No TC keepalive plugin included.");
#endif
}

void sl_zigbee_af_test_harness_ota_image_mangle_command(sl_cli_command_arg_t *arguments)
{
#if defined (SL_CATALOG_ZIGBEE_OTA_STORAGE_SIMPLE_RAM_PRESENT)
  uint16_t index = sl_cli_get_argument_uint16(arguments, 0);
  if (index >= sli_zigbee_af_ota_storage_drive_get_image_size()) {
    testHarnessPrintln("Error: Index %d > image size of %d",
                       index,
                       sli_zigbee_af_ota_storage_drive_get_image_size());
  } else {
    sli_zigbee_af_ota_storage_driver_corrupt_image(index);
  }
#else
  UNUSED_VAR(arguments);
  testHarnessPrintln("No OTA Storage Simple RAM plugin included");
#endif
}

#if defined (SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST_PRESENT) \
  && defined(SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_UNICAST_PRESENT)
void sl_zigbee_af_test_harness_key_update_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar0 = sl_cli_get_command_string(arguments, position)[0];
  if (commandChar0 == 'u') {
    unicastKeyUpdate = true;
    sl_zigbee_app_debug_println("Key update set to unicast");
  } else if (commandChar0 == 'b') {
    unicastKeyUpdate = false;
    sl_zigbee_app_debug_println("Key update set to broadcast");
  } else if (commandChar0 == 'n') {
    sl_status_t status = sl_zigbee_af_trust_center_start_network_key_update();
    sl_zigbee_app_debug_println("Starting NWK Key update, status: 0x%02X", status);
  }
}
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
extern void sli_zigbee_set_trust_center_aps_encryption(sl_zigbee_tc_aps_encrypt_mode_t option);
void sl_zigbee_af_test_harness_key_update_security_command(sl_cli_command_arg_t *arguments)
{
#ifndef EZSP_HOST
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar1 = sl_cli_get_command_string(arguments, position)[1];
  if (commandChar1 == 'n') {
    sli_zigbee_set_trust_center_aps_encryption(TC_APS_ENCRYPT_ENABLE);
    sl_zigbee_app_debug_println("Key update security set to enable");
  } else if (commandChar1 == 'f') {
    sli_zigbee_set_trust_center_aps_encryption(TC_APS_ENCRYPT_DISABLE);
    sl_zigbee_app_debug_println("Key update security set to disable");
  } else if (commandChar1 == 'd') {
    sli_zigbee_set_trust_center_aps_encryption(TC_APS_ENCRYPT_DEFAULT);
    sl_zigbee_app_debug_println("Key update security set to default");
  }
#else
  (void) arguments;
  testHarnessPrintln("Not supported on host.");
#endif
}
#endif
#else

void sl_zigbee_af_test_harness_key_update_command(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_zigbee_app_debug_println("NWK Key Update Plugin not enabled.");
}
void sl_zigbee_af_test_harness_key_update_security_command(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_zigbee_app_debug_println("NWK Key Update Plugin not enabled.");
}

#endif // SL_CATALOG_ZIGBEE_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST/UNICAST

void sl_zigbee_af_test_harness_concentrator_start_stop_command(sl_cli_command_arg_t *arguments)
{
#if defined(SL_CATALOG_ZIGBEE_CONCENTRATOR_PRESENT)
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar2 = sl_cli_get_command_string(arguments, position)[2];
  if (commandChar2 == 'o') {
    sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_OFF);
  } else if (commandChar2 == 'a') {
    sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_ON);
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
#else
  (void)arguments;
#endif // SL_CATALOG_ZIGBEE_CONCENTRATOR_PRESENT
}

#if defined(STACK_TEST_HARNESS)
void sl_zigbee_af_test_harness_limit_beacons_on_off_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar1 = sl_cli_get_command_string(arguments, position)[1];

  if (commandChar1 == 'f') {
    sli_zigbee_test_harness_beacon_suppress_set(255);
  } else if (commandChar1 == 'n') {
    sli_zigbee_test_harness_beacon_suppress_set(1);
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}
// TODO: this should be modified once we've upgraded the generated CLI
// mechanism to conditionally generate CLI based on macros such as these
#elif defined(SL_ZIGBEE_AF_GENERATE_CLI)
void sl_zigbee_af_test_harness_limit_beacons_on_off_command(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
}
#endif

#if defined(SL_CATALOG_ZIGBEE_NETWORK_FIND_PRESENT)

void sl_zigbee_af_test_harness_channel_mask_add_or_remove_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar0 = sl_cli_get_command_string(arguments, position)[0];
  uint8_t channel = sl_cli_get_argument_uint8(arguments, 0);

  if (channel < 11 || channel > 26) {
    testHarnessPrintln("Error: Invalid channel '%d'.", channel);
    return;
  }
  if (commandChar0 == 'a') {
    testHarnessChannelMask |= (1 << channel);
  } else if (commandChar0 == 'r') {
    testHarnessChannelMask &= ~(1 << channel);
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}

void sl_zigbee_af_test_harness_channel_mask_reset_clear_all_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar0 = sl_cli_get_command_string(arguments, position)[0];

  if (commandChar0 == 'c') {
    testHarnessChannelMask = 0;
  } else if (commandChar0 == 'r') {
    testHarnessChannelMask = testHarnessOriginalChannelMask;
  } else if (commandChar0 == 'a') {
    testHarnessChannelMask = SL_ZIGBEE_ALL_802_15_4_CHANNELS_MASK;
  } else {
    testHarnessPrintln("Error: Unknown command.");
  }
}

#endif

void sl_zigbee_af_test_harness_enable_disable_endpoint_command(sl_cli_command_arg_t *arguments)
{
#if defined(EZSP_HOST)
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 0);
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar0 = sl_cli_get_command_string(arguments, position)[0];
  bool disable = commandChar0 == 'd';

  sl_zigbee_ezsp_set_endpoint_flags(endpoint,
                                    (disable
                                     ? SL_ZIGBEE_EZSP_ENDPOINT_DISABLED
                                     : SL_ZIGBEE_EZSP_ENDPOINT_ENABLED));
#else
  UNUSED_VAR(arguments);
  testHarnessPrintln("Unsupported on SOC.");

#endif
}

void sl_zigbee_af_test_harness_endpoint_status_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
#if defined(EZSP_HOST)
  uint8_t i;

  for (i = 0; i < sl_zigbee_af_endpoint_count(); i++) {
    uint8_t endpoint = sl_zigbee_af_endpoint_from_index(i);
    sl_zigbee_ezsp_endpoint_flags_t flags;
    sl_zigbee_ezsp_get_endpoint_flags(endpoint, &flags);
    testHarnessPrintln("EP %d, Flags 0x%04X [%s]",
                       endpoint,
                       flags,
                       ((flags & SL_ZIGBEE_EZSP_ENDPOINT_ENABLED)
                        ? "Enabled"
                        : "Disabled"));
#ifdef SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT
    if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_KEYS) {
      sli_zigbee_af_key_establishment_generate_cbke_keys_handler(SL_STATUS_OK,
                                                                 (sl_zigbee_public_key_data_t*)delayedData);
    } else if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_KEYS_283K1) {
      sli_zigbee_af_key_establishment_generate_cbke_keys_handler283k1(SL_STATUS_OK,
                                                                      (sl_zigbee_public_key_283k1_data_t*)delayedData);
    } else if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_SECRET) {
      sli_zigbee_af_key_establishment_calculate_smacs_handler(SL_STATUS_OK,
                                                              (sl_zigbee_smac_data_t*)delayedData,
                                                              (sl_zigbee_smac_data_t*)(delayedData + SL_ZIGBEE_SMAC_SIZE));
    } else if (delayedCbkeOperation == CBKE_OPERATION_GENERATE_SECRET_283K1) {
      sli_zigbee_af_key_establishment_calculate_smacs_handler283k1(SL_STATUS_OK,
                                                                   (sl_zigbee_smac_data_t*)delayedData,
                                                                   (sl_zigbee_smac_data_t*)(delayedData + SL_ZIGBEE_SMAC_SIZE));
    } else {
      testHarnessPrintln("Test Harness Event Handler: Unknown operation 0x%08X", delayedCbkeOperation);
    }
#endif // SL_CATALOG_ZIGBEE_KEY_ESTABLISHMENT_PRESENT
  }

#else
  testHarnessPrintln("Unsupported on SOC");

#endif
}

void sl_zigbee_af_test_harness_cluster_endpoint_index_command(sl_cli_command_arg_t *arguments)
{
  uint8_t endpoint, mask, clusterEndpointIndex;
  sl_zigbee_af_cluster_id_t clusterId;

  endpoint = sl_cli_get_argument_uint8(arguments, 0);
  clusterId = (sl_zigbee_af_cluster_id_t)sl_cli_get_argument_uint16(arguments, 1);
  mask = sl_cli_get_argument_uint8(arguments, 2);

  clusterEndpointIndex = (uint8_t)(mask == 0
                                   ? sl_zigbee_af_find_cluster_client_endpoint_index(endpoint, clusterId)
                                   : sl_zigbee_af_find_cluster_server_endpoint_index(endpoint, clusterId));

  testHarnessPrintln("endpoint: 0x%04X cluster: 0x%04X clusterEndpointIndex: 0x%04X %s",
                     endpoint,
                     clusterId,
                     clusterEndpointIndex,
                     ((mask == 0)
                      ? "(client)"
                      : "(server)"));
}

void sl_zigbee_af_test_harness_radio_on_off_command(sl_cli_command_arg_t *arguments)
{
  uint8_t position = sl_cli_get_command_count(arguments) - 1;
  uint8_t commandChar1 = sl_cli_get_command_string(arguments, position)[1];
  bool radioOff = (commandChar1 == 'f');
  sl_status_t status;
  if (radioOff) {
    status = sl_zigbee_start_scan(SL_ZIGBEE_START_RADIO_OFF_SCAN,
                                  0, // channels (doesn't matter)
                                  0); // duration (doesn't matter)

#ifdef SL_CATALOG_ZIGBEE_LIGHT_LINK_PRESENT
    if (sl_zigbee_zll_rx_on_when_idle_get_active()) {
      // Ensure radio doesn't get turned back on while it's supposed to be off.
      sl_zigbee_zll_cancel_rx_on_when_idle();
    }
#endif
  } else {
    status = sl_zigbee_stop_scan();
#ifdef SL_CATALOG_ZIGBEE_LIGHT_LINK_PRESENT
    sl_zigbee_zll_set_rx_on_when_idle(0xFFFFFFFF);
#endif
  }
  sl_zigbee_app_debug_println("Radio %s status: 0x%02X",
                              (radioOff ? "OFF" : "ON"),
                              status);
}

void sl_zigbee_af_test_harness_set_radio_power(sl_cli_command_arg_t *arguments)
{
  int8_t val = (int8_t)sl_cli_get_argument_int8(arguments, 0);
  sl_zigbee_set_radio_power(val);
  sl_zigbee_app_debug_println("radio power %d", val);
}

void sl_zigbee_af_test_harness_get_radio_channel(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
#ifndef EZSP_HOST
  uint8_t logicalChannel = sl_zigbee_get_radio_channel();
  uint8_t radioChannel = sli_mac_lower_mac_get_radio_channel(PHY_INDEX_NATIVE);
  sl_zigbee_af_app_println("%s %d %s %d", "Logical channel:", logicalChannel, "Radio channel:", radioChannel);
#else
  testHarnessPrintln("Not supported on host.");

#endif
}

void sl_zigbee_af_test_harness_add_child_command(sl_cli_command_arg_t *arguments)
{
#if !defined(EZSP_HOST)
  sl_802154_short_addr_t shortId;
  sl_802154_long_addr_t longId;
  uint8_t nodeType;
  sl_status_t status;
  shortId = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_copy_eui64_arg(arguments, 1, longId, false);
  nodeType = sl_cli_get_argument_uint16(arguments, 2);

  status = sl_zigbee_add_child(shortId, longId, nodeType);
  sl_zigbee_app_debug_println("status 0x%02X", status);
#else
  UNUSED_VAR(arguments);
#endif
}

#endif

#if defined(SL_CATALOG_CLI_PRESENT)
void sl_zigbee_af_test_harness_set_node_descriptor_compliance_revision(SL_CLI_COMMAND_ARG)
{
  uint8_t val = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
#ifdef EZSP_HOST
  sl_status_t status;
  status = sl_zigbee_ezsp_set_value(SL_ZIGBEE_EZSP_VALUE_ENABLE_R21_BEHAVIOR, 1, &val);
  if (status == SL_STATUS_OK) {
    sl_zigbee_app_debug_println("The compliance revision of the device has been changed to R%d (0x%02X)", val, status);
  }
#else
  sli_zigbee_set_stack_compliance_revision(val);
  sl_zigbee_app_debug_println("The compliance revision of the device has been changed to R%d", val);
#endif
}

void sl_zigbee_af_test_harness_set_max_children(SL_CLI_COMMAND_ARG)
{
  uint8_t maxChildren = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);

  sl_zigbee_set_max_end_device_children(maxChildren);

  sl_zigbee_app_debug_println("Set maximum children to %d", maxChildren);
}

void sl_zigbee_af_test_harness_set_neighbor_table_size(SL_CLI_COMMAND_ARG)
{
  // CAUTION : This function does not cause memory to be allocated toward the
  // neighbor table, therefore, the set value needs to be under the compile
  // time size (1,16,26 - depending on SL_ZIGBEE_NEIGHBOR_TABLE_SIZE)
  uint8_t neighborTableSize = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
#ifdef EZSP_HOST
  sl_status_t status;
  status = sl_zigbee_af_set_ezsp_config_value(SL_ZIGBEE_EZSP_CONFIG_NEIGHBOR_TABLE_SIZE,
                                              neighborTableSize,
                                              "neighbor table size");
  if (status != SL_STATUS_OK) {
    return;
  }
#else
  sli_zigbee_router_neighbor_table_size = neighborTableSize;
#endif
  sl_zigbee_app_debug_println("Set neighbor table size to %d", neighborTableSize);
}
#endif

//------------------------------------------------------------------------------
// Zcl response suppression commands

void sl_zigbee_suppress_cluster(sl_cli_command_arg_t *arguments)
{
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  uint16_t clusterId = (uint16_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t serverClient = (uint8_t)sl_cli_get_argument_uint8(arguments, 1);

  char *action = sl_zigbee_af_get_suppress_cluster(clusterId, serverClient) ? "unsuppress" : "suppress";
  sl_zigbee_af_status_t status = sl_zigbee_af_set_suppress_cluster(clusterId, serverClient);
  sl_zigbee_core_debug_println("%s clstr %d side %d: 0x%02X",
                               action,
                               clusterId,
                               serverClient,
                               status);
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_suppress_command(sl_cli_command_arg_t *arguments)
{
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  uint16_t clusterId = (uint16_t)sl_cli_get_argument_uint16(arguments, 0);
  uint8_t serverClient = (uint8_t)sl_cli_get_argument_uint8(arguments, 1);
  uint8_t commandId = (uint8_t)sl_cli_get_argument_uint8(arguments, 2);

  char *action = sl_zigbee_af_get_suppress_command(clusterId, serverClient, commandId) ? "unsuppress" : "suppress";
  sl_zigbee_af_status_t status = sl_zigbee_af_set_suppress_command(clusterId, serverClient, commandId);
  sl_zigbee_core_debug_println("%s clstr %d side %d cmd %d: 0x%02X",
                               action,
                               clusterId,
                               serverClient,
                               commandId,
                               status);
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_print_suppression_table(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_Z3_PRESENT
  sl_zigbee_af_print_suppression_table();
#endif
}
