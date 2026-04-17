/***************************************************************************/ /**
 * @file sli_si91x_wifi_event_handler.c
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
#include "sl_si91x_host_interface.h"
#include "sl_si91x_types.h"
#include "sl_si91x_protocol_types.h"
#include "sl_si91x_driver.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_types.h"
#include "sl_rsi_utility.h"
#include "cmsis_os2.h"
#include "cmsis_compiler.h"
#include "sl_cmsis_utility.h"
#include "sl_si91x_core_utilities.h"
#include "sl_core.h"
#include <string.h>
#include "sli_wifi_constants.h"
#include "sli_wifi_power_profile.h"
#include "sl_utility.h"
#include "sl_log_helper_si91x.h"

#ifdef SL_NET_COMPONENT_INCLUDED
#include "sl_net_types.h"
#include "sl_net_constants.h"
#include "sl_net_wifi_types.h"
#include "sli_net_types.h"
#include "sl_net.h"
#include "sli_net_constants.h"
#endif

#include "sli_wifi_utility.h"
#ifdef SLI_SI91X_MCU_INTERFACE
#include "rsi_m4.h"
#endif

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
#include "sl_si91x_socket_types.h"
#include "sl_si91x_socket_utility.h"
#include "sl_net_si91x_integration_handler.h"
#else
// This macro defines a handler for dispatching network events.
// It is used to handle events related to the SI91x module
#define SL_NET_EVENT_DISPATCH_HANDLER(data, packet) \
  {                                                 \
    UNUSED_PARAMETER(data);                         \
    UNUSED_PARAMETER(packet);                       \
  }
#endif

#ifdef SLI_SI91X_ENABLE_BLE
#include "rsi_bt_common.h"
#endif

extern osMessageQueueId_t sli_network_manager_request_queue;
#define BUS_THREAD_EVENTS \
  (SL_SI91X_ALL_TX_PENDING_COMMAND_EVENTS | SL_SI91X_NCP_HOST_BUS_RX_EVENT | SL_SI91X_TA_BUFFER_FULL_CLEAR_EVENT)

/**
 * All flags used with async events
 */
#define SL_SI91X_NOTIFICATION_FLAG(x) (1 << (x))

// Indicates asynchronous notification for command engine status events
#define SLI_SI91X_NCP_HOST_COMMAND_ENGINE_STATUS_NOTIFICATION_EVENT SL_SI91X_NOTIFICATION_FLAG(SI91X_CMD_MAX + 1)

// Indicates asynchronous RX response received for common command type
#define NCP_HOST_COMMON_NOTIFICATION_EVENT SL_SI91X_NOTIFICATION_FLAG(SLI_WIFI_COMMON_CMD)

// Indicates asynchronous RX response received for WLAN command type
#define NCP_HOST_WLAN_NOTIFICATION_EVENT SL_SI91X_NOTIFICATION_FLAG(SLI_WIFI_WLAN_CMD)

// Indicates asynchronous RX response received for NETWORK command type
#define NCP_HOST_NETWORK_NOTIFICATION_EVENT SL_SI91X_NOTIFICATION_FLAG(SLI_SI91X_NETWORK_CMD)

// Indicates asynchronous RX response received for SOCKET command type
#define NCP_HOST_SOCKET_NOTIFICATION_EVENT SL_SI91X_NOTIFICATION_FLAG(SLI_SI91X_SOCKET_CMD)

// Indicates asynchronous RX response received for SOCKET data type
#define NCP_HOST_SOCKET_DATA_NOTIFICATION_EVENT SL_SI91X_NOTIFICATION_FLAG(SI91X_CMD_MAX)

// Indicates asynchronous RX response received for BLE command type
#define NCP_HOST_BLE_NOTIFICATION_EVENT SL_SI91X_NOTIFICATION_FLAG(SLI_SI91X_BT_CMD)

// Define a constant for identifying a Wi-Fi packet type
#define SLI_WIFI_PACKET 1

// Define a constant for identifying a BLE packet type
#define SLI_BLE_PACKET 2

/******************************************************
 *               Variable Definitions
 ******************************************************/

extern osEventFlagsId_t si91x_async_events;
extern osMessageQueueId_t sli_command_engine_status_msg_queue;

volatile uint32_t tx_command_queues_status             = 0;
volatile uint32_t tx_socket_command_queues_status      = 0;
volatile uint32_t tx_socket_data_queues_status         = 0;
volatile uint32_t tx_generic_socket_data_queues_status = 0;

volatile uint32_t tx_command_queues_command_in_flight_status       = 0;
volatile uint8_t tx_socket_command_command_in_flight_queues_status = 0;

#if SLI_SI91X_MCU_INTERFACE
extern sli_wifi_buffer_queue_t sli_ahb_bus_rx_queue;
#endif

#if SL_NCP_UART_INTERFACE
extern sli_wifi_buffer_queue_t sli_uart_bus_rx_queue;
#endif

static uint16_t interrupt_status = 0;

bool global_queue_block = false;

// Define enum for wait times (can be represented with just 2 bits)
typedef enum {
  SLI_WIFI_WAIT_TIME_ZERO        = 0, // 0 ms
  SLI_WIFI_WAIT_TIME_YIELD       = 1, // 10 ms
  SLI_WIFI_WAIT_TIME_WAITFOREVER = 2  // osWaitForever
} sli_wait_time_type_t;

/**
 * @brief Lookup table encoding command engine wait times for 16 possible states.
 *
 * This 32-bit constant encodes wait times for each of the 16 possible combinations
 * of four status bits, using 2 bits per entry (total 16 entries).
 *
 * The four status bits (from MSB to LSB) are:
 *   - Bit 3: wifi_buffer_full   (1 = WiFi buffer is full, 0 = not full)
 *   - Bit 2: wifi_packets       (1 = WiFi packets present, 0 = none)
 *   - Bit 1: ble_buffer_full    (1 = BLE buffer is full, 0 = not full)
 *   - Bit 0: ble_packets        (1 = BLE packets present, 0 = none)
 *
 * The 2-bit value for each entry specifies the wait time action:
 *   - SLI_WIFI_WAIT_TIME_ZERO         : No wait
 *   - SLI_WIFI_WAIT_TIME_YIELD        : Yield
 *   - SLI_WIFI_WAIT_TIME_WAITFOREVER  : Wait forever
 *
 * The lookup table is constructed as:
 *   (wait_time_for_index15 << 30) | (wait_time_for_index14 << 28) | ... | (wait_time_for_index0 << 0)
 * where index is the 4-bit value: (wifi_buffer_full << 3) | (wifi_packets << 2) | (ble_buffer_full << 1) | (ble_packets)
 *
 * Example:
 *   Index 0b1111 (15): wifi_buffer_full=1, wifi_packets=1, ble_buffer_full=1, ble_packets=1
 *   Index 0b0000 (0):  wifi_buffer_full=0, wifi_packets=0, ble_buffer_full=0, ble_packets=0
 */
// command engine wait time lookup table (16 entries, 2 bits each => 32 bits total)
static const uint32_t sli_si91x_command_engine_wait_time_lookup =
  (SLI_WIFI_WAIT_TIME_YIELD << (2 * 15)) |       // index 15: 1111
  (SLI_WIFI_WAIT_TIME_YIELD << (2 * 14)) |       // index 14: 1110
  (SLI_WIFI_WAIT_TIME_ZERO << (2 * 13)) |        // index 13: 1101
  (SLI_WIFI_WAIT_TIME_YIELD << (2 * 12)) |       // index 12: 1100
  (SLI_WIFI_WAIT_TIME_YIELD << (2 * 11)) |       // index 11: 1011
  (SLI_WIFI_WAIT_TIME_WAITFOREVER << (2 * 10)) | // index 10: 1010
  (SLI_WIFI_WAIT_TIME_ZERO << (2 * 9)) |         // index 9: 1001
  (SLI_WIFI_WAIT_TIME_WAITFOREVER << (2 * 8)) |  // index 8: 1000
  (SLI_WIFI_WAIT_TIME_ZERO << (2 * 7)) |         // index 7: 0111
  (SLI_WIFI_WAIT_TIME_ZERO << (2 * 6)) |         // index 6: 0110
  (SLI_WIFI_WAIT_TIME_ZERO << (2 * 5)) |         // index 5: 0101
  (SLI_WIFI_WAIT_TIME_ZERO << (2 * 4)) |         // index 4: 0100
  (SLI_WIFI_WAIT_TIME_YIELD << (2 * 3)) |        // index 3: 0011
  (SLI_WIFI_WAIT_TIME_WAITFOREVER << (2 * 2)) |  // index 2: 0010
  (SLI_WIFI_WAIT_TIME_ZERO << (2 * 1)) |         // index 1: 0001
  (SLI_WIFI_WAIT_TIME_WAITFOREVER << (2 * 0));   // index 0: 0000

/******************************************************
 *             Extern Variable Declarations
 ******************************************************/
// Declaration of a global flag to indicate if background mode is enabled
extern bool bg_enabled;

// Declaration of the SI91x event handler function
extern sl_wifi_event_handler_t si91x_event_handler;

extern sli_wifi_command_queue_t cmd_queues[SI91X_CMD_MAX];
extern sli_wifi_buffer_queue_t sli_tx_data_queue;

sl_status_t sli_create_generic_rx_packet_from_params(sli_si91x_queue_packet_t **queue_packet,
                                                     sl_wifi_buffer_t **packet_buffer,
                                                     uint16_t packet_id,
                                                     uint8_t flags,
                                                     void *sdk_context,
                                                     uint16_t frame_status);

#ifdef SLI_SI91X_MCU_INTERFACE
// External declaration of a function to configure M4 DMA descriptors on reset
extern void sli_si91x_config_m4_dma_desc_on_reset(void);
#endif

/******************************************************
 *               Function Declarations
 ******************************************************/

sl_status_t sli_submit_rx_buffer(void);

static inline void sli_si91x_wifi_handle_rx_events(uint32_t *event);

static inline void sli_si91x_wifi_handle_tx_event(uint32_t *event);

static void set_async_event(uint32_t event_mask);

static void sli_handle_dhcp_and_rejoin_failure(void *sdk_context, sl_wifi_buffer_t *temp_buffer, uint16_t frame_status);

static void sli_handle_and_queue_wlan_response(sli_si91x_queue_packet_t *node,
                                               sl_wifi_buffer_t *buffer,
                                               sl_wifi_buffer_t *packet);

#ifdef SLI_SI91X_MCU_INTERFACE
void unmask_ta_interrupt(uint32_t interrupt_no);
#endif

static sl_status_t bus_write_data_frame(sli_wifi_buffer_queue_t *queue);

static sl_status_t bus_write_frame(sli_wifi_command_queue_t *queue,
                                   sli_wifi_command_type_t command_type,
                                   sl_wifi_buffer_type_t buffer_type,
                                   bool *global_queue_block);

sl_status_t sli_si91x_req_wakeup(void);

void sli_handle_wifi_events(const sli_si91x_queue_packet_t *data, sl_wifi_system_packet_t *packet);

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
static sli_si91x_socket_t *get_socket_from_packet(sl_wifi_system_packet_t *socket_packet);
#endif

static bool sli_si91x_is_bus_ready(bool global_queue_block, uint8_t packet_type);
void sli_si91x_process_common_events();
void sli_si91x_process_wifi_events();
void sli_si91x_process_network_events();
void sli_si91x_process_socket_events();
void sli_si91x_process_socket_data_events();
void sli_si91x_process_command_engine_status_events();
#ifdef SLI_SI91X_ENABLE_BLE
void sli_si91x_process_ble_events();
#endif

/******************************************************
 *             Static Function Definitions
 ******************************************************/
