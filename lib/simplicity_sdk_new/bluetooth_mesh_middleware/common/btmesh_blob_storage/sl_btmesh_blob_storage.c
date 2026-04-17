/***************************************************************************//**
 * @file
 * @brief Implementation of BLOB storage component
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

// -----------------------------------------------------------------------------
// Includes

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "app_assert.h"

#include "btl_interface.h"
#include "btl_interface_storage.h"

#include "sl_bgapi.h"
#include "sl_common.h"
#include "sl_memory_manager.h"
#include "sl_status.h"

#include "sl_btmesh_blob_storage.h"
#include "sl_btmesh_blob_storage_app_id.h"
#include "sl_btmesh_blob_storage_config.h"
#include "sli_btmesh_blob_storage.h"

// Warning! The app_btmesh_util shall be included after the component configuration
// header file in order to provide the component specific logging macro.
#include "app_btmesh_util.h"

// -----------------------------------------------------------------------------
// Macros

// Align the input value to the following word barrier
#define sli_btmesh_blob_storage_align_to_next_word(_val)   \
  (((_val) + SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL - 1) \
   & (~(SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL - 1)))

// Align the input value to the previous word barrier
#define sli_btmesh_blob_storage_align_to_prev_word(_val) \
  ((_val) & (~(SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL - 1)))

// Calculates the number of bytes over alignment
#define sli_btmesh_blob_storage_word_align_remainder(_val) \
  ((_val) % SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL)

// Calculates the number of bytes remaining until the next aligned value
#define sli_btmesh_blob_storage_word_align_pad_length(_val) \
  (SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL                 \
   - sli_btmesh_blob_storage_word_align_remainder(_val))

// Calculate the maximum size of the BLOB which fits into storage slot
#define sli_calc_max_blob_size(slot_size)                   \
  ((sizeof(sli_btmesh_blob_storage_footer_t) < (slot_size)) \
   ? ((slot_size) - sizeof(sli_btmesh_blob_storage_footer_t)) : 0)

// Check status code and return on error
#define CHECK_SL_STATUS_RETURN(status) \
  do {                                 \
    if ((status) != SL_STATUS_OK) {    \
      return status;                   \
    }                                  \
  } while (0)

// Version of the storage implementation, stored in footer version field
#define STORAGE_VERSION                     2

// Value to be used as padding for application specific footers
#define APP_FOOTER_PADDING_PATTERN          0xAA

// Buffer size for blob_storage_compare
// Warning: The app footer data chunk size determines the size of temporary stack
//          buffer used for comparison or checksum calculation. High values can
//          lead to excess stack usage and stack overflow. Modify it with caution!
#define APP_FOOTER_DATA_CHUNK_SIZE          64

// Invalid app footer identifier
// Note: The erased state of the flash is 0xFF so this adds some protection
#define APP_FOOTER_ID_INVALID               UINT16_MAX

// Invalid app footer length
// Note: The erased state of the flash is 0xFF so this adds some protection
#define APP_FOOTER_LENGTH_INVALID           UINT16_MAX

// Non-existing app footer metadata offset
#define APP_FOOTER_METADATA_OFFSET_NONE     UINT32_MAX

// -----------------------------------------------------------------------------
// Enums, structs, typedefs

/// BLOB Storage API state representation
typedef struct {
  /// Index of the currently written slot (i.e. Slot ID)
  uint32_t current_index;
  /// Number of bytes written during the current write process
  uint32_t written_bytes;
  /// Flag indicating whether padding has already been done
  ///
  /// Padding is used to align written data to needed length. Can occur only once
  /// during writing into one slot.
  bool padding;
  /// Length of the cache
  uint32_t cache_length;
  /// Pointer to the slot data cache
  sl_btmesh_blob_storage_slot_metadata_cache_t *slot_cache;
} sl_btmesh_blob_storage_t;

// -----------------------------------------------------------------------------
// Static function declarations

/*******************************************************************************
 * Check the current status of a slot identified by slot identifier
 *
 * @param[in] slot_id Index of the slot inquired about
 * @param[in] app_footer_check The strictness of the app footer checks
 *
 * @return Status of the slot
 * @retval EMPTY In case the slot doesn't have any data and any footer
 * @retval CORRUPTED In case footer is corrupted, or data is present but footer
 *                   is erased (empty), or footer version mismatch is detected
 * @retval INVALIDATED In case data is present and the footer was invalidated
 *                     by application but the footer is not corrupted
 * @retval OCCUPIED In case data is present and footer is valid
 * @retval INTERNAL_ERROR In case of error in underlying bootloader API
 ******************************************************************************/
static sl_btmesh_blob_storage_status_t blob_storage_check_slot_status(uint32_t slot_id,
                                                                      sli_btmesh_blob_storage_check_t app_footer_check);

/*******************************************************************************
 * Check the validity of the given BLOB storage footer
 *
 * @param[in] footer BLOB storage footer to be checked
 *
 * @return BLOB storage footer to be checked
 * @retval SL_STATUS_OK In case of valid footer
 * @retval SL_STATUS_EMPTY In case of erased footer
 * @retval SL_STATUS_INVALID_SIGNATURE In case of corrupted footer
 * @retval SL_STATUS_INVALID_TYPE In case of footer version mismatch
 ******************************************************************************/
static sl_status_t blob_storage_check_footer(sli_btmesh_blob_storage_footer_t *footer);

/*******************************************************************************
 * Read the footer into the buffer
 *
 * @note The footer need not be valid for the function to return with footer data
 *
 * @param[in] slot_id Index of the slot holding the footer
 * @param[out] footer Buffer for the footer
 *
 * @return Status code
 * @retval SL_STATUS_OK In case of success
 * @retval SL_STATUS_EMPTY In case of erased footer
 * @retval SL_STATUS_FAIL In case of internal bootloader API error
 * @retval SL_STATUS_INVALID_SIGNATURE In case of corrupted footer
 * @retval SL_STATUS_INVALID_TYPE In case of footer version mismatch
 ******************************************************************************/
static sl_status_t blob_storage_get_footer(uint32_t slot_id,
                                           sli_btmesh_blob_storage_footer_t *footer);

/*******************************************************************************
 * Provides the BLOB size from the footer
 *
 * This function doesn't check the validity of the footer so the footer shall
 * exist and it shall be valid when this function is called
 *
 * @note This is important when the cache is not loaded yet at init time
 *
 * @param[in] slot_id Index of the slot holding the footer
 * @param[out] blob_size Buffer for the BLOB size
 * @returns Status of get operation
 * @retval SL_STATUS_OK In case of success
 * @retval SL_STATUS_FAIL In case of internal bootloader API error
 ******************************************************************************/
static sl_status_t blob_storage_get_blob_size_from_footer(uint32_t slot_id, uint32_t *blob_size);

/*******************************************************************************
 * Check the validity of the application footer data in the selected storage slot
 * at the given offset
 *
 * @param[in] slot_id Index of the slot holding the app footer
 * @param[in] footer_offset Offset of the app footer in the storage slot
 * @param[in] length Length of the app footer data
 * @returns Result of the validity check
 * @retval SL_STATUS_OK In case of valid app footer data
 * @retval SL_STATUS_FAIL In case of internal bootloader API error
 * @retval SL_STATUS_INVALID_SIGNATURE In case of invalid app footer data
 ******************************************************************************/
static sl_status_t blob_storage_check_app_footer_data(uint32_t slot_id,
                                                      uint32_t footer_offset,
                                                      uint16_t length);

/*******************************************************************************
 * Check the validity of the application footer metadata
 *
 * @param[in] metadata The metadata to be checked
 * @returns Result of the validity check
 * @retval SL_STATUS_OK In case of valid app footer metadata
 * @retval SL_STATUS_EMPTY In case of erased app footer metadata
 * @retval SL_STATUS_INVALID_SIGNATURE In case of corrupted app footer data
 ******************************************************************************/
static sl_status_t blob_storage_check_app_footer_metadata(sli_btmesh_blob_storage_app_footer_metadata_t *metadata);

