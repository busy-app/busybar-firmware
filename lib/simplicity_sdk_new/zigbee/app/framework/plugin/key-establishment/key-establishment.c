/***************************************************************************//**
 * @file
 * @brief Routines for the Key Establishment plugin, which handles
 *        establishment of link keys per the Smart Energy Key Establishment
 *        cluster.
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
#include "app/framework/security/crypto-state.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "hal/hal.h"
#include "stack/include/zigbee-security-manager.h"

#include "stack/include/zigbee-security-manager.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
#include "test-harness.h"
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

#if !defined(EZSP_HOST)
  #include "stack/include/cbke-crypto-engine.h"
#endif
#include "key-establishment.h"
#include "key-establishment-storage.h"
#include "zap-cluster-command-parser.h"
#ifndef SL_ZIGBEE_SCRIPTED_TEST
#include "zigbee_af_cluster_functions.h"
#endif
#if defined(ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_SERVER_COMMAND_ID)
#define ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_SERVER_COMMAND_ID
#elif defined(ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_CLIENT_COMMAND_ID)
#define ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID ZCL_TERMINATE_KEY_ESTABLISHMENT_FROM_CLIENT_COMMAND_ID
#else
#error "ZCL TerminateKeyEstablishment Command ID not defined"
#endif

WEAK(bool sl_zigbee_af_key_establishment_inter_pan_cb(sl_zigbee_af_key_establishment_notify_message_t status,
                                                      bool amInitiator,
                                                      sl_802154_pan_id_t panId,
                                                      const sl_802154_long_addr_t eui64,
                                                      uint8_t delayInSeconds))
{
  UNUSED_VAR(status);
  UNUSED_VAR(amInitiator);
  UNUSED_VAR(panId);
  UNUSED_VAR(eui64);
  UNUSED_VAR(delayInSeconds);
  return true;
}

WEAK(bool sl_zigbee_af_key_establishment_event_cb(sl_zigbee_af_key_establishment_notify_message_t status,
                                                  bool amInitiator,
                                                  sl_802154_short_addr_t partnerShortId,
                                                  uint8_t delayInSeconds))
{
  UNUSED_VAR(status);
  UNUSED_VAR(amInitiator);
  UNUSED_VAR(partnerShortId);
  UNUSED_VAR(delayInSeconds);
  return true;
}

//------------------------------------------------------------------------------
// Globals

//#define FORCE_283K1_BEHAVIOR

#define LAST_KEY_ESTABLISH_EVENT INITIATOR_RECEIVED_CONFIRM_KEY

// These are initialized by the init routine.
static sli_zigbee_key_establish_event_t lastEvent;

typedef struct {
  sl_802154_long_addr_t eui64;
  sl_802154_pan_id_t panId;
} sli_zigbee_inter_pan_t;

typedef struct {
  sl_802154_short_addr_t nodeId;
  uint8_t       endpoint;
} IntraPan;

typedef union {
  sli_zigbee_inter_pan_t interPan;
  IntraPan intraPan;
} sli_802154_pan_data_t;

typedef struct {
  sli_802154_pan_data_t pan;
  bool isInitiator;
  bool isIntraPan;
  uint8_t sequenceNumber;
} sli_zigbee_key_establishment_partner_t;

static sli_zigbee_key_establishment_partner_t keyEstPartner;

static sl_802154_short_addr_t lastNodeSuccessfulCbke = SL_ZIGBEE_NULL_NODE_ID;
static uint8_t lastConfirmKeyDataResponseApsSequence;

sl_zigbee_af_event_t sl_zigbee_af_key_establishment_aps_duplicate_detection_event;
#define apsDuplicateDetectionEvent (&sl_zigbee_af_key_establishment_aps_duplicate_detection_event)
void sl_zigbee_af_key_establishment_aps_duplicate_detection_event_handler(sl_zigbee_af_event_t *event);

#define KEY_ESTABLISHMENT_TIMEOUT_BASE_SECONDS 10

//This is mask for the Valid Key Usage byte.
#define KEY_USAGE_BYTE 36
#define KEY_USAGE_BYTE_VALID_VALUE 0x08
#define TYPE_BYTE 0
#define TYPE_BYTE_VALID_VALUE 0x00
#define CURVE_BYTE 9
#define CURVE_BYTE_VALID_VALUE 0x0D
#define HASH_BYTE 10
#define HASH_BYTE_VALID_VALUE 0x08
// This is the delay time the local device will report when it sends the
// Terminate message
#define BACK_OFF_TIME_REPORTED_TO_PARTNER    30      // in seconds

// This is the delay time the local device will use when it encounters
// a failure that doesn't receive an explicit delay from the partner.
#define INTERNAL_ERROR_BACK_OFF_TIME         10      // in seconds

#if !defined(SL_ZIGBEE_AF_CUSTOM_KE_EPHEMERAL_DATA_GENERATE_TIME_SECONDS)
  #define SL_ZIGBEE_AF_CUSTOM_KE_EPHEMERAL_DATA_GENERATE_TIME_SECONDS \
  DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS
#endif

#if !defined(SL_ZIGBEE_AF_CUSTOM_KE_GENERATE_SHARED_SECRET_TIME_SECONDS)
  #define SL_ZIGBEE_AF_CUSTOM_KE_GENERATE_SHARED_SECRET_TIME_SECONDS \
  DEFAULT_GENERATE_SHARED_SECRET_TIME_SECONDS
#endif

#define EPHEMERAL_DATA_GENERATE_TIME_SECONDS   SL_ZIGBEE_AF_CUSTOM_KE_EPHEMERAL_DATA_GENERATE_TIME_SECONDS
#define GENERATE_SHARED_SECRET_TIME_SECONDS    SL_ZIGBEE_AF_CUSTOM_KE_GENERATE_SHARED_SECRET_TIME_SECONDS

// For purposes of testing timeouts, we allow redefining the advertised
// ephemeral data generate time separately from the actual
// ephemeral data generate time.
#if !defined(EM_AF_ADVERTISED_EPHEMERAL_DATA_GEN_TIME_SECONDS)
  #define EM_AF_ADVERTISED_EPHEMERAL_DATA_GEN_TIME_SECONDS EPHEMERAL_DATA_GENERATE_TIME_SECONDS
#endif

// Offsets within the payload of the Initiate Key request/response messages
#define EPHEMERAL_DATA_TIME_OFFSET 2
#define CONFIRM_KEY_TIME_OFFSET    3

sl_zigbee_af_cbke_key_establishment_suite_t sli_zigbee_af_available_cbke_suite = SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1;
sl_zigbee_af_cbke_key_establishment_suite_t sli_zigbee_af_current_cbke_suite   = SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1;
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
static sl_zigbee_af_cbke_key_establishment_suite_t sli_zigbee_af_use_test_harness_suite  = SL_ZIGBEE_AF_INVALID_KEY_ESTABLISHMENT_SUITE;
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

#if defined SL_ZIGBEE_TEST
sli_zigbee_key_establish_event_t timeoutState = NO_KEY_ESTABLISHMENT_EVENT;
#endif

#define CBKE_163K1_SUITE_OFFSET 0
#define CBKE_283K1_SUITE_OFFSET 4

static uint8_t cbkeSuiteOffset = CBKE_163K1_SUITE_OFFSET;
//The SMAC and Terminate sizes are duplicated to be able to easily select
//suite based sizes for the 163k and the 283k curve.
// This sli_zigbee_key_establish_message_t enum to message size.
const uint8_t sli_zigbee_af_key_establish_message_to_data_size[] = {
  EM_AF_KE_INITIATE_SIZE,
  EM_AF_KE_EPHEMERAL_SIZE,
  EM_AF_KE_SMAC_SIZE,
  EM_AF_KE_TERMINATE_SIZE,
  EM_AF_KE_INITIATE_SIZE_283K1,
  EM_AF_KE_EPHEMERAL_SIZE_283K1,
  EM_AF_KE_SMAC_SIZE,
  EM_AF_KE_TERMINATE_SIZE,
};

// This relates AppNotifyMesage enum to sl_zigbee_af_ami_key_establishment_status_t codes.
// A value of SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS means no terminate
// message is sent.
static const sl_zigbee_af_ami_key_establishment_status_t appNotifyMessageToKeyEstablishStatus[] = {
  // Success!
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // NO_APP_MESSAGE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // RECEIVED_PARTNER_CERTIFICATE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // GENERATING_EPHEMERAL_KEYS
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // GENERATING_SHARED_SECRET
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // KEY_GENERATION_DONE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // GENERATE_SHARED_SECRET_DONE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // LINK_KEY_ESTABLISHED

  // Failures
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES,       // NO_LOCAL_RESOURCES
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // PARTNER_NO_RESOURCES
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // TIMEOUT_OCCURRED
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // INVALID_APP_COMMAND
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // MESSAGE_SEND_FAILURE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // PARTNER_SENT_TERMINATE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_BAD_MESSAGE,        // INVALID_PARTNER_MESSAGE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_BAD_MESSAGE,        // PARTNER_SENT_DEFAULT_RESPONSE_ERROR

  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_UNKNOWN_ISSUER,     // BAD_CERTIFICATE_ISSUER
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_BAD_KEY_CONFIRM,    // KEY_CONFIRM_FAILURE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_UNSUPPORTED_SUITE,  // BAD_KEY_ESTABLISHMENT_SUITE
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES,       // KEY_TABLE_FULL
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES,       // NO_ESTABLISHMENT_ALLOWED
  SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_INVALID_KEY_USAGE,  // INVALID_CERTIFICATE_KEY_USAGE
};

#define sendEphemeralData(publicKey)                                   \
  (sendNextKeyEstablishMessage(ZCL_EPHEMERAL_DATA_RESPONSE_COMMAND_ID, \
                               (publicKey)))
#define sendConfirmKey(smac)                                            \
  (sendNextKeyEstablishMessage(ZCL_CONFIRM_KEY_DATA_REQUEST_COMMAND_ID, \
                               (smac)))

// This relates the sli_zigbee_key_establish_event_t enum to the timeouts for each event.
// We will setup the values when we receive the first message.
// Values in seconds.  The timeout values passed in the protocol are 8-bit
// but that means when we add our fudge factor it can overflow beyond
// 255.  So we make these 16-bit values to prevent problems.
static uint16_t eventTimeoutsSec[LAST_KEY_ESTABLISH_EVENT];

// This is the last endpoint that was initialized for key establishment
// it is the one we use for all our event scheduling
static uint8_t keyEstablishmentEndpoint = 0xFF;

// We record the sequence numbers of our partner device's messages so
// we can filter out dupes.  3 messages can be received during normal
// KE, plus 1 for a possible Terminate message.
#define NUM_SEQ_NUMBER 4
static uint8_t apsSequenceNumbersReceived = 0;
static uint8_t apsSequenceNumbers[NUM_SEQ_NUMBER];

static uint8_t certSubjectOffset = CERT_SUBJECT_OFFSET;
static uint8_t certIssuerOffset  = CERT_ISSUER_OFFSET;

static const char * terminateStatus[] = TERMINATE_STATUS_STRINGS;

// Over the air message lengths for Initiate Key Establishment Request and Response
// certificate + keyEstablishmentSuite + ephemeralDataGenerateTime + confirmKeyGenerateTime
#define INITIATE_KEY_ESTABLISHMENT_LENGTH_163K1 SL_ZIGBEE_CERTIFICATE_SIZE + 2 + 1 + 1
#define INITIATE_KEY_ESTABLISHMENT_LENGTH_283K1 SL_ZIGBEE_CERTIFICATE_283K1_SIZE + 2 + 1 + 1
// The fixed length of the Key Establishment commands.
#define INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_MIN_LENGTH INITIATE_KEY_ESTABLISHMENT_LENGTH_163K1
#define EPHEMERAL_DATA_REQUEST_COMMAND_MIN_LENGTH 22
#define CONFIRM_KEY_DATA_REQUEST_COMMAND_MIN_LENGTH 16
#define TERMINATE_KEY_ESTABLISHMENT_COMMAND_MIN_LENGTH 4

#define INITIATE_KEY_ESTABLISHMENT_RESPONSE_COMMAND_MIN_LENGTH INITIATE_KEY_ESTABLISHMENT_LENGTH_163K1
#define EPHEMERAL_DATA_RESPONSE_COMMAND_MIN_LENGTH 22
#define CONFIRM_KEY_DATA_RESPONSE_COMMAND_MIN_LENGTH 16
//------------------------------------------------------------------------------
// Forward declarations
static bool checkMalformed283k1Command(bool isCertificate);
static void checkSupportedCurve(void);
static bool checkKeyTable(uint8_t *bigEndianEui64);
static void keyEstablishStateMachine(sli_zigbee_key_establish_event_t newEvent,
                                     uint8_t *data1,
                                     uint8_t *data2);
static void clearKeyEstablishmentState(void);
static bool askApplicationWithDelay(sl_zigbee_af_key_establishment_notify_message_t message,
                                    uint8_t delayInSec);
#define askApplication(message) askApplicationWithDelay((message), 0)
static void sendKeyEstablishMessage(sli_zigbee_key_establish_message_t message);
static void sendTerminateMessage(sl_zigbee_af_ami_key_establishment_status_t status,
                                 uint8_t backOffTime);
static sl_zigbee_af_key_establishment_notify_message_t verifySmac(sl_zigbee_smac_data_t *otherSmac);
static bool checkRequestedSuite(uint8_t *suiteLittleEndian);

static void setupEventTimeouts(uint8_t theirGenerateKeyTimeSeconds,
                               uint8_t theirConfirmKeyTimeSeconds);
static void messageSentHandler(sl_zigbee_outgoing_message_type_t type,
                               uint16_t indexOrDestination,
                               sl_zigbee_aps_frame_t *apsFrame,
                               uint16_t msgLen,
                               uint8_t *message,
                               sl_status_t status);
static bool commandIsFromOurPartner(const sl_zigbee_af_cluster_command_t *cmd);
static bool setPartnerFromCommand(const sl_zigbee_af_cluster_command_t *cmd);
static void writeKeyEstablishmentClusterAttribute(uint8_t endpoint);
static void debugPrintSmac(bool initiatorSmac, uint8_t *smac);
static void debugPrintOtherSmac(bool received, uint8_t *smac);
static void debugPrintCert(bool initiatorCert, uint8_t *cert);
void sli_zigbee_af_print_data283k1(uint8_t *buffer, uint8_t size, unsigned char *type);
static void debugPrintKey(bool initiatorKey, uint8_t *key);

#if defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_TEST)

  #if defined(SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT)
// Test code only
    #define NEW_KEY_TABLE_ENTRY_ALLOWED (sli_zigbee_af_key_establishment_policy_allow_new_key_entries)

  #else
    #define NEW_KEY_TABLE_ENTRY_ALLOWED true
  #endif

#elif defined(SL_ZIGBEE_AF_HAS_SECURITY_PROFILE_SE_FULL)
  #define NEW_KEY_TABLE_ENTRY_ALLOWED false

#else
  #error SE Security not configuration defined
#endif

//------------------------------------------------------------------------------
// Internal Functions
static void checkSupportedCurve(void)
{
  // This function should only get called for Intra-Pan Key Establishment. We currently
  // don't support the read attributes stage for Inter-Pan Key Establishment.
  uint16_t attributeIds;
  attributeIds = ZCL_KEY_ESTABLISHMENT_SUITE_SERVER_ATTRIBUTE_ID;
  sl_zigbee_af_fill_command_global_client_to_server_read_attributes(ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                                    &attributeIds,
                                                                    sizeof(attributeIds));
  sl_zigbee_af_set_command_endpoints(keyEstablishmentEndpoint, keyEstPartner.pan.intraPan.endpoint);
  sl_zigbee_af_send_command_unicast(SL_ZIGBEE_OUTGOING_DIRECT, keyEstPartner.pan.intraPan.nodeId);
}

static bool checkValidByteValue283k1(uint8_t byte, uint8_t requiredValue, uint8_t mask)
{
  if (isCbkeKeyEstablishmentSuite283k1()) {
    if ((byte & mask) != requiredValue) {
      sl_zigbee_af_key_establishment_cluster_println("Error: Invalid byte in cert: byteValue %u requiredValue %u", byte, requiredValue);
      cleanupAndStop(INVALID_CERTIFICATE_KEY_USAGE);
      return false;
    }
  }
  return true;
}

static bool checkKeyTable(uint8_t *bigEndianEui64)
{
  sl_802154_long_addr_t eui64;
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;
  sl_zigbee_sec_man_init_context(&context);

  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
  sl_zigbee_sec_man_get_aps_key_info(&context, &key_data);

  sl_util_reverse_mem_copy(eui64, bigEndianEui64, EUI64_SIZE);

  sl_zigbee_af_key_establishment_cluster_print("Checking key table for IEEE ");
  sl_zigbee_af_key_establishment_cluster_debug_exec(sl_zigbee_af_print_big_endian_eui64(eui64));
  sl_zigbee_af_key_establishment_cluster_println("");

  int test = (key_data.bitmask & SL_ZIGBEE_KEY_HAS_PARTNER_EUI64);

  if (keyEstPartner.isIntraPan
      && keyEstPartner.pan.intraPan.nodeId == SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    if ((key_data.bitmask & SL_ZIGBEE_KEY_HAS_PARTNER_EUI64)
        && memcmp(eui64, context.eui64, EUI64_SIZE) == 0) {
      // The key to be updated is our existing TC Link Key, therefore
      // we have room.
      return true;
    }
    sl_zigbee_af_key_establishment_cluster_println("Error: EUI64 of TC does not match its cert. %d", test);
    sl_zigbee_af_key_establishment_cluster_println("Error: EUI64 of TC does not match its cert.");
    cleanupAndStop(INVALID_PARTNER_MESSAGE);
    return false;
  } else {
    // We are the trust center
    // or at least act as one in a direct CBKE between two non-TC devices

    // We either have an existing link key entry that we can update,
    // or there is an empty entry.
    sl_zigbee_sec_man_context_t context_existing;
    sl_zigbee_sec_man_context_t context_open;
    sl_zigbee_sec_man_export_link_key_by_eui(eui64, &context_existing, NULL, NULL);
    sl_zigbee_sec_man_export_link_key_by_eui(NULL, &context_open, NULL, NULL);
    if (context_existing.key_index != 0xFF) {
      return true;
    } else if (NEW_KEY_TABLE_ENTRY_ALLOWED != true) {
      sl_zigbee_af_key_establishment_cluster_println("Error: Unknown EUI64 trying to perform CBKE.");
      cleanupAndStop(INVALID_PARTNER_MESSAGE);
      return false;
    } else if (0xFF != context_open.key_index) {
      return true;
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }

    sl_zigbee_af_key_establishment_cluster_println("Error: Key table full.");
    cleanupAndStop(KEY_TABLE_FULL);
    return false;
  }
}

// data1 will used for the initiate key metadata, ephemeral public key, the
//   initiator locally calculated smac, or the partner's received smac.
// data2 will used for the partner certificate or for the responder locally
//   calculated smac.

static void keyEstablishStateMachine(sli_zigbee_key_establish_event_t newEvent,
                                     uint8_t *data1,
                                     uint8_t *data2)
{
  // The state machine is called by the public APIs, library callbacks, and
  // command handlers.  Most of the processing in this state machine only cares
  // whether the next event is a result of a received command.
  sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();

  if (cmd != NULL && cmd->type >= SL_ZIGBEE_INCOMING_MULTICAST) {
    sl_zigbee_af_key_establishment_cluster_println("Ignoring Broadcast KE message");
    return;
  }

  sl_zigbee_af_key_establishment_cluster_println("Key Establish State Machine event:%d",
                                                 newEvent);

  if (cmd != NULL) {
    // First catch any duplicates of the ConfirmKeyDataResponse message that
    // could be sent as APS retries by the neighboring device
    if ((cmd->interPanHeader == NULL)
        && (lastNodeSuccessfulCbke == cmd->source)
        && (lastConfirmKeyDataResponseApsSequence == cmd->apsFrame->sequence)
        && (newEvent == INITIATOR_RECEIVED_CONFIRM_KEY)) {
      sl_zigbee_af_key_establishment_cluster_println("Got duplicate APS message "
                                                     "(seq:%d), dropping!",
                                                     cmd->apsFrame->sequence);
      scriptTestCheckpoint("Key Establishment dropped duplicate ConfirmKeyData "
                           "frame");
      return;
    } else if (commandIsFromOurPartner(cmd)
               && (keyEstPartner.isInitiator || lastEvent != NO_KEY_ESTABLISHMENT_EVENT)) {
      // Filter out duplicate APS messages.

      // Edge Case: If the same partner initiates key establishment with us and
      // they happen to use one of the same APS sequence numbers as the last
      // time, this will fail since we assume it is a duplicate message.  The
      // hope is that the partner will retry and it should succeed.
      uint8_t i;
      for (i = 0; i < apsSequenceNumbersReceived; i++) {
        if (cmd->apsFrame->sequence == apsSequenceNumbers[i]) {
          sl_zigbee_af_key_establishment_cluster_println("Got duplicate APS message "
                                                         "(seq:%d), dropping!",
                                                         cmd->apsFrame->sequence);
          return;
        }
      }
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }

    // If we got here and we're not currently doing key establishment, it means
    // we are starting key establishment with a new partner.  Remember the new
    // partner and clear the previous set of sequence numbers.  We must handle
    // the case where the same partner is initiating key establishment with us
    // that did it last time.  We can't use the above else clause to do that.
    if (lastEvent == NO_KEY_ESTABLISHMENT_EVENT) {
      if (!setPartnerFromCommand(cmd)) {
        return;
      }
      apsSequenceNumbersReceived = 0;
    }

    // Remember the received APS sequence numbers so we can filter duplicates.
    if (keyEstPartner.isIntraPan) {
      apsSequenceNumbers[apsSequenceNumbersReceived] = cmd->apsFrame->sequence;
      apsSequenceNumbersReceived++;
    }

    // Remember the received ZCL sequence number for the response.
    if (keyEstPartner.isInitiator) {
      keyEstPartner.sequenceNumber = cmd->seqNum;
    }
  }

  // If we receive an unexpected message, we terminate and hope the partner
  // tries again.
  //CHECK_SUPPORTED_CURVES is only done by the initiator. Therefore, the server
  // will see a jump of 2 for BEGIN_KEY_ESTABLISHMENT
  if (newEvent != lastEvent + 1) {
    if (keyEstPartner.isInitiator && lastEvent == NO_KEY_ESTABLISHMENT_EVENT) {
      if (newEvent != BEGIN_KEY_ESTABLISHMENT) {
        sl_zigbee_af_key_establishment_cluster_println("Got wrong message in the sequence. last event %u, new event %u", lastEvent, newEvent);
        scriptTestCheckpoint("Got wrong message in CBKE sequence");
        cleanupAndStop(INVALID_PARTNER_MESSAGE);
        return;
      }
    } else {
      sl_zigbee_af_key_establishment_cluster_println("Got wrong message in the sequence. last event %u, new event %u", lastEvent, newEvent);
      cleanupAndStop(INVALID_PARTNER_MESSAGE);
      return;
    }
  }

#if defined SL_ZIGBEE_TEST
  // Testing only
  if (timeoutState != NO_KEY_ESTABLISHMENT_EVENT && timeoutState <= newEvent) {
    sl_zigbee_af_key_establishment_cluster_println("Dropping message to induce timeout in partner.");
    cleanupAndStop(TIMEOUT_OCCURRED);
    return;
  }
#endif

  switch (newEvent) {
    case CHECK_SUPPORTED_CURVES:
    {
      checkSupportedCurve();
      scriptTestCheckpoint("CBKE: check supported curves");
      break;
    }
    // For initiator, send the cert wait for reply.
    // For responder, we received the cert.
    case BEGIN_KEY_ESTABLISHMENT:
    {
      sl_zigbee_af_key_establishment_notify_message_t result = NO_APP_MESSAGE;
      sl_zigbee_af_key_establishment_cluster_println("Current Suite %u", sli_zigbee_af_current_cbke_suite);
      if (keyEstPartner.isInitiator) {
        if (!checkRequestedSuite(data1)
            || !checkIssuer(data2 + certIssuerOffset)
            || !checkKeyTable(data2 + certSubjectOffset)
            || !checkValidByteValue283k1(*(data2 + KEY_USAGE_BYTE), KEY_USAGE_BYTE_VALID_VALUE, KEY_USAGE_BYTE_VALID_VALUE)
            || !checkValidByteValue283k1(*(data2 + TYPE_BYTE), TYPE_BYTE_VALID_VALUE, 0xFF)
            || !checkValidByteValue283k1(*(data2 + CURVE_BYTE), CURVE_BYTE_VALID_VALUE, 0xFF)
            || !checkValidByteValue283k1(*(data2 + HASH_BYTE), HASH_BYTE_VALID_VALUE, 0xFF)) {
          debugPrintCert(true, data2);
          // For all of the above functions, a terminate message is
          // sent with the appropriate error code.
          return;
        }

        //TO DO: Print 283k1 cert here.
        debugPrintCert(true, data2);

        if (!askApplication(RECEIVED_PARTNER_CERTIFICATE)
            || !storePublicPartnerData(true, // certificate?
                                       data2)) {
          result = NO_LOCAL_RESOURCES;
        } else {
          setupEventTimeouts(data1[EPHEMERAL_DATA_TIME_OFFSET],
                             data1[CONFIRM_KEY_TIME_OFFSET]);
        }
      }
      if (result == NO_APP_MESSAGE) {
        result = sendCertificate();
      }
      if (result != NO_APP_MESSAGE) {
        // We now accept a keyEstablishment attribute of 0 and have
        // to watch out for this additional error case.
        // In case our device doesn't have the suite that was incorrectly
        // agreed on, we don't want to send a Terminate Key Establishment
        // message in response to a read attributes response.
        if (!keyEstPartner.isInitiator) {
          result = BAD_KEY_ESTABLISHMENT_SUITE;
        }
        cleanupAndStop(result);
        return;
      } else {
        scriptTestCheckpoint("CBKE: sent CBKE certificate");
      }
      break;
    }

    // For initiator, we received responder cert it is time to generate keys.
    // For responder, we received ephemeral data it is time to generate keys.
    case GENERATE_KEYS:
      if (!keyEstPartner.isInitiator) {
        if (!checkRequestedSuite(data1)
            || !checkIssuer(data2 + certIssuerOffset)
            || !checkKeyTable(data2 + certSubjectOffset)
            || !checkValidByteValue283k1(*(data2 + KEY_USAGE_BYTE), KEY_USAGE_BYTE_VALID_VALUE, KEY_USAGE_BYTE_VALID_VALUE)
            || !checkValidByteValue283k1(*(data2 + TYPE_BYTE), TYPE_BYTE_VALID_VALUE, 0xFF)
            || !checkValidByteValue283k1(*(data2 + CURVE_BYTE), CURVE_BYTE_VALID_VALUE, 0xFF)
            || !checkValidByteValue283k1(*(data2 + HASH_BYTE), HASH_BYTE_VALID_VALUE, 0xFF)) {
          // For all of the above functions, a terminate message is
          // sent with the appropriate error code.
          debugPrintCert(true, data2);
          return;
        }
        setupEventTimeouts(data1[EPHEMERAL_DATA_TIME_OFFSET],
                           data1[CONFIRM_KEY_TIME_OFFSET]);

        debugPrintCert(false, data2);
      } else {
        debugPrintKey(true, data1);
      }

      if (!askApplication(GENERATING_EPHEMERAL_KEYS)
          || !storePublicPartnerData(!keyEstPartner.isInitiator, // certificate?
                                     (!keyEstPartner.isInitiator
                                      ? data2            // partner cert
                                      : data1))          // partner key
          || sli_zigbee_af_generate_cbke_keys_for_curve() != SL_STATUS_IN_PROGRESS) {
        cleanupAndStop(NO_LOCAL_RESOURCES);
        return;
      }
      scriptTestCheckpoint("CBKE: beginning crypto operation");
      sli_zigbee_af_set_crypto_operation_in_progress();
      break;

    // For both roles, we are done generating keys.  Send the message.
    case SEND_EPHEMERAL_DATA_MESSAGE:
      if (!askApplication(KEY_GENERATION_DONE)) {
        cleanupAndStop(NO_LOCAL_RESOURCES);
        return;
      }
      scriptTestCheckpoint("CBKE: sending ephemeral data");
      sendEphemeralData(data1);
      break;

    // For initiator, we received ephemeral data resp.  Generate shared secret.
    // For responder, we received confirm key request.  Generate shared secret.
    case GENERATE_SHARED_SECRET:
    {
      sl_zigbee_certificate_283k1_data_t partnerCert;
      sl_zigbee_public_key_283k1_data_t partnerEphemeralPublicKey;

      if (!keyEstPartner.isInitiator) {
        debugPrintKey(false, data1);
      } else {
        debugPrintOtherSmac(true, data1);
      }
      // For the initiator this is slightly ineffecient because we store
      // the public key but then immediately retrieve it.  However it
      // saves on flash to treat responder and initiator the same.
      if (!askApplication(GENERATING_SHARED_SECRET)
          || (!keyEstPartner.isInitiator
              ? !storePublicPartnerData(false, data1)
              : !storeSmac((sl_zigbee_smac_data_t *)data1))
          || !retrieveAndClearPublicPartnerData(&partnerCert,
                                                &partnerEphemeralPublicKey)
          || (SL_STATUS_IN_PROGRESS
              != sli_zigbee_af_calculate_smacs_for_curve(!keyEstPartner.isInitiator,
                                                         &partnerCert,
                                                         &partnerEphemeralPublicKey))) {
        cleanupAndStop(NO_LOCAL_RESOURCES);
        return;
      }
      scriptTestCheckpoint("CBKE: begin crypto operation from ephemeral data");
      sli_zigbee_af_set_crypto_operation_in_progress();
      break;
    }

    // For both roles, we are done generating shared secret,
    //   send confirm key message.
    case SEND_CONFIRM_KEY_MESSAGE:
    {
      sl_zigbee_smac_data_t *initiatorSmac = (sl_zigbee_smac_data_t *)data1;
      sl_zigbee_smac_data_t *responderSmac = (sl_zigbee_smac_data_t *)data2;

      debugPrintSmac(true, sl_zigbee_smac_contents(initiatorSmac));
      debugPrintSmac(false, sl_zigbee_smac_contents(responderSmac));

      if (!askApplication(GENERATE_SHARED_SECRET_DONE)
          || (!keyEstPartner.isInitiator && !storeSmac(responderSmac))) {
        cleanupAndStop(NO_LOCAL_RESOURCES);
        return;
      }

      if (keyEstPartner.isInitiator) {
        sl_zigbee_af_key_establishment_notify_message_t result = verifySmac(initiatorSmac);
        if (result != NO_APP_MESSAGE) {
          cleanupAndStop(result);
          return;
        }
      }

      sendConfirmKey(sl_zigbee_smac_contents(!keyEstPartner.isInitiator
                                             ? initiatorSmac
                                             : responderSmac));

      scriptTestCheckpoint("CBKE: sent confirm key data");
      break;
    }

    case INITIATOR_RECEIVED_CONFIRM_KEY:
    {
      sl_zigbee_af_key_establishment_notify_message_t message = verifySmac((sl_zigbee_smac_data_t *)data1);
      debugPrintOtherSmac(true, data1);

      if (keyEstPartner.isIntraPan) {
        // Store this the partner node ID and the APS sequence associated with
        // the ConfirmKeyDataResponse, so that an APS retry of it doesn't cause
        // a TerminateKeyResponse be sent in response (once CBKE completes here,
        // an APS retry of the ConfirmKeyDataResponse treated as a brand new
        // attempt at CBKE)
        // We put a timer on this APS duplicate detection mechanism, in the case
        // that a valid packet, whose APS counter has wrapped, is received later
        lastNodeSuccessfulCbke = keyEstPartner.pan.intraPan.nodeId;
        if (cmd != NULL) {
          lastConfirmKeyDataResponseApsSequence = cmd->apsFrame->sequence;
        }
        sl_zigbee_af_event_set_delay_ms(
          apsDuplicateDetectionEvent,
          KEY_ESTABLISHMENT_APS_DUPLICATE_DETECTION_TIMEOUT_SEC << 10);
      }

      // For both success and failure, stop key establishment.
      cleanupAndStop(message == NO_APP_MESSAGE
                     ? LINK_KEY_ESTABLISHED
                     : message);

      return;
    }

    default:
      // Unknown message, ignore.
      return;
  }

  {
    uint32_t timeMs = (uint32_t)(eventTimeoutsSec[newEvent]) * MILLISECOND_TICKS_PER_SECOND;
    sl_zigbee_zcl_schedule_server_tick(keyEstablishmentEndpoint,
                                       ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                       timeMs);
  }
  lastEvent = newEvent;
  return;
}

static void clearKeyEstablishmentState(void)
{
  keyEstPartner.isInitiator = true;
  //sl_zigbee_af_key_establishment_cluster_println("keyEstPartner.isInitiator clearKeyState %u",keyEstPartner.isInitiator);
  lastEvent = NO_KEY_ESTABLISHMENT_EVENT;
  clearAllTemporaryPublicData();
  sl_zigbee_clear_temporary_data_maybe_store_link_key_for_curve(false);
  sl_zigbee_zcl_deactivate_server_tick(keyEstablishmentEndpoint,
                                       ZCL_KEY_ESTABLISHMENT_CLUSTER_ID);

  // NOTE: When clearing the state, we intentionally retain information about
  // the partner (e.g., node id, APS sequence numbers, etc.).  That information
  // is used to help determine if the a received message is a duplicate of one
  // we already processed.  Because we may receive a retried message after we
  // have completed key establishment, we must remember the partner information.
}

static bool askApplicationWithDelay(sl_zigbee_af_key_establishment_notify_message_t message,
                                    uint8_t delayInSec)
{
  const char * appNotifyText[] = APP_NOTIFY_MESSAGE_TEXT;

  if (message == NO_APP_MESSAGE) {
    return true;
  }

  sl_zigbee_af_key_establishment_cluster_flush();
  sl_zigbee_af_key_establishment_cluster_print("%s: %s %s: %s (%d), %s ",
                                               (!keyEstPartner.isInitiator
                                                ? "Initiator"
                                                : "Responder"),
                                               "Key Establish",
                                               (message >= APP_NOTIFY_ERROR_CODE_START
                                                ? "Error"
                                                : (message == LINK_KEY_ESTABLISHED
                                                   ? "Success"
                                                   : "Notify")),
                                               appNotifyText[message],
                                               message,
                                               "keyEstPartner");
  if (keyEstPartner.isIntraPan) {
    sl_zigbee_af_key_establishment_cluster_println("0x%04X", keyEstPartner.pan.intraPan.nodeId);
    return sl_zigbee_af_key_establishment_event_cb(message,
                                                   !keyEstPartner.isInitiator,
                                                   keyEstPartner.pan.intraPan.nodeId,
                                                   delayInSec);
  } else {
    sl_zigbee_af_key_establishment_cluster_debug_exec(sl_zigbee_af_print_big_endian_eui64(keyEstPartner.pan.interPan.eui64));
    sl_zigbee_af_key_establishment_cluster_println("");
    return sl_zigbee_af_key_establishment_inter_pan_cb(message,
                                                       !keyEstPartner.isInitiator,
                                                       keyEstPartner.pan.interPan.panId,
                                                       keyEstPartner.pan.interPan.eui64,
                                                       delayInSec);
  }
}

static void sendKeyEstablishMessage(sli_zigbee_key_establish_message_t message)
{
  // It is assumed the "appResponseData" has already been filled
  // with data and the "appResponseLength" has been set appropriately
  // including the message overhead that will be filled in by this function.
  uint8_t *ptr = appResponseData;
  *ptr++ = (ZCL_CLUSTER_SPECIFIC_COMMAND
            | (!keyEstPartner.isInitiator
               ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
               : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT));
  // When the Initiator is sending Initiate Key Establishment Request, it needs
  // the update the frame with the next ZCL sequence number. Else, if it is
  // responding to its partner then the use the partner sequence number.
  // The flag keyEstPartner.isInitiator means partner is initiator.
  if (keyEstPartner.isInitiator) {
    *ptr++ = keyEstPartner.sequenceNumber;
  } else {
    *ptr++ = sl_zigbee_af_next_sequence();
  }
  *ptr   = message;

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  if (!sli_zigbee_af_key_establishment_test_harness_message_send_callback(message)) {
    return;
  }
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

  if (keyEstPartner.isIntraPan) {
    sl_zigbee_aps_frame_t apsFrame;
    apsFrame.clusterId = ZCL_KEY_ESTABLISHMENT_CLUSTER_ID;
    apsFrame.sourceEndpoint = keyEstablishmentEndpoint;
    apsFrame.destinationEndpoint = keyEstPartner.pan.intraPan.endpoint;
    apsFrame.options = (SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS | SL_ZIGBEE_APS_OPTION_RETRY);
    sl_zigbee_af_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                              keyEstPartner.pan.intraPan.nodeId,
                              &apsFrame,
                              appResponseLength,
                              appResponseData);
  } else {
    sl_zigbee_af_send_inter_pan(keyEstPartner.pan.interPan.panId,
                                keyEstPartner.pan.interPan.eui64,
                                SL_ZIGBEE_NULL_NODE_ID,
                                0, // multicast id - unused
                                ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                SE_PROFILE_ID,
                                appResponseLength,
                                appResponseData);
  }
}

static void sendTerminateMessage(sl_zigbee_af_ami_key_establishment_status_t status,
                                 uint8_t backOffTime)
{
  uint8_t *ptr = appResponseData + SL_ZIGBEE_AF_ZCL_OVERHEAD;
  *ptr++ = status;
  *ptr++ = backOffTime;

  // Since we only support the single key establishment suite, we can
  // get away with casting this into an uint8_t and simply zeroing the high bits.
  *ptr++ = LOW_BYTE(sli_zigbee_af_current_cbke_suite);
  *ptr++ = HIGH_BYTE(sli_zigbee_af_current_cbke_suite);

  appResponseLength = (ptr - appResponseData);

  sendKeyEstablishMessage(ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID);
}

static sl_zigbee_af_key_establishment_notify_message_t verifySmac(sl_zigbee_smac_data_t *otherSmac)
{
  // For the initiator, the stored SMAC will be the responder's version
  //   locally calculated.
  // For the responder, the stored SMAC will be the initiator's version
  //   received via the Confirm Key request message.
  sl_zigbee_smac_data_t *ptr;
  if (!getSmacPointer(&ptr)) {
    return NO_LOCAL_RESOURCES;
  }

  sl_zigbee_af_key_establishment_cluster_println("My local SMAC:");
  debugPrintOtherSmac(false, sl_zigbee_smac_contents(ptr));

  return (memcmp(sl_zigbee_smac_contents(ptr), otherSmac, SL_ZIGBEE_SMAC_SIZE) == 0
          ? NO_APP_MESSAGE        // success
          : KEY_CONFIRM_FAILURE);
}

static bool checkRequestedSuite(uint8_t *suiteLittleEndian)
{
  uint16_t requestedSuite;
  requestedSuite = suiteLittleEndian[0] + ((uint16_t)(suiteLittleEndian[1]) << 8);
  if (requestedSuite == SL_ZIGBEE_AF_INVALID_KEY_ESTABLISHMENT_SUITE) {
    sl_zigbee_af_key_establishment_cluster_println("Bad Key Establishment Suite");
    cleanupAndStop(BAD_KEY_ESTABLISHMENT_SUITE);
    return false;
  }
  return true;
}

static void setupEventTimeouts(uint8_t theirGenerateKeyTimeSeconds,
                               uint8_t theirConfirmKeyTimeSeconds)
{
  uint8_t i;
  eventTimeoutsSec[0] = 0;  // NO_KEY_ESTABLISHMENT_EVENT

  for (i = 1; i < LAST_KEY_ESTABLISH_EVENT; i++) {
    eventTimeoutsSec[i] = KEY_ESTABLISHMENT_TIMEOUT_BASE_SECONDS;
  }
  eventTimeoutsSec[BEGIN_KEY_ESTABLISHMENT]     += (!keyEstPartner.isInitiator
                                                    ? 0
                                                    : theirGenerateKeyTimeSeconds);
  eventTimeoutsSec[GENERATE_KEYS]               += EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
  eventTimeoutsSec[GENERATE_SHARED_SECRET]      += GENERATE_SHARED_SECRET_TIME_SECONDS;
  eventTimeoutsSec[SEND_EPHEMERAL_DATA_MESSAGE] += (!keyEstPartner.isInitiator
                                                    ? theirGenerateKeyTimeSeconds
                                                    : theirConfirmKeyTimeSeconds);
  // Only initiator needs this timeout while waiting for the event
  // INITIATER_RECEIVED_CONFIRM_KEY_MESSAGE.  Responder ends KE when
  // getting to this event.
  eventTimeoutsSec[SEND_CONFIRM_KEY_MESSAGE]    += theirConfirmKeyTimeSeconds;
}

static void messageSentHandler(sl_zigbee_outgoing_message_type_t type,
                               uint16_t indexOrDestination,
                               sl_zigbee_aps_frame_t *apsFrame,
                               uint16_t msgLen,
                               uint8_t *message,
                               sl_status_t status)
{
  UNUSED_VAR(type);
  UNUSED_VAR(apsFrame);
  UNUSED_VAR(msgLen);
  UNUSED_VAR(message);
  if (!keyEstPartner.isIntraPan
      || lastEvent == NO_KEY_ESTABLISHMENT_EVENT
      || keyEstPartner.pan.intraPan.nodeId != indexOrDestination) {
    // Unknown APS Ack, or an Ack for key establishment not in progress.
    return;
  }

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_key_establishment_cluster_println("Error: Failed to send key establish message to 0x%04X, status: 0x%02X",
                                                   indexOrDestination,
                                                   status);
    cleanupAndStop(MESSAGE_SEND_FAILURE);
    return;
  } else {
    if (lastEvent == SEND_CONFIRM_KEY_MESSAGE
        && keyEstPartner.isInitiator) {
      // Success!
      cleanupAndStop(LINK_KEY_ESTABLISHED);
      return;
    }
  }
}

// This function should only return false for inter-PAN commands that do not
// include the source long address or if there is no endpoint for the inter-PAN
// network.
static bool setPartnerFromCommand(const sl_zigbee_af_cluster_command_t *cmd)
{
  keyEstPartner.isInitiator = true;
  //sl_zigbee_af_key_establishment_cluster_println("keyEstPartner.isInitiator setPartnerFromCommand %u",keyEstPartner.isInitiator);
  keyEstPartner.isIntraPan = (cmd->interPanHeader == NULL);
  if (keyEstPartner.isIntraPan) {
    keyEstPartner.pan.intraPan.nodeId = cmd->source;
    keyEstPartner.pan.intraPan.endpoint = cmd->apsFrame->sourceEndpoint;
    keyEstablishmentEndpoint = cmd->apsFrame->destinationEndpoint;
  } else {
    if (!(cmd->interPanHeader->options
          & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)) {
      return false;
    }
    keyEstPartner.pan.interPan.panId = cmd->interPanHeader->panId;
    memmove(keyEstPartner.pan.interPan.eui64,
            cmd->interPanHeader->longAddress,
            EUI64_SIZE);
    keyEstablishmentEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();
    if (keyEstablishmentEndpoint == 0xFF) {
      return false;
    }
  }
  return true;
}

static bool commandIsFromOurPartner(const sl_zigbee_af_cluster_command_t *cmd)
{
  // For intra-PAN commands, we should check that the source and destination
  // endpoints and the sequence numbers of the request/response pairs match.
  // However, devices in the field are sometimes not careful about endpoints
  // or sequence numbers, so strictness here would cause interoperability
  // problems.  Neither endpoint nor sequence number mismatches are likely to
  // cause serious problems in practice because Key Establishment is intended
  // to be device-wide rather than per-endpoint and we only support a single
  // partner at a time, so checking our partner and correlating requests and
  // responses is not difficult.

  // We generally make sure that the direction bit is set correctly, but for a
  // terminate command we do not.  This is due to timeouts and the fact that
  // our 4.3 stack will always assume that if it is not in the middle of
  // key establishment, then it is the server.  In that case it will send the
  // Terminate message with the direction set 'server-to-client' even if
  // the request was also 'server-to-client'.
  return (((keyEstPartner.isInitiator
            == (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER))
           || (cmd->commandId == ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID))
          && keyEstPartner.isIntraPan == (cmd->interPanHeader == NULL)
          && (keyEstPartner.isIntraPan
              ? keyEstPartner.pan.intraPan.nodeId == cmd->source
              : ((cmd->interPanHeader->options
                  & SL_ZIGBEE_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)
                 && (memcmp(keyEstPartner.pan.interPan.eui64,
                            cmd->interPanHeader->longAddress,
                            EUI64_SIZE) == 0))));
}

static void writeKeyEstablishmentClusterAttribute(uint8_t endpoint)
{
  if (isCbkeKeyEstablishmentSuiteValid()) {
    uint16_t keSuiteId = sli_zigbee_af_available_cbke_suite;
    sl_zigbee_af_status_t attrWriteStatus;

    // Make the 'mask' 16-bit so that we can bit shift the client mask (0x80)
    // and detect it is bigger than 0x80 so our for() loop terminates.
    uint16_t mask;
    for (mask = CLUSTER_MASK_SERVER; mask <= CLUSTER_MASK_CLIENT; mask <<= 1) {
      attrWriteStatus = sl_zigbee_af_write_attribute(endpoint,
                                                     ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                     // Conveniently the server and client attribute
                                                     // ID are the same.  So we pass just the server ID.
                                                     ZCL_KEY_ESTABLISHMENT_SUITE_SERVER_ATTRIBUTE_ID,
                                                     (uint8_t)mask,
                                                     (uint8_t*)&keSuiteId,
                                                     ZCL_ENUM16_ATTRIBUTE_TYPE);
      if (attrWriteStatus != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
        sl_zigbee_af_key_establishment_cluster_println("%s%s%s%02X",
                                                       "Key Est. Suite attr write failed (",
                                                       ((mask == CLUSTER_MASK_SERVER)
                                                        ? "server"
                                                        : "client"),
                                                       "): 0x",
                                                       attrWriteStatus);
      }
    }
  }
}
//------------------------------------------------------------------------------
// DEBUG

static void debugPrintSmac(bool initiatorSmac, uint8_t *smac)
{
  sl_zigbee_af_key_establishment_cluster_println("%s SMAC",
                                                 (initiatorSmac
                                                  ? "Initiator"
                                                  : "Responder"));
  // Conveniently Zigbee Keys are the same length as the SMAC
  sl_zigbee_af_print_zigbee_key(smac);
}

static void debugPrintOtherSmac(bool received, uint8_t *smac)
{
  sl_zigbee_af_key_establishment_cluster_println("%s SMAC",
                                                 (received ? "Received" : "Calculated"));
  // Conveniently Zigbee Keys are the same length as the SMAC
  sl_zigbee_af_print_zigbee_key(smac);
}

static void debugPrintCert(bool initiatorCert, uint8_t *cert)
{
  sl_zigbee_af_key_establishment_cluster_println("%s Cert",
                                                 (initiatorCert
                                                  ? "Initiator"
                                                  : "Responder"));
  if (isCbkeKeyEstablishmentSuite163k1()) {
    sl_zigbee_af_print_cert(cert);
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    sl_zigbee_af_print_cert_283k1(cert);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

static void debugPrintKey(bool initiatorKey, uint8_t *key)
{
  sl_zigbee_af_key_establishment_cluster_println("%s public key",
                                                 (initiatorKey
                                                  ? "Initiator"
                                                  : "Responder"));
  if (isCbkeKeyEstablishmentSuite163k1()) {
    sl_zigbee_af_print_public_key(key);
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    sl_zigbee_af_print_key_283k1(true, key);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

static sl_status_t initiateKeyEstablishment(const sl_802154_long_addr_t eui64,
                                            uint16_t nodeIdOrPanId,
                                            uint8_t endpoint)
{
  uint8_t validLastEvent = NO_KEY_ESTABLISHMENT_EVENT;
  if (isCbkeKeyEstablishmentSuiteValid() && lastEvent == NO_KEY_ESTABLISHMENT_EVENT) {
    keyEstablishmentEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();
    if (keyEstablishmentEndpoint == 0xFF) {
      return SL_STATUS_INVALID_STATE;
    }
    keyEstPartner.isInitiator = false;
    //sl_zigbee_af_key_establishment_cluster_println("keyEstPartner.isInitiator initiateKeyEstablishment %u",keyEstPartner.isInitiator);
    keyEstPartner.isIntraPan = (eui64 == NULL);
    if (keyEstPartner.isIntraPan) {
      keyEstPartner.pan.intraPan.nodeId = nodeIdOrPanId;
      keyEstPartner.pan.intraPan.endpoint = endpoint;
    } else {
      keyEstPartner.pan.interPan.panId = nodeIdOrPanId;
      memmove(keyEstPartner.pan.interPan.eui64, eui64, EUI64_SIZE);
    }
    keyEstPartner.sequenceNumber = sl_zigbee_af_next_sequence();
    apsSequenceNumbersReceived = 0;

    // For inter-PAN key establishment, we don't read the supported curves and
    // use the support that the user specified for the image
    if (!keyEstPartner.isIntraPan) {
      lastEvent = CHECK_SUPPORTED_CURVES;
      validLastEvent = CHECK_SUPPORTED_CURVES;

      sl_zigbee_af_cbke_key_establishment_suite_t suite;
      if ((sli_zigbee_af_available_cbke_suite & SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1) != 0U) {
        suite = SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1;
      } else if ((sli_zigbee_af_available_cbke_suite & SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1) != 0U) {
        suite = SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1;
      } else {
        return SL_STATUS_FAIL;
      }

      sl_zigbee_af_key_establishment_cluster_println(
        "Skipping read attributes of supported ECC curves for inter-PAN"
        " (using %sk1)",
        (SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1 == suite) ? "283" : "163");

      sli_zigbee_af_key_establishment_select_curve(suite);
      keyEstablishStateMachine(BEGIN_KEY_ESTABLISHMENT, NULL, NULL);
      return ((lastEvent == validLastEvent)
              ? SL_STATUS_FAIL
              : SL_STATUS_OK);
    }

    // The Test Harness can skip the read attributes stage. This is useful
    // in running curve specific key establishment tests, irrespective of
    // which binaries are supported.
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
    if (sli_zigbee_af_use_test_harness_suite != SL_ZIGBEE_AF_INVALID_KEY_ESTABLISHMENT_SUITE) {
      lastEvent = CHECK_SUPPORTED_CURVES;
      validLastEvent = CHECK_SUPPORTED_CURVES;
      sli_zigbee_af_key_establishment_select_curve(sli_zigbee_af_use_test_harness_suite);
      keyEstablishStateMachine(BEGIN_KEY_ESTABLISHMENT, NULL, NULL);
    } else
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
    {
      keyEstablishStateMachine(CHECK_SUPPORTED_CURVES, NULL, NULL);
    }

    return ((lastEvent == validLastEvent)
            ? SL_STATUS_FAIL
            : SL_STATUS_OK);
  }
  return SL_STATUS_INVALID_STATE;
}

static bool checkMalformed283k1Command(bool isCertificate)
{
  sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();
  uint8_t validSize = isCertificate
                      ? INITIATE_KEY_ESTABLISHMENT_LENGTH_283K1
                      : SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE;
  if (isCbkeKeyEstablishmentSuite283k1()) {
    sl_zigbee_af_key_establishment_cluster_println("Checking size Certificate size %u, expected %u", cmd->bufLen - cmd->payloadStartIndex, validSize);
    if (cmd->bufLen < cmd->payloadStartIndex + validSize) {
      sl_zigbee_af_key_establishment_cluster_println(
        "Invalid length for KE command: %d (expected: %d, actual: %d)",
        cmd->commandId,
        sli_zigbee_af_key_establish_message_to_data_size[cmd->commandId + cbkeSuiteOffset],
        (cmd->bufLen - cmd->payloadStartIndex));

      cleanupAndStop(INVALID_PARTNER_MESSAGE);
      return true;
    }
  }
  return false;
}

static sl_zigbee_af_status_t validateNewCbkeSuite(uint8_t endpoint,
                                                  uint16_t newCbkeSuite)
{
  UNUSED_VAR(endpoint);
  if (newCbkeSuite <= sl_zigbee_af_is_full_smart_energy_security_present()) {
    //sli_zigbee_af_available_cbke_suite = newCbkeSuite;
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  } else {
    return SL_ZIGBEE_ZCL_STATUS_INVALID_VALUE;
  }
}
//-----------------------------------------------------------------------------
//Helper functions. These get called from key-establishment-curve-support.c

void cleanupAndStopWithDelay(sl_zigbee_af_key_establishment_notify_message_t message,
                             uint8_t delayInSec)
{
  sl_zigbee_af_ami_key_establishment_status_t status;
  bool linkKeyEstablished = (message == LINK_KEY_ESTABLISHED);
  sl_status_t storeLinkKeyStatus = sl_zigbee_clear_temporary_data_maybe_store_link_key_for_curve(linkKeyEstablished);

  scriptTestCheckpoint("CBKE: link key %s",
                       linkKeyEstablished ? "established" : "failure");

  if (delayInSec == 0) {
    delayInSec = INTERNAL_ERROR_BACK_OFF_TIME;
  }

  sl_zigbee_af_key_establishment_cluster_flush();
  sl_zigbee_af_key_establishment_cluster_println("End Key Establishment Status: 0x%02X, Store Link Key Status: 0x%02X",
                                                 message,
                                                 storeLinkKeyStatus);

  if (linkKeyEstablished && storeLinkKeyStatus != SL_STATUS_OK) {
    message = NO_LOCAL_RESOURCES;
  }

  status = appNotifyMessageToKeyEstablishStatus[message];

  // This is a notify because either we are terminating key establishment
  // prematurely, or it succeeded.
  askApplicationWithDelay(message, delayInSec);

  if (!keyEstPartner.isInitiator && linkKeyEstablished) {
    sl_zigbee_af_send_immediate_default_response(SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  } else if (status != SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS
             && message != PARTNER_SENT_DEFAULT_RESPONSE_ERROR
             && message != BAD_KEY_ESTABLISHMENT_SUITE
             && (keyEstPartner.isInitiator || lastEvent != NO_KEY_ESTABLISHMENT_EVENT)) {
    // No point in sending a terminate when this is the first step and we are
    // the initiator.
    sendTerminateMessage(status, BACK_OFF_TIME_REPORTED_TO_PARTNER);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }

  clearKeyEstablishmentState();
}

void sendNextKeyEstablishMessage(sli_zigbee_key_establish_message_t message,
                                 uint8_t *data)
{
  uint8_t *ptr = appResponseData + SL_ZIGBEE_AF_ZCL_OVERHEAD;
  uint8_t size = sli_zigbee_af_key_establish_message_to_data_size[message + cbkeSuiteOffset];
  bool certMessage = (message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID);
  if (certMessage) {
    *ptr++ = LOW_BYTE(sli_zigbee_af_current_cbke_suite);
    *ptr++ = HIGH_BYTE(sli_zigbee_af_current_cbke_suite);
    *ptr++ = EM_AF_ADVERTISED_EPHEMERAL_DATA_GEN_TIME_SECONDS;
    *ptr++ = GENERATE_SHARED_SECRET_TIME_SECONDS;
    size -= 4;  // reduce the size for the 4 bytes we already added
  }
  memmove(ptr, data, size);
  ptr += size;

  appResponseLength = (ptr - appResponseData);
  sendKeyEstablishMessage(message);
}

//-----------------------------------------------------------------------------
//Helper functions for the test-harness.

void sli_zigbee_af_skip_check_supported_curves(sl_zigbee_af_cbke_key_establishment_suite_t suite)
{
#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  sli_zigbee_af_use_test_harness_suite = suite;
#else
  UNUSED_VAR(suite);
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
}

void sli_zigbee_af_set_available_curves(sl_zigbee_af_cbke_key_establishment_suite_t suite)
{
  sli_zigbee_af_available_cbke_suite = suite;
  writeKeyEstablishmentClusterAttribute(keyEstablishmentEndpoint);
}

void sli_zigbee_af_key_establishment_select_curve(sl_zigbee_af_cbke_key_establishment_suite_t suite)
{
  if ((suite & sli_zigbee_af_available_cbke_suite) == 0) {
    sli_zigbee_af_current_cbke_suite = SL_ZIGBEE_AF_INVALID_KEY_ESTABLISHMENT_SUITE;
    return;
  }

  sli_zigbee_af_current_cbke_suite   = suite;
  if (suite == SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1) {
    cbkeSuiteOffset   = CBKE_163K1_SUITE_OFFSET;
    certSubjectOffset = CERT_SUBJECT_OFFSET;
    certIssuerOffset  = CERT_ISSUER_OFFSET;
  } else if (suite == SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1) {
    cbkeSuiteOffset   = CBKE_283K1_SUITE_OFFSET;
    certSubjectOffset = CERT_SUBJECT_OFFSET_283K1;
    certIssuerOffset  = CERT_ISSUER_OFFSET_283K1;
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

//-----------------------------------------------------------------------------

sl_zigbee_af_status_t sl_zigbee_af_key_establishment_cluster_server_pre_attribute_changed_cb(uint8_t endpoint,
                                                                                             sl_zigbee_af_attribute_id_t attributeId,
                                                                                             sl_zigbee_af_attribute_type_t attributeType,
                                                                                             uint8_t size,
                                                                                             uint8_t *value)
{
  UNUSED_VAR(attributeType);
  switch (attributeId) {
    case ZCL_KEY_ESTABLISHMENT_SUITE_SERVER_ATTRIBUTE_ID:
    {
      uint16_t newCbkeSuite;
      memmove(&newCbkeSuite, value, size);
      return validateNewCbkeSuite(endpoint, newCbkeSuite);
    }
    default:
      return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  }
}

sl_zigbee_af_status_t sl_zigbee_af_key_establishment_cluster_client_pre_attribute_changed_cb(uint8_t endpoint,
                                                                                             sl_zigbee_af_attribute_id_t attributeId,
                                                                                             sl_zigbee_af_attribute_type_t attributeType,
                                                                                             uint8_t size,
                                                                                             uint8_t *value)
{
  return sl_zigbee_af_key_establishment_cluster_server_pre_attribute_changed_cb(endpoint,
                                                                                attributeId,
                                                                                attributeType,
                                                                                size,
                                                                                value);
}

void sl_zigbee_af_key_establishment_cluster_client_attribute_changed_cb(uint8_t endpoint,
                                                                        sl_zigbee_af_attribute_id_t attributeId)
{
  uint16_t availableSuites;
  uint16_t serverSuites;

  sl_zigbee_af_status_t status = sl_zigbee_af_read_client_attribute(endpoint,
                                                                    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                                    attributeId,
                                                                    (uint8_t *)&availableSuites,
                                                                    sizeof(availableSuites));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_core_println("Reading KE client attribute failed");
    return;
  }

  sli_zigbee_af_available_cbke_suite = availableSuites;

  status = sl_zigbee_af_read_server_attribute(endpoint,
                                              ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                              attributeId,
                                              (uint8_t *)&serverSuites,
                                              sizeof(serverSuites));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_core_println("Reading KE server attribute failed");
    return;
  }

  if (availableSuites != serverSuites) {
    status = sl_zigbee_af_write_server_attribute(endpoint,
                                                 ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                 attributeId,
                                                 (uint8_t *)&availableSuites,
                                                 ZCL_ENUM16_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_core_println("Updating KE server status failed");
    }
  }
}

void sl_zigbee_af_key_establishment_cluster_server_attribute_changed_cb(uint8_t endpoint,
                                                                        sl_zigbee_af_attribute_id_t attributeId)
{
  uint16_t clientSuites;
  uint16_t availableSuites;
  sl_zigbee_af_status_t status = sl_zigbee_af_read_server_attribute(endpoint,
                                                                    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                                    attributeId,
                                                                    (uint8_t *)&availableSuites,
                                                                    sizeof(availableSuites));
  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_core_println("Reading server attribute failed");
    return;
  }

  sli_zigbee_af_available_cbke_suite = availableSuites;

  status = sl_zigbee_af_read_client_attribute(endpoint,
                                              ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                              attributeId,
                                              (uint8_t *)&clientSuites,
                                              sizeof(clientSuites));

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_core_println("Reading KE client attribute failed");
    return;
  }
  if (availableSuites != clientSuites) {
    status = sl_zigbee_af_write_client_attribute(endpoint,
                                                 ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                 attributeId,
                                                 (uint8_t *)&availableSuites,
                                                 ZCL_ENUM16_ATTRIBUTE_TYPE);
    if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
      sl_zigbee_af_core_println("Updating server status failed");
    }

    //sl_zigbee_af_core_println("Upading client suite to %u",availableSuites);
  }
}

void sl_zigbee_af_key_establishment_read_attributes_cb(sl_zigbee_af_cbke_key_establishment_suite_t suite)
{
  sl_zigbee_af_key_establishment_cluster_println("Supported Suites %u", suite);
  sl_zigbee_af_key_establishment_cluster_println("keyEstPartner.isInitiator sl_zigbee_af_key_establishment_read_attributes_cb %u", keyEstPartner.isInitiator);
  if (!keyEstPartner.isInitiator) {
    //We identify the highest available suite to use.
    if ((suite & sli_zigbee_af_available_cbke_suite)
        >= SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1) {
      sl_zigbee_af_key_establishment_cluster_println("Using cbke-283k1");
      sli_zigbee_af_key_establishment_select_curve(SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1);
    } else if ((suite & sli_zigbee_af_available_cbke_suite)
               <= SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1) {
      //We accept 0x00(SL_ZIGBEE_AF_INVALID_KEY_ESTABLISHMENT_SUITE) as an acceptable value for
      //163k1 KE because our stacks 4.6 and older don't initialize the KE attribute to 1.
      sl_zigbee_af_key_establishment_cluster_println("Using cbke-163k1");
      sli_zigbee_af_key_establishment_select_curve(SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1);
    } else {
      sl_zigbee_af_key_establishment_cluster_println("Error: No valid Cluster");
      cleanupAndStop(BAD_KEY_ESTABLISHMENT_SUITE);
      return;
    }
    keyEstablishStateMachine(BEGIN_KEY_ESTABLISHMENT, NULL, NULL);
  }
}

sl_status_t sl_zigbee_af_initiate_key_establishment_cb(sl_802154_short_addr_t nodeId,
                                                       uint8_t endpoint)
{
  return initiateKeyEstablishment(NULL, // eui64 - unused
                                  nodeId,
                                  endpoint);
}

sl_status_t sl_zigbee_af_initiate_inter_pan_key_establishment_cb(sl_802154_pan_id_t panId,
                                                                 const sl_802154_long_addr_t eui64)
{
  return initiateKeyEstablishment(eui64,
                                  panId,
                                  0); // endpoint - unused
}

bool sl_zigbee_af_performing_key_establishment_cb(void)
{
  return (lastEvent != NO_KEY_ESTABLISHMENT_EVENT);
}

void sli_zigbee_af_key_establishment_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_event_init(apsDuplicateDetectionEvent,
                          sl_zigbee_af_key_establishment_aps_duplicate_detection_event_handler);
}

void sl_zigbee_af_key_establishment_cluster_server_init_cb(uint8_t endpoint)
{
  clearKeyEstablishmentState();
  setupEventTimeouts(0, 0);
  keyEstablishmentEndpoint = endpoint;

//If the test harness is selected, it is expected to force the selection of the ECC 283k1 curve.
//By default, all variables use the ECC 163k1 curve.
  sli_zigbee_af_available_cbke_suite = sl_zigbee_af_is_full_smart_energy_security_present();

  if (sli_zigbee_af_current_cbke_suite == SL_ZIGBEE_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1) {
    cbkeSuiteOffset   = CBKE_283K1_SUITE_OFFSET;
    certSubjectOffset = CERT_SUBJECT_OFFSET_283K1;
    certIssuerOffset  = CERT_ISSUER_OFFSET_283K1;
  }

  // We use a "core" print in hopes that this message will be seen.
  // Key establishment will not work and mysteriously fail when started.
  sl_zigbee_af_core_println("Key Est. Init %s 0x%02X", (isCbkeKeyEstablishmentSuiteValid() ? "Success" : "FAILED!"), LOW_BYTE(sli_zigbee_af_available_cbke_suite));
  sl_zigbee_af_key_establishment_cluster_println("Key Est. Init %s",
                                                 (isCbkeKeyEstablishmentSuiteValid()
                                                  ? "Success"
                                                  : "FAILED!"));

  // (Case 14692) If we do have full SE security, indicate the proper KE suite
  // in the K.E. Suite attribute for the appropriate endpoint.
  // NOTE: We also take this opportunity to set the client-side KE Suite
  // attribute value to match.  This is really the KEClusterClientInitCallback's
  // job, but since this plugin is a really a package deal (for client+server)
  // and since the values need to match anyway, we do it here.
  // Implementing the KEClusterClientInitCallback just for this setting
  // seems like overkill anyway and would consume more code space.
  writeKeyEstablishmentClusterAttribute(endpoint);
}

void sl_zigbee_af_key_establishment_cluster_server_tick_cb(uint8_t endpoint)
{
  UNUSED_VAR(endpoint);
  cleanupAndStop(TIMEOUT_OCCURRED);
}

bool sl_zigbee_af_key_establishment_cluster_initiate_key_establishment_request_cb(uint16_t keyEstablishmentSuite,
                                                                                  uint8_t ephemeralDataGenerateTime,
                                                                                  uint8_t confirmKeyGenerateTime,
                                                                                  uint8_t *identity)
{
  UNUSED_VAR(ephemeralDataGenerateTime);
  UNUSED_VAR(confirmKeyGenerateTime);
  sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();
  sl_zigbee_af_key_establishment_cluster_println("Suite %u\r\n", keyEstablishmentSuite);
  sli_zigbee_af_key_establishment_select_curve(keyEstablishmentSuite);
  if (cmd != NULL && setPartnerFromCommand(cmd)) {
    keyEstPartner.sequenceNumber = cmd->seqNum;
    if (checkMalformed283k1Command(true)) {
      return true;
    }
  } else {
    return true;
  }
  keyEstablishStateMachine(BEGIN_KEY_ESTABLISHMENT,
                           cmd->buffer + cmd->payloadStartIndex,
                           identity);
  return true;
}

bool sl_zigbee_af_key_establishment_cluster_ephemeral_data_request_cb(uint8_t *ephemeralData)
{
  if (checkMalformed283k1Command(false)) {
    return true;
  }
  keyEstablishStateMachine(GENERATE_KEYS, ephemeralData, NULL);
  return true;
}

bool sl_zigbee_af_key_establishment_cluster_confirm_key_data_request_cb(uint8_t *secureMessageAuthenticationCode)
{
  keyEstablishStateMachine(GENERATE_SHARED_SECRET,
                           secureMessageAuthenticationCode,
                           NULL);
  return true;
}

bool sl_zigbee_af_key_establishment_cluster_terminate_key_establishment_cb(uint8_t statusCode,
                                                                           uint8_t waitTime,
                                                                           uint16_t keyEstablishmentSuite)
{
  UNUSED_VAR(keyEstablishmentSuite);
  sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();
  if (cmd != NULL && cmd->type >= SL_ZIGBEE_INCOMING_MULTICAST) {
    sl_zigbee_af_key_establishment_cluster_println("Ignoring Broadcast KE terminate");
    return true;
  }

  if (lastEvent != NO_KEY_ESTABLISHMENT_EVENT && cmd != NULL && commandIsFromOurPartner(cmd)) {
    if (keyEstPartner.isIntraPan) {
      apsSequenceNumbers[apsSequenceNumbersReceived] = cmd->apsFrame->sequence;
      apsSequenceNumbersReceived++;
    }
    sl_zigbee_af_key_establishment_cluster_println("Terminate Received, Status(%d): %s",
                                                   statusCode,
                                                   terminateStatus[((statusCode > UNKNOWN_TERMINATE_STATUS)
                                                                    ? UNKNOWN_TERMINATE_STATUS
                                                                    : statusCode)]);
    cleanupAndStopWithDelay(PARTNER_SENT_TERMINATE, waitTime);
  }
  sl_zigbee_af_send_default_response(cmd, SL_ZIGBEE_ZCL_STATUS_SUCCESS);
  return true;
}

void sl_zigbee_af_key_establishment_cluster_server_message_sent_cb(sl_zigbee_outgoing_message_type_t type,
                                                                   uint16_t indexOrDestination,
                                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                                   uint16_t msgLen,
                                                                   uint8_t *message,
                                                                   sl_status_t status)
{
  messageSentHandler(type, indexOrDestination, apsFrame, msgLen, message, status);
}

static void defaultResponseCallback(uint8_t endpoint,
                                    uint8_t commandId,
                                    sl_zigbee_af_status_t status)
{
  UNUSED_VAR(endpoint);
  UNUSED_VAR(commandId);
  sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();

  if (cmd != NULL && cmd->type >= SL_ZIGBEE_INCOMING_MULTICAST) {
    sl_zigbee_af_key_establishment_cluster_println("Ignoring Broadcast KE default resp");
    return;
  }

  if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS
      && lastEvent != NO_KEY_ESTABLISHMENT_EVENT
      && cmd != NULL
      && commandIsFromOurPartner(cmd)) {
    sl_zigbee_af_key_establishment_cluster_println("Got Default Response with error code: %d",
                                                   status);
    // While the actual status code may be more meaningful, we don't really care.
    // We interpret it as a transient failure.  Otherwise we would have got an
    // explicit Terminate Message.
    cleanupAndStop(PARTNER_SENT_DEFAULT_RESPONSE_ERROR);
  } else {
    if (lastEvent == SEND_CONFIRM_KEY_MESSAGE
        && keyEstPartner.isInitiator) {
      // Success!
      cleanupAndStop(LINK_KEY_ESTABLISHED);
      return;
    }
  }
}

void sl_zigbee_af_key_establishment_cluster_client_default_response_cb(uint8_t endpoint,
                                                                       uint8_t commandId,
                                                                       sl_zigbee_af_status_t status)
{
  defaultResponseCallback(endpoint, commandId, status);
}

void sl_zigbee_af_key_establishment_cluster_server_default_response_cb(uint8_t endpoint,
                                                                       uint8_t commandId,
                                                                       sl_zigbee_af_status_t status)
{
  defaultResponseCallback(endpoint, commandId, status);
}

bool sl_zigbee_af_key_establishment_cluster_initiate_key_establishment_response_cb(uint16_t requestedKeyEstablishmentSuite,
                                                                                   uint8_t ephemeralDataGenerateTime,
                                                                                   uint8_t confirmKeyGenerateTime,
                                                                                   uint8_t *identity)
{
  UNUSED_VAR(requestedKeyEstablishmentSuite);
  UNUSED_VAR(ephemeralDataGenerateTime);
  UNUSED_VAR(confirmKeyGenerateTime);
  sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();
  if (checkMalformed283k1Command(true)) {
    return true;
  }
  keyEstablishStateMachine(GENERATE_KEYS,
                           cmd->buffer + cmd->payloadStartIndex,
                           identity);
  return true;
}

bool sl_zigbee_af_key_establishment_cluster_ephemeral_data_response_cb(uint8_t *ephemeralData)
{
  if (checkMalformed283k1Command(false)) {
    return true;
  }
  keyEstablishStateMachine(GENERATE_SHARED_SECRET, ephemeralData, NULL);
  return true;
}

bool sl_zigbee_af_key_establishment_cluster_confirm_key_data_response_cb(uint8_t *secureMessageAuthenticationCode)
{
  keyEstablishStateMachine(INITIATOR_RECEIVED_CONFIRM_KEY,
                           secureMessageAuthenticationCode,
                           NULL);
  return true;
}

void sl_zigbee_af_key_establishment_cluster_client_message_sent_cb(sl_zigbee_outgoing_message_type_t type,
                                                                   uint16_t indexOrDestination,
                                                                   sl_zigbee_aps_frame_t *apsFrame,
                                                                   uint16_t msgLen,
                                                                   uint8_t *message,
                                                                   sl_status_t status)
{
  messageSentHandler(type, indexOrDestination, apsFrame, msgLen, message, status);
}

//------------------------------------------------------------------------------
// CBKE Library Callbacks

void sli_zigbee_af_key_establishment_generate_cbke_keys_handler(sl_status_t status,
                                                                sl_zigbee_public_key_data_t *ephemeralPublicKey)
{
  sl_zigbee_af_key_establishment_cluster_println("GenerateCbkeKeysHandler() returned: 0x%02X",
                                                 status);
  sli_zigbee_af_crypto_operation_complete();

  if (status != SL_STATUS_OK) {
    cleanupAndStop(NO_LOCAL_RESOURCES);
    return;
  }

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  if (sli_zigbee_af_key_establishment_test_harness_cbke_callback(CBKE_OPERATION_GENERATE_KEYS,
                                                                 ephemeralPublicKey->contents,
                                                                 NULL)) {
    return;
  }
#endif //SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

  (void) sl_zigbee_af_push_endpoint_network_index(keyEstablishmentEndpoint);
  keyEstablishStateMachine(SEND_EPHEMERAL_DATA_MESSAGE,
                           sl_zigbee_public_key_contents(ephemeralPublicKey),
                           NULL);
  (void) sl_zigbee_af_pop_network_index();
}

void sli_zigbee_af_key_establishment_calculate_smacs_handler(sl_status_t status,
                                                             sl_zigbee_smac_data_t *initiatorSmac,
                                                             sl_zigbee_smac_data_t *responderSmac)
{
  sl_zigbee_af_key_establishment_cluster_println("CalculateSmacsHandler() returned: 0x%02X",
                                                 status);
  sli_zigbee_af_crypto_operation_complete();
  debugPrintSmac(true, sl_zigbee_smac_contents(initiatorSmac));
  debugPrintSmac(false, sl_zigbee_smac_contents(responderSmac));

  if (status != SL_STATUS_OK) {
    cleanupAndStop(NO_LOCAL_RESOURCES);
    return;
  }

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  if (sli_zigbee_af_key_establishment_test_harness_cbke_callback(CBKE_OPERATION_GENERATE_SECRET,
                                                                 initiatorSmac->contents,
                                                                 responderSmac->contents)) {
    return;
  }
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

  (void) sl_zigbee_af_push_endpoint_network_index(keyEstablishmentEndpoint);
  keyEstablishStateMachine(SEND_CONFIRM_KEY_MESSAGE,
                           sl_zigbee_key_contents(initiatorSmac),
                           sl_zigbee_key_contents(responderSmac));
  (void) sl_zigbee_af_pop_network_index();
}

void sli_zigbee_af_key_establishment_generate_cbke_keys_handler283k1(sl_status_t status,
                                                                     sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey)
{
  sl_zigbee_af_key_establishment_cluster_println("GenerateCbkeKeysHandler283k1() returned: 0x%02X",
                                                 status);
  sli_zigbee_af_crypto_operation_complete();

  if (status != SL_STATUS_OK) {
    cleanupAndStop(NO_LOCAL_RESOURCES);
    return;
  }

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  if (sli_zigbee_af_key_establishment_test_harness_cbke_callback(CBKE_OPERATION_GENERATE_KEYS_283K1,
                                                                 ephemeralPublicKey->contents,
                                                                 NULL)) {
    return;
  }
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

  (void) sl_zigbee_af_push_endpoint_network_index(keyEstablishmentEndpoint);
  keyEstablishStateMachine(SEND_EPHEMERAL_DATA_MESSAGE,
                           sl_zigbee_public_key_283k1_contents(ephemeralPublicKey),
                           NULL);
  (void) sl_zigbee_af_pop_network_index();
}

void sli_zigbee_af_key_establishment_calculate_smacs_handler283k1(sl_status_t status,
                                                                  sl_zigbee_smac_data_t* initiatorSmac,
                                                                  sl_zigbee_smac_data_t* responderSmac)
{
  sl_zigbee_af_key_establishment_cluster_println("CalculateSmacsHandler() returned: 0x%02X",
                                                 status);
  sli_zigbee_af_crypto_operation_complete();

  if (status != SL_STATUS_OK) {
    cleanupAndStop(NO_LOCAL_RESOURCES);
    return;
  }

#ifdef SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT
  if (sli_zigbee_af_key_establishment_test_harness_cbke_callback(CBKE_OPERATION_GENERATE_SECRET_283K1,
                                                                 initiatorSmac->contents,
                                                                 responderSmac->contents)) {
    return;
  }
#endif // SL_CATALOG_ZIGBEE_TEST_HARNESS_PRESENT

  (void) sl_zigbee_af_push_endpoint_network_index(keyEstablishmentEndpoint);
  keyEstablishStateMachine(SEND_CONFIRM_KEY_MESSAGE,
                           sl_zigbee_key_contents(initiatorSmac),
                           sl_zigbee_key_contents(responderSmac));
  (void) sl_zigbee_af_pop_network_index();
}

//------------------------------------------------------------------------------
// Event Handlers

void sl_zigbee_af_key_establishment_aps_duplicate_detection_event_handler(sl_zigbee_af_event_t * event)
{
  UNUSED_VAR(event);
  sl_zigbee_af_event_set_inactive(apsDuplicateDetectionEvent);

  lastNodeSuccessfulCbke = SL_ZIGBEE_NULL_NODE_ID;
}

//----------------------
// ZCL commands handling

static void postZclCommandProcedure(sl_zigbee_af_cluster_command_t *cmd,
                                    sl_zigbee_af_status_t status)
{
  if (status == SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND) {
    sl_zigbee_af_key_establishment_cluster_println(
      "Invalid length for KE command: %d (expected: %d, actual: %d)",
      cmd->commandId,
      sli_zigbee_af_key_establish_message_to_data_size[cmd->commandId + cbkeSuiteOffset],
      (cmd->bufLen - cmd->payloadStartIndex));

    cleanupAndStop(INVALID_PARTNER_MESSAGE);
  } else if (status != SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
    sl_zigbee_af_send_default_response(cmd, status);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

uint32_t sl_zigbee_af_key_establishment_cluster_server_command_parse(sl_service_opcode_t opcode,
                                                                     sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_status_t zclStatus = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (((!commandIsFromOurPartner(cmd)) && (lastEvent != NO_KEY_ESTABLISHMENT_EVENT)) \
      || (!isCbkeKeyEstablishmentSuiteValid())) {
    sli_zigbee_key_establishment_partner_t tmpPartner;
    sl_zigbee_af_key_establishment_cluster_println("sli_zigbee_af_available_cbke_suite %u last event %u", sli_zigbee_af_available_cbke_suite, lastEvent);
    // If we have not successfully initialized or we are already in doing
    // key establishment with another partner, tell this new partner to go
    // away and maybe try again later.  The sendTerminateMessage function
    // assumes it is sending to the current partner, so we have to temporarily
    // switch to the new partner, send the terminate, and then switch back to
    // our real partner.
    memcpy(&tmpPartner, &keyEstPartner, sizeof(sli_zigbee_key_establishment_partner_t));
    sl_zigbee_af_key_establishment_cluster_println(isCbkeKeyEstablishmentSuiteValid()
                                                   ? "Second Key estabishment not supported, terminating it."
                                                   : "Key Est. FAILED INITIALIZATION, terminating");
    if (setPartnerFromCommand(cmd)) {
      keyEstPartner.sequenceNumber = cmd->seqNum;
      sendTerminateMessage(SL_ZIGBEE_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES,
                           BACK_OFF_TIME_REPORTED_TO_PARTNER);
    }
    memmove(&keyEstPartner, &tmpPartner, sizeof(sli_zigbee_key_establishment_partner_t));
    return SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  } else if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID:
      {
        uint16_t payloadOffset = cmd->payloadStartIndex;
        if (cmd->bufLen < payloadOffset + INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_MIN_LENGTH) {
          zclStatus = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
        } else {
          sl_zcl_key_establishment_cluster_initiate_key_establishment_request_command_t cmd_data;
          zclStatus = zcl_decode_key_establishment_cluster_initiate_key_establishment_request_command(cmd, &cmd_data);

          if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            sl_zigbee_af_key_establishment_cluster_initiate_key_establishment_request_cb(cmd_data.keyEstablishmentSuite,
                                                                                         cmd_data.ephemeralDataGenerateTime,
                                                                                         cmd_data.confirmKeyGenerateTime,
                                                                                         cmd_data.identity);
          }
        }
        break;
      }
      case ZCL_EPHEMERAL_DATA_REQUEST_COMMAND_ID:
      {
        // The minimum length for this command is 16 + 6 bytes
        uint16_t payloadOffset = cmd->payloadStartIndex;
        if (cmd->bufLen < payloadOffset + EPHEMERAL_DATA_REQUEST_COMMAND_MIN_LENGTH) {
          zclStatus = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
        } else {
          sl_zcl_key_establishment_cluster_ephemeral_data_request_command_t cmd_data;
          zclStatus = zcl_decode_key_establishment_cluster_ephemeral_data_request_command(cmd, &cmd_data);

          if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            sl_zigbee_af_key_establishment_cluster_ephemeral_data_request_cb(cmd_data.ephemeralData);
          }
        }
        break;
      }
      case ZCL_CONFIRM_KEY_DATA_REQUEST_COMMAND_ID:
      {
        // The minimum length for this command is 16 bytes
        uint16_t payloadOffset = cmd->payloadStartIndex;
        if (cmd->bufLen < payloadOffset + CONFIRM_KEY_DATA_REQUEST_COMMAND_MIN_LENGTH) {
          zclStatus = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
        } else {
          sl_zcl_key_establishment_cluster_confirm_key_data_request_command_t cmd_data;
          zclStatus = zcl_decode_key_establishment_cluster_confirm_key_data_request_command(cmd, &cmd_data);

          if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            sl_zigbee_af_key_establishment_cluster_confirm_key_data_request_cb(cmd_data.secureMessageAuthenticationCode);
          }
        }
        break;
      }
      case ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID:
      {
        uint16_t payloadOffset = cmd->payloadStartIndex;
        if (cmd->bufLen < payloadOffset + TERMINATE_KEY_ESTABLISHMENT_COMMAND_MIN_LENGTH) {
          zclStatus = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
        } else {
          sl_zcl_key_establishment_cluster_terminate_key_establishment_from_client_command_t cmd_data;
          zclStatus = zcl_decode_key_establishment_cluster_terminate_key_establishment_from_client_command(cmd, &cmd_data);

          if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            sl_zigbee_af_key_establishment_cluster_terminate_key_establishment_cb(cmd_data.statusCode,
                                                                                  cmd_data.waitTime,
                                                                                  cmd_data.keyEstablishmentSuite);
          }
        }
        break;
      }
      default:
        break;
    }
  }

  postZclCommandProcedure(cmd, zclStatus);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}

uint32_t sl_zigbee_af_key_establishment_cluster_client_command_parse(sl_service_opcode_t opcode,
                                                                     sl_service_function_context_t *context)
{
  (void)opcode;

  sl_zigbee_af_cluster_command_t *cmd = (sl_zigbee_af_cluster_command_t *)context->data;
  sl_zigbee_af_status_t zclStatus = SL_ZIGBEE_ZCL_STATUS_UNSUP_COMMAND;

  if (!cmd->mfgSpecific) {
    switch (cmd->commandId) {
      case ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID:
      {
        uint16_t payloadOffset = cmd->payloadStartIndex;
        if (cmd->bufLen < payloadOffset + TERMINATE_KEY_ESTABLISHMENT_COMMAND_MIN_LENGTH) {
          zclStatus = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
        } else {
          sl_zcl_key_establishment_cluster_terminate_key_establishment_from_server_command_t cmd_data;
          zclStatus = zcl_decode_key_establishment_cluster_terminate_key_establishment_from_server_command(cmd, &cmd_data);

          if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            sl_zigbee_af_key_establishment_cluster_terminate_key_establishment_cb(cmd_data.statusCode,
                                                                                  cmd_data.waitTime,
                                                                                  cmd_data.keyEstablishmentSuite);
          }
        }
        break;
      }
      case ZCL_INITIATE_KEY_ESTABLISHMENT_RESPONSE_COMMAND_ID:
      {
        uint16_t payloadOffset = cmd->payloadStartIndex;
        if (cmd->bufLen < payloadOffset + INITIATE_KEY_ESTABLISHMENT_RESPONSE_COMMAND_MIN_LENGTH) {
          zclStatus = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
        } else {
          sl_zcl_key_establishment_cluster_initiate_key_establishment_response_command_t cmd_data;
          zclStatus = zcl_decode_key_establishment_cluster_initiate_key_establishment_response_command(cmd, &cmd_data);

          if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            sl_zigbee_af_key_establishment_cluster_initiate_key_establishment_response_cb(cmd_data.requestedKeyEstablishmentSuite,
                                                                                          cmd_data.ephemeralDataGenerateTime,
                                                                                          cmd_data.confirmKeyGenerateTime,
                                                                                          cmd_data.identity);
          }
        }
        break;
      }
      case ZCL_EPHEMERAL_DATA_RESPONSE_COMMAND_ID:
      {
        uint16_t payloadOffset = cmd->payloadStartIndex;
        if (cmd->bufLen < payloadOffset + EPHEMERAL_DATA_RESPONSE_COMMAND_MIN_LENGTH) {
          zclStatus = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
        } else {
          sl_zcl_key_establishment_cluster_ephemeral_data_response_command_t cmd_data;
          zclStatus = zcl_decode_key_establishment_cluster_ephemeral_data_response_command(cmd, &cmd_data);

          if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            sl_zigbee_af_key_establishment_cluster_ephemeral_data_response_cb(cmd_data.ephemeralData);
          }
        }
        break;
      }
      case ZCL_CONFIRM_KEY_DATA_RESPONSE_COMMAND_ID:
      {
        uint16_t payloadOffset = cmd->payloadStartIndex;
        if (cmd->bufLen < payloadOffset + CONFIRM_KEY_DATA_RESPONSE_COMMAND_MIN_LENGTH) {
          zclStatus = SL_ZIGBEE_ZCL_STATUS_MALFORMED_COMMAND;
        } else {
          sl_zcl_key_establishment_cluster_confirm_key_data_response_command_t cmd_data;
          zclStatus = zcl_decode_key_establishment_cluster_confirm_key_data_response_command(cmd, &cmd_data);

          if (zclStatus == SL_ZIGBEE_ZCL_STATUS_SUCCESS) {
            sl_zigbee_af_key_establishment_cluster_confirm_key_data_response_cb(cmd_data.secureMessageAuthenticationCode);
          }
        }
        break;
      }
      default:
        break;
    }
  }

  postZclCommandProcedure(cmd, zclStatus);

  return SL_ZIGBEE_ZCL_STATUS_INTERNAL_COMMAND_HANDLED;
}
