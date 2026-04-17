/***************************************************************************/ /**
 * @file file_operations_example.c
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
#include "file_operations_example.h"
#include "lfs.h"
#include "sl_si91x_littlefs_hal.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
lfs_t lfs;        //File-System handle
lfs_file_t file;  //File handle
char buffer[256]; //Data buffer

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
static void create_file(const char *path);
static void write_to_flash_file(const char *path, const char *data);
static void append_to_flash_file(const char *path, const char *data);
static void read_from_flash_file(const char *path);
static void seek_and_read_file(const char *path, lfs_soff_t offset);
static void truncate_file(const char *path, lfs_size_t size);
static void clear_file(const char *path);
static void simulate_reboot(void);

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

/*******************************************************************************
 *  QSPI File-System Example Initialization function
 ******************************************************************************/
void file_operations_example_init(void)
{
  int32_t err;

  // Initialize the qspi
  sl_si91x_littlefs_qspi_init();

  // mount the filesystem
  err = lfs_mount(&lfs, &cfg);

  // reformat if we can't mount the filesystem
  // this should only happen on the first boot
  if (err) {
    lfs_format(&lfs, &cfg);
    lfs_mount(&lfs, &cfg);
  }

  // Create the file if it doesn't exist
  create_file("myfile.txt");

  // Read data from the file at startup to verify persistence
  read_from_flash_file("myfile.txt");
  printf("Read from file at startup: %s\n", buffer);

  // Write data to a file
  const char *data = "Hello, LittleFS!";
  write_to_flash_file("myfile.txt", data);

  // Append data to the file
  const char *append_data = "Appending more data.";
  append_to_flash_file("myfile.txt", append_data);

  //The data read before and after reboot should be same.
  read_from_flash_file("myfile.txt");
  printf("Read from file before reboot: %s\n", buffer);

  // Simulate a reboot
  simulate_reboot();

  // Read data from the file after reboot
  read_from_flash_file("myfile.txt");
  printf("Read from file after reboot: %s\n", buffer);

  // Seek and read from the file
  // Seek to the 7th byte
  seek_and_read_file("myfile.txt", 7);
  printf("Read from offset 7: %s\n", buffer);

  // Truncate the file
  truncate_file("myfile.txt", 5);
  read_from_flash_file("myfile.txt");
  printf("After truncation: %s\n", buffer);

  // Clear the file
  clear_file("myfile.txt");
  read_from_flash_file("myfile.txt");
  printf("After clearing: %s\n", buffer);

  // Unmount the filesystem
  lfs_unmount(&lfs);
}

/*******************************************************************************
 * Example ticking function
 ******************************************************************************/
void file_operations_example_process_action(void)
{
}

/*******************************************************************************
 * Function to create a new file if it does not exist
 ******************************************************************************/
static void create_file(const char *path)
{
  // Try opening the file in read-only mode to check if it already exists
  int err = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);

  if (err < 0) {
    // File does not exist, create it
    err = lfs_file_open(&lfs, &file, path, LFS_O_WRONLY | LFS_O_CREAT);
    if (err < 0) {
      printf("Failed to create file\n");
      return;
    }

    // write initial data
    const char *initial_data = "File created.\n";
    err                      = lfs_file_write(&lfs, &file, initial_data, strlen(initial_data));
    if (err < 0) {
      printf("Failed to write initial data\n");
    }

    // Close the file after creation
    err = lfs_file_close(&lfs, &file);
    if (err < 0) {
      printf("Failed to close file\n");
    }
  } else {
    // File already exists, close it
    err = lfs_file_close(&lfs, &file);
    if (err < 0) {
      printf("Failed to close existing file\n");
    }
  }
}

/*******************************************************************************
 * Function to write data to a flash file
 ******************************************************************************/
static void write_to_flash_file(const char *path, const char *data)
{
  // Open the file for writing, create it if it doesn't exist
  int err = lfs_file_open(&lfs, &file, path, LFS_O_WRONLY | LFS_O_CREAT);
  if (err < 0) {
    printf("Failed to open file for writing\n");
    return;
  }

  // Write the data to the file
  err = lfs_file_write(&lfs, &file, data, strlen(data));
  if (err < 0) {
    printf("Failed to write data to file\n");
  }

  // The storage is not updated until the file is closed successfully
  err = lfs_file_close(&lfs, &file);
  if (err < 0) {
    printf("Failed to close file after writing\n");
  }
}

