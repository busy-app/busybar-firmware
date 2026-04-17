/*******************************************************************************
 * @file
 * @brief Common Token Manager examples functions
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

#include "ctm_app.h"
#include "sl_token_manager_api.h"
#include "sl_token_manufacturing_api.h"
#include "sl_token_manager_defines.h"
#include "sl_token_api.h"
#include "sl_token_manager_manufacturing.h"
#include "sl_cli.h"
#include "sl_cli_instances.h"

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/
// --- Common Token Manager Buffer Size ---
#define SL_CTM_READ_BUFFER_SIZE                     SL_CLI_INPUT_BUFFER_SIZE

// --- Dynamic Tokens ---
#define CTM_COUNTER_TOKEN_KEY                       (0x130)
// Create a 32-bit token identifier for counter token
#define CTM_SAMPLE_APP_TOK_UINT32_COUNTER           SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_USER | CTM_COUNTER_TOKEN_KEY), 1)

// --- Static Device Tokens ---
#define CTM_SAMPLE_APP_TOK_UINT16_STATIC_DEVICE     (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x0252)
#define CTM_SAMPLE_APP_TOK_UINT32_STATIC_DEVICE     (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x0254)
#define CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_DEVICE (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x0258)
#define CTM_SAMPLE_APP_TOK_STRING_STATIC_DEVICE     (SL_TOKEN_STATIC_DEVICE_TOKENS | 0x0260)

// --- Static Secure Tokens ---
#define CTM_SAMPLE_APP_TOK_UINT16_STATIC_SECURE     (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x0602)
#define CTM_SAMPLE_APP_TOK_UINT32_STATIC_SECURE     (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x0604)
#define CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_SECURE (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x0608)
#define CTM_SAMPLE_APP_TOK_STRING_STATIC_SECURE     (SL_TOKEN_STATIC_SECURE_DATA_TOKENS | 0x0610)

// --- Sizes ---
#define CTM_SAMPLE_APP_UINT8_SIZE           1
#define CTM_SAMPLE_APP_UINT16_SIZE          2
#define CTM_SAMPLE_APP_UINT32_SIZE          4
#define CTM_SAMPLE_APP_BYTE_ARRAY_SIZE      8
#define CTM_SAMPLE_APP_STRING_SIZE          32
#define CTM_SAMPLE_APP_UINT32_COUNTER_SIZE  4

#define CTM_SAMPLE_APP_TOK_UINT16_STATIC_SECURE_SIZE      CTM_SAMPLE_APP_UINT16_SIZE
#define CTM_SAMPLE_APP_TOK_UINT32_STATIC_SECURE_SIZE      CTM_SAMPLE_APP_UINT32_SIZE
#define CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_SECURE_SIZE  CTM_SAMPLE_APP_BYTE_ARRAY_SIZE
#define CTM_SAMPLE_APP_TOK_STRING_STATIC_SECURE_SIZE      CTM_SAMPLE_APP_STRING_SIZE

#define CTM_SAMPLE_APP_TOK_UINT16_STATIC_DEVICE_SIZE       CTM_SAMPLE_APP_UINT16_SIZE
#define CTM_SAMPLE_APP_TOK_UINT32_STATIC_DEVICE_SIZE       CTM_SAMPLE_APP_UINT32_SIZE
#define CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_DEVICE_SIZE   CTM_SAMPLE_APP_BYTE_ARRAY_SIZE
#define CTM_SAMPLE_APP_TOK_STRING_STATIC_DEVICE_SIZE       CTM_SAMPLE_APP_STRING_SIZE

/*******************************************************************************
 **************************   LOCAL VARIABLES   ********************************
 ******************************************************************************/

// Buffer for reading from CTM
static char read_buffer[SL_CTM_READ_BUFFER_SIZE];

