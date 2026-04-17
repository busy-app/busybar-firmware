/***************************************************************************//**
 * @brief Various meta information about the stack
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

#include <stdlib.h>
#include "ember.h"
#include "config/config.h"
#include "stack/core/sli-connect-api.h"
#include "sl_platform_version.h"

#if BOOTLOADER_ENABLE
#include "btl-interface.h"
#endif

EmberStatus emApiGetVersionInfo(uint16_t *platform_version, uint16_t *connect_stack_version, uint32_t *bootloader_version)
{
  if (platform_version && connect_stack_version && bootloader_version) {
#if BOOTLOADER_ENABLE
    BootloaderInformation_t *info;
    bootloader_getInfo(info);
    *bootloader_version = info->version;
#else
    *bootloader_version = 0;
#endif
    *platform_version = (uint16_t)SL_PLATFORM_VERSION;
    *connect_stack_version = (uint16_t)EMBER_FULL_VERSION;
    return EMBER_SUCCESS;
  }
  return EMBER_BAD_ARGUMENT;
}