/*******************************************************************************
 * Retrieve the content and offset of footer metadata contained in the selected
 * storage slot and having the given application identifier. The offset of the
 * whole app footer and previous app footer metadata is retrieved as well.
 *
 * If the app_id is APP_FOOTER_ID_INVALID, then the returned storage slot
 * metadata_offset points to location where the next application footer metadata
 * shall be written. The footer_offset isn't modified in this case.
 *
 * The check_mode parameter determines the strictness of the checks during
 * app footer iterations.
 * If check_mode is SLI_BTMESH_BLOB_STORAGE_CHECK_STRICT, then app footer metadata
 * is checked and the app footer checksum is calculated over the whole app footer
 * data and compared to the stored checksum.
 * If check_mode is SLI_BTMESH_BLOB_STORAGE_CHECK_RELAXED, then the app footer
 * metadata is checked only and the time consuming checksum calculation is avoided.
 * If check_mode is SLI_BTMESH_BLOB_STORAGE_CHECK_IGNORE, then no checks are performed.
 *
 * @note If the app_id is APP_FOOTER_ID_INVALID and check_mode is
 *   SLI_BTMESH_BLOB_STORAGE_CHECK_STRICT then the function checks each app
 *   footer metadata data and checksum in the storage slot.
 *   The function might return SL_STATUS_NOT_FOUND if there is no app footer
 *   metadata size free space between the blob data and last app footer.
 *
 * @param[in] slot_id Index of the slot holding the app footer
 * @param[in] app_id Identifier of the application data
 * @param[in] check_mode The strictness of the app footer checks
 * @param[out] metadata Buffer for the metadata
 * @param[out] metadata_offset Provides app footer metadata offset in storage slot.
 *   Optional, can be NULL if not needed.
 * @param[out] prev_metadata_offset Provides offset of the previous app footer
 *   metadata in storage slot. If there is no previous metadata, then the value
 *   is set to APP_FOOTER_METADATA_OFFSET_NONE. Optional, can be NULL if not needed.
 * @param[out] footer_offset Offset of the whole app footer in storage slot.
 *   Optional, can be NULL if not needed. If the app_id is APP_FOOTER_ID_INVALID
 *   then it is set to APP_FOOTER_METADATA_OFFSET_NONE.
 *
 * @return Status code
 * @retval SL_STATUS_OK In case of success
 * @retval SL_STATUS_NOT_FOUND If app_id and slot_id doesn't identify any footer
 * @retval SL_STATUS_INVALID_SIGNATURE In case of corrupted app footer metadata
 * @retval SL_STATUS_FAIL In case of internal bootloader API error
 ******************************************************************************/
static sl_status_t blob_storage_get_app_footer_metadata(uint32_t slot_id,
                                                        uint16_t app_id,
                                                        sli_btmesh_blob_storage_check_t check_mode,
                                                        sli_btmesh_blob_storage_app_footer_metadata_t *metadata,
                                                        uint32_t *metadata_offset,
                                                        uint32_t *prev_metadata_offset,
                                                        uint32_t *footer_offset);

/*******************************************************************************
 * Provide the supported maximum BLOB size in the slot
 *
 * @param[in] slot_id Index of the slot inquired about
 *
 * @return Maximum supported BLOB size in the slot
 ******************************************************************************/
static uint32_t blob_storage_get_max_blob_size_in_slot(uint32_t slot_id);

/***************************************************************************//**
 * Compare specific part of a bootloader storage slot with a RAM buffer
 *
 * @param[in] slot_id Identifier of the storage slot to be compared
 * @param[in] slot_offset Offset in the slot where the comparison shall be started
 * @param[in] expected_data Expected data which shall be compared to storage slot data
 * @param[in] length Number of compared bytes
 * @param[out] comparison_result Comparison result shall be interpreted as memcmp
 *
 * @return Status of the comparison
 * @retval SL_STATUS_OK In case of success
 * @retval SL_STATUS_FAIL In case of internal bootloader API error
 ******************************************************************************/
static sl_status_t blob_storage_compare(uint32_t slot_id,
                                        uint32_t slot_offset,
                                        uint8_t const *expected_data,
                                        uint32_t length,
                                        int32_t *comparison_result);

// -----------------------------------------------------------------------------
// Static variables

// Magic pattern to detect BLOB storage footer corruption
static const uint8_t blob_storage_footer_magic[SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT] = {
  0x6F, 0x49, 0xD1, 0x79, 0x4A, 0x8B, 0xE2, 0xC3
};

// BLOB Storage state
sl_btmesh_blob_storage_t blob_storage = { .slot_cache = NULL };

void sl_btmesh_blob_storage_init(void)
{
  BootloaderStorageInformation_t info;
  // Initialize Bootloader
  int32_t bootloader_status = bootloader_init();
  app_assert(bootloader_status == BOOTLOADER_OK,
             "Bootloader initialization failed, error code %ld!",
             bootloader_status);
  // Read storage information from Bootloader
  bootloader_getStorageInfo(&info);

  // Extract number of slots
  blob_storage.cache_length = info.numStorageSlots;

  sl_status_t sc;
  if (NULL == blob_storage.slot_cache) {
    // Allocate memory to store cached data about slots
    sc = sl_memory_calloc(blob_storage.cache_length,
                          sizeof(sl_btmesh_blob_storage_slot_metadata_cache_t),
                          BLOCK_TYPE_LONG_TERM,
                          (void **)&blob_storage.slot_cache);
  } else {
    // Free and reallocate
    // Using free and calloc instead of realloc, because calloc clears memory
    sl_memory_free(blob_storage.slot_cache);
    blob_storage.slot_cache = NULL;
    sc = sl_memory_calloc(blob_storage.cache_length,
                          sizeof(sl_btmesh_blob_storage_slot_metadata_cache_t),
                          BLOCK_TYPE_LONG_TERM,
                          (void **)&blob_storage.slot_cache);
  }

  app_assert(sc == SL_STATUS_OK, "Allocation error!");

  // Set write related status parameters to unknown, i.e. 0xFFFFFFFF
  blob_storage.current_index = UINT32_MAX;
  blob_storage.written_bytes = UINT32_MAX;

  // Sync with strict app footer checks in order to detect errors at initialization
  // time and avoid time consuming checks during normal operation.
  // If the reset is triggered by firmware update and the firmware update required
  // FW reconstruction in the storage slot (from delta path and previous firmware),
  // then the slot footer and app footers might have been erased and overwritten.
  // The strict check ensures that the aforementioned scenario is detected at
  // initialization time and the slot marked as corrupted.
  // Note: It is better to loose an already installed FW image than rejecting
  //       the FW reconstruction because of lack of space in the storage slot
  //       due to footer and app footer data. It doesn't really make sense to
  //       keep a FW image which can't be installed due to lack of space for
  //       FW image reconstruction.
  sli_btmesh_blob_storage_sync(SLI_BTMESH_BLOB_STORAGE_CHECK_STRICT);

  sli_btmesh_blob_storage_erase_init();
}

void sl_btmesh_blob_storage_deinit(void)
{
  // Free allocated memory
  sl_memory_free(blob_storage.slot_cache);
  // Clear status variables
  memset(&blob_storage, 0, sizeof(blob_storage));
}

sl_status_t sl_btmesh_blob_storage_write_start(sl_bt_uuid_64_t const *blob_id,
                                               uint32_t size)
{
  if (sl_btmesh_blob_storage_get_erase_error_code()
      == SL_BTMESH_BLOB_STORAGE_DELETE_BUSY) {
    return SL_STATUS_BUSY;
  }

  sl_status_t ret_val = SL_STATUS_NO_MORE_RESOURCE;

  // Check if BLOB with same ID exists
  if (UINT32_MAX != sli_btmesh_blob_storage_get_slot_id(blob_id)) {
    return SL_STATUS_ALREADY_EXISTS;
  }

  BootloaderStorageSlot_t slot;
  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    if (SL_BTMESH_BLOB_STORAGE_STATUS_EMPTY
        == blob_storage.slot_cache[slot_id].status) {
      bootloader_getStorageSlotInfo(slot_id, &slot);
      // Check if footer and data would fit
      if (size > sli_calc_max_blob_size(slot.length)) {
        ret_val = SL_STATUS_WOULD_OVERFLOW;
        continue;
      }
      // Copy BLOB ID into cache
      memcpy(&blob_storage.slot_cache[slot_id].blob_id,
             blob_id,
             sizeof(sl_bt_uuid_64_t));
      // Store BLOB size
      blob_storage.slot_cache[slot_id].blob_size = size;
      // Initialize write variables
      blob_storage.written_bytes = 0;
      blob_storage.current_index = slot_id;
      blob_storage.padding = false;
      ret_val = SL_STATUS_OK;
      log_debug("BLOB Storage write start; slot %lu, address 0x%08lX, "
                "length 0x%08lX" NL,
                blob_storage.current_index,
                slot.address,
                slot.length);
      break;
    }
  }
  if (ret_val != SL_STATUS_OK) {
    log_warning("BLOB Storage write start, empty slot not available"NL);
  }

  return ret_val;
}

