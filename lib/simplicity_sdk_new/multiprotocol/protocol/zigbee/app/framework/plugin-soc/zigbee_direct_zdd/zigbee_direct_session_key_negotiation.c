/***************************************************************************//**
 * @file zigbee_direct_session_key_negotiation.c
 * @brief Implementation of Session Key Negotiation with SPEKE/ECDHE
 *        message sending/handling of key-negotiation messages carrying
 *        public variables for establishing a TC Link Key
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"

#include "zigbee_direct_common.h"
#include "zigbee_direct_tlv.h"
#include "sl_component_catalog.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "stack/include/sl_zigbee_types.h"
#include "stack/include/sl_zigbee_tlv_core.h"
#include "stack/include/sl_zigbee_stack_specific_tlv.h"
#include "stack/include/sl_zigbee_dlk_negotiation.h"
#include "stack/include/zigbee-security-manager.h"
#include "stack/include/sl_zigbee_security_manager_dlk_ecc.h"
#include "stack/include/zigbee-device-stack.h"
#include "zigbee_direct_session_key_negotiation.h"
#include "zigbee_direct_zdd_config.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "sl_custom_token_header.h"
#include "app/framework/security/af-security.h"
#include "mbedtls/sha256.h"
#include "stack/config/sl_zigbee_token_defines.h"

#ifndef SL_ZIGBEE_DIRECT_ZDD_DEFAULT_PASSCODE
// The default passcode to use for session key negotiation.
// Passcode with all zeros would be rejected so customers
// need to specify their own passcode in the app SLCP file.
#define SL_ZIGBEE_DIRECT_ZDD_DEFAULT_PASSCODE { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#endif // SL_ZIGBEE_DIRECT_ZDD_DEFAULT_PASSCODE

//difference between size of a TLV struct and the value in its length field
#define TLV_T_LENGTH_BYTE_OFFSET 3
#define TLV_TAG_LENGTH_SIZE 2

uint8_t sl_my_passcode[SL_ZIGBEE_ENCRYPTION_KEY_SIZE] = SL_ZIGBEE_DIRECT_ZDD_DEFAULT_PASSCODE;
sl_zigbee_direct_dlk_public_key_tlv_t ourPointTlv;
sl_zigbee_direct_dlk_public_key_tlv_t peerPointTlv;
const uint8_t emDlkAnonymousKeyContents[SL_ZIGBEE_ENCRYPTION_KEY_SIZE] = "ZigBeeAlliance18";

static sl_zigbee_sec_man_dlk_ecc_context_t ecdhCtx;
static uint8_t our_public_point[DLK_ECC_P256_PUBLIC_KEY_SIZE];

extern uint8_t sl_my_connection;
extern sl_zigbee_direct_connection_status_t sl_zvd_connection_status;
extern sl_zigbee_direct_connection_status_t sl_zvd_connection_status_next;

// Convert EUI64 to UINT64 in little endianness order for EUI64 comparison
#define EUI64_TO_UINT64_LE(eui64) (((uint64_t)(((uint8_t *)(eui64))[0]) <<  0)   \
                                   + ((uint64_t)(((uint8_t *)(eui64))[1]) <<  8) \
                                   + ((uint64_t)(((uint8_t *)(eui64))[2]) << 16) \
                                   + ((uint64_t)(((uint8_t *)(eui64))[3]) << 24) \
                                   + ((uint64_t)(((uint8_t *)(eui64))[4]) << 32) \
                                   + ((uint64_t)(((uint8_t *)(eui64))[5]) << 40) \
                                   + ((uint64_t)(((uint8_t *)(eui64))[6]) << 48) \
                                   + ((uint64_t)(((uint8_t *)(eui64))[7]) << 56))

// ----------------------------------------------------------------------------
// Supported Key Negotiation Bitmask
#define SLI_ZIGBEE_DIRECT_SESSION_KEY_REQUEST 1
#define SLI_ZIGBEE_DIRECT_SESSION_KEY_RESPONSE 0

// ----------------------------------------------------------------------------
//  TLV processing

// define the local tlv processing validation environment
static sl_zigbee_tlv_tag_min_length_t dlkReqLocalTlvEnv[] =
{ { .tag_id = SL_ZIGBEE_DIRECT_DLK_TLV_SELECTED_NEGOTIATION_METHOD_TAG_ID,
    .min_length = SL_ZIGBEE_DIRECT_DLK_TLV_SELECTED_NEGOTIATION_METHOD_MIN_LEN },
  { .tag_id = SL_ZIGBEE_DIRECT_SECURITY_TLV_P256_PUBLIC_POINT_TAG_ID,
    .min_length = SL_ZIGBEE_DIRECT_SECURITY_TLV_P256_PUBLIC_POINT_MIN_LEN },
  { .tag_id = SL_ZIGBEE_DIRECT_SECURITY_TLV_C25519_PUBLIC_POINT_TAG_ID,
    .min_length =  SL_ZIGBEE_DIRECT_SECURITY_TLV_C25519_PUBLIC_POINT_MIN_LEN },
  TLV_ENV_TERMINATOR, };

// NOTE we'll decode the selected method to determine the public point type and
// length of the key. Use a boolean switch to indicate request vs response
static void constructKeyNegotiationTlvPayload(uint8_t payload_length,
                                              uint8_t* payload,
                                              uint16_t tlvIndex,
                                              bool isRequest)
{
  (void)isRequest;
  sl_zigbee_node_type_t myNodeType;
  sl_status_t status = sl_zigbee_get_network_parameters(&myNodeType, NULL);

  //tlvindex has no meaning, we are just appending
  if (tlvIndex + ourPointTlv.length + TLV_T_LENGTH_BYTE_OFFSET <= payload_length) {
    memmove(payload + tlvIndex, &ourPointTlv, ourPointTlv.length + TLV_T_LENGTH_BYTE_OFFSET);
    // if is key negotiation request, we need to include the selected key negotiation method
    // NOTE the latest revision of the spec mandates that both the request and response include
    // the selected key negotiation method

    if (status != SL_STATUS_NOT_JOINED) {
      sl_zigbee_app_debug_println("Appending nwk sequence no tlv");
    }
  } else {
    sl_zigbee_core_debug_println("Error appending tlv");
  }
}

// returns a bool to indicate if the message contains a public point tlv
// returns the result
static bool containsPublicKeyTlv(uint8_t messageLength,
                                 uint8_t* messageContents,
                                 uint16_t index,
                                 sl_zigbee_direct_dlk_public_key_tlv_t *tlvResult)
{
  uint8_t p256Tag = SL_ZIGBEE_DIRECT_SECURITY_TLV_P256_PUBLIC_POINT_TAG_ID;
  uint8_t c25519Tag = SL_ZIGBEE_DIRECT_SECURITY_TLV_C25519_PUBLIC_POINT_TAG_ID;
  // this is not very readable but it basically scans once to see if a certain id exists
  // then again to actually fetch the results
  if (sl_zigbee_tlv_search_buffer_payload_for_id(messageLength,
                                                 messageContents,
                                                 index,
                                                 p256Tag,
                                                 NULL,
                                                 SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MAX_LEN) == SL_STATUS_OK) {
    sl_zigbee_tlv_search_buffer_payload_for_id(messageLength,
                                               messageContents,
                                               index,
                                               p256Tag,
                                               (sl_zigbee_tlv_t *) tlvResult,
                                               SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MAX_LEN);
  } else if (sl_zigbee_tlv_search_buffer_payload_for_id(messageLength,
                                                        messageContents,
                                                        index,
                                                        c25519Tag,
                                                        NULL,
                                                        SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MAX_LEN) == SL_STATUS_OK) {
    sl_zigbee_tlv_search_buffer_payload_for_id(messageLength,
                                               messageContents,
                                               index,
                                               c25519Tag,
                                               (sl_zigbee_tlv_t *) tlvResult,
                                               SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MAX_LEN);
  } else {
    return false;
  }
  return true;
}

static sl_dlk_ecc_curve_id get_curve_id_with_valid_public_key_tlv(sl_zigbee_direct_dlk_public_key_tlv_t tlv,
                                                                  sli_dlk_ecc_selected_method selected_method)
{
  // check to make sure the tlv matches the expected key negotiation method,
  // and that the length is sensible
  sl_dlk_ecc_curve_id curve;
  sli_dlk_ecc_selected_method method;
  uint16_t length;
  if (tlv.tag_id == SL_ZIGBEE_DIRECT_SECURITY_TLV_P256_PUBLIC_POINT_TAG_ID) {
    curve = DLK_ECC_CURVE_P256;
    method = DLK_ECC_P256_SHA_256;
    length = DLK_ECC_P256_PUBLIC_KEY_SIZE;
  } else if (tlv.tag_id == SL_ZIGBEE_DIRECT_SECURITY_TLV_C25519_PUBLIC_POINT_TAG_ID) {
    curve =  DLK_ECC_CURVE_25519;
    method = DLK_ECC_CURVE25519_AES_MMO_128;
    length = DLK_ECC_COORDINATE_SIZE;
  } else {
    curve = DLK_ECC_CURVE_INVALID;
    method = DLK_ECC_NONE;
    length = 0xFFFF;
  }

  bool curveMatch = (method == selected_method);
  bool lengthOk = ((sl_zigbee_tlv_get_length(&tlv) - EUI64_SIZE) == length);

  if (!curveMatch || !lengthOk) {
    return DLK_ECC_CURVE_INVALID;
  }
  return curve;
}

static sl_status_t sli_zigbee_direct_p256_expand_shared_secret(sl_zigbee_sec_man_dlk_ecc_context_t *ctx)
{
  extern uint8_t sl_zvd_public_point_x[DLK_ECC_COORDINATE_SIZE];
  extern uint8_t sl_zvd_public_point_y[DLK_ECC_COORDINATE_SIZE];
  extern sl_802154_long_addr_t sl_zvd_eui;

  uint8_t buffer[DLK_ECC_COORDINATE_SIZE + SL_ZIGBEE_ENCRYPTION_KEY_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE];
  int buffLen = DLK_ECC_COORDINATE_SIZE + SL_ZIGBEE_ENCRYPTION_KEY_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE;
  int ret;

  ret = mbedtls_mpi_write_binary(&ctx->x_k, buffer, DLK_ECC_COORDINATE_SIZE);
  if (ret != 0) {
    return ret;
  }

  if (EUI64_TO_UINT64_LE(sl_zigbee_get_eui64()) > EUI64_TO_UINT64_LE(sl_zvd_eui)) {
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE, sl_zvd_eui, EUI64_SIZE);
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE, sl_zvd_public_point_x, DLK_ECC_COORDINATE_SIZE);
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE + DLK_ECC_COORDINATE_SIZE, sl_zvd_public_point_y, DLK_ECC_COORDINATE_SIZE);
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE, sl_zigbee_get_eui64(), EUI64_SIZE);
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE + EUI64_SIZE, our_public_point, DLK_ECC_P256_PUBLIC_KEY_SIZE);
  } else {
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE, sl_zigbee_get_eui64(), EUI64_SIZE);
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE, our_public_point, DLK_ECC_P256_PUBLIC_KEY_SIZE);
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE, sl_zvd_eui, EUI64_SIZE);
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE + EUI64_SIZE, sl_zvd_public_point_x, DLK_ECC_COORDINATE_SIZE);
    memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE + EUI64_SIZE + DLK_ECC_COORDINATE_SIZE, sl_zvd_public_point_y, DLK_ECC_COORDINATE_SIZE);
  }
  memcpy(buffer + DLK_ECC_COORDINATE_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE + EUI64_SIZE + DLK_ECC_P256_PUBLIC_KEY_SIZE, ctx->psk, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

  ret = mbedtls_sha256(buffer, buffLen, ctx->secret, 0);
  return ret;
}

sl_status_t sli_zigbee_direct_handle_incoming_dlk_negotiation_request(uint8_t messageLength,
                                                                      uint8_t* messageContents,
                                                                      uint16_t payloadIndex,
                                                                      sl_802154_short_addr_t partnerId,
                                                                      uint16_t characteristic)
{
  (void)partnerId;
  sl_zigbee_key_data_t psk;
  tokTypeMfgInstallationCode tokInstallCode;
  sl_zigbee_key_data_t basic_key;
  uint8_t *pointy2;
  size_t length;
  uint8_t installcode_with_crc[18];
  sl_status_t status;

  uint8_t response[SL_ZIGBEE_DIRECT_RESPONSE_MIN_LENGTH + SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MAX_LEN];

  // NOTE declarations are up here to satisfy the compiler as a result of the GOTO
  // the control flow is a little weird here since we need to shortcut
  // sending response on failures (considered harmful?)
  sl_zigbee_direct_dlk_selected_negotiation_method_tlv_t selectedTlv;

  sl_status_t sendStatus;
  //check if we are missing required TLVs
  if (!(sl_zigbee_tlv_check_general_format_env(messageLength, messageContents, payloadIndex, dlkReqLocalTlvEnv) == SL_STATUS_OK)
      || !((sl_zigbee_tlv_search_buffer_payload_for_id(messageLength,
                                                       messageContents,
                                                       payloadIndex,
                                                       SL_ZIGBEE_DIRECT_DLK_TLV_SELECTED_NEGOTIATION_METHOD_TAG_ID,
                                                       (sl_zigbee_tlv_t *) &selectedTlv,
                                                       SL_ZIGBEE_DIRECT_DLK_TLV_SELECTED_NEGOTIATION_METHOD_MAX_LEN) == SL_STATUS_OK)
           // can contain either key
           && containsPublicKeyTlv(messageLength, messageContents, payloadIndex, &peerPointTlv))) {
    sl_zigbee_app_debug_println("Seem to be missing a needed TLV");
    return SL_STATUS_FAIL;
  }

  sl_zigbee_app_debug_print("Peer Point TLV [");
  for (uint8_t i = 0; i < peerPointTlv.length; i++) {
    sl_zigbee_app_debug_print(" %02X", peerPointTlv.value[i]);
  }
  sl_zigbee_app_debug_println("]");

  sli_dlk_ecc_selected_method selected_method = (sli_dlk_ecc_selected_method)selectedTlv.value[0];
  sl_dlk_ecc_curve_id curve_id = get_curve_id_with_valid_public_key_tlv(peerPointTlv, selected_method);
  // discard if the public point tlv is invalid or unsupported
  if (curve_id == DLK_ECC_CURVE_INVALID) {
    sl_zigbee_app_debug_println("Not the negotiation method %02X  against peerpoint %02X with length %04X but will continue anyway", selected_method, peerPointTlv.tag_id, peerPointTlv.length);
    return SL_STATUS_FAIL;
  }

  sl_802154_long_addr_t *peerEuiData = (sl_802154_long_addr_t *) peerPointTlv.value;

  switch (selectedTlv.value[1]) {
    case DLK_SECRET_ENUM_WELL_KNOWN_KEY:   // well-known secret
      if (sl_zvd_connection_status == PROVISIONED_IN_PROVISIONING_SESSION) {
        sl_zvd_connection_status = PROVISIONED;
        (void)sl_token_manager_set_data(COMMON_TOKEN_PLUGIN_ZDD_AUTH_STATUS, (void *)&sl_zvd_connection_status, sizeof(uint8_t));
      }
      if ((sl_zigbee_direct_anonymous_join_timeout_sec == 0) || ((sl_zvd_connection_status >= PROVISIONED) && (sl_zigbee_get_permit_joining() == false))) {
        sl_zigbee_app_debug_println("Killing attempt as we are already commissioned (%02X) or permit joining is closed %02X sec or timeout %08X sec",
                                    sl_zvd_connection_status, sl_zigbee_af_get_open_network_duration_sec(), sl_zigbee_direct_anonymous_join_timeout_sec);
        sl_bt_connection_close(sl_my_connection);
        return SL_STATUS_FAIL;
      } else {
        sl_zigbee_app_debug_println("NOT Killing attempt as commissioned (%02X) and permit joining %02X sec and timeout %08X sec",
                                    sl_zvd_connection_status, sl_zigbee_af_get_open_network_duration_sec(), sl_zigbee_direct_anonymous_join_timeout_sec);
      }
      sl_zvd_connection_status_next = PROVISIONED_IN_PROVISIONING_SESSION;
      memmove(sl_zigbee_key_contents(&psk), emDlkAnonymousKeyContents, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
      break;

    case DLK_SECRET_ENUM_PRECONFIG_INSTALL_CODE:   //install code
      if (sl_zvd_connection_status == PROVISIONED_IN_PROVISIONING_SESSION) {
        sl_zvd_connection_status = PROVISIONED;
        (void)sl_token_manager_set_data(COMMON_TOKEN_PLUGIN_ZDD_AUTH_STATUS, (void *)&sl_zvd_connection_status, sizeof(uint8_t));
      }

      if (sl_zvd_connection_status >= PROVISIONED) {
        sl_zigbee_app_debug_println("Killing attempt as we are already commissioned (%02X)", sl_zvd_connection_status);
        sl_bt_connection_close(sl_my_connection);
        return SL_STATUS_FAIL;
      }
      sl_zvd_connection_status_next = PROVISIONED_IN_PROVISIONING_SESSION;
      (void)sl_token_manager_get_data(SL_TOKEN_GET_STATIC_SECURE_TOKEN(TOKEN_MFG_INSTALLATION_CODE), (void *)&tokInstallCode, sizeof(tokTypeMfgInstallationCode));
      sl_zigbee_core_debug_println("Install Code:");
      sl_zigbee_core_debug_println("CRC: %04X", tokInstallCode.crc);
      memcpy(installcode_with_crc, tokInstallCode.value, 16);
      installcode_with_crc[16] = LOW_BYTE(tokInstallCode.crc);
      installcode_with_crc[17] = HIGH_BYTE(tokInstallCode.crc);
      status = sli_zigbee_af_install_code_to_key(installcode_with_crc, 18, &psk);
      sl_zigbee_core_debug_println("Install Code based key status %02X: ", status);
      sl_zigbee_af_print_zigbee_key(sl_zigbee_key_contents(&psk));
      break;

#ifdef SL_CATALOG_ZIGBEE_DIRECT_SECURITY_CURVE25519_PRESENT
    case DLK_SECRET_ENUM_VARIABLE_LENGTH_PASSCODE:   // Variable length passcode can be used like install code, only acceptable with curve25519
      if (curve_id != DLK_ECC_CURVE_25519) {
        sl_zigbee_core_debug_println("Variable length passcode only allowed with curve25519");
        return SL_STATUS_FAIL;
      }
      sl_zigbee_core_debug_println("Variable length passcode:");
      sl_zigbee_af_print_zigbee_key(sl_my_passcode);
      if ((sl_my_passcode[0] == 0x00)
          && (memcmp(sl_my_passcode, sl_my_passcode + 1, SL_ZIGBEE_ENCRYPTION_KEY_SIZE - 1) == 0)) {
        sl_zigbee_core_debug_println("Error: Default passcode with all 0s is not acceptable.");
        return SL_STATUS_FAIL;
      }
      sl_zvd_connection_status_next = PROVISIONED_IN_PROVISIONING_SESSION;
      memmove(sl_zigbee_key_contents(&psk), sl_my_passcode, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
      break;
#endif //SL_CATALOG_ZIGBEE_DIRECT_SECURITY_CURVE25519_PRESENT

    case DLK_SECRET_ENUM_BASIC_ACCESS_KEY:   // basic key,
      sli_zigbee_direct_calculate_basic_key((uint8_t*)peerEuiData, sl_zigbee_key_contents(&basic_key));
      memmove(sl_zigbee_key_contents(&psk), sl_zigbee_key_contents(&basic_key), SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
      sl_zigbee_core_debug_println("Calculated Basic Key:");
      sl_zigbee_af_print_zigbee_key(sl_zigbee_key_contents(&basic_key));
      sl_zigbee_app_debug_println("Setting COMMISSIONED_BASIC");
      sl_zvd_connection_status_next = COMMISSIONED_BASIC;
      break;

    case DLK_SECRET_ENUM_ADMIN_ACCESS_KEY:   //Admin Key
      memmove(sl_zigbee_key_contents(&psk), admin_key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
      sl_zigbee_app_debug_println("Setting COMMISSIONED_ADMIN");
      sl_zvd_connection_status_next = COMMISSIONED_ADMIN;
      break;

    default:
      return SL_STATUS_FAIL;
      break;
  }

  sl_dlk_ecc_operation_id operation_id;
  sl_dlk_ecc_hash_id hash_id;
  // Construct the public point tlv
  if (curve_id == DLK_ECC_CURVE_25519) {
    operation_id = DLK_ECC_OPERATION_SPEKE;
    hash_id = DLK_ECC_HASH_AES_MMO_128;
    sl_zigbee_af_app_println("Curve25519");
    ourPointTlv.tag_id = SL_ZIGBEE_DIRECT_SECURITY_TLV_C25519_PUBLIC_POINT_TAG_ID;
    sl_zigbee_tlv_set_length(&ourPointTlv, SL_ZIGBEE_DIRECT_SECURITY_TLV_C25519_PUBLIC_POINT_MAX_LEN + 1);
  } else { // P256
    operation_id = DLK_ECC_OPERATION_ECDHE_PSK;
    hash_id = DLK_ECC_HASH_SHA_256;
    sl_zigbee_af_app_println("Curve P256");
    ourPointTlv.tag_id = SL_ZIGBEE_DIRECT_SECURITY_TLV_P256_PUBLIC_POINT_TAG_ID;
    sl_zigbee_tlv_set_length(&ourPointTlv, 72);//SL_ZIGBEE_DIRECT_SECURITY_TLV_P256_PUBLIC_POINT_MAX_LEN;
  }
  sl_zigbee_app_debug_println("Starting DLK with curve_id %02X and hash_id %02X length of public point %02X", curve_id, hash_id, ourPointTlv.length);
  // first 8 bytes of tlv are EUI64
  memcpy(ourPointTlv.value, sl_zigbee_get_eui64(), EUI64_SIZE);
  // the rest of the tlv is the public point
  uint8_t *publicKeyBuff = ourPointTlv.value + EUI64_SIZE;
  size_t pointSize;
  sl_status_t ret;
  sl_zb_dlk_ecc_config_t zddEccConfig = { operation_id, curve_id, hash_id };
  ret = sl_zigbee_sec_man_ecc_init(&ecdhCtx, &zddEccConfig, psk.contents);
  if (ret == SL_STATUS_OK) {
    ret = sl_zigbee_sec_man_ecc_generate_keypair(&ecdhCtx, publicKeyBuff, &pointSize);
    if (ret == SL_STATUS_OK) {
      sl_zigbee_app_debug_println("Getting the public key with size %02X", pointSize);
    } else {
      sl_zigbee_sec_man_ecc_free(&ecdhCtx);
      sl_zigbee_app_debug_println("Not ACTUALLY getting the public key with size %02X!!!", pointSize);
    }
  }

  uint8_t response_tlv_length = 0;
  if (curve_id == DLK_ECC_CURVE_25519) {
    memcpy(our_public_point, ourPointTlv.value + EUI64_SIZE, 64);
    response_tlv_length = SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_C25519_MAX_LEN;
  } else {
    memcpy(our_public_point, ourPointTlv.value + EUI64_SIZE, 64);
    response_tlv_length = SL_ZIGBEE_DIRECT_DLK_TLV_PUBLIC_KEY_P256_MAX_LEN;
  }
  // clear the flag and set it to indicate whether we are sending a request or response
  sl_zigbee_app_debug_println("pointSize %X", pointSize);

  //finish key negotiation
  sl_zigbee_key_data_t keyResult;
  // if key negotiation was successful construct the tlv response
  uint8_t *peerKeyBytes = peerPointTlv.value + EUI64_SIZE;
  sl_status_t finishStatus = SL_STATUS_FAIL;
  ret = sl_zigbee_sec_man_ecc_extract_shared_secret(&ecdhCtx, peerKeyBytes, pointSize);
  if (ret == SL_STATUS_OK) {
    if (ecdhCtx.config.operation_id == DLK_ECC_OPERATION_SPEKE) {
      ret = sl_zigbee_sec_man_speke_expand_shared_secret(&ecdhCtx, sl_zigbee_get_eui64(), *peerEuiData);
    }
    if (ecdhCtx.config.operation_id == DLK_ECC_OPERATION_ECDHE_PSK) {
      ret = sli_zigbee_direct_p256_expand_shared_secret(&ecdhCtx);
    }
    if (ret == SL_STATUS_OK) {
      ret = sl_zigbee_sec_man_ecc_derive_link_key(&ecdhCtx);
      if (ret == SL_STATUS_OK) {
        memcpy(keyResult.contents, ecdhCtx.derived_key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
        finishStatus = SL_STATUS_OK;
      }
    }
  }

  pointy2 = response;
  length = SL_ZIGBEE_DIRECT_RESPONSE_MIN_LENGTH + response_tlv_length + TLV_TAG_LENGTH_SIZE;

  if (finishStatus == SL_STATUS_OK) {
    constructKeyNegotiationTlvPayload(length,
                                      response,
                                      SL_ZIGBEE_DIRECT_RESPONSE_MIN_LENGTH,
                                      SLI_ZIGBEE_DIRECT_SESSION_KEY_RESPONSE);
    // set emberTransientKeyTimeoutS to reserved value to prevent session key from timing out
    uint16_t currentTransientKeyTimeoutS = sl_zigbee_get_transient_key_timeout_s();
    sl_zigbee_set_transient_key_timeout_s(SL_ZIGBEE_TRANSIENT_KEY_SESSION_TIMEOUT_VAL);
    // store session key
    sl_zigbee_sec_man_import_transient_key(*peerEuiData, (sl_zigbee_sec_man_key_t*)&keyResult);
    // reset transient key timeout
    sl_zigbee_set_transient_key_timeout_s(currentTransientKeyTimeoutS);
  } else {
    sl_zigbee_app_debug_println("Error finishing key negotiation");
  }

  sl_zigbee_sec_man_ecc_free(&ecdhCtx);
  //send_response:

  pointy2[0] = 0x02;

  sl_zigbee_app_debug_print("Response has length %02X and is [", length);
  for (uint16_t i = 0; i < length; i++) {
    sl_zigbee_app_debug_print(" %02X", pointy2[i]);
  }
  sl_zigbee_app_debug_print("]");

  sendStatus = sl_bt_gatt_server_send_indication(sl_my_connection,
                                                 characteristic,
                                                 length,
                                                 pointy2);

  if (sendStatus != 0) {
    sl_zigbee_app_debug_println("Error sending message 2 with status %04X", sendStatus);
    return SL_STATUS_FAIL;
  } else {
    sl_zigbee_app_debug_println("Message 2 sent successfully");
    return SL_STATUS_OK;
  }
}
