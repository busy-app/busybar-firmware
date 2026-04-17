/***************************************************************************//**
 * @file
 * @brief Zigbee Over-the-air bootload cluster for upgrading firmware and
 * downloading specific file.
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

#include "app/framework/util/common.h"

#include "app/util/serial/sl_zigbee_command_interpreter.h"

#include "ota-storage-common-config.h"

#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-common/ota-cli.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif  // SL_COMPONENT_CATALOG_PRESENT

// client and server use same storage interface
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#define otaPrintln(...) sl_zigbee_af_ota_bootload_cluster_println(__VA_ARGS__)

//------------------------------------------------------------------------------
// Functions

void sli_zigbee_af_ota_print_all_images(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
  uint8_t i = 0;
  sl_zigbee_af_ota_image_id_t id = sl_zigbee_af_ota_storage_iterator_first_cb();
  while (sl_zigbee_af_is_ota_image_id_valid(&id)) {
    sl_zigbee_af_ota_header_t header;
    sl_zigbee_af_tag_data_t tagInfo[SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE];
    uint16_t totalTags;
    otaPrintln("Image %d", i);
    if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
        != sl_zigbee_af_ota_storage_get_full_header_cb(&id,
                                                       &header)) {
      otaPrintln("  ERROR: Could not get full header!");
    } else {
      otaPrintln("  Header Version: 0x%04X", header.headerVersion);
      otaPrintln("  Header Length:  %d bytes", header.headerLength);
      otaPrintln("  Field Control:  0x%04X", header.fieldControl);
      sl_zigbee_af_ota_bootload_cluster_flush();
      otaPrintln("  Manuf ID:       0x%04X", header.manufacturerId);
      otaPrintln("  Image Type:     0x%04X", header.imageTypeId);
      otaPrintln("  Version:        0x%08X", header.firmwareVersion);
      sl_zigbee_af_ota_bootload_cluster_flush();
      otaPrintln("  Zigbee Version: 0x%04X", header.zigbeeStackVersion);
      otaPrintln("  Header String:  %s", header.headerString);
      otaPrintln("  Image Size:     %ld bytes", header.imageSize);
      sl_zigbee_af_ota_bootload_cluster_flush();
      if (headerHasSecurityCredentials(&header)) {
        otaPrintln("  Security Cred:  0x%02X", header.securityCredentials);
      }
      if (headerHasUpgradeFileDest(&header)) {
        sl_zigbee_af_ota_bootload_cluster_print("  Upgrade Dest:   ");
        if (header.headerVersion == OTA_HEADER_VERSION_ZIGBEE) {
          sl_zigbee_af_ota_bootload_cluster_debug_exec(sl_zigbee_af_print_big_endian_eui64(header.upgradeFileDestination.EUI64));
        } else {
          sl_zigbee_af_ota_bootload_cluster_print_buffer((uint8_t *)&header.upgradeFileDestination, sizeof(header.upgradeFileDestination), true);
        }
        sl_zigbee_af_ota_bootload_cluster_flush();
        otaPrintln("");
      }
      if (headerHasHardwareVersions(&header)) {
        otaPrintln("  Min. HW Ver:    0x%04X", header.minimumHardwareVersion);
        otaPrintln("  Max. HW Ver:    0x%04X", header.maximumHardwareVersion);
        sl_zigbee_af_ota_bootload_cluster_flush();
      }
      if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS == sli_zigbee_af_ota_storage_read_all_tag_info(&id,
                                                                                          tagInfo,
                                                                                          SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE,
                                                                                          &totalTags)) {
        uint16_t i;
        otaPrintln("  Total Tags: %d", totalTags);
        for (i = 0; i < SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE && i < totalTags; i++) {
          otaPrintln("    Tag: 0x%04X", tagInfo[i].id);
          otaPrintln("      Length: %ld", tagInfo[i].length);
        }
        sl_zigbee_af_ota_bootload_cluster_flush();
      } else {
        otaPrintln("Error: Could not obtain tag info from image.");
      }
    }
    id = sl_zigbee_af_ota_storage_iterator_next_cb();
    i++;
  }

  otaPrintln("\n%d images in OTA storage.", sl_zigbee_af_ota_storage_get_count_cb());
}

sl_zigbee_af_ota_image_id_t sli_zigbee_af_ota_find_image_id_by_index(uint8_t index)
{
  uint8_t i = 0;
  sl_zigbee_af_ota_image_id_t id = sl_zigbee_af_ota_storage_iterator_first_cb();
  do {
    if (i == index) {
      return id;
    }
    i++;
    id = sl_zigbee_af_ota_storage_iterator_next_cb();
  } while (sl_zigbee_af_is_ota_image_id_valid(&id));

  return sl_zigbee_af_invalid_image_id;
}

void sli_zigbee_af_ota_image_delete(sl_cli_command_arg_t *arguments)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_af_ota_image_id_t id = sli_zigbee_af_ota_find_image_id_by_index(index);
  sl_zigbee_af_ota_storage_status_t status;

  status = sl_zigbee_af_ota_storage_delete_image_cb(&id);
  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS == status
      || SL_ZIGBEE_AF_OTA_STORAGE_OPERATION_IN_PROGRESS == status) {
    otaPrintln("Image delete%s.",
               (SL_ZIGBEE_AF_OTA_STORAGE_OPERATION_IN_PROGRESS == status
                ? " in progress"
                : "d"));
    return;
  }
  otaPrintln("Error: Failed to delete image.");
}

void sli_zigbee_af_ota_reload_storage_device(sl_cli_command_arg_t *arguments)
{
  UNUSED_VAR(arguments);
#if defined (SL_CATALOG_ZIGBEE_OTA_STORAGE_POSIX_FILESYSTEM_PRESENT)
  sli_zigbee_af_ota_storage_close();
#endif  // SL_CATALOG_ZIGBEE_OTA_STORAGE_POSIX_FILESYSTEM_PRESENT

  sl_zigbee_af_ota_storage_init_cb();
}

#define PRINT_BLOCK_LENGTH 64

void sli_zigbee_af_ota_storage_data_print(sl_cli_command_arg_t *arguments)
{
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t offset = sl_cli_get_argument_uint32(arguments, 1);
  sl_zigbee_af_ota_image_id_t id = sli_zigbee_af_ota_find_image_id_by_index(index);
  uint32_t actualLength;
  uint8_t data[PRINT_BLOCK_LENGTH];
  uint8_t extraLength;

  sl_zigbee_af_ota_storage_status_t status = sl_zigbee_af_ota_storage_read_image_data_cb(&id,
                                                                                         offset,
                                                                                         PRINT_BLOCK_LENGTH,
                                                                                         data,
                                                                                         &actualLength);
  if (status) {
    otaPrintln("Error:  Could not read block: 0x%02X.\n", status);
    return;
  }

  extraLength = actualLength % 8;
  sl_zigbee_af_print8_byte_blocks(actualLength / 8,
                                  data,
                                  true); // CR between blocks?
  sl_zigbee_af_print_buffer(sl_zigbee_af_print_active_area,
                            &(data[actualLength - extraLength]),
                            extraLength,
                            true); // with whitespace?
  sl_zigbee_af_print_text_line("");
}

void sli_zigbee_af_ota_storage_info_print_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sli_zigbee_af_ota_storage_info_print();
}