sl_status_t sl_btmesh_blob_storage_write(uint32_t offset,
                                         uint32_t len,
                                         void *data)
{
  int32_t r; // Bootloader result

  log_debug("BLOB Storage write; offset 0x%08lX, "
            "length %lu cache status %d" NL,
            offset,
            len, blob_storage.slot_cache[blob_storage.current_index].status);
  // Write data into flash
  r = bootloader_writeStorage(blob_storage.current_index,
                              offset,
                              data,
                              // Truncate length to alignment (if needed)
                              sli_btmesh_blob_storage_align_to_prev_word(len));
  if (BOOTLOADER_OK != r) {
    goto write_failed;
  }
  // Check whether length had to be truncated
  if (0 != sli_btmesh_blob_storage_word_align_remainder(len)) {
    uint8_t buffer[SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL];
    // Padding is only done once at the end of a BLOB. If already padded,
    // inputs are wrong.
    app_assert_s(!blob_storage.padding);
    // Indicate padding
    blob_storage.padding = true;
    // Fill buffer with padding
    memset(buffer, UINT8_MAX, SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL);
    // Copy remainder data into buffer
    memcpy(buffer,
           &((uint8_t *)data)[sli_btmesh_blob_storage_align_to_prev_word(len)],
           sli_btmesh_blob_storage_word_align_remainder(len));
    // Write (padded) data into flash
    r = bootloader_writeStorage(blob_storage.current_index,
                                offset
                                + sli_btmesh_blob_storage_align_to_prev_word(len),
                                buffer,
                                SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL);
    if (BOOTLOADER_OK != r) {
      goto write_failed;
    }
  }
  blob_storage.written_bytes += len;
  blob_storage.slot_cache[blob_storage.current_index].status = SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED;
  return SL_STATUS_OK;

  write_failed:
  // If the write failed due to non-erased storage, check if the content matches
  // what is already in the storage. If it does, the write is considered successful.
  if (BOOTLOADER_ERROR_STORAGE_NEEDS_ERASE == r) {
    int32_t diff = 1;
    (void)blob_storage_compare(blob_storage.current_index,
                               offset,
                               data,
                               len,
                               &diff);
    if (diff == 0) {
      return SL_STATUS_OK;
    }
  }
  log_error("BLOB Storage write failed; offset 0x%08lX, length %lu, error 0x%04lX" NL,
            offset, len, r);
  return SL_STATUS_FLASH_PROGRAM_FAILED;
}

void sl_btmesh_blob_storage_get_cache(sl_btmesh_blob_storage_slot_metadata_cache_t const **cache,
                                      uint32_t *len)
{
  if (cache != NULL) {
    *cache = blob_storage.slot_cache;
  }
  if (len != NULL) {
    *len = blob_storage.cache_length;
  }
}

bool sl_btmesh_blob_storage_any_blob_corrupted(void)
{
  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    if (blob_storage.slot_cache[slot_id].status == SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED) {
      return true;
    }
  }
  return false;
}

sl_status_t sl_btmesh_blob_storage_verify(void)
{
  if (UINT32_MAX == blob_storage.current_index) {
    return SL_STATUS_INVALID_STATE;
  }
  // Check whether written length equals the received size of BLOB
  if (blob_storage.written_bytes
      != blob_storage.slot_cache[blob_storage.current_index].blob_size) {
    return SL_STATUS_INVALID_STATE;
  }

  sli_btmesh_blob_storage_footer_t footer;
  memset(&footer, UINT8_MAX, sizeof(sli_btmesh_blob_storage_footer_t));

  // Copy BLOB ID into footer
  memcpy(&footer.blob_id,
         &blob_storage.slot_cache[blob_storage.current_index].blob_id,
         sizeof(sl_bt_uuid_64_t));

  footer.blob_size =
    blob_storage.slot_cache[blob_storage.current_index].blob_size;
  footer.version = STORAGE_VERSION;

  // Set inverted fields
  for (uint8_t idx = 0; idx < sizeof(sl_bt_uuid_64_t); idx++) {
    footer.blob_id_inv.data[idx] = ~footer.blob_id.data[idx];
  }
  footer.blob_size_inv = ~footer.blob_size;
  footer.version_inv = ~footer.version;

  // Set magic bytes
  memcpy(footer.magic, blob_storage_footer_magic, sizeof(footer.magic));

  BootloaderStorageSlot_t slot;
  if (BOOTLOADER_OK
      != bootloader_getStorageSlotInfo(blob_storage.current_index, &slot)) {
    return SL_STATUS_FAIL;
  }

  if (BOOTLOADER_OK != bootloader_writeStorage(blob_storage.current_index,
                                               slot.length - sizeof(sli_btmesh_blob_storage_footer_t),
                                               (uint8_t*)&footer,
                                               sizeof(sli_btmesh_blob_storage_footer_t)) ) {
    return SL_STATUS_FLASH_PROGRAM_FAILED;
  }
  // Resync without app footer checks because no app footer belongs to just written BLOB
  sli_btmesh_blob_storage_sync(SLI_BTMESH_BLOB_STORAGE_CHECK_IGNORE);
  log_debug("BLOB Storage write verified status: %d"NL, blob_storage.slot_cache[blob_storage.current_index].status);
  // Clear status
  blob_storage.current_index = UINT32_MAX;
  blob_storage.written_bytes = UINT32_MAX;
  return SL_STATUS_OK;
}

bool sl_btmesh_blob_storage_is_present(sl_bt_uuid_64_t const *blob_id)
{
  return (UINT32_MAX != sli_btmesh_blob_storage_get_slot_id(blob_id));
}

sl_status_t sl_btmesh_blob_storage_get_blob_size(sl_bt_uuid_64_t const *blob_id,
                                                 uint32_t *blob_size)
{
  uint32_t slot_id = sli_btmesh_blob_storage_get_slot_id(blob_id);
  if (UINT32_MAX == slot_id) {
    return SL_STATUS_NOT_FOUND;
  }

  *blob_size = blob_storage.slot_cache[slot_id].blob_size;

  return SL_STATUS_OK;
}

sl_status_t sl_btmesh_blob_storage_read(sl_bt_uuid_64_t const *blob_id,
                                        uint32_t offset,
                                        uint32_t *len,
                                        void *buffer)
{
  uint32_t slot_id = sli_btmesh_blob_storage_get_slot_id(blob_id);
  int32_t r; // Bootloader result

  if (slot_id == UINT32_MAX) {
    return SL_STATUS_NOT_FOUND;
  }

  // If read operation would read over BLOB data, truncate length
  if (blob_storage.slot_cache[slot_id].blob_size < offset + *len) {
    *len = blob_storage.slot_cache[slot_id].blob_size - offset;
  }

  r = bootloader_readStorage(slot_id, offset, buffer, *len);

  switch (r) {
    case BOOTLOADER_OK:
      return SL_STATUS_OK;
    case BOOTLOADER_ERROR_STORAGE_INVALID_SLOT:
      return SL_STATUS_NOT_FOUND;
    default:
      return SL_STATUS_FAIL;
  }
}

sl_status_t sl_btmesh_blob_storage_invalidate(sl_bt_uuid_64_t const *blob_id)
{
  uint32_t slot_id;
  sl_status_t sc;
  slot_id = sli_btmesh_blob_storage_get_slot_id(blob_id);

  if (UINT32_MAX == slot_id) {
    return SL_STATUS_NOT_FOUND;
  }

  sc = sli_blob_storage_invalidate_slot(slot_id, false);

  return sc;
}

sl_status_t sl_btmesh_blob_storage_invalidate_by_owner(uint16_t owner_id)
{
  sl_btmesh_blob_storage_status_t slot_status;
  sl_status_t sc_all = SL_STATUS_OK;

  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    slot_status = blob_storage.slot_cache[slot_id].status;
    if (SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED != slot_status) {
      continue;
    }
    sl_bt_uuid_64_t *blob_id = &blob_storage.slot_cache[slot_id].blob_id;
    // Invalidate only those BLOBs which belongs to the specific owner
    if (!(sl_btmesh_blob_storage_is_managed_by_owner(blob_id, owner_id))) {
      continue;
    }
    sl_status_t sc = sli_blob_storage_invalidate_slot(slot_id, false);
    // The first error code is returned but all storage slot invalidation
    // is requested because other slot invalidation might succeed
    if ((SL_STATUS_OK != sc) && (SL_STATUS_OK == sc_all)) {
      sc_all = sc;
    }
  }
  return sc_all;
}

