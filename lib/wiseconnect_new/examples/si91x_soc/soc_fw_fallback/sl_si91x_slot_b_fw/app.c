/*******************************************************************************
 * @file
 * @brief Firmware Update Example Application
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
#include "errno.h"
#include "rsi_debug.h"
#include "sl_board_configuration.h"
#include "sl_net.h"
#include "sl_si91x_fw_fallback.h"
#include "sl_si91x_driver.h"
#include "sl_utility.h"
#include "sl_wifi.h"
#include "socket.h"
#include <string.h>
#include <stddef.h>

#ifdef SLI_SI91X_MCU_INTERFACE
#include "sl_si91x_hal_soc_soft_reset.h"
#endif

/******************************************************
  *                      Macros
  ******************************************************/

/******************************************************
  *                    Constants
  ******************************************************/
//! Server IP address - The IP address of the server where the OTA update will be fetched.
#define SERVER_IP_ADDRESS "192.168.0.158"

//! Server port number - The port on which the server is listening for connections.
#define SERVER_PORT 5005

//! Receive data length - The buffer size for receiving data packets.
#define RECV_BUFFER_SIZE 1027

//! Number of remaining bytes to be processed in the current OTA update.
uint16_t number_of_remaining_bytes = 0;

//! Chunk index - Tracks the current chunk number being processed.
uint16_t chunk = 1;

//! OTA image start address - The memory address where the OTA update will be stored.
static uint32_t ota_image_start_address = 0;

//! OTA image size - The total size of the OTA update in bytes.
static uint32_t ota_image_size = 0;

//! M4 OTA image - The memory location or reference for the M4 core's OTA update.
static uint32_t m4_ota_image = 0;

//! NWP OTA image - The memory location or reference for the NWP OTA update.
static uint32_t ta_ota_image = 0;

//! Buffer to store received data from the server during the firmware update process.
uint8_t recv_buffer[RECV_BUFFER_SIZE];

//! Enumeration representing the different states of the firmware update process.
typedef enum {
  STATE_SENDING_REQUEST,     //!< Sending the request to the server for the firmware update.
  STATE_RECEIVING_RESPONSE,  //!< Receiving the response (firmware data) from the server.
  STATE_PROCESSING_RESPONSE, //!< Processing the received firmware data (e.g., storing, verifying).
  STATE_COMPLETED,           //!< Firmware update process completed successfully.
  STATE_ERROR                //!< An error occurred during the firmware update process.
} firmware_update_state_t;

/* Extern symbols from the linker script */
extern uint32_t __rom_start;
extern uint32_t __rom_length;

/** @brief Default flash address for NWP Slot A. */
#define DEFAULT_NWP_SLOT_A_OFFSET 0x4011000

/** @brief Example firmware image size. */
#define DEFAULT_NWP_IMAGE_SIZE 0x2EFE0

//! Enables or disables toggling of slot information during the firmware update process.
#define SL_APP_TOGGLE_SLOT_INFO 0

//! Enables or disables updating of firmware slot information after a successful firmware update.
#define SL_APP_UPDATE_FIRMWARE_SLOT 0

//! Enables or disables combined image support (handling multiple images in sequence).
#define SL_APP_COMBINED_IMAGE_SUPPORT 0

//! Enables burning NWP security version after Wi-Fi connect (0: disable, 1: enable)
#define SL_APP_BURN_NWP_SECURITY_VERSION 0

// Get the start address of the ROM from the linker script
uint32_t rom_address = (uint32_t)&__rom_start;
// Get the length of the ROM from the linker script
uint32_t rom_length = (uint32_t)&__rom_length;

// OTA firmware upgrade type
uint8_t ota_fw_upgrade_type = 0;

// Flash offset for updater image
uint32_t flash_offset_for_updater_image = 0;

#if SL_APP_COMBINED_IMAGE_SUPPORT
//! Count of combined images processed.
uint8_t combined_image_count = 0;
#endif

// Global variable to store slot information for firmware fallback operations
sl_si91x_fw_ab_slot_management_t global_slot_info = { 0 };

// Initialize the Wi-Fi client interface

/******************************************************
  *               Function Declarations
  ******************************************************/

//! Starts the application, initializing necessary components and handling the main process.
static void application_start(void *argument);

//! Manages the overall firmware update process by handling communication with the server.
static sl_status_t firmware_update_process(int client_socket);

//! Sends a request to the server for a firmware update, specifying the chunk to be received.
static sl_status_t sl_app_sending_request(int client_socket, uint16_t chunk, uint16_t chunk_max_count);

//! Receives a response from the server, extracting the firmware update chunk type and length.
static sl_status_t sl_app_receiving_response(int client_socket, uint8_t *fwup_chunk_type, uint16_t *fwup_chunk_length);

//! Processes the received firmware update chunk, verifying and storing the data appropriately.
static sl_status_t sl_app_processing_response(int client_socket,
                                              uint8_t fwup_chunk_type,
                                              uint16_t fwup_chunk_length,
                                              uint16_t *chunk,
                                              uint16_t *chunk_max_count);

//! Writes firmware chunk data to flash memory using the unified flash write API.
static sl_status_t sl_app_write_firmware_chunk_to_flash(uint8_t *recv_buffer,
                                                        uint16_t fwup_chunk_length,
                                                        uint32_t *current_flash_offset);

//! Displays detailed information about the current firmware slots.
void sl_app_display_ab_slot_info(const sl_si91x_fw_ab_slot_management_t *slot_info);

