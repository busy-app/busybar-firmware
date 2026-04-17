/***************************************************************************//**
 * @file
 * @brief Interface header for BLOB storage component internal interfaces
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SLI_BTMESH_BLOB_STORAGE_H
#define SLI_BTMESH_BLOB_STORAGE_H

// -----------------------------------------------------------------------------
// Includes

#include <assert.h>
#include <stdint.h>

#include "sl_bgapi.h"
#include "sl_status.h"

#include "sl_btmesh_blob_storage_config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// -----------------------------------------------------------------------------
// Macros

// The biggest alignment supported
#define SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT                       8

// -----------------------------------------------------------------------------
// Enums, structs, typedefs

// These values control the strictness of the validity checks of storage slot content
// The exact meaning of these values are determined by the specific function that
// uses this enum as a parameter.
typedef enum {
  SLI_BTMESH_BLOB_STORAGE_CHECK_STRICT = 0,  // Strict and exhaustive but time-consuming check
  SLI_BTMESH_BLOB_STORAGE_CHECK_RELAXED = 1, // Relaxed and performant subset of checks
  SLI_BTMESH_BLOB_STORAGE_CHECK_IGNORE = 2   // No check
} sli_btmesh_blob_storage_check_t;

/// Structure of the next application specific footer metadata storage offset
PACKSTRUCT(typedef struct {
  /// Offset of the next application footer metadata in storage slot
  uint32_t offset;
  /// Inverted next
  uint32_t offset_inv;
}) sli_btmesh_blob_storage_app_footer_next_t;

// Size of the next field of the application footer metadata shall be integer
// multiple of storage alignment otherwise the bootloader_writeStorage function
// fails because the next field is written separately from the rest of the struct
static_assert(sizeof(sli_btmesh_blob_storage_app_footer_next_t)
              % SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL == 0,
              "Size of app footer metadata next field isn't integer multiple "
              "of storage alignment.");

/// Structure of the application specific footer metadata
PACKSTRUCT(typedef struct {
  /// Next application footer metadata storage slot offset
  /// @note It is essential to keep the next and next_inv fields on multiple of
  /// 8 bytes because that is the maximum alignment supported and these two fields
  /// are written separately from the rest of the stucture
  sli_btmesh_blob_storage_app_footer_next_t next;
  /// Inverted application identifier
  uint16_t app_id_inv;
  /// Inverted length
  uint16_t length_inv;
  /// Application identifier to identify footer user
  uint16_t app_id;
  /// Length of the footer and this metadata
  uint16_t length;
}) sli_btmesh_blob_storage_app_footer_metadata_t;

// Size of application footer metadata shall be integer multiple of storage
// alignment otherwise the bootloader_writeStorage function fails
static_assert(sizeof(sli_btmesh_blob_storage_app_footer_metadata_t)
              % SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL == 0,
              "Size of app footer metadata isn't integer multiple of storage alignment.");

// Offset of the next field of the application footer metadata in the struct
// shall be integer multiple of alignment otherwise the bootloader_writeStorage
// function fails because the next field is written separately from the rest of
// the structure
static_assert(offsetof(sli_btmesh_blob_storage_app_footer_metadata_t, next)
              % SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL == 0,
              "Offset of app footer metadata next field isn't integer multiple "
              "of storage alignment.");

/// Structure of the application specific footer metadata checksum
PACKSTRUCT(typedef struct {
  /// Inverted checksum
  uint32_t checksum_inv;
  /// Checksum of application footer metadata payload
  uint32_t checksum;
}) sli_btmesh_blob_storage_app_footer_checksum_t;

// Size of application footer metadata checksum shall be integer multiple of
// alignment otherwise the bootloader_writeStorage function fails
static_assert(sizeof(sli_btmesh_blob_storage_app_footer_checksum_t)
              % SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL == 0,
              "Size of app footer metadata checksum isn't integer multiple of "
              "storage alignment.");

/// Footer of a BLOB stored at the end of the slot
PACKSTRUCT(typedef struct {
  /// Magic bytes to detect footer corruption
  uint8_t magic[SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT];
  /// Inverted BLOB identifier
  sl_bt_uuid_64_t blob_id_inv;
  /// Inverted BLOB size
  uint32_t blob_size_inv;
  /// Inverted Reserved (not enforced inversion)
  uint8_t rfu_inv[3];
  /// Inverted version
  uint8_t version_inv;
  /// Validity flag
  uint8_t validity_flag[SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT];
  /// Identifier of BLOB in storage slot
  sl_bt_uuid_64_t blob_id;
  /// Size of BLOB in storage slot
  uint32_t blob_size;
  /// Reserved for future use
  uint8_t rfu[3];
  /// Footer version
  uint8_t version;
}) sli_btmesh_blob_storage_footer_t;

// Size of the BLOB storage footer shall be integer multiple of alignment
// otherwise the bootloader_writeStorage function fails
static_assert(sizeof(sli_btmesh_blob_storage_footer_t)
              % SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL == 0,
              "Size of BLOB storage footer isn't integer multiple of "
              "storage alignment.");

// Size of the validity flag field of the BLOB storage footer shall be integer
// multiple of alignment otherwise the bootloader_writeStorage function fails
// because the validity flag field is written separately from the rest of the
// structure
static_assert(sizeof(((sli_btmesh_blob_storage_footer_t *)0)->validity_flag)
              % SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL == 0,
              "Size of BLOB storage footer validity flag field isn't integer "
              "multiple of storage alignment.");

// Offset of the validity flag field of the BLOB storage footer in the struct
// shall be integer multiple of alignment otherwise the bootloader_writeStorage
// function fails because the validity flag field is written separately from the
// rest of the structure
static_assert(offsetof(sli_btmesh_blob_storage_footer_t, validity_flag)
              % SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL == 0,
              "Offset of BLOB storage footer validity flag field isn't integer "
              "multiple of storage alignment.");

// -----------------------------------------------------------------------------
// Internal function declarations

/***************************************************************************//**
 * Initializes the storage erase functionality
 ******************************************************************************/