sl_status_t sl_btmesh_blob_storage_invalidate_all(void)
{
  sl_btmesh_blob_storage_status_t slot_status;
  sl_status_t sc_all = SL_STATUS_OK;

  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    slot_status = blob_storage.slot_cache[slot_id].status;
    if (SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED == slot_status) {
      sl_status_t sc = sli_blob_storage_invalidate_slot(slot_id, false);
      // The first error code is returned but all storage slot invalidation
      // is requested because other slot invalidation might succeed
      if ((SL_STATUS_OK != sc) && (SL_STATUS_OK == sc_all)) {
        sc_all = sc;
      }
    }
  }
  return sc_all;
}

uint32_t sl_btmesh_blob_storage_calc_app_footer_size(uint16_t footer_data_length)
{
  // The size of sli_btmesh_blob_storage_app_footer_metadata_t and
  // sli_btmesh_blob_storage_app_footer_checksum_t structures
  // are aligned to SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT bytes
  uint32_t footer_data_padded_length = sli_btmesh_blob_storage_align_to_next_word(footer_data_length);
  return sizeof(sli_btmesh_blob_storage_app_footer_metadata_t)
         + footer_data_padded_length
         + sizeof(sli_btmesh_blob_storage_app_footer_checksum_t);
}

sl_status_t sl_btmesh_blob_storage_write_app_footer(sl_bt_uuid_64_t const * blob_id,
                                                    void const * footer,
                                                    uint16_t app_id,
                                                    uint16_t length)
{
  BootloaderStorageSlot_t slot;
  sli_btmesh_blob_storage_app_footer_metadata_t metadata;
  sli_btmesh_blob_storage_app_footer_checksum_t metadata_checksum;
  uint32_t slot_id, metadata_offset, prev_metadata_offset, footer_offset, checksum, write_length;
  sl_status_t sc;
  const uint16_t remainder_length = sli_btmesh_blob_storage_word_align_remainder(length);
  const uint16_t aligned_length = sli_btmesh_blob_storage_align_to_prev_word(length);

  // Find storage slot which contains BLOB with passed blob_id
  slot_id = sli_btmesh_blob_storage_get_slot_id(blob_id);
  if (slot_id == UINT32_MAX) {
    return SL_STATUS_NOT_FOUND;
  }

  if ((app_id == APP_FOOTER_ID_INVALID) || (length == APP_FOOTER_LENGTH_INVALID)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (BOOTLOADER_OK != bootloader_getStorageSlotInfo(slot_id, &slot)) {
    return SL_STATUS_FAIL;
  }

  // Find the offset where the metadata shall be written
  sc = blob_storage_get_app_footer_metadata(slot_id,
                                            APP_FOOTER_ID_INVALID,
                                            SLI_BTMESH_BLOB_STORAGE_CHECK_RELAXED,
                                            &metadata,
                                            &metadata_offset,
                                            &prev_metadata_offset,
                                            NULL);
  CHECK_SL_STATUS_RETURN(sc);

  // Calculate full length of data to be written
  write_length = sl_btmesh_blob_storage_calc_app_footer_size(length);

  // Make sure subtraction of write_length doesn't wrap around
  if (metadata_offset + sizeof(sli_btmesh_blob_storage_app_footer_metadata_t)
      < write_length) {
    return SL_STATUS_FULL;
  }

  // Calculate offset relative to the start of the empty footer metadata slot
  footer_offset = metadata_offset
                  + sizeof(sli_btmesh_blob_storage_app_footer_metadata_t)
                  - write_length;

  // If write start offset is inside BLOB
  if (footer_offset < blob_storage.slot_cache[slot_id].blob_size) {
    return SL_STATUS_FULL;
  }

  // Write the next field of previous metadata to point to the new app footer
  // metadata if there is any
  if (prev_metadata_offset != APP_FOOTER_METADATA_OFFSET_NONE) {
    sli_btmesh_blob_storage_app_footer_next_t next = {
      .offset = metadata_offset,
      .offset_inv = ~metadata_offset
    };
    uint32_t prev_metadata_next_offset =
      prev_metadata_offset + offsetof(sli_btmesh_blob_storage_app_footer_metadata_t, next);
    if (BOOTLOADER_OK != bootloader_writeStorage(slot_id,
                                                 prev_metadata_next_offset,
                                                 (uint8_t *) &next,
                                                 sizeof(next))) {
      return SL_STATUS_FLASH_PROGRAM_FAILED;
    }
  }

  // Calculate checksum over app footer data
  checksum = sl_btmesh_blob_storage_calculate_checksum((const uint8_t *) footer, length, 0);

  // Assemble footer metadata
  metadata.app_id = app_id;
  metadata.length = length;
  metadata.app_id_inv = ~metadata.app_id;
  metadata.length_inv = ~metadata.length;
  metadata.next.offset = APP_FOOTER_METADATA_OFFSET_NONE;
  metadata.next.offset_inv = APP_FOOTER_METADATA_OFFSET_NONE;
  metadata_checksum.checksum = checksum;
  metadata_checksum.checksum_inv = ~metadata_checksum.checksum;

  // Write metadata checksum first to the lower offset
  // This can be important when the app footer is written through flash page
  // boundary because the checksum and its inverse can detect with high confidence
  // when the lower offset flash page is erased only.
  // The app footer checksum is guaranteed to be integer multiple of
  // SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT so it can be written in one
  // step without the footer data.
  if (BOOTLOADER_OK != bootloader_writeStorage(slot_id,
                                               footer_offset,
                                               (uint8_t *) &metadata_checksum,
                                               sizeof(metadata_checksum))) {
    return SL_STATUS_FLASH_PROGRAM_FAILED;
  }
  footer_offset += sizeof(metadata_checksum);

  // Write the aligned part of app footer data
  // It the footer is aligned to 4 bytes then bootloader can use DMA to write
  // data into the flash but this is no longer mandatory.
  // Note: It doesn't make sense to use local aligned temp buffer because it leads
  // to additional copying and increased stack usage. If the temp buffer is small
  // then it requires multiple bootloader_writeStorage calls which diminishes
  // any runtime gain from DMA transfer.
  if (aligned_length != 0) {
    if (BOOTLOADER_OK != bootloader_writeStorage(slot_id,
                                                 footer_offset,
                                                 (uint8_t *) footer,
                                                 aligned_length)) {
      return SL_STATUS_FLASH_PROGRAM_FAILED;
    }
    footer_offset += aligned_length;
  }

  if (remainder_length != 0) {
    // Buffer to be used for padding the data, so no allocation is required
    uint8_t buffer[SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL];

    // Initialize padded buffer
    memset(buffer,
           APP_FOOTER_PADDING_PATTERN,
           SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL);

    // Copy remainder data into padded buffer
    memcpy(buffer,
           ((const uint8_t *)footer) + aligned_length,
           remainder_length);

    // // Write the remainder part of app footer data
    if (BOOTLOADER_OK != bootloader_writeStorage(slot_id,
                                                 footer_offset,
                                                 buffer,
                                                 SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL)) {
      return SL_STATUS_FLASH_PROGRAM_FAILED;
    }
  }

  // Write app footer metadata
  if (BOOTLOADER_OK != bootloader_writeStorage(slot_id,
                                               metadata_offset,
                                               (uint8_t *) &metadata,
                                               sizeof(sli_btmesh_blob_storage_app_footer_metadata_t))) {
    return SL_STATUS_FLASH_PROGRAM_FAILED;
  }

  return SL_STATUS_OK;
}

sl_status_t sl_btmesh_blob_storage_set_managed(sl_bt_uuid_64_t const *blob_id,
                                               uint16_t owner_id)
{
  return sl_btmesh_blob_storage_write_app_footer(blob_id,
                                                 &owner_id,
                                                 BLOB_STORAGE_APP_ID_MANAGED_FLAG,
                                                 sizeof(owner_id));
}

bool sl_btmesh_blob_storage_is_managed(sl_bt_uuid_64_t const *blob_id)
{
  uint16_t owner_id;
  uint16_t length = sizeof(owner_id);

  sl_status_t sc = sl_btmesh_blob_storage_get_app_footer(blob_id,
                                                         BLOB_STORAGE_APP_ID_MANAGED_FLAG,
                                                         &owner_id,
                                                         &length);
  (void) length;
  if (SL_STATUS_OK == sc) {
    return true;
  }
  return false;
}

bool sli_btmesh_blob_storage_is_managed(uint32_t slot_id)
{
  uint16_t owner_id;
  uint16_t length = sizeof(owner_id);

  sl_status_t sc = sli_btmesh_blob_storage_get_app_footer(slot_id,
                                                          BLOB_STORAGE_APP_ID_MANAGED_FLAG,
                                                          &owner_id,
                                                          &length);
  (void) length;
  if (SL_STATUS_OK == sc) {
    return true;
  }
  return false;
}

bool sl_btmesh_blob_storage_is_managed_by_owner(sl_bt_uuid_64_t const *blob_id,
                                                uint16_t owner_id)
{
  uint16_t blob_owner_id;
  uint16_t length = sizeof(blob_owner_id);
  sl_status_t sc = sl_btmesh_blob_storage_get_app_footer(blob_id,
                                                         BLOB_STORAGE_APP_ID_MANAGED_FLAG,
                                                         &blob_owner_id,
                                                         &length);

  if ((SL_STATUS_OK == sc) && (length == sizeof(blob_owner_id))) {
    return (blob_owner_id == owner_id);
  }
  return false;
}

bool sli_btmesh_blob_storage_is_managed_by_owner(uint32_t slot_id,
                                                 uint16_t owner_id)
{
  uint16_t blob_owner_id;
  uint16_t length = sizeof(blob_owner_id);
  sl_status_t sc = sli_btmesh_blob_storage_get_app_footer(slot_id,
                                                          BLOB_STORAGE_APP_ID_MANAGED_FLAG,
                                                          &blob_owner_id,
                                                          &length);

  if ((SL_STATUS_OK == sc) && (length == sizeof(blob_owner_id))) {
    return (blob_owner_id == owner_id);
  }
  return false;
}

uint32_t sl_btmesh_blob_storage_get_managed_flag_size(void)
{
  return sl_btmesh_blob_storage_calc_app_footer_size(sizeof(uint16_t));
}

sl_status_t sl_btmesh_blob_storage_get_blob_id_by_owner(uint16_t owner_id,
                                                        uint32_t occurrence_idx,
                                                        sl_bt_uuid_64_t *blob_id)
{
  sl_status_t sc = SL_STATUS_NOT_FOUND;
  uint32_t current_occurrence_idx = 0;
  uint32_t cache_length = blob_storage.cache_length;

  for (uint32_t slot_id = 0; slot_id < cache_length; ++slot_id) {
    sl_btmesh_blob_storage_status_t slot_status =
      blob_storage.slot_cache[slot_id].status;
    if (SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED == slot_status
        && sli_btmesh_blob_storage_is_managed_by_owner(slot_id, owner_id)) {
      if (occurrence_idx <= current_occurrence_idx) {
        sc = SL_STATUS_OK;
        memcpy(blob_id,
               &blob_storage.slot_cache[slot_id].blob_id,
               sizeof(sl_bt_uuid_64_t));
        break;
      } else {
        current_occurrence_idx++;
      }
    }
  }
  return sc;
}

sl_status_t sl_btmesh_blob_storage_get_app_footer_length(sl_bt_uuid_64_t const *blob_id,
                                                         uint16_t app_id,
                                                         uint16_t *length)
{
  sli_btmesh_blob_storage_app_footer_metadata_t metadata;
  uint32_t slot_id, metadata_offset;
  sl_status_t sc;

  // Retrieve slot ID belonging to BLOB
  slot_id = sli_btmesh_blob_storage_get_slot_id(blob_id);
  sc = blob_storage_get_app_footer_metadata(slot_id,
                                            app_id,
                                            SLI_BTMESH_BLOB_STORAGE_CHECK_RELAXED,
                                            &metadata,
                                            &metadata_offset,
                                            NULL,
                                            NULL);
  if (sc == SL_STATUS_OK) {
    *length = metadata.length;
  }
  return sc;
}

sl_status_t sl_btmesh_blob_storage_get_app_footer(sl_bt_uuid_64_t const *blob_id,
                                                  uint16_t app_id,
                                                  void *buffer,
                                                  uint16_t *length)
{
  uint32_t slot_id;
  // Retrieve slot ID belonging to BLOB
  slot_id = sli_btmesh_blob_storage_get_slot_id(blob_id);
  return sli_btmesh_blob_storage_get_app_footer(slot_id, app_id, buffer, length);
}

sl_status_t sli_btmesh_blob_storage_get_app_footer(uint32_t slot_id,
                                                   uint16_t app_id,
                                                   void *buffer,
                                                   uint16_t *length)
{
  sli_btmesh_blob_storage_app_footer_metadata_t metadata;
  uint32_t offset, data_offset;
  sl_status_t sc;
  sc = blob_storage_get_app_footer_metadata(slot_id,
                                            app_id,
                                            SLI_BTMESH_BLOB_STORAGE_CHECK_RELAXED,
                                            &metadata,
                                            NULL,
                                            NULL,
                                            &offset);
  if (SL_STATUS_OK != sc) {
    return sc;
  }
  // Buffer is too short for this footer
  if (metadata.length > *length) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  *length = metadata.length;
  data_offset = offset + sizeof(sli_btmesh_blob_storage_app_footer_checksum_t);
  if (BOOTLOADER_OK
      != bootloader_readStorage(slot_id, data_offset, buffer, *length)) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t sl_btmesh_blob_storage_get_blob_id_by_footer(uint16_t app_id,
                                                         const void *expected_footer,
                                                         uint16_t length,
                                                         sl_bt_uuid_64_t *blob_id)
{
  sl_status_t sc;
  sli_btmesh_blob_storage_app_footer_metadata_t metadata;
  uint32_t offset;

  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    if (SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED
        == blob_storage.slot_cache[slot_id].status) {
      sc = blob_storage_get_app_footer_metadata(slot_id,
                                                app_id,
                                                SLI_BTMESH_BLOB_STORAGE_CHECK_RELAXED,
                                                &metadata,
                                                NULL,
                                                NULL,
                                                &offset);

      // Nothing is found or the length of the footer is different then expected
      // continue with the next slot
      if ((SL_STATUS_NOT_FOUND == sc) || (metadata.length != length)) {
        continue;
      } else if (SL_STATUS_OK != sc) {
        // Return with error code in case other error occurs
        return sc;
      }

      int32_t comparison_result;

      // The app footer data begins after the checksum
      offset += sizeof(sli_btmesh_blob_storage_app_footer_checksum_t);

      sc = blob_storage_compare(slot_id,
                                offset,
                                expected_footer,
                                length,
                                &comparison_result);
      if (SL_STATUS_OK != sc) {
        // If comparison operation failed, return error code
        return sc;
      }

      if (0 == comparison_result) {
        memcpy(blob_id,
               &blob_storage.slot_cache[slot_id].blob_id,
               sizeof(sl_bt_uuid_64_t));
        return SL_STATUS_OK;
      }
    }
  }
  return SL_STATUS_NOT_FOUND;
}

uint32_t sl_btmesh_blob_storage_get_total_space(void)
{
  uint32_t total_space = 0;

  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    total_space += blob_storage_get_max_blob_size_in_slot(slot_id);
  }
  return total_space;
}

