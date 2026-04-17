/*******************************************************************************
 * @file fw_app.c
 * @brief Firmware Update Example using Host MCU UART
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

#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "socket.h"
#include "sl_utility.h"
#include "errno.h"
#include <string.h>
#include <stdarg.h>
#include "sl_si91x_driver.h"
#include "app.h"
#include "uartdrv.h"

#if defined _SILICON_LABS_32B_SERIES_2_CONFIG_5
#include "sl_iostream_eusart_vcom_config.h"
#include "sl_uartdrv_instances.h"
#elif defined _SILICON_LABS_32B_SERIES_2_CONFIG_1
#include "sl_iostream_usart_vcom_config.h"
#include "sl_uartdrv_instances.h"
#endif

/******************************************************
 *                      Macros
 ******************************************************/
/******************************************************
 *                    Constants
 ******************************************************/

// Receive data length
#define RECV_BUFFER_SIZE (1 * 1024)

#define CHUNK_SIZE          RECV_BUFFER_SIZE //Update this value depending upon 'chunk_size_kb' passed in python script
#define MAX_CHUNK_SIZE      1024
#define MAGIC_WORD          0x900d900d
#define HANDSHAKE_DATA_SIZE 43

#define MAX_LOG_BUF_SIZE 128

/******************************************************
 *               Global Variable
 ******************************************************/
uint32_t start, end;
volatile bool handshake_complete = false;
volatile bool tx_done            = false;

/******************************************************
 *               Function Declarations
 ******************************************************/
sl_status_t update_firmware(void);
sl_status_t send_1k_chunks(uint8_t *buffer, size_t total_length);
void com_port_send(char *buffer, uint16_t buffer_length);
void com_port_receive(uint8_t *buffer, uint16_t buffer_length);
void serial_com_handshake();
void release_update_semaphore();

// Global variable to hold the current state
static firmware_state_t current_state = STATE_HANDSHAKE;

// Semaphore to synchronize firmware update and handshake completion
osSemaphoreId_t update_semaphore;

// Callback triggered when UARTDRV has completed transmission.
static void UART_tx_callback(UARTDRV_Handle_t handle,
                             Ecode_t transferStatus,
                             uint8_t *data,
                             UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)data;
  (void)transferCount;

  if (transferStatus == ECODE_EMDRV_UARTDRV_OK) {
    tx_done = true;
  }
}

void release_update_semaphore()
{
  // Signal the semaphore indicating completion of either handshake or data received
  osSemaphoreRelease(update_semaphore);
}

static void UART_rx_callback(UARTDRV_Handle_t handle,
                             Ecode_t transferStatus,
                             uint8_t *data,
                             UARTDRV_Count_t transferCount)
{
  (void)handle;
  (void)data;
  (void)transferCount;

  if (transferStatus == ECODE_EMDRV_UARTDRV_OK && transferCount > 0) {
    handshake_complete = true;  // Set handshake complete flag
    release_update_semaphore(); // Release semaphore
  }
}

void sl_debug_log(const char *format, ...)
{
  osMutexAcquire(printf_mutex, 0xFFFFFFFFUL);
  va_list args;
  va_start(args, format);

  // Determine length of formatted string
  int length = vsnprintf(NULL, 0, format, args);
  if (length < 0) {
    va_end(args);
    return;
  }

  if (length >= MAX_LOG_BUF_SIZE)
    length = MAX_LOG_BUF_SIZE;

  // Allocate buffer for formatted string
  char buffer[length + 1]; // +1 for null terminator
  vsnprintf(buffer, length + 1, format, args);

  va_end(args);

#if defined _SILICON_LABS_32B_SERIES_2_CONFIG_5
  UARTDRV_Transmit(sl_uartdrv_eusart_exp_handle, (uint8_t *)buffer, strlen(buffer), UART_tx_callback);
#elif defined _SILICON_LABS_32B_SERIES_2_CONFIG_1
  UARTDRV_Transmit(sl_uartdrv_usart_usart1_handle, (uint8_t *)buffer, strlen(buffer), UART_tx_callback);
#endif

  while (!tx_done)
    ;
  tx_done = 0;
  osMutexRelease(printf_mutex);
}