//! Handles the retrieval and initialization of firmware slot information.
static void sl_app_handle_ab_slot_info(sl_si91x_fw_ab_slot_management_t *app_ab_slot_info);

//! Erases the flash memory region designated for the updater image.
static sl_status_t sl_app_flash_erase_inactive_m4_slot(uint32_t ota_image_size);

//! Calculates the flash offset for the updater image.
static uint32_t sl_app_calculate_flash_offset_for_updater(void);

#if SL_APP_COMBINED_IMAGE_SUPPORT
//! Resets all state variables for processing the next firmware image in a combined image update.
static void sl_app_reset_state_for_next_image(uint16_t *data_chunk,
                                              uint16_t *chunk_max_count,
                                              uint16_t *fwup_chunk_length,
                                              uint8_t *fwup_chunk_type,
                                              firmware_update_state_t *state);
#endif

/******************************************************
  *               Global Variable
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

static sl_wifi_device_configuration_t sl_wifi_firmware_update_configuration = {
  .boot_option         = SL_SI91X_LOAD_NWP_FW_WITH_IMAGE_NUMBER,
  .nwp_fw_image_number = SL_SI91X_NWP_FW_IMAGE_NUMBER_1,
  .mac_address         = NULL,
  .band                = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code         = US,
  .boot_config         = { .oper_mode                  = SL_SI91X_CLIENT_MODE,
                           .coex_mode                  = SL_SI91X_WLAN_ONLY_MODE,
                           .feature_bit_map            = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION),
                           .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
                           .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                           .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_XTAL_CLK
#ifndef SLI_SI91X_MCU_INTERFACE
                                                  | SL_SI91X_RAM_LEVEL_NWP_ALL_AVAILABLE
#else
                                                  | SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC
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

/******************************************************
  *               Function Definitions
  ******************************************************/

void app_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  sl_status_t status                 = SL_STATUS_FAIL;
  struct sockaddr_in server_address  = { 0 }; // Server address structure
  socklen_t socket_length            = sizeof(struct sockaddr_in);
  int client_socket                  = -1;           // Client socket descriptor
  int socket_return_value            = 0;            // Return value for socket operations
  sl_wifi_firmware_version_t version = { 0 };        // Structure to store firmware version
  sl_si91x_fw_ab_slot_management_t app_ab_slot_info; // Structure to manage firmware slots

  UNUSED_PARAMETER(argument); // Avoids compiler warning for unused parameter

  DEBUGOUT("\r\n FW FallBack App is Running......\r\n");

  // This function checks the current active NWP slot information retrieved from the slot management structure.
  // If the active slot is valid (either SLOT_A or SLOT_B), it assigns the corresponding firmware image number
  // to the Wi-Fi firmware update configuration.
  status = sl_si91x_ab_get_slot_info(&app_ab_slot_info);
  if (app_ab_slot_info.nwp_slot_info.current_active_nwp_slot != 0xFF
      && (app_ab_slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_A
          || app_ab_slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_B)) {
    sl_wifi_firmware_update_configuration.nwp_fw_image_number =
      app_ab_slot_info.nwp_slot_info.current_active_nwp_slot - 1;
    DEBUGOUT(" \r\n Assigned NWP Image Number from Slot Info: %d \r\n",
             sl_wifi_firmware_update_configuration.nwp_fw_image_number);
  }

  // Select the default NWP firmware image based on the current fw_image_number.
  status = sl_si91x_select_default_nwp_fw(sl_wifi_firmware_update_configuration.nwp_fw_image_number);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n Failed to select default NWP firmware: %d\r\n", (int16_t)status);
    while (1)
      ; // Halt execution in case of failure
  }

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_firmware_update_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    while (1)
      ; // Halt execution in case of failure
  }

  // Retrieve the current slot information
  sl_app_handle_ab_slot_info(&app_ab_slot_info);

  // Store slot information in global variable for firmware fallback operations
  global_slot_info = app_ab_slot_info;

  // Display the slot information
  sl_app_display_ab_slot_info(&app_ab_slot_info);

  // Retrieve the current firmware version
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\nFailed to fetch firmware version: 0x%lx\r\n", status);
  } else {
    print_firmware_version(&version);
  }

  // Bring up the Wi-Fi client interface
  status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);
  if (status != SL_STATUS_OK) {
#if SL_APP_TOGGLE_SLOT_INFO
    // This API toggles the active firmware slot between Slot A and Slot B for
    // both the M4 and NWP cores. If valid images are not present in both slots,
    // the toggle operation may fail or result in undefined behavior.
    status = sl_si91x_toggle_slot_info(1, 1);
    if (status != SL_STATUS_OK) {
      while (1)
        ;
    }
    sl_si91x_soc_nvic_reset(); //Reset the system to apply the new slot configuration
#endif
    DEBUGOUT("Failed to bring Wi-Fi client interface up: 0x%lx\r\n", status);
    osThreadExit(); // Exit thread on failure
  }
  DEBUGOUT("\r\nWi-Fi Client Connected\r\n");