uint32_t sl_btmesh_blob_storage_get_remaining_space(bool include_unmanaged_blobs)
{
  sl_btmesh_blob_storage_status_t slot_status;
  uint32_t max_blob_size_in_slot;
  uint32_t remaining_space = 0;

  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    slot_status = blob_storage.slot_cache[slot_id].status;
    bool is_slot_available = false;

    if (sli_btmesh_blob_storage_is_slot_unoccupied(slot_id)) {
      // If the slot is empty or corrupted or invalidated then it can be used to
      // store BLOBs without any restrictions
      is_slot_available = true;
    } else if (SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED == slot_status) {
      // The unmanaged BLOBs can be deleted on demand when space is required
      // for new BLOBs which makes it necessary to include it in the remaining
      // space calculation in some cases (application dependent)
      is_slot_available = include_unmanaged_blobs
                          && !sli_btmesh_blob_storage_is_managed(slot_id);
    }

    if (is_slot_available) {
      max_blob_size_in_slot = blob_storage_get_max_blob_size_in_slot(slot_id);
      remaining_space += max_blob_size_in_slot;
    }
  }
  return remaining_space;
}

uint32_t sl_btmesh_blob_storage_get_max_blob_size(void)
{
  uint32_t max_blob_size = 0;
  uint32_t max_blob_size_in_slot;

  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    max_blob_size_in_slot = blob_storage_get_max_blob_size_in_slot(slot_id);
    if (max_blob_size < max_blob_size_in_slot) {
      max_blob_size = max_blob_size_in_slot;
    }
  }
  return max_blob_size;
}