static sl_status_t bus_write_frame(sli_wifi_command_queue_t *queue,
                                   sli_wifi_command_type_t command_type,
                                   sl_wifi_buffer_type_t buffer_type,
                                   bool *global_queue_block)
{
  UNUSED_PARAMETER(buffer_type);
  UNUSED_PARAMETER(command_type);
  sl_status_t status;
  sl_wifi_buffer_t *buffer;
  sl_wifi_system_packet_t *packet;
  sli_si91x_queue_packet_t *node = NULL;

  // Wake up the device if not in high performance mode
  if ((current_performance_profile != HIGH_PERFORMANCE) && (sli_si91x_req_wakeup() != SL_STATUS_OK)) {
    return SL_STATUS_TIMEOUT;
  }

  // Remove a buffer from the TX queue
  status = sli_si91x_remove_from_queue(&queue->tx_queue, &buffer);
  if (status != SL_STATUS_OK) {
    if (current_performance_profile != HIGH_PERFORMANCE) {
      sl_si91x_host_clear_sleep_indicator();
    }
    VERIFY_STATUS_AND_RETURN(status);
  }

  // Get the node and packet from the buffer
  node            = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  packet          = sli_wifi_host_get_buffer_data(node->host_packet, 0, NULL);
  uint16_t length = packet->length;

  // Modify the packet's descriptor to include the firmware queue ID in the length field
  packet->desc[1] |= (node->firmware_queue_id << 4);

  if (packet->command) {
    // Set the global_queue_block flag if it is present in the packet's flags
    if (SLI_WIFI_PACKET_GLOBAL_QUEUE_BLOCK & node->flags) {
      *global_queue_block = true;
    }

    if (SLI_WIFI_PACKET_WITH_ASYNC_RESPONSE != (node->flags & SLI_WIFI_PACKET_WITH_ASYNC_RESPONSE)) {
      // Update trace information with packet details
      // If the packet doesn't have an async response, mark the command as in flight
      queue->command_in_flight = true;
      queue->packet_id         = node->host_packet->id;
      queue->firmware_queue_id = node->firmware_queue_id;
      queue->frame_type        = packet->command;
      queue->flags             = node->flags;
      queue->command_timeout   = node->command_timeout;
      queue->command_tickcount = node->command_tickcount;
      queue->sdk_context       = node->sdk_context;
    }
  }
#ifdef SLI_SI91X_MCU_INTERFACE
  // Set flash command status for certain commands
  if ((packet->command == SLI_COMMON_RSP_TA_M4_COMMANDS) || (packet->command == SLI_WLAN_REQ_SET_CERTIFICATE)
      || (packet->command == SLI_COMMON_RSP_SOFT_RESET)) {
    sli_si91x_update_flash_command_status(true);
  }
  sli_si91x_update_tx_command_status(true);
#endif

  // Write the frame to the bus using packet data and length
  status = sli_si91x_bus_write_frame(packet, packet->data, length);

#ifdef SLI_SI91X_MCU_INTERFACE
  sli_si91x_update_tx_command_status(false);
#endif

  // Handle errors during frame writing
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("\r\n BUS_WRITE_ERROR \r\n");
    node->frame_status = SL_STATUS_BUS_ERROR;
#ifdef SLI_SI91X_ENABLE_BLE
    // Notify BLE stack that transmission is done
    if (command_type == SLI_SI91X_BT_CMD) {
      rsi_bt_common_tx_done(packet, status);
    }
#endif
    // Flush the current packet
    sli_flush_tx_packet(queue, buffer, node, SL_STATUS_BUS_ERROR, node->event_mask);
    sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_BUS_ERROR);
  } else {
#ifdef SLI_SI91X_MCU_INTERFACE
    // Reconfigure DMA descriptors on soft reset
    if (packet->desc[2] == SLI_COMMON_REQ_SOFT_RESET) {
      sli_si91x_config_m4_dma_desc_on_reset();
    }
#endif
    SL_PRINT_STRING_DEBUG("<>>>> Tx -> queueId : %u, frameId : 0x%x, length : %u\n",
                          node->firmware_queue_id,
                          packet->command,
                          length);
    if (packet->command) {
      // Set the global_queue_block flag if it is present in the packet's flags
      if (SI91X_PACKET_GLOBAL_QUEUE_BLOCK & node->flags) {
        *global_queue_block = true;
      }

      // If the packet type is not async response, note down the command as in flight and other details
      if (SI91X_PACKET_WITH_ASYNC_RESPONSE != (node->flags & SI91X_PACKET_WITH_ASYNC_RESPONSE)) {
        queue->command_in_flight = true;
        queue->packet_id         = node->host_packet->id;
        queue->firmware_queue_id = node->firmware_queue_id;
        queue->frame_type        = packet->command;
        queue->flags             = node->flags;
        queue->command_timeout   = node->command_timeout;
        queue->command_tickcount = node->command_tickcount;
        queue->sdk_context       = node->sdk_context;
        queue->event_mask        = node->event_mask;
      }
    }
    queue->tx_counter++;

#ifdef SLI_SI91X_ENABLE_BLE
    // Notify BLE stack that transmission is done
    if (command_type == SLI_SI91X_BT_CMD) {
      rsi_bt_common_tx_done(packet, status);
    }
#endif
    // Free the host packet and buffer after successful transmission
    sli_si91x_host_free_buffer(node->host_packet);
    sli_si91x_host_free_buffer(buffer);
  }

  // Clear the sleep indicator if the current performance profile is not high performance
  if (current_performance_profile != HIGH_PERFORMANCE) {
    sl_si91x_host_clear_sleep_indicator();
  }
  return status;
}

// This function is called for writing data
static sl_status_t bus_write_data_frame(sli_wifi_buffer_queue_t *queue)
{
  sl_status_t status;
  sl_wifi_buffer_t *buffer;
  sl_wifi_system_packet_t *packet;

  if ((current_performance_profile != HIGH_PERFORMANCE) && (sli_si91x_req_wakeup() != SL_STATUS_OK)) {
    return SL_STATUS_TIMEOUT;
  }

  status = sli_si91x_remove_from_queue(queue, &buffer);
  if (status != SL_STATUS_OK) {
    if (current_performance_profile != HIGH_PERFORMANCE) {
      sl_si91x_host_clear_sleep_indicator();
    }
    VERIFY_STATUS_AND_RETURN(status);
  }

  packet          = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
  uint16_t length = packet->length;

  // Modify the packet's descriptor to include the firmware queue ID in the length field
  packet->desc[1] |= (5 << 4);

#ifdef SLI_SI91X_MCU_INTERFACE
  sli_si91x_update_tx_command_status(true);
#endif

  // Write the frame to the bus using packet data and length
  status = sli_si91x_bus_write_frame(packet, packet->data, length);

#ifdef SLI_SI91X_MCU_INTERFACE
  sli_si91x_update_tx_command_status(false);
#endif

  // Handle errors during frame writing
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("\r\n BUS_WRITE_ERROR \r\n");
    sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_BUS_ERROR);
  } else {
    SL_PRINT_STRING_DEBUG("<>>>> Tx -> queueId : %u, frameId : 0x%x, length : %u\n", 5, 0, length);
  }

  if (current_performance_profile != HIGH_PERFORMANCE) {
    sl_si91x_host_clear_sleep_indicator();
  }

  sli_si91x_host_free_buffer(buffer);
  return status;
}

static void set_async_event(uint32_t event_mask)
{
  osEventFlagsSet(si91x_async_events, event_mask);
}

// Function to check if the bus is ready for writing
static bool sli_si91x_is_bus_ready(bool global_queue_block, uint8_t packet_type)
{
  // If the global queue block flag is set, the bus is not ready
  if (global_queue_block) {
    return false;
  }

#ifndef SLI_SI91X_MCU_INTERFACE
  // If the current performance profile is not high performance, request wakeup
  if ((current_performance_profile != HIGH_PERFORMANCE) && (sli_si91x_req_wakeup() != SL_STATUS_OK)) {
    return false;
  }
#endif

  // Read the interrupt status from the bus
  sli_si91x_bus_read_interrupt_status(&interrupt_status);

#ifndef SLI_SI91X_MCU_INTERFACE
  // Clear the sleep indicator if the current performance profile is not high performance
  if (current_performance_profile != HIGH_PERFORMANCE) {
    sl_si91x_host_clear_sleep_indicator();
  }
#endif

  // If the buffer is full for the given packet type, unmask the TA interrupt (if MCU interface) and return false
  if ((packet_type == SLI_WIFI_PACKET) && (interrupt_status & SLI_WIFI_BUFFER_FULL)) {
#ifdef SLI_SI91X_MCU_INTERFACE
    unmask_ta_interrupt(TA_RSI_BUFFER_FULL_CLEAR_EVENT);
#endif
    return false;
  } else if ((packet_type == SLI_BLE_PACKET) && (interrupt_status & SLI_BLE_BUFFER_FULL)) {
    return false;
  }
  // The bus is ready for writing
  return true;
}

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
static sli_si91x_socket_t *get_socket_from_packet(sl_wifi_system_packet_t *socket_packet)
{
  int socket_id = sli_si91x_get_socket_id(socket_packet);

  if (socket_packet->command == SLI_WLAN_RSP_CONN_ESTABLISH) {
    socket_packet->command = SLI_WLAN_RSP_SOCKET_ACCEPT;
  }

  if (socket_packet->command == SLI_WLAN_RSP_SOCKET_CREATE) {
    const sli_si91x_socket_create_response_t *socket_create_response =
      ((sli_si91x_socket_create_response_t *)socket_packet->data);
    return sli_si91x_get_socket_from_id(
      -1,
      RESET,
      (int16_t)(socket_create_response->socket_type[0] | (socket_create_response->socket_type[1] << 8)));
  } else if (socket_packet->command == SLI_WLAN_RSP_SOCKET_ACCEPT) {
    const uint16_t port = ((sli_si91x_rsp_ltcp_est_t *)socket_packet->data)->src_port_num;
    for (uint8_t i = 0; i < SLI_NUMBER_OF_SOCKETS; ++i) {
      if (sli_si91x_sockets[i] != NULL && sli_si91x_sockets[i]->local_address.sin6_port == port
          && sli_si91x_sockets[i]->state == LISTEN) {
        return sli_si91x_sockets[sli_si91x_sockets[i]->client_id];
      }
    }
    return NULL;
  } else if (socket_packet->command == SLI_WLAN_RSP_SOCKET_CLOSE) {
    if (((sl_si91x_socket_close_response_t *)socket_packet->data)->socket_id == 0) {
      const uint16_t port = ((sl_si91x_socket_close_response_t *)socket_packet->data)->port_number;
      for (uint8_t i = 0; i < SLI_NUMBER_OF_SOCKETS; ++i) {
        if (sli_si91x_sockets[i] != NULL && sli_si91x_sockets[i]->local_address.sin6_port == port
            && sli_si91x_sockets[i]->state == LISTEN) {
          return sli_si91x_sockets[i];
        }
      }
      return NULL;
    }
    return sli_si91x_get_socket_from_id(socket_id, RESET, -1);
  } else {
    return sli_si91x_get_socket_from_id(socket_id, LISTEN, -1);
  }
}
#endif

static void sli_handle_dhcp_and_rejoin_failure(void *sdk_context,
                                               sl_wifi_buffer_t *response_buffer,
                                               uint16_t frame_status)
{
  sl_status_t status;
  sl_wifi_buffer_t *temp_buffer = NULL;
  sli_si91x_queue_packet_t *node;

  // Retrieve the packet data from the response buffer
  sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(response_buffer, 0, NULL);

  // Create a generic RX packet from the parameters
  status = sli_create_generic_rx_packet_from_params(&node, &temp_buffer, 0, 0, NULL, frame_status);
  if (status != SL_STATUS_OK) {
    return; // Exit if packet creation fails
  }

  sl_wifi_buffer_t *dummy_packet_buffer = NULL;

  // Allocate buffer for the dummy packet
  status = sli_si91x_host_allocate_buffer(&dummy_packet_buffer,
                                          SL_WIFI_RX_FRAME_BUFFER,
                                          (sizeof(packet->desc) + packet->length),
                                          SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  if (status != SL_STATUS_OK) {
    sli_si91x_host_free_buffer(temp_buffer); // Free temp buffer on failure
    return;                                  // Exit if buffer allocation fails
  }

  // Get the dummy packet data from the allocated buffer
  sl_wifi_system_packet_t *dummy_packet = sli_wifi_host_get_buffer_data(dummy_packet_buffer, 0, NULL);
  node->host_packet                     = dummy_packet_buffer; // Link dummy packet to the node
  node->sdk_context                     = sdk_context;

  // Copy the original packet data to the dummy packet
  memcpy(dummy_packet, packet, (sizeof(packet->desc) + packet->length));

  // Enqueue the node packet and trigger the async event
  sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].event_queue, temp_buffer);
  set_async_event(NCP_HOST_NETWORK_NOTIFICATION_EVENT);
}

static void sli_handle_and_queue_wlan_response(sli_si91x_queue_packet_t *node,
                                               sl_wifi_buffer_t *buffer,
                                               sl_wifi_buffer_t *packet)
{
  if (node == NULL || buffer == NULL || packet == NULL) {
    return;
  }
  node->firmware_queue_id = SLI_WLAN_MGMT_Q;
  node->command_type      = SLI_WIFI_WLAN_CMD;
  node->sdk_context       = cmd_queues[SLI_WIFI_WLAN_CMD].sdk_context;
  node->flags             = cmd_queues[SLI_WIFI_WLAN_CMD].flags;
  buffer->id              = (uint8_t)cmd_queues[SLI_WIFI_WLAN_CMD].packet_id;
  packet->id              = (uint8_t)cmd_queues[SLI_WIFI_WLAN_CMD].packet_id;
  node->host_packet       = NULL;

  if (sl_si91x_host_elapsed_time(cmd_queues[SLI_WIFI_WLAN_CMD].command_tickcount)
      <= (cmd_queues[SLI_WIFI_WLAN_CMD].command_timeout)) {
    // Add the response packet to the WLAN response queue and set the WLAN response event
    sli_si91x_add_to_queue(&cmd_queues[SLI_WIFI_WLAN_CMD].rx_queue, packet);
    sli_wifi_set_event(NCP_HOST_WLAN_RESPONSE_EVENT);
  } else {
    // no user thread is waiting for the response so flush the packet and buffer
    sli_si91x_host_free_buffer(packet);
  }

  cmd_queues[SLI_WIFI_WLAN_CMD].command_tickcount = 0;
  cmd_queues[SLI_WIFI_WLAN_CMD].command_timeout   = 0;
}

