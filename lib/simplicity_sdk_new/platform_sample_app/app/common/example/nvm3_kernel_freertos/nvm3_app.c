/***************************************************************************//**
 * @file
 * @brief NVM3 example functions
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "nvm3_app.h"
#include "nvm3_default.h"
#include "nvm3_default_config.h"
#include "sl_cli.h"
#include "sl_cli_instances.h"
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Maximum number of data objects saved
#define MAX_OBJECT_COUNT    200

// Max and min keys for data objects
#define MIN_DATA_KEY        NVM3_KEY_MIN
#define MAX_DATA_KEY        (MIN_DATA_KEY + MAX_OBJECT_COUNT - 1)

// Key of write counter object
#define WRITE_COUNTER_KEY   MAX_OBJECT_COUNT

// Key of delete counter object
#define DELETE_COUNTER_KEY  (WRITE_COUNTER_KEY + 1)

// Use the default nvm3 handle from nvm3_default.h
#define NVM3_DEFAULT_HANDLE nvm3_defaultHandle

// Max retry attempts for repacking
#define MAX_REPACK_TRIES    500

/*******************************************************************************
 **************************   LOCAL VARIABLES   ********************************
 ******************************************************************************/

// Buffer for reading from NVM3
static char buffer[NVM3_DEFAULT_MAX_OBJECT_SIZE];
static size_t obj_sizes[MAX_OBJECT_COUNT];
nvm3_ObjectKey_t keys[MAX_OBJECT_COUNT];

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/
static void initialise_counters(void)
{
  uint32_t type;
  size_t len;
  sl_status_t sta;

  // check if the designated keys contain counters, and initialise if needed.
  sta = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, &type, &len);
  if ((sta != SL_STATUS_OK) || (type != NVM3_OBJECTTYPE_COUNTER)) {
    nvm3_writeCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, 0);
  }

  sta = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, &type, &len);
  if ((sta != SL_STATUS_OK) || (type != NVM3_OBJECTTYPE_COUNTER)) {
    nvm3_writeCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, 0);
  }
}
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Write data to NVM3
 *
 * This function implements the CLI command 'write' (see nvm3_app.slcp)
 * It stores string data at the selected NVM3 key.
 ******************************************************************************/
void nvm3_app_write(sl_cli_command_arg_t *arguments)
{
  uint32_t key;
  size_t len;

  // Get the key and data from CLI
  key = sl_cli_get_argument_uint32(arguments, 0);
  char *data = sl_cli_get_argument_string(arguments, 1);

  if (key > MAX_DATA_KEY) {
    printf("Invalid key\r\n");
    return;
  }

  len = strlen(data);

  if (len > NVM3_DEFAULT_MAX_OBJECT_SIZE) {
    printf("Maximum object size exceeded\r\n");
    return;
  }

  if (SL_STATUS_OK == nvm3_writeData(NVM3_DEFAULT_HANDLE,
                                     key,
                                     (unsigned char *)data,
                                     len)) {
    printf("Stored data at key 0x%lx\r\n", key);
    // Track number of writes in counter object
    nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, NULL);
  } else {
    printf("Error storing data\r\n");
  }
}

/***************************************************************************//**
 * Delete data in NVM3.
 *
 * This function implements the CLI command 'delete' (see nvm3_app.slcp)
 * It deletes the data object stored at the selected NVM3 key.
 ******************************************************************************/
void nvm3_app_delete(sl_cli_command_arg_t *arguments)
{
  // Get the key from CLI
  uint32_t key = sl_cli_get_argument_uint32(arguments, 0);

  if (key > MAX_DATA_KEY) {
    printf("Invalid key\r\n");
    return;
  }

  if (SL_STATUS_OK == nvm3_deleteObject(NVM3_DEFAULT_HANDLE, key)) {
    printf("Deleted data at key 0x%lx\r\n", key);
    // Track number or deletes in counter object
    nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, NULL);
  } else {
    printf("Error deleting key\r\n");
  }
}

/***************************************************************************//**
 * Read data from NVM3.
 *
 * This function implements the CLI command 'read' (see nvm3_app.slcp)
 * It reads the data object stored at the selected NVM3 key.
 ******************************************************************************/
