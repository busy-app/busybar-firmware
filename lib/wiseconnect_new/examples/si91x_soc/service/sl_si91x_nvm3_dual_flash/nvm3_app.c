/***************************************************************************/ /**
 * @file
 * @brief NVM3 examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_component_catalog.h"
#include "rsi_debug.h"
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "cmsis_os2.h"
#endif

#include "rsi_ccp_user_config.h"
#include "rsi_rom_egpio.h"
#include "nvm3.h"
#include "nvm3_hal_flash.h"
#include "nvm3_default.h"
#include "nvm3_default_config.h"
#include "nvm3_app.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

// Maximum number of data objects saved
#define MAX_OBJECT_COUNT 10

// Max and min keys for data objects
#define MIN_DATA_KEY NVM3_KEY_MIN
#define MAX_DATA_KEY (MIN_DATA_KEY + MAX_OBJECT_COUNT - 1)

// Key of write counter object
#define WRITE_COUNTER_KEY MAX_OBJECT_COUNT

// Key of delete counter object
#define DELETE_COUNTER_KEY (WRITE_COUNTER_KEY + 1)

// Use the default nvm3 handle from nvm3_default.h
#define NVM3_DEFAULT_HANDLE nvm3_defaultHandle

// UART input command arguments
#define CMD_INPUT 3

// Maximum string size
#define STRING_SIZE 260
/*******************************************************************************
 **************************   LOCAL VARIABLES   ********************************
 ******************************************************************************/

// Buffer for reading from NVM3
static char buffer[NVM3_DEFAULT_MAX_OBJECT_SIZE];
unsigned char newString[CMD_INPUT][NVM3_DEFAULT_MAX_OBJECT_SIZE];

#if defined(SL_CATALOG_KERNEL_PRESENT)
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
#endif

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/*******************************************************************************
 * Function to initialise the NVM3 counters
 * 
 * @param none
 * @return none
 ******************************************************************************/
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

/*******************************************************************************
 * Function to write data to NVM3
 * It stores string data at the selected NVM3 key.
 * 
 * @param[in] key   nvm3 key identifier
 * @param[in] data  pointer to the nvm3 data
 * @return none
 ******************************************************************************/
