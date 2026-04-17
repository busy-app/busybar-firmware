/***************************************************************************//**
 * @file
 * @brief This file defines the interface to a Over-the-air (OTA) storage device.  It
 * can be used by either a server or client.  The server can store 0 or more
 * files that are indexed uniquely by an identifier made up of their Version
 * Number, Manufacturer ID, and Device ID.
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

#ifndef SILABS_OTA_STORAGE_H
#define SILABS_OTA_STORAGE_H

/**
 * @defgroup ota-storage-common OTA Storage Common
 * @ingroup component
 * @brief API and Callbacks for the OTA Storage Common Component
 *
 * Common code for any storage module implementing an Over-the-Air bootload cluster.
 * It is used by either the Zigbee or legacy Ember Standalone bootloader implementations.
 *
 */

/**
 * @addtogroup ota-storage-common
 * @{
 */

/**
 * @def OTA_MINIMUM_HEADER_LENGTH
 * @brief Defines the minimum header length for the OTA file
 *
 * The total length (in bytes) was calulated from the following fields: \n
 * 4 bytes: OTA upgrade file identifier \n
 * 2 bytes: Unsigned 16-bit integer OTA Header version \n
 * 2 bytes: Unsigned 16-bit integer OTA Header length \n
 * 2 bytes: Unsigned 16-bit integer OTA Header Field control \n
 * 2 bytes: Unsigned 16-bit integer Manufacturer code \n
 * 2 bytes: Unsigned 16-bit integer Image type \n
 * 4 bytes: Unsigned 32-bit integer File version \n
 * 2 bytes: Unsigned 16-bit integer ZigBee Stack version \n
 * 32 bytes: Character string OTA Header string \n
 * 4 bytes: Unsigned 32-bit integer Total Image size (including header)
 */
#define OTA_MINIMUM_HEADER_LENGTH (20 + 32 + 4)
// Optional fields are: security credentials, upgrade dest, and HW versions
/**
 * @def OTA_MAXIMUM_HEADER_LENGTH
 * @brief Defines the maximum header length for the OTA file
 *
 * The total length (in bytes) was calulated from the following fields: \n
 * 1 byte: Unsigned 8-bit integer Security credential version \n
 * 8 bytes: IEEE Address Upgrade file destination \n
 * 2 bytes: Unsigned 16-bit integer Minimum hardware version \n
 * 2 bytes: Unsigned 16-bit integer
 */
#define OTA_MAXIMUM_HEADER_LENGTH (OTA_MINIMUM_HEADER_LENGTH + 1 + 8 + 4)

// For EEPROM parts with 2-byte word sizes we need to make sure we read
// on word boundaries.
#define OTA_MAXIMUM_HEADER_LENGTH_2_BYTE_ALIGNED (OTA_MAXIMUM_HEADER_LENGTH + 1)
#define OTA_MAXIMUM_HEADER_LENGTH_4_BYTE_ALIGNED (OTA_MAXIMUM_HEADER_LENGTH + 3)

#define OTA_FILE_MAGIC_NUMBER        0x0BEEF11EL

#define MAGIC_NUMBER_OFFSET    0
#define HEADER_VERSION_OFFSET  4
#define HEADER_LENGTH_OFFSET   6
#define FIELD_CONTROL_OFFSET   8
#define MANUFACTURER_ID_OFFSET 10
#define IMAGE_TYPE_ID_OFFSET   12
#define VERSION_OFFSET         14
#define STACK_VERSION_OFFSET   18
#define HEADER_STRING_OFFSET   20
#define IMAGE_SIZE_OFFSET      52
#define OPTIONAL_FIELDS_OFFSET 56
// The rest are optional fields.

#define HEADER_LENGTH_FIELD_LENGTH 2

/**
 * @def TAG_OVERHEAD
 * @brief Tag Overhead. 2 bytes for the Tag ID and 4 bytes for the length
 */
#define TAG_OVERHEAD (2 + 4)   // 2 bytes for the tag ID, 4 bytes for the length