void sli_btmesh_blob_storage_erase_init(void);

/***************************************************************************//**
 * Read data from bootloader and stores it in the cache
 *
 * @param[in] app_footer_check Strictness of checking the application footer
 ******************************************************************************/
void sli_btmesh_blob_storage_sync(sli_btmesh_blob_storage_check_t app_footer_check);

/***************************************************************************//**
 * Query status of the slot
 *
 * @param[in] slot_id Index of the slot
 *
 * @return Status of the slot
 * @retval SL_BTMESH_BLOB_STORAGE_STATUS_EMPTY If slot is empty
 * @retval SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED If data is present and corrupted
 * @retval SL_BTMESH_BLOB_STORAGE_STATUS_INVALIDATED If data was invalidated by application
 * @retval SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED If data is present and valid
 * @retval SL_BTMESH_BLOB_STORAGE_STATUS_INTERNAL_ERROR Some internal error occurred
 ******************************************************************************/
sl_btmesh_blob_storage_status_t sli_btmesh_blob_storage_get_slot_status(uint32_t slot_id);

/***************************************************************************//**
 * Check if slot is occupied
 *
 * @param[in] slot_id Index of the slot (assumed to be valid)
 *
 * @return Slot occupied status
 * @retval true Slot is occupied
 * @retval false Slot is unoccupied (empty, corrupted, invalidated) or
 *               internal error occurred
 ******************************************************************************/
bool sli_btmesh_blob_storage_is_slot_occupied(uint32_t slot_id);

/***************************************************************************//**
 * Check if slot is unoccupied
 *
 * @param[in] slot_id Index of the slot (assumed to be valid)
 *
 * @return Slot unoccupied status
 * @retval true Slot is unoccupied (empty, corrupted, invalidated)
 * @retval false Slot is occupied or internal error occurred
 ******************************************************************************/
bool sli_btmesh_blob_storage_is_slot_unoccupied(uint32_t slot_id);

