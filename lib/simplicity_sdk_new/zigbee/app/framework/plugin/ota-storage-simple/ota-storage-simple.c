/***************************************************************************//**
 * @file
 * @brief This is the implementation of a very simple storage interface for either
 * OTA client or server.  It keeps track of only one file.  The actual storage
 * device is defined by the developer, but it is assumed to be a device
 * with a continuous block of space (such as an EEPROM).
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

//------------------------------------------------------------------------------
// Globals

// Either we have a full image that can be used by a server to send to clients
// and can be used by clients to bootload, or we have a partial image that can
// be used by clients in the process of downloading a file.
static bool storageHasFullImage;

// We cache certain image info since it will be accessed often.
static sl_zigbee_af_ota_image_id_t cachedImageId;
static uint32_t cachedImageSize;

static const uint8_t magicNumberBytes[] = { 0x1e, 0xf1, 0xee, 0x0b };
#define TOTAL_MAGIC_NUMBER_BYTES  4

// This is the minimum length we need to obtain maunfacturer ID, image type ID
// and version
#define IMAGE_INFO_HEADER_LENGTH (VERSION_OFFSET + 4)

//------------------------------------------------------------------------------
// Forward Declarations

static sl_zigbee_af_ota_storage_status_t readAndValidateStoredImage(sl_zigbee_af_ota_header_t* returnData,
                                                                    bool printMagicNumberError);
static sl_zigbee_af_ota_image_id_t getCurrentImageId(void);
static uint16_t getLittleEndianInt16uFromBlock(const uint8_t* block);
static uint32_t getLittleEndianInt32uFromBlock(const uint8_t* block);
static bool initDone = false;

//------------------------------------------------------------------------------

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_init_cb(void)
{
  sl_zigbee_af_ota_header_t header;

  if (initDone) {
    return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
  }

  if (!sl_zigbee_af_ota_storage_driver_init_cb()) {
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  storageHasFullImage = (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
                         == readAndValidateStoredImage(&header,
                                                       false));  // print magic number error?

  initDone = true;

  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

void sli_zigbee_af_ota_storage_simple_init_callback(uint8_t init_level)
{
  (void)init_level;

  sl_zigbee_af_ota_storage_init_cb();
}

uint8_t sl_zigbee_af_ota_storage_get_count_cb(void)
{
  return (storageHasFullImage ? 1 : 0);
}

sl_zigbee_af_ota_image_id_t sl_zigbee_af_ota_storage_search_cb(uint16_t manufacturerId,
                                                               uint16_t manufacturerDeviceId,
                                                               const uint16_t* hardwareVersion)
{
  if (manufacturerId != cachedImageId.manufacturerId
      || manufacturerDeviceId != cachedImageId.imageTypeId) {
    return sl_zigbee_af_invalid_image_id;
  }

  if (hardwareVersion) {
    sl_zigbee_af_ota_header_t header;
    if (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
        != readAndValidateStoredImage(&header,
                                      false)) {  // print magic number error?
      return sl_zigbee_af_invalid_image_id;
    }
    if (headerHasHardwareVersions(&header)
        && (header.minimumHardwareVersion > *hardwareVersion
            || header.maximumHardwareVersion < *hardwareVersion)) {
      return sl_zigbee_af_invalid_image_id;
    }
  }
  return cachedImageId;
}

sl_zigbee_af_ota_image_id_t sl_zigbee_af_ota_storage_iterator_first_cb(void)
{
  return getCurrentImageId();
}

sl_zigbee_af_ota_image_id_t sl_zigbee_af_ota_storage_iterator_next_cb(void)
{
  return sl_zigbee_af_invalid_image_id;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_get_full_header_cb(const sl_zigbee_af_ota_image_id_t* id,
                                                                              sl_zigbee_af_ota_header_t* returnData)
{
  sl_zigbee_af_ota_storage_status_t status;

  if (!storageHasFullImage) {
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  status = readAndValidateStoredImage(returnData,
                                      false);  // print magic number error?
  return (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS == status
          && 0 == memcmp(&cachedImageId, id, sizeof(sl_zigbee_af_ota_image_id_t))
          ? SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
          : SL_ZIGBEE_AF_OTA_STORAGE_ERROR);
}

uint32_t sl_zigbee_af_ota_storage_get_total_image_size_cb(const sl_zigbee_af_ota_image_id_t* id)
{
  if (storageHasFullImage
      && (0 == memcmp(&cachedImageId, id, sizeof(sl_zigbee_af_ota_image_id_t)))) {
    return cachedImageSize;
  }
  return 0;
}

static void clearCachedId(void)
{
  memset(&cachedImageId, 0, sizeof(sl_zigbee_af_ota_image_id_t));
}

static sl_zigbee_af_ota_storage_status_t readAndValidateStoredImage(sl_zigbee_af_ota_header_t* returnData,
                                                                    bool printMagicNumberError)
{
  uint8_t data[OTA_MAXIMUM_HEADER_LENGTH_4_BYTE_ALIGNED];
  uint8_t indexOrReadLength = 0;
  uint8_t parseData;

  // Read up to the maximum OTA header size even if it is not that big.
  // The code will not process the extra bytes unless the header indicates
  // optional fields are used.
  if (!sl_zigbee_af_ota_storage_driver_read_cb(0,   // offset
                                               OTA_MAXIMUM_HEADER_LENGTH_4_BYTE_ALIGNED,
                                               data)) {
    otaPrintln("sl_zigbee_af_ota_storage_driver_read_cb() failed!");
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  memset(returnData, 0, sizeof(sl_zigbee_af_ota_header_t));

  if (0 != memcmp(data, magicNumberBytes, TOTAL_MAGIC_NUMBER_BYTES)) {
    if (printMagicNumberError) {
      otaPrintln("Bad magic number in file: 0x%02X 0x%02X 0x%02X 0x%02X",
                 data[0],
                 data[1],
                 data[2],
                 data[3]);
    }
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  returnData->headerVersion =
    getLittleEndianInt16uFromBlock(data + HEADER_VERSION_OFFSET);

  if (returnData->headerVersion != OTA_HEADER_VERSION_ZIGBEE) {
    otaPrintln("Bad version in file 0x%04X", returnData->headerVersion);
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }

  returnData->headerLength =
    getLittleEndianInt16uFromBlock(data + HEADER_LENGTH_OFFSET);
  returnData->fieldControl =
    getLittleEndianInt16uFromBlock(data + FIELD_CONTROL_OFFSET);
  returnData->manufacturerId =
    getLittleEndianInt16uFromBlock(data + MANUFACTURER_ID_OFFSET);
  returnData->imageTypeId =
    getLittleEndianInt16uFromBlock(data + IMAGE_TYPE_ID_OFFSET);
  returnData->firmwareVersion =
    getLittleEndianInt32uFromBlock(data + VERSION_OFFSET);
  returnData->zigbeeStackVersion =
    getLittleEndianInt16uFromBlock(data + STACK_VERSION_OFFSET);

  // We add +1 because the actual length of the string in the data structure
  // is longer.  This is to account for a 32-byte string in the OTA header
  // that does NOT have a NULL terminator.
  memset(returnData->headerString, 0, SL_ZIGBEE_AF_OTA_MAX_HEADER_STRING_LENGTH + 1);
  memmove(returnData->headerString,
          data + HEADER_STRING_OFFSET,
          SL_ZIGBEE_AF_OTA_MAX_HEADER_STRING_LENGTH);

  returnData->imageSize =
    getLittleEndianInt32uFromBlock(data + IMAGE_SIZE_OFFSET);
  cachedImageSize = returnData->imageSize;

  clearCachedId();
  cachedImageId.manufacturerId  = returnData->manufacturerId;
  cachedImageId.imageTypeId     = returnData->imageTypeId;
  cachedImageId.firmwareVersion = returnData->firmwareVersion;

  if (returnData->fieldControl == 0) {
    return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
  }

  // In an attempt to save flash space, reuse the conditional code here.
  // First time through we figure out the total size of the optional fields
  // to read from flash.
  // Second time through we extract the data and copy into the data struct.
  for (parseData = 0; parseData < 2; parseData++) {
    if (headerHasSecurityCredentials(returnData)) {
      if (parseData) {
        returnData->securityCredentials = data[indexOrReadLength];
      }
      indexOrReadLength++;
    }
    if (headerHasUpgradeFileDest(returnData)) {
      uint16_t upgradeFileDestinationLength = sli_zigbee_af_get_upgrade_file_destination_length(returnData->headerVersion);
      if (parseData) {
        memmove(&returnData->upgradeFileDestination,
                data + indexOrReadLength,
                upgradeFileDestinationLength);
        memmove(cachedImageId.deviceSpecificFileEui64,
                data + indexOrReadLength,
                upgradeFileDestinationLength);
      }
      indexOrReadLength += upgradeFileDestinationLength;
    }
    if (headerHasHardwareVersions(returnData)) {
      if (parseData) {
        returnData->minimumHardwareVersion =
          getLittleEndianInt16uFromBlock(data + indexOrReadLength);
        returnData->maximumHardwareVersion =
          getLittleEndianInt16uFromBlock(data + indexOrReadLength + 2);
      }
      indexOrReadLength += 4;
    }

    if (parseData) {
      // We are done.  Don't read data from the storage device.
      continue;
    }

    if (indexOrReadLength > OTA_MAXIMUM_HEADER_LENGTH_4_BYTE_ALIGNED) {
      otaPrintln("Error: readAndValidateStoredImage() tried to read more data than it could hold.");
      return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
    }

    if (!sl_zigbee_af_ota_storage_driver_read_cb(OPTIONAL_FIELDS_OFFSET,
                                                 indexOrReadLength, // now it's a read length
                                                 data)) {
      otaPrintln("sl_zigbee_af_ota_storage_driver_read_cb() FAILED!");
      return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
    }
    indexOrReadLength = OPTIONAL_FIELDS_OFFSET;  // now it's an index again
  } // end for loop

  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_read_image_data_cb(const sl_zigbee_af_ota_image_id_t* id,
                                                                              uint32_t offset,
                                                                              uint32_t length,
                                                                              uint8_t* returnData,
                                                                              uint32_t* returnedLength)
{
  if (0 != memcmp(id, &cachedImageId, sizeof(sl_zigbee_af_ota_image_id_t))) {
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }
  length = (length > (cachedImageSize - offset)
            ? (cachedImageSize - offset)
            : length);

  if (!sl_zigbee_af_ota_storage_driver_read_cb(offset,
                                               length,
                                               returnData)) {
    otaPrintln("sl_zigbee_af_ota_storage_driver_read_cb() FAILED!");
    return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
  }
  *returnedLength = length;
  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_finish_download_cb(uint32_t offset)
{
  sl_zigbee_af_ota_storage_driver_download_finish_cb(offset);
  return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_clear_temp_data_cb(void)
{
  storageHasFullImage = false;

  return sl_zigbee_af_ota_storage_driver_invalidate_image_cb();
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_write_temp_data_cb(uint32_t offset,
                                                                              uint32_t length,
                                                                              const uint8_t* data)
{
  return (sl_zigbee_af_ota_storage_driver_write_cb(data,
                                                   offset,
                                                   length)
          ? SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
          : SL_ZIGBEE_AF_OTA_STORAGE_ERROR);
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_check_temp_data_cb(uint32_t* returnOffset,
                                                                              uint32_t* returnTotalSize,
                                                                              sl_zigbee_af_ota_image_id_t* returnOtaImageId)
{
  // Right now, we don't support continuing a download after a reboot.  Either the
  // entire image is in flash or we assume nothing has been downloaded.
  sl_zigbee_af_ota_header_t header;
  uint32_t lastOffset = sl_zigbee_af_ota_storage_driver_retrieve_last_stored_offset_cb();
  otaPrintln("Last offset downloaded: 0x%08X", lastOffset);
  if (lastOffset > OTA_MINIMUM_HEADER_LENGTH
      && (SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS
          == readAndValidateStoredImage(&header,
                                        true))) {  // print magic number error?
    *returnTotalSize = header.imageSize;
    *returnOtaImageId = sli_zigbee_af_ota_storage_get_image_id_from_header(&header);
    if (lastOffset >= header.imageSize) {
      storageHasFullImage = true;
      *returnOffset = header.imageSize;
    } else {
      *returnOffset = lastOffset;
      return SL_ZIGBEE_AF_OTA_STORAGE_PARTIAL_FILE_FOUND;
    }
    return SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;
  }
  storageHasFullImage = false;

  return SL_ZIGBEE_AF_OTA_STORAGE_ERROR;
}

sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_delete_image_cb(const sl_zigbee_af_ota_image_id_t* id)
{
  UNUSED_VAR(id);
  sl_zigbee_af_ota_storage_status_t status = SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS;

  // EMAPPFWKV2-1169: if we have an image, we try to delete it, and return that
  // status code. If we don't have an image, then we are ready to receive
  // another one, so we should return success.

  if (storageHasFullImage) {
    status = sl_zigbee_af_ota_storage_driver_invalidate_image_cb();
  }

  storageHasFullImage = false;
  clearCachedId();

  return status;
}

void sli_zigbee_af_ota_storage_info_print(void)
{
  otaPrintln("Storage Module: OTA Simple Storage Plugin");
  otaPrintln("Images Stored:  %d of 1\n",
             (storageHasFullImage ? 1 : 0));
  sli_zigbee_af_ota_storage_driver_info_print();
}

static sl_zigbee_af_ota_image_id_t getCurrentImageId(void)
{
  if (!storageHasFullImage) {
    return sl_zigbee_af_invalid_image_id;
  }
  return cachedImageId;
}

static uint16_t getLittleEndianInt16uFromBlock(const uint8_t* block)
{
  return (block[0] + (((uint16_t)(block[1])) << 8));
}

static uint32_t getLittleEndianInt32uFromBlock(const uint8_t* block)
{
  uint32_t value = (getLittleEndianInt16uFromBlock(block)
                    + (((uint32_t)getLittleEndianInt16uFromBlock(block + 2))
                       << 16));
  return value;
}