uint32_t uint32_test_data = 0x87654321;
uint8_t array_test_data[8] = { 0xab, 0xac, 0xba, 0xbe, 0xec, 0xfc, 0xda, 0xae };

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/******************************************************************************
* Write static secure tokens using common token manager.
******************************************************************************/
void write_custom_static_secure_data_token()
{
  uint16_t uint16_test_data = 0x2468;
  char string_test_data[32] = "Secure Token Example";
  sl_status_t status = SL_STATUS_OK;

  status = sl_token_manager_set_data(SL_TOKEN_GET_STATIC_SECURE_TOKEN(CTM_SAMPLE_APP_TOK_UINT16_STATIC_SECURE),
                                     &uint16_test_data, CTM_SAMPLE_APP_TOK_UINT16_STATIC_SECURE_SIZE);
  printf("Static secure UINT16 token: \r\n key:0x%x \r\n data: 0x%04x \r\n status: %lx\r\n",
         CTM_SAMPLE_APP_TOK_UINT16_STATIC_SECURE, uint16_test_data, status);

  status = sl_token_manager_set_data(SL_TOKEN_GET_STATIC_SECURE_TOKEN(CTM_SAMPLE_APP_TOK_STRING_STATIC_SECURE),
                                     string_test_data, CTM_SAMPLE_APP_TOK_STRING_STATIC_SECURE_SIZE);
  printf("Static secure STRING token: \r\n key:0x%x \r\n data: %s \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_STRING_STATIC_SECURE, string_test_data, status);

  //Set using the Legacy API as for common token manager, legacy API is also supported.
  halCommonSetMfgToken(CTM_SAMPLE_APP_TOK_UINT32_STATIC_SECURE, &uint32_test_data);
  printf("Static secure UINT32 token:\r\n key:0x%x \r\n data: 0x%08lx \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_UINT32_STATIC_SECURE, uint32_test_data, status);
  halCommonSetMfgToken(CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_SECURE, array_test_data);
  printf("Static secure BYTE ARRAY token: \r\n key:0x%x \r\n data:",
         CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_SECURE);
  for (size_t i = 0; i < CTM_SAMPLE_APP_BYTE_ARRAY_SIZE; i++) {
    printf("%02x", array_test_data[i]);
  }
  printf("\r\n status: %lx \r\n", status);
}

/******************************************************************************
* Read static secure tokens using common token manager.
******************************************************************************/
void read_custom_static_secure_data_token()
{
  uint16_t uint16_data;
  uint32_t uint32_data;
  uint8_t array_data[8];
  char string_data[32];
  sl_status_t status = SL_STATUS_OK;
  status = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_SECURE_TOKEN(CTM_SAMPLE_APP_TOK_UINT16_STATIC_SECURE), &uint16_data, CTM_SAMPLE_APP_UINT16_SIZE);
  printf("Reading Static secure UINT16 token: \r\n key:0x%x \r\n data: 0x%04x \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_UINT16_STATIC_SECURE, uint16_data, status);
  status = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_SECURE_TOKEN(CTM_SAMPLE_APP_TOK_STRING_STATIC_SECURE), &string_data, CTM_SAMPLE_APP_STRING_SIZE);
  printf("Reading Static secure STRING token: \r\n key:0x%x \r\n data: %s \r\n status: %lx \r\n", CTM_SAMPLE_APP_TOK_STRING_STATIC_SECURE, string_data, status);

  //Get using the Legacy API as for common token manager, legacy API is also supported.
  halCommonGetMfgToken(&uint32_data, CTM_SAMPLE_APP_TOK_UINT32_STATIC_SECURE);
  printf("Reading Static secure UINT32 token: \r\n key:0x%x \r\n data: 0x%08lx \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_UINT32_STATIC_SECURE, uint32_data, status);
  halCommonGetMfgToken(&array_data, CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_SECURE);
  printf("Reading Static secure BYTE ARRAY token:\r\n key:0x%x \r\n data:", CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_SECURE);
  for (size_t i = 0; i < CTM_SAMPLE_APP_BYTE_ARRAY_SIZE; i++) {
    printf("%02x", array_data[i]);
  }
  printf("\r\n status: %lx \r\n", status);
}

/******************************************************************************
* Write static device tokens using common token manager.
******************************************************************************/
void write_custom_static_device_data_token()
{
  uint16_t uint16_test_data = 0x8765;
  char string_test_data[32] = "Device Token Test";
  sl_status_t status = SL_STATUS_OK;

  status = sl_token_manager_set_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(CTM_SAMPLE_APP_TOK_UINT16_STATIC_DEVICE),
                                     &uint16_test_data, CTM_SAMPLE_APP_TOK_UINT16_STATIC_DEVICE_SIZE);
  printf("Static device UINT16 token: \r\n key:0x%x \r\n data: 0x%04x \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_UINT16_STATIC_DEVICE, uint16_test_data, status);

  status = sl_token_manager_set_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(CTM_SAMPLE_APP_TOK_UINT32_STATIC_DEVICE),
                                     &uint32_test_data, CTM_SAMPLE_APP_TOK_UINT32_STATIC_DEVICE_SIZE);
  printf("Static device UINT32 token: \r\n key:0x%x \r\n data: 0x%08lx \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_UINT32_STATIC_DEVICE, uint32_test_data, status);

  status = sl_token_manager_set_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_DEVICE),
                                     array_test_data, CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_DEVICE_SIZE);
  printf("Static device BYTE ARRAY token: \r\n key:0x%x \r\n data:", CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_DEVICE);
  for (size_t i = 0; i < CTM_SAMPLE_APP_BYTE_ARRAY_SIZE; i++) {
    printf("%02x", array_test_data[i]);
  }
  printf("\r\n status: %lx \r\n", status);

  status = sl_token_manager_set_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(CTM_SAMPLE_APP_TOK_STRING_STATIC_DEVICE),
                                     string_test_data, CTM_SAMPLE_APP_TOK_STRING_STATIC_DEVICE_SIZE);
  printf("Static device STRING token: \r\n key:0x%x \r\n data: %s \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_STRING_STATIC_DEVICE, string_test_data, status);
}

