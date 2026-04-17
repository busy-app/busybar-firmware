/***************************************************************************//**
 * @file
 * @brief Simulation for the OTA Storage POSIX Filesystem plugin.
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
#include "ota-storage-linux-config.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// Globals

#define INVALID_MANUFACTURER_ID  0xFFFF
#define INVALID_DEVICE_ID        0xFFFF
#define INVALID_FIRMWARE_VERSION 0xFFFFFFFFL
#define INVALID_EUI64 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#define INVALID_SLOT (uint32_t)-1

#define INVALID_OTA_IMAGE_ID  \
  { INVALID_MANUFACTURER_ID,  \
    INVALID_DEVICE_ID,        \
    INVALID_FIRMWARE_VERSION, \
    INVALID_EUI64,            \
  }

sl_zigbee_af_ota_image_id_t sl_zigbee_invalid_image_id = INVALID_OTA_IMAGE_ID;
// Dummy variable only to fix build error for simulation host target.
char defaultStorageDirectory[OTA_FILE_STORAGE_DIR_LENGTH] = "ota-files";
//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------

void sli_zigbee_af_ota_storage_close(void)
{
}

void sli_zigbee_af_ota_storage_info_print(void)
{
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_check_temp_data_cb(uint32_t* returnOffset,
                                                                              uint32_t* returnTotalSize,
                                                                              sl_zigbee_af_ota_image_id_t* returnOtaImageId)
{
  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_clear_temp_data_cb(void)
{
  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

void sli_zigbee_af_ota_load_file_command(void)
{
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_delete_image_cb(const sl_zigbee_af_ota_image_id_t* id)
{
  return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
}

uint8_t sl_zigbee_af_ota_storage_get_count_cb(void)
{
  return 0;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_get_full_header_cb(const sl_zigbee_af_ota_image_id_t* id,
                                                                              sl_zigbee_af_ota_header_t* returnData)
{
  return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
}

uint32_t sl_zigbee_af_ota_storage_get_total_image_size_cb(const sl_zigbee_af_ota_image_id_t* id)
{
  return 0;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_init_cb(void)
{
  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

sl_zigbee_af_ota_image_id_t sl_zigbee_af_ota_storage_iterator_first_cb(void)
{
  return sl_zigbee_invalid_image_id;
}

sl_zigbee_af_ota_image_id_t sl_zigbee_af_ota_storage_iterator_next_cb(void)
{
  return sl_zigbee_invalid_image_id;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_read_image_data_cb(const sl_zigbee_af_ota_image_id_t* id,
                                                                              uint32_t offset,
                                                                              uint32_t length,
                                                                              uint8_t* returnData,
                                                                              uint32_t* returnedLength)
{
  return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
}

sl_zigbee_af_ota_image_id_t sl_zigbee_af_ota_storage_search_cb(uint16_t manufacturerId,
                                                               uint16_t manufacturerDeviceId,
                                                               const uint16_t* hardwareVersion)
{
  return sl_zigbee_invalid_image_id;
}

uint32_t sli_zigbee_af_ota_storage_get_slot(void)
{
  return INVALID_SLOT;
}

uint32_t sl_zigbee_af_ota_storage_driver_max_download_size_cb(void)
{
  // In theory we are limited by the local disk space, but for now
  // assume there is no limit.
  return 0xFFFFFFFFUL;
}