void nvm3_app_read(sl_cli_command_arg_t *arguments)
{
  uint32_t type;
  size_t len;
  sl_status_t sta;

  // Get the key from CLI
  uint32_t key = sl_cli_get_argument_uint32(arguments, 0);

  if (key > MAX_DATA_KEY) {
    printf("Invalid key\r\n");
    return;
  }

  sta = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, key, &type, &len);
  if (sta != SL_STATUS_OK || type != NVM3_OBJECTTYPE_DATA) {
    printf("Key does not contain data object\r\n");
    return;
  }

  sta = nvm3_readData(NVM3_DEFAULT_HANDLE, key, buffer, len);
  if (SL_STATUS_OK == sta) {
    buffer[len] = '\0';
    printf("Read data from key 0x%lx:\r\n", key);
    printf("%s\r\n", buffer);
  } else {
    printf("Error reading data from key 0x%lx\r\n", key);
  }
}

/***************************************************************************//**
 * Display NVM3 data.
 *
 * This function implements the CLI command 'display' (see nvm3_app.slcp)
 * It displays:
 *   - Deleted objects
 *   - Valid objects stored in NVM3 (KEY, TYPE, SIZE, DATA)
 *   - Number of objects deleted since last display
 *   - Number of objects written since last display
 ******************************************************************************/
void nvm3_app_display(sl_cli_command_arg_t *arguments)
{
  size_t len, objects_count;
  uint32_t type;
  sl_status_t sta;
  uint32_t counter = 0;
  size_t i;

  (void)arguments;

  // Get deleted objects count
  objects_count = nvm3_enumDeletedObjects(NVM3_DEFAULT_HANDLE,
                                          (uint32_t *)keys,
                                          sizeof(keys) / sizeof(keys[0]),
                                          MIN_DATA_KEY,
                                          MAX_DATA_KEY + 2);
  if (objects_count == 0) {
    printf("No deleted objects found\r\n");
  } else {
    printf("Keys of objects deleted from NVM3:\r\n");
    for (i = 0; i < objects_count; i++) {
      printf("> 0x%lx\r\n", keys[i]);
    }
  }

  // Get valid objects count including write and delete counters
  objects_count = nvm3_enumObjects(NVM3_DEFAULT_HANDLE,
                                   (uint32_t *)keys,
                                   sizeof(keys) / sizeof(keys[0]),
                                   MIN_DATA_KEY,
                                   MAX_DATA_KEY + 2);

  if (objects_count == 0) {
    printf("No stored objects found\r\n");
  } else {
    printf("Valid objects stored in NVM3:\r\n");
    printf("%-10s %-11s %-7s %s\r\n", "KEY", "TYPE", "SIZE", "DATA");
    for (i = 0; i < objects_count; i++) {
      sta = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, keys[i], &type, &len);
      EFM_ASSERT(sta == SL_STATUS_OK);
      if (type == NVM3_OBJECTTYPE_DATA) {
        sta = nvm3_readData(NVM3_DEFAULT_HANDLE, keys[i], buffer, len);
        EFM_ASSERT(sta == SL_STATUS_OK);
        printf("0x%06lx   %-8s %6u B   ", keys[i], "Data", (unsigned)len);
        size_t display_len = len > 8 ? 8 : len;
        for (size_t j = 0; j < display_len; ++j) {
          printf("0x%02x%s", buffer[j], (j < display_len - 1) ? " " : "");
        }
        if (len > 8) {
          printf(" (+ %u more bytes)", (unsigned)(len - 8));
        }
        printf("\r\n");
      } else if (type == NVM3_OBJECTTYPE_COUNTER) {
        uint32_t counter_val = 0;
        sta = nvm3_readCounter(NVM3_DEFAULT_HANDLE, keys[i], &counter_val);
        EFM_ASSERT(sta == SL_STATUS_OK);
        printf("0x%06lx   %-8s %6u B   %lu\r\n", keys[i], "Counter", 204U, counter_val);
      } else {
        printf("0x%06lx   %-8s %6u B   ?\r\n", keys[i], "Unknown", (unsigned)len);
      }
    }
  }

  // Display and reset counters
  sta = nvm3_readCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, &counter);
  if (sta == SL_STATUS_OK) {
    printf("%lu objects have been deleted since last display\r\n", counter);
  }
  nvm3_writeCounter(NVM3_DEFAULT_HANDLE, DELETE_COUNTER_KEY, 0);

  sta = nvm3_readCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, &counter);
  if (sta == SL_STATUS_OK) {
    printf("%lu objects have been written since last display\r\n", counter);
  }
  nvm3_writeCounter(NVM3_DEFAULT_HANDLE, WRITE_COUNTER_KEY, 0);
}

/***************************************************************************//**
 * Delete all data in NVM3.
 *
 * This function implements the CLI command 'reset' (see nvm3_app.slcp)
 * It deletes all data stored in NVM3.
 ******************************************************************************/