/******************************************************************************
* Read static device tokens using common token manager.
******************************************************************************/
void read_custom_static_device_data_token()
{
  uint16_t uint16_data;
  uint32_t uint32_data;
  uint8_t array_data[8];
  char string_data[32];
  sl_status_t status = SL_STATUS_OK;

  status = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(CTM_SAMPLE_APP_TOK_UINT16_STATIC_DEVICE),
                                     &uint16_data, CTM_SAMPLE_APP_UINT16_SIZE);
  printf("Reading Static device UINT16 token: \r\n key:0x%x \r\n data: 0x%04x \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_UINT16_STATIC_DEVICE, uint16_data, status);

  status = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(CTM_SAMPLE_APP_TOK_UINT32_STATIC_DEVICE),
                                     &uint32_data, CTM_SAMPLE_APP_UINT32_SIZE);
  printf("Reading Static device UINT32 token: \r\n key:0x%x \r\n data: 0x%08lx \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_UINT32_STATIC_DEVICE, uint32_data, status);

  status = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_DEVICE),
                                     array_data, CTM_SAMPLE_APP_BYTE_ARRAY_SIZE);
  printf("Reading Static device BYTE ARRAY token: \r\n key:0x%x \r\n data:",
         CTM_SAMPLE_APP_TOK_BYTE_ARRAY_STATIC_DEVICE);
  for (size_t i = 0; i < CTM_SAMPLE_APP_BYTE_ARRAY_SIZE; i++) {
    printf("%02x", array_data[i]);
  }
  printf("\r\n status: %lx \r\n", status);

  status = sl_token_manager_get_data(SL_TOKEN_GET_STATIC_DEVICE_TOKEN(CTM_SAMPLE_APP_TOK_STRING_STATIC_DEVICE),
                                     string_data, CTM_SAMPLE_APP_STRING_SIZE);
  printf("Reading Static device STRING token: \r\n key:0x%x \r\n data: %s \r\n status: %lx \r\n",
         CTM_SAMPLE_APP_TOK_STRING_STATIC_DEVICE, string_data, status);
}

/******************************************************************************
* Write static secure or static device tokens using common token manager.
*
* This function implements the CLI command 'write_custom_static_token'.
* (see common_token_manager_baremetal.slcp)
******************************************************************************/
void ctm_write_custom_static_token(sl_cli_command_arg_t *arguments)
{
  printf("<<write static token command>>\r\n");
  uint32_t type = sl_cli_get_argument_uint32(arguments, 0);

  switch (type) {
    case 0:
      printf("Writing static secure tokens...\r\n");
      write_custom_static_secure_data_token();
      break;
    case 1:
      printf("Writing static device tokens...\r\n");
      write_custom_static_device_data_token();
      break;
    default:
      printf("Invalid token type\r\n");
      break;
  }
}

