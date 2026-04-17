/***************************************************************************//**
 * @file
 * @brief Over The Air [Cluster] Upgrade Image Storage, Common Routines
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

// this file contains all the common includes for clusters in the util
#include "app/framework/util/common.h"
#include "app/framework/util/attribute-storage.h"

#include "app/framework/plugin/ota-common/ota.h"
#include "ota-storage.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_CATALOG_ZIGBEE_OTA_STORAGE_COMMON_PRESENT
#include "ota-storage-common-config.h"
#endif
#if defined (IMAGE_BUILDER)
// For our PC tool, we use a simpler #define to turn on this code.
  #define ZCL_USING_OTA_BOOTLOAD_CLUSTER_CLIENT
#endif

//------------------------------------------------------------------------------
// Globals

#if !defined SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE
  #define SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE 6
#endif

//------------------------------------------------------------------------------
// API

sl_zigbee_af_ota_image_id_t sli_zigbee_af_ota_storage_get_image_id_from_header(const sl_zigbee_af_ota_header_t* header)
{
  sl_zigbee_af_ota_image_id_t id = INVALID_OTA_IMAGE_ID;
  id.manufacturerId = header->manufacturerId;
  id.imageTypeId = header->imageTypeId;
  id.firmwareVersion = header->firmwareVersion;

  if (headerHasUpgradeFileDest(header)) {
    memcpy(id.deviceSpecificFileEui64,
           &header->upgradeFileDestination,
           EUI64_SIZE);
  }

  return id;
}

uint16_t sli_zigbee_af_get_upgrade_file_destination_length(uint16_t headerVersion)
{
  if (!isValidHeaderVersion(headerVersion)) {
    return 0;
  }
  return ((headerVersion == OTA_HEADER_VERSION_THREAD) ? UID_SIZE : EUI64_SIZE);
}

// Although the header length is really 16-bit, we often want to use it to increment
// a 32-bit offset variable, so just make it 32-bit.

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_header_length_and_image_size(const sl_zigbee_af_ota_image_id_t* id,
                                                                                             uint32_t *returnHeaderLength,
                                                                                             uint32_t *returnImageSize)
{
  // The sl_zigbee_af_ota_header_t struct is rather large, and on the 32-bit machines
  // all the 16-bit fields will be padded to 32-bits, thus increasing its size.
  // Most of the time we only want the header length and image size, so to
  // conserve the data stack this routine returns only those values.

  sl_zigbee_af_ota_header_t fullHeader;
  sl_zigbee_af_ota_storage_status_t status
    = sl_zigbee_af_ota_storage_get_full_header_cb(id,
                                                  &fullHeader);

  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS != status) {
    return status;
  }

  if (returnHeaderLength) {
    *returnHeaderLength = fullHeader.headerLength;
  }
  if (returnImageSize) {
    *returnImageSize = fullHeader.imageSize;
  }
  return status;
}

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_zigbee_stack_version(const sl_zigbee_af_ota_image_id_t* id,
                                                                                     uint16_t *returnZigbeeStackVersion)
{
  sl_zigbee_af_ota_header_t fullHeader;
  sl_zigbee_af_ota_storage_status_t status = sl_zigbee_af_ota_storage_get_full_header_cb(id, &fullHeader);

  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS != status) {
    return status;
  }

  if (returnZigbeeStackVersion) {
    *returnZigbeeStackVersion = fullHeader.zigbeeStackVersion;
  }
  return status;
}

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_tag_offset_and_size(const sl_zigbee_af_ota_image_id_t* id,
                                                                                    uint16_t tag,
                                                                                    uint32_t* returnTagOffset,
                                                                                    uint32_t* returnTagSize)
{
  sl_zigbee_af_tag_data_t tags[SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE];
  uint16_t totalTags;
  uint8_t i;
  bool found = false;
  uint32_t offset;
  //Initializer needed according to Lint
  uint32_t tagLength = 0;

  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
      != sli_zigbee_af_ota_storage_get_header_length_and_image_size(id,
                                                                    &offset,
                                                                    NULL) // image size return ptr (not needed)
      || (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
          != sli_zigbee_af_ota_storage_read_all_tag_info(id,
                                                         tags,
                                                         SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE,
                                                         &totalTags))) {
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  for (i = 0; !found && i < totalTags; i++) {
    offset += TAG_OVERHEAD;
    if (tags[i].id == tag) {
      tagLength = tags[i].length;
      found = true;
    } else {
      offset += tags[i].length;
    }
  }
  if (!found) {
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  *returnTagOffset = offset;
  *returnTagSize = tagLength;
  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_tag_offsets_and_sizes(const sl_zigbee_af_ota_image_id_t* id,
                                                                                      uint16_t tag,
                                                                                      uint32_t** returnTagOffset,
                                                                                      uint32_t** returnTagSize)
{
  sl_zigbee_af_tag_data_t tags[SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE];
  uint16_t totalTags;
  uint8_t i;
  uint8_t found = 0;
  uint32_t offset;
  //Initializer needed according to Lint

  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
      != sli_zigbee_af_ota_storage_get_header_length_and_image_size(id,
                                                                    &offset,
                                                                    NULL) // image size return ptr (not needed)
      || (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
          != sli_zigbee_af_ota_storage_read_all_tag_info(id,
                                                         tags,
                                                         SL_ZIGBEE_AF_PLUGIN_OTA_STORAGE_COMMON_MAX_TAGS_IN_OTA_FILE,
                                                         &totalTags))) {
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  for (i = 0; i < totalTags; i++) {
    offset += TAG_OVERHEAD;
    if (tags[i].id == tag) {
      (*returnTagOffset)[found] = offset;
      (*returnTagSize)[found] = tags[i].length;
      found++;
    }

    offset += tags[i].length;
  }

  if (found == 0) {
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  } else {
    return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
  }
}

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_tag_data_from_image(const sl_zigbee_af_ota_image_id_t* id,
                                                                                    uint16_t tag,
                                                                                    uint8_t* returnData,
                                                                                    uint32_t* returnDataLength,
                                                                                    uint32_t maxReturnDataLength)
{
  uint32_t offset;
  uint32_t tagLength;
  sl_zigbee_af_ota_storage_status_t status;

  status =
    sli_zigbee_af_ota_storage_get_tag_offset_and_size(id,
                                                      tag,
                                                      &offset,
                                                      &tagLength);
  if (status != SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS) {
    return status;
  }

  if (tagLength > maxReturnDataLength) {
    return SL_ZIGBEE_AF_OTA_STORAGE_RETURN_DATA_TOO_LONG;
  }

  return sl_zigbee_af_ota_storage_read_image_data_cb(id,
                                                     offset,
                                                     tagLength,
                                                     returnData,
                                                     returnDataLength);
}

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_raw_header_data(const sl_zigbee_af_ota_image_id_t* id,
                                                                                uint8_t* returnData,
                                                                                uint32_t* returnDataLength,
                                                                                uint32_t maxReturnDataLength)
{
  UNUSED_VAR(maxReturnDataLength);

  // We need to know the header size before we can get the raw data.
  uint32_t headerLength;
  sl_zigbee_af_ota_storage_status_t status
    = sli_zigbee_af_ota_storage_get_header_length_and_image_size(id,
                                                                 &headerLength,
                                                                 NULL); // image size ptr (don't care)

  if (status) {
    return status;
  }

  if (*returnDataLength < headerLength) {
    return SL_ZIGBEE_AF_OTA_STORAGE_RETURN_DATA_TOO_LONG;
  }

  return sl_zigbee_af_ota_storage_read_image_data_cb(id,
                                                     0,      // start offset
                                                     headerLength,
                                                     returnData,
                                                     returnDataLength);
}

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_read_all_tag_info(const sl_zigbee_af_ota_image_id_t* id,
                                                                              sl_zigbee_af_tag_data_t* tagInfo,
                                                                              uint16_t maxTags,
                                                                              uint16_t* totalTags)
{
  uint8_t tagData[TAG_OVERHEAD];
  uint32_t returnedLength;
  uint32_t offset;
  uint32_t imageSize;
  sl_zigbee_af_ota_storage_status_t status
    = sli_zigbee_af_ota_storage_get_header_length_and_image_size(id,
                                                                 &offset,
                                                                 &imageSize);

  if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS != status) {
    return status;
  }

  // Although there is a field in the file that tells us the max
  // number of tags in the file, we do not rely upon that.
  // We count the number of tags actually in the file.
  *totalTags = 0;

  while (offset < imageSize) {
    uint32_t lengthOfEmberAfOtaStorageStatusInFile;
    sl_zigbee_af_ota_storage_status_t status =
      sl_zigbee_af_ota_storage_read_image_data_cb(id,
                                                  offset,
                                                  TAG_OVERHEAD,
                                                  tagData,
                                                  &returnedLength);
    if (status != SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
        || TAG_OVERHEAD != returnedLength) {
      // File is too short or could not read the data on disk.
      return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
    }
    lengthOfEmberAfOtaStorageStatusInFile = ((uint32_t)(tagData[2])
                                             + ((uint32_t)(tagData[3]) << 8)
                                             + ((uint32_t)(tagData[4]) << 16)
                                             + ((uint32_t)(tagData[5]) << 24));
    if (*totalTags < maxTags) {
      tagInfo[*totalTags].id = tagData[0] + (tagData[1] << 8);
      tagInfo[*totalTags].length = lengthOfEmberAfOtaStorageStatusInFile;
    } else if (*totalTags > maxTags) {
      // To prevent bad data from making us go into an endless loop we will
      // stop processing tags once we surpass the number of max tags we
      // can return data for.
      return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
    }
    (*totalTags)++;
    offset += TAG_OVERHEAD + lengthOfEmberAfOtaStorageStatusInFile;
  }

  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

bool sl_zigbee_af_is_ota_image_id_valid(const sl_zigbee_af_ota_image_id_t* idToCompare)
{
  return (0 != memcmp(idToCompare,
                      &sl_zigbee_af_invalid_image_id,
                      sizeof(sl_zigbee_af_ota_image_id_t)));
}