#if SL_APP_BURN_NWP_SECURITY_VERSION
  {
    uint32_t nwp_fw_addr = 0;
    if (app_ab_slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_A) {
      nwp_fw_addr = app_ab_slot_info.nwp_slot_info.nwp_slot_A.slot_image_offset;
    } else if (app_ab_slot_info.nwp_slot_info.current_active_nwp_slot == SLOT_B) {
      nwp_fw_addr = app_ab_slot_info.nwp_slot_info.nwp_slot_B.slot_image_offset;
    }
    if (nwp_fw_addr != 0) {
      sl_status_t burn_status = sl_si91x_burn_nwp_security_version(nwp_fw_addr);
      if (burn_status != SL_STATUS_OK) {
        DEBUGOUT("\r\nFailed to burn NWP security version: 0x%lX at 0x%lX\r\n", burn_status, nwp_fw_addr);
      } else {
        DEBUGOUT("\r\nBurned NWP security version at 0x%lX\r\n", nwp_fw_addr);
      }
    } else {
      DEBUGOUT("\r\nSkipping NWP security version burn: invalid NWP address\r\n");
    }
  }
#endif

  // Configure the server address
  server_address.sin_family = AF_INET;
  server_address.sin_port   = SERVER_PORT;
  sl_net_inet_addr(SERVER_IP_ADDRESS, &server_address.sin_addr.s_addr);

  // Create a TCP socket
  client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket < 0) {
    DEBUGOUT("\r\nSocket creation failed with BSD error: %d\r\n", errno);
    osThreadExit(); // Exit thread on failure
  }
  DEBUGOUT("\r\nClient socket : %d\r\n", client_socket);

  // Connect to the firmware update server
  socket_return_value = connect(client_socket, (struct sockaddr *)&server_address, socket_length);
  if (socket_return_value < 0) {
    DEBUGOUT("\r\nSocket Connect failed with BSD error: %d\r\n", errno);
    close(client_socket);
    osThreadExit(); // Exit thread on failure
  }
  DEBUGOUT("\r\nConnect to TCP Server Success\r\n");

  // **Start Firmware Update Process**
  status = firmware_update_process(client_socket);
  if (status == SL_STATUS_OK) {
    DEBUGOUT("\r\nFirmware update was successful. Task will continue monitoring.\r\n");
    // reset the system after firmware update
    //sl_si91x_soc_nvic_reset();
  } else {
    DEBUGOUT("\r\nFirmware update failed. Retrying...\r\n");
    osThreadExit();
  }
}

/**
 * @brief Manages the overall firmware update process by handling communication with the server.
 *
 * This function coordinates the sending of requests, receiving of responses,
 * and processing of firmware data chunks. It maintains the state of the firmware
 * update process and handles errors appropriately.
 *
 * @param client_socket The socket descriptor for communication with the server.
 *
 * @return The status of the operation (success, error, or completion).
 */
static sl_status_t firmware_update_process(int client_socket)
{
  uint16_t data_chunk           = 0;                     // Tracks the current firmware data chunk
  uint16_t chunk_max_count      = 0;                     // Total number of chunks expected
  uint16_t fwup_chunk_length    = 0;                     // Length of the received firmware chunk
  uint8_t fwup_chunk_type       = 0;                     // Type of received firmware chunk
  firmware_update_state_t state = STATE_SENDING_REQUEST; // Initial state
  uint32_t status               = SL_STATUS_FAIL;        // Default failure status

  while (1) {
    switch (state) {
      case STATE_SENDING_REQUEST:
        // Send a request to the server for firmware update
        state = sl_app_sending_request(client_socket, data_chunk, chunk_max_count);
        break;

      case STATE_RECEIVING_RESPONSE:
        // Receive the firmware update response from the server
        state = sl_app_receiving_response(client_socket, &fwup_chunk_type, &fwup_chunk_length);
        break;

      case STATE_PROCESSING_RESPONSE:
        // Process the received firmware chunk and update state accordingly
        state =
          sl_app_processing_response(client_socket, fwup_chunk_type, fwup_chunk_length, &data_chunk, &chunk_max_count);
        break;

      case STATE_COMPLETED:
        // Firmware update process completed successfully
        DEBUGOUT("\r\n Firmware Update Completed Successfully \r\n");

        // Perform integrity check on the downloaded firmware
        if (ota_fw_upgrade_type == 1) {
          ota_image_start_address = flash_offset_for_updater_image;
        }
        status = sl_si91x_verify_image(ota_image_start_address);
        if (status != SL_STATUS_SI91X_FW_UPDATE_DONE) {
          DEBUGOUT("\r\nFirmware verify check fail:%lX %lX\r\n", status, ota_image_start_address);
          return STATE_ERROR;
        } else {
          DEBUGOUT("\r\nFirmware verify check success:%lX %lX\r\n", status, ota_image_start_address);

          // Update slot information after image verification
#if SL_APP_UPDATE_FIRMWARE_SLOT
          if (ota_fw_upgrade_type == 0) {
            // Updating M4 firmware slot
            if (m4_ota_image) {

              // This API updates the slot information for the M4 core with the new firmware image.
              status = sl_si91x_ab_upgrade_set_slot_info(ota_image_start_address,
                                                         ota_image_size,
                                                         SL_SI91X_AB_OTA_IMAGE_TYPE_M4);
              if (status != SL_STATUS_OK) {
                DEBUGOUT("Failed to update M4 slot, error: %lu\n", status);
              } else {
                DEBUGOUT("\r\n Successfully updated M4 slot information \r\n");
              }
            }

            // Updating NWP firmware slot
            if (ta_ota_image) {
              // Set the slot information for the NWP core
              // This API updates the slot information for the NWP core with the new firmware image.
              status = sl_si91x_ab_upgrade_set_slot_info(ota_image_start_address,
                                                         ota_image_size,
                                                         SL_SI91X_AB_OTA_IMAGE_TYPE_NWP);
              if (status != SL_STATUS_OK) {
                DEBUGOUT("Failed to update NWP slot, error: %lu\n", status);
              } else {
                DEBUGOUT("\r\n Successfully updated NWP slot information \r\n");
              }
            }
          }
#endif

          // After updating slot info, check if we need to process the next image
#if SL_APP_COMBINED_IMAGE_SUPPORT
          if (!combined_image_count) {
            DEBUGOUT("\r\n Ready for the Second Image \r\n");
            sl_app_reset_state_for_next_image(&data_chunk,
                                              &chunk_max_count,
                                              &fwup_chunk_length,
                                              &fwup_chunk_type,
                                              &state);
          }

          combined_image_count++;
#endif
        }

#if SL_APP_COMBINED_IMAGE_SUPPORT
        if (combined_image_count == 2) {
          // Close the client socket after all images are processed
          close(client_socket);
          return SL_STATUS_OK;
        } else {
          break;
        }
#else
        // Close the client socket after single image is processed
        close(client_socket);
        return SL_STATUS_OK; // Return success for single image
#endif

      case STATE_ERROR:
        // Handle errors encountered during the firmware update process
        DEBUGOUT("\r\nError occurred during firmware update process\r\n");
        close(client_socket);
        return SL_STATUS_FAIL; // Return failure

      default:
        // Handle unexpected states
        DEBUGOUT("\r\nUnknown state encountered\r\n");
        return SL_STATUS_FAIL; // Return failure
    }
  }
}
/**
 * @brief Sends a request to the server for a firmware update, specifying the chunk to be received.
 *
 * This function constructs and sends a request message to the server, indicating
 * the chunk number to be sent. It handles errors related to socket operations and
 * manages the state of the firmware update process.
 *
 * @param client_socket The socket descriptor for communication with the server.
 * @param data_chunk The current chunk number being requested.
 * @param chunk_max_count The maximum number of chunks expected from the server.
 *
 * @return The status of the operation (success, error, or waiting for response).
 */
