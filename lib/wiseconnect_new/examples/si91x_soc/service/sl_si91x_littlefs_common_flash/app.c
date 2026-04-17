/***************************************************************************/ /**
 * @file app.c
 * @brief Top level application functions
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
#include "lfs.h"
#include "sl_net.h"
#include "sl_si91x_driver.h"
#include "sl_wifi_device.h"
#include "sl_si91x_littlefs_hal.h"
#include "cmsis_os2.h"
#include "rsi_debug.h"
#include "app.h"

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
  .read   = si91x_block_device_read,   // Function to read data from the block device
  .prog   = si91x_block_device_prog,   // Function to program data to the block device
  .erase  = si91x_block_device_erase,  // Function to erase a block on the block device
  .sync   = si91x_block_device_sync,   // Function to synchronize the block device
  .lock   = si91x_block_device_lock,   // Function to lock the block device
  .unlock = si91x_block_device_unlock, // Function to unlock the block device

  // block device configuration
  .read_size      = 64,   // Minimum size of a read operation in bytes
  .prog_size      = 64,   // Minimum size of a program operation in bytes
  .block_size     = 4096, // Size of an erasable block in bytes
  .block_count    = 64,   // Number of blocks in the block device
  .cache_size     = 256,  // Size of the cache in bytes
  .lookahead_size = 32,   // Size of the lookahead buffer in bytes
  .block_cycles   = 500,  // Number of erase cycles before the block is considered worn out
};

// Total size allocated for LittleFS in flash = block_size * block_count
// Total Size = 4096 * 64 = 262144 bytes (or 256 KB)

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
void application_start(const void *unused);

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t station_init_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION
#ifdef SLI_SI91X_MCU_INTERFACE
                                       | SL_WIFI_FEAT_WPS_DISABLE
#endif
                                       ),
                   .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
                   .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map = (
#ifdef SLI_SI91X_MCU_INTERFACE
                     SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC
#else
                     SL_SI91X_RAM_LEVEL_NWP_ALL_MCU_ZERO
#endif
#if defined(SLI_SI917)
                     | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                     ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = 0,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};

/*******************************************************************************
 * File system example initialization.
 ******************************************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

/*******************************************************************************
 * This function initializes the application and handles the boot count.
 *
 * It performs the following tasks:
 * Initializes the Wi-Fi client interface using the provided configuration.
 * Mounts the LittleFS filesystem using the provided configuration.
 * If the filesystem cannot be mounted, it reformats the filesystem.
 * Opens a file named "boot_count" for reading and writing.
 * Reads the current boot count from the file.
 * Increments the boot count by 1.
 * Rewinds the file and writes the updated boot count back to the file.
 * Closes the file.
 * Unmounts the LittleFS filesystem.
 * Prints the updated boot count.
 ******************************************************************************/
void application_start(const void *unused)
{
  UNUSED_PARAMETER(unused);
  int32_t err;
  uint32_t status;
  uint32_t boot_count = 0;

  DEBUGOUT("\r\nInitializing wireless stack\r\n");
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\nFailed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  DEBUGOUT("\r\nWireless stack initialized successfully\r\n");

  DEBUGOUT("\r\nMounting file system\r\n");
  // mount the filesystem
  err = lfs_mount(&lfs, &cfg);

  // reformat if we can't mount the filesystem
  // this should only happen on the first boot
  if (err) {
    DEBUGOUT("\r\nFile system not found, formatting\r\n");
    err = lfs_format(&lfs, &cfg);
    err = lfs_mount(&lfs, &cfg);
    DEBUGOUT("\r\nFile system formatted and mounted\r\n");
  } else {
    DEBUGOUT("\r\nFile system mounted successfully\r\n");
  }

  DEBUGOUT("\r\nBoot count operations\r\n");
  // read current count
  err = lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
  if (err) {
    DEBUGOUT("\r\nFailed to open the file \r\n");
    return;
  }
  DEBUGOUT("\r\nBoot count file opened\r\n");

  lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
  DEBUGOUT("\r\nCurrent boot count read: %ld\r\n", boot_count);

  // update boot count
  boot_count += 1;
  lfs_file_rewind(&lfs, &file);
  lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
  DEBUGOUT("\r\nBoot count written: %ld\r\n", boot_count);

  // remember the storage is not updated until the file is closed successfully
  lfs_file_close(&lfs, &file);
  DEBUGOUT("\r\nBoot count file closed\r\n");

  // release any resources we were using
  lfs_unmount(&lfs);
  DEBUGOUT("\r\nFile system unmounted\r\n");

  // print the boot count
  DEBUGOUT("\r\nboot_count: %ld\r\n", boot_count);
}
