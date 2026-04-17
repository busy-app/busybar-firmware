/***************************************************************************/ /**
 * @file
 * @brief
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
#pragma once

#ifndef _SL_RSI_UTILITY_H_
#define _SL_RSI_UTILITY_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "sl_status.h"
#include "sl_constants.h"
#include "sl_wifi_constants.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_protocol_types.h"
#include "sl_utility.h"
#include "sl_si91x_driver.h"
#include "sl_wifi_device.h"
#include "sl_si91x_types.h"
#include "sli_wifi_types.h"

//! @cond Doxygen_Suppress

/// Internal structure for representing a TLS extension
typedef struct {
  uint16_t type;   ///< Specifies the TLS extension type.
  uint16_t length; ///< Length of the value[] field.
  uint8_t value[]; ///< Data corresponding to the specified extension type.
} sli_si91x_tls_extension_info_t;

/// Low Transmit Power Threshold for Wi-Fi.
#define SLI_SI91X_LOW_TRANSMIT_POWER_THRESHOLD 6

/// Medium Transmit Power Threshold for Wi-Fi.
#define SLI_SI91X_MEDIUM_TRANSMIT_POWER_THRESHOLD 4

/**
 * @brief Defines the priority level for the event thread.
 *
 * This macro sets the priority level for the event thread.
 * The priority level is set to `osPriorityRealtime1`, which is a real-time priority level.
 * 
 * @note
 * - The priority level of this thread should be highest among all the threads in the system.
 */
#ifndef SL_WLAN_EVENT_THREAD_PRIORITY
#define SL_WLAN_EVENT_THREAD_PRIORITY osPriorityRealtime1
#endif

/**
 * Stack size of the event handler thread that processes all Wi-Fi and networking callbacks.
 * This value can be overridden by defining a new value for SL_SI91X_EVENT_HANDLER_STACK_SIZE in your project or
 * adding -DSL_SI91X_EVENT_HANDLER_STACK_SIZE=<new value> to your compiler command line options.
 */
#ifndef SL_SI91X_EVENT_HANDLER_STACK_SIZE
#define SL_SI91X_EVENT_HANDLER_STACK_SIZE 1536
#endif
typedef bool (*sli_si91x_wifi_buffer_comparator)(const sl_wifi_buffer_t *buffer, const void *userdata);
typedef uint32_t sl_si91x_host_timestamp_t;

void sli_handle_wifi_beacon(sl_wifi_system_packet_t *packet);

typedef void (*sli_si91x_host_atomic_action_function_t)(void *user_data);
typedef uint8_t (*sli_si91x_compare_function_t)(sl_wifi_buffer_t *node, void *user_data);
typedef void (*sli_si91x_node_free_function_t)(sl_wifi_buffer_t *node);

/* Indicates the current performance profile */
extern sl_wifi_system_performance_profile_t current_performance_profile;
extern volatile uint32_t tx_command_queues_status;
extern volatile uint32_t tx_socket_command_queues_status;
extern volatile uint32_t tx_socket_data_queues_status;
extern volatile uint32_t tx_generic_socket_data_queues_status;

extern volatile uint32_t tx_command_queues_command_in_flight_status;
extern volatile uint8_t tx_socket_command_command_in_flight_queues_status;

/* Function converts NWP client info to SDK client info */
sl_status_t sli_convert_si91x_wifi_client_info(sl_wifi_client_info_response_t *client_info_response,
                                               const sli_wifi_client_info_response *sli_wifi_client_info_response);

/* Function converts NWP events to SDK events */
sl_wifi_event_t sli_convert_si91x_event_to_sl_wifi_event(uint32_t command, uint16_t frame_status);

/* Function used to set whether tcp auto close is enabled or disabled */
void sli_save_tcp_auto_close_choice(bool is_tcp_auto_close_enabled);

/* Function used to check whether tcp auto close is enabled or disabled */
bool sli_is_tcp_auto_close_enabled();
void sli_si91x_save_tcp_ip_total_config_select_request(uint8_t tcp_ip_total_select);
uint8_t sli_si91x_get_tcp_ip_total_config_select_request();

/* Function used to set whether card ready is required or not */
void sli_set_card_ready_required(bool card_ready_required);

/* Function used to check whether card ready is required or not */
bool sli_get_card_ready_required();

/* Function used to set the maximum transmission power */
void sli_save_max_tx_power(uint8_t max_scan_tx_power, uint8_t max_join_tx_power);