void nvm3_app_reset(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  printf("Deleting all data stored in NVM3\r\n");
  nvm3_eraseAll(NVM3_DEFAULT_HANDLE);
  // This deletes the counters, too, so they must be re-initialised
  initialise_counters();
}

/***************************************************************************//**
 * Display current NVM3 memory information to the CLI.
 *
 * This function implements the CLI command 'get mem info' (see nvm3_app.slcp)
 * It displays available memory, low memory status, and cache status.
 ******************************************************************************/
void nvm3_app_get_mem_info(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  nvm3_MemInfo_t memInfo;
  sl_status_t sta = nvm3_getMemInfo(nvm3_defaultHandle, &memInfo);
  if (sta != SL_STATUS_OK) {
    printf("Failed to get NVM3 memory info (status: %lu)\r\n", sta);
    return;
  }
  printf("NVM3 Memory Info:\r\n"
         "  Available memory before repack: %u bytes\r\n"
         "  Low memory: %s\r\n"
         "  Cache low: %s\r\n",
         memInfo.availableMemory,
         memInfo.isMemoryLow ? "YES" : "NO",
         memInfo.isCacheLow ? "YES" : "NO");
  if (memInfo.isCacheLow) {
    printf("  Additional cache needed: %u bytes\r\n", memInfo.additionalCacheNeeded);
  }
}

/***************************************************************************//**
 * Fill a buffer with random bytes.
 ******************************************************************************/
void fill_random_buffer(char* buffer, size_t length)
{
  for (size_t i = 0; i < length; i++) {
    buffer[i] = (char)(rand() & 0xFF);
  }
}

/***************************************************************************//**
 * Get initial available memory info.
 ******************************************************************************/
sl_status_t nvm3_get_available_memory(uint32_t *total_available_memory)
{
  nvm3_MemInfo_t memInfo;
  sl_status_t sta = nvm3_getMemInfo(nvm3_defaultHandle, &memInfo);
  EFM_ASSERT(sta == SL_STATUS_OK);
  *total_available_memory = memInfo.availableMemory;

  return sta;
}

/***************************************************************************//**
 * Initialize the random number generator seed.
 ******************************************************************************/
void init_random_seed(void)
{
  uint32_t seed = xTaskGetTickCount() * portTICK_PERIOD_MS;
  srand(seed);
}

/***************************************************************************//**
 * Generate random object sizes to fill about half of the available memory.
 *
 * This function fills the obj_sizes array with random object sizes, such that
 * the total size is approximately half of the provided available memory.
 * The number of objects generated is stored in key_count.
 * If total_available_memory is zero, no objects are generated.
 ******************************************************************************/
void nvm3_generate_object_sizes(size_t *obj_sizes, size_t max_objs, uint32_t total_available_memory, uint32_t *key_count)
{
  size_t used_memory = 0;
  *key_count = 0;
  if (total_available_memory == 0) {
    return;
  }
  while (used_memory < (total_available_memory / 2)
         && *key_count < max_objs) {
    size_t obj_size = (rand() % NVM3_DEFAULT_MAX_OBJECT_SIZE) + 1;
    obj_sizes[(*key_count)++] = obj_size;
    used_memory += obj_size;
  }
}

/***************************************************************************//**
 * Write random-sized objects to NVM until low memory threshold is reached.
 *
 * This function generates random data buffers of up to NVM3_DEFAULT_MAX_OBJECT_SIZE
 * bytes and writes them to NVM3 using a set of keys. Each key is written at least
 * twice to provide duplicate data for repack testing, before the low memory
 * threshold is reached. The loop continues until the NVM3 low memory threshold is
 * reached or a write/memory info error occurs.
 ******************************************************************************/