uint32_t sl_btmesh_blob_storage_get_max_blob_size_free(bool include_unmanaged_blobs)
{
  sl_btmesh_blob_storage_status_t slot_status;
  uint32_t max_blob_size_in_slot;
  uint32_t max_blob_size_free = 0;

  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    slot_status = sli_btmesh_blob_storage_get_slot_status(slot_id);
    bool is_slot_available = false;

    if (sli_btmesh_blob_storage_is_slot_unoccupied(slot_id)) {
      // If the slot is empty or corrupted or invalidated then it can be used
      // to store BLOBs without any restrictions
      is_slot_available = true;
    } else if (SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED == slot_status) {
      // The unmanaged BLOBs can be deleted on demand when space is required
      // for new BLOBs which makes it necessary to include it in the maximum
      // free BLOB size calculation in some cases (application dependent)
      is_slot_available = include_unmanaged_blobs
                          && !sli_btmesh_blob_storage_is_managed(slot_id);
    }

    if (is_slot_available) {
      max_blob_size_in_slot = blob_storage_get_max_blob_size_in_slot(slot_id);
      if (max_blob_size_free < max_blob_size_in_slot) {
        max_blob_size_free = max_blob_size_in_slot;
      }
    }
  }
  return max_blob_size_free;
}

uint32_t sl_btmesh_blob_storage_get_max_blob_count(void)
{
  return blob_storage.cache_length;
}

uint8_t sl_btmesh_blob_storage_get_alignment(void)
{
  return SL_BTMESH_BLOB_STORAGE_ALIGNMENT_CFG_VAL;
}

sl_btmesh_blob_storage_status_t sli_btmesh_blob_storage_get_slot_status(uint32_t slot_id)
{
  return blob_storage.slot_cache[slot_id].status;
}

bool sli_btmesh_blob_storage_is_slot_occupied(uint32_t slot_id)
{
  sl_btmesh_blob_storage_status_t slot_status;
  slot_status = sli_btmesh_blob_storage_get_slot_status(slot_id);
  return (slot_status == SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED);
}

bool sli_btmesh_blob_storage_is_slot_unoccupied(uint32_t slot_id)
{
  sl_btmesh_blob_storage_status_t slot_status;
  slot_status = sli_btmesh_blob_storage_get_slot_status(slot_id);
  return ((slot_status == SL_BTMESH_BLOB_STORAGE_STATUS_EMPTY)
          || (slot_status == SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED)
          || (slot_status == SL_BTMESH_BLOB_STORAGE_STATUS_INVALIDATED));
}

bool sli_btmesh_blob_storage_has_slot_garbage(uint32_t slot_id)
{
  sl_btmesh_blob_storage_status_t slot_status;
  slot_status = sli_btmesh_blob_storage_get_slot_status(slot_id);
  return ((slot_status == SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED)
          || (slot_status == SL_BTMESH_BLOB_STORAGE_STATUS_INVALIDATED));
}

void sli_btmesh_blob_storage_sync(sli_btmesh_blob_storage_check_t app_footer_check)
{
  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    blob_storage.slot_cache[slot_id].status = blob_storage_check_slot_status(slot_id,
                                                                             app_footer_check);
    if (blob_storage.slot_cache[slot_id].status == SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED) {
      sli_btmesh_blob_storage_footer_t footer;
      sl_status_t sc = blob_storage_get_footer(slot_id, &footer);
      if (SL_STATUS_OK == sc) {
        memcpy(&blob_storage.slot_cache[slot_id].blob_id,
               &footer.blob_id,
               sizeof(sl_bt_uuid_64_t));
        blob_storage.slot_cache[slot_id].blob_size = footer.blob_size;
      } else {
        // This should not happen because this code is executed only if the same
        // blob_storage_get_footer call was successful in blob_storage_check_slot_status
        // function but to be on the safe side it is handled here.
        blob_storage.slot_cache[slot_id].status = SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED;
      }
    }
  }
}

static sl_status_t blob_storage_check_footer(sli_btmesh_blob_storage_footer_t *footer)
{
  app_assert_s(footer != NULL);

  // Check if the footer is erased
  if (footer->blob_size == footer->blob_size_inv) {
    // If the blob size and its inverse are equal then the whole footer shall
    // be erased (0xFF bytes) including the blob size otherwise the footer is
    // considered as corrupted
    uint8_t *footer_ptr = (uint8_t *)footer;
    for (uint8_t idx = 0; idx < sizeof(sli_btmesh_blob_storage_footer_t); idx++) {
      if (footer_ptr[idx] != UINT8_MAX) {
        return SL_STATUS_INVALID_SIGNATURE;
      }
    }
    return SL_STATUS_EMPTY;
  }

  if (memcmp(footer->magic, blob_storage_footer_magic, sizeof(footer->magic))) {
    return SL_STATUS_INVALID_SIGNATURE;
  }

  for (uint8_t idx = 0; idx < sizeof(sl_bt_uuid_64_t); idx++) {
    uint8_t expected_blob_id_data = ~footer->blob_id_inv.data[idx];
    if (footer->blob_id.data[idx] != expected_blob_id_data) {
      return SL_STATUS_INVALID_SIGNATURE;
    }
  }

  uint8_t expected_version = ~footer->version_inv;
  if ((footer->blob_size != ~footer->blob_size_inv)
      || (footer->version != expected_version)) {
    return SL_STATUS_INVALID_SIGNATURE;
  }

  if (footer->version != STORAGE_VERSION) {
    return SL_STATUS_INVALID_TYPE;
  }

  return SL_STATUS_OK;
}

static sl_status_t blob_storage_get_footer(uint32_t slot_id,
                                           sli_btmesh_blob_storage_footer_t *footer)
{
  BootloaderStorageSlot_t slot;
  if (BOOTLOADER_OK != bootloader_getStorageSlotInfo(slot_id, &slot)) {
    return SL_STATUS_FAIL;
  }

  // Read footer from the end of slot
  if (BOOTLOADER_OK
      != bootloader_readStorage(slot_id,
                                slot.length - sizeof(sli_btmesh_blob_storage_footer_t),
                                (uint8_t*)footer,
                                sizeof(sli_btmesh_blob_storage_footer_t))) {
    return SL_STATUS_FAIL;
  }

  return blob_storage_check_footer(footer);
}