static inline void sli_si91x_wifi_handle_rx_events(uint32_t *event)
{
  sl_status_t status;
  uint16_t temp;
  sl_wifi_performance_profile_v2_t current_power_profile_mode = { 0 };
  sli_si91x_queue_packet_t *node                              = NULL;
  sl_wifi_buffer_t *packet                                    = NULL;
  sl_wifi_buffer_t *buffer                                    = NULL;
  sl_wifi_buffer_t *error_packet                              = NULL;
  sli_si91x_queue_packet_t *error_node                        = NULL;
  uint8_t *data                                               = NULL;
  uint16_t length                                             = 0;
  uint8_t queue_id                                            = 0;
  uint16_t frame_type                                         = 0;
  uint16_t frame_status                                       = 0;

  // Check if there is an RX packet pending or bus RX event is set
  if ((*event & SL_SI91X_NCP_HOST_BUS_RX_EVENT)
#ifndef SLI_SI91X_MCU_INTERFACE
      && (interrupt_status & SLI_RX_PKT_PENDING)
#endif
      && (sli_si91x_bus_read_frame(&buffer) == SL_STATUS_OK)) { // Allocation from RX buffer type!

    if (current_performance_profile != HIGH_PERFORMANCE) {
      sl_si91x_host_clear_sleep_indicator();
    }

    // Check if the rx queue is empty
#ifdef SLI_SI91X_MCU_INTERFACE
    if (sli_si91x_buffer_queue_empty(&sli_ahb_bus_rx_queue))
#else
#ifdef SL_NCP_UART_INTERFACE
    if (sli_si91x_buffer_queue_empty(&sli_uart_bus_rx_queue))
#endif
#endif
    {
      *event &= ~SL_SI91X_NCP_HOST_BUS_RX_EVENT; // Reset the event flag
    }

    data = (uint8_t *)sli_wifi_host_get_buffer_data(buffer, 0, &length);
    // Process the frame
    queue_id     = ((data[1] & 0xF0) >> 4);                // Extract the queue ID
    frame_type   = (uint16_t)(data[2] + (data[3] << 8));   // Extract the frame type
    frame_status = (uint16_t)(data[12] + (data[13] << 8)); // Extract the frame status
#ifdef SLI_SI91X_MCU_INTERFACE
    if ((frame_type == SLI_COMMON_RSP_TA_M4_COMMANDS) || (frame_type == SLI_WLAN_REQ_SET_CERTIFICATE)
        || (frame_type == SLI_COMMON_RSP_SOFT_RESET)) {
      // clear flag
      sli_si91x_update_flash_command_status(false);
    }
#endif

    const sl_wifi_system_packet_t *response = (const sl_wifi_system_packet_t *)data;
    SL_PRINT_STRING_DEBUG("><<<< Rx -> queueId : %u, frameId : 0x%x, ", queue_id, frame_type);
    SL_PRINT_STRING_DEBUG("frameStatus: 0x%x, length : %u\n", frame_status, (response->length & (~(0xF000))));

    switch (queue_id) {
      case SLI_WLAN_MGMT_Q: {
        // Erase queue ID as it overlays with the length field which is only 24-bit
        data[1] &= 0xF;
        switch (frame_type) {
          // Handle different frame types within the WLAN management queue
          case SLI_WIFI_RSP_OPERMODE:
          case SLI_COMMON_RSP_SOFT_RESET:
          case SLI_COMMON_RSP_PWRMODE: {
            if (frame_type == cmd_queues[SLI_WIFI_COMMON_CMD].frame_type) {
              if ((SLI_COMMON_RSP_PWRMODE == frame_type) && (frame_status == SL_STATUS_OK)) {
                sli_wifi_get_current_performance_profile(&current_power_profile_mode);
                current_performance_profile = current_power_profile_mode.profile;
              }
              if (current_performance_profile != HIGH_PERFORMANCE) {
                // Clear the sleep indicator if the device is not in high-performance mode
                sl_si91x_host_clear_sleep_indicator();
              }
              global_queue_block = false;
            }
          }
            // intentional fallthrough
            __attribute__((fallthrough));
          case SLI_COMMON_RSP_GET_EFUSE_DATA:
          case SLI_COMMON_RSP_GET_RAM_DUMP:
          case SLI_WIFI_RSP_ANTENNA_SELECT:
          case SLI_COMMON_RSP_ENCRYPT_CRYPTO:
          case SLI_SI91X_FW_FALLBACK_RSP_FROM_HOST:
#ifdef SLI_PUF_ENABLE
          case SLI_COMMON_RSP_PUF_ENROLL:
          case SLI_COMMON_RSP_PUF_DIS_ENROLL:
          case SLI_COMMON_RSP_PUF_START:
          case SLI_COMMON_RSP_PUF_SET_KEY:
          case SLI_COMMON_RSP_PUF_DIS_SET_KEY:
          case SLI_COMMON_RSP_PUF_GET_KEY:
          case SLI_COMMON_RSP_PUF_DIS_GET_KEY:
          case SLI_COMMON_RSP_PUF_LOAD_KEY:
          case SLI_COMMON_RSP_AES_ENCRYPT:
          case SLI_COMMON_RSP_AES_DECRYPT:
          case SLI_COMMON_RSP_AES_MAC:
          case SLI_COMMON_RSP_PUF_INTR_KEY:
#endif
          case SLI_COMMON_RSP_SET_RTC_TIMER:
          case SLI_COMMON_RSP_GET_RTC_TIMER:
          case SLI_COMMON_RSP_TA_M4_COMMANDS:
          case SLI_COMMON_RSP_SET_CONFIG:
          case SLI_COMMON_RSP_GET_CONFIG:
          case SLI_COMMON_RSP_DEBUG_LOG:
          case SLI_COMMON_RSP_FEATURE_FRAME:
          case SLI_COMMON_RSP_NWP_LOGGING: {
            ++cmd_queues[SLI_WIFI_COMMON_CMD].rx_counter; // Increment the received counter for common commands

            // Check if this command is expected to have a response status
            if (((cmd_queues[SLI_WIFI_COMMON_CMD].flags & SLI_WIFI_PACKET_RESPONSE_STATUS)
                 == SLI_WIFI_PACKET_RESPONSE_STATUS)
                && (cmd_queues[SLI_WIFI_COMMON_CMD].frame_type == frame_type)) {
              // Allocate a packet to store the response
              status = sli_si91x_host_allocate_buffer(&packet,
                                                      SL_WIFI_RX_FRAME_BUFFER,
                                                      sizeof(sli_si91x_queue_packet_t),
                                                      1000);
              if (status != SL_STATUS_OK) {
                SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
                sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
                sli_si91x_host_free_buffer(buffer);
                break;
              }
              node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

              // Store packet ID before conditional to avoid use-after-free
              uint8_t pkt_id = (uint8_t)cmd_queues[SLI_WIFI_COMMON_CMD].packet_id;

              // Depending on the configuration, attach the original buffer or not
              if (SLI_WIFI_PACKET_RESPONSE_PACKET
                  == (cmd_queues[SLI_WIFI_COMMON_CMD].flags & SLI_WIFI_PACKET_RESPONSE_PACKET)) {
                node->host_packet = buffer;
                buffer->id        = pkt_id; // Safe to set here since buffer is kept
              } else {
                node->host_packet = NULL;
                sli_si91x_host_free_buffer(buffer);
                buffer = NULL; // Safeguard to prevent accidental use-after-free
              }

              // Populate packet metadata
              node->frame_status      = frame_status;
              node->firmware_queue_id = SLI_WLAN_MGMT_Q;
              node->command_type      = SLI_WIFI_COMMON_CMD;
              node->sdk_context       = cmd_queues[SLI_WIFI_COMMON_CMD].sdk_context;
              node->flags             = cmd_queues[SLI_WIFI_COMMON_CMD].flags;
              packet->id              = pkt_id;

              if (sl_si91x_host_elapsed_time(cmd_queues[SLI_WIFI_COMMON_CMD].command_tickcount)
                  <= (cmd_queues[SLI_WIFI_COMMON_CMD].command_timeout)) {
                // Add the response packet to the common response queue
                sli_wifi_append_to_buffer_queue(&cmd_queues[SLI_WIFI_COMMON_CMD].rx_queue, packet);
                sli_wifi_set_event(NCP_HOST_COMMON_RESPONSE_EVENT);
              } else {
                // no user thread is waiting for the response so flush the packet
                sli_si91x_host_free_buffer(packet);
                if ((cmd_queues[SLI_WIFI_COMMON_CMD].flags & SLI_WIFI_PACKET_RESPONSE_PACKET)
                    == SLI_WIFI_PACKET_RESPONSE_PACKET) {
                  sli_si91x_host_free_buffer(buffer);
                }
              }
              cmd_queues[SLI_WIFI_COMMON_CMD].command_tickcount = 0;
              cmd_queues[SLI_WIFI_COMMON_CMD].command_timeout   = 0;
            } else {
              sli_si91x_host_free_buffer(buffer);
            }

            // Marking a received frame as not in flight when it matches the expected type.
            if (frame_type == cmd_queues[SLI_WIFI_COMMON_CMD].frame_type) {
              cmd_queues[SLI_WIFI_COMMON_CMD].command_in_flight = false;
              cmd_queues[SLI_WIFI_COMMON_CMD].frame_type        = 0;
            }
            break;
          }
          case SLI_WIFI_RSP_BAND:
          case SLI_WIFI_RSP_INIT:
          case SLI_WLAN_RSP_RADIO:
          case SLI_WIFI_RSP_EAP_CONFIG:
          case SLI_WLAN_RSP_SET_CERTIFICATE:
          case SLI_WIFI_RSP_HOST_PSK:
          case SLI_WIFI_RSP_JOIN:
          case SLI_WIFI_RSP_SCAN:
          case SLI_WIFI_RSP_SCAN_RESULTS:
          case SLI_WLAN_RSP_FW_VERSION:
          case SLI_WLAN_RSP_FULL_FW_VERSION:
          case SLI_WLAN_RSP_FWUP:
          case SLI_WIFI_RSP_DISCONNECT:
          case SLI_WIFI_RSP_AP_STOP:
          case SLI_WIFI_RSP_RSSI:
          case SLI_WIFI_RSP_TSF:
          case SLI_WIFI_RSP_AP_CONFIGURATION:
          case SLI_WIFI_RSP_WPS_METHOD:
          case SLI_WIFI_RSP_QUERY_NETWORK_PARAMS:
          case SLI_WIFI_RSP_SET_MAC_ADDRESS:
          case SLI_WLAN_RSP_SET_REGION:
          case SLI_WLAN_RSP_SET_REGION_AP:
          case SLI_WIFI_RSP_MAC_ADDRESS:
          case SLI_WIFI_RSP_EXT_STATS:
          case SLI_WIFI_RSP_GET_STATS:
          case SLI_WIFI_RSP_RX_STATS:
          case SLI_WLAN_RSP_MODULE_STATE:
          case SLI_WIFI_RSP_QUERY_GO_PARAMS:
          case SLI_WIFI_RSP_ROAM_PARAMS:
          case SLI_WLAN_RSP_HTTP_OTAF:
          case SLI_WLAN_RSP_CLIENT_CONNECTED:
          case SLI_WLAN_RSP_CLIENT_DISCONNECTED:
          case SLI_WLAN_RSP_CALIB_WRITE:
          case SLI_WLAN_RSP_GET_DPD_DATA:
          case SLI_WLAN_RSP_CALIB_READ:
          case SLI_WLAN_RSP_FREQ_OFFSET:
          case SLI_WLAN_RSP_EVM_OFFSET:
          case SLI_WLAN_RSP_EVM_WRITE:
          case SLI_WLAN_RSP_EFUSE_READ:
          case SLI_WIFI_RSP_FILTER_BCAST_PACKETS:
          case SLI_WIFI_RSP_TWT_PARAMS:
          case SLI_WLAN_RSP_TWT_ASYNC:
          case SLI_WIFI_RSP_TWT_AUTO_CONFIG:
          case SLI_WIFI_RSP_11AX_PARAMS:
          case SLI_WIFI_RSP_RESCHEDULE_TWT:
          case SLI_WIFI_RSP_REJOIN_PARAMS:
          case SLI_WIFI_RSP_GAIN_TABLE:
          case SLI_WLAN_RSP_TX_TEST_MODE:
          case SLI_WLAN_RSP_TIMEOUT:
          case SLI_WIFI_RSP_BEACON_STOP:
          case SLI_WLAN_RSP_DYNAMIC_POOL:
          case SLI_WIFI_RSP_TRANSCEIVER_SET_CHANNEL:
          case SLI_WIFI_RSP_TRANSCEIVER_CONFIG_PARAMS:
          case SLI_WIFI_RSP_TRANSCEIVER_PEER_LIST_UPDATE:
          case SLI_WIFI_RSP_SET_TRANSCEIVER_MCAST_FILTER:
          case SLI_WIFI_RSP_TRANSCEIVER_FLUSH_DATA_Q:
          case SLI_WLAN_RSP_TRANSCEIVER_TX_DATA_STATUS:
          case SLI_WIFI_RSP_HT_CAPABILITIES:
          case SLI_WIFI_RSP_SET_MULTICAST_FILTER:
          case SLI_WIFI_RSP_VENDOR_IE: {
            ++cmd_queues[SLI_WIFI_WLAN_CMD].rx_counter;

            // Marking a received frame as not in flight when it matches the expected type
            if (frame_type == cmd_queues[SLI_WIFI_WLAN_CMD].frame_type) {
              cmd_queues[SLI_WIFI_WLAN_CMD].command_in_flight = false;
            }

            // Check for the following scenarios:
            // 1. If an asynchronous RX JOIN command fails (frame status not OK).
            // 2. If the station (STA) disconnects from a third-party Access Point (AP).
            // 3. If a third-party station disconnects from the DUT's Access Point (AP).
            // 4. If the DUT's Access Point (AP) stops operating.
            if (((SLI_WIFI_RSP_JOIN == frame_type && cmd_queues[SLI_WIFI_WLAN_CMD].frame_type != frame_type)
                 && (frame_status != SL_STATUS_OK))
                || (SLI_WIFI_RSP_DISCONNECT == frame_type) || (SLI_WLAN_RSP_CLIENT_DISCONNECTED == frame_type)
                || (SLI_WIFI_RSP_AP_STOP == frame_type)) {

              // create dummy packets for respective queues to be cleared
              sli_handle_dhcp_and_rejoin_failure(cmd_queues[SLI_WIFI_WLAN_CMD].sdk_context, buffer, frame_status);
            }
            if ((SLI_WIFI_RSP_JOIN == frame_type) && (frame_status != SL_STATUS_OK)) {
              // Reset current performance profile and set it to high performance
              sli_reset_coex_current_performance_profile();
              current_performance_profile = SL_WIFI_SYSTEM_ASSOCIATED_POWER_SAVE_LOW_LATENCY;
            } else if (SLI_WIFI_RSP_DISCONNECT == frame_type) {
              // Reset current performance profile and set it to high performance
              sli_reset_coex_current_performance_profile();
              current_performance_profile = SL_WIFI_SYSTEM_HIGH_PERFORMANCE;
            }

            // check if the frame type is valid
            if (((cmd_queues[SLI_WIFI_WLAN_CMD].flags & SLI_WIFI_PACKET_RESPONSE_STATUS)
                 == SLI_WIFI_PACKET_RESPONSE_STATUS)
                && (cmd_queues[SLI_WIFI_WLAN_CMD].frame_type == frame_type)) {

              // Allocate a buffer for the response packet
              status = sli_si91x_host_allocate_buffer(&packet,
                                                      SL_WIFI_RX_FRAME_BUFFER,
                                                      sizeof(sli_si91x_queue_packet_t),
                                                      1000);
              if (status != SL_STATUS_OK) {
                SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
                sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
                sli_si91x_host_free_buffer(buffer);
                break;
              }

              node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

              // Store packet ID before conditional to avoid use-after-free
              uint8_t pkt_id = (uint8_t)cmd_queues[SLI_WIFI_WLAN_CMD].packet_id;

              // Check if the packet response mode is set, and associate the host packet accordingly
              if (SLI_WIFI_PACKET_RESPONSE_PACKET
                  == (cmd_queues[SLI_WIFI_WLAN_CMD].flags & SLI_WIFI_PACKET_RESPONSE_PACKET)) {
                node->host_packet = buffer;
                buffer->id        = pkt_id; // Safe to set here since buffer is kept
              } else {
                node->host_packet = NULL;
                sli_si91x_host_free_buffer(buffer);
                buffer = NULL; // Safeguard to prevent accidental use-after-free
              }

              // Populate the response packet information
              node->frame_status      = frame_status;
              node->firmware_queue_id = SLI_WLAN_MGMT_Q;
              node->command_type      = SLI_WIFI_WLAN_CMD;
              node->sdk_context       = cmd_queues[SLI_WIFI_WLAN_CMD].sdk_context;
              node->flags             = cmd_queues[SLI_WIFI_WLAN_CMD].flags;
              packet->id              = pkt_id;

              if (sl_si91x_host_elapsed_time(cmd_queues[SLI_WIFI_WLAN_CMD].command_tickcount)
                  <= (cmd_queues[SLI_WIFI_WLAN_CMD].command_timeout)) {
                // Add the response packet to the WLAN response queue and set the WLAN response event
                sli_si91x_add_to_queue(&cmd_queues[SLI_WIFI_WLAN_CMD].rx_queue, packet);
                sli_wifi_set_event(NCP_HOST_WLAN_RESPONSE_EVENT);
              } else {
                // no user thread is waiting for the response so flush the packet
                sli_si91x_host_free_buffer(packet);
                if ((cmd_queues[SLI_WIFI_WLAN_CMD].flags & SLI_WIFI_PACKET_RESPONSE_PACKET)
                    == SLI_WIFI_PACKET_RESPONSE_PACKET) {
                  sli_si91x_host_free_buffer(buffer);
                }
              }
              cmd_queues[SLI_WIFI_WLAN_CMD].command_tickcount = 0;
              cmd_queues[SLI_WIFI_WLAN_CMD].command_timeout   = 0;
            } else {
              // The received frame does not match the expected response status and frame type,
              // so add it to the WLAN event queue and set the WLAN notification event
              sli_si91x_add_to_queue(&cmd_queues[SLI_WIFI_WLAN_CMD].event_queue, buffer);
              set_async_event(NCP_HOST_WLAN_NOTIFICATION_EVENT);
            }

            // Resetting the frame_type in cmd_queues when it matches the expected frame_type
            if (frame_type == cmd_queues[SLI_WIFI_WLAN_CMD].frame_type) {
              cmd_queues[SLI_WIFI_WLAN_CMD].frame_type = 0;
            }

            break;
          }
          // Handle WLAN response frame type for background scan
          case SLI_WIFI_RSP_BG_SCAN: {
            ++cmd_queues[SLI_WIFI_WLAN_CMD].rx_counter;

            // Check if the received frame matches the expected response status and frame type
            if (((cmd_queues[SLI_WIFI_WLAN_CMD].flags & SLI_WIFI_PACKET_RESPONSE_STATUS)
                 == SLI_WIFI_PACKET_RESPONSE_STATUS)
                && (cmd_queues[SLI_WIFI_WLAN_CMD].frame_type == frame_type)) {
              // Allocate a buffer for the response packet
              status = sli_si91x_host_allocate_buffer(&packet,
                                                      SL_WIFI_RX_FRAME_BUFFER,
                                                      sizeof(sli_si91x_queue_packet_t),
                                                      1000);
              if (status != SL_STATUS_OK) {
                SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
                sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
                sli_si91x_host_free_buffer(buffer);
                break;
              }
              node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

              node->frame_status = frame_status;
              sli_handle_and_queue_wlan_response(node, buffer, packet);
              sli_si91x_host_free_buffer(buffer);
            } else {
              // If frame status is OK, set bg_enabled flag
              if (frame_status == SL_STATUS_OK) {
                bg_enabled = true;
              }
              // Add the received frame to the WLAN event queue and set the WLAN notification event
              sli_si91x_add_to_queue(&cmd_queues[SLI_WIFI_WLAN_CMD].event_queue, buffer);
              set_async_event(NCP_HOST_WLAN_NOTIFICATION_EVENT);
            }
            // Marking a received frame as not in flight when it matches the expected type
            if (frame_type == cmd_queues[SLI_WIFI_WLAN_CMD].frame_type) {
              cmd_queues[SLI_WIFI_WLAN_CMD].command_in_flight = false;
              cmd_queues[SLI_WIFI_WLAN_CMD].frame_type        = 0;
            }
            break;
          }
          case SLI_WIFI_RSP_CONFIG: {
            ++cmd_queues[SLI_WIFI_WLAN_CMD].rx_counter;

            // Check if the received frame matches the expected response status and frame type
            if (((cmd_queues[SLI_WIFI_WLAN_CMD].flags & SLI_WIFI_PACKET_RESPONSE_STATUS)
                 == SLI_WIFI_PACKET_RESPONSE_STATUS)
                && (cmd_queues[SLI_WIFI_WLAN_CMD].frame_type == frame_type)) {
              // Allocate a buffer for the response packet
              status = sli_si91x_host_allocate_buffer(&packet,
                                                      SL_WIFI_RX_FRAME_BUFFER,
                                                      sizeof(sli_si91x_queue_packet_t),
                                                      1000);
              if (status != SL_STATUS_OK) {
                SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
                sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
                sli_si91x_host_free_buffer(buffer);
                break;
              }
              node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

              // Check if the frame status indicates an invalid configuration type
              if ((SL_STATUS_SI91X_INVALID_CONFIG_TYPE & 0xFF) == frame_status) {
                node->frame_status = 0;
              } else {
                node->frame_status = frame_status;
              }
              sli_handle_and_queue_wlan_response(node, buffer, packet);
            }

            // check if the frame type is valid.
            if (frame_type == cmd_queues[SLI_WIFI_WLAN_CMD].frame_type) {
              cmd_queues[SLI_WIFI_WLAN_CMD].command_in_flight = false;
              cmd_queues[SLI_WIFI_WLAN_CMD].frame_type        = 0;
            }

            sli_si91x_host_free_buffer(buffer);
            break;
          }
          case SLI_COMMON_RSP_ULP_NO_RAM_RETENTION: {
            //This frame will come, when the M4 is waken in without ram retention. This frame is equivalent to SLI_WIFI_RSP_CARDREADY
            sl_si91x_host_clear_sleep_indicator();
          }
            // intentional fallthrough
            __attribute__((fallthrough));
          case SLI_WIFI_RSP_CARDREADY: {
            ++cmd_queues[SLI_WIFI_COMMON_CMD].rx_counter;

            // Check if the frame type is valid
            if (frame_type == cmd_queues[SLI_WIFI_COMMON_CMD].frame_type) {
              // Mark the common command as not in flight
              cmd_queues[SLI_WIFI_COMMON_CMD].command_in_flight = false;
              cmd_queues[SLI_WIFI_COMMON_CMD].frame_type        = 0;
              cmd_queues[SLI_WIFI_COMMON_CMD].command_tickcount = 0;
              cmd_queues[SLI_WIFI_COMMON_CMD].command_timeout   = 0;
            }

            sli_si91x_host_free_buffer(buffer);
            sli_wifi_set_event(NCP_HOST_COMMON_RESPONSE_EVENT);
            break;
          }
          case SLI_WLAN_RSP_IPCONFV4:
          case SLI_WLAN_RSP_IPCONFV6:
          case SLI_WLAN_RSP_IPV4_CHANGE:
          case SLI_WLAN_RSP_OTA_FWUP:
          case SLI_WLAN_RSP_DNS_QUERY:
          case SLI_WLAN_RSP_DNS_SERVER_ADD:
          case SLI_WLAN_RSP_SET_SNI_EMBEDDED:
          case SLI_WLAN_RSP_MULTICAST:
          case SLI_WLAN_RSP_PING_PACKET:
          case SLI_WLAN_RSP_SNTP_CLIENT:
          case SLI_WLAN_RSP_EMB_MQTT_CLIENT:
          case SLI_WLAN_RSP_EMB_MQTT_PUBLISH_PKT:
          case SLI_WLAN_RSP_MQTT_REMOTE_TERMINATE:
          case SLI_WLAN_RSP_MDNSD:
          case SLI_WLAN_RSP_DISCOVER_SERVICE:
          case SLI_WLAN_RSP_NAT: {
            // Increment the received frame counter for network commands
            ++cmd_queues[SLI_SI91X_NETWORK_CMD].rx_counter;

            // Allocate a buffer for the response packet
            status =
              sli_si91x_host_allocate_buffer(&packet, SL_WIFI_RX_FRAME_BUFFER, sizeof(sli_si91x_queue_packet_t), 1000);
            if (status != SL_STATUS_OK) {
              SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
              sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
              sli_si91x_host_free_buffer(buffer);
              break;
            }
            node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

            // Set frame status, queue ID, and command type for the network response
            node->frame_status      = frame_status;
            node->firmware_queue_id = SLI_WLAN_MGMT_Q;
            node->command_type      = SLI_SI91X_NETWORK_CMD;

            // Check if the frame type is valid
            if (frame_type == cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type
                || (frame_type == SLI_WLAN_RSP_IPCONFV6
                    && cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type == SLI_WLAN_REQ_IPCONFV6)) {
              node->sdk_context = cmd_queues[SLI_SI91X_NETWORK_CMD].sdk_context;
              node->flags       = cmd_queues[SLI_SI91X_NETWORK_CMD].flags;
              packet->id        = (uint8_t)cmd_queues[SLI_SI91X_NETWORK_CMD].packet_id;
              buffer->id        = (uint8_t)cmd_queues[SLI_SI91X_NETWORK_CMD].packet_id;
            } else {
              node->sdk_context = NULL;
              node->flags       = 0;
              packet->id        = 0;
              buffer->id        = 0;
            }
            // Set the host packet to the received buffer
            node->host_packet = buffer;

            if (((frame_type == SLI_WLAN_RSP_MQTT_REMOTE_TERMINATE)
                 || (frame_type == SLI_WLAN_RSP_EMB_MQTT_CLIENT
                     && frame_status == (SL_STATUS_SI91X_MQTT_KEEP_ALIVE_TERMINATE_ERROR & ~BIT(16))))
                && (cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type == SLI_WLAN_RSP_EMB_MQTT_CLIENT)) {
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_in_flight = false;
              cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type        = 0;

              if (cmd_queues[SLI_SI91X_NETWORK_CMD].flags == 0) {
                sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].event_queue, packet);
                set_async_event(NCP_HOST_NETWORK_NOTIFICATION_EVENT);
                break;
              }

              // if command present in cmd_queues, an error RX packet is enqueued to network queue and command in flight is set to false.

              // Allocate a buffer for the error packet
              status = sli_si91x_host_allocate_buffer(&error_packet,
                                                      SL_WIFI_RX_FRAME_BUFFER,
                                                      sizeof(sli_si91x_queue_packet_t),
                                                      1000);
              if (status != SL_STATUS_OK) {
                SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
                sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
                sli_si91x_host_free_buffer(buffer);
                break;
              }
              error_node = sli_wifi_host_get_buffer_data(error_packet, 0, NULL);
              error_node->frame_status =
                (SL_STATUS_SI91X_MQTT_REMOTE_TERMINATE_ERROR & 0xFFFF); // error given by firmware for remote terminate
              error_node->host_packet = NULL;
              error_node->flags       = cmd_queues[SLI_SI91X_NETWORK_CMD].flags;
              error_packet->id        = (uint8_t)cmd_queues[SLI_SI91X_NETWORK_CMD].packet_id;

              sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].rx_queue, error_packet);
              sli_wifi_set_event(NCP_HOST_NETWORK_RESPONSE_EVENT);
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_tickcount = 0;
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_timeout   = 0;
              break;
            }

            // Check if it's a response packet, and handle accordingly
            if (((cmd_queues[SLI_SI91X_NETWORK_CMD].flags & SLI_WIFI_PACKET_RESPONSE_STATUS)
                 == SLI_WIFI_PACKET_RESPONSE_STATUS)
                && ((cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type == frame_type)
                    || (cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type == SLI_WLAN_REQ_IPCONFV6
                        && frame_type == SLI_WLAN_RSP_IPCONFV6))) {

              // Check if the response packet should be free or not
              if (SLI_WIFI_PACKET_RESPONSE_PACKET
                  != (cmd_queues[SLI_SI91X_NETWORK_CMD].flags & SLI_WIFI_PACKET_RESPONSE_PACKET)) {
                node->host_packet = NULL;
                sli_si91x_host_free_buffer(buffer);
              }
              if (sl_si91x_host_elapsed_time(cmd_queues[SLI_SI91X_NETWORK_CMD].command_tickcount)
                  <= (cmd_queues[SLI_SI91X_NETWORK_CMD].command_timeout)) {
                // Add the response packet to response queue and set the network response event
                sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].rx_queue, packet);
                sli_wifi_set_event(NCP_HOST_NETWORK_RESPONSE_EVENT);
              } else {
                // no user thread is waiting for the response so flush the packet
                sli_si91x_host_free_buffer(packet);
                if ((cmd_queues[SLI_SI91X_NETWORK_CMD].flags & SLI_WIFI_PACKET_RESPONSE_PACKET)
                    == SLI_WIFI_PACKET_RESPONSE_PACKET) {
                  sli_si91x_host_free_buffer(buffer);
                }
              }
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_tickcount = 0;
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_timeout   = 0;
            } else {
              // Add the packet to event queue and set the async network event
              sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].event_queue, packet);
              set_async_event(NCP_HOST_NETWORK_NOTIFICATION_EVENT);
            }

            // Check If the frame type is valid.
            if ((frame_type == cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type)
                || (frame_type == SLI_WLAN_RSP_IPCONFV6
                    && cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type == SLI_WLAN_REQ_IPCONFV6)) {
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_in_flight = false;
              cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type        = 0;
            }

            break;
          }
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
          case SLI_WLAN_RSP_CONN_ESTABLISH:
            frame_type = SLI_WLAN_RSP_SOCKET_ACCEPT;
            // intentional fallthrough
            __attribute__((fallthrough));
          case SLI_WLAN_RSP_SOCKET_CONFIG:
          case SLI_WLAN_RSP_SOCKET_CREATE:
          case SLI_WLAN_RSP_SOCKET_CLOSE:
          case SLI_WLAN_RSP_SELECT_REQUEST:
          case SLI_WLAN_RSP_SOCKET_READ_DATA: // Socket read data response only is expected incase of failure
          case SLI_WLAN_RSP_SOCKET_ACCEPT:
          case SLI_WLAN_RSP_REMOTE_TERMINATE: {
            // Find relevant socket
            sl_wifi_system_packet_t *socket_packet = (sl_wifi_system_packet_t *)data;
            sli_si91x_socket_t *socket             = get_socket_from_packet(socket_packet);
            sli_wifi_command_queue_t *socket_command_queue;

            if ((socket == NULL) || (frame_type == SLI_WLAN_RSP_SELECT_REQUEST)
                || (frame_type == SLI_WLAN_RSP_SOCKET_CREATE) || (frame_type == SLI_WLAN_RSP_SOCKET_CONFIG)) {
              socket_command_queue = &cmd_queues[SLI_SI91X_SOCKET_CMD];
            } else {
              socket_command_queue = &socket->command_queue;
            }

            if (frame_type == SLI_WLAN_RSP_SELECT_REQUEST) {
              // Allocate a buffer for the response packet
              status = sli_si91x_host_allocate_buffer(&packet,
                                                      SL_WIFI_RX_FRAME_BUFFER,
                                                      sizeof(sli_si91x_queue_packet_t),
                                                      1000);
              if (status != SL_STATUS_OK) {
                SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
                sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
                sli_si91x_host_free_buffer(buffer);
                break;
              }
              node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

              // Set frame status, queue ID, and command type for the network response
              node->frame_status      = frame_status;
              node->firmware_queue_id = SLI_WLAN_MGMT_Q;
              node->command_type      = SLI_SI91X_SOCKET_CMD;
              node->host_packet       = buffer;
              node->sdk_context       = NULL;
              // and set asynchronous socket notification event
              sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_SOCKET_CMD].event_queue, packet);
              set_async_event(NCP_HOST_SOCKET_NOTIFICATION_EVENT);
              break;
            }
            ++socket_command_queue->rx_counter;

            if ((socket_command_queue->command_timeout != 0)
                && (sl_si91x_host_elapsed_time(socket_command_queue->command_tickcount)
                    > (socket_command_queue->command_timeout))) {
              sli_si91x_host_free_buffer(buffer);
              socket_command_queue->command_tickcount = 0;
              socket_command_queue->command_timeout   = 0;
              break;
            }

            if (frame_type == SLI_WLAN_RSP_SOCKET_READ_DATA && socket_command_queue->frame_type == frame_type) {
              buffer->id                              = (uint8_t)(socket->command_queue.packet_id);
              socket_command_queue->command_in_flight = false;
              socket_command_queue->frame_type        = 0;
              sli_si91x_add_to_queue(&socket->rx_data_queue, buffer);
              sli_si91x_set_socket_event(1 << socket->index);
              break;
            }

            // Allocate a buffer for the response packet
            status =
              sli_si91x_host_allocate_buffer(&packet, SL_WIFI_RX_FRAME_BUFFER, sizeof(sli_si91x_queue_packet_t), 1000);
            if (status != SL_STATUS_OK) {
              SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
              sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
              sli_si91x_host_free_buffer(buffer);
              break;
            }
            node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

            // Set frame status, queue ID, and command type for the socket response
            node->frame_status      = frame_status;
            node->firmware_queue_id = SLI_WLAN_MGMT_Q;
            node->command_type      = SLI_SI91X_SOCKET_CMD;
            node->host_packet       = buffer;

            // Check if the frame type is valid
            if (socket_command_queue->frame_type == frame_type) {
              node->sdk_context = socket_command_queue->sdk_context;
              node->flags       = socket_command_queue->flags;
              buffer->id        = (uint8_t)socket_command_queue->packet_id;
              packet->id        = (uint8_t)socket_command_queue->packet_id;
            } else {
              node->sdk_context = NULL;
              node->flags       = 0;
              buffer->id        = 0;
              packet->id        = 0;
            }

            // Check if it's a response packet, and handle accordingly
            if ((socket_command_queue->flags & SLI_WIFI_PACKET_RESPONSE_STATUS)
                && (socket_command_queue->frame_type == frame_type)) {

              // Check if the response packet should be free or not
              if ((socket_command_queue->flags & SLI_WIFI_PACKET_RESPONSE_PACKET) == 0) {
                node->host_packet = NULL;
                sli_si91x_host_free_buffer(buffer);
              }

              // Add the response packet to the socket response queue and set the socket response event
              sli_si91x_add_to_queue(&socket_command_queue->rx_queue, packet);

              if (socket_command_queue == &cmd_queues[SLI_SI91X_SOCKET_CMD]) {
                sli_wifi_set_event(NCP_HOST_SOCKET_RESPONSE_EVENT);
              } else {
                sli_si91x_set_socket_event(1 << socket->index);
              }
              socket_command_queue->command_tickcount = 0;
              socket_command_queue->command_timeout   = 0;

            } else {
              // and set asynchronous socket notification event
              sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_SOCKET_CMD].event_queue, packet);
              set_async_event(NCP_HOST_SOCKET_NOTIFICATION_EVENT);
            }

            // If the frame_type matches the expected frame_type for the socket command
            // mark the command as not in flight and clear the frame_type
            if (frame_type == socket_command_queue->frame_type) {
              socket_command_queue->command_in_flight = false;
              socket_command_queue->frame_type        = 0;
            }

            break;
          }