static sl_status_t sl_app_sending_request(int client_socket, uint16_t data_chunk, uint16_t chunk_max_count)
{
  uint8_t send_buffer[3] = { 0 }; // Buffer to hold the request message

  // Check if the requested chunk exceeds the allowed maximum count
  if (chunk_max_count > 0 && (data_chunk > chunk_max_count)) {
    DEBUGOUT("\r\nChunk %d exceeded chunk_max_count %d. Firmware update failed.\r\n", data_chunk, chunk_max_count);
    return STATE_ERROR;
  }

  // Determine the request type: first chunk (header) or subsequent chunks (content)
  send_buffer[0] = (chunk == 1) ? SL_FWUP_RPS_HEADER : SL_FWUP_RPS_CONTENT;

  // Copy the chunk number into the request buffer (little-endian format)
  memcpy(&send_buffer[1], &chunk, 2);

  // Send the request to the server
  int data_length = send(client_socket, (uint8_t *)send_buffer, 3, 0);
  if (data_length < 0) {
    // If there's a buffer issue, retry sending the request
    if (errno == ENOBUFS)
      return STATE_SENDING_REQUEST; // Retry sending

    // Log other socket errors and return failure
    DEBUGOUT("\r\nFailed to send data, BSD Error Code: %d\r\n", errno);
    return STATE_ERROR;
  }

  // Successfully sent the request, move to the next state (receiving response)
  return STATE_RECEIVING_RESPONSE;
}

static sl_status_t sl_app_receiving_response(int client_socket, uint8_t *fwup_chunk_type, uint16_t *fwup_chunk_length)
{
  uint8_t recv_buffer[3] = { 0 };
  int recv_size          = 3;

  int data_length = recv(client_socket, recv_buffer, recv_size, 0);
  if (data_length < 0) {
    DEBUGOUT("\r\nFailed to receive data, BSD Error Code: %d\r\n", errno);
    return STATE_ERROR;
  }

  *fwup_chunk_type    = recv_buffer[0] & 0x0F; //get the lower 4 bits
  ota_fw_upgrade_type = recv_buffer[0] >> 4;   //get the upper 4 bits
  memcpy(fwup_chunk_length, &recv_buffer[1], 2);
  return STATE_PROCESSING_RESPONSE;
}

/**
 * @brief Processes the received firmware update chunk, verifying and storing the data appropriately.
 *
 * This function handles the received firmware chunk based on its type (header or content).
 * It extracts metadata from the header, erases flash memory if necessary, and writes the
 * firmware content to flash memory. It also manages the state of the firmware update process.
 *
 * @param client_socket The socket descriptor for communication with the server.
 * @param fwup_chunk_type The type of the received firmware chunk (header or content).
 * @param fwup_chunk_length The length of the received firmware chunk.
 * @param data_chunk Pointer to a variable tracking the current firmware data chunk.
 * @param chunk_max_count Pointer to a variable holding the maximum number of chunks expected.
 *
 * @return The status of the operation (success, error, or completion).
 */
