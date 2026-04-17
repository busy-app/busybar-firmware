/***************************************************************************//**
 * @file
 * @brief Bootload specific commands
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
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/framework/plugin/ota-common/ota.h"

#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#define DEFINETYPES
#endif
#include "stack/config/sl_zigbee_token_defines.h"
#include "sl_token_manager_api.h"

#if !defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST)
#include "api/btl_interface.h"
#endif

void printBootloaderInfoCommand(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
#if !defined(EZSP_HOST) && !defined(SL_ZIGBEE_TEST)
  BootloaderInformation_t info = { .type = SL_BOOTLOADER, .version = 0U, .capabilities = 0U };
  bootloader_getInfo(&info);
  tokTypeMfgSecureBootloaderKey keyData;
  otaPrintln("Installed Type (Base):  0x%02X", info.type);
  otaPrintln("Capabilities:           0x%04X", info.capabilities);
  otaPrintln("Bootloader Version:     0x%04X", info.version);

#if defined(SL_ZIGBEE_TEST)
  memset(keyData, 0xFF, SL_ZIGBEE_ENCRYPTION_KEY_SIZE);
#else
  (void)sl_token_manager_get_data(SL_TOKEN_GET_STATIC_SECURE_TOKEN(TOKEN_MFG_SECURE_BOOTLOADER_KEY), (void *)&keyData, sizeof(tokTypeMfgSecureBootloaderKey));
#endif

  otaPrint("Secure Bootloader Key:      ");
  sl_zigbee_af_print_zigbee_key((uint8_t const *)&keyData);
  otaPrintln("");

#else
  otaPrintln("Unsupported on EZSP Host");
#endif
}
