
/***************************************************************************/ /**
 * @file
 * @brief NVM3 Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_si91x_driver.h"
#include "nvm3_default.h"
#include "nvm3_default_config.h"
#include "sl_wifi_device.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define NVM3_DEFAULT_HANDLE nvm3_defaultHandle
// Maximum number of data objects saved
#define MAX_OBJECT_COUNT 10

// Max and min keys for data objects
#define MIN_DATA_KEY NVM3_KEY_MIN
#define MAX_DATA_KEY (MIN_DATA_KEY + MAX_OBJECT_COUNT - 1)

// Key of write counter object
#define WRITE_COUNTER_KEY MAX_OBJECT_COUNT

// Key of delete counter object
#define DELETE_COUNTER_KEY (WRITE_COUNTER_KEY + 1)
/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/
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
static char buffer[NVM3_DEFAULT_MAX_OBJECT_SIZE];

/******************************************************
 *               Function Declarations
 ******************************************************/
void application_start(const void *unused);
static void nvm3_app_read(nvm3_ObjectKey_t key);
static void nvm3_app_write(uint32_t key, unsigned char *data, uint32_t len);
static void nvm3_app_delete(uint32_t key);
static void nvm3_app_display(void);
static void initialise_counters(void);

/******************************************************
 *               Function Definitions
 ******************************************************/
void app_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

void application_start(const void *unused)
{
  UNUSED_PARAMETER(unused);
  uint32_t err            = 0;
  sl_status_t status      = SL_STATUS_OK;
  uint8_t write_data1[12] = { "Silicon labs" };
  uint8_t write_data2[4]  = { "NVM3" };

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init success\r\n");
  err = nvm3_initDefault();
  printf("\r\n NVM3 init status %lx \r\n", err);
  // Initialise the counter objects to track writes and deletes.
  initialise_counters();

  printf("\nwrite key 1 data\r\n");
  nvm3_app_write(1, write_data1, 12);
  nvm3_app_read(1);
  printf("\nwrite key 2 data\r\n");
  nvm3_app_write(2, write_data2, 4);
  nvm3_app_read(2);
  printf("\nwrite key 3 data\r\n");
  nvm3_app_write(3, write_data2, 4);
  nvm3_app_read(3);
  printf("\nwrite key 4 data\r\n");
  nvm3_app_write(4, write_data1, 12);
  nvm3_app_read(4);
  nvm3_app_display();
  printf("\nDeleting all keys\r\n");
  nvm3_app_delete(1);
  nvm3_app_delete(2);
  nvm3_app_delete(3);
  nvm3_app_delete(4);
  nvm3_app_display();
  // Delete all data in NVM3.
  err = nvm3_eraseAll(NVM3_DEFAULT_HANDLE);
  if (ECODE_NVM3_OK == err) {
    printf("Deleting all data stored in NVM3\r\n");
  }
}

static void nvm3_app_read(nvm3_ObjectKey_t key)
{
  uint32_t type;
  size_t len;
  Ecode_t err;

  do {
    // check for NVM3 maximum key value
    if (key > MAX_DATA_KEY) {
      printf("Invalid key\r\n");
      break;
    }
    err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, key, &type, &len);
    if (err != NVM3_OBJECTTYPE_DATA || type != NVM3_OBJECTTYPE_DATA) {
      printf("Key does not contain data object\r\n");
      break;
    }
    err = nvm3_readData(NVM3_DEFAULT_HANDLE, key, buffer, len);
    // check for error code
    if (ECODE_NVM3_OK == err) {
      buffer[len] = '\0';
      printf("Read data from key %lu:\r\n", key);
      printf("%s\r\n", buffer);
    } else {
      printf("Error reading data from key %lu\r\n", key);
    }
  } while (false);

  return;
}