static sl_status_t blob_storage_get_blob_size_from_footer(uint32_t slot_id, uint32_t *blob_size)
{
  BootloaderStorageSlot_t slot;

  app_assert_s(blob_size != NULL);

  if (BOOTLOADER_OK != bootloader_getStorageSlotInfo(slot_id, &slot)) {
    return SL_STATUS_FAIL;
  }

  uint32_t offset = slot.length
                    - sizeof(sli_btmesh_blob_storage_footer_t)
                    + offsetof(sli_btmesh_blob_storage_footer_t, blob_size);

  // Read footer from the end of slot
  if (BOOTLOADER_OK
      != bootloader_readStorage(slot_id,
                                offset,
                                (uint8_t *) blob_size,
                                sizeof(blob_size))) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

static sl_btmesh_blob_storage_status_t blob_storage_check_slot_status(uint32_t slot_id,
                                                                      sli_btmesh_blob_storage_check_t app_footer_check)
{
  sl_btmesh_blob_storage_status_t ret_val;
  BootloaderStorageSlot_t slot;
  sli_btmesh_blob_storage_footer_t footer;
  sl_status_t footer_status;

  if (BOOTLOADER_OK != bootloader_getStorageSlotInfo(slot_id, &slot)) {
    return SL_BTMESH_BLOB_STORAGE_STATUS_INTERNAL_ERROR;
  }

  // Read footer associated with slot
  footer_status = blob_storage_get_footer(slot_id, &footer);
  switch (footer_status) {
    case SL_STATUS_EMPTY:
      // Footer is erased (empty)
      ret_val = SL_BTMESH_BLOB_STORAGE_STATUS_EMPTY;
      break;
    case SL_STATUS_OK:
      // Footer is valid
      ret_val = SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED;
      break;
    case SL_STATUS_FAIL:
      // Low-level error during reading the footer
      return SL_BTMESH_BLOB_STORAGE_STATUS_INTERNAL_ERROR;
    case SL_STATUS_INVALID_SIGNATURE:
    case SL_STATUS_INVALID_TYPE:
    default:
      // Corrupted footer or version mismatch
      return SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED;
  }

  // Check validity flag. It's enough to check the first byte, since it's
  // written as a whole. If the footer is erased then the validity flag is
  // UINT8_MAX so it isn't necessary to distinguish between erased and valid
  // footer in this case.
  if (footer.validity_flag[0] != UINT8_MAX) {
    return SL_BTMESH_BLOB_STORAGE_STATUS_INVALIDATED;
  }

  if (footer_status == SL_STATUS_OK
      && app_footer_check != SLI_BTMESH_BLOB_STORAGE_CHECK_IGNORE) {
    sli_btmesh_blob_storage_app_footer_metadata_t metadata;
    sl_status_t app_footer_status = blob_storage_get_app_footer_metadata(slot_id,
                                                                         APP_FOOTER_ID_INVALID,
                                                                         app_footer_check,
                                                                         &metadata,
                                                                         NULL,
                                                                         NULL,
                                                                         NULL);
    switch (app_footer_status) {
      case SL_STATUS_FAIL:
        // Low-level error during reading the footer
        return SL_BTMESH_BLOB_STORAGE_STATUS_INTERNAL_ERROR;
      case SL_STATUS_INVALID_SIGNATURE:
        // Corrupted app footer or version mismatch
        return SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED;
      case SL_STATUS_NOT_FOUND:
        // There is no free space between BLOB data and the last app footer.
        // This is a valid scenario when the last app footer barely fits.
        break;
      default:
        break;
    }
  }

  // Check if non-footer data is empty.
  // Since checking every byte would take long, use a heuristic approach of
  // checking only certain bytes.
  for (uint32_t idx = 0;
       idx < (slot.length - sizeof(sli_btmesh_blob_storage_footer_t));
       idx += SL_BTMESH_BLOB_STORAGE_DATA_CHECK_JUMP_CFG_VAL) {
    uint8_t check;
    if (BOOTLOADER_OK != bootloader_readStorage(slot_id, idx, &check, sizeof(check))) {
      return SL_BTMESH_BLOB_STORAGE_STATUS_INTERNAL_ERROR;
    }
    if (UINT8_MAX != check) {
      ret_val = SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED;
      break;
    }
  }

  if (ret_val == SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED
      && footer_status == SL_STATUS_EMPTY) {
    // If slot is occupied but footer is erased then the slot is corrupted
    ret_val = SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED;
  }

  // If the slot data bytes below the footer are erased (empty) and the storage
  // slot footer is valid then the slot is considered as occupied because it is
  // possible that the data contains 0xFF bytes only.
  return ret_val;
}

uint32_t sli_btmesh_blob_storage_get_slot_id(sl_bt_uuid_64_t const *blob_id)
{
  for (uint32_t slot_id = 0; slot_id < blob_storage.cache_length; ++slot_id) {
    // Don't bother with invalid or empty slots
    if (blob_storage.slot_cache[slot_id].status
        != SL_BTMESH_BLOB_STORAGE_STATUS_OCCUPIED) {
      continue;
    }
    // Compare BLOB ID
    if (0 == memcmp(blob_id,
                    &blob_storage.slot_cache[slot_id].blob_id,
                    sizeof(sl_bt_uuid_64_t))) {
      // Return slot ID
      return slot_id;
    }
  }
  return UINT32_MAX;
}

sl_status_t sli_blob_storage_invalidate_slot(uint32_t slot_id, bool force)
{
  BootloaderStorageSlot_t slot;
  uint8_t invalid_flag[SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT];
  uint32_t validity_flag_offset;

  if ((force == false)
      && (sl_btmesh_blob_storage_get_erase_error_code()
          == SL_BTMESH_BLOB_STORAGE_DELETE_BUSY)) {
    return SL_STATUS_BUSY;
  }

  // Clear all fields which results in invalid footer
  memset(invalid_flag, 0, SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT);
  if (BOOTLOADER_OK != bootloader_getStorageSlotInfo(slot_id, &slot)) {
    return SL_STATUS_FAIL;
  }

  validity_flag_offset = slot.length
                         - sizeof(sli_btmesh_blob_storage_footer_t)
                         + offsetof(sli_btmesh_blob_storage_footer_t, validity_flag);

  // Write invalidity flag in the footer.
  // The layout of sli_btmesh_blob_storage_footer_t is designed to position the validity_flag
  // field to an offset which is integer multiple of storage slot alignment.
  // The size of validity_flag is also integer multiple of storage slot alignment.
  // This guarantees that the validity_flag field can be written separately from
  // the rest of the footer data.
  if (BOOTLOADER_OK != bootloader_writeStorage(slot_id,
                                               validity_flag_offset,
                                               invalid_flag,
                                               SLI_BTMESH_BLOB_STORAGE_MAX_ALIGNMENT)) {
    return SL_STATUS_FLASH_PROGRAM_FAILED;
  }
  blob_storage.slot_cache[slot_id].status = SL_BTMESH_BLOB_STORAGE_STATUS_CORRUPTED;
  return SL_STATUS_OK;
}

uint32_t sli_blob_storage_count_bits(const uint8_t *data, uint16_t length)
{
  uint32_t count = 0;
  for (uint16_t idx = 0; idx < length; idx++) {
    uint8_t byte = data[idx];
    byte = byte - ((byte >> 1) & 0x55);
    byte = (byte & 0x33) + ((byte >> 2) & 0x33);
    count += ((byte + (byte >> 4)) & 0x0F);
  }
  return count;
}

SL_WEAK uint32_t sl_btmesh_blob_storage_calculate_checksum(const uint8_t *data,
                                                           uint16_t length,
                                                           uint32_t initial_value)
{
  // The default checksum is the Berger code which is the number of zeros in data.
  // Flash write operation can clear bits only (1 -> 0) which means the number
  // of zeros can increase only in the data assuming writes only (no erase).
  // The key is that the checksum value stored in flash can decrease only
  // during write operations (1 -> 0) so the detection of unidirectional errors
  // are guaranteed by Berger code.
  uint32_t checksum = 0;
  if (data != NULL) {
    // Berger code = Number of bits - Number of 1 bits => Number of 0 bits
    checksum = initial_value
               + (((uint32_t) length) << 3)
               - sli_blob_storage_count_bits(data, length);
  }
  return checksum;
}

static sl_status_t blob_storage_check_app_footer_metadata(sli_btmesh_blob_storage_app_footer_metadata_t *metadata)
{
  app_assert_s(metadata != NULL);

  // If metadata contains all 0xFF bytes then it is considered valid because
  // it was read from erased storage slot space
  if ((metadata->app_id == APP_FOOTER_ID_INVALID)
      || (metadata->length == APP_FOOTER_LENGTH_INVALID)) {
    uint8_t *metadata_bytes = (uint8_t *) metadata;
    for (uint16_t idx = 0; idx < sizeof(sli_btmesh_blob_storage_app_footer_metadata_t); idx++) {
      if (metadata_bytes[idx] != UINT8_MAX) {
        return SL_STATUS_INVALID_SIGNATURE;
      }
    }
    return SL_STATUS_EMPTY;
  }

  // If app_id is not APP_FOOTER_ID_INVALID then app_id and length shall match
  // with their inverses
  uint16_t expected_app_id = ~metadata->app_id_inv;
  uint16_t expected_length = ~metadata->length_inv;
  if ((metadata->app_id != expected_app_id)
      || (metadata->length != expected_length)) {
    return SL_STATUS_INVALID_SIGNATURE;
  }

  // The next field of non-last app footer metadata shall match with its inverse
  if ((metadata->next.offset != APP_FOOTER_METADATA_OFFSET_NONE)
      && (metadata->next.offset != ~metadata->next.offset_inv)) {
    return SL_STATUS_INVALID_SIGNATURE;
  }

  // The last app footer metadata shall have its next field and its inverse set
  // to APP_FOOTER_METADATA_OFFSET_NONE. The next field of non-last app footer
  // metadata shall match with its inverse.
  if ((metadata->next.offset == metadata->next.offset_inv)
      && (metadata->next.offset != APP_FOOTER_METADATA_OFFSET_NONE)) {
    return SL_STATUS_INVALID_SIGNATURE;
  }

  return SL_STATUS_OK;
}

static sl_status_t blob_storage_check_app_footer_data(uint32_t slot_id,
                                                      uint32_t footer_offset,
                                                      uint16_t length)
{
  app_assert_s(length != APP_FOOTER_LENGTH_INVALID);

  uint8_t chunk_buffer[APP_FOOTER_DATA_CHUNK_SIZE];
  sli_btmesh_blob_storage_app_footer_checksum_t checksum;
  uint32_t offset = footer_offset;
  uint16_t remaining_length = length;
  uint32_t chunk_offset = 0;
  uint32_t checksum_calculated = 0;

  if (BOOTLOADER_OK != bootloader_readStorage(slot_id,
                                              offset,
                                              (uint8_t *)&checksum,
                                              sizeof(checksum))) {
    return SL_STATUS_FAIL;
  }

  if (checksum.checksum != ~checksum.checksum_inv) {
    return SL_STATUS_INVALID_SIGNATURE;
  }

  offset += sizeof(sli_btmesh_blob_storage_app_footer_checksum_t);

  while (0 < remaining_length) {
    uint16_t chunk_size = (remaining_length < APP_FOOTER_DATA_CHUNK_SIZE)
                          ? remaining_length : APP_FOOTER_DATA_CHUNK_SIZE;

    if (BOOTLOADER_OK != bootloader_readStorage(slot_id,
                                                offset + chunk_offset,
                                                chunk_buffer,
                                                chunk_size)) {
      return SL_STATUS_FAIL;
    }

    checksum_calculated = sl_btmesh_blob_storage_calculate_checksum(chunk_buffer,
                                                                    chunk_size,
                                                                    checksum_calculated);
    remaining_length -= chunk_size;
    chunk_offset += chunk_size;
  }

  if (checksum_calculated != checksum.checksum) {
    return SL_STATUS_INVALID_SIGNATURE;
  }

  return SL_STATUS_OK;
}

static sl_status_t blob_storage_get_app_footer_metadata(uint32_t slot_id,
                                                        uint16_t app_id,
                                                        sli_btmesh_blob_storage_check_t check_mode,
                                                        sli_btmesh_blob_storage_app_footer_metadata_t *metadata,
                                                        uint32_t *metadata_offset,
                                                        uint32_t *prev_metadata_offset,
                                                        uint32_t *footer_offset)
{
  BootloaderStorageSlot_t slot;
  uint32_t offset, prev_offset, blob_size;
  uint32_t app_footer_offset = 0, app_footer_size = 0;
  const uint32_t footer_size = sizeof(sli_btmesh_blob_storage_footer_t);
  const uint32_t app_footer_metadata_size = sizeof(sli_btmesh_blob_storage_app_footer_metadata_t);
  bool metadata_expected = false;
  sl_status_t sc;

  // Sanity checks for the input parameters
  app_assert_s(metadata != NULL);

  // Slot identifier check so the caller shouldn't need to check it
  if (slot_id >= blob_storage.cache_length) {
    return SL_STATUS_NOT_FOUND;
  }

  // The blob size shall be queried from the footer because this function can
  // be called at initialization phase before the cached is filled
  sc = blob_storage_get_blob_size_from_footer(slot_id, &blob_size);
  CHECK_SL_STATUS_RETURN(sc);

  // If app_id is found (or empty free space is searched) and there is no
  // previous app footer in the slot then prev_metadata_offset shall be set
  // to APP_FOOTER_METADATA_OFFSET_NONE
  prev_offset = APP_FOOTER_METADATA_OFFSET_NONE;

  if (BOOTLOADER_OK != bootloader_getStorageSlotInfo(slot_id, &slot)) {
    return SL_STATUS_FAIL;
  }
  // Set offset to the first possible position for footer metadata
  offset = slot.length
           - footer_size
           - app_footer_metadata_size;

  do {
    if (BOOTLOADER_OK != bootloader_readStorage(slot_id,
                                                offset,
                                                (uint8_t*)metadata,
                                                app_footer_metadata_size)) {
      return SL_STATUS_FAIL;
    }

    if (metadata_expected && (metadata->app_id == APP_FOOTER_ID_INVALID)) {
      // The metadata is expected but it is empty so it is considered as invalid.
      // This can happen when the app footers doesn't fit into a single storage
      // slot flash page. Some app footers can be deleted by page erase without
      // erasing the storage slot footer which can be detected by the next field
      // of the pervious app footer metadata.
      return SL_STATUS_INVALID_SIGNATURE;
    }

    // App footer metadata check is fast so it shall be performed in relaxed and
    // strict mode as well
    if (check_mode == SLI_BTMESH_BLOB_STORAGE_CHECK_RELAXED
        || check_mode == SLI_BTMESH_BLOB_STORAGE_CHECK_STRICT) {
      sc = blob_storage_check_app_footer_metadata(metadata);
      if (sc != SL_STATUS_OK && sc != SL_STATUS_EMPTY) {
        return sc;
      }
    }

    // App footer data check can be time consuming so it shall be performed in
    // strict mode
    if (metadata->app_id != APP_FOOTER_ID_INVALID) {
      app_footer_size  = sl_btmesh_blob_storage_calc_app_footer_size(metadata->length);
      // Check if app_footer_offset can be calculated without wrap-around.
      // If it is not possible then the metadata is considered as corrupted.
      if (offset + app_footer_metadata_size < app_footer_size) {
        return SL_STATUS_INVALID_SIGNATURE;
      }

      // Calculate the offset of the app footer
      app_footer_offset = offset
                          - app_footer_size
                          + app_footer_metadata_size;

      // If the app footer and blob data overlaps then the app footer metadata
      // is considered as corrupted
      if (app_footer_offset < blob_size) {
        return SL_STATUS_INVALID_SIGNATURE;
      }

      if (check_mode == SLI_BTMESH_BLOB_STORAGE_CHECK_STRICT) {
        sc = blob_storage_check_app_footer_data(slot_id,
                                                app_footer_offset,
                                                metadata->length);
        CHECK_SL_STATUS_RETURN(sc);
      }
    }

    // Check if the app footer metadata contains the searched app_id
    if (app_id == metadata->app_id) {
      if (metadata_offset != NULL) {
        *metadata_offset = offset;
      }
      if (prev_metadata_offset != NULL) {
        *prev_metadata_offset = prev_offset;
      }
      if (footer_offset != NULL) {
        if (metadata->app_id == APP_FOOTER_ID_INVALID) {
          *footer_offset = APP_FOOTER_METADATA_OFFSET_NONE;
        } else {
          *footer_offset = app_footer_offset;
        }
      }
      return SL_STATUS_OK;
    }

    if (metadata->app_id != APP_FOOTER_ID_INVALID) {
      prev_offset = offset;

      // Try to find next metadata
      if (metadata->next.offset == APP_FOOTER_METADATA_OFFSET_NONE) {
        // The last app footer iteration and the next app footer metadata location
        // can be calculated from the size of the app footer (no next filed set).
        // The next app footer metadata shall be empty (0xFF bytes) or it can
        // be on the top of the blob data.

        if (offset < blob_size + app_footer_size) {
          // If the last app footer overlaps with the blob data then the metadata
          // is considered as corrupted
          if (offset + app_footer_metadata_size < blob_size + app_footer_size) {
            return SL_STATUS_INVALID_SIGNATURE;
          } else {
            return SL_STATUS_NOT_FOUND;
          }
        }
        offset -= app_footer_size;
        metadata_expected = false;
      } else {
        // If the next field of app footer metadata points to higher offset or
        // into the blob data then the metadata is considered as corrupted
        if (offset <= metadata->next.offset && metadata->next.offset < blob_size) {
          return SL_STATUS_INVALID_SIGNATURE;
        }
        offset = metadata->next.offset;
        metadata_expected = true;
      }
    }
  } while (metadata->app_id != APP_FOOTER_ID_INVALID && blob_size <= offset);
  return SL_STATUS_NOT_FOUND;
}

static uint32_t blob_storage_get_max_blob_size_in_slot(uint32_t slot_id)
{
  BootloaderStorageSlot_t slot;

  // If it is not possible to query the storage slot then no blob can be stored
  // in it therefore 0 is returned as the supported max blob size in the slot.
  if (BOOTLOADER_OK != bootloader_getStorageSlotInfo(slot_id, &slot)) {
    return 0;
  }

  return sli_calc_max_blob_size(slot.length);
}

static sl_status_t blob_storage_compare(uint32_t slot_id,
                                        uint32_t slot_offset,
                                        uint8_t const * expected_data,
                                        uint32_t length,
                                        int32_t *comparison_result)
{
  uint8_t compare_chunk_buf[APP_FOOTER_DATA_CHUNK_SIZE];
  uint32_t remaining_length = length;
  uint32_t chunk_offset = 0;
  int chunk_compare_result;

  while (0 < remaining_length) {
    uint32_t chunk_size = (remaining_length < APP_FOOTER_DATA_CHUNK_SIZE)
                          ? remaining_length : APP_FOOTER_DATA_CHUNK_SIZE;

    if (BOOTLOADER_OK != bootloader_readStorage(slot_id,
                                                slot_offset + chunk_offset,
                                                &compare_chunk_buf[0],
                                                chunk_size)) {
      return SL_STATUS_FAIL;
    }

    chunk_compare_result = memcmp(&compare_chunk_buf[0],
                                  &expected_data[chunk_offset],
                                  chunk_size);

    if (0 != chunk_compare_result) {
      *comparison_result = chunk_compare_result;
      return SL_STATUS_OK;
    }

    remaining_length -= chunk_size;
    chunk_offset += chunk_size;
  }

  *comparison_result = 0;
  return SL_STATUS_OK;
}