static sl_status_t sl_app_processing_response(int client_socket,
                                              uint8_t fwup_chunk_type,
                                              uint16_t fwup_chunk_length,
                                              uint16_t *data_chunk,
                                              uint16_t *chunk_max_count)
{
  uint8_t recv_buffer[fwup_chunk_length]; // Buffer to hold received firmware data chunk
  int recv_size = fwup_chunk_length;
  sl_status_t status;
  ota_image_info_t ota_config;
  static uint32_t current_flash_offset = 0; // Track current flash write offset

  if (recv_size > 0) {

    // Receive the firmware chunk from the server
    int data_length = recv(client_socket, recv_buffer, recv_size, 0);
    if (data_length < 0) {
      DEBUGOUT("\r\nFailed to receive data from remote peer, BSD Error Code: %d\r\n", errno);
      return STATE_ERROR; // Fatal error: Cannot proceed without valid firmware data
    }

    // Check if received chunk is the firmware header
    if (fwup_chunk_type == SL_SI91X_HEADER_PACKET) {

      // Extract firmware metadata from the received header
      status = sl_si91x_ab_upgrade_get_rps_configs(recv_buffer, &ota_config);
      if (status != SL_STATUS_OK) {
        DEBUGOUT("\r\nError: OTA config extraction failed.\r\n");
        return STATE_ERROR; // Fatal error: Unable to parse firmware metadata
      }

      // Calculate total number of chunks required for complete firmware transfer
      *chunk_max_count =
        (ota_config.ota_image_size + SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE - 1) / SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE;
      number_of_remaining_bytes = (ota_config.ota_image_size % SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE);

      ota_image_start_address = ota_config.ota_image_offset;
      ota_image_size          = ota_config.ota_image_size;

      // Identify firmware type (M4 or NWP)
      if (ota_config.ota_image_type == SL_SI91X_M4_RPS_BIT) {
        m4_ota_image = 1;
      } else if (ota_config.ota_image_type == SL_SI91X_NWP_RPS_BIT) {
        ta_ota_image = 1;
      }

      current_flash_offset = ota_config.ota_image_offset;

      // Erase flash memory before writing new firmware

      DEBUGOUT("\r\nFirmware flash erase progress.....\r\n");

      uint32_t remaining_size     = ota_config.ota_image_size;
      uint32_t current_erase_addr = ota_config.ota_image_offset;

      if (ota_fw_upgrade_type == 1) {
        // Erase flash memory in chunks(4096)
        DEBUGOUT("\r\nFirmware flash erase progress for updater image.....%lX %lX \r\n",
                 remaining_size,
                 current_erase_addr);
        status = sl_app_flash_erase_inactive_m4_slot(ota_config.ota_image_size);
        if (status != SL_STATUS_OK) {
          DEBUGOUT("\r\nFirmware flash erase fail for updater image: %lX\r\n", status);
          return STATE_ERROR; // Fatal error: Cannot write new firmware without erasing flash
        }
        DEBUGOUT("\r\nFirmware flash erase success for updater image:%lX\r\n", status);
      } else {
        // Erase flash memory in chunks(4096)
        while (remaining_size > 0) {
          uint32_t erase_size = (remaining_size > SL_SI91X_CHUNK_LENGTH) ? SL_SI91X_CHUNK_LENGTH : remaining_size;

          status = sl_si91x_flash_erase(current_erase_addr, erase_size);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("\r\nFirmware flash erase fail at address 0x%lX: %lX\r\n", current_erase_addr, status);
            return STATE_ERROR; // Fatal error: Cannot write new firmware without erasing flash
          }
          current_erase_addr += erase_size;
          remaining_size -= erase_size;
        }
      }
      DEBUGOUT("\r\nFirmware flash erase success:%lu\r\n", status);

      DEBUGOUT("\r\nchunk writing progress.....\r\n");
    } else {
      // Determine size of the current firmware chunk to write
      fwup_chunk_length =
        (*data_chunk < *chunk_max_count)
          ? SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE
          : (number_of_remaining_bytes == 0 ? SL_SI91X_MAX_OTA_IMAGE_CHUNK_SIZE : number_of_remaining_bytes);

      // Write firmware chunk to flash memory using dedicated function
      status = sl_app_write_firmware_chunk_to_flash(recv_buffer, fwup_chunk_length, &current_flash_offset);
      if (status != SL_STATUS_OK) {
        return STATE_ERROR;
      }
    }

    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\nFirmware update operation failed.\r\n");
      return STATE_ERROR; // Fatal error: Firmware update process failed
    }

    // Check if all chunks have been processed
    if (*data_chunk >= (*chunk_max_count)) {
      return STATE_COMPLETED; // Firmware update completed successfully
    } else {
      (*data_chunk)++; // Move to the next chunk
      chunk++;
      return STATE_SENDING_REQUEST; // Request the next firmware chunk from the server
    }
  }

  return STATE_COMPLETED; // Firmware update completed successfully
}
/**
 * @brief Displays detailed information about the current firmware slots.
 *
 * This function prints the slot information, including the magic word, slot IDs,
 * image offsets, image sizes, and currently active slots for both M4 and NWP cores.
 *
 * @param slot_info Pointer to the structure containing slot information.
 */