/******************************************************************************
* Read static secure or static device tokens using common token manager.
*
* This function implements the CLI command 'read_custom_static_token'.
* (see common_token_manager_baremetal.slcp)
******************************************************************************/
void ctm_read_custom_static_token(sl_cli_command_arg_t *arguments)
{
  printf("<<read static token command>>\r\n");
  uint32_t type = sl_cli_get_argument_uint32(arguments, 0);
  switch (type) {
    case 0:
      printf("Reading static secure tokens...\r\n");
      read_custom_static_secure_data_token();
      break;
    case 1:
      printf("Reading static device tokens...\r\n");
      read_custom_static_device_data_token();
      break;
    default:
      printf("Invalid token type\r\n");
      break;
  }
}

/******************************************************************************
* Writes dynamic tokens to the Common Token Manager
*
* This function implements the CLI command 'write_dynamic_token' and supports:
*   - Dynamic data object tokens
*   - Dynamic override tokens for static device
*   - Dynamic override tokens for static secure
* (see common_token_manager_baremetal.slcp)
******************************************************************************/
void ctm_write_dynamic_token(sl_cli_command_arg_t *arguments)
{
  printf("<<Write dynamic token command>>\r\n");
  sl_status_t status;
  uint32_t token_key;
  uint32_t type = sl_cli_get_argument_uint32(arguments, 0);
  uint32_t key = sl_cli_get_argument_uint32(arguments, 1);
  char *data = sl_cli_get_argument_string(arguments, 2);

  switch (type) {
    case 0:
      printf("Writing dynamic data object token...\r\n");
      status = sl_token_manager_set_data(SL_TOKEN_GET_DYNAMIC_TOKEN(key, 0), data, strlen(data));
      if (SL_STATUS_OK == status) {
        printf("Dynamic data object token written: \r\n key:0x%lx \r\n data: %s \r\n", key, data);
      } else {
        printf("Error writing dynamic data object token: \r\n key:0x%lx \r\n data: %s \r\n", key, data);
        printf("Write dynamic data object token status is %lu.\r\n", status);
      }
      break;
    case 1:
      if (key > 0xFFF) {
        printf("Invalid key for override static device token. Key should be less than 0xFFF.\r\n");
        return;
      }
      token_key = key | SL_TOKEN_STATIC_DEVICE_TOKENS; // Set the key to static device token range
      printf("Writing static device override token...\r\n");
      status = sl_token_manager_set_data(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token_key), data, strlen(data));
      if (SL_STATUS_OK == status) {
        printf("Override static device token written: \r\n key:0x%lx \r\n data: %s \r\n", key, data);
      } else {
        printf("Error writing override static device token: \r\n key:0x%lx \r\n data: %s \r\n", key, data);
        printf("Write override static device token status is %lu.\r\n", status);
      }
      break;
    case 2:
      if (key > 0xFFF) {
        printf("Invalid key for override static secure token. Key should be less than 0xFFF.\r\n");
        return;
      }
      token_key = key | SL_TOKEN_STATIC_SECURE_DATA_TOKENS; // Set the key to static secure token range
      printf("Writing override token for static secure token...\r\n");
      status = sl_token_manager_set_data(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token_key), data, strlen(data));
      if (SL_STATUS_OK == status) {
        printf("Override static secure token written: \r\n key:0x%lx \r\n data: %s \r\n", key, data);
      } else {
        printf("Error writing override static secure token: \r\n key:0x%lx \r\n data: %s \r\n", key, data);
        printf("Write override static secure token status is %lu.\r\n", status);
      }
      break;
    default:
      printf("Invalid token type.\r\n");
      break;
  }
}

