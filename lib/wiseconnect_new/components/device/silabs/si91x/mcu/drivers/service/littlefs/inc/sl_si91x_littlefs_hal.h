/***************************************************************************/ /**
 * @file sl_si91x_littlefs_hal.h
 * @brief littlefs hal api implementation
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

#ifndef SL_SI91X_LITTLEFS_HAL_H
#define SL_SI91X_LITTLEFS_HAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lfs.h"
#include "rsi_qspi_proto.h"
#include "sl_status.h"

// flash page size
#define LITTLEFS_FLASH_PAGE_SIZE 256
// Default memory allocated for littlefs
#define LITTLEFS_DEFAULT_MEM_SIZE 524288

#define QSPI_OK                (0) // QSPI operation success
#define QSPI_ERROR             (1) // Error In qspi Opearation
#define SL_MAX_FILENAME_LENGTH 32  // Maximum file name length to save space.Usercan have till LFS_NAME_MAX

//File Structure
typedef struct sl_lfs_files {
  bool is_dir;
  char filename[SL_MAX_FILENAME_LENGTH];
} sl_si91x_littefs_dir_entry;
/***************************************************************************/ /**
 * @brief get the qspi default configs.
 * @details This function containts the default Configurations for QSPI module
 *
 * @param[in] spi_config     A pointer to the qspi Configurations
 * @return none
 ******************************************************************************/
void set_qspi_configs(spi_config_t *spi_config);

/***************************************************************************/ /**
 * @brief Initialize the qspi for littlefs.
 * @details This function initializes the qspi
 *
 * @param[in] none
 * @return none
 ******************************************************************************/
void sl_si91x_littlefs_qspi_init(void);

/***************************************************************************/ /**
 * @brief Reads the region in block.
 * @details  Read a region in a block. Negative error codes are propagated
 * to the user.
 *
 * @param[in] cfg  littlefs config struct
 * @param[in] block block no to be read region
 * @param[in] off  offset in the block to read
 * @param[in] buffer  buffer to store the data read
 * @param[in] size  no of bytes to read 
 * @return 0 - QSPI_OK for success else error code(1 - QSPI_ERROR)
 ******************************************************************************/
int si91x_block_device_read(const struct lfs_config *cfg,
                            lfs_block_t block,
                            lfs_off_t off,
                            void *buffer,
                            lfs_size_t size);

/***************************************************************************/ /**
 * @brief Program the region in block.
 * @details Program a region in a block. The block must have previously been erased.
 * Negative error codes are propagated to the user. May return LFS_ERR_CORRUPT
 * if the block should be considered bad.
 *
 * @param[in] cfg  littlefs config struct
 * @param[in] block block no to write in region
 * @param[in] off  offset in the block to program
 * @param[in] buffer  data buffer to program the region
 * @param[in] size  no of bytes to program  
 * @return 0 - QSPI_OK for success else error code(1 - QSPI_ERROR)
 ******************************************************************************/
int si91x_block_device_prog(const struct lfs_config *cfg,
                            lfs_block_t block,
                            lfs_off_t off,
                            const void *buffer,
                            lfs_size_t size);

/***************************************************************************/ /**
 * @brief Erase a block.
 * @details Erase a block. A block must be erased before being programmed. The state of an
 * erased block is undefined. Negative error codes are propagated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad..
 *
 * @param[in] cfg  littlefs config struct
 * @param[in] block block no to write
 * @return 0 - QSPI_OK for success else error code(1 - QSPI_ERROR)
 ******************************************************************************/
int si91x_block_device_erase(const struct lfs_config *cfg, lfs_block_t block);

/***************************************************************************/ /**
 * @brief Sync the state of the underlying block device.
 * @details Sync the state of the underlying block device. if the write function does not
 * perform caching, and therefore each read or write call hits the memory, the
 * sync function can simply return 0.
 *
 * @param[in] cfg  littlefs config struct
 * @return 0 - QSPI_OK
 ******************************************************************************/
int si91x_block_device_sync(const struct lfs_config *c);

/***************************************************************************/ /**
 * @brief Locks the LittleFS filesystem to prevent concurrent access.
 *
 * This function is used to lock the LittleFS filesystem to prevent concurrent access
 * from multiple tasks or interrupts. It is typically used in conjunction with
 * @ref si91x_block_device_unlock to ensure exclusive access to the filesystem.
 *
 * @note The default LittleFS protection functions can be substituted by the application
 * if other synchronization functions are available and disabling interrupts for
 * extended periods is not desired.
 *
 * @param[in] c  Pointer to the LittleFS configuration structure.
 *
 * @return 0 - QSPI_OK on success.
 ******************************************************************************/
int si91x_block_device_lock(const struct lfs_config *c);

/***************************************************************************/ /**
 * @brief Unlocks the LittleFS filesystem to allow concurrent access.
 *
 * This function is used to unlock the LittleFS filesystem after it has been locked
 * using @ref si91x_block_device_lock to prevent concurrent access from multiple tasks or interrupts.
 * It is typically used in conjunction with @ref si91x_block_device_lock to ensure exclusive access to the filesystem.
 *
 * @note The default LittleFS protection functions can be substituted by the application
 * if other synchronization functions are available and disabling interrupts for
 * extended periods is not desired.
 *
 * @param[in] c  Pointer to the LittleFS configuration structure.
 *
 * @return 0 - QSPI_OK on success.
 ******************************************************************************/
int si91x_block_device_unlock(const struct lfs_config *c);

/***************************************************************************/ /**
 * @brief List and count files in Directory.
 * @details List the files in Directory.Also counts the files in the Directory.
 *
 * @param[in] littlefs filesystem type
 * @param[in] Path of Directory
 * @param[out] List of files
 * @param[out] Count of files
 * @param[in] Size of Array
 * @return SL_STATUS_OK  for Success and SL_STATUS_FAIL for Failure
 ******************************************************************************/
sl_status_t sl_si91x_littefs_dir_list(lfs_t *lfs,
                                      const char *path,
                                      sl_si91x_littefs_dir_entry *entries,
                                      int *entries_count,
                                      int entries_buffer_array_size);

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_LITTLEFS_HAL_H */