// Optional fields are: security credentials, upgrade dest, and HW versions
/**
 * @def isValidHeaderVersion
 * @brief Check if the header version is valid
 */
#define isValidHeaderVersion(headerVersion) \
  ((headerVersion == OTA_HEADER_VERSION_ZIGBEE) || (headerVersion == OTA_HEADER_VERSION_THREAD))

#define SECURITY_CREDENTIAL_VERSION_FIELD_PRESENT_MASK 0x0001
#define DEVICE_SPECIFIC_FILE_PRESENT_MASK              0x0002
#define HARDWARE_VERSIONS_PRESENT_MASK                 0x0004

/**
 * @def headerHasSecurityCredentials
 * @brief Check if the header has security credentials present
 */
#define headerHasSecurityCredentials(header) \
  ((header)->fieldControl & SECURITY_CREDENTIAL_VERSION_FIELD_PRESENT_MASK)
/**
 * @def headerHasUpgradeFileDest
 * @brief Check if the header has an upgrade file destination
 */
#define headerHasUpgradeFileDest(header) \
  ((header)->fieldControl & DEVICE_SPECIFIC_FILE_PRESENT_MASK)

/**
 * @def headerHasHardwareVersions
 * @brief Check if the header has the hardware version present
 */
#define headerHasHardwareVersions(header) \
  ((header)->fieldControl & HARDWARE_VERSIONS_PRESENT_MASK)

// This size does NOT include the tag overhead.
#define SIGNATURE_TAG_DATA_SIZE       (EUI64_SIZE + SL_ZIGBEE_SIGNATURE_SIZE)
#define SIGNATURE_283K1_TAG_DATA_SIZE (EUI64_SIZE + SL_ZIGBEE_SIGNATURE_283K1_SIZE)

#define INVALID_MANUFACTURER_ID  0xFFFF
#define INVALID_DEVICE_ID        0xFFFF
#define INVALID_FIRMWARE_VERSION 0xFFFFFFFFUL
#define INVALID_EUI64 { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

#define INVALID_OTA_IMAGE_ID  \
  { INVALID_MANUFACTURER_ID,  \
    INVALID_DEVICE_ID,        \
    INVALID_FIRMWARE_VERSION, \
    INVALID_EUI64,            \
  }

#define INVALID_SLOT      (uint32_t)-1

/**
 * @def DEFAULT_SLOT
 * @brief Default to using storage slot 0
 */
#define DEFAULT_SLOT      0

/**
 * @name API
 * @{
 */

/** @brief A call to check if the OTA Image ID is valid
 *
 * @param[in] idToCompare a pointer of type: sl_zigbee_af_ota_image_id_t referring to the OTA image ID
 *
 * @returns A ::bool value returning true if the OTA Id is valid
 */
bool sl_zigbee_af_is_ota_image_id_valid(const sl_zigbee_af_ota_image_id_t* idToCompare);

/** @} */ // end of name API

/**
 * @name Callbacks
 * @{
 */

/**
 * @defgroup ota_storage_cb OTA Storage
 * @ingroup af_callback
 * @brief Callbacks for OTA Storage Component
 *
 */

/**
 * @addtogroup ota_storage_cb
 * @{
 */

// This should be moved into the plugin callbacks file.
/** @brief A callback returning the status of the OTA storage deleting an image.
 *
 * @param[in] id  a pointer of type: sl_zigbee_af_ota_image_id_t referring to the OTA image ID
 *
 * @returns A ::sl_zigbee_af_ota_storage_status_t on if image deletion was successful.
 */
sl_zigbee_af_ota_storage_status_t sl_zigbee_af_ota_storage_delete_image_cb(const sl_zigbee_af_ota_image_id_t* id);

/** @brief OTA Storage Driver max download size callback.
 *  @param NULL
 *  @returns A ::uint32_t on the maximum download size for the OTA file.
 */
uint32_t sl_zigbee_af_ota_storage_driver_max_download_size_cb(void);