/***************************************************************************//**
 * Check if slot contains garbage data
 *
 * @param[in] slot_id Index of the slot (assumed to be valid)
 *
 * @return Slot garbage status
 * @retval true Slot is corrupted or invalidated
 * @retval false Slot is empty, occupied or internal error occurred
 ******************************************************************************/
bool sli_btmesh_blob_storage_has_slot_garbage(uint32_t slot_id);

/***************************************************************************//**
 * Check if BLOB ID is stored in any of the slots and identifies it.
 *
 * @param[in] blob_id BLOB ID inquired about
 *
 * @return Identifier of the slot containing the identified BLOB
 * @retval UINT32_MAX In case no slot is found
 ******************************************************************************/
uint32_t sli_btmesh_blob_storage_get_slot_id(sl_bt_uuid_64_t const *blob_id);

/***************************************************************************//**
 * Extracts footer information assigned to a given slot, belonging to an
 * application identifier.
 *
 * @param slot_id Index of the slot
 * @param[in] app_id Identifier of the application layer user
 * @param[out] buffer Pointer to footer buffer
 * @param[inout] length Length of the buffer; contains the length of the data
 *                      read in case of success
 *
 * @return Status code
 * @retval SL_STATUS_OK In case of successful reading
 * @retval SL_STATUS_NOT_FOUND If app_id and blob_id don't identify a footer
 * @retval SL_STATUS_INVALID_PARAMETER If buffer is too short
 * @retval SL_STATUS_FAIL In case of internal API error
 ******************************************************************************/
sl_status_t sli_btmesh_blob_storage_get_app_footer(uint32_t slot_id,
                                                   uint16_t app_id,
                                                   void *buffer,
                                                   uint16_t *length);
/***************************************************************************//**
 * Check if BLOB is managed.
 *
 * @see sl_btmesh_blob_storage_set_managed
 *
 * @param[in] slot_id Index of the slot
 *
 * @return Boolean value
 * @retval true BLOB is managed
 * @retval false Slot isn't managed
 ******************************************************************************/
bool sli_btmesh_blob_storage_is_managed(uint32_t slot_id);

/***************************************************************************//**
 * Check if slot is managed by a specific owner.
 *
 * @see sl_btmesh_blob_storage_set_managed
 *
 * @param[in] slot_id Index of the slot
 * @param[in] owner_id Identifier of expected owner
 *
 * @return Boolean value
 * @retval true BLOB belongs to the specified owner
 * @retval false Slot belongs to another owner or Slot is not managed or
 *   BLOB does not exist
 ******************************************************************************/
bool sli_btmesh_blob_storage_is_managed_by_owner(uint32_t slot_id,
                                                 uint16_t owner_id);

/***************************************************************************//**
 * Invalidate BLOB footer in the requested slot
 *
 * @param[in] slot_id Identifier of slot where BLOB footer shall be invalidated
 * @param[in] force If false then invalidation fails in case of any active
 *   asynchronous delete operation otherwise it is executed unconditionally.
 *
 * @return Result code
 * @retval SL_STATUS_OK In case of success
 * @retval SL_STATUS_BUSY In case of active delete operation (if force is false)
 * @retval SL_STATUS_FAIL In case of internal API error
 * @retval SL_STATUS_FLASH_PROGRAM_FAILED In case storage writing has failed
 ******************************************************************************/
sl_status_t sli_blob_storage_invalidate_slot(uint32_t slot_id,
                                             bool force);

/***************************************************************************//**
 * Counts the number of bits set to 1 in a byte array
 *
 * This function takes a pointer to byte array and the length of the array,
 * and counts the number of bits that are set to 1 in the data. It returns the
 * total count of bits set to 1.
 *
 * @param[in] data Pointer to the byte array
 * @param[in] length Length of the byte array
 * @return The number of bits set to 1 in the data
 ******************************************************************************/
uint32_t sli_blob_storage_count_bits(const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* SLI_BTMESH_BLOB_STORAGE_H */