/* Function used to get maximum transmission power */
sl_wifi_max_tx_power_t sli_get_max_tx_power();

/* Function used to set maximum transmission power to default value(31 dBm) */
void sli_reset_max_tx_power();

/* Function used to set the current performance profile */
void sli_save_wifi_current_performance_profile(const sl_wifi_performance_profile_v2_t *profile);

/* Function used to get current wifi performance profile */
void sli_get_wifi_current_performance_profile(sl_wifi_performance_profile_v2_t *profile);

/* Function used to set the bluetooth performance profile */
void sli_save_bt_current_performance_profile(const sl_bt_performance_profile_t *profile);

/* Function used to retrieve bluetooth performance profile */
void sli_get_bt_current_performance_profile(sl_bt_performance_profile_t *profile);

/* Function used to zero out the coex performance profile */
void sli_reset_coex_current_performance_profile(void);

/* Function used to update the boot configuration */
void sli_save_boot_configuration(const sl_wifi_system_boot_configuration_t *boot_configuration);

/* Function used to retrieve the boot configuration */
void sli_get_saved_boot_configuration(sl_wifi_system_boot_configuration_t *boot_configuration);
/***************************************************************************/ /**
 * @brief
 *   Initializes new task register index for storing firmware status.
 *
 * @details
 *   This function sets up the task register index to store the firmware status in thread-specific storage.
 *   For all the threads at this index of the thread local array firmware status will be stored.
 *
 * @return
 *   sl_status_t. See [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details.
 ******************************************************************************/
sl_status_t sli_fw_status_storage_index_init(void);

/***************************************************************************/ /**
 * @brief
 *   Get the Efuse Data content from flash.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_wifi_init should be called before this API.
 * @param[out] efuse_data
 *   @ref sli_wifi_efuse_data_t object that contains the Manufacturing software version.
 *   efuse_data_type which holds the type of efuse data to be read.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 * @note
 *   This API is not supported in the current release.
 ******************************************************************************/
sl_status_t sli_si91x_get_flash_efuse_data(sli_wifi_efuse_data_t *efuse_data, uint8_t efuse_data_type);

/***************************************************************************/ /**
 * @brief
 *   Get the Efuse Data content from driver context.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_wifi_init should be called before this API.
 * @param[out] efuse_data
 *   @ref sli_wifi_efuse_data_t object that contains the Manufacturing software version.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
void sli_si91x_get_efuse_data(sli_wifi_efuse_data_t *efuse_data);

/***************************************************************************/ /**
 * @brief
 *   Set the Efuse Data content in driver context.
 * @pre Pre-conditions:
 * - 
 *   @ref sl_wifi_init should be called before this API.
 * @param[out] efuse_data
 *   @ref sli_wifi_efuse_data_t object that contains the Manufacturing software version.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
void sli_si91x_set_efuse_data(const sli_wifi_efuse_data_t *efuse_data);

/**
 * A utility function to convert dBm value to si91x specific power value
 * @param wifi_max_tx_power which holds the join power value with dBm as units.
 * @return si91x power level
 */
static inline uint8_t sli_convert_dbm_to_si91x_power_level(sl_wifi_max_tx_power_t wifi_max_tx_power)
{
  uint8_t power_value_in_dBm = wifi_max_tx_power.join_tx_power;
  if (power_value_in_dBm >= SLI_SI91X_LOW_TRANSMIT_POWER_THRESHOLD) {
    return SLI_SI91X_LOW_POWER_LEVEL;
  } else if (power_value_in_dBm >= SLI_SI91X_MEDIUM_TRANSMIT_POWER_THRESHOLD) {
    return SLI_SI91X_MEDIUM_POWER_LEVEL;
  } else {
    return SLI_SI91X_HIGH_POWER_LEVEL;
  }
}

sl_status_t sl_si91x_platform_init(void);
sl_status_t sli_si91x_platform_deinit(void);

// Event API
/* Function used to set specified flags for event */
void sli_si91x_set_event(uint32_t event_mask);

/* Function used to set specified flags for async event */
void sli_si91x_host_set_async_event(uint32_t event_mask);

uint32_t sli_si91x_wait_for_event(uint32_t event_mask, uint32_t timeout);

/* Function used to clear flags for specific event */
uint32_t sli_si91x_clear_event(uint32_t event_mask);

/* Function to send the requested Wi-Fi and BT/BLE performance profile to firmware */
sl_status_t sli_si91x_send_power_save_request(const sl_wifi_performance_profile_v2_t *wifi_profile,
                                              const sl_bt_performance_profile_t *bt_profile);

