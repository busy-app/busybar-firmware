/*******************************************************************************
 * @file  fs_commands.c
 * @brief File System AT Commands using littlefs
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_board_configuration.h"
#include "console.h"
#include "sl_net.h"
#include "lfs.h"
#include "sl_si91x_littlefs_hal.h"
#include "at_utility.h"
#include "at_command_data_mode.h"
#include "fs_commands.h"
#include <string.h>
#include <inttypes.h>

/******************************************************
 *                    Constants
 ******************************************************/

#define VERIFY_ERR_AND_RETURN(result) \
  do {                                \
    if (result < 0) {                 \
      return (sl_status_t)result;     \
    }                                 \
  } while (0)

#define FS_INVALID_INDEX 0xFFFF

#define FS_ROOT_DIRECTORY "/"

#ifndef FS_MAX_OPEN_FILES
#define FS_MAX_OPEN_FILES 64
#endif

#ifndef FS_DIR_ENTRIES_SIZE
#define FS_DIR_ENTRIES_SIZE FS_MAX_OPEN_FILES
#endif

#ifndef FS_READ_SIZE
#define FS_READ_SIZE 256
#endif

#ifndef FS_PROG_SIZE
#define FS_PROG_SIZE 256
#endif

#ifndef FS_BLOCK_SIZE
#define FS_BLOCK_SIZE 4096
#endif

#ifndef FS_BLOCK_COUNT
#define FS_BLOCK_COUNT 64
#endif

#ifndef FS_CACHE_SIZE
#define FS_CACHE_SIZE 256
#endif

#ifndef FS_LOOKAHEAD_SIZE
#define FS_LOOKAHEAD_SIZE 32
#endif

#ifndef FS_BLOCK_CYCLES
#define FS_BLOCK_CYCLES 500
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/
static bool fs_mounted = false;
static lfs_t lfs;
static lfs_file_t *files[FS_MAX_OPEN_FILES] = { 0 };

const struct lfs_config cfg = {
  // block device operations
  .read   = si91x_block_device_read,   // Function to read data from the block device
  .prog   = si91x_block_device_prog,   // Function to program data to the block device
  .erase  = si91x_block_device_erase,  // Function to erase a block on the block device
  .sync   = si91x_block_device_sync,   // Function to synchronize the block device
  .lock   = si91x_block_device_lock,   // Function to lock the block device
  .unlock = si91x_block_device_unlock, // Function to unlock the block device

  // block device configuration
  .read_size      = FS_READ_SIZE,      // Minimum size of a read operation in bytes
  .prog_size      = FS_PROG_SIZE,      // Minimum size of a program operation in bytes
  .block_size     = FS_BLOCK_SIZE,     // Size of an erasable block in bytes
  .block_count    = FS_BLOCK_COUNT,    // Number of blocks in the block device
  .cache_size     = FS_CACHE_SIZE,     // Size of the cache in bytes
  .lookahead_size = FS_LOOKAHEAD_SIZE, // Size of the lookahead buffer in bytes
  .block_cycles   = FS_BLOCK_CYCLES,   // Number of erase cycles before the block is considered worn out
};

void fs_init(void)
{
  sl_si91x_littlefs_qspi_init();
}

#ifndef LFS_READONLY
static sl_status_t fs_fwrite_send_buffer_handler(uint8_t *buffer, uint32_t length, void *user_data)
{
  lfs_file_t *file = (lfs_file_t *)user_data;
  if ((file == NULL) || (buffer == NULL)) {
    return SL_STATUS_NULL_POINTER;
  }

  if (length == 0) {
    return SL_STATUS_INVALID_RANGE;
  }

  lfs_ssize_t bytes_written = lfs_file_write(&lfs, file, buffer, length);
  VERIFY_ERR_AND_RETURN(bytes_written);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%" PRIi32 "\r\n", bytes_written);
  return SL_STATUS_OK;
}
#endif

// at+fs-format
sl_status_t fs_format_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

#ifndef LFS_READONLY
  int result = lfs_format(&lfs, &cfg);
  VERIFY_ERR_AND_RETURN(result);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
