/***************************************************************************//**
 * @file
 * @brief Routines for bootloading an SOC.
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
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#ifndef SL_ZIGBEE_TEST
#include "api/btl_interface.h"
#include "api/btl_interface_storage.h"
#endif // !SL_ZIGBEE_TEST
//------------------------------------------------------------------------------
// Globals

//------------------------------------------------------------------------------
sl_status_t bootloaderInstallNewImage(void)
{
#ifndef SL_ZIGBEE_TEST
  int32_t storageSlot = DEFAULT_SLOT;
  if (bootloader_setImagesToBootload(&storageSlot, 1) == BOOTLOADER_OK) {
    // should not return
    bootloader_rebootAndInstall();
  }
#endif // SL_CATALOG_SLOT_MANAGER_PRESENT
  return SL_STATUS_FAIL;
}

uint8_t sl_zigbee_af_ota_bootload_cb(const sl_zigbee_af_ota_image_id_t* id,
                                     uint16_t ncpUpgradeTagId)
{
  UNUSED_VAR(id);
  UNUSED_VAR(ncpUpgradeTagId);

  bootloaderInstallNewImage();
  sl_zigbee_af_core_println("Bootloading failed!");

  // If we got here, it is a failure.
  return 1;
}
