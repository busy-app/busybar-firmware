/***************************************************************************//**
 * @file
 * @brief Implementation of Zigbee Security Manager for host.
 * The implementation itself stores the internal/one-off key type on host and
 * contains software implementations for cryptographic APIs.
 * Operations dealing with access to key types wrap around EZSP frames; this is done
 * so host code calling security functions can use the same function structure as
 * SoC/NCP code.
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

//TODO: are all of these needed for basic EZSP support?
#include "stack/include/sl_zigbee_types.h"
#include "stack/include/sl_zigbee_random_api.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"
#include "app/util/ezsp/ezsp-utils.h"
#include "app/util/ezsp/serial-interface.h"
#include "app/util/ezsp/command-prototypes.h"
#include "app/util/ezsp/ezsp-enum.h"

#include "stack/include/zigbee-security-manager.h"
#include "stack/security/zigbee-security-manager-internal.h"
#include "stack/security/zigbee-security-manager-host.h"

#if defined (SL_CATALOG_ZIGBEE_AES_SOFTWARE_PRESENT)
#include "stack/platform/micro/aes.h"
#include "stack/include/aes-mmo.h"

// The global context key, or key that is used in crypto operations.
// Place inside this define to avoid unused variable warning.
static uint8_t zb_sec_man_context_key[SL_ZIGBEE_ENCRYPTION_KEY_SIZE];
#endif

// A scratch key that we only need temporarily, like CBKE operations
static uint8_t zb_sec_man_internal_key[SL_ZIGBEE_ENCRYPTION_KEY_SIZE];

//MIC length used for most CCM purposes
//not unused but compiler may throw such a warning in tests
UNUSED static uint8_t stored_mic_length = 4;

sl_status_t sl_zigbee_sec_man_import_key(sl_zigbee_sec_man_context_t* context,
                                         const sl_zigbee_sec_man_key_t* plaintext_key)
{
  sl_status_t status;
  switch (context->core_key_type) {
    case SL_ZB_SEC_MAN_KEY_TYPE_INTERNAL:
      //this one-off key is used locally
      status = zb_sec_man_store_internal_key(context, plaintext_key);
      break;
    default:
      //other key types are being imported onto the NCP
      status = sl_zigbee_ezsp_sec_man_import_key(context, (sl_zigbee_sec_man_key_t*)plaintext_key);
      if (context->core_key_type == SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK
          && context->key_index == 0xFF) {
        //Find out which index the key got stored in (SoC implementation
        //will modify context but the EZSP frame doesn't send that back)
        sl_zigbee_sec_man_context_t test_context;
        sl_zigbee_sec_man_export_link_key_by_eui(context->eui64,
                                                 &test_context,
                                                 NULL,
                                                 NULL);
        context->key_index = test_context.key_index;
      }
      break;
  }
  return status;
}

sl_status_t sl_zigbee_sec_man_export_key(sl_zigbee_sec_man_context_t* context,
                                         sl_zigbee_sec_man_key_t* plaintext_key)
{
  sl_status_t status;
  switch (context->core_key_type) {
    case SL_ZB_SEC_MAN_KEY_TYPE_INTERNAL:
      status = zb_sec_man_fetch_internal_key(context, plaintext_key);
      break;
    default:
      status = sl_zigbee_ezsp_sec_man_export_key(context, plaintext_key);
      break;
  }
  return status;
}

void sl_zigbee_sec_man_init_context(sl_zigbee_sec_man_context_t* context)
{
  memset(context, 0, sizeof(*context));
  context->psa_key_alg_permission = ZB_PSA_ALG;
}

uint8_t sl_zigbee_sec_man_version()
{
  uint8_t value = 0xFF;
  uint8_t valueLength = 1;
  sl_zigbee_ezsp_get_value(SL_ZIGBEE_EZSP_VALUE_KEY_STORAGE_VERSION, &valueLength, &value);
  return value;
}

sl_status_t sl_zigbee_sec_man_export_link_key_by_eui(sl_802154_long_addr_t eui64,
                                                     sl_zigbee_sec_man_context_t* context,
                                                     sl_zigbee_sec_man_key_t* plaintext_key,
                                                     sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_status_t status;
  //Arguments besides context are allowed to be NULL - needs to be separately handled on
  //host so the EZSP code doesn't fail by trying to write something there.
  UNUSED sl_802154_long_addr_t unused_eui;
  memset(unused_eui, 0, sizeof(sl_802154_long_addr_t));
  UNUSED sl_zigbee_sec_man_key_t unused_key;
  UNUSED sl_zigbee_sec_man_aps_key_metadata_t unused_metadata;
  sl_zigbee_sec_man_key_t* key_ptr;
  sl_zigbee_sec_man_aps_key_metadata_t* metadata_ptr;
  if (plaintext_key == NULL) {
    key_ptr = &unused_key;
  } else {
    key_ptr = plaintext_key;
  }
  if (key_data == NULL) {
    metadata_ptr = &unused_metadata;
  } else {
    metadata_ptr = key_data;
  }
  if (eui64 == NULL) {
    status = sl_zigbee_ezsp_sec_man_export_link_key_by_eui(unused_eui, context, key_ptr, metadata_ptr);
  } else {
    status = sl_zigbee_ezsp_sec_man_export_link_key_by_eui(eui64, context, key_ptr, metadata_ptr);
  }

  return status;
}

#if defined (SL_CATALOG_ZIGBEE_AES_SOFTWARE_PRESENT)
//helper function for HMAC w/ AES-MMO, ported from hmac.c to avoid
//dependency on the deprecated sl_zigbee_hmac_aes_hash API
static void xorKeyWithByte(const uint8_t key[ENCRYPTION_BLOCK_SIZE],
                           uint8_t byte,
                           uint8_t result[ENCRYPTION_BLOCK_SIZE])
{
  uint8_t i;
  for (i = 0; i < ENCRYPTION_BLOCK_SIZE; i++) {
    result[i] = key[i] ^ byte;
  }
}

//Code ported over from sl_zigbee_hmac_aes_hash; has Zigbee security manager
//handle key loading instead of taking in key data as an argument.

void sl_zigbee_sec_man_hmac_aes_mmo(const uint8_t* input,
                                    const uint8_t data_length,
                                    uint8_t* output)
{
  //load key from other function calls first
  sli_zigbee_get_key_from_core((uint8_t*) &zb_sec_man_context_key);

  uint8_t buffer[ENCRYPTION_BLOCK_SIZE];
  uint8_t run;
  sl_zigbee_aes_mmo_hash_context_t context;

  for (run = 0; run < 2; run++) {
    xorKeyWithByte((const uint8_t*) &zb_sec_man_context_key,
                   (run == 0
                    ? 0x36
                    : 0x5C),
                   buffer);

    sl_zigbee_aes_mmo_hash_init(&context);
    sl_zigbee_aes_mmo_hash_update(&context, ENCRYPTION_BLOCK_SIZE, buffer);
    sl_zigbee_aes_mmo_hash_final(&context,
                                 (run == 0
                                  ? data_length
                                  : ENCRYPTION_BLOCK_SIZE),
                                 (run == 0
                                  ? input
                                  : output));
    memmove(output, context.result, ENCRYPTION_BLOCK_SIZE);
  }
}

#define STANDALONE_FLAGS_INDEX                   0
#define STANDALONE_NONCE_INDEX                   1
#define STANDALONE_VARIABLE_FIELD_INDEX_HIGH    14
#define STANDALONE_VARIABLE_FIELD_INDEX_LOW     15

#ifndef TEMP_BUFFER_SIZE
#define TEMP_BUFFER_SIZE 256
#endif

static uint8_t encryptionBlock[ENCRYPTION_BLOCK_SIZE];

static void encryptNonce(uint8_t *nonce,
                         uint8_t flagsMask,
                         uint16_t variableField,
                         uint8_t *block)
{
  memcpy(block + STANDALONE_NONCE_INDEX, nonce, NONCE_LENGTH);

  block[STANDALONE_FLAGS_INDEX] = flagsMask;
  block[STANDALONE_VARIABLE_FIELD_INDEX_HIGH] = HIGH_BYTE(variableField);
  block[STANDALONE_VARIABLE_FIELD_INDEX_LOW] = LOW_BYTE(variableField);

  sli_util_stand_alone_encrypt_block(block);
}

static uint16_t min(uint16_t num1,
                    uint16_t num2)
{
  if (num1 <= num2) {
    return num1;
  }
  return num2;
}

static uint8_t xorBytesIntoBlock(uint8_t *block,
                                 uint8_t blockIndex,
                                 const uint8_t *bytes,
                                 uint16_t count)
{
  uint16_t i;

  for (i = 0; i < count; ) {
    uint16_t needed = ENCRYPTION_BLOCK_SIZE - blockIndex;
    uint16_t todo = count - i;
    uint16_t copied = min(todo, needed);
    uint16_t j;

    for (j = 0; j < copied; j++, blockIndex++) {
      block[blockIndex] ^= *bytes++;
    }
    i += copied;

    if (blockIndex == ENCRYPTION_BLOCK_SIZE) {
      sli_util_stand_alone_encrypt_block(block);
      blockIndex = 0;
    }
  }
  return blockIndex;
}

static void ccmEncryptData(uint8_t *bytes,
                           uint16_t length,
                           uint8_t blockCount,
                           uint8_t *nonce)
{
  while (length > 0) {
    uint8_t todo = min(length, ENCRYPTION_BLOCK_SIZE);
    uint8_t i;
    encryptNonce(nonce, 0x01, blockCount, encryptionBlock);
    blockCount += 1;

    for (i = 0; i < todo; i++) {
      *bytes++ ^= encryptionBlock[i];
    }

    length -= todo;
  }
}

static sl_status_t zb_sec_man_calc_encrypt_mic(uint8_t* nonce,
                                               uint8_t* data,
                                               uint8_t encryption_start_index,
                                               uint8_t length)
{
  uint8_t blockIndex = 2;     // skip over frame length field
  uint8_t *chunk = data;
  uint16_t chunklen;
  uint8_t phase;

  chunklen = encryption_start_index;
  encryptNonce(nonce, 0x49, length - encryption_start_index, encryptionBlock);
  // first two bytes are the 16-bit representation of the frame length,
  // high byte first
  encryptionBlock[0] ^= HIGH_BYTE(encryption_start_index);
  encryptionBlock[1] ^= LOW_BYTE(encryption_start_index);
  // phase 0: go over authenticated data
  // phase 1: go over the payload
  for (phase = 0; phase < 2; phase++) {
    blockIndex = xorBytesIntoBlock(encryptionBlock,
                                   blockIndex,
                                   chunk,
                                   chunklen);
    chunk = data + encryption_start_index;
    chunklen = length - encryption_start_index;

    // finish off authData if not on an encryption block boundary
    if (blockIndex > 0) {
      sli_util_stand_alone_encrypt_block(encryptionBlock);
      blockIndex = 0;
    }
  }

  memcpy(data + length, encryptionBlock, stored_mic_length);
  // The MIC gets encrypted as block zero of the message.
  ccmEncryptData(data + length, stored_mic_length, 0, nonce);
  return SL_STATUS_OK;
}

//Software implementation here is based on implementation for emberCcmCalculateAndEncryptMic,
//but assumes packet input/output format.
//output points to auth data, with encrypt data at encryption_start_index past it and MIC
//starting at output + length (4 bytes long).
//There is an argument to set MIC length to a value other than 4, although
//the use cases for that don't currently apply to host.
sl_status_t sl_zigbee_sec_man_aes_ccm_extended(uint8_t* nonce,
                                               bool encrypt,
                                               const uint8_t* input,
                                               uint8_t encryption_start_index,
                                               uint8_t length,
                                               uint8_t mic_length,
                                               uint8_t* output)
{
  stored_mic_length = mic_length;
  //Move input into output location for in-place encryption
  memcpy(output, input, length);
  bool authenticated;
  if (encrypt) {
    zb_sec_man_calc_encrypt_mic(nonce, output, encryption_start_index, length);
    if (length > encryption_start_index) {
      ccmEncryptData(output + encryption_start_index, length - encryption_start_index, 1, nonce);
    }
  } else {
    ccmEncryptData(output + encryption_start_index, length - encryption_start_index, 1, nonce);
    uint8_t temp[TEMP_BUFFER_SIZE];
    memcpy(temp, output, length);
    zb_sec_man_calc_encrypt_mic(nonce, temp, encryption_start_index, length);
    authenticated = memcmp(input + length, temp + length, stored_mic_length) == 0;
    if (!authenticated) {
      //MIC check failed
      return SL_STATUS_INVALID_SIGNATURE;
    }
  }

  return SL_STATUS_OK;
}
#endif //SL_CATALOG_ZIGBEE_AES_SOFTWARE_PRESENT

sl_status_t zb_sec_man_derive_key(sl_zigbee_sec_man_key_t* source_key,
                                  const sl_zigbee_sec_man_context_t* context,
                                  sl_zigbee_sec_man_key_t* derived_key)
{
  #if defined(SL_CATALOG_ZIGBEE_AES_SOFTWARE_PRESENT)
  uint8_t tag;

  //source_key is used as the key for hashing;
  //When loading a key context that includes a derived key,
  //callers must set the core key type correctly to ensure that
  //key derivation is done correctly.

  sli_util_load_key_into_core((uint8_t*) source_key);

  switch (context->derived_type) {
    case SL_ZB_SEC_MAN_DERIVED_KEY_TYPE_KEY_TRANSPORT_KEY:
    {
      tag = DERIVE_TRANSPORT_KEY_TAG;
      sl_zigbee_sec_man_hmac_aes_mmo(&tag, 1, (uint8_t*)derived_key);
      break;
    }
    case SL_ZB_SEC_MAN_DERIVED_KEY_TYPE_KEY_LOAD_KEY:
    {
      tag = DERIVE_LOAD_KEY_TAG;
      sl_zigbee_sec_man_hmac_aes_mmo(&tag, 1, (uint8_t*) derived_key);
      break;
    }
    case SL_ZB_SEC_MAN_DERIVED_KEY_TYPE_VERIFY_KEY:
    {
      tag = DERIVE_VERIFY_KEY_TAG;
      sl_zigbee_sec_man_hmac_aes_mmo(&tag, 1, (uint8_t*) derived_key);
      break;
    }
    case SL_ZB_SEC_MAN_DERIVED_KEY_TYPE_TC_SWAP_OUT_KEY:
    {
      sl_zigbee_aes_hash_simple(SL_ZIGBEE_ENCRYPTION_KEY_SIZE,
                                (const uint8_t*) source_key,
                                (uint8_t* ) derived_key);
      break;
    }
    case SL_ZB_SEC_MAN_DERIVED_KEY_TYPE_TC_HASHED_LINK_KEY:
      sl_zigbee_sec_man_hmac_aes_mmo(context->eui64, EUI64_SIZE, (uint8_t*) derived_key);
      break;
    case SL_ZB_SEC_MAN_DERIVED_KEY_TYPE_NONE:
      return SL_STATUS_INVALID_PARAMETER;
      break;
    default:
      break;
  }
  return SL_STATUS_OK;
  #else
  (void) source_key;
  (void) context;
  (void) derived_key;
  return SL_STATUS_NOT_SUPPORTED;
  #endif //SL_CATALOG_ZIGBEE_AES_SOFTWARE_PRESENT
}

sl_status_t sl_zigbee_sec_man_load_key_context(sl_zigbee_sec_man_context_t* context)
{
  #if defined(SL_CATALOG_ZIGBEE_AES_SOFTWARE_PRESENT)
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_export_key(context, &plaintext_key);
  if (context->derived_type != SL_ZB_SEC_MAN_DERIVED_KEY_TYPE_NONE) {
    sl_zigbee_sec_man_key_t derived_key;
    zb_sec_man_derive_key(&plaintext_key, context, &derived_key);
    memcpy(&zb_sec_man_context_key, &derived_key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  } else {
    memcpy(&zb_sec_man_context_key, &plaintext_key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  }
  //TODO: Investigate removing this (and only using zb_sec_man_context_key as the "core")
  sli_util_load_key_into_core((const uint8_t *)&zb_sec_man_context_key);
  return SL_STATUS_OK;
  #else
  (void) context;
  return SL_STATUS_NOT_SUPPORTED;
  #endif //SL_CATALOG_ZIGBEE_AES_SOFTWARE_PRESENT
}

sl_status_t zb_sec_man_store_internal_key(UNUSED sl_zigbee_sec_man_context_t* context,
                                          const sl_zigbee_sec_man_key_t* plaintext_key)
{
  memmove(&zb_sec_man_internal_key, plaintext_key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  return SL_STATUS_OK;
}

sl_status_t zb_sec_man_fetch_internal_key(UNUSED sl_zigbee_sec_man_context_t* context,
                                          sl_zigbee_sec_man_key_t* plaintext_key)
{
  memmove(plaintext_key, &zb_sec_man_internal_key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  return SL_STATUS_OK;
}

bool sl_zigbee_sec_man_link_key_slot_available(sl_802154_long_addr_t eui64)
{
  sl_zigbee_sec_man_context_t context_existing;
  sl_zigbee_sec_man_context_t context_open;
  sl_zigbee_sec_man_export_link_key_by_eui(eui64, &context_existing, NULL, NULL);
  sl_zigbee_sec_man_export_link_key_by_eui(NULL, &context_open, NULL, NULL);
  if (0xFF != context_existing.key_index
      || 0xFF != context_open.key_index) {
    return true;
  }

  return false;
}

bool sl_zigbee_sec_man_compare_key_to_value(sl_zigbee_sec_man_context_t* context, const sl_zigbee_sec_man_key_t* test_key)
{
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_export_key(context, &plaintext_key);

  if (memcmp(&plaintext_key, test_key, SL_ZIGBEE_ENCRYPTION_KEY_SIZE) == 0) {
    return true;
  }

  return false;
}

//other stack APIs that utilize sl_zigbee_sec_man APIs for EZSP support
uint32_t sl_zigbee_get_security_frame_counter(void)
{
  sl_zigbee_sec_man_network_key_info_t nwk_key_info;
  sl_zigbee_sec_man_get_network_key_info(&nwk_key_info);
  return nwk_key_info.network_key_frame_counter;
}

uint32_t sl_zigbee_get_aps_frame_counter(void)
{
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
  sl_zigbee_sec_man_aps_key_metadata_t aps_key_info;
  sl_zigbee_sec_man_get_aps_key_info(&context, &aps_key_info);
  return aps_key_info.outgoing_frame_counter;
}

uint32_t sl_zigbee_get_incoming_tc_link_key_frame_counter(void)
{
  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK;
  sl_zigbee_sec_man_aps_key_metadata_t aps_key_info;
  sl_zigbee_sec_man_get_aps_key_info(&context, &aps_key_info);
  return aps_key_info.incoming_frame_counter;
}

#define FLAGS_MASK         0x0006
#define SPECIAL_FLAGS_MASK 0x0600
sl_status_t sl_zigbee_get_key_from_install_code(sl_zigbee_key_data_t *key)
{
  static const uint8_t tokenFlagsToSizeArray[] = { 6, 8, 12, 16 };
  uint8_t tokenData[SL_ZIGBEE_ENCRYPTION_KEY_SIZE + 2 + 2] = { 0 }; // 2 bytes flags + max 16 bytes key + 2 bytes crc
  uint8_t code[SL_ZIGBEE_ENCRYPTION_KEY_SIZE + 2] = { 0 }; // max 16 bytes key + 2 bytes crc
  uint16_t flags;
  if (sl_zigbee_ezsp_get_mfg_token(SL_ZIGBEE_EZSP_MFG_INSTALLATION_CODE, tokenData) == 0xff) {
    return SL_STATUS_NOT_FOUND;
  }

  flags = (FLAGS_MASK & ((tokenData[1] << 8) | tokenData[0])) >> 1;
  if (flags == 0) {
    // Special flags support:  Some customers programmed the flags field
    // incorrectly and set the upper bits instead of the lower bits.  Allow
    // both to be supported.  Bugzid 9314.
    flags = (SPECIAL_FLAGS_MASK & ((tokenData[1] << 8) | tokenData[0])) >> 9;
  }
  uint8_t size = tokenFlagsToSizeArray[flags];
  memmove(code, tokenData + 2, size + 2); // skip 2 bytes of flags

  return sl_zigbee_aes_hash_simple(size + 2, (const uint8_t*) code, sl_zigbee_key_contents(key));
}