#else
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

// at+fs-mount
sl_status_t fs_mount_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  int result = lfs_mount(&lfs, &cfg);
  VERIFY_ERR_AND_RETURN(result);

  fs_mounted = true;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+fs-unmount
sl_status_t fs_unmount_command_handler(console_args_t *arguments)
{
  UNUSED_PARAMETER(arguments);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  // Close opened files
  for (uint32_t i = 0; i < FS_MAX_OPEN_FILES; ++i) {
    if (files[i]) {
      (void)lfs_file_close(&lfs, files[i]);
      SL_CLEANUP_MALLOC(files[i]);
    }
  }

  // Unmount
  int result = lfs_unmount(&lfs);
  VERIFY_ERR_AND_RETURN(result);

  fs_mounted = false;

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+fs-ls=<path>
sl_status_t fs_ls_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  const char *path = GET_OPTIONAL_COMMAND_ARG(arguments, 0, FS_ROOT_DIRECTORY, const char *);
  if (path == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int entries_count                                       = 0;
  sl_si91x_littefs_dir_entry entries[FS_DIR_ENTRIES_SIZE] = { 0 };

  sl_status_t status = sl_si91x_littefs_dir_list(&lfs, path, entries, &entries_count, FS_DIR_ENTRIES_SIZE);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d", entries_count);

  for (int i = 0; i < entries_count; ++i) {
    AT_PRINTF(" %s %d", entries[i].filename, entries[i].is_dir);
  }
  AT_PRINTF("\r\n");

  return SL_STATUS_OK;
}

// at+fs-fopen=<path>,<flags>
sl_status_t fs_fopen_command_handler(console_args_t *arguments)
{
#ifndef LFS_NO_MALLOC
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  const char *path = GET_OPTIONAL_COMMAND_ARG(arguments, 0, NULL, const char *);
  int flags        = GET_OPTIONAL_COMMAND_ARG(arguments, 1, LFS_O_RDONLY, int);
  if (path == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint16_t index = FS_INVALID_INDEX;
  for (uint16_t i = 0; i < FS_MAX_OPEN_FILES; ++i) {
    if (files[i] == NULL) {
      index = i;
      break;
    }
  }

  if (index == FS_INVALID_INDEX) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  files[index] = (lfs_file_t *)calloc(1, sizeof(lfs_file_t));
  if (files[index] == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  int result = lfs_file_open(&lfs, files[index], path, flags);
  if (result < 0) {
    SL_CLEANUP_MALLOC(files[index]);
    return (sl_status_t)result;
  }

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%d\r\n", index);

  return SL_STATUS_OK;
#else
  UNUSED_PARAMETER(arguments);
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

// at+fs-fseek=<file-id>,<offset>,<whence>
sl_status_t fs_fseek_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x07);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t file_id  = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  lfs_soff_t offset = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, lfs_soff_t);
  int whence        = GET_OPTIONAL_COMMAND_ARG(arguments, 2, LFS_SEEK_SET, int);

  if ((file_id >= FS_MAX_OPEN_FILES) || (files[file_id] == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  lfs_soff_t pos = lfs_file_seek(&lfs, files[file_id], offset, whence);
  VERIFY_ERR_AND_RETURN(pos);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%" PRIi32 "\r\n", pos);

  return SL_STATUS_OK;
}

// at+fs-fsize=<file-id>
sl_status_t fs_fsize_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t file_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  if ((file_id >= FS_MAX_OPEN_FILES) || (files[file_id] == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  lfs_soff_t size = lfs_file_size(&lfs, files[file_id]);
  VERIFY_ERR_AND_RETURN(size);

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%" PRIi32 "\r\n", size);
  return SL_STATUS_OK;
}

// at+fs-frewind=<file-id>
sl_status_t fs_frewind_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t file_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  if ((file_id >= FS_MAX_OPEN_FILES) || (files[file_id] == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int result = lfs_file_rewind(&lfs, files[file_id]);
  VERIFY_ERR_AND_RETURN(result);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+fs-fread=<file-id>,<size>
sl_status_t fs_fread_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t file_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  lfs_size_t size  = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, lfs_size_t);

  if ((file_id >= FS_MAX_OPEN_FILES) || (files[file_id] == NULL) || (size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((files[file_id]->flags & LFS_O_RDONLY) != LFS_O_RDONLY) {
    return SL_STATUS_OBJECT_READ;
  }

  uint8_t *buffer = (uint8_t *)calloc(1, size);
  if (buffer == NULL) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  lfs_ssize_t bytes_read = lfs_file_read(&lfs, files[file_id], buffer, size);
  if (bytes_read < 0) {
    SL_CLEANUP_MALLOC(buffer);
    return (sl_status_t)bytes_read;
  }

  PRINT_AT_CMD_SUCCESS;
  AT_PRINTF("%" PRIi32 " ", bytes_read);
  at_print_char_buffer((char *)buffer, (uint32_t)bytes_read);

  SL_CLEANUP_MALLOC(buffer);

  return SL_STATUS_OK;
}

// at+fs-fwrite=<file-id>,<size>
sl_status_t fs_fwrite_command_handler(console_args_t *arguments)
{
#ifndef LFS_READONLY
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t file_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  lfs_size_t size  = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, lfs_size_t);

  if ((file_id >= FS_MAX_OPEN_FILES) || (files[file_id] == NULL) || (size == 0)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((files[file_id]->flags & LFS_O_WRONLY) != LFS_O_WRONLY) {
    return SL_STATUS_OBJECT_WRITE;
  }

  sl_status_t status = at_command_goto_data_mode(fs_fwrite_send_buffer_handler, size, files[file_id]);
  VERIFY_STATUS_AND_RETURN(status);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
#else
  UNUSED_PARAMETER(arguments);
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

// at+fs-ftrunc=<file-id>,<size>
sl_status_t fs_ftrunc_command_handler(console_args_t *arguments)
{
#ifndef LFS_READONLY
  CHECK_ARGUMENT_BITMAP(arguments, 0x03);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t file_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  lfs_size_t size  = GET_OPTIONAL_COMMAND_ARG(arguments, 1, 0, lfs_size_t);

  if ((file_id >= FS_MAX_OPEN_FILES) || (files[file_id] == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((files[file_id]->flags & LFS_O_WRONLY) != LFS_O_WRONLY) {
    return SL_STATUS_OBJECT_WRITE;
  }

  int result = lfs_file_truncate(&lfs, files[file_id], size);
  VERIFY_ERR_AND_RETURN(result);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
#else
  UNUSED_PARAMETER(arguments);
  return SL_STATUS_NOT_SUPPORTED;
#endif
}

// at+fs-fclose=<file-id>
sl_status_t fs_fclose_command_handler(console_args_t *arguments)
{
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  uint16_t file_id = GET_OPTIONAL_COMMAND_ARG(arguments, 0, 0, uint16_t);
  if ((file_id >= FS_MAX_OPEN_FILES) || (files[file_id] == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int result = lfs_file_close(&lfs, files[file_id]);
  SL_CLEANUP_MALLOC(files[file_id]);

  VERIFY_ERR_AND_RETURN(result);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
}

// at+fs-fremove=<path>
sl_status_t fs_fremove_command_handler(console_args_t *arguments)
{
#ifndef LFS_READONLY
  CHECK_ARGUMENT_BITMAP(arguments, 0x01);

  if (!fs_mounted) {
    return SL_STATUS_NOT_INITIALIZED;
  }

  const char *path = GET_OPTIONAL_COMMAND_ARG(arguments, 0, NULL, const char *);
  if (path == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  int result = lfs_remove(&lfs, path);
  VERIFY_ERR_AND_RETURN(result);

  PRINT_AT_CMD_SUCCESS;
  return SL_STATUS_OK;
#else
  UNUSED_PARAMETER(arguments);
  return SL_STATUS_NOT_SUPPORTED;
#endif
}