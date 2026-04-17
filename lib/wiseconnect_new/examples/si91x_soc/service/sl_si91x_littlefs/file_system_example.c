/***************************************************************************/ /**
 * @file file_system_example.c
 * @brief File system example using littlefs library
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "rsi_debug.h"
#include "file_system_example.h"
#include "lfs.h"
#include "sl_si91x_littlefs_hal.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
lfs_t lfs;
lfs_file_t file;

// lfs structure cfg variable
const struct lfs_config cfg = {
  // block device operations
  .read  = si91x_block_device_read,  // Function to read data from the block device
  .prog  = si91x_block_device_prog,  // Function to program data to the block device
  .erase = si91x_block_device_erase, // Function to erase a block on the block device
  .sync  = si91x_block_device_sync,  // Function to synchronize the block device

  // block device configuration
  .read_size      = 16,   // Minimum size of a read operation in bytes
  .prog_size      = 16,   // Minimum size of a program operation in bytes
  .block_size     = 4096, // Size of an erasable block in bytes
  .block_count    = 64,   // Number of blocks in the block device
  .cache_size     = 16,   // Size of the cache in bytes
  .lookahead_size = 16,   // Size of the lookahead buffer in bytes
  .block_cycles   = 500,  // Number of erase cycles before the block is considered worn out
};

// Total size allocated for LittleFS in flash = block_size * block_count
// Total Size = 4096 * 64 = 262144 bytes (or 256 KB)

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

/*******************************************************************************
 * QSPI Example Initialization functions
 ******************************************************************************/

//  File system example init
void file_system_example_init(void)
{
  int32_t err;
  // Initialize the qspi
  sl_si91x_littlefs_qspi_init();

  // mount the filesystem
  err = lfs_mount(&lfs, &cfg);

  // reformat if we can't mount the filesystem
  // this should only happen on the first boot
  if (err) {
    err = lfs_format(&lfs, &cfg);
    err = lfs_mount(&lfs, &cfg);
  }

  // read current count
  uint32_t boot_count = 0;
  err                 = lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
  lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

  // update boot count
  boot_count += 1;
  lfs_file_rewind(&lfs, &file);
  lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

  // remember the storage is not updated until the file is closed successfully
  lfs_file_close(&lfs, &file);

  // release any resources we were using
  lfs_unmount(&lfs);

  // print the boot count
  printf("boot_count: %ld\n", boot_count);
}

/*******************************************************************************
 * Example ticking function
 ******************************************************************************/
void file_system_example_process_action(void)
{
}
