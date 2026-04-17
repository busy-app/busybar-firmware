/***************************************************************************//**
 * @file
 * @brief Code for backing up and restoring a trust center.
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
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "app/framework/util/af-main.h"
#include "app/framework/security/af-security.h"
#include "stack/include/zigbee-security-manager.h"

// *****************************************************************************
// Globals

// *****************************************************************************
// Functions

sl_status_t sl_zigbee_trust_center_export_backup_data(sl_zigbee_af_trust_center_backup_data_t* backup)
{
  sl_zigbee_node_type_t nodeType;
  sl_zigbee_network_parameters_t params;
  uint8_t i;
  uint8_t keyTableSize = sl_zigbee_af_get_key_table_size();
  backup->keyListLength = 0;

  if (backup->maxKeyListLength < keyTableSize) {
    return SL_STATUS_FULL;
  }

  if (sl_zigbee_af_get_node_id() != SL_ZIGBEE_TRUST_CENTER_NODE_ID) {
    return SL_STATUS_INVALID_STATE;
  }

  sl_zigbee_af_get_network_parameters(&nodeType,
                                      &params);
  memmove(backup->extendedPanId,
          params.extendedPanId,
          EUI64_SIZE);

  sl_zigbee_sec_man_context_t context;
  sl_zigbee_sec_man_key_t plaintext_key;
  sl_zigbee_sec_man_aps_key_metadata_t key_data;

  for (i = 0; i < keyTableSize; i++) {
    context.key_index = i;
    sl_status_t status = sl_zigbee_sec_man_export_link_key_by_index(i, &context, &plaintext_key, &key_data);
    if (status == SL_STATUS_OK) {
      memmove(backup->keyList[backup->keyListLength].deviceId,
              context.eui64,
              EUI64_SIZE);
      // Rather than give the real link key, the backup
      // contains a hashed version of the key.  This is done
      // to prevent a compromise of the backup data from compromising
      // the current link keys.  This is per the Smart Energy spec.
      sl_zigbee_aes_hash_simple(SL_ZIGBEE_ENCRYPTION_KEY_SIZE,
                                (uint8_t*) &(plaintext_key.key),
                                sl_zigbee_key_contents(&(backup->keyList[backup->keyListLength].key)));
      backup->keyListLength++;
    }
  }
  return SL_STATUS_OK;
}

sl_status_t sl_zigbee_trust_center_import_backup_and_start_network(const sl_zigbee_af_trust_center_backup_data_t* backup)
{
  // 1. Check that the network is down.
  // 2. Add keys
  // 3. Form the network.
  // 4. Profit!

  uint8_t i;
  uint8_t keyTableSize = sl_zigbee_af_get_key_table_size();
  sl_status_t status;

  if (SL_ZIGBEE_NO_NETWORK != sl_zigbee_af_network_state()) {
    sl_zigbee_af_security_println("%s: Cannot import TC data while network is up.");
    return SL_STATUS_INVALID_STATE;
  }

  if (backup->keyListLength > keyTableSize) {
    sl_zigbee_af_security_println("%s: Current key table of %d too small for import of backup (%d)!",
                                  "Error",
                                  keyTableSize,
                                  backup->keyListLength);
    return SL_STATUS_FAIL;
  }

  for (i = 0; i < keyTableSize; i++) {
    if (i >= backup->keyListLength) {
      status = sl_zigbee_erase_key_table_entry(i);
    } else {
      // Copy key data to a local to get around compiler warning about
      // passing "const" data into 'sl_zigbee_sec_man_import_link_key()'
      sl_zigbee_key_data_t key;
      memmove(sl_zigbee_key_contents(&key),
              sl_zigbee_key_contents(&(backup->keyList[i].key)),
              SL_ZIGBEE_ENCRYPTION_KEY_SIZE);

      sl_status_t import_status = sl_zigbee_sec_man_import_link_key(i,
                                                                    backup->keyList[i].deviceId,
                                                                    (sl_zigbee_sec_man_key_t*)&key);
      status = ((import_status == SL_STATUS_OK) ? SL_STATUS_OK : SL_STATUS_INVALID_PARAMETER);
    }

    if (status != SL_STATUS_OK) {
      sl_zigbee_af_security_println("%s: Failed to %s key table entry at index %d: 0%02X",
                                    "Error",
                                    ((i >= backup->keyListLength)
                                     ? "erase"
                                     : "set"),
                                    i,
                                    status);
      sl_zigbee_af_security_println("TC Import failed");
      return status;
    }
  }

  sl_zigbee_af_security_println("Imported %d keys",
                                backup->keyListLength);

  // Disable clearing the link key table.
  sl_zigbee_af_clear_link_key_table_upon_forming_or_joining = false;

  // This EUI64 is used by the Network-find plugin when forming a network.
  sl_zigbee_af_set_form_and_join_extended_pan_id_cb(backup->extendedPanId);
  sl_zigbee_af_security_println("Starting search for unused short PAN ID");
  status = sl_zigbee_af_find_unused_pan_id_and_form_cb();
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("Failed to start PAN ID search.");
  }
  return status;
}