#endif

          // Handle TCP ACK Indication response
          case SLI_WLAN_RSP_TCP_ACK_INDICATION: {
            // Allocate memory for a new packet
            status =
              sli_si91x_host_allocate_buffer(&packet, SL_WIFI_RX_FRAME_BUFFER, sizeof(sli_si91x_queue_packet_t), 1000);
            if (status != SL_STATUS_OK) {
              SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
              sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
              sli_si91x_host_free_buffer(buffer);
              break;
            }
            node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

            // Populate the packet's information
            node->frame_status      = frame_status;
            node->firmware_queue_id = SLI_WLAN_MGMT_Q;
            node->command_type      = SLI_SI91X_SOCKET_CMD;
            node->host_packet       = buffer;

            node->sdk_context = NULL;
            node->flags       = 0;
            buffer->id        = 0;
            packet->id        = 0;

            // Add the packet to the socket event queue and set the asynchronous socket notification event
            sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_SOCKET_CMD].event_queue, packet);
            set_async_event(NCP_HOST_SOCKET_NOTIFICATION_EVENT);
            break;
          }
#ifdef SLI_SI91X_INTERNAL_HTTP_CLIENT
          case SLI_WLAN_RSP_HTTP_CLIENT_GET:
          case SLI_WLAN_RSP_HTTP_CLIENT_POST:
          case SLI_WLAN_RSP_HTTP_CLIENT_POST_DATA: {
            ++cmd_queues[SLI_SI91X_NETWORK_CMD].rx_counter;

            if (cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type == SLI_WLAN_RSP_HTTP_CLIENT_GET) {
              // If it's an HTTP GET response, check if the frame_status is not OK or if end_of_data is set to 1.
              sl_wifi_system_packet_t *get_response_packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
              const uint16_t *end_of_data                  = (uint16_t *)&get_response_packet->data;

              if (frame_status != SL_STATUS_OK || *end_of_data == 1) {
                // Mark the command as not in flight and clear the frame_type
                cmd_queues[SLI_SI91X_NETWORK_CMD].command_in_flight = false;
                cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type        = 0;
              }
            } else {
              // For other HTTP responses, mark the command as not in flight and clear the frame_type
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_in_flight = false;
              cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type        = 0;
            }

            // Allocate memory for a new packet
            status =
              sli_si91x_host_allocate_buffer(&packet, SL_WIFI_RX_FRAME_BUFFER, sizeof(sli_si91x_queue_packet_t), 1000);
            if (status != SL_STATUS_OK) {
              SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
              sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
              sli_si91x_host_free_buffer(buffer);
              break;
            }
            node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

            // Populate the packet's information
            node->host_packet       = buffer;
            node->frame_status      = frame_status;
            node->firmware_queue_id = SLI_WLAN_MGMT_Q;
            node->command_type      = SLI_SI91X_NETWORK_CMD;
            node->sdk_context       = cmd_queues[SLI_SI91X_NETWORK_CMD].sdk_context;
            node->flags             = cmd_queues[SLI_SI91X_NETWORK_CMD].flags;
            buffer->id              = (uint8_t)cmd_queues[SLI_SI91X_NETWORK_CMD].packet_id;
            packet->id              = (uint8_t)cmd_queues[SLI_SI91X_NETWORK_CMD].packet_id;

            // Add the packet to the network event queue and set the asynchronous network notification event
            sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].event_queue, packet);
            set_async_event(NCP_HOST_NETWORK_NOTIFICATION_EVENT);
            break;
          }
          case SLI_WLAN_RSP_HTTP_ABORT:
          case SLI_WLAN_RSP_HTTP_CLIENT_PUT: {
            ++cmd_queues[SLI_SI91X_NETWORK_CMD].rx_counter;

            status =
              sli_si91x_host_allocate_buffer(&packet, SL_WIFI_RX_FRAME_BUFFER, sizeof(sli_si91x_queue_packet_t), 1000);
            if (status != SL_STATUS_OK) {
              SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
              sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
              break;
            }
            node = sli_wifi_host_get_buffer_data(packet, 0, &temp);

            // Populate the packet's information
            node->frame_status      = frame_status;
            node->firmware_queue_id = SLI_WLAN_MGMT_Q;
            node->command_type      = SLI_SI91X_NETWORK_CMD;
            node->sdk_context       = cmd_queues[SLI_SI91X_NETWORK_CMD].sdk_context;
            node->flags             = cmd_queues[SLI_SI91X_NETWORK_CMD].flags;
            buffer->id              = (uint8_t)cmd_queues[SLI_SI91X_NETWORK_CMD].packet_id;
            packet->id              = (uint8_t)cmd_queues[SLI_SI91X_NETWORK_CMD].packet_id;

            if (((cmd_queues[SLI_SI91X_NETWORK_CMD].flags & SLI_WIFI_PACKET_RESPONSE_STATUS)
                 == SLI_WIFI_PACKET_RESPONSE_STATUS)
                && (cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type == frame_type)) {
              // If it's a response status and the frame_type matches, set host_packet to NULL and free the buffer
              node->host_packet = NULL;
              sli_si91x_host_free_buffer(buffer);

              if (sl_si91x_host_elapsed_time(cmd_queues[SLI_SI91X_NETWORK_CMD].command_tickcount)
                  <= (cmd_queues[SLI_SI91X_NETWORK_CMD].command_timeout)) {
                // Add the packet to the network response queue and set the network response event
                sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].rx_queue, packet);
                sli_wifi_set_event(NCP_HOST_NETWORK_RESPONSE_EVENT);
              } else {
                // no user thread is waiting for the response so flush the packet and buffer
                sli_si91x_host_free_buffer(packet);
                if ((cmd_queues[SLI_SI91X_NETWORK_CMD].flags & SLI_WIFI_PACKET_RESPONSE_STATUS)
                    != SLI_WIFI_PACKET_RESPONSE_STATUS) {
                  sli_si91x_host_free_buffer(buffer);
                }
              }
            } else {
              // For other cases, set host_packet to buffer and add it to the network event queue
              node->host_packet = buffer;

              sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].event_queue, packet);
              set_async_event(NCP_HOST_NETWORK_NOTIFICATION_EVENT);
            }

            if (frame_type == cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type) {
              // mark the command as not in flight and clear the frame_type
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_in_flight = false;
              cmd_queues[SLI_SI91X_NETWORK_CMD].frame_type        = 0;
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_tickcount = 0;
              cmd_queues[SLI_SI91X_NETWORK_CMD].command_timeout   = 0;
            }
            break;
          }