void sl_app_display_ab_slot_info(const sl_si91x_fw_ab_slot_management_t *slot_info)
{
  if (slot_info == NULL) {
    DEBUGOUT("\r\nError: NULL pointer passed for slot_info!\n");
    return;
  }

  DEBUGOUT("\r\n========= Firmware Fallback A/B Slot Information =========");
  DEBUGOUT("\r\nMagic Word: 0x%lX", (unsigned long)slot_info->slot_magic_word);

  // M4 Slot Information
  DEBUGOUT("\r\n---- M4 Slot Information ----");
  DEBUGOUT("\r\nM4 Slot A:");
  DEBUGOUT("\r\n  Slot ID: %u", slot_info->m4_slot_info.m4_slot_A.slot_id);
  DEBUGOUT("\r\n  Image Offset: 0x%lX",
           (unsigned long)slot_info->m4_slot_info.m4_slot_A.slot_image_offset & 0x00FFFFFF);
  DEBUGOUT("\r\n  Image Size: 0x%lX bytes", (unsigned long)slot_info->m4_slot_info.m4_slot_A.image_size);

  DEBUGOUT("\r\nM4 Slot B:");
  DEBUGOUT("\r\n  Slot ID: %u", slot_info->m4_slot_info.m4_slot_B.slot_id);
  DEBUGOUT("\r\n  Image Offset: 0x%lX",
           (unsigned long)slot_info->m4_slot_info.m4_slot_B.slot_image_offset & 0x00FFFFFF);
  DEBUGOUT("\r\n  Image Size: 0x%lX bytes", (unsigned long)slot_info->m4_slot_info.m4_slot_B.image_size);

  DEBUGOUT("\r\nCurrently Active M4 Slot: %c",
           slot_info->m4_slot_info.current_active_M4_slot == SLOT_A
             ? 'A'
             : (slot_info->m4_slot_info.current_active_M4_slot == SLOT_B ? 'B' : 'N'));

  // NWP Slot Information
  DEBUGOUT("\r\n---- NWP Slot Information ----");
  DEBUGOUT("\r\nNWP Slot A:");
  DEBUGOUT("\r\n  Slot ID: %u", slot_info->nwp_slot_info.nwp_slot_A.slot_id);
  DEBUGOUT("\r\n  Image Offset: 0x%lX",
           (unsigned long)slot_info->nwp_slot_info.nwp_slot_A.slot_image_offset & 0x00FFFFFF);
  DEBUGOUT("\r\n  Image Size: 0x%lX bytes", (unsigned long)slot_info->nwp_slot_info.nwp_slot_A.image_size);

  DEBUGOUT("\r\nNWP Slot B:");
  DEBUGOUT("\r\n  Slot ID: %u", slot_info->nwp_slot_info.nwp_slot_B.slot_id);
  DEBUGOUT("\r\n  Image Offset: 0x%lX",
           (unsigned long)slot_info->nwp_slot_info.nwp_slot_B.slot_image_offset & 0x00FFFFFF);
  DEBUGOUT("\r\n  Image Size: 0x%lX bytes", (unsigned long)slot_info->nwp_slot_info.nwp_slot_B.image_size);

  DEBUGOUT("\r\nCurrently Active NWP Slot: %c",
           slot_info->nwp_slot_info.current_active_nwp_slot == SLOT_A
             ? 'A'
             : (slot_info->nwp_slot_info.current_active_nwp_slot == SLOT_B ? 'B' : 'N'));

  DEBUGOUT("\r\nSlot Structure CRC: 0x%lX", (unsigned long)slot_info->slot_struct_crc);
  DEBUGOUT("\r\n=============================================\r\n");
}

/**
 * @brief Handles the retrieval and initialization of firmware slot information.
 *
 * This function retrieves the current slot information and, if not available,
 * loads default slot information for both M4 and NWP cores.
 *
 * @param app_ab_slot_info Pointer to the structure to store the slot information.
 */
static void sl_app_handle_ab_slot_info(sl_si91x_fw_ab_slot_management_t *app_ab_slot_info)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Retrieve the current slot information
  status = sl_si91x_ab_get_slot_info(app_ab_slot_info);

  if (status == SL_STATUS_OK) {
    // Slot information is available, print it
    DEBUGOUT("\r\nSlot information retrieved successfully.\r\n");
    return;
  }

  // If slot information is not available, log the error and load default information
  DEBUGOUT("\r\nSlot information is not available. Loading default slot information.\r\n");

  // Set the slot info for the M4 core
  status =
    sl_si91x_ab_upgrade_set_slot_info(rom_address - SL_SI91X_CHUNK_LENGTH, rom_length, SL_SI91X_AB_OTA_IMAGE_TYPE_M4);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("Failed to update M4 slot, error: %lu\n", status);
  } else {
    DEBUGOUT("\r\n Successfully updated M4 slot information \r\n");
  }

  // Set the slot info for the NWP core
  status = sl_si91x_ab_upgrade_set_slot_info(DEFAULT_NWP_SLOT_A_OFFSET,
                                             DEFAULT_NWP_IMAGE_SIZE,
                                             SL_SI91X_AB_OTA_IMAGE_TYPE_NWP);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("Failed to update NWP slot, error: %lu\n", status);
  } else {
    DEBUGOUT("Successfully updated NWP slot information\r\n");
  }

  // Read back the updated slot information
  DEBUGOUT("\r\nReading back updated slot information...\r\n");
  status = sl_si91x_ab_get_slot_info(app_ab_slot_info);
  if (status == SL_STATUS_OK) {
    DEBUGOUT("\r\nUpdated slot information retrieved successfully.\r\n");
  } else {
    DEBUGOUT("\r\nFailed to retrieve updated slot information, error: %lu\n", status);
  }
}

/**
 * @brief Writes a firmware chunk to flash memory at the specified offset.
 *
 * This function prepares and writes a firmware chunk to flash memory using the
 * firmware fallback configuration structure. For updater images, it determines
 * the correct inactive M4 slot offset and writes the chunk accordingly. The
 * function handles both regular and updater firmware images, updating the flash
 * offset as needed and performing error checks for slot validity.
 * 
 * Chunk 1 is using for the extarct the header information and from chunk 2 also have 
 *
 * @param recv_buffer Pointer to the buffer containing the firmware chunk data.
 * @param fwup_chunk_length Length of the firmware chunk to be written.
 * @param current_flash_offset Pointer to the current flash write offset; may be updated.
 * @return sl_status_t Status of the flash write operation (success or error).
 */