/******************************************************************************
* Reads dynamic tokens to the Common Token Manager
*
* This function implements the CLI command 'read_dynamic_token' and supports:
*   - Dynamic data object tokens
*   - Dynamic override tokens for static device
*   - Dynamic override tokens for static secure
* (see common_token_manager_baremetal.slcp)
******************************************************************************/
void ctm_read_dynamic_token(sl_cli_command_arg_t *arguments)
{
  printf("<<read dynamic token command>>\r\n");
  uint32_t type = sl_cli_get_argument_uint32(arguments, 0);
  uint32_t key = sl_cli_get_argument_uint32(arguments, 1);
  uint32_t length;
  sl_status_t status;
  uint32_t token_key;
  switch (type) {
    case 0:
      printf("Reading dynamic data object token...\r\n");
      status = sl_token_manager_get_size(SL_TOKEN_GET_DYNAMIC_TOKEN(key, 0), &length);
      if (status != SL_STATUS_OK) {
        printf("Token is not present.\r\n");
        return;
      }
      status = sl_token_manager_get_data(SL_TOKEN_GET_DYNAMIC_TOKEN(key, 0), &read_buffer, length);
      if (SL_STATUS_OK == status) {
        read_buffer[length] = '\0';
        printf("Dynamic data object token read: \r\n key:0x%lx \r\n data: %s \r\n", key, read_buffer);
      } else {
        printf("Error reading dynamic data object token for \r\n key:0x%lx \r\n", key);
        printf("Read dynamic data object token status is %lu.\r\n", status);
      }
      break;
    case 1:
      if (key > 0xFFF) {
        printf("Invalid key for static device override token. Key should be less than 0xFFF.\r\n");
        return;
      }
      token_key = key | SL_TOKEN_STATIC_DEVICE_TOKENS; // Set the key to static device token range
      printf("Reading Static Device Override Token...\r\n");
      status = sl_token_manager_get_size(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token_key), &length);
      if (status != SL_STATUS_OK) {
        printf("Token is not present.\r\n");
        return;
      }
      status = sl_token_manager_get_data(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token_key), &read_buffer, length);
      if (SL_STATUS_OK == status) {
        read_buffer[length] = '\0';
        printf("Static device override token read: \r\n key:0x%lx \r\n data: %s \r\n", key, read_buffer);
      } else {
        printf("Error reading static device override token for \r\n key:0x%lx", key);
        printf("Read static device override token status is %lu.\r\n", status);
      }
      break;
    case 2:
      if (key > 0xFFF) {
        printf("Invalid key for static secure override token. Key should be less than 0xFFF.\r\n");
        return;
      }
      token_key = key | SL_TOKEN_STATIC_SECURE_DATA_TOKENS; // Set the key to static secure token range
      printf("Reading Override Token For Static Secure Token...\r\n");
      status = sl_token_manager_get_size(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token_key), &length);
      if (status != SL_STATUS_OK) {
        printf("Token is not present.\r\n");
        return;
      }
      status = sl_token_manager_get_data(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token_key), &read_buffer, length);
      if (SL_STATUS_OK == status) {
        read_buffer[length] = '\0';
        printf("Static secure override token read: \r\n key:0x%lx \r\n data: %s \r\n", key, read_buffer);
      } else {
        printf("Error reading static secure override token for \r\n key:0x%lx", key);
        printf("Read static secure override token status is %lu.\r\n", status);
      }
      break;
    default:
      printf("Invalid token type\r\n");
      break;
  }
}

/******************************************************************************
* Deletes dynamic tokens using Common Token Manager
*
* This function implements the CLI command 'delete_dynamic_token' and supports:
*   - Dynamic tokens
*   - Dynamic override tokens for static device
*   - Dynamic override tokens for static secure
* (see common_token_manager_baremetal.slcp)
******************************************************************************/
void ctm_delete_dynamic_token(sl_cli_command_arg_t *arguments)
{
  printf("<<delete dynamic token command>>\r\n");
  uint32_t type = sl_cli_get_argument_uint32(arguments, 0);
  uint32_t key = sl_cli_get_argument_uint32(arguments, 1);
  sl_status_t status;
  uint32_t token_key;
  switch (type) {
    case 0:
      printf("Delete dynamic token...\r\n");
      status = sl_token_manager_delete_dynamic_token(key);
      if (SL_STATUS_OK == status) {
        printf("Deleted dynamic token with \r\n key:0x%lx \r\n", key);
      } else {
        printf("Error deleting dynamic token with \r\n key:0x%lx \r\n", key);
        printf("Delete token status is %lu.\r\n", status);
      }
      break;
    case 1:
      if (key > 0xFFF) {
        printf("Invalid key for deleting static device override token. Key should be less than 0xFFF.\r\n");
        return;
      }
      printf("Delete static device override token...\r\n");
      token_key = key | SL_TOKEN_STATIC_DEVICE_TOKENS; // Set the key to static device token range
      status = sl_token_manager_delete_dynamic_token(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token_key));
      if (SL_STATUS_OK == status) {
        printf("Deleted static device override token with \r\n key:0x%lx \r\n", key);
      } else {
        printf("Error deleting static device override token with \r\n key:0x%lx \r\n", key);
        printf("Delete token status is %lu.\r\n", status);
      }
      break;
    case 2:
      if (key > 0xFFF) {
        printf("Invalid key for deleting static secure override token. Key should be less than 0xFFF.\r\n");
        return;
      }
      printf("Delete static secure override token...\r\n");
      token_key = key | SL_TOKEN_STATIC_SECURE_DATA_TOKENS; // Set the key to static secure token range
      status = sl_token_manager_delete_dynamic_token(SL_TOKEN_GET_DYNAMIC_OVERRIDE_TOKEN(token_key));
      if (SL_STATUS_OK == status) {
        printf("Deleted static secure override token with \r\n key:0x%lx \r\n", key);
      } else {
        printf("Error deleting static secure override token with \r\n key:0x%lx \r\n", key);
        printf("Delete token status is %lu.\r\n", status);
      }
      break;
    default:
      printf("Invalid token type\r\n");
      break;
  }
}