#endif
          default: {
            // frame_type doesn't match any known cases
            if (PRINT_ERROR_LOGS) {
              PRINT_ERROR_STATUS(INFO_TAG, frame_type);
            }
            // Free the buffer
            sli_si91x_host_free_buffer(buffer);
            break;
          }
        }
        break;
      }

      case SLI_WLAN_DATA_Q: {
        // Erase queue ID as it overlays with the length field which is only 24-bit
        data[1] &= 0xF;
        if (frame_type == SLI_RECEIVE_RAW_DATA) {
          // If the frame type is raw data reception
          SL_PRINT_STRING_INFO("Raw Data\n");

#if defined(SLI_SI91X_OFFLOAD_NETWORK_STACK) && !defined(SLI_SI91X_NETWORK_DUAL_STACK)
          // Offload only mode is not enabled
          sl_wifi_system_packet_t *socket_packet = (sl_wifi_system_packet_t *)data;
          sli_si91x_socket_t *socket             = NULL;
          int socket_id                          = sli_si91x_get_socket_id(socket_packet);
          socket                                 = sli_si91x_get_socket_from_id(socket_id, LISTEN, -1);

          // Check if we found a matching socket
          if (socket != NULL) {
            buffer->id = (uint8_t)(socket->command_queue.packet_id);
            // Check if command has timed out
            if (socket->command_queue.command_tickcount == 0
                || (sl_si91x_host_elapsed_time(socket->command_queue.command_tickcount)
                    <= (socket->command_queue.command_timeout))) {
              if (((socket->command_queue.frame_type == SLI_WLAN_RSP_SOCKET_READ_DATA)
                   || (socket->command_queue.frame_type == socket_packet->command))
                  && socket->command_queue.command_in_flight
                  && socket->command_queue.flags & SLI_WIFI_PACKET_RESPONSE_PACKET) {
                socket->command_queue.command_in_flight = false;
                sli_si91x_add_to_queue(&socket->rx_data_queue, buffer);
                sli_si91x_set_socket_event(1 << socket->index);
              } else {
                sli_si91x_add_to_queue(&socket->rx_data_queue, buffer);
                set_async_event(NCP_HOST_SOCKET_DATA_NOTIFICATION_EVENT);
              }
            } else {
              sli_si91x_add_to_queue(&socket->rx_data_queue, buffer);
              set_async_event(NCP_HOST_SOCKET_DATA_NOTIFICATION_EVENT);
            }
            socket->command_queue.command_tickcount = 0;
            socket->command_queue.command_timeout   = 0;
          }
#elif defined(SLI_SI91X_NETWORK_DUAL_STACK)
          extern bool bypass_mode_enabled;

          if (!bypass_mode_enabled) {
            sl_wifi_system_packet_t *socket_packet = (sl_wifi_system_packet_t *)data;
            sli_si91x_socket_t *socket             = NULL;
            int socket_id                          = sli_si91x_get_socket_id(socket_packet);
            socket                                 = sli_si91x_get_socket_from_id(socket_id, LISTEN, -1);

            // Check if we found a matching socket
            if (socket != NULL) {
              buffer->id = (uint8_t)(socket->command_queue.packet_id);
              // Check if command has timed out
              if (socket->command_queue.command_tickcount == 0
                  || (sl_si91x_host_elapsed_time(socket->command_queue.command_tickcount)
                      <= (socket->command_queue.command_timeout))) {
                if (((socket->command_queue.frame_type == SLI_WLAN_RSP_SOCKET_READ_DATA)
                     || (socket->command_queue.frame_type == socket_packet->command))
                    && socket->command_queue.command_in_flight
                    && socket->command_queue.flags & SI91X_PACKET_RESPONSE_PACKET) {
                  socket->command_queue.command_in_flight = false;
                  sli_si91x_add_to_queue(&socket->rx_data_queue, buffer);
                  sli_si91x_set_socket_event(1 << socket->index);
                } else {
                  sli_si91x_add_to_queue(&socket->rx_data_queue, buffer);
                  set_async_event(NCP_HOST_SOCKET_DATA_NOTIFICATION_EVENT);
                }
              } else {
                sli_si91x_add_to_queue(&socket->rx_data_queue, buffer);
                set_async_event(NCP_HOST_SOCKET_DATA_NOTIFICATION_EVENT);
              }
              socket->command_queue.command_tickcount = 0;
              socket->command_queue.command_timeout   = 0;
            }
          } else {
            // If SLI_SI91X_OFFLOAD_NETWORK_STACK is defined and dual stack mode is enabled, process the raw data frame.
            sl_si91x_host_process_data_frame(SL_WIFI_CLIENT_INTERFACE, buffer);
            sli_si91x_host_free_buffer(buffer);
          }
#else
          // In bypass mode, process the data frame and free the buffer.
          sl_si91x_host_process_data_frame(SL_WIFI_CLIENT_INTERFACE, buffer);
          sli_si91x_host_free_buffer(buffer);
#endif
        } else if (frame_type == SLI_NET_DUAL_STACK_RX_RAW_DATA_FRAME) {
          // If network dual stack mode is enabled, process the received data frame of type 0x1 and free the buffer.
          sl_si91x_host_process_data_frame(SL_WIFI_CLIENT_INTERFACE, buffer);
          sli_si91x_host_free_buffer(buffer);
        } else if (frame_type == SLI_SI91X_WIFI_RX_DOT11_DATA) {
          ++cmd_queues[SLI_WIFI_WLAN_CMD].rx_counter;

          // Marking a received frame as not in flight when it matches the expected type
          if (frame_type == cmd_queues[SLI_WIFI_WLAN_CMD].frame_type) {
            cmd_queues[SLI_WIFI_WLAN_CMD].command_in_flight = false;
            cmd_queues[SLI_WIFI_WLAN_CMD].command_tickcount = 0;
            cmd_queues[SLI_WIFI_WLAN_CMD].command_timeout   = 0;
          }

          // Add it to the WLAN event queue and set the WLAN notification event
          sli_si91x_add_to_queue(&cmd_queues[SLI_WIFI_WLAN_CMD].event_queue, buffer);
          set_async_event(NCP_HOST_WLAN_NOTIFICATION_EVENT);
        }
        break;
      }