static sl_status_t sl_app_write_firmware_chunk_to_flash(uint8_t *recv_buffer,
                                                        uint16_t fwup_chunk_length,
                                                        uint32_t *current_flash_offset)
{
  sl_status_t status                      = SL_STATUS_FAIL;
  sl_si91x_fw_fallback_config_t fw_config = { 0 };
  static uint32_t updater_flash_offset    = 0; // Track the offset within inactive slot for updater images
  uint32_t flash_offset_to_use            = *current_flash_offset;

  // Configure the firmware fallback config structure
  fw_config.ota_image_data_length = fwup_chunk_length;
  fw_config.ota_image_chunk_type  = SL_SI91X_DATA_PACKET;
  fw_config.m4_updater_ota_flag   = (ota_fw_upgrade_type == 1) ? SL_SI91X_M4_UPDATER_OTA_FLAG
                                                               : 0x00; // Set based on image type
  fw_config.ota_image_type        = ota_fw_upgrade_type;

  // For updater images (ota_fw_upgrade_type = 1), use flash_offset_for_updater_image as base
  if (ota_fw_upgrade_type == 1) {
    if (chunk == 2) {
      // For chunk 2, start from the base updater image offset
      flash_offset_to_use  = flash_offset_for_updater_image;
      updater_flash_offset = 0; // Reset the incremental offset
    } else {
      // For subsequent chunks, use base + accumulated offset
      flash_offset_to_use = flash_offset_for_updater_image + updater_flash_offset;
    }
  } else {
    // For application images (ota_fw_upgrade_type != 1), use the original flash offset
    flash_offset_to_use = *current_flash_offset;
  }

  // Check for error from the calculation function
  if (flash_offset_to_use == 0) {
    return STATE_ERROR;
  }

  fw_config.ota_image_flash_offset = flash_offset_to_use;

  // Write firmware chunk to flash memory
  if (chunk == 2) {
    // Special handling for chunk 2: split into header and data writes
    // Write the first 64 bytes (header) to flash
    fw_config.ota_image_chunk_type  = SL_SI91X_HEADER_PACKET;
    fw_config.ota_image_data_length = SLI_SI91X_RPS_HEADER_SIZE;

    status = sl_si91x_fw_fallback_ota_flash_write(&fw_config, recv_buffer);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\nFirmware flash Write fail (header):%lu\r\n", status);
      return STATE_ERROR;
    }

    // Write the remaining bytes (firmware data) to flash
    fw_config.ota_image_chunk_type  = SL_SI91X_DATA_PACKET;
    fw_config.ota_image_data_length = (fwup_chunk_length - SLI_SI91X_RPS_HEADER_SIZE);

    // Calculate the flash offset for the remaining data
    uint32_t remaining_data_offset   = flash_offset_to_use + SLI_SI91X_RPS_HEADER_SIZE;
    fw_config.ota_image_flash_offset = remaining_data_offset;

    status = sl_si91x_fw_fallback_ota_flash_write(&fw_config, recv_buffer + SLI_SI91X_RPS_HEADER_SIZE);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\nFirmware flash Write fail (firmware):%lu\r\n", status);
      return STATE_ERROR;
    }

    // Update offsets for next chunk
    if (ota_fw_upgrade_type == 1) {
      updater_flash_offset += fwup_chunk_length; // Increment by full chunk length
    }
    *current_flash_offset += fwup_chunk_length; // Also update the passed parameter
  } else {
    // All other chunks: write as single chunk
    status = sl_si91x_fw_fallback_ota_flash_write(&fw_config, recv_buffer);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\nFirmware flash Write fail:%lu\r\n", status);
      return STATE_ERROR;
    }

    // Update offsets for next chunk
    if (ota_fw_upgrade_type == 1) {
      updater_flash_offset += fwup_chunk_length; // Increment by chunk length
    }
    *current_flash_offset += fwup_chunk_length; // Also update the passed parameter
  }

  return SL_STATUS_OK;
}

/**
 * @brief Calculates the flash offset for updater images based on the current active slot.
 *
 * This function determines the correct inactive M4 slot offset for updater images
 * based on the current active slot and returns the appropriate flash offset.
 *
 * @return uint32_t The calculated flash offset for the updater image.
 */