/******************************************************************************
* Writes counter token using Common Token Manager.
*
* This function implements the CLI command 'write_counter_token'
* (see common_token_manager_baremetal.slcp)
******************************************************************************/
void ctm_write_counter_token(sl_cli_command_arg_t *arguments)
{
  printf("<<write counter token command>>\r\n");
  uint32_t counter_value = sl_cli_get_argument_uint32(arguments, 0);
  sl_status_t status = SL_STATUS_OK;
  // Write the counter value to the token
  status = sl_token_manager_set_data(CTM_SAMPLE_APP_TOK_UINT32_COUNTER, &counter_value, sizeof(counter_value));
  if (status == SL_STATUS_OK) {
    printf("Counter token 0x%x written successfully with counter_value: %lu \r\n", CTM_COUNTER_TOKEN_KEY, counter_value);
  } else {
    printf("Error writing counter token 0x%x with counter_value: %lu \r\n", CTM_COUNTER_TOKEN_KEY, counter_value);
    printf("Write counter token status is %lu.\r\n", status);
  }
}

/******************************************************************************
* Increments counter token using Common Token Manager.
*
* This function implements the CLI command 'increment_counter_token'
* (see common_token_manager_baremetal.slcp)
******************************************************************************/
void ctm_increment_counter_token(sl_cli_command_arg_t *arguments)
{
  printf("<<increment counter token command>>\r\n");
  (void)arguments;
  sl_status_t status = SL_STATUS_OK;
  // Increment the counter value in the token
  status = sl_token_manager_increment_counter(CTM_SAMPLE_APP_TOK_UINT32_COUNTER);
  if (status == SL_STATUS_OK) {
    printf("Counter token 0x%x incremented successfully\r\n", CTM_COUNTER_TOKEN_KEY);
  } else {
    printf("Error incrementing counter token 0x%x \r\n", CTM_COUNTER_TOKEN_KEY);
    printf("Increment counter token status is %lu.\r\n", status);
  }
}

/******************************************************************************
* Reads counter token using Common Token Manager.
*
* This function implements the CLI command 'increment_counter_token'
* (see common_token_manager_baremetal.slcp)
******************************************************************************/
void ctm_read_counter_token(sl_cli_command_arg_t *arguments)
{
  printf("<<read counter token command>>\r\n");
  (void)arguments;
  uint32_t counter_value;
  sl_status_t status = SL_STATUS_OK;
  // Read the counter value from the token
  status = sl_token_manager_get_data(CTM_SAMPLE_APP_TOK_UINT32_COUNTER, &counter_value, sizeof(counter_value));
  if (status == SL_STATUS_OK) {
    printf("Counter token 0x%x read successfully with counter_value: %lu \r\n", CTM_COUNTER_TOKEN_KEY, counter_value);
  } else {
    printf("Error reading counter token 0x%x \r\n", CTM_COUNTER_TOKEN_KEY);
    printf("Read counter token status is %lu.\r\n", status);
  }
}

/******************************************************************************
* Initialize Common Token Manager example.
******************************************************************************/
void ctm_app_init(void)
{
  sl_status_t status;
  status = sl_token_init();
  EFM_ASSERT(status == SL_STATUS_OK);
  printf("\r\nWelcome to the ctm sample application\r\n");
  printf("Before starting, we recommend you to go through the readme file.\r\n");
  printf("Type 'help' to see available commands\r\n");
}