#ifdef SLI_SI91X_ENABLE_BLE
      case SLI_BT_Q: {
        SL_PRINT_STRING_INFO("Received BLE packet\n");
        // Increment the receive counter for the Bluetooth command
        ++cmd_queues[SLI_SI91X_BT_CMD].rx_counter;

        if (frame_type == cmd_queues[SLI_SI91X_BT_CMD].frame_type) {
          // Mark the command as not in flight and clear the frame_type
          cmd_queues[SLI_SI91X_BT_CMD].command_in_flight = false;
          cmd_queues[SLI_SI91X_BT_CMD].frame_type        = 0;
        }

        // Add it to the BLE event queue and set the BLE notification event
        sli_si91x_add_to_queue(&cmd_queues[SLI_SI91X_BT_CMD].event_queue, buffer);
        set_async_event(NCP_HOST_BLE_NOTIFICATION_EVENT);
        break;
      }
#endif
      case SLI_LOG_Q: {
        ++cmd_queues[SLI_WIFI_COMMON_CMD].rx_counter;

        sli_si91x_add_to_queue(&cmd_queues[SLI_WIFI_COMMON_CMD].event_queue, buffer);
        set_async_event(NCP_HOST_COMMON_NOTIFICATION_EVENT);
        break;
      }

      default: {
        // If the frame_type doesn't match any known cases
        // Free the buffer
        sli_si91x_host_free_buffer(buffer);
        break;
      }
    }
    status = sli_submit_rx_buffer();
    if (status == SL_STATUS_ALLOCATION_FAILED) {
      sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
    }
  } else {
    *event &= ~SL_SI91X_NCP_HOST_BUS_RX_EVENT; // Reset the event flag
  }

  return;
}

