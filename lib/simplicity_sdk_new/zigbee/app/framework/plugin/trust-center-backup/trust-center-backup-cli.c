/***************************************************************************//**
 * @file
 * @brief User interface for backing up and restoring a trust center.
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
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/framework/plugin/trust-center-backup/trust-center-backup.h"

#include "app/framework/util/af-main.h"

#if defined(SL_ZIGBEE_TEST)
  #include "app/framework/test/test-framework-security.h"
#endif

#if defined(SL_ZIGBEE_SCRIPTED_TEST)
  #define SL_ZIGBEE_LINK_KEY_TABLE_SIZE                               TEST_FRAMEWORK_KEY_TABLE_SIZE
#endif

#if SL_ZIGBEE_LINK_KEY_TABLE_SIZE > SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_MAX_CLI_BACKUP_SIZE
  #error SL_ZIGBEE_LINK_KEY_TABLE_SIZE > SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_MAX_CLI_BACKUP_SIZE
#endif

// A shorter more manageable name.
#define MAX_CLI_SIZE SL_ZIGBEE_AF_PLUGIN_TRUST_CENTER_BACKUP_MAX_CLI_BACKUP_SIZE

#ifdef SL_CATALOG_ZIGBEE_CORE_CLI_PRESENT
//avoid compiler warnings for unit tests not using these
static sl_zigbee_af_link_key_backup_data_t importKeyList[MAX_CLI_SIZE];
static sl_zigbee_af_trust_center_backup_data_t importData = {
  { 0, 0, 0, 0, 0, 0, 0, 0 }, // extended PAN ID (filled in later)
  0,                          // key list length (will be filled in later)
  MAX_CLI_SIZE,
  importKeyList
};
#endif

// *****************************************************************************
// Functions

static UNUSED void printBackup(const sl_zigbee_af_trust_center_backup_data_t* backup)
{
  uint8_t i = 0;

  sl_zigbee_af_security_print("Extended PAN ID: ");
  sl_zigbee_af_print_ieee_line(backup->extendedPanId);

  sl_zigbee_af_security_println("Index  EUI64                Hashed Key");
  //123456 (>)0123456789ABCDEF

  for (i = 0; i < backup->keyListLength; i++) {
    sl_zigbee_af_security_flush();
    sl_zigbee_af_security_print("%d      ", i);
    sl_zigbee_af_print_big_endian_eui64(backup->keyList[i].deviceId);
    sl_zigbee_af_security_print("  ");
    sl_zigbee_af_print_zigbee_key(sl_zigbee_key_contents(&(backup->keyList[i].key)));
  }
  sl_zigbee_af_security_flush();
  sl_zigbee_af_security_println("\n%d keys in backup", i);
}

#ifdef SL_CLI_TYPES_H
void printExportDataCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_zigbee_af_link_key_backup_data_t exportKeyList[MAX_CLI_SIZE];
  sl_zigbee_af_trust_center_backup_data_t export;
  sl_status_t status;

  export.maxKeyListLength = MAX_CLI_SIZE;
  export.keyList = exportKeyList;
  status = sl_zigbee_trust_center_export_backup_data(&export);

  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("%s: Failed to get TC backup data.", "Error");
    return;
  }

  printBackup(&export);
}

void printImportDataCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  printBackup(&importData);
}

void importClearCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  memset(&importData, 0, sizeof(sl_zigbee_af_trust_center_backup_data_t));
  memset(importKeyList,
         0,
         sizeof(sl_zigbee_af_link_key_backup_data_t) * MAX_CLI_SIZE);
  importData.keyList = importKeyList;
  importData.maxKeyListLength = MAX_CLI_SIZE;
}

void importKeyCommand(sl_cli_command_arg_t *arguments)
{
  sl_802154_long_addr_t partnerEUI64;
  sl_zigbee_key_data_t newKey;
  uint8_t index = (uint8_t)sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_copy_eui64_arg(arguments, 1, partnerEUI64, true);
  sl_zigbee_copy_key_arg((arguments), 2, &newKey);

  if (0 == memcmp(sl_zigbee_af_null_eui64, partnerEUI64, EUI64_SIZE)) {
    sl_zigbee_af_security_println("%s: EUI64 cannot be all zeroes.",
                                  "Error");
    return;
  }

  if (index > MAX_CLI_SIZE) {
    sl_zigbee_af_security_println("%s: index %d is greater than max import limit of %d.",
                                  "Error",
                                  index,
                                  MAX_CLI_SIZE);
    return;
  }

  memmove(importKeyList[index].deviceId, partnerEUI64, EUI64_SIZE);
  memmove(sl_zigbee_key_contents(&(importKeyList[index].key)),
          sl_zigbee_key_contents(&newKey),
          SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
  if (importData.keyListLength <= index) {
    importData.keyListLength = index + 1;
  }
}

void setExtendedPanIdCommand(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_copy_eui64_arg(arguments, 0, importData.extendedPanId, true);
}

void restoreFromBackupCommand(sl_cli_command_arg_t *arguments)
{
  (void) arguments;
  sl_status_t status;

  if (0 == memcmp(importData.extendedPanId,
                  sl_zigbee_af_null_eui64,
                  EUI64_SIZE)) {
    sl_zigbee_af_security_println("%s: Import contains NULL extended PAN ID",
                                  "Error");
    return;
  }

  status = sl_zigbee_trust_center_import_backup_and_start_network(&importData);
  if (status != SL_STATUS_OK) {
    sl_zigbee_af_security_println("%s: Failed to import backup data and form network.",
                                  "Error");
    return;
  }
  sl_zigbee_af_security_println("Import successful.");
}
#endif //SL_CLI_TYPES_H
