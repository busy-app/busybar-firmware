/***************************************************************************//**
 * @file
 * @brief Implementation of additional key functionality that is
 * necessary for a Full Zigbee Pro Stack.  This includes things
 * like setting and getting the Trust Center Master Key, and
 * sending and processing Requests for Keys.
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

#include "core/sl_zigbee_multi_network.h"
#include "stack/include/sl_zigbee_types.h"
#include "stack/include/sl_zigbee_zdo_security.h"
#include "hal/hal.h"
#include "internal/inc/internal-defs-patch.h"
#include "aps-keys-full.h"
#include "aps-keys.h"
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"

#include "stack/include/cbke-crypto-engine.h"
#include "stack/include/zigbee-security-manager.h"

extern void sli_zigbee_request_key_timeout_control(bool start, bool useBdbTimeoutValues);
extern bool sli_zigbee_is_token_data_initialized(uint8_t* data, uint8_t length);
extern uint32_t sli_zigbee_get_aps_frame_counter(void);
extern bool sli_zigbee_is_null_key(sl_zigbee_key_data_t * key);
extern bool sli_zigbee_get_trust_center_eui64(sl_802154_long_addr_t address);
extern bool sli_zigbee_are_keys_authorized_by_default(void);

const sl_zigbee_library_status_t sli_zigbee_security_link_keys_library_status =
  SL_ZIGBEE_LIBRARY_PRESENT_MASK;

//------------------------------------------------------------------------------

extern void sli_zigbee_stack_fetch_key_table_entry_at_index(uint8_t index, tokTypeStackKeyTable *tok);
extern void sli_zigbee_stack_set_key_table_entry_at_index(uint8_t index, tokTypeStackKeyTable* tok);
extern sl_status_t sli_zigbee_stack_get_authentication_level(sl_802154_short_addr_t dest,
                                                             sl_802154_long_addr_t target);
//------------------------------------------------------------------------------
// The link key table.

void sli_zigbee_aps_security_read_frame_counters(void)
{
  memset(sli_zigbee_incoming_aps_frame_counters,
         0x0,
         sl_zigbee_key_table_size * sizeof(uint32_t));
}

// Copy the indicated key.  Returns false on bad index or no valid key data.
// Note: this no longer fills in result->keyData. ZB SM will fill that, grabbing
// the key from either tokens or PSA

sl_status_t sli_zigbee_get_key_table_entry(uint8_t index, sl_zigbee_key_struct_t *result)
{
  tokTypeStackKeyTable tok;

  if (sl_zigbee_key_table_size <= index) {
    return SL_STATUS_INVALID_INDEX;
  }

  sli_zigbee_stack_fetch_key_table_entry_at_index(index, &tok);

  if ( !sli_zigbee_is_token_data_initialized(&(tok[KEY_ENTRY_IEEE_OFFSET]),
                                             EUI64_SIZE + SL_ZIGBEE_ENCRYPTION_KEY_SIZE) ) {
    return SL_STATUS_INVALID_KEY;
  }

  memmove(result->partnerEUI64,
          &(tok[KEY_ENTRY_IEEE_OFFSET]),
          EUI64_SIZE);
  result->bitmask = SL_ZIGBEE_KEY_HAS_PARTNER_EUI64;

  if ((tok[KEY_ENTRY_INFO_OFFSET] & KEY_TABLE_AUTHORIZED_KEY_MASK)
      != KEY_TABLE_UNAUTHORIZED_KEY) {
    result->bitmask |= SL_ZIGBEE_KEY_IS_AUTHORIZED;
  }
  if (tok[KEY_ENTRY_INFO_OFFSET] & KEY_TABLE_SLEEPY_END_DEVICE) {
    result->bitmask |= SL_ZIGBEE_KEY_PARTNER_IS_SLEEPY;
  }
  if (tok[KEY_ENTRY_INFO_OFFSET] & KEY_TABLE_SYMMETRIC_PASSPHRASE) {
    result->bitmask |= SL_ZIGBEE_KEY_IS_AUTHENTICATION_TOKEN;
  }

  if ( (tok[KEY_ENTRY_INFO_OFFSET] & KEY_TABLE_TYPE_MASK)
       == KEY_TABLE_TYPE_LINK_KEY ) {
    result->type = SL_ZIGBEE_APPLICATION_LINK_KEY;
    result->incomingFrameCounter = sli_zigbee_incoming_aps_frame_counters[index];
    result->outgoingFrameCounter = sli_zigbee_get_aps_frame_counter();
    result->bitmask |= (SL_ZIGBEE_KEY_HAS_INCOMING_FRAME_COUNTER
                        | SL_ZIGBEE_KEY_HAS_OUTGOING_FRAME_COUNTER);
  } else {
    return SL_STATUS_INVALID_KEY;
  }

  if (tok[KEY_ENTRY_INFO_OFFSET] & KEY_TABLE_ENTRY_HAS_PSA_ID) {
    result->bitmask |= SL_ZIGBEE_KEY_HAS_PSA_ID;
  }

  return SL_STATUS_OK;
}

uint8_t sli_zigbee_find_key_table_entry(sl_802154_long_addr_t address, bool linkKey, uint8_t bitmask)
{
  sl_status_t status;

  sl_zigbee_sec_man_context_t context;
  sli_zigbee_stack_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  context.flags |= ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;

  if (!linkKey) {
    return SL_STATUS_INVALID_STATE;
  }

  uint8_t i;
  sl_802154_long_addr_t NULL_EUI = { 0, 0, 0, 0, 0, 0, 0, 0 };
  bool lookForEmptyEntry =
    (address == NULL
     || (0 == memcmp(address, NULL_EUI, EUI64_SIZE)));

  for ( i = 0; i < sl_zigbee_key_table_size; i++) {
    tokTypeStackKeyTable tok;
    //Set token to some invalid initial value
    memset(&tok, 0xFF, sizeof (tok));

    uint8_t tokenBitmask;
    sli_zigbee_stack_fetch_key_table_entry_at_index(i, &tok);
    tokenBitmask = tok[KEY_ENTRY_INFO_OFFSET];

    //Check if token read was unsuccessful and assert
    if ( tokenBitmask == 0xFF ) {
      assert(false);
    }

    if (lookForEmptyEntry) {
      if (!sli_zigbee_is_token_data_initialized(&tok[KEY_ENTRY_IEEE_OFFSET], EUI64_SIZE)) {
        return i;
      }
    } else {
      context.key_index = i;
      status = sli_zigbee_stack_sec_man_check_key_context(&context);

      if ((0 == memcmp(&tok[KEY_ENTRY_IEEE_OFFSET],
                       address,
                       EUI64_SIZE))
          && ((status == SL_STATUS_OK))
          && ((tokenBitmask & KEY_TABLE_TYPE_MASK) == KEY_TABLE_TYPE_LINK_KEY)
          && ((bitmask == 0xFF) || (bitmask == 0 && !(tokenBitmask & KEY_TABLE_SYMMETRIC_PASSPHRASE)) || (bitmask != 0 && (bitmask & tokenBitmask) == bitmask))  // match bitmask only if bitmask is valid
          && sli_zigbee_get_key_entry_network_index(tokenBitmask) == sli_zigbee_get_current_network_index()) { // search only for those keys with the correct nwk index
        return i;
      }
    }
  }
  return 0xFF;
}

// This function searches through the Key table to find an entry
// that has the same Address and key type as the passed value.
// If NULL is passed in for the address then it finds the first
// unused entry (type is ignored) and sets passed index.

uint8_t sli_zigbee_stack_find_key_table_entry(sl_802154_long_addr_t address, bool linkKey)
{
  if (!linkKey) {
    return SL_STATUS_INVALID_STATE;
  }

  sl_status_t status;

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;

  status = sli_zigbee_stack_sec_man_export_link_key_by_eui(address,
                                                           &context,
                                                           &plaintext_key,
                                                           &key_data);
  if (status != SL_STATUS_OK) {
    return 0xFF;
  }
  return context.key_index;
}

sl_status_t sli_zigbee_stack_erase_key_table_entry(uint8_t index)
{
  sl_status_t status;
  sl_zigbee_sec_man_context_t context;
  sli_zigbee_stack_sec_man_init_context(&context);
  context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
  context.key_index = index;
  context.flags = ZB_SEC_MAN_FLAG_KEY_INDEX_IS_VALID;
  status = sli_zigbee_stack_sec_man_delete_key(&context);

  return (status == SL_STATUS_OK) ? SL_STATUS_OK : SL_STATUS_INVALID_PARAMETER;
}

// If erase = true, only the 'index' parameter matters.  The rest may be 0.
sl_status_t sli_zigbee_af_set_key_table_entry(bool erase,
                                              uint8_t index,
                                              sl_zigbee_key_struct_t* keyStruct)
{
  tokTypeStackKeyTable tok;

  if (sl_zigbee_key_table_size <= index) {
    return SL_STATUS_INVALID_INDEX;
  }

  if ( erase ) {
    memset(&tok, 0, sizeof(tokTypeStackKeyTable));
  } else {
    sl_802154_long_addr_t tcAddress;
    uint8_t existingIndex = sli_zigbee_stack_find_key_table_entry(keyStruct->partnerEUI64, KEY_TABLE_TYPE_LINK_KEY);
    if ( sli_zigbee_stack_is_local_eui64(keyStruct->partnerEUI64)
         || (existingIndex != 0xFF && existingIndex != index) ) {
      sl_zigbee_key_struct_t duplicate;
      (void) sli_zigbee_get_key_table_entry(existingIndex, &duplicate);
      // NOTE however we make an exception if one (and only one) of them is an authentication token
      if (!((duplicate.bitmask ^ keyStruct->bitmask) & SL_ZIGBEE_KEY_IS_AUTHENTICATION_TOKEN)) {
        // Don't allow an application to setup a key in the table with
        // a duplicate address of another entry (which is not this one).
        return SL_STATUS_INVALID_PARAMETER;
      }
    }

    // NOTE non-tc devices will store their authentication token in the link key table
    if (!(keyStruct->bitmask & SL_ZIGBEE_KEY_IS_AUTHENTICATION_TOKEN)
        && sli_zigbee_get_trust_center_eui64(tcAddress)
        && (0 == memcmp(keyStruct->partnerEUI64, tcAddress, EUI64_SIZE))) {
      // Standard Security:
      // Don't allow an application to setup a link key in the key
      // table to the TC.  If we are the TC, this doesn't make any sense.
      // If we are not the TC, the key table is for app keys.  We can't
      // have an app key with the TC (per the spec).

      return SL_STATUS_INVALID_PARAMETER;
    }
    memmove(&(tok[KEY_ENTRY_IEEE_OFFSET]), keyStruct->partnerEUI64, EUI64_SIZE);

    tok[KEY_ENTRY_INFO_OFFSET] = (KEY_TABLE_TYPE_LINK_KEY
                                  | (sli_zigbee_are_keys_authorized_by_default()
                                     ? 0
                                     : KEY_TABLE_UNAUTHORIZED_KEY)
                                  | (sli_zigbee_get_current_network_index() // set the nwk index
                                     << KEY_TABLE_TYPE_NETWORK_INDEX_OFFSET)
                                  | (keyStruct->bitmask & SL_ZIGBEE_KEY_IS_AUTHENTICATION_TOKEN
                                     ? KEY_TABLE_SYMMETRIC_PASSPHRASE : 0));
  }
  sli_zigbee_incoming_aps_frame_counters[index] = 0;
  sli_zigbee_stack_set_key_table_entry_at_index(index, &tok);

  return SL_STATUS_OK;
}

sl_status_t sli_zigbee_update_key_state(uint8_t index,
                                        uint8_t setFlags,
                                        uint8_t clearFlags)
{
  if (sl_zigbee_key_table_size <= index) {
    return SL_STATUS_INVALID_INDEX;
  } else {
    tokTypeStackKeyTable tok;
    sli_zigbee_stack_fetch_key_table_entry_at_index(index, &tok);

    if (!sli_zigbee_is_token_data_initialized((uint8_t*)&tok, sizeof(tokTypeStackKeyTable))) {
      return SL_STATUS_INVALID_KEY;
    }
    tok[KEY_ENTRY_INFO_OFFSET] &= (uint8_t) (~clearFlags);
    tok[KEY_ENTRY_INFO_OFFSET] |= setFlags;

    sli_zigbee_stack_set_key_table_entry_at_index(index, &tok);
  }
  return SL_STATUS_OK;
}

void sli_zigbee_note_sleepy_device_in_key_table(sl_802154_long_addr_t eui64, bool sleepy)
{
  sl_zigbee_sec_man_context_t context;
  sli_zigbee_stack_sec_man_export_link_key_by_eui(eui64, &context, NULL, NULL);
  if (context.key_index != 0xFF) {
    sli_zigbee_update_key_note_end_device(context.key_index, sleepy);
  }
}

//------------------------------------------------------------------------------

bool sli_zigbee_process_application_link_key(sl_802154_long_addr_t partnerEui64,
                                             bool amInitiator,
                                             sl_zigbee_key_data_t* keyData)
{
  sl_status_t status;
  bool store_key = false;
  sl_802154_long_addr_t partner_link_key_partner;
  sli_zigbee_partner_link_key_get_device(partner_link_key_partner);

  (void) amInitiator;
  // In BDB 3.1/R23 partner link keys, keys must be verified before they can be used, hence we mark them as unconfirmed and place them in the transient key table.
  // In prior versions of the app/stack, partner link keys were valid as soon as they were received from the TC
  if (sli_zigbee_stack_get_stack_compliance_revision() >= R23_COMPLIANCE_REVISION) {
    // We only accept partner link keys if we are either an initiator who started a partner link key update
    // or a target who is free to service a partner link key update session
#if !defined(SL_ZIGBEE_GOLDEN_UNIT)
    // Golden Unit does not use the app link key state machine, it has CLIs to separately send the messages
    // (Get Authen Sec Level, Request Key, Verify Key Req)
    if ((sli_zigbee_get_update_app_link_key_state() == UPDATE_APP_LINK_KEY_STATE_REQUEST_KEY_INITIATOR)
        && (memcmp(partner_link_key_partner, partnerEui64, EUI64_SIZE) == 0)) {
      // Initiator: wait for Verify Key
      sli_zigbee_set_update_app_link_key_state(UPDATE_APP_LINK_KEY_STATE_AWAIT_VERIFY_KEY_INITIATOR);
      sli_zigbee_set_partner_key_update_timer(true, BDBC_TC_LINK_KEY_EXCHANGE_TIMEOUT_MS * 2);
      store_key = true;
    } else if (sli_zigbee_get_update_app_link_key_state() == UPDATE_APP_LINK_KEY_STATE_NONE) {
      // We are now a target. Send the Start Security Get Auth Level
      status = sli_zigbee_stack_get_authentication_level(SL_ZIGBEE_TRUST_CENTER_NODE_ID, partnerEui64);
      if (status == SL_STATUS_OK) {
        sli_zigbee_partner_link_key_set_device(partnerEui64);
        sli_zigbee_set_update_app_link_key_state(UPDATE_APP_LINK_KEY_STATE_SECURITY_LEVEL_TARGET);
        sli_zigbee_set_partner_key_update_timer(true, BDBC_TC_LINK_KEY_EXCHANGE_TIMEOUT_MS);
        store_key = true;
      } else {
        (void)sli_zigbee_stack_terminate_app_link_key_request();
        sli_zigbee_stack_zigbee_key_establishment_handler(partnerEui64, SL_ZIGBEE_FAILED_GET_AUTH_SECURITY);
      }
    }

    if (store_key) {
      sl_zigbee_sec_man_flags_t flags = ZB_SEC_MAN_FLAG_EUI_IS_VALID | ZB_SEC_MAN_FLAG_UNCONFIRMED_TRANSIENT_KEY;
      sl_status_t sec_status = zb_sec_man_import_transient_key(partnerEui64, (sl_zigbee_sec_man_key_t *) keyData, flags);
      if (sec_status != SL_STATUS_OK) {
        return false;
      }
    }
#else
    (void) status;
    (void) store_key;
#endif
  } else {
    sl_zigbee_sec_man_context_t context;
    sli_zigbee_stack_sec_man_init_context(&context);
    context.core_key_type = SL_ZB_SEC_MAN_KEY_TYPE_APP_LINK;
    context.key_index = 0xFF;
    memmove(context.eui64, partnerEui64, sizeof(sl_802154_long_addr_t));

    if (SL_STATUS_OK != sli_zigbee_stack_sec_man_import_key(&context, (sl_zigbee_sec_man_key_t*) keyData)) {
      sli_zigbee_call_zigbee_key_establishment_handler(partnerEui64, SL_ZIGBEE_KEY_TABLE_FULL);
      return false;
    }

    if (context.key_index == 0xFF) {
      return false;
    }
    // Assume all Application keys sent by the Trust Center are authorized.
    sli_zigbee_update_key_authorization(context.key_index,
                                        true); // key is authorized

    sli_zigbee_request_key_timeout_control(false, false);
  }
  sli_zigbee_call_zigbee_key_establishment_handler(partnerEui64, SL_ZIGBEE_APP_LINK_KEY_ESTABLISHED);

  return true;
}

//------------------------------------------------------------------------------

// Clear all the entries belonging to the current network.
sl_status_t sli_zigbee_stack_clear_key_table(void)
{
  uint8_t i;
  for ( i = 0; i < sl_zigbee_key_table_size; i++) {
#if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
    tokTypeStackKeyTable tok;
    //Set token to some invalid initial value
    memset(&tok, 0xFF, sizeof (tok));

    uint8_t tokenBitmask;
    sli_zigbee_stack_fetch_key_table_entry_at_index(i, &tok);

    tokenBitmask = tok[KEY_ENTRY_INFO_OFFSET];

    //Check if token read was unsuccessful
    if ( tokenBitmask == 0xFF ) {
      assert(false);
    }

    if (sli_zigbee_get_key_entry_network_index(tokenBitmask) == sli_zigbee_get_current_network_index()) {
#endif // !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
    sli_zigbee_stack_erase_key_table_entry(i);

    // closing bracket for MISRA Compliance
    #if !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
  }
    #endif // !defined(SL_ZIGBEE_MULTI_NETWORK_STRIPPED)
  }

  return SL_STATUS_OK;
}
