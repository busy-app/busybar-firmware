/***************************************************************************/ /**
 * @file
 * @brief Flash Read Write Example Application
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

#include "sl_net.h"
#include "sl_utility.h"
#include "cmsis_os2.h"
#include "sl_wifi_callback_framework.h"
#include "sl_si91x_driver.h"
/******************************************************
 *               Variable Definitions
 ******************************************************/

#define M4_FLASH_ADDRESS  0x8300000
#define NWP_FLASH_ADDRESS 0

#define LENGTH_OF_BUFFER           (1024 * 4)
#define DISPLAY_LENGTH             64
#define FLASH_SECTOR_ERASE_ENABLE  1
#define FLASH_SECTOR_ERASE_DISABLE 0

unsigned char data_read_buffer[LENGTH_OF_BUFFER];
unsigned char data_write_buffer[LENGTH_OF_BUFFER];

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

/******************************************************
 *               Function Declarations
 ******************************************************/

static void application_start(void *argument);
static void print_data(const char *message, const unsigned char *data, uint32_t length);

/******************************************************
 *               Function Definitions
 ******************************************************/

static void print_data(const char *message, const unsigned char *data, uint32_t length)
{
  printf("%s", message);
  for (uint32_t i = 0; i < length; i++) {
    printf("%02X ", data[i]);
    if ((i + 1) % 16 == 0)
      printf("\r\n");
  }
}

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, NULL, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("\r\nFailed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }

  printf("\r\nApplication started.\r\n");

  // Fill data_write_buffer with test data
  for (int i = 0; i < LENGTH_OF_BUFFER; i++) {
    data_write_buffer[i] = i;
  }

  print_data("\r\nData to write to M4 flash memory (first 64 bytes):\r\n", data_write_buffer, DISPLAY_LENGTH);

  // M4 flash memory
  status = (int)sl_si91x_command_to_write_common_flash(M4_FLASH_ADDRESS,
                                                       data_write_buffer,
                                                       LENGTH_OF_BUFFER,
                                                       FLASH_SECTOR_ERASE_DISABLE);
  if (status == SL_STATUS_OK) {
    printf("\r\nData successfully written to M4 flash memory.\r\n");
  } else {
    printf("\r\nError writing data to M4 flash memory.\r\n");
    return;
  }

  // clear data_read_buffer with 0x00
  memset(data_read_buffer, 0x00, LENGTH_OF_BUFFER * sizeof(unsigned char));

  // Read from M4 flash memory
  memcpy((uint8_t *)data_read_buffer, (uint8_t *)M4_FLASH_ADDRESS, LENGTH_OF_BUFFER);

  print_data("\r\nData read from M4 flash memory (first 64 bytes):\r\n", data_read_buffer, DISPLAY_LENGTH);

  // Check if the read data matches the written data
  if (memcmp(data_read_buffer, data_write_buffer, LENGTH_OF_BUFFER * sizeof(unsigned char)) == 0) {
    printf("\r\nData read from M4 flash memory matches written data.\r\n");
  } else {
    printf("\r\nData read from M4 flash memory does NOT match written data.\r\n");
    return;
  }

  // NWP flash memory erase
  status = (int)sl_si91x_command_to_write_common_flash(NWP_FLASH_ADDRESS,
                                                       data_write_buffer,
                                                       LENGTH_OF_BUFFER,
                                                       FLASH_SECTOR_ERASE_ENABLE);
  if (status == SL_STATUS_OK) {
    printf("\r\nSuccessfully erased the NWP flash memory.\r\n");
  } else {
    printf("\r\nError erasing NWP flash memory.\r\n");
    return;
  }

  print_data("\r\nData to write to NWP flash memory (first 64 bytes):\r\n", data_write_buffer, DISPLAY_LENGTH);

  // NWP flash memory write
  status = (int)sl_si91x_command_to_write_common_flash(NWP_FLASH_ADDRESS,
                                                       data_write_buffer,
                                                       LENGTH_OF_BUFFER,
                                                       FLASH_SECTOR_ERASE_DISABLE);
  if (status == SL_STATUS_OK) {
    printf("\r\nSuccessfully written to the NWP flash memory.\r\n");
  } else {
    printf("\r\nError writing to NWP flash memory.\r\n");
    return;
  }

  memset(data_read_buffer, 0x00, LENGTH_OF_BUFFER * sizeof(unsigned char));

  // Read from NWP flash memory
  status = (int)sl_si91x_command_to_read_common_flash(NWP_FLASH_ADDRESS, LENGTH_OF_BUFFER, data_read_buffer);

  if (status == SL_STATUS_OK) {
    print_data("\r\nData successfully read from NWP flash memory (first 64 bytes):\r\n",
               data_read_buffer,
               DISPLAY_LENGTH);
  } else {
    printf("\r\nError reading data from NWP flash memory.\r\n");
    return;
  }

  // Check if the read data matches the written data
  if (memcmp(data_read_buffer, data_write_buffer, LENGTH_OF_BUFFER * sizeof(unsigned char)) == 0) {
    printf("\r\nData read from NWP flash memory matches written data.\r\n");
  } else {
    printf("\r\nData read from NWP flash memory does NOT match written data.\r\n");
    return;
  }

  printf("\r\nSuccessfully demonstrated the flash read write example.\r\n");

  while (1) {
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#else
    osDelay(osWaitForever);
#endif
  }
}