static void nvm3_app_write(uint32_t key, unsigned char *data, uint32_t len)
{
  do {

    // check for NVM3 Maximum object size
    //    if (len > NVM3_DEFAULT_MAX_OBJECT_SIZE) {
    //      printf("Maximum object size exceeded\r\n");
    //      break;
    //    }
    // check for NVM3 maximum key value
    if (key > MAX_DATA_KEY) {
      printf("Invalid key\r\n");
      break;
    }
    // check for NVM3 write success or not
    if (ECODE_NVM3_OK == nvm3_writeData(NVM3_DEFAULT_HANDLE, key, (unsigned char *)data, len)) {
      printf("Stored data at key %lu\r\n", key);
      // Track number of writes in counter object
      nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, NULL);
    } else {
      printf("Error storing data\r\n");
    }
  } while (false);

  return;
}

static void nvm3_app_delete(uint32_t key)
{
  if (key > MAX_DATA_KEY) {
    printf("Invalid key\r\n");
  } else {
    // check for NVM3 delete object success or not
    if (ECODE_NVM3_OK == nvm3_deleteObject(NVM3_DEFAULT_HANDLE, key)) {
      printf("Deleted data at key %lu\r\n", key);
      // Track number or deletes in counter object
      nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, NULL);
    } else {
      printf("Error deleting key\r\n");
    }
  }
  return;
}

static void initialise_counters(void)
{
  uint32_t type;
  size_t len;
  Ecode_t err;

  // check if the designated keys contain counters, and initialise if needed.
  err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, &type, &len);
  if ((err != ECODE_NVM3_OK) || (type != NVM3_OBJECTTYPE_COUNTER)) {
    nvm3_writeCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, 0);
  }

  err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, &type, &len);
  if ((err != ECODE_NVM3_OK) || (type != NVM3_OBJECTTYPE_COUNTER)) {
    nvm3_writeCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, 0);
  }
}

static void nvm3_app_display(void)
{
  nvm3_ObjectKey_t keys[MAX_OBJECT_COUNT];
  size_t len, objects_count;
  uint32_t type;
  Ecode_t err;
  uint32_t counter = 0;
  size_t i;

  objects_count = nvm3_enumDeletedObjects(NVM3_DEFAULT_HANDLE,
                                          (uint32_t *)keys,
                                          sizeof(keys) / sizeof(keys[0]),
                                          MIN_DATA_KEY,
                                          MAX_DATA_KEY);
  // check for NVM3 deleted object count
  if (objects_count == 0) {
    printf("No deleted objects found\r\n");
  } else {
    printf("Keys of objects deleted from NVM3:\r\n");
    for (i = 0; i < objects_count; i++) {
      printf("> %lu\r\n", keys[i]);
    }
  }

  // Retrieve the keys of stored data
  objects_count =
    nvm3_enumObjects(NVM3_DEFAULT_HANDLE, (uint32_t *)keys, sizeof(keys) / sizeof(keys[0]), MIN_DATA_KEY, MAX_DATA_KEY);

  // check for NVM3 stored object count
  if (objects_count == 0) {
    printf("No stored objects found\r\n");
  } else {
    printf("Keys and contents of objects stored in NVM3:\r\n");
    for (i = 0; i < objects_count; i++) {
      nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, keys[i], &type, &len);
      if (type == NVM3_OBJECTTYPE_DATA) {
        err = nvm3_readData(NVM3_DEFAULT_HANDLE, keys[i], buffer, len);
        EFM_ASSERT(ECODE_NVM3_OK == err);
        buffer[len] = '\0';
        printf("> %lu: %s\r\n", keys[i], buffer);
      }
    }
  }
  // Display and reset counters
  err = nvm3_readCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, &counter);
  if (ECODE_NVM3_OK == err) {
    printf("%lu objects have been deleted since last display\r\n", counter);
  }
  nvm3_writeCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, 0);
  err = nvm3_readCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, &counter);
  if (ECODE_NVM3_OK == err) {
    printf("%lu objects have been written since last display\r\n", counter);
  }
  nvm3_writeCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, 0);
}