sl_status_t sli_si91x_host_init_buffer_manager(const sl_wifi_buffer_configuration_t *config);
sl_status_t sli_si91x_host_deinit_buffer_manager(void);

/* Function used to allocate memory */
sl_status_t sli_si91x_host_allocate_buffer(sl_wifi_buffer_t **buffer,
                                           sl_wifi_buffer_type_t type,
                                           uint32_t buffer_size,
                                           uint32_t wait_duration_ms);

// Helper functions for command packet processing
/**
 * @brief Set flags for command packet based on wait period and command type
 * @param command Command type
 * @param wait_period Wait period configuration
 * @param data_buffer Pointer to data buffer (can be NULL)
 * @return Flags for the command packet
 */
uint8_t sli_set_command_packet_flags(uint32_t command, sli_wifi_wait_period_t wait_period, const void *data_buffer);

/**
 * @brief Configure command packet node properties
 * @param node Pointer to the queue packet node
 * @param buffer Host packet buffer
 * @param command_type Command type
 * @param flags Packet flags
 * @param sdk_context SDK context
 * @param wait_period Wait period configuration
 */
void sli_configure_command_packet_node(sli_si91x_queue_packet_t *node,
                                       sl_wifi_buffer_t *buffer,
                                       sli_wifi_command_type_t command_type,
                                       uint8_t flags,
                                       void *sdk_context,
                                       sli_wifi_wait_period_t wait_period);

/**
 * @brief Enqueue command packet to the appropriate queue
 * @param command_type Command type
 * @param queue_packet Queue packet buffer
 * @param buffer Host packet buffer
 * @param packet_id Packet ID
 * @return Status of the operation
 */
sl_status_t sli_enqueue_command_packet(sli_wifi_command_type_t command_type,
                                       sl_wifi_buffer_t *queue_packet,
                                       sl_wifi_buffer_t *buffer,
                                       uint8_t packet_id);

/**
 * @brief Handle command response and cleanup
 * @param command_type Command type
 * @param wait_period Wait period configuration
 * @param packet_id Packet ID
 * @param data_buffer Pointer to data buffer (can be NULL)
 * @param response Pointer to response buffer
 * @return Status of the operation
 */
sl_status_t sli_handle_command_response(sli_wifi_command_type_t command_type,
                                        sli_wifi_wait_period_t wait_period,
                                        uint8_t packet_id,
                                        void **data_buffer,
                                        sl_wifi_buffer_t **response);

/**
 * @brief Configure command packet node properties for SI91X driver
 * @param node Pointer to the queue packet node
 * @param buffer Host packet buffer
 * @param command_type Command type
 * @param flags Packet flags
 * @param sdk_context SDK context
 * @param wait_period Wait period configuration
 */
void sli_configure_si91x_command_packet_node(sli_si91x_queue_packet_t *node,
                                             sl_wifi_buffer_t *buffer,
                                             sli_wifi_command_type_t command_type,
                                             uint8_t flags,
                                             void *sdk_context,
                                             sli_wifi_wait_period_t wait_period);

/**
 * @brief Enqueue command packet to the appropriate queue for SI91X driver
 * @param command_type Command type
 * @param queue_packet Queue packet buffer
 * @param buffer Host packet buffer
 * @param packet_id Packet ID
 * @return Status of the operation
 */
sl_status_t sli_enqueue_si91x_command_packet(sli_wifi_command_type_t command_type,
                                             sl_wifi_buffer_t *queue_packet,
                                             sl_wifi_buffer_t *buffer,
                                             uint8_t packet_id);

//! @endcond

/** \addtogroup EXTERNAL_HOST_INTERFACE_FUNCTIONS
 * \ingroup EXTERNAL_HOST_INTERFACE
 * @{ */
/***************************************************************************/ /**
 * @brief
 *   Retrieve data from a buffer with a specified offset.
 *
 * @details
 *   This function is designed to retrieve data from a buffer at a specified offset.
 *
 * @param[in]  buffer
 *   A pointer to an [sl_wifi_buffer_t](../wiseconnect-api-reference-guide-wi-fi/sl-wifi-buffer-t) structure from which data is to be retrieved.
 * 
 * @param[in]  offset
 *   Offset from the start of the buffer where data retrieval begins.
 * 
 * @param[out] data_length
 *   Pointer to a variable where the remaining data length from the offset will be stored.
 *
 * @return
 *   Pointer to the data at the specified offset within the buffer.
 *
 ******************************************************************************/