static void nvm3_app_write(uint32_t key, unsigned char *data)
{
  size_t len;

  do {
    // check for NVM3 maximum key value
    if (key > MAX_DATA_KEY) {
      printf("Invalid key\r\n");
      break;
    }

    len = strlen((const char *)data);

    // check for NVM3 Maximum object size
    if (len > NVM3_DEFAULT_MAX_OBJECT_SIZE) {
      printf("Maximum object size exceeded\r\n");
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

/*******************************************************************************
 * Function to delete data in NVM3.
 * It deletes the data object stored at the selected NVM3 key.
 * 
 * @param[in] key   nvm3 key identifier
 * @return none
 ******************************************************************************/
static void nvm3_app_delete(uint32_t key)
{
  // check for NVM3 maximum key value
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

/*******************************************************************************
 * Function to read data from NVM3.
 * It reads the data object stored at the selected NVM3 key.
 * 
 * @param[in] key   nvm3 key identifier
 * @return none
 ******************************************************************************/
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

/***************************************************************************/ /**
 * Function to display NVM3 data.
 *
 * This function implements the uart command 'display' (see nvm3_app.slcp)
 * It displays:
 *     - keys of deleted objects
 *     - keys and contents of saved objects
 *     - number of objects deleted since last display
 *     - number of objects written since last display
 * @param  none
 * @return none
 ******************************************************************************/
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

/***************************************************************************/ /**
 * Function to delete all data in NVM3.
 *
 * It deletes all data stored in NVM3.s
 * @param  none
 * @return none
 ******************************************************************************/
static void nvm3_app_reset(void)
{
  Ecode_t err;
  err = nvm3_eraseAll(NVM3_DEFAULT_HANDLE);
  if (ECODE_NVM3_OK == err) {
    printf("Deleting all data stored in NVM3\r\n");
  }
  // This deletes the counters, too, so they must be re-initialised
  initialise_counters();
}

/***************************************************************************/ /**
 * Function to get data from uart
 *
 * This function takes the data from uart console
 * @param  none
 * @return none
 ******************************************************************************/
static void get_data_from_uartconsole(void)
{
  uint32_t index_counter1 = 0;
  uint32_t index_counter2 = 0;
  uint32_t ctr            = 0;
  char str1[STRING_SIZE];

  // Get the input from uart console
  while (index_counter1 < STRING_SIZE) {
    str1[index_counter1] = DEBUGIN();
    if (str1[index_counter1] == 13) {
      str1[index_counter1] = '\0';
      break;
    }
    index_counter1++;
  }

  // Get the command, key and data from uart input
  for (index_counter1 = 0; index_counter1 <= (strlen(str1)); index_counter1++) {
    // if space or NULL found, assign NULL into newString[ctr]
    if (str1[index_counter1] == ' ' || str1[index_counter1] == '\0') {
      if (ctr == 2) {

        newString[ctr][index_counter2] = str1[index_counter1];
        index_counter2++;
      } else {
        newString[ctr][index_counter2] = '\0';
        ctr++;              //for next word
        index_counter2 = 0; //for next word, init index to 0
      }
    } else {
      newString[ctr][index_counter2] = str1[index_counter1];
      index_counter2++;
    }
  }
}
/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/ /**
 * Initialize NVM3 example.
 ******************************************************************************/
void nvm3_app_init(void)
{
#if defined(SL_CATALOG_KERNEL_PRESENT)
  osThreadNew((osThreadFunc_t)nvm3_app_process_action, NULL, &thread_attributes);
#endif
}

/***************************************************************************/ /**
 * NVM3 ticking function.
 ******************************************************************************/
void nvm3_app_process_action(void)
{
  int err;
  RSI_EGPIO_HostPadsGpioModeEnable(29);
  RSI_EGPIO_HostPadsGpioModeEnable(30);

  // This will call nvm3_open() with default parameters for
  // memory base address and size, cache size, etc.
  err = nvm3_initDefault();
  DEBUGOUT("\r\n NVM3 init status %d \r\n", err);
  EFM_ASSERT(err == ECODE_NVM3_OK);

  // Initialise the counter objects to track writes and deletes.
  initialise_counters();

  // printf is configured to output over IOStream
  printf("\r\nWelcome to the nvm3 sample application\r\n");
  printf("Type 'help' to see available commands\r\n");

  // Start of execution of the NVM3 sample application
  while (1) {
    get_data_from_uartconsole();

    printf("\n ");
    if (strcmp((char *)newString[0], "help") == 0) {
      DEBUGOUT("\r\nSelect any one option in below list: \r\n");
      DEBUGOUT("\r\n write \t\t:Write date to NVM3 memory\r\n");
      DEBUGOUT("\r\n read  \t\t:Read date from NVM3 memory\r\n");
      DEBUGOUT("\r\n delete \t\t:Delete key from NVM3 memory\r\n");
      DEBUGOUT("\r\n erase  \t\t:Erase NVM3 memory\r\n");
      DEBUGOUT("\r\n display  \t\t:Display all keys data in NVM3 memory\r\n");
    } else if (strcmp((char *)newString[0], "write") == 0) {
      // write data for object with key identifier
      nvm3_app_write(atoi((char *)newString[1]), &newString[2][0]);
    } else if (strcmp((char *)newString[0], "read") == 0) {
      // Find size of data for object with key identifier and read out
      nvm3_app_read(atoi((char *)newString[1]));
    } else if (strcmp((char *)newString[0], "delete") == 0) {
      // Find size of data for object with key identifier and delete data
      nvm3_app_delete(atoi((char *)newString[1]));
    } else if (strcmp((char *)newString[0], "erase") == 0) {
      // Deleting all data stored in NVM3
      nvm3_app_reset();
    } else if (strcmp((char *)newString[0], "display") == 0) {
      // Find size of data for objects with key identifier and display data
      nvm3_app_display();
    } else if (strcmp((char *)newString[0], "repack") == 0) {
      // Forced repacking
      printf("Repacking NVM...\r\n");
      nvm3_repack(NVM3_DEFAULT_HANDLE);
    } else {
      printf("Entered wrong command, Please enter valid command.....\r\n");
    }
  }
}
