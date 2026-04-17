/***************************************************************************//**
 * @file
 * @brief This file handle how the application can configure and interact with the OTA
 * cluster.
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
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#ifdef HAL_CONFIG
#include "hal-config.h"
#include "ember-hal-config.h"
#endif
#include "ota-client-policy.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#if (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_POLICY_GBL_VERIFICATION == 1)
#define GBL_VERIFICATION
#endif
#if (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_POLICY_INCLUDE_HARDWARE_VERSION == 1)
#define INCLUDE_HARDWARE_VERSION
#endif
#if (SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_POLICY_DELETE_FAILED_DOWNLOADS == 1)
#define DELETE_FAILED_DOWNLOADS
#endif

#ifdef SL_CATALOG_SLOT_MANAGER_PRESENT
 #include "slot-manager.h"
 #define slotManagerBeginImageValidation(...) \
  (void)sl_util_af_slot_manager_image_is_valid_reset(__VA_ARGS__)
 #define slotManagerBootSlot(...) (void)sl_util_af_slot_manager_boot_slot(__VA_ARGS__)
#else
 #define slotManagerBeginImageValidation(...)
 #define slotManagerBootSlot(...)
#endif // SL_CATALOG_SLOT_MANAGER_PRESENT

// Right now this sample policy only supports a single set of version
// information for the device, which will be supplied to the OTA cluster in
// order to query and download a new image when it is available.  This does not
// support multiple products with multiple download images.

#if defined(INCLUDE_HARDWARE_VERSION)
  #define HARDWARE_VERSION SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_POLICY_HARDWARE_VERSION
#else
  #define HARDWARE_VERSION SL_ZIGBEE_AF_INVALID_HARDWARE_VERSION
#endif

//------------------------------------------------------------------------------
// Globals
sl_zigbee_af_image_verify_status_t slotManagerContinueImageValidation();

//------------------------------------------------------------------------------

void sl_zigbee_af_ota_client_version_info_cb(sl_zigbee_af_ota_image_id_t* currentImageInfo,
                                             uint16_t* hardwareVersion)
{
  // This callback is fired when a new query and download is initiated.
  // The application will fill in the currentImageInfo with their manufacturer
  // ID, image type ID, and current software version number to use in that
  // query. The deviceSpecificFileEui64 can be ignored.

  // It may be necessary to dynamically determine this data by talking to
  // another device, as is the case with a host talking to an NCP device.

  // The OTA client plugin will cache the data returned by this callback
  // and use it for the subsequent transaction, which could be a query
  // or a query and download.  Therefore it is possible to instruct the
  // OTA client cluster code to query about multiple images by returning
  // different values.

  memset(currentImageInfo, 0, sizeof(sl_zigbee_af_ota_image_id_t));
  currentImageInfo->manufacturerId  = SL_ZIGBEE_ZCL_MANUFACTURER_CODE;
  currentImageInfo->imageTypeId     = SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_POLICY_IMAGE_TYPE_ID;
  currentImageInfo->firmwareVersion = SL_ZIGBEE_AF_PLUGIN_OTA_CLIENT_POLICY_FIRMWARE_VERSION;

  if (hardwareVersion != NULL) {
    *hardwareVersion = HARDWARE_VERSION;
  }
}

sl_zigbee_af_image_verify_status_t sl_zigbee_af_ota_client_custom_verify_cb(bool newVerification,
                                                                            const sl_zigbee_af_ota_image_id_t* id)
{
  // Manufacturing specific checks can be made to the image in this function to
  // determine if it is valid.  This function is called AFTER cryptographic
  // checks have passed.  If the cryptographic checks failed, this function will
  // never be called.

  // The function shall return one of the following based on its own
  // verification process.
  // 1) SL_ZIGBEE_AF_IMAGE_GOOD - the image has passed all checks
  // 2) SL_ZIGBEE_AF_IMAGE_BAD  - the image is not valid
  // 3) SL_ZIGBEE_AF_IMAGE_VERIFY_IN_PROGRESS - the image is valid so far, but more
  //      checks are needed.  This callback shall be re-executed later to
  //      continue verification.  This allows other code in the framework to run.

  // Note that GBL verification is an SoC-only feature.

  UNUSED_VAR(id);
#if !defined(EZSP_HOST) && defined(GBL_VERIFICATION)
  sl_zigbee_af_image_verify_status_t status;
  uint32_t slot;

  // If we're using slots, we'll need to call a different set of APIs
  slot = sli_zigbee_af_ota_storage_get_slot();

  // For sleepies, we must re-initalize the EEPROM / bootloader
  // after each nap/hibernate.  This call will only re-initalize the EEPROM
  // if that is the case.
  sl_zigbee_af_eeprom_init_cb();

  if (newVerification) {
    otaPrintln("Starting GBL verification");
    if (INVALID_SLOT != slot) {
      slotManagerBeginImageValidation(slot);
    } else {
      slotManagerBeginImageValidation(DEFAULT_SLOT);
    }
  }

  status = slotManagerContinueImageValidation();
  if (status == SL_ZIGBEE_AF_IMAGE_VERIFY_IN_PROGRESS) {
    return status;
  } else if (status == SL_ZIGBEE_AF_IMAGE_BAD) {
    otaPrintln("GBL failed verification.");
    return status;
  }
  otaPrintln("GBL passed verification.");
#else
  UNUSED_VAR(newVerification);
#endif

  return SL_ZIGBEE_AF_IMAGE_GOOD;
}

bool sl_zigbee_af_ota_client_download_complete_cb(sl_zigbee_af_ota_download_result_t result,
                                                  const sl_zigbee_af_ota_image_id_t* id)
{
  // At this point the image has been completely downloaded and cryptographic
  // checks (if applicable) have been performed.
  // Manufacturer verification callback has also been called and passed.
  UNUSED_VAR(id);

  if (result != SL_ZIGBEE_AF_OTA_DOWNLOAD_AND_VERIFY_SUCCESS) {
    sl_zigbee_af_ota_bootload_cluster_println("Download failed.");

#if defined(DELETE_FAILED_DOWNLOADS)
    if (result != SL_ZIGBEE_AF_OTA_ERASE_FAILED) {
      sl_zigbee_af_ota_storage_status_t status = sl_zigbee_af_ota_storage_clear_temp_data_cb();
      sl_zigbee_af_ota_bootload_cluster_println("Deleting failed download, status: 0x%02X",
                                                status);
    }
#endif

    // Returning true to indicate that, even in case of a bad result,
    // the OTA state machine will "go ahead" handling the process.
    return true;
  }

  // If the client wants to abort for some reason then it can do so here
  // and return false.  Otherwise it should give the "go ahead" by returning
  // true.

  return true;
}

void sl_zigbee_af_ota_client_bootload_cb(const sl_zigbee_af_ota_image_id_t* id)
{
  // OTA Server has told us to bootload.
  // Any final preperation prior to the bootload should be done here.
  // It is assumed that the device will reset in most all cases.

  uint32_t offset;
  uint32_t endOffset;
  uint32_t slot;

  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
      != sli_zigbee_af_ota_storage_get_tag_offset_and_size(id,
                                                           OTA_TAG_UPGRADE_IMAGE,
                                                           &offset,
                                                           &endOffset)) {
    sl_zigbee_af_core_flush();
    otaPrintln("Image does not contain an Upgrade Image Tag (0x%04X). Skipping "
               "upgrade.", OTA_TAG_UPGRADE_IMAGE);
#if defined(DELETE_FAILED_DOWNLOADS)
    sl_zigbee_af_ota_storage_status_t status = sl_zigbee_af_ota_storage_clear_temp_data_cb();
    sl_zigbee_af_ota_bootload_cluster_println("Deleting failed download, status: 0x%02X",
                                              status);
#endif // DELETE_FAILED_DOWNLOADS
    return;
  }

  otaPrintln("Executing bootload callback.");
  (void) sli_legacy_serial_wait_send(APP_SERIAL);

  // If we're using slots, we'll need to use a different set of APIs
  slot = sli_zigbee_af_ota_storage_get_slot();

  // These routines will NOT return unless we failed to launch the bootloader.
  if (INVALID_SLOT != slot) {
    slotManagerBootSlot(slot);
  } else {
    sl_zigbee_af_ota_bootload_cb(id, OTA_TAG_UPGRADE_IMAGE);
  }
}

sl_zigbee_af_image_verify_status_t slotManagerContinueImageValidation()
{
#ifdef SL_CATALOG_SLOT_MANAGER_PRESENT
  sl_status_t slotManagerResult = sl_util_af_slot_manager_image_is_valid();

  if (slotManagerResult == SL_STATUS_IN_PROGRESS) {
    return SL_ZIGBEE_AF_IMAGE_VERIFY_IN_PROGRESS;
  } else if (slotManagerResult != SL_STATUS_OK) {
    return SL_ZIGBEE_AF_IMAGE_BAD;
  } else {
    return SL_ZIGBEE_AF_IMAGE_GOOD;
  }
#else
  return SL_ZIGBEE_AF_IMAGE_BAD;
#endif // SL_CATALOG_SLOT_MANAGER_PRESENT
}