void *sl_si91x_host_get_buffer_data(sl_wifi_buffer_t *buffer, uint16_t offset, uint16_t *data_length);
/** @} */

//! @cond Doxygen_Suppress

/* Function used to deallocate the memory associated with buffer */
void sli_si91x_host_free_buffer(sl_wifi_buffer_t *buffer);

/* Function enqueues response into corresponding response queue */
sl_status_t sli_si91x_add_to_queue(sli_wifi_buffer_queue_t *queue, sl_wifi_buffer_t *buffer);

/* Function dequeues responses from Asynch response queues */
sl_status_t sli_si91x_remove_from_queue(sli_wifi_buffer_queue_t *queue, sl_wifi_buffer_t **buffer);

/* Function used to flush the pending TX packets from the specified queue */
sl_status_t sli_si91x_flush_nodes_from_queue(sli_wifi_command_queue_t *queue,
                                             sli_si91x_node_free_function_t node_free_function);

/* Function used to remove the buffer from the specified queue by using comparator */
sl_status_t sli_wifi_remove_buffer_from_queue_by_comparator(sli_wifi_buffer_queue_t *queue,
                                                            const void *user_data,
                                                            sli_si91x_wifi_buffer_comparator comparator,
                                                            sl_wifi_buffer_t **buffer);

sl_status_t sli_si91x_flush_all_tx_wifi_queues(uint16_t frame_status);

/* Function used to flush all the pending TX packets from the specified queue */
sl_status_t sli_si91x_flush_queue_based_on_type(sli_wifi_command_queue_t *queue,
                                                uint32_t event_mask,
                                                uint16_t frame_status,
                                                sli_si91x_compare_function_t compare_function,
                                                void *user_data);

/* Function used to check whether queue is empty or not */
uint32_t sli_si91x_host_queue_status(const sli_wifi_buffer_queue_t *queue);

// These aren't host APIs. These should go into a wifi bus API header
/* Function used to set buffer pointer to point to specified memory address */
sl_status_t sl_si91x_bus_read_memory(uint32_t addr, uint16_t length, const uint8_t *buffer);

/* Function used to set specified memory address to point to buffer */
sl_status_t sl_si91x_bus_write_memory(uint32_t addr, uint16_t length, const uint8_t *buffer);

/*==============================================*/
/**
 * @brief       Send chunk of data from Host to Si91x using SPI slave mode.
 * @param[in]   data_length   -  Actual data length to send 
 * @param[in]   buffer        - Pointer to data  
 * @return      sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.  
 *
 */
sl_status_t sli_si91x_bus_write_slave(uint32_t data_length, const uint8_t *buffer);

/* Function used to read contents of the register */
sl_status_t sli_si91x_bus_read_register(uint8_t address, uint8_t register_size, uint16_t *output);

/* Function used to write data into register */
sl_status_t sli_si91x_bus_write_register(uint8_t address, uint8_t register_size, uint16_t data);

/* Function used to read frame */
sl_status_t sli_si91x_bus_read_frame(sl_wifi_buffer_t **buffer);

/* Function used to write frames */
sl_status_t sli_si91x_bus_write_frame(sl_wifi_system_packet_t *packet,
                                      const uint8_t *payloadparam,
                                      uint16_t size_param);

/* Function used to check the bus availability */
sl_status_t sl_si91x_bus_init();

/* Function used to check the bus availability */
sl_status_t sli_si91x_bus_rx_irq_handler(void);

/* Function used to check the bus availability */
void sli_si91x_bus_rx_done_handler(void);

/*==============================================*/
/**
 * @brief       Sends boot instructions to WiFi module
 * @param[in]   uint8 type, type of the insruction to perform
 * @param[in]   uint32 *data, pointer to data which is to be read/write
 * @param[out]  none
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 * @section description 
 * This API is used to send boot instructions to WiFi module.
 **************************************************/
sl_status_t sli_si91x_boot_instruction(uint8_t type, uint16_t *data);

/***************************************************************************/ /**
 * @brief
 *   The @ref sli_si91x_bus_enable_high_speed() should be called only if the SPI clock frequency is more than 25 MHz.
 * @note
 *   SPI initialization has to be done in low-speed mode only.
 *   After device SPI is configured, this API is used for high-speed mode (>25 MHz).
 *   In addition to this API, the following API sl_si91x_host_enable_high_speed_bus has to be ported by the user to implement the host clock switch.
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 ******************************************************************************/
sl_status_t sli_si91x_bus_enable_high_speed();

