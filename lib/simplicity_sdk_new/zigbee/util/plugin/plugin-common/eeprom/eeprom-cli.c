/**************************************************************************//**
 * Copyright 2017 Silicon Laboratories, Inc.
 *
 *****************************************************************************/

#include PLATFORM_HEADER
#include "hal.h"
#include "sl_cli.h"
#include "zcl-debug-print.h"
#include "eeprom-config.h"

#include "eeprom.h"

//------------------------------------------------------------------------------
// Macros

#define DATA_BLOCK 64

//------------------------------------------------------------------------------

void sli_eeprom_data_print_command(sl_cli_command_arg_t *arguments)
{
  uint32_t offset = sl_cli_get_argument_uint32(arguments, 0);
  uint8_t data[DATA_BLOCK];
  uint8_t status = sl_zigbee_af_eeprom_read(offset, data, DATA_BLOCK);

  if (status) {
    sl_zigbee_af_core_println("Error:  Failed to read from EEPROM at 0x%4X, status: %d",
                              offset,
                              status);
    return;
  }

  // Divide by 8 to get the number of 8-byte blocks to print.
  sl_zigbee_af_print8_byte_blocks(DATA_BLOCK >> 3, // number of blocks
                                  data,
                                  true); // CR between blocks?
}

void sli_eeprom_status_command(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);

  uint8_t i;
  sl_zigbee_af_core_println("EEPROM Initialized: %c",
                            (sli_eeprom_is_eeprom_initialized()
                             ? 'y'
                             : 'n'));
  // NOTE:  Calling sl_util_af_eeprom_busy() will actually initialize the
  // EEPROM, so we avoid triggering a side-effect during this 'status' command.
  sl_zigbee_af_core_println("EEPROM Busy: %c",
                            (!sli_eeprom_is_eeprom_initialized()
                             ? '?'
                             : (sl_util_af_eeprom_busy()
                                ? 'y'
                                : 'n')));
  sl_zigbee_af_core_println("Partial Word Storage Count: %d",
                            EMBER_AF_PLUGIN_EEPROM_PARTIAL_WORD_STORAGE_COUNT);

  for (i = 0; i < EMBER_AF_PLUGIN_EEPROM_PARTIAL_WORD_STORAGE_COUNT; i++) {
    sl_zigbee_af_core_println("Address: 0x%4X, Partial Word: 0x%X",
                              sli_eeprom_saved_partial_writes[i].address,
                              sli_eeprom_saved_partial_writes[i].data);
  }
}

void sli_eeprom_info()
{
  const HalEepromInformationType* part = sl_util_af_eeprom_info();
  sl_zigbee_af_core_println("\nEEPROM Info");
  if (part == NULL) {
    sl_zigbee_af_core_println("Not available (older bootloader)");
  } else {
    uint8_t wordSize = sl_util_af_eeprom_get_word_size();
    bool confirmedWordSize = false;
    if (part->version >= EEPROM_INFO_MIN_VERSION_WITH_WORD_SIZE_SUPPORT) {
      confirmedWordSize = true;
    }
    sl_zigbee_af_core_println("Part Description:          %s", ((part->partDescription != NULL) ? part->partDescription : "NA"));
    sl_zigbee_af_core_println("Capabilities:              0x%2X", part->capabilitiesMask);
    sl_zigbee_af_core_println("Page Erase time (s):      %d", part->pageEraseMs);
    if ((part->capabilitiesMask & EEPROM_CAPABILITIES_PART_ERASE_SECONDS)
        == EEPROM_CAPABILITIES_PART_ERASE_SECONDS) {
      sl_zigbee_af_core_println("Part Erase time (s):      %d", part->partEraseTime);
    } else {
      sl_zigbee_af_core_println("Part Erase time (ms):      %d", part->partEraseTime);
    }
    sl_zigbee_af_core_println("Page size (bytes):         %l", part->pageSize);
    sl_zigbee_af_core_println("Part size (bytes):         %l", part->partSize);
    sl_zigbee_af_core_println("Word size (bytes):         %d (%p)",
                              wordSize,
                              (confirmedWordSize
                               ? "confirmed"
                               : "assumed"));
  }
}

void sli_eeprom_info_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sli_eeprom_info();
}