/** @} */ // end of ota_storage_cb
/** @} */ // end of name Callbacks
/** @} */ // end of ota-storage-common

//------------------------------------------------------------------------------

// Initialization
// (For the POSIX implementation the device will be a file or directory)
sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_set_device(const void* device);
void sli_zigbee_af_ota_storage_close(void);

const char* sli_zigbee_af_ota_storage_get_filepath(const sl_zigbee_af_ota_image_id_t* id);

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_add_image_file(const char* filename);

// Creating (two options)
//  - Create a file based on a passed "sl_zigbee_af_ota_header_t" structure, stored
//      at the filename passed to the function.  This is usually done by a
//      PC tool.
//  - Create a file based on raw data (presumably received over the air)
//      This will be stored in a single static temp file.
sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_create_image(sl_zigbee_af_ota_header_t* header,
                                                                         const char* filename);
sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_append_image_data(const char* filename,
                                                                              uint32_t length,
                                                                              const uint8_t* data);

//------------------------------------------------------------------------------
// Generic routines that are independent of the actual storage mechanism.

// Will return 0 on invalid headerVersion
uint16_t sli_zigbee_af_get_upgrade_file_destination_length(uint16_t headerVersion);

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_header_length_and_image_size(const sl_zigbee_af_ota_image_id_t* id,
                                                                                             uint32_t *returnHeaderLength,
                                                                                             uint32_t *returnImageSize);

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_zigbee_stack_version(const sl_zigbee_af_ota_image_id_t* id,
                                                                                     uint16_t *returnZigbeeStackVersion);

sl_zigbee_af_ota_image_id_t sli_zigbee_af_ota_storage_get_image_id_from_header(const sl_zigbee_af_ota_header_t* header);

// Returns the offset and size of the actual data (does not include
// tag meta-data) in the specified tag.
sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_tag_offset_and_size(const sl_zigbee_af_ota_image_id_t* id,
                                                                                    uint16_t tag,
                                                                                    uint32_t* returnTagOffset,
                                                                                    uint32_t* returnTagSize);

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_tag_offsets_and_sizes(const sl_zigbee_af_ota_image_id_t* id,
                                                                                      uint16_t tag,
                                                                                      uint32_t** returnTagOffset,
                                                                                      uint32_t** returnTagSize);

sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_tag_data_from_image(const sl_zigbee_af_ota_image_id_t* id,
                                                                                    uint16_t tag,
                                                                                    uint8_t* returnData,
                                                                                    uint32_t* returnDataLength,
                                                                                    uint32_t maxReturnDataLength);

// This gets the OTA header as it is formatted in the file, including
// the magic number.
sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_get_raw_header_data(const sl_zigbee_af_ota_image_id_t* id,
                                                                                uint8_t* returnData,
                                                                                uint32_t* returnDataLength,
                                                                                uint32_t maxReturnDataLength);

// This retrieves a list of all tags in the file and their lengths.
// It will read at most 'maxTags' and return that array data in tagInfo.
sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_storage_read_all_tag_info(const sl_zigbee_af_ota_image_id_t* id,
                                                                              sl_zigbee_af_tag_data_t* tagInfo,
                                                                              uint16_t maxTags,
                                                                              uint16_t* totalTags);

void sli_zigbee_af_ota_storage_info_print(void);
void sli_zigbee_af_ota_storage_driver_info_print(void);

// This retrieves the slot being used to store OTA images if the OTA Simple
// Storage EEPROM Driver plugin has been configured to use slots
uint32_t sli_zigbee_af_ota_storage_get_slot(void);

// Our storage device examines all files in the passed directory, or simply
// loads a single file into its header cache.
sl_zigbee_af_ota_storage_status_t sli_zigbee_af_ota_set_storage_device(const void* device);

//------------------------------------------------------------------------------
// Internal (for debugging malloc() and free())
int remainingAllocations(void);

#endif // SILABS_OTA_STORAGE_H