/* Function used to read the interrupt register */
sl_status_t sli_si91x_bus_read_interrupt_status(uint16_t *interrupt_status);

/* Function used to block specified interrupts */
sl_status_t sli_si91x_bus_set_interrupt_mask(uint32_t mask);

/* Function used to initialize SPI interface on ULP wakeup */
void sli_si91x_ulp_wakeup_init(void);

sli_wifi_command_queue_t *sli_si91x_get_command_queue(sli_wifi_command_type_t type);

bool sli_si91x_get_flash_command_status();

void sli_si91x_update_flash_command_status(bool flag);

bool sli_si91x_is_sdk_ok_to_sleep();
//! @endcond

/**
* @addtogroup EXTERNAL_HOST_INTERFACE_FUNCTIONS
* @{ 
*/

/***************************************************************************/
/**
 * @brief
 *   Delay execution for a specified number of milliseconds.
 * 
 * @details
 *   This function introduces a delay for the specified amount of time in milliseconds. It uses the underlying OS 
 *   delay function (`osDelay`) to yield the CPU, allowing other tasks to execute during the delay period. This 
 *   ensures that the delay does not block the execution flow.
 * 
 * @param[in] delay_milliseconds 
 *   The time delay in milliseconds.
 *****************************************************************************/
void sl_si91x_host_delay_ms(uint32_t delay_milliseconds);

/**
 * @brief
 *   Retrieves the current timestamp.
 * 
 * @details
 *   This function retrieves the current timestamp from the host system. The timestamp can be used for various purposes such as logging, time measurements, and synchronization.
 * 
 * @return
 *   The current timestamp of type sl_si91x_host_timestamp_t.
 */
sl_si91x_host_timestamp_t sl_si91x_host_get_timestamp(void);

/**
 * @brief
 *   Calculates the elapsed time since a given starting timestamp.
 * 
 * @details
 *   This function calculates the difference between the current timestamp and a provided starting timestamp. It is useful for measuring the time elapsed during operations.
 * 
 * @param[in] starting_timestamp
 *   The starting timestamp from which the elapsed time is calculated.
 * 
 * @return
 *   The elapsed time in milliseconds of type sl_si91x_host_timestamp_t.
 */
sl_si91x_host_timestamp_t sl_si91x_host_elapsed_time(uint32_t starting_timestamp);

/**
 * @brief
 *   Checks if the device is initialized.
 * 
 * @details
 *   This function verifies whether the device has been properly initialized. It is typically used to ensure that the device is ready for operation before performing any further actions.
 * 
 * @return
 *   Returns `true` if the device is initialized, `false` otherwise.
 */
bool sl_si91x_is_device_initialized(void);

/** @} */
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
sl_status_t sli_si91x_flush_all_socket_command_queues(uint16_t frame_status, uint8_t vap_id);

sl_status_t sli_si91x_flush_socket_command_queues_based_on_queue_type(uint8_t index, uint16_t frame_status);

sl_status_t sli_si91x_flush_all_socket_data_queues(uint8_t vap_id);

sl_status_t sli_si91x_flush_socket_data_queues_based_on_queue_type(uint8_t index);
#endif

/**
 * @brief Flushes all packets from the specified data transmission queue.
 * @details This function removes all packets from the provided transmission queue (`tx_data_queue`) and frees the associated memory. It ensures thread-safe operation by preventing race conditions during the process.
 *
 * @param[in, out] tx_data_queue Pointer to the transmission data queue to be flushed. 
 *                               The queue will be reset to an empty state after the function completes.
 *
 * @return 
 * - `SL_STATUS_OK`: The operation was successful, and the queue has been flushed.
 * - `SL_STATUS_FAIL`: The provided queue pointer is NULL.
 *
 * @note 
 * - This function is typically used to clear transmission buffers in scenarios such as error recovery or reinitialization.
 * - The function uses atomic operations to ensure that the queue is safely manipulated in multi-threaded environments.
 * - The function resets the queue to an empty state after flushing all packets.
 */
sl_status_t sli_si91x_flush_generic_data_queues(sli_wifi_buffer_queue_t *tx_data_queue);