/*******************************************************************************
 * Function to append data to a flash file
 ******************************************************************************/
static void append_to_flash_file(const char *path, const char *data)
{
  // Open the file for appending
  int err = lfs_file_open(&lfs, &file, path, LFS_O_WRONLY | LFS_O_APPEND);
  if (err < 0) {
    printf("Failed to open file for appending\n");
    return;
  }

  // Add a newline character
  err = lfs_file_write(&lfs, &file, "\n", 1);
  if (err < 0) {
    printf("Failed to write newline before appending\n");
  }

  // Write the data to the file
  err = lfs_file_write(&lfs, &file, data, strlen(data));
  if (err < 0) {
    printf("Failed to write data to file\n");
  }

  // Add a newline character
  err = lfs_file_write(&lfs, &file, "\n", 1);
  if (err < 0) {
    printf("Failed to write newline after appending\n");
  }

  // Close the file to ensure data is written to storage
  err = lfs_file_close(&lfs, &file);
  if (err < 0) {
    printf("Failed to close file after appending\n");
  }
}

/*******************************************************************************
 * Function to read data from a flash file
 ******************************************************************************/
static void read_from_flash_file(const char *path)
{
  // Open the file for reading
  int err = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
  if (err < 0) {
    printf("Failed to open file for reading\n");
    return;
  }

  // Read the data into the buffer
  int size = lfs_file_read(&lfs, &file, buffer, sizeof(buffer));
  if (size < 0) {
    printf("Failed to read data from file\n");
  }
  // Null-terminate the string
  buffer[size] = '\0';

  // Close the file
  err = lfs_file_close(&lfs, &file);
  if (err < 0) {
    printf("Failed to close file after reading\n");
  }
}

/*******************************************************************************
 * Function to seek and read from a flash file
 ******************************************************************************/
static void seek_and_read_file(const char *path, lfs_soff_t offset)
{
  // Open the file for reading
  int err = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
  if (err < 0) {
    printf("Failed to open file for seeking and reading\n");
    return;
  }

  // Seek to the specified offset
  int pos = lfs_file_seek(&lfs, &file, offset, LFS_SEEK_SET);
  printf("Seek Position:%d\n", pos);

  // Read data from the new position
  int size = lfs_file_read(&lfs, &file, buffer, sizeof(buffer));
  if (size < 0) {
    printf("Failed to read data from file after seeking\n");
  } else {
    printf("Size of the characters read:%d\n", size);
  }

  // Null-terminate the string
  buffer[size] = '\0';

  // Close the file
  err = lfs_file_close(&lfs, &file);
  if (err < 0) {
    printf("Failed to close file after seeking and reading\n");
  }
}

/*******************************************************************************
 * Function to truncate a file to a specific size
 ******************************************************************************/
static void truncate_file(const char *path, lfs_size_t size)
{
  // Open the file for writing
  int err = lfs_file_open(&lfs, &file, path, LFS_O_RDWR);
  if (err < 0) {
    printf("Failed to open file for truncation\n");
    return;
  }
  // Truncate the file to the specified size
  err = lfs_file_truncate(&lfs, &file, size);
  if (err < 0) {
    printf("Failed to write data to truncated file\n");
  }

  // Close the file
  err = lfs_file_close(&lfs, &file);
  if (err < 0) {
    printf("Failed to close file after truncation\n");
  }
}

/*******************************************************************************
 * Function to clear the contents of a file
 ******************************************************************************/
static void clear_file(const char *path)
{
  // Truncate the file to zero length to clear its contents
  truncate_file(path, 0);
}

/*******************************************************************************
 * Function to simulate a reboot
 * 
 * This function simulates a file system reboot by unmounting and remounting 
 * the filesystem.It verifies data persistence by checking file contents 
 * before and after the reboot, ensuring filesystem integrity.
 ******************************************************************************/
static void simulate_reboot(void)
{
  // Unmount the filesystem
  lfs_unmount(&lfs);

  // Mount the filesystem again to simulate a reboot
  int err = lfs_mount(&lfs, &cfg);
  if (err) {
    // Reformat if we can't mount the filesystem
    err = lfs_format(&lfs, &cfg);
    err = lfs_mount(&lfs, &cfg);
  }
  if (err) {
    printf("Remounting of the File System failed\n");
  }
}
