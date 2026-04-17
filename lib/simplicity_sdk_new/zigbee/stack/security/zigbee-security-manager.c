/***************************************************************************//**
 * @file
 * @brief Implementation of security-related APIs, including key storage, key
 *   fetching, and crypto operations. Keys are ultimately stored in Secure Vault
 *   for Vault-capable parts (like Series 2 chips) or in NVM3 tokens.
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

#include "stack/include/zigbee-security-manager.h"
#include "stack/security/zigbee-security-manager-internal.h"
#include "stack/include/security.h"
#include "stack/include/stack-info.h"
#include "stack/core/sl_zigbee_multi_network.h"
#include "stack/internal/inc/internal-defs-patch.h"
#include "stack/include/sl_zigbee_types_internal.h"
#include "hal.h" // for TOKEN_resolution
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "sl_token_manager_api.h"

// Externs

extern bool sli_zigbee_get_trust_center_eui64(sl_802154_long_addr_t address);
extern sl_status_t sli_zigbee_get_transient_key_table_entry(uint8_t index, sl_zigbee_transient_key_data_t *transientKeyData);
extern sl_status_t sli_zigbee_get_key_table_entry(uint8_t index, sl_zigbee_key_struct_t *result);
extern bool findTransientLinkKey(const sl_802154_long_addr_t eui64ToFind,
                                 sl_zigbee_transient_key_data_t *keyDataReturn,
                                 sl_zigbee_key_struct_bitmask_t* bitmask);
extern void sli_zigbee_stack_token_primitive(bool tokenRead,
                                             void* tokenStruct,
                                             uint32_t tokenAddress,
                                             uint8_t length);
extern bool sli_zigbee_is_token_data_initialized(uint8_t* data, uint8_t length);
extern uint8_t sli_zigbee_find_key_table_entry(sl_802154_long_addr_t address, bool linkKey, uint8_t bitmask);
extern sl_status_t sli_zigbee_get_link_key(sl_802154_long_addr_t partner,
                                           sl_zigbee_key_data_t* key,
                                           sl_zigbee_sec_man_context_t* context,
                                           uint32_t **frameCounterLocLoc,
                                           uint8_t getKeyBitmask);
#define sli_zigbee_get_security_state(item) ((sli_zigbee_security_state_bitmask & (item)) == (item))
extern void sli_legacy_mfg_security_config_modify_key(sl_zigbee_key_data_t* key);

// Extended key table
void sli_zigbee_stack_fetch_key_table_entry_at_index(uint8_t index, tokTypeStackKeyTable *tok);
void sli_zigbee_stack_set_key_table_entry_at_index(uint8_t index, tokTypeStackKeyTable* tok);

sl_status_t sli_zigbee_stack_sec_man_import_key(sl_zigbee_sec_man_context_t* context,
                                                const sl_zigbee_sec_man_key_t* plaintext_key)
{
  sl_status_t status = SL_STATUS_INVALID_PARAMETER;

  if (context == NULL
      || plaintext_key == NULL
      || context->derived_type != SL_ZB_SEC_MAN_DERIVED_KEY_TYPE_NONE) {
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }

  switch (context->core_key_type) {
    case SL_ZB_SEC_MAN_KEY_TYPE_NETWORK:
      status = zb_sec_man_store_nwk_key(context, plaintext_key);
      break;
    case SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK:
      status = zb_sec_man_store_tc_link_key(context, plaintext_key);
      break;
    case SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK_WITH_TIMEOUT:
      status = zb_sec_man_store_transient_key(context, plaintext_key);
      break;
    case SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK:
      status = zb_sec_man_store_in_link_key_table(context, plaintext_key);
      break;
#if defined(SL_CATALOG_ZIGBEE_LIGHT_LINK_PRESENT) || defined(SL_ZIGBEE_TEST)
    case SL_ZB_SEC_MAN_KEY_TYPE_ZLL_ENCRYPTION_KEY:
    case SL_ZB_SEC_MAN_KEY_TYPE_ZLL_PRECONFIGURED_KEY:
      status = zb_sec_man_store_zll_key(context, plaintext_key);
      break;
#endif // defined(SL_CATALOG_ZIGBEE_LIGHT_LINK_PRESENT) || defined(SL_ZIGBEE_TEST)
#if defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(SL_ZIGBEE_TEST)
    case SL_ZB_SEC_MAN_KEY_TYPE_GREEN_POWER_PROXY_TABLE_KEY:
    case SL_ZB_SEC_MAN_KEY_TYPE_GREEN_POWER_SINK_TABLE_KEY:
      status = zb_sec_man_store_gp_key(context, plaintext_key);
      break;
#endif // defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(SL_ZIGBEE_TEST)
    case SL_ZB_SEC_MAN_KEY_TYPE_INTERNAL:
      status = zb_sec_man_store_internal_key(context, plaintext_key);
      break;
    default:
      break;
  }

  return status;
}

sl_status_t sli_zigbee_stack_sec_man_export_key(sl_zigbee_sec_man_context_t* context,
                                                sl_zigbee_sec_man_key_t* plaintext_key)
{
  sl_status_t status = SL_STATUS_INVALID_PARAMETER;
  if (plaintext_key == NULL || context == NULL) {
    return status;
  }

  switch (context->core_key_type) {
    case SL_ZB_SEC_MAN_KEY_TYPE_NETWORK:
      status = zb_sec_man_fetch_nwk_key(context, plaintext_key);
      break;
    case SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK:
      status = zb_sec_man_fetch_tc_link_key(context, plaintext_key);
      break;
    case SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK_WITH_TIMEOUT:
      status = zb_sec_man_fetch_transient_key(context, plaintext_key);
      break;
    case SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK:
      status = zb_sec_man_fetch_from_link_key_table(context, plaintext_key);
      break;
#if defined(SL_CATALOG_ZIGBEE_LIGHT_LINK_PRESENT) || defined(SL_ZIGBEE_TEST)
    case SL_ZB_SEC_MAN_KEY_TYPE_ZLL_ENCRYPTION_KEY:
    case SL_ZB_SEC_MAN_KEY_TYPE_ZLL_PRECONFIGURED_KEY:
      status = zb_sec_man_fetch_zll_key(context, plaintext_key);
      break;
#endif // defined(SL_CATALOG_ZIGBEE_LIGHT_LINK_PRESENT) || defined(SL_ZIGBEE_TEST)
#if defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(SL_ZIGBEE_TEST)
    case SL_ZB_SEC_MAN_KEY_TYPE_GREEN_POWER_PROXY_TABLE_KEY:
    case SL_ZB_SEC_MAN_KEY_TYPE_GREEN_POWER_SINK_TABLE_KEY:
      status = zb_sec_man_fetch_gp_key(context, plaintext_key);
      break;
#endif // defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(SL_ZIGBEE_TEST)
    case SL_ZB_SEC_MAN_KEY_TYPE_INTERNAL:
      status = zb_sec_man_fetch_internal_key(context, plaintext_key);
      break;
    default:
      break;
  }

  //Clear key data if read protection token instructs this.
  sli_legacy_mfg_security_config_modify_key((sl_zigbee_key_data_t*) plaintext_key);
  return status;
}

sl_status_t sli_zigbee_stack_sec_man_import_link_key(uint8_t index,
                                                     sl_802154_long_addr_t address,
                                                     sl_zigbee_sec_man_key_t* plaintext_key)
{
  sl_status_t status;

  sl_zigbee_sec_man_context_t context;
  sli_zigbee_stack_sec_man_init_context(&context);

  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  // Right now, a null address deletes the entry. This will be
  // eventually reworked
  if (address) {
    memmove(context.eui64, address, EUI64_SIZE);
  }
  context.key_index = index;

  status = sli_zigbee_stack_sec_man_import_key(&context, plaintext_key);

  return status;
}

sl_status_t sli_zigbee_stack_sec_man_export_link_key_by_index(uint8_t index,
                                                              sl_zigbee_sec_man_context_t* context,
                                                              sl_zigbee_sec_man_key_t* plaintext_key,
                                                              sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_status_t status;

  sli_zigbee_stack_sec_man_init_context(context);

  context->core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  context->key_index = index;
  context->flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;

  status = sli_zigbee_stack_sec_man_export_key(context, plaintext_key);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }

  status = sli_zigbee_stack_sec_man_get_aps_key_info(context, key_data);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }

  return status;
}

sl_status_t sli_zigbee_stack_sec_man_export_link_key_by_eui(sl_802154_long_addr_t eui,
                                                            sl_zigbee_sec_man_context_t* context,
                                                            sl_zigbee_sec_man_key_t* plaintext_key,
                                                            sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_status_t status;

  sli_zigbee_stack_sec_man_init_context(context);

  context->core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  if (eui != NULL) {
    memmove(context->eui64, eui, EUI64_SIZE);
  }
  context->flags |= ZB_SEC_MAN_FLAG_EUI_IS_VALID;

  if (plaintext_key == NULL) {
    //For callers who want this to return a key's index but not its actual value
    context->key_index = sli_zigbee_find_key_table_entry(eui, true, 0);
    status = (context->key_index != 0xFF) ? SL_STATUS_OK : SL_STATUS_NOT_FOUND;
  } else {
    status = sli_zigbee_stack_sec_man_export_key(context, plaintext_key);
  }

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }
  context->flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;

  if (eui != NULL && key_data != NULL) {
    //populate key_data if we did find a link key for this EUI
    (void) sli_zigbee_stack_sec_man_get_aps_key_info(context, key_data);
  }

  return status;
}

sl_status_t sli_zigbee_stack_sec_man_get_aps_key_info(sl_zigbee_sec_man_context_t* context,
                                                      sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_status_t status = SL_STATUS_INVALID_TYPE;

  memset(key_data, 0, sizeof(*key_data));

  switch (context->core_key_type) {
    case SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK:
      status = zb_sec_man_fetch_tc_link_key_info(context, key_data);
      break;
    case SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK_WITH_TIMEOUT:
      status = zb_sec_man_fetch_transient_key_info(context, key_data);
      break;
    case SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK:
      status = zb_sec_man_fetch_link_key_table_key_info(context, key_data);
      break;
    default:
      break;
  }

  return status;
}

sl_status_t sli_zigbee_stack_sec_man_import_transient_key(sl_802154_long_addr_t eui64,
                                                          sl_zigbee_sec_man_key_t* plaintext_key)
{
  return zb_sec_man_import_transient_key(eui64, plaintext_key, ZB_SEC_MAN_FLAG_NONE);
}

sl_status_t zb_sec_man_import_transient_key(sl_802154_long_addr_t eui64,
                                            sl_zigbee_sec_man_key_t* plaintext_key,
                                            sl_zigbee_sec_man_flags_t flags)
{
  sl_status_t status;

  sl_zigbee_sec_man_context_t context;
  sli_zigbee_stack_sec_man_init_context(&context);

  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK_WITH_TIMEOUT;
  context.flags = flags;
  memmove(context.eui64, eui64, EUI64_SIZE);

  status = sli_zigbee_stack_sec_man_import_key(&context, plaintext_key);

  return status;
}

sl_status_t sli_zigbee_stack_sec_man_export_transient_key_by_eui(sl_802154_long_addr_t eui,
                                                                 sl_zigbee_sec_man_context_t* context,
                                                                 sl_zigbee_sec_man_key_t* plaintext_key,
                                                                 sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_status_t status;

  sli_zigbee_stack_sec_man_init_context(context);

  context->core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK_WITH_TIMEOUT;
  memmove(context->eui64, eui, EUI64_SIZE);
  context->flags |= ZB_SEC_MAN_FLAG_EUI_IS_VALID;

  status = sli_zigbee_stack_sec_man_export_key(context, plaintext_key);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }

  status = sli_zigbee_stack_sec_man_get_aps_key_info(context, key_data);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }

  return status;
}

sl_status_t sli_zigbee_stack_sec_man_export_transient_key_by_index(uint8_t index,
                                                                   sl_zigbee_sec_man_context_t* context,
                                                                   sl_zigbee_sec_man_key_t* plaintext_key,
                                                                   sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_status_t status;

  sli_zigbee_stack_sec_man_init_context(context);

  context->core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_TC_LINK_WITH_TIMEOUT;
  context->key_index = index;
  context->flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;

  status = sli_zigbee_stack_sec_man_export_key(context, plaintext_key);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }

  status = sli_zigbee_stack_sec_man_get_aps_key_info(context, key_data);

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }

  return status;
}

sl_status_t sli_zigbee_sec_man_export_derived_key(sl_zigbee_sec_man_context_t* context,
                                                  sl_zigbee_sec_man_key_t* derived_key)
{
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_status_t status = sli_zigbee_stack_sec_man_export_key(context, &plaintext_key);
  if (status != SL_STATUS_OK) {
    return status;
  }
  status = zb_sec_man_derive_key(&plaintext_key, context, derived_key);
  return status;
}

void zb_sec_man_set_network_key_info(sl_zigbee_sec_man_network_key_info_t* network_key_info)
{
  tokTypeStackKeys tok;

  sli_zigbee_stack_token_primitive(true, &tok, COMMON_TOKEN_STACK_KEYS, sizeof(tokTypeStackKeys));
  if (tok.activeKeySeqNum != network_key_info->network_key_sequence_number) {
    tok.activeKeySeqNum = network_key_info->network_key_sequence_number;
    sli_zigbee_stack_token_primitive(false, &tok, COMMON_TOKEN_STACK_KEYS, sizeof(tokTypeStackKeys));
  }

  sli_zigbee_stack_token_primitive(true, &tok, COMMON_TOKEN_STACK_ALTERNATE_KEY, sizeof(tokTypeStackKeys));
  if (tok.activeKeySeqNum != network_key_info->alt_network_key_sequence_number) {
    tok.activeKeySeqNum = network_key_info->alt_network_key_sequence_number;
    sli_zigbee_stack_token_primitive(false, &tok, COMMON_TOKEN_STACK_ALTERNATE_KEY, sizeof(tokTypeStackKeys));
  }
}

sl_status_t zb_sec_man_fetch_tc_link_key_info(sl_zigbee_sec_man_context_t* context,
                                              sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_zigbee_current_security_state_t securityState;
  sl_status_t status = sli_zigbee_stack_get_current_security_state(&securityState);

  if ((status != SL_STATUS_OK)
      || (!(securityState.bitmask & SL_ZIGBEE_HAVE_TRUST_CENTER_LINK_KEY_TOKEN))) {
    return SL_STATUS_INVALID_MODE;
  }

  memset(key_data, 0, sizeof(*key_data));

  sl_802154_long_addr_t tcAddress;
  if (sli_zigbee_get_trust_center_eui64(tcAddress)) {
    if ((context->flags & ZB_SEC_MAN_FLAG_EUI_IS_VALID)
        && (0 != memcmp(context->eui64, tcAddress, EUI64_SIZE))) {
      return SL_STATUS_NOT_FOUND;
    }
    memmove(context->eui64, tcAddress, EUI64_SIZE);
    context->flags |= ZB_SEC_MAN_FLAG_EUI_IS_VALID;
    key_data->bitmask |= SL_ZIGBEE_KEY_HAS_PARTNER_EUI64;
  }

  key_data->bitmask |= SL_ZIGBEE_KEY_HAS_OUTGOING_FRAME_COUNTER;
  key_data->outgoing_frame_counter = sli_zigbee_stack_get_aps_frame_counter();

  if (sli_zigbee_stack_get_node_id() != 0x0000) {
    key_data->incoming_frame_counter = sli_zigbee_incoming_tc_link_key_frame_counter;
    key_data->bitmask |= SL_ZIGBEE_KEY_HAS_INCOMING_FRAME_COUNTER;
  }

  if (!sli_zigbee_get_security_state(KEY_IS_NOT_AUTHORIZED) ) {
    key_data->bitmask |= SL_ZIGBEE_KEY_IS_AUTHORIZED;
  }

  if (sli_zigbee_get_security_state(SL_ZIGBEE_TC_SUPPORTS_FC_SYNC)) {
    key_data->bitmask |= SL_ZIGBEE_KEY_FC_SYNC_SUPPORTED;
  }

  return SL_STATUS_OK;
}

sl_status_t zb_sec_man_fetch_transient_key_info(sl_zigbee_sec_man_context_t* context,
                                                sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_status_t status = SL_STATUS_FAIL;
  sl_zigbee_transient_key_data_t transientKeyData;

  if (context->flags & ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID) {
    status = sli_zigbee_get_transient_key_table_entry(context->key_index, &transientKeyData);
  } else if (context->flags & ZB_SEC_MAN_FLAG_EUI_IS_VALID) {
    // sli_zigbee_stack_sec_man_export_transient_key_by_eui is called when using the original transient and not the
    // unconfirmed transient key. Hence initializing the bitmask to 0.
    sl_zigbee_key_struct_bitmask_t bitmask = 0;
    bool found = findTransientLinkKey(context->eui64, &transientKeyData, &bitmask);
    status = found ? SL_STATUS_OK : SL_STATUS_NOT_FOUND;
  }

  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }

  memset(key_data, 0, sizeof(*key_data));

  key_data->bitmask = transientKeyData.bitmask;
  key_data->incoming_frame_counter = transientKeyData.incomingFrameCounter;
  key_data->ttl_in_seconds = transientKeyData.remainingTimeSeconds;

  context->multi_network_index = transientKeyData.networkIndex;
  memmove(context->eui64, transientKeyData.eui64, sizeof(sl_802154_long_addr_t));

  return SL_STATUS_OK;
}

sl_status_t zb_sec_man_fetch_link_key_table_key_info(sl_zigbee_sec_man_context_t* context,
                                                     sl_zigbee_sec_man_aps_key_metadata_t* key_data)
{
  sl_status_t status = SL_STATUS_FAIL;
  sl_zigbee_key_struct_t result;
  if (context->flags & ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID) {
    status = sli_zigbee_get_key_table_entry(context->key_index, &result);
  }
  if (status != SL_STATUS_OK) {
    return SL_STATUS_NOT_FOUND;
  }
  memmove(context->eui64, result.partnerEUI64, EUI64_SIZE);
  context->flags |= ZB_SEC_MAN_FLAG_EUI_IS_VALID;
  memset(key_data, 0, sizeof(*key_data));
  key_data->bitmask = result.bitmask;
  key_data->incoming_frame_counter = result.incomingFrameCounter;
  key_data->bitmask |= SL_ZIGBEE_KEY_HAS_INCOMING_FRAME_COUNTER;
  key_data->outgoing_frame_counter = result.outgoingFrameCounter;

  return SL_STATUS_OK;
}

uint8_t zb_sec_man_version(void)
{
  return ZB_SEC_MAN_VERSION;
}

/// NOTE authentication token is only required for ZigBee Revision 23 and greater

// Symmetric Passphrase Functions

// NOTE auth token is in key table entry
// NOTE copied from aps-keys-full.h
// This bit indicates if entry in the key table is a Symmetric Passphrase
#define KEY_TABLE_SYMMETRIC_PASSPHRASE      (BIT(7))

uint8_t sli_zigbee_stack_sec_man_find_symmetric_passphrase_key_table_index(sl_802154_long_addr_t address)
{
  // Look for the entry in the key table for the non-null address
  if (address != NULL) {
    return sli_zigbee_find_key_table_entry(address, true, KEY_TABLE_SYMMETRIC_PASSPHRASE);
  }
  return 0xFF;
}

sl_status_t sli_zigbee_stack_sec_man_import_symmetric_passphrase(uint8_t index,
                                                                 sl_802154_long_addr_t address,
                                                                 const sl_zigbee_key_data_t* key_data)
{
  sl_status_t status;

  sl_zigbee_sec_man_context_t context;
  sli_zigbee_stack_sec_man_init_context(&context);

  memcpy(&context.eui64, address, sizeof(sl_802154_long_addr_t));
  // NOTE APP_LINK key type is a little misleading, as the symmetric
  // passphrase is technically not a link key, however this key type
  // corresponds to the correct storage type for the symmetric passphrase
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  context.key_index = index;

  context.flags |= ZB_SEC_MAN_FLAG_SYMMETRIC_PASSPHRASE;
  status = sli_zigbee_stack_sec_man_import_key(&context, (sl_zigbee_sec_man_key_t *) key_data);

  return status;
}

sl_status_t sli_zigbee_stack_sec_man_update_symmetric_passphrase_eui(sl_802154_long_addr_t old_eui64, sl_802154_long_addr_t new_eui64)
{
  uint8_t index = sli_zigbee_stack_sec_man_find_symmetric_passphrase_key_table_index(old_eui64);
  if (index == 0xFF) {
    return SL_STATUS_NOT_FOUND;
  }
  tokTypeStackKeyTable tok;
  sli_zigbee_stack_fetch_key_table_entry_at_index(index, &tok);
  memmove(&(tok[KEY_ENTRY_IEEE_OFFSET]),
          new_eui64,
          EUI64_SIZE);
  sli_zigbee_stack_set_key_table_entry_at_index(index, &tok);
  return SL_STATUS_OK;
}

sl_status_t sli_zigbee_stack_sec_man_export_symmetric_passphrase(sl_802154_long_addr_t address,
                                                                 sl_zigbee_key_data_t* key_data)
{
  uint8_t index = sli_zigbee_stack_sec_man_find_symmetric_passphrase_key_table_index(address);
  if (index == 0xFF) {
    return SL_STATUS_FAIL;
  }

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_aps_key_metadata_t metadata;

  return sli_zigbee_stack_sec_man_export_link_key_by_index(index, &context, (sl_zigbee_sec_man_key_t *) key_data, &metadata);
}

bool sli_zigbee_stack_sec_man_have_link_key(sl_802154_long_addr_t eui)
{
  // Since we are simply checking for the existence of a key,
  // we don't care if it is authorized or not.
  return (SL_STATUS_OK
          == sli_zigbee_get_link_key(eui,
                                     NULL, // key return data
                                     NULL, // context return data
                                     NULL, // frame counter location
                                     0)); // key is not transient, and does not need to be authorized
}

bool sli_zigbee_stack_sec_man_link_key_slot_available(sl_802154_long_addr_t eui64)
{
  sl_zigbee_sec_man_context_t context_existing;
  sl_zigbee_sec_man_context_t context_open;
  sli_zigbee_stack_sec_man_export_link_key_by_eui(eui64, &context_existing, NULL, NULL);
  sli_zigbee_stack_sec_man_export_link_key_by_eui(NULL, &context_open, NULL, NULL);
  if (0xFF != context_existing.key_index
      || 0xFF != context_open.key_index) {
    return true;
  }
  return false;
}

// NOTE we support an additional 127 link keys by allocating a second indexed token
// and redirecting key table access to the second key table when the index exceeds
// the boundaries of the original token.
// main benefits to this approach is being able to leave original tokens and keys
// 'in place' simplifying the requirements for upgrade

#define STACK_KEY_TABLE_MAX_COMBINED_ENTRIES 254
#define ORIGINAL_TOKEN_STACK_KEY_TABLE_MAX_INDEX 0x7F
#define STACK_KEY_TABLE_EXTENDED_MAX_LENGTH (STACK_KEY_TABLE_MAX_COMBINED_LENGTH - ORIGINAL_TOKEN_STACK_KEY_TABLE_MAX_INDEX)

void sli_zigbee_stack_fetch_key_table_entry_at_index(uint8_t index, tokTypeStackKeyTable *tok)
{
  // Set token to some invalid initial value
  memset(tok, 0xFF, sizeof(tokTypeStackKeyTable));
  if (index < ORIGINAL_TOKEN_STACK_KEY_TABLE_MAX_INDEX) {
    (void)sl_token_manager_get_data(COMMON_TOKEN_STACK_KEY_TABLE + index, (void *)tok, sizeof(tokTypeStackKeyTable));
  } else {
    (void)sl_token_manager_get_data(COMMON_TOKEN_STACK_KEY_TABLE_EXTENDED + (index - ORIGINAL_TOKEN_STACK_KEY_TABLE_MAX_INDEX), (void *)tok, sizeof(tokTypeStackKeyTable));
  }
}

void sli_zigbee_stack_set_key_table_entry_at_index(uint8_t index, tokTypeStackKeyTable *tok)
{
  if (index < ORIGINAL_TOKEN_STACK_KEY_TABLE_MAX_INDEX) {
    (void)sl_token_manager_set_data(COMMON_TOKEN_STACK_KEY_TABLE + index, (void *)tok, sizeof(tokTypeStackKeyTable));
  } else {
    (void)sl_token_manager_set_data(COMMON_TOKEN_STACK_KEY_TABLE_EXTENDED + (index - ORIGINAL_TOKEN_STACK_KEY_TABLE_MAX_INDEX), (void *)tok, sizeof(tokTypeStackKeyTable));
  }
}

sl_status_t slxi_zigbee_stack_set_trust_center_additional_info(uint32_t tok)
{
  return sl_token_manager_set_data(COMMON_TOKEN_STACK_TRUST_CENTER_ADDITIONAL_INFO, (void *)&tok, sizeof(tokStackTrustCenterAdditionalInfo));
}

sl_status_t slxi_zigbee_stack_get_trust_center_additional_info(uint32_t *tok)
{
  return sl_token_manager_get_data(COMMON_TOKEN_STACK_TRUST_CENTER_ADDITIONAL_INFO, tok, sizeof(tokStackTrustCenterAdditionalInfo));
}

sl_status_t slxi_zigbee_stack_set_key_table_additional_info(uint8_t index, uint32_t tok)
{
  return sl_token_manager_set_data(COMMON_TOKEN_STACK_KEY_TABLE_ADDITIONAL_INFO + index, (void *)&tok, sizeof(tokStackKeyTableAdditionalInfo));
}

sl_status_t slxi_zigbee_stack_get_key_table_additional_info(uint8_t index, uint32_t *tok)
{
  return sl_token_manager_get_data(COMMON_TOKEN_STACK_KEY_TABLE_ADDITIONAL_INFO + index, tok, sizeof(tokStackKeyTableAdditionalInfo));
}