/***************************************************************************/ /**
 * @brief
 *   Retrieves the current status of the TX command.
 *
 * @details
 *   This function returns the current status flag indicating whether a TX (transmit) command is in progress or completed.
 *   It is typically used to check if the system is ready to send a new TX command or if a previous command is still pending.
 *
 * @return
 *   Returns `true` if a TX command is in progress, `false` otherwise.
 ******************************************************************************/
bool sli_si91x_get_tx_command_status(void);

/***************************************************************************/ /**
 * @brief
 *   Updates the status of the TX command.
 *
 * @details
 *   This function sets the status flag for the TX (transmit) command. It can be used to mark the TX command as in progress or completed.
 *   Typically, this is called internally when starting or finishing a TX command to keep track of the command state.
 *
 * @param[in] flag
 *   Set to `true` to indicate a TX command is in progress, or `false` to indicate it is completed.
 ******************************************************************************/
void sli_si91x_update_tx_command_status(bool flag);

/**
 * @brief Flushes a transmit (TX) packet from the Wi-Fi command queue.
 *
 * This function removes a specified TX packet from the command queue, performs any necessary cleanup,
 * and updates the queue state based on the provided frame status and event mask.
 *
 * @param[in,out] queue           Pointer to the Wi-Fi command queue structure.
 * @param[in]     current_packet  Pointer to the current TX packet buffer to be flushed.
 * @param[in]     queue_node      Pointer to the queue node associated with the packet.
 * @param[in]     frame_status    Status code indicating the result or reason for flushing the packet.
 * @param[in]     event_mask      Bitmask specifying which events to trigger or handle during the flush operation.
 */
void sli_flush_tx_packet(sli_wifi_command_queue_t *queue,
                         sl_wifi_buffer_t *current_packet,
                         sli_si91x_queue_packet_t *queue_node,
                         uint16_t frame_status,
                         uint32_t event_mask);

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
/**
 * @brief 
 *    Configures the Server Name Indication (SNI) extension for a socket.
 *
 *  @details
 *    This function sets up the SNI extension, which is used in TLS communication
 *    to specify the hostname of the server the client intends to connect to. It prepares
 *    the necessary request structure and initiates the configuration process for the
 *    embedded socket.
 *
 * @param[in] sni_extension 
 *    Pointer to the SNI extension data of type `sl_si91x_socket_type_length_value_t`.
 *    This structure contains the type, length, and value of the SNI extension.
 * @param[in] sni_target_protocol
 *    SNI target type (e.g., HTTPS or MQTT).
 *
 * @return sl_status_t
 *    - SL_STATUS_OK: Operation completed successfully.
 *    - SL_STATUS_WOULD_OVERFLOW: The SNI extension size exceeds the allowed limit.
 *    - Other error codes: Refer to [Status Codes](https://docs.silabs.com/gecko-platform/latest/platform-common/status) 
 *      and [WiSeConnect Status Codes](../wiseconnect-api-reference-guide-err-codes/wiseconnect-status-codes) for details. 
 */
sl_status_t sli_si91x_set_sni_for_embedded_socket(const sli_si91x_tls_extension_info_t *sni_extension,
                                                  sli_si91x_sni_target_protocol_t sni_target_protocol);

/***************************************************************************/ /**
 * @brief
 *   Configure SNI (Server Name Indication) extension for TLS connection.
 *
 * @details
 *   This function configures the SNI extension for embedded socket TLS connections.
 *   It can either use a pre-configured SNI extension structure or create one from
 *   a hostname string. Callers using public socket types should convert to internal
 *   sli_si91x_tls_extension_info_t before calling this function.
 *
 * @param[in] sni_extension
 *   Pointer to internal SNI extension structure. Can be NULL if using hostname.
 *
 * @param[in] host_name
 *   Hostname string to create SNI extension from. Can be NULL if using sni_extension.
 *
 * @param[in] sni_target_protocol
 *   Specifies the protocol type (HTTP/HTTPS/MQTT) for which SNI is configured.
 *
 * @return
 *   sl_status_t. See https://docs.silabs.com/gecko-platform/latest/platform-common/status for details.
 *   - SL_STATUS_OK: SNI configured successfully
 *   - SL_STATUS_ALLOCATION_FAILED: Memory allocation failed
 *   - Other error codes from underlying API calls
 ******************************************************************************/
sl_status_t sli_configure_sni(const sli_si91x_tls_extension_info_t *sni_extension,
                              const uint8_t *host_name,
                              sli_si91x_sni_target_protocol_t sni_target_protocol);
#endif

#endif // _SL_RSI_UTILITY_H_