static inline void sli_si91x_wifi_handle_tx_event(uint32_t *event)
{
  if (*event & SL_SI91X_ALL_TX_PENDING_COMMAND_EVENTS) {
    // This condition is checked before writing frames to the bus
    for (uint8_t i = 0; i < SI91X_CMD_MAX; i++) {
      if (!(*event & (SL_SI91X_TX_PENDING_FLAG(i)))) {
        continue;
      }
      if (cmd_queues[i].command_in_flight == true) {
        tx_command_queues_command_in_flight_status |= SL_SI91X_TX_PENDING_FLAG(i);
        continue;
      } else {
        tx_command_queues_command_in_flight_status &= ~SL_SI91X_TX_PENDING_FLAG(i);
      }
      uint8_t packet_type = (i == SLI_SI91X_BT_CMD) ? SLI_BLE_PACKET : SLI_WIFI_PACKET;
      // Check if the bus is ready for a packet
      if (!sli_si91x_is_bus_ready(global_queue_block, packet_type)) {
        break;
      }

      bus_write_frame(&cmd_queues[i], i, SL_WIFI_CONTROL_BUFFER, &global_queue_block);
      if (sli_si91x_buffer_queue_empty(&cmd_queues[i].tx_queue)) {
        *event &= ~SL_SI91X_TX_PENDING_FLAG(i);
        tx_command_queues_status &= ~SL_SI91X_TX_PENDING_FLAG(i);
      }
    }
  }

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
  if (tx_socket_command_queues_status & (~tx_command_queues_command_in_flight_status)) {
    for (uint8_t i = 0; i < SLI_NUMBER_OF_SOCKETS; i++) {
      if (sli_si91x_sockets[i] != NULL
          && !sli_si91x_buffer_queue_empty(&sli_si91x_sockets[i]->command_queue.tx_queue)) {
        if (sli_si91x_sockets[i]->command_queue.command_in_flight == true) {
          tx_socket_command_command_in_flight_queues_status |= (1 << i);
          continue;
        } else {
          tx_socket_command_command_in_flight_queues_status &= ~(1 << i);
        }
        // Check if the bus is ready for a packet
        if (!sli_si91x_is_bus_ready(global_queue_block, SLI_WIFI_PACKET)) {
          break;
        }

        // Send the socket specific command
        bus_write_frame(&sli_si91x_sockets[i]->command_queue,
                        SLI_SI91X_SOCKET_CMD,
                        SL_WIFI_CONTROL_BUFFER,
                        &global_queue_block);
        if (sli_si91x_buffer_queue_empty(&sli_si91x_sockets[i]->command_queue.tx_queue)) {
          tx_socket_command_queues_status &= ~(1 << i);
        }
      }
    }
    // Clear the event flag for socket queues if there are no commands left
    if (tx_socket_command_queues_status == 0) {
      *event &= ~SL_SI91X_SOCKET_COMMAND_TX_PENDING_EVENT;
    }
  }

  if (*event & SL_SI91X_SOCKET_DATA_TX_PENDING_EVENT) {
    bool all_socket_data_sent = true;
    // Check each socket if it has something to send
    for (uint8_t i = 0; i < SLI_NUMBER_OF_SOCKETS; ++i) {
      if (sli_si91x_sockets[i] != NULL && !sli_si91x_buffer_queue_empty(&sli_si91x_sockets[i]->tx_data_queue)) {
        all_socket_data_sent = false;

        // Check if the bus is ready for a packet
        if (!sli_si91x_is_bus_ready(global_queue_block, SLI_WIFI_PACKET)) {
          break;
        }

        sl_status_t status = bus_write_data_frame(&sli_si91x_sockets[i]->tx_data_queue);
        if (status == SL_STATUS_OK) {
          // Atomic protection for data_buffer_count to prevent race condition
          CORE_irqState_t state1 = CORE_EnterAtomic();
          --sli_si91x_sockets[i]->data_buffer_count;
          CORE_ExitAtomic(state1);
        }
        if (sli_si91x_buffer_queue_empty(&sli_si91x_sockets[i]->tx_data_queue)) {
          tx_socket_data_queues_status &= ~(1 << i);
        }
      }
    }

    // Clear event bit if we confirmed no more packets to send
    if (all_socket_data_sent) {
      *event &= ~SL_SI91X_SOCKET_DATA_TX_PENDING_EVENT;
    }
  }
#endif
  if (*event & SL_SI91X_GENERIC_DATA_TX_PENDING_EVENT) {
    // Check if the bus is ready for a packet
    if (sli_si91x_is_bus_ready(global_queue_block, SLI_WIFI_PACKET)) {
      bus_write_data_frame(&sli_tx_data_queue);
      if (sli_si91x_buffer_queue_empty(&sli_tx_data_queue)) {
        *event &= ~SL_SI91X_GENERIC_DATA_TX_PENDING_EVENT;
        tx_generic_socket_data_queues_status &= ~(SL_SI91X_GENERIC_DATA_TX_PENDING_EVENT);
      }
    }
  }

  return;
}

/******************************************************
 *               Function Definitions
 ******************************************************/
void sli_wifi_event_handler_init(void)
{
  // Array to track the status of commands in flight

  cmd_queues[SLI_WIFI_COMMON_CMD].sequential   = true;
  cmd_queues[SLI_WIFI_WLAN_CMD].sequential     = true;
  cmd_queues[SLI_SI91X_NETWORK_CMD].sequential = true;
  cmd_queues[SLI_SI91X_BT_CMD].sequential      = true;
  cmd_queues[SLI_SI91X_SOCKET_CMD].sequential  = true;
}

void sli_wifi_event_handler_deinit(void)
{
}

uint32_t sli_wifi_event_handler_get_flags_to_wait_on(void)
{
  return BUS_THREAD_EVENTS;
}

// Helper function to extract wait time from the packed lookup table based on current queue/buffer status.
static inline int32_t sli_get_wait_time(bool wifi_buffer_full,
                                        bool wifi_packets,
                                        bool ble_buffer_full,
                                        bool ble_packets)
{
  // Compose a 4-bit key from the status flags.
  uint8_t key = (wifi_buffer_full << 3) | (wifi_packets << 2) | (ble_buffer_full << 1) | (ble_packets);

  // Extract the 2-bit wait type for the key from the packed lookup table.
  sli_wait_time_type_t type = (sli_si91x_command_engine_wait_time_lookup >> (2 * key)) & 0x3;

  switch (type) {
    case SLI_WIFI_WAIT_TIME_ZERO:
      return 0; // No wait
    case SLI_WIFI_WAIT_TIME_YIELD:
      return 10; // Yield for 10 ms
    case SLI_WIFI_WAIT_TIME_WAITFOREVER:
      return osWaitForever; // Wait indefinitely
    default:
      return osWaitForever;
  }
}

// Inline function to determine the optimal wait time for the command engine thread
extern inline uint32_t sli_wifi_event_handler_get_wait_time(uint32_t *event)
{
  // If an RX event is pending, return immediately (no wait).
  if (*event & SL_SI91X_NCP_HOST_BUS_RX_EVENT) {
    return 0;
  }
  // If the global queue is blocked, return wait forever.
  else if (global_queue_block) {
    return osWaitForever;
  }

  // Flags to track if Wi-Fi or BLE TX queues are pending and if their buffers are full.
  bool wifi_tx_queues_pending = false;
  bool ble_tx_queues_pending  = false;
  bool wifi_buffer_full       = interrupt_status & SLI_WIFI_BUFFER_FULL;
  bool ble_buffer_full        = interrupt_status & SLI_BLE_BUFFER_FULL;

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
  // Check if any socket data or command queues are pending (excluding those in flight).
  wifi_tx_queues_pending =
    (tx_socket_data_queues_status != 0)
    || ((tx_socket_command_queues_status & (~tx_socket_command_command_in_flight_queues_status)) != 0);
#endif
  // Check if any Wi-Fi TX queues (excluding BLE) or generic data queues are pending.
  wifi_tx_queues_pending =
    wifi_tx_queues_pending
    || (((tx_command_queues_status & ~SL_SI91X_BT_TX_PENDING_EVENT) & (~tx_command_queues_command_in_flight_status))
        != 0)
    || (tx_generic_socket_data_queues_status != 0);

#ifdef SLI_SI91X_ENABLE_BLE
  // Check if any BLE TX queues are pending (excluding those in flight).
  ble_tx_queues_pending =
    ((tx_command_queues_status & SL_SI91X_BT_TX_PENDING_EVENT) & (~tx_command_queues_command_in_flight_status)) != 0;
#endif

  // Use a lookup table to select the appropriate wait time based on buffer and queue status.
  return sli_get_wait_time(wifi_buffer_full, wifi_tx_queues_pending, ble_buffer_full, ble_tx_queues_pending);
}

/// Thread which handles the notification events.
void sli_si91x_async_rx_event_handler_thread(const void *args)
{
  UNUSED_PARAMETER(args); // Prevent compiler warning about unused parameters.
  uint32_t event = 0;     // Event variable to track pending events.
  const uint32_t event_mask =
    (NCP_HOST_COMMON_NOTIFICATION_EVENT | NCP_HOST_WLAN_NOTIFICATION_EVENT | NCP_HOST_NETWORK_NOTIFICATION_EVENT
     | NCP_HOST_SOCKET_NOTIFICATION_EVENT | NCP_HOST_SOCKET_DATA_NOTIFICATION_EVENT | NCP_HOST_BLE_NOTIFICATION_EVENT
     | SLI_SI91X_NCP_HOST_COMMAND_ENGINE_STATUS_NOTIFICATION_EVENT);

  // Infinite loop to handle incoming events
  while (1) {
    event = osEventFlagsWait(si91x_async_events, event_mask, osFlagsWaitAny, osWaitForever);

    // Ignore any error events
    if (event & osFlagsError) {
      continue;
    }

    // Check and process common notification events
    if (event & NCP_HOST_COMMON_NOTIFICATION_EVENT) {
      sli_si91x_process_common_events();
    }

    // Check and process WLAN notification events
    if (event & NCP_HOST_WLAN_NOTIFICATION_EVENT) {
      sli_si91x_process_wifi_events();
    }
    // Check and process Network notification events
    if (event & NCP_HOST_NETWORK_NOTIFICATION_EVENT) {
      sli_si91x_process_network_events();
    }
    // Check and process Socket notification events
    if (event & NCP_HOST_SOCKET_NOTIFICATION_EVENT) {
      sli_si91x_process_socket_events();
    }
    // Check and process command engine status notifications
    if (event & SLI_SI91X_NCP_HOST_COMMAND_ENGINE_STATUS_NOTIFICATION_EVENT) {
      sli_si91x_process_command_engine_status_events();
    }
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
    // Check and process Socket Data notification events, if network stack offload is enabled
    if (event & NCP_HOST_SOCKET_DATA_NOTIFICATION_EVENT) {
      sli_si91x_process_socket_data_events();
    }
#endif

#ifdef SLI_SI91X_ENABLE_BLE
    // Check and process BLE notification events, if BLE is enabled
    if (event & NCP_HOST_BLE_NOTIFICATION_EVENT) {
      sli_si91x_process_ble_events();
    }
#endif
  }
}

// Process common notification events from the queue.
void sli_si91x_process_common_events()
{
  sl_wifi_buffer_t *buffer = NULL;

  while (sli_si91x_host_queue_status(&cmd_queues[SLI_WIFI_COMMON_CMD].event_queue) != 0) {
    if (sli_si91x_remove_from_queue(&cmd_queues[SLI_WIFI_COMMON_CMD].event_queue, &buffer) == SL_STATUS_OK) {
      sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);

      // Special handling for the log event
      if (SLI_COMMON_RSP_NWP_LOGGING == packet->command) {
        uint16_t pkt_length = (uint16_t)(packet->desc[0] + ((packet->desc[1] & 0x0F) << 8));
        sli_handle_nwp_log_packet(packet->data, pkt_length);
      }

      sli_si91x_host_free_buffer(buffer);
    }
  }
}

/// Process WLAN notification events from the queue.
void sli_si91x_process_wifi_events()
{
  sl_wifi_buffer_t *buffer = NULL;
  // Check if there are pending WLAN packet in the queue and process them
  while (sli_si91x_host_queue_status(&cmd_queues[SLI_WIFI_WLAN_CMD].event_queue) != 0) {
    // Remove packet from queue if available
    if (sli_si91x_remove_from_queue(&cmd_queues[SLI_WIFI_WLAN_CMD].event_queue, &buffer) == SL_STATUS_OK) {
      sl_wifi_system_packet_t *packet = sli_wifi_host_get_buffer_data(buffer, 0, NULL);
      uint16_t frame_status           = sli_get_si91x_frame_status(packet);
#ifdef SL_NET_COMPONENT_INCLUDED
      // Check if the command received is a WLAN join response
      if (packet->command == SLI_WIFI_RSP_JOIN) {
        sli_network_manager_message_t message = { 0 };    // Create a new message for the network manager
        message.interface = SL_NET_WIFI_CLIENT_INTERFACE; // Specify the network interface as the Wi-Fi client interface
        message.event_flags = frame_status != SL_STATUS_OK
                                ? SLI_NET_DISCONNECT_Q_EVENT
                                : SLI_NET_CONNECT_Q_EVENT; // Set the event flags based on the frame status
        osMessageQueuePut(sli_network_manager_request_queue,
                          &message,
                          SLI_NET_MSG_PRIO_NORMAL,
                          0); // Add the message to the network manager queue
      }
#endif // SL_NET_COMPONENT_INCLUDED
      // Invoke registered event handler if it exists
      if (si91x_event_handler != NULL) {
        sl_wifi_event_t wifi_event = sli_convert_si91x_event_to_sl_wifi_event(packet->command, frame_status);

        // Special handling for scan results events
        if (SLI_WLAN_RSP_SCAN_RESULTS == packet->command) {
          sli_handle_wifi_beacon(packet);
        }

        // Only call handler if event is valid
        if (wifi_event != SL_WIFI_INVALID_EVENT) {
          si91x_event_handler(wifi_event, buffer);
        }
      }
      // Free the buffer after processing the event
      sli_si91x_host_free_buffer(buffer);
    }
  }
}