void nvm3_write_until_low_mem(void)
{
  sl_status_t sta;
  const uint32_t key_start = MIN_DATA_KEY;
  uint32_t key_count = 0;
  uint32_t total_available_memory = 0;

  // Seed the random number generator
  init_random_seed();

  // Get available memory
  sta = nvm3_get_available_memory(&total_available_memory);
  EFM_ASSERT(sta == SL_STATUS_OK);

  // Generate random object sizes and get key count
  nvm3_generate_object_sizes(obj_sizes, sizeof(obj_sizes) / sizeof(obj_sizes[0]),
                             total_available_memory, &key_count);
  if (key_count == 0) {
    return;
  }

  printf("Writing keys: \r\n");

  for (int pass = 1; pass <= 2; pass++) {
    // Regenerate object sizes for the second pass
    if (pass == 2) {
      nvm3_generate_object_sizes(obj_sizes, sizeof(obj_sizes) / sizeof(obj_sizes[0]),
                                 total_available_memory, &key_count);
    }
    for (uint32_t i = 0; i < key_count; i++) {
      uint32_t key = key_start + i;
      size_t obj_size = obj_sizes[i];
      fill_random_buffer(buffer, obj_size);

      nvm3_MemInfo_t memInfo;
      sta = nvm3_getMemInfo(nvm3_defaultHandle, &memInfo);
      EFM_ASSERT(sta == SL_STATUS_OK);

      if (!memInfo.isMemoryLow) {
        if (obj_size > memInfo.availableMemory) {
          obj_size = memInfo.availableMemory;
        }
        printf("Write: key=0x%lx, size=%u bytes\r\n", key, (unsigned)obj_size);
        sta = nvm3_writeData(nvm3_defaultHandle, key, buffer, obj_size);
        EFM_ASSERT(sta == SL_STATUS_OK);
      } else {
        printf("Low memory: stopping writes\r\n");
        return;
      }
    }
  }
}

/***************************************************************************//**
 * Perform NVM3 repack operation with custom headroom and repack count.
 *
 * Implements the CLI command 'repack' (see nvm3_app.slcp).
 * Fills NVM3 with data until memory is low, then performs repack operations
 * using the specified repack headroom. Repack continues until available memory
 * meets or exceeds the user-defined repack headroom threshold. The process
 * repeats for the given repack count.
 ******************************************************************************/
void nvm3_app_repack(sl_cli_command_arg_t *arguments)
{
  sl_status_t sta;
  nvm3_MemInfo_t memInfo;
  uint32_t time_us;
  uint32_t total_repack_time_us = 0;
  uint32_t repack_tries;

  // Get the repack headroom and count from CLI
  uint32_t repack_headroom = sl_cli_get_argument_uint32(arguments, 0);
  uint32_t repack_count = sl_cli_get_argument_uint32(arguments, 1);

  if (repack_headroom > 4096) {
    printf("Repack headroom cannot exceed 4096 bytes\r\n");
    return;
  }
  if (repack_count < 1 || repack_count > 10000) {
    printf("Repack count must be between 1 and 10000\r\n");
    return;
  }

  // Set the repack headroom to the user-defined threshold
  if (repack_headroom > 0) {
    nvm3_defaultInit->repackHeadroom = repack_headroom;
    sta = nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
    if (sta != SL_STATUS_OK) {
      printf("NVM3 re-open with repack headroom %lu failed: %lu\r\n", repack_headroom, sta);
      return;
    }
  }

  printf("Starting repack: headroom=%lu, count=%lu\r\n", repack_headroom, repack_count);
  for (uint32_t i = 1; i <= repack_count; ++i) {
    // Write objects until low memory
    nvm3_write_until_low_mem();

    // Continue repacking until available memory meets or exceeds the user-defined repack headroom threshold
    repack_tries = 0; // Reset tries at the start of each iteration
    do {
      if (!nvm3_repackNeeded(nvm3_defaultHandle)) {
        printf("Repack not needed in iteration %lu\r\n", i);
        break;
      }
      sta = nvm3_timed_call_ret(nvm3_repack(nvm3_defaultHandle), NULL, &time_us);
      EFM_ASSERT(sta == SL_STATUS_OK);
      total_repack_time_us += time_us;
      sta = nvm3_getMemInfo(nvm3_defaultHandle, &memInfo);
      EFM_ASSERT(sta == SL_STATUS_OK);
      repack_tries++;
      if (repack_tries >= MAX_REPACK_TRIES) {
        printf("No duplicate memory to reclaim after %u repack tries, aborting repack loop.\r\n", MAX_REPACK_TRIES);
        return;
      }
    } while (memInfo.availableMemory <= nvm3_defaultHandle->repackHeadroom);

    printf("Repack Iteration: %lu, Time: %lu ms, Available memory: %u bytes\r\n\n",
           i, (total_repack_time_us / 1000U), memInfo.availableMemory);
    total_repack_time_us = 0;
  }
}

/***************************************************************************//**
 * Initialize NVM3 example.
 ******************************************************************************/
void nvm3_app_init(void)
{
  sl_status_t sta;

  // This will call nvm3_open() with default parameters for
  // memory base address and size, cache size, etc.
  sta = nvm3_initDefault();
  EFM_ASSERT(sta == SL_STATUS_OK);

  // Initialise the counter objects to track writes and deletes.
  initialise_counters();

  // printf is configured to output over IOStream
  printf("\r\nWelcome to the nvm3 sample application\r\n");
  printf("Type 'help' to see available commands\r\n");
}