static uint32_t sl_app_calculate_flash_offset_for_updater(void)
{
  static uint32_t updater_flash_offset_updater = 0;
  uint32_t flash_offset_to_use                 = 0;

  // Determine the M4 inactive slot based on current active slot
  if (global_slot_info.m4_slot_info.current_active_M4_slot == SLOT_A) {
    // If Slot A is active, use Slot B's offset
    updater_flash_offset_updater = global_slot_info.m4_slot_info.m4_slot_B.slot_image_offset;

    // Check for invalid slot offset values (0xFF or 0x00)
    if (updater_flash_offset_updater == 0xFFFFFFFF || updater_flash_offset_updater == 0x00) {
      DEBUGOUT("\r\nError: Invalid Slot B offset (0x%lX) for updater image\r\n",
               (unsigned long)updater_flash_offset_updater);
      return 0; // Return 0 to indicate error
    }
  } else if (global_slot_info.m4_slot_info.current_active_M4_slot == SLOT_B) {
    // If Slot B is active, use Slot A's offset
    updater_flash_offset_updater = global_slot_info.m4_slot_info.m4_slot_A.slot_image_offset;

    // Check for invalid slot offset values (0xFF or 0x00)
    if (updater_flash_offset_updater == 0xFFFFFFFF || updater_flash_offset_updater == 0x00) {
      DEBUGOUT("\r\nError: Invalid Slot A offset (0x%lX) for updater image\r\n",
               (unsigned long)updater_flash_offset_updater);
      return 0; // Return 0 to indicate error
    }
  } else {
    DEBUGOUT("\r\nError: Unknown current_active_M4_slot value: %d\r\n",
             global_slot_info.m4_slot_info.current_active_M4_slot);
    return 0;
  }

  // Additional check for updater OTA images: ensure updater_flash_offset matches any M4 slot offset address
  if (updater_flash_offset_updater != global_slot_info.m4_slot_info.m4_slot_A.slot_image_offset
      && updater_flash_offset_updater != global_slot_info.m4_slot_info.m4_slot_B.slot_image_offset) {
    DEBUGOUT("\r\nError: updater_flash_offset (0x%lX) does not match any M4 slot offset address\r\n",
             (unsigned long)updater_flash_offset_updater);
    DEBUGOUT("\r\nSlot A offset: 0x%lX, Slot B offset: 0x%lX\r\n",
             (unsigned long)global_slot_info.m4_slot_info.m4_slot_A.slot_image_offset,
             (unsigned long)global_slot_info.m4_slot_info.m4_slot_B.slot_image_offset);
    return 0; // Return 0 to indicate error
  }

  DEBUGOUT("\r\nInitial updater_flash_offset: 0x%lX\r\n", (unsigned long)updater_flash_offset_updater);

  // Use the updater flash offset for all chunks
  flash_offset_to_use = updater_flash_offset_updater;
  DEBUGOUT("\r\nflash_offset_to_use for updater image: 0x%lX\r\n", (unsigned long)flash_offset_to_use);

  return flash_offset_to_use;
}

/**
 * @brief Erases the flash memory region designated for the updater image.
 *
 * This function calculates the correct flash offset for the updater image based on the
 * inactive M4 slot and erases the required flash region in 4096-byte chunks.
 *
 * @param ota_image_size The size of the updater image to erase in flash.
 * @return sl_status_t Returns SL_STATUS_OK on success, or an error code on failure.
 */
static sl_status_t sl_app_flash_erase_inactive_m4_slot(uint32_t ota_image_size)
{
  sl_status_t status                  = SL_STATUS_FAIL;
  uint32_t updater_flash_erase_offset = 0;

  updater_flash_erase_offset = sl_app_calculate_flash_offset_for_updater();

  if (updater_flash_erase_offset == 0) {
    DEBUGOUT("\r\nError: Invalid updater flash offset\r\n");
    return SL_STATUS_FAIL;
  }

  flash_offset_for_updater_image = updater_flash_erase_offset;
  DEBUGOUT("\r\nFirmware flash erase 0x%lX %lX:\r\n", flash_offset_for_updater_image, ota_image_size);
  // Erase flash memory in chunks (4096 bytes)
  while (ota_image_size > 0) {
    uint32_t erase_size = (ota_image_size > SL_SI91X_CHUNK_LENGTH) ? SL_SI91X_CHUNK_LENGTH : ota_image_size;

    status = sl_si91x_flash_erase(updater_flash_erase_offset, erase_size);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\nFirmware flash erase fail at address 0x%lX: %lX\r\n", updater_flash_erase_offset, status);
      return status; // Fatal error: Cannot write new firmware without erasing flash
    }
    updater_flash_erase_offset += erase_size;
    ota_image_size -= erase_size;
  }
  return status;
}

#if SL_APP_COMBINED_IMAGE_SUPPORT
/**
 * @brief Resets all state variables for processing the next firmware image in a combined image update.
 *
 * This function resets all the state variables, counters, and flags needed to process
 * the next firmware image when handling combined images (e.g., M4 + NWP images).
 *
 * @param data_chunk Pointer to the current firmware data chunk counter (will be reset to 0).
 * @param chunk_max_count Pointer to the maximum chunk count (will be reset to 0).
 * @param fwup_chunk_length Pointer to the firmware chunk length (will be reset to 0).
 * @param fwup_chunk_type Pointer to the firmware chunk type (will be reset to 0).
 * @param state Pointer to the firmware update state (will be reset to STATE_SENDING_REQUEST).
 */
static void sl_app_reset_state_for_next_image(uint16_t *data_chunk,
                                              uint16_t *chunk_max_count,
                                              uint16_t *fwup_chunk_length,
                                              uint8_t *fwup_chunk_type,
                                              firmware_update_state_t *state)
{
  // Reset chunk tracking variables
  *data_chunk        = 0;
  *chunk_max_count   = 0;
  *fwup_chunk_length = 0;
  *fwup_chunk_type   = 0;
  *state             = STATE_SENDING_REQUEST;

  // Reset global state variables
  number_of_remaining_bytes      = 0;
  chunk                          = 1;
  ota_image_start_address        = 0;
  ota_image_size                 = 0;
  m4_ota_image                   = 0;
  ta_ota_image                   = 0;
  ota_fw_upgrade_type            = 0;
  flash_offset_for_updater_image = 0;
}
#endif

// End of file