/// Process Network notification events from the queue.
void sli_si91x_process_network_events()
{
  sl_wifi_buffer_t *buffer = NULL;
  // Avoid memory leaks by freeing packets if firmware sends network events without `sl_net` component.
  while (sli_si91x_host_queue_status(&cmd_queues[SLI_SI91X_NETWORK_CMD].event_queue) != 0) {
    if (sli_si91x_remove_from_queue(&cmd_queues[SLI_SI91X_NETWORK_CMD].event_queue, &buffer) == SL_STATUS_OK) {
      sli_si91x_queue_packet_t *data = (sli_si91x_queue_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
      sl_wifi_system_packet_t *packet =
        (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(data->host_packet, 0, NULL);

      sli_handle_wifi_events(data, packet);

      // Dispatch the network event to the appropriate handler.
      SL_NET_EVENT_DISPATCH_HANDLER(data, packet);

      // Free resources associated with the packet after processing
      sli_si91x_host_free_buffer(data->host_packet);
      sli_si91x_host_free_buffer(buffer);
    }
  }
}

/// Process Socket notification events from the queue.
void sli_si91x_process_socket_events()
{
  sl_wifi_buffer_t *buffer = NULL;
  // Process socket events in the queue
  while (sli_si91x_host_queue_status(&cmd_queues[SLI_SI91X_SOCKET_CMD].event_queue) != 0) {
    if (sli_si91x_remove_from_queue(&cmd_queues[SLI_SI91X_SOCKET_CMD].event_queue, &buffer) == SL_STATUS_OK) {
      sli_si91x_queue_packet_t *data = (sli_si91x_queue_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
      sl_wifi_system_packet_t *packet =
        (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(data->host_packet, 0, NULL);

      // Dispatch the socket event to the appropriate handler.
      SL_NET_EVENT_DISPATCH_HANDLER(data, packet);

      // Free resources after processing the packet.
      sli_si91x_host_free_buffer(data->host_packet);
      sli_si91x_host_free_buffer(buffer);
    }
  }
}

/// Process command engine status notification events
void sli_si91x_process_command_engine_status_events()
{
  sl_status_t status;
  // Attempt to retrieve the error message from the message queue
  while (osMessageQueueGet(sli_command_engine_status_msg_queue, &status, NULL, 0) == osOK) {
    // Call the Event Handler
    if (si91x_event_handler != NULL) {
      si91x_event_handler(SL_WIFI_COMMAND_ENGINE_STATUS_EVENT, (sl_wifi_buffer_t *)&status);
    }
  }
}

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
/// Process Socket Data events if network stack offloading is enabled.
void sli_si91x_process_socket_data_events()
{
  sl_wifi_buffer_t *buffer = NULL;

  // Process data packet for each socket if a data queue is present
  for (uint8_t i = 0; i < SLI_NUMBER_OF_SOCKETS; ++i) {
    if (sli_si91x_sockets[i] != NULL) {
      // If data is available in the RX queue, remove and process it
      if (sli_si91x_remove_from_queue(&sli_si91x_sockets[i]->rx_data_queue, &buffer) == SL_STATUS_OK) {
        if (sli_si91x_sockets[i]->recv_data_callback != NULL) {
          sl_wifi_system_packet_t *packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
          sl_si91x_socket_metadata_t *receive_data = (sl_si91x_socket_metadata_t *)packet->data;
          // Call the receive callback function with the received data
          sli_si91x_sockets[i]->recv_data_callback(i,
                                                   &packet->data[receive_data->offset],
                                                   receive_data->length,
                                                   receive_data);
        }
        // Free the buffer after data processing
        sli_si91x_host_free_buffer(buffer);
      }
    }
  }
}
#endif

#ifdef SLI_SI91X_ENABLE_BLE
/// Process BLE notification events if BLE is enabled.
void sli_si91x_process_ble_events()
{
  sl_wifi_buffer_t *buffer = NULL;
  // Process Bluetooth (BLE) events in the queue
  while (sli_si91x_host_queue_status(&cmd_queues[SLI_SI91X_BT_CMD].event_queue) != 0) {
    if (sli_si91x_remove_from_queue(&cmd_queues[SLI_SI91X_BT_CMD].event_queue, &buffer) == SL_STATUS_OK) {
      sl_wifi_system_packet_t *packet = (sl_wifi_system_packet_t *)sli_wifi_host_get_buffer_data(buffer, 0, NULL);
      // Handle Bluetooth response and free the buffer
      rsi_driver_process_bt_resp_handler(packet);
      sli_si91x_host_free_buffer(buffer);
    }
  }
}
#endif

uint32_t sli_wifi_command_engine_wait_for_event(uint32_t event_mask, uint32_t timeout)
{
  return sli_si91x_wait_for_event(event_mask, timeout);
}

uint32_t sli_wifi_command_engine_set_event(uint32_t event_mask)
{
  sli_wifi_set_event(event_mask);
  return 0;
}

void sli_wifi_handle_event(uint32_t *event)
{
#ifndef SLI_SI91X_MCU_INTERFACE
  // Wake device, if needed
  if ((current_performance_profile != HIGH_PERFORMANCE)) {
    while (sli_si91x_req_wakeup() != SL_STATUS_OK) {
      osDelay(SLI_SYSTEM_MS_TO_TICKS(1));
    }
  }

  if (sli_si91x_bus_read_interrupt_status(&interrupt_status) != SL_STATUS_OK) {
    return; // returning instead of continue.
  }

  // Check if there is no RX packet pending or the bus RX event is not set
  if (!((interrupt_status & SLI_RX_PKT_PENDING) || (*event & SL_SI91X_NCP_HOST_BUS_RX_EVENT))) {
    if (current_performance_profile != HIGH_PERFORMANCE) {
      // Clear the sleep indicator if the device is not in high-performance mode
      sl_si91x_host_clear_sleep_indicator();
    }
  }
#endif

  if (*event & SL_SI91X_NCP_HOST_BUS_RX_EVENT) {
    sli_si91x_wifi_handle_rx_events(event);
  }

  if (*event & SL_SI91X_ALL_TX_PENDING_COMMAND_EVENTS) {
    sli_si91x_wifi_handle_tx_event(event);
  }

  return;
}

sl_status_t sli_create_generic_rx_packet_from_params(sli_si91x_queue_packet_t **queue_packet,
                                                     sl_wifi_buffer_t **packet_buffer,
                                                     uint16_t packet_id,
                                                     uint8_t flags,
                                                     void *sdk_context,
                                                     uint16_t frame_status)
{
  sli_si91x_queue_packet_t *packet = NULL;
  sl_wifi_buffer_t *buffer         = NULL;
  sl_status_t status               = SL_STATUS_OK;
  uint16_t temp                    = 0;

  status = sli_si91x_host_allocate_buffer(&buffer,
                                          SL_WIFI_RX_FRAME_BUFFER,
                                          sizeof(sli_si91x_queue_packet_t),
                                          SLI_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
    sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  *packet_buffer = buffer;

  packet = sli_wifi_host_get_buffer_data(buffer, 0, &temp);
  if (packet == NULL) {
    return SL_STATUS_NOT_AVAILABLE;
  }

  // Fill dummy packet with details passed
  buffer->id                = packet_id;
  packet->sdk_context       = sdk_context;
  packet->flags             = flags;
  packet->frame_status      = frame_status;
  packet->firmware_queue_id = SLI_WLAN_MGMT_Q;

  *queue_packet = packet;
  return SL_STATUS_OK;
}

// Weak implementation of the function to process data frames received from the SI91x module
__WEAK sl_status_t sl_si91x_host_process_data_frame(sl_wifi_interface_t interface, sl_wifi_buffer_t *buffer)
{
  UNUSED_PARAMETER(interface);
  UNUSED_PARAMETER(buffer);
  return SL_STATUS_OK;
}

static void sli_handle_client_disconnection(sl_wifi_system_packet_t *packet)
{
  uint32_t payload_length = packet->length & 0x0FFF;

  // Extract the MAC address based on the specific disconnection command.
  if ((packet->command == SLI_WLAN_RSP_CLIENT_DISCONNECTED)
      && (payload_length == sizeof(sli_si91x_ap_disconnect_resp_t))) {

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
    sli_si91x_flush_third_party_station_dependent_sockets((sli_si91x_ap_disconnect_resp_t *)packet->data);
#endif
  }
}

static void sli_handle_tx_flush(const sli_si91x_queue_packet_t *data,
                                const sl_wifi_system_packet_t *packet,
                                sl_wifi_operation_mode_t current_operation_mode)
{
  // Handle cases where a general TX flush might be needed due to connection changes.
  // Check if the condition necessitates a general TX Wi-Fi queue flush.
  bool is_general_tx_queue_flush_needed =
    (packet->command == SLI_WIFI_RSP_JOIN || packet->command == SLI_WLAN_RSP_IPV4_CHANGE
     || packet->command == SLI_WLAN_RSP_IPCONFV4 || packet->command == SLI_WLAN_RSP_IPCONFV6
     || (packet->command == SLI_WIFI_RSP_DISCONNECT
         && (current_operation_mode == SL_SI91X_CLIENT_MODE || current_operation_mode == SL_SI91X_ENTERPRISE_CLIENT_MODE
             || current_operation_mode == SL_SI91X_TRANSCEIVER_MODE
             || current_operation_mode == SL_SI91X_TRANSMIT_TEST_MODE
             || (current_operation_mode == SL_SI91X_CONCURRENT_MODE && packet->desc[7] == SL_WIFI_CLIENT_VAP_ID))));

  // If a general flush is required, clear all TX Wi-Fi queues as the connection is lost.
  if (is_general_tx_queue_flush_needed) {
    sli_si91x_flush_all_tx_wifi_queues((uint16_t)SL_STATUS_WIFI_CONNECTION_LOST);
#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
    // Flush the select request table based on the provided frame_status
    sli_si91x_flush_select_request_table(data->frame_status);
#else
    (void)data; // Mark data as unused when SLI_SI91X_OFFLOAD_NETWORK_STACK is not defined.
#endif
  }

#ifdef SLI_SI91X_OFFLOAD_NETWORK_STACK
  // Define the VAP ID for the client (default to client VAP ID).
  uint8_t vap_id_for_flush = SL_WIFI_CLIENT_VAP_ID;

  // In concurrent mode with an AP stop command, use the AP VAP ID for the flush.
  if (current_operation_mode == SL_SI91X_CONCURRENT_MODE && packet->command == SLI_WIFI_RSP_AP_STOP) {
    vap_id_for_flush = SL_WIFI_AP_VAP_ID;
  }
  // Flush all pending socket commands for the determined VAP ID.
  sli_si91x_flush_all_socket_command_queues(data->frame_status, vap_id_for_flush);

  // Flush all pending socket data for the determined VAP ID.
  sli_si91x_flush_all_socket_data_queues(vap_id_for_flush);

  // Shutdown and update the state of the sockets associated with the VAP ID.
  sli_si91x_vap_shutdown(vap_id_for_flush, SLI_SI91X_BSD_DISCONNECT_REASON_INTERFACE_DOWN);
#endif
}

void sli_handle_wifi_events(const sli_si91x_queue_packet_t *data, sl_wifi_system_packet_t *packet)
{
  // Retrieve the current operation mode (e.g., client, AP, concurrent).
  sl_wifi_operation_mode_t current_operation_mode = sli_wifi_get_opermode();

  // Determine if a Wi-Fi client is disconnected from the access point.
  // This includes cases where the device is operating as an access point (AP mode)
  // or in concurrent mode where the AP VAP ID is relevant.
  bool is_client_disconnected_from_ap =
    (packet->command == SLI_WLAN_RSP_CLIENT_DISCONNECTED
     || (packet->command == SLI_WIFI_RSP_DISCONNECT
         && (current_operation_mode == SL_SI91X_ACCESS_POINT_MODE
             || (current_operation_mode == SL_SI91X_CONCURRENT_MODE && packet->desc[7] == SL_WIFI_AP_VAP_ID))));

  // Determine if a TX flush is required.
  // This is true for scenarios such as join failures, IP address changes, and disconnections.
  bool is_tx_flush_required = (((packet->command == SLI_WIFI_RSP_JOIN) && (data->frame_status != SL_STATUS_OK))
                               || packet->command == SLI_WLAN_RSP_IPV4_CHANGE
                               || packet->command == SLI_WLAN_RSP_IPCONFV4 || packet->command == SLI_WLAN_RSP_IPCONFV6
                               || packet->command == SLI_WIFI_RSP_DISCONNECT
                               || packet->command == SLI_WIFI_RSP_AP_STOP);

  // Handle the scenario where a Wi-Fi client disconnects from the AP.
  if (is_client_disconnected_from_ap) {
    sli_handle_client_disconnection(packet);
  } else if (is_tx_flush_required) {
    sli_handle_tx_flush(data, packet, current_operation_mode);
  }
}