void fw_up_configurator_task(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status                          = SL_STATUS_OK;
  sl_wifi_firmware_version_t firmware_version = { 0 };

  // Create semaphore for firmware update and handshake completion
  update_semaphore = osSemaphoreNew(1, 0, NULL);
  if (update_semaphore == NULL) {
    // Error handling if semaphore creation fails
    LOG_PRINT("Error: Failed to create semaphore for firmware update and handshake.\r\n");
    return;
  }

  while (1) {
    // State machine logic
    switch (current_state) {
      case STATE_HANDSHAKE:
        // Perform handshake asynchronously
        serial_com_handshake();
        // Wait for handshake completion
        if (osSemaphoreAcquire(update_semaphore, osWaitForever) == osOK) {
          LOG_PRINT("\r\n Host MCU is ready for Firmware update process \r\n");
          current_state = STATE_UPDATE_FIRMWARE;
        } else {
          LOG_PRINT("\r\n Handshake failed\r\n");
          current_state = STATE_ERROR;
        }
        break;

      case STATE_UPDATE_FIRMWARE:
        status = update_firmware();
        if (status == SL_STATUS_OK) {
          LOG_PRINT("\r\n Firmware Upgrade Completed\r\n");
          current_state = STATE_COMPLETED;
        } else {
          LOG_PRINT("\r\n Firmware update failed with status %lx\r\n", status);
          current_state = STATE_ERROR;
        }
        break;

      case STATE_COMPLETED:
        firmware_update_operation_complete();

        status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nError while wifi deinit: 0x%lX \r\n", status);
          return;
        } else {
          LOG_PRINT("\r\nWi-Fi Deinit is successful\r\n");
        }
        status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_default_concurrent_v6_configuration, NULL, NULL);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to start Wi-Fi client interface: 0x%lx\r\n", status);
          return;
        }
        LOG_PRINT("\r\nWi-Fi Client interface init\r\n");
        status = sl_wifi_get_firmware_version(&firmware_version);
        if (status != SL_STATUS_OK) {
          LOG_PRINT("\r\nFailed to fetch firmware version: 0x%lx\r\n", status);
          return;
        } else {
          LOG_PRINT("\r\nFirmware version is: %x%x.%d.%d.%d.%d.%d.%d\r\n",
                    firmware_version.chip_id,
                    firmware_version.rom_id,
                    firmware_version.major,
                    firmware_version.minor,
                    firmware_version.security_version,
                    firmware_version.patch_num,
                    firmware_version.customer_id,
                    firmware_version.build_num);
        }

        osThreadTerminate(osThreadGetId());
        break;

      case STATE_ERROR:
        // Suspend thread indefinitely to halt further execution
        osThreadSuspend(osThreadGetId());
        break;
    }
  }
}

void firmware_update_operation_complete()
{
  osDelay(1000);

  //! Indicate firmware update operation completed to Host PC
  com_port_send("done", strlen("done"));

  fw_update_status(true);

  //! Reset the state machine
  handshake_complete = false;
  current_state      = STATE_HANDSHAKE;
}

// wrapper function to send 1k chunks to firmware
sl_status_t send_1k_chunks(uint8_t *buffer, size_t total_length)
{
  size_t chunk_size       = MAX_CHUNK_SIZE; // 1k chunk size
  size_t remaining_length = total_length;
  uint8_t *current_buffer = buffer;

  while (remaining_length > 0) {
    size_t send_length = (remaining_length > chunk_size) ? chunk_size : remaining_length;

    // Call sl_si91x_fwup_load with the current 1k chunk
    sl_status_t result = sl_si91x_fwup_load(current_buffer, send_length);

    // SL_STATUS_SI91X_FW_UPDATE_DONE is a valid status code for completion
    if (result == SL_STATUS_SI91X_FW_UPDATE_DONE) {
      LOG_PRINT("\r\nFirmware update successful !!!: %lx\n", result);
      return result;
    }

    if (result != SL_STATUS_OK) {
      LOG_PRINT("\r\nFirmware update failed !!! : %lx\n", result);
      return result;
    }

    // Move to the next chunk
    current_buffer += send_length;
    remaining_length -= send_length;
  }

  // All chunks sent successfully
  return SL_STATUS_OK;
}

/*==================================================================*/
/**
 * @fn         void_t com_port_send(uint8_t *buffer,uint16_t buffer_length)
 * @param[in]  uint8 *buffer, pointer to the buffer with the data to be sent
 * @param[in]  uint16 buffer_length, number of bytes to send
 * @param[out] None
 * @section description
 * This API is used to send data to the Host PC through the Host UART interface.
 */
void com_port_send(char *buffer, uint16_t buffer_length)
{
  UNUSED_PARAMETER(buffer);
  for (int txCount = 0; txCount < buffer_length; txCount++) {
#if defined _SILICON_LABS_32B_SERIES_2_CONFIG_5
    EUSART_Tx(SL_IOSTREAM_EUSART_VCOM_PERIPHERAL, *buffer++);
#elif defined _SILICON_LABS_32B_SERIES_2_CONFIG_1
    USART_Tx(SL_IOSTREAM_USART_VCOM_PERIPHERAL, *buffer++);
#else
    //! Depends upon Other MCU Tx port
#endif
  }
}

/*==================================================================*/
/**
 * @fn         void com_port_receive(uint8_t *buffer,uint16_t buffer_length)
 * @param[in]  uint8_t *buffer, pointer to the buffer with the data to be received
 * @param[in]  uint16_t buffer_length, number of bytes to send
 * @param[out] None
 * @return     0, 0=success
 * @description
 * This API is used to receive data from Host PC through the Host UART interface.
 */

void com_port_receive(uint8_t *buffer, uint16_t buffer_length)
{
  UNUSED_PARAMETER(buffer);
#if defined _SILICON_LABS_32B_SERIES_2_CONFIG_5 || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4)
  UARTDRV_Receive(sl_uartdrv_eusart_vcom_handle, buffer, buffer_length, UART_rx_callback); //FG25
#elif defined _SILICON_LABS_32B_SERIES_2_CONFIG_1
  UARTDRV_Receive(sl_uartdrv_usart_vcom_handle, buffer, buffer_length, UART_rx_callback); //MG21
#else
  //! Depends upon Other MCU Rx port
#endif
}

// Function to perform handshake with Host PC
void serial_com_handshake()
{
  char recv_data[HANDSHAKE_DATA_SIZE];
  memset(recv_data, 0, sizeof(recv_data));
  static const char pythoncmd[] = "Python Ready for firmware update process";

  LOG_PRINT("\r\nWaiting for Handshake to Perform...\r\n");

  while (!handshake_complete) {
    com_port_receive((uint8_t *)recv_data, strlen(pythoncmd));
    osSemaphoreAcquire(update_semaphore, osWaitForever);

    // Check if received data matches expected acknowledgment
    if (strncmp((const char *)recv_data, pythoncmd, strlen(pythoncmd)) == 0) {
      handshake_complete = true;
      break;
    }
    //! Retry again
    osDelay(1000);
  }

  // Handshake successful, proceed with firmware update process
  com_port_send("Host is ready for Firmware update process", strlen("Host is ready for Firmware update process"));

  osDelay(2000);

  // Invoke completion handler if handshake complete flag is set
  if (handshake_complete) {
    release_update_semaphore();
  }
}

sl_status_t update_firmware()
{
  sl_status_t status                 = SL_STATUS_FAIL;
  uint16_t fwup_chunk_length         = 0;
  uint8_t fwup_chunk_type            = SL_FWUP_RPS_HEADER; //Set to Header
  uint16_t count_chunk               = 1;
  uint16_t total_chunks              = 0;
  uint16_t number_of_remaining_bytes = 0;
  uint32_t fw_image_size             = 0;
  uint8_t recv_buffer[RECV_BUFFER_SIZE];

  LOG_PRINT("\r\nFirmware update start\r\n");
  start = osKernelGetTickCount();

  while (1) {
    // Call corresponding firmware upgrade API based on the chunk type
    if (fwup_chunk_type == SL_FWUP_RPS_HEADER) {

      memset(recv_buffer, 0, sizeof(recv_buffer));

      com_port_send("header", strlen("header"));

      com_port_receive(recv_buffer, sizeof(sl_wifi_firmware_header_t));
      osSemaphoreAcquire(update_semaphore, osWaitForever);

      // Check if the received header contains the magic word
      if (*((uint32_t *)(recv_buffer + 4)) != MAGIC_WORD) {
        // Magic word not found, re-send the header request
        LOG_PRINT("Magic word not found in header, resending...\n");
        continue; // Go back to the beginning of the loop to re-send the header request
      }

      //! Send the first chunk to extract header
      status = sl_wifi_get_firmware_size((char *)recv_buffer, &fw_image_size);
      if (status != SL_STATUS_OK) {
        LOG_PRINT("Unable to fetch firmware size. Status: 0x%lx\n", status);
        continue; // Go back to the beginning of the loop to re-send the header request
      }
      total_chunks              = (fw_image_size / CHUNK_SIZE);
      number_of_remaining_bytes = (fw_image_size % CHUNK_SIZE);

      //Send RPS header which is received as the first chunk
      status = sl_si91x_fwup_start(recv_buffer);

      // Update chunk_type flag
      fwup_chunk_type = SL_FWUP_RPS_CONTENT;
      osDelay(1000);
    } else {
      memset(recv_buffer, 0, sizeof(recv_buffer));

      fwup_chunk_length = (count_chunk <= total_chunks) ? CHUNK_SIZE : number_of_remaining_bytes;

      com_port_send("data", strlen("data"));

      com_port_receive(recv_buffer, fwup_chunk_length);
      osSemaphoreAcquire(update_semaphore, osWaitForever);

      // Send RPS content
      status = send_1k_chunks(recv_buffer, fwup_chunk_length);
      count_chunk++;
    }

    if (status != SL_STATUS_OK) {
      if (status == SL_STATUS_SI91X_FW_UPDATE_DONE) {
        end = osKernelGetTickCount();
        return SL_STATUS_OK;
      } else {
        end = osKernelGetTickCount();
        return status;
      }
    }
  }
  return SL_STATUS_OK;
}
