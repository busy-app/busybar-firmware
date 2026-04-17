/***************************************************************************/ /**
 * @file sli_command_engine.h
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_COMMAND_ENGINE_H
#define SLI_COMMAND_ENGINE_H

#include <stdint.h>
#include "cmsis_os2.h"
#include "sl_status.h"
#include "sl_slist.h"
#include "sli_queue_manager.h"
#include "sli_buffer_manager.h"
#include "sli_routing_utility.h"

/**`
  * @brief Defines the maximum length for the WLAN command engine thread name.
  *
  * This macro sets the maximum length for the name of the WLAN command engine thread.
  * The default value is set to 32 characters.
  *
  * @note
  * - Ensure that the thread name does not exceed this length to avoid buffer overflows.
  */
#ifndef SLI_COMMAND_ENGINE_THREAD_NAME_LENGTH
#define SLI_COMMAND_ENGINE_THREAD_NAME_LENGTH 32
#endif

/**
 * @brief Enumeration of packet flags used in the command engine.
 *
 * This enumeration defines the bitwise flags representing different types of packets
 * and their behaviors within the command engine. These flags are used to identify
 * the nature of the packet (command, data, response, etc.) and to control how the
 * command engine processes each packet.
 */
typedef enum {
  SLI_COMMAND_ENGINE_COMMAND_PACKET               = (1 << 0), ///< Command packet type
  SLI_COMMAND_ENGINE_DATA_PACKET                  = (1 << 1), ///< Data packet type
  SLI_COMMAND_ENGINE_ASYNC_RESPONSE_PACKET        = (1 << 2), ///< Asynchronous response packet type
  SLI_COMMAND_ENGINE_SYNC_RESPONSE_STATUS_PACKET  = (1 << 3), ///< Synchronous response status packet type
  SLI_COMMAND_ENGINE_SYNC_RESPONSE_DATA_PACKET    = (1 << 4), ///< Synchronous response data packet type
  SLI_COMMAND_ENGINE_REQUEST_WITH_GLOBAL_TX_BLOCK = (1 << 5), ///< Request with global TX block
} sli_command_engine_packet_flags_t;

/**
 * @brief Enumeration of packet processing types.
 *
 * Specifies the processing type for packets handled by the command engine.
 */
typedef enum {
  SLI_COMMAND_ENGINE_PACKET_PROCESSING_TYPE_COMMAND = 0, ///< Command packet processing
  SLI_COMMAND_ENGINE_PACKET_PROCESSING_TYPE_DATA         ///< Data packet processing
} sli_command_engine_packet_processing_type_t;

// Forward declaration of the command engine handle for use in callback signatures.
typedef struct sli_command_engine_s sli_command_engine_t;

/**
 * @brief Structure containing metadata for a packet to be transmitted.
 *
 * Holds information about the packet type, data buffer, length, frame ID,
 * flags, timeout, context, and packet ID. Used internally by the command engine
 * to track and process outgoing packets.
 */
typedef struct {
  uint16_t packet_type;                    ///< Packet type, used internally to track packets
  void *data_packet;                       ///< Data packet buffer
  uint16_t data_packet_length;             ///< Data packet buffer length
  uint16_t frame_id;                       ///< Implementation-specific frame ID
  sli_command_engine_packet_flags_t flags; ///< Flags indicating packet type and response behavior
  uint32_t timeout;                        ///< Timeout for the command
  void *context;                           ///< User-defined context for the command
  uint16_t packet_id;                      ///< Unique packet ID
} sli_command_engine_tx_info_t;

/**
 * @brief Structure containing metadata for a packet in the command engine.
 *
 * Includes queue node, instance pointer, packet status, transmission info,
 * and tick count for tracking packet processing.
 */
typedef struct {
  sli_command_engine_t *instance;       ///< Command engine instance
  uint16_t packet_status;               ///< Packet status
  sli_command_engine_tx_info_t tx_info; ///< Transmission metadata
  uint32_t packet_start_tickcount;      ///< Tick count when packet was submitted
  osThreadId_t sync_resp_thread_id;     ///< Thread ID for synchronous response
} sli_command_engine_metadata_t;

/**
 * @brief Callback to extract packet metadata for the command engine.
 *
 * @param instance Pointer to the command engine instance.
 * @param packet Pointer to the packet.
 * @param metadata Pointer to the metadata structure to populate.
 * @return Status of the operation.
 */
typedef sl_status_t (*sli_command_engine_get_packet_metadata_t)(sli_command_engine_t *instance,
                                                                void *packet,
                                                                sli_command_engine_metadata_t *metadata);

/**
 * @brief Callback to handle a packet in the command engine.
 *
 * @param instance Pointer to the command engine instance.
 * @param packet_type Type of the packet.
 * @param data Pointer to the packet data.
 * @return Status of the operation.
 */
typedef sl_status_t (*sli_command_engine_packet_handler_t)(sli_command_engine_t *instance,
                                                           uint16_t packet_type,
                                                           void *data);

/**
 * @brief Callback to handle command engine events.
 *
 * @param instance Pointer to the command engine instance.
 * @param event Event identifier.
 * @param data Pointer to event-specific data.
 */
typedef void (*sli_command_engine_event_handler_t)(sli_command_engine_t *instance, uint8_t event, void *data);

/**
 * @brief Enumeration of command engine error statuses.
 *
 * Defines possible error statuses that can occur in the command engine.
 */
typedef enum {
  SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR = 0,            ///< Fatal error, thread termination
  SLI_COMMAND_ENGINE_STATUS_INTERFACE_ERROR,            ///< Interface error
  SLI_COMMAND_ENGINE_STATUS_MEMORY_ERROR,               ///< Memory allocation error
  SLI_COMMAND_ENGINE_STATUS_COMMAND_TX_FAILED,          ///< Command transmission failed
  SLI_COMMAND_ENGINE_STATUS_COMMAND_TX_TIMEOUT,         ///< Command transmission timeout
  SLI_COMMAND_ENGINE_STATUS_COMMAND_PROCESSING_TIMEOUT, ///< Command processing timeout
  SLI_COMMAND_ENGINE_FATAL_ERROR_EVENT,                 ///< Fatal error event
} sl_command_engine_error_status_t;

/**
 * @brief Configuration for a specific packet type in the command engine.
 *
 * Contains handlers and settings for processing a particular packet type.
 */
typedef struct {
  sli_command_engine_packet_handler_t rx_event_handler;               ///< RX event handler
  sli_command_engine_packet_handler_t pre_tx_handler;                 ///< Pre-TX event handler
  sli_command_engine_packet_processing_type_t packet_processing_type; ///< Packet processing type
  uint16_t route_packet_type;                                         ///< Internal routing packet type
  sli_queue_t *sync_response_queue;                                   ///< Synchronous response queue handle
  uint32_t sync_response_event;                                       ///< Synchronous response event flag
  osEventFlagsId_t *sync_response_event_id;                           ///< Event ID for synchronous response
  uint8_t max_in_flight_command_count;                                ///< Maximum in-flight commands
  sli_queue_t *async_response_queue;                                  ///< Asynchronous response queue handle
  uint32_t async_response_event;                                      ///< Asynchronous response event flag
  osEventFlagsId_t *async_response_event_id;                          ///< Event ID for asynchronous response
} sli_command_engine_packet_type_configuration_t;

/**
 * @brief Command engine configuration structure.
 *
 * Contains settings and handler pointers for initializing a command engine instance.
 */
typedef struct {
  char name[SLI_COMMAND_ENGINE_THREAD_NAME_LENGTH]; ///< Command engine thread name
  uint8_t packet_type_count;                        ///< Number of packet types handled (max 10)
  sli_command_engine_packet_type_configuration_t *packet_type_configuration; ///< Packet type handler list
  sli_command_engine_get_packet_metadata_t get_packet_metadata;              ///< Metadata extraction handler
  osPriority_t priority;                                                     ///< Thread priority
  uint32_t stack_size;                                                       ///< Thread stack size
  sli_routing_table_t *routing_table;                                        ///< Routing table
  sli_queue_t *error_event_queue;                                            ///< Error event queue handle
  uint32_t error_event;                                                      ///< Error event flag
  osEventFlagsId_t *error_event_id;                                          ///< Event ID for error events
  sli_buffer_manager_pool_types_t metadata_buffer_pool_type;                 ///< Metadata buffer pool type
  sli_buffer_manager_pool_types_t error_buffer_pool_type;                    ///< Error buffer pool type
} sli_command_engine_configuration_t;

/**
 * @brief Structure containing queue information for the command engine.
 *
 * Tracks packet queues, in-flight command count, packet IDs, and statistics.
 */
typedef struct {
  sli_queue_t packet_queue;          ///< Packet queue handle
  sli_queue_t inflight_packet_queue; ///< In-flight packet queue handle
  bool sequential;                   ///< Whether commands are processed sequentially
  uint8_t in_flight_command_count;   ///< Number of in-flight commands
  uint16_t packet_id;                ///< Unique packet ID generator
  uint32_t rx_counter;               ///< Received packet counter
  uint32_t tx_counter;               ///< Transmitted packet counter
} sli_command_engine_queue_info_t;

typedef struct sli_command_engine_packet_type_configuration_node_s {
  struct sli_command_engine_packet_type_configuration_node_s *next;
  uint8_t packet_type;
  sli_command_engine_packet_type_configuration_t packet_config;
  sli_command_engine_queue_info_t queue_info;
} sli_command_engine_packet_type_configuration_node_t;

/**
 * @brief Command engine instance structure.
 *
 * Represents a running command engine, including configuration, thread and event IDs,
 * queue information, and error buffers.
 */
struct sli_command_engine_s {
  sli_command_engine_configuration_t config;                                ///< Command engine configuration
  osThreadId_t command_engine_threadId;                                     ///< Thread ID
  osEventFlagsId_t command_engine_eventId;                                  ///< Event flags ID
  sli_command_engine_queue_info_t *queue_info;                              ///< Queue information
  sli_queue_t rx_packet_queue;                                              ///< RX packet queue handle
  sli_queue_t tx_status_packet_queue;                                       ///< TX status packet queue handle
  sli_queue_t control_queue;                                                ///< Control packet queue handle
  sli_command_engine_packet_type_configuration_node_t *dynamic_packet_type; ///< Dynamic packet type configuration
  void *lock;                                                               ///< Instance lock for thread safety
  sl_command_engine_error_status_t *error_buffer;                           ///< Error status buffer
};

/**
 * @brief Initialize a command engine instance.
 *
 * @param[in] instance Pointer to the command engine instance.
 * @param[in] command_config Pointer to the configuration structure.
 * @return Status of the initialization operation.
 *         - SL_STATUS_OK: Initialization successful.
 *         - SL_STATUS_FAIL: Initialization failed.
 */
sl_status_t sli_command_engine_init(sli_command_engine_t *instance,
                                    const sli_command_engine_configuration_t *command_config);

/**
 * @brief Deinitialize a command engine instance.
 *
 * @param[in] instance Pointer to the command engine instance.
 * @return Status of the deinitialization operation.
 *         - SL_STATUS_OK: Deinitialization successful.
 *         - SL_STATUS_FAIL: Deinitialization failed.
 */
sl_status_t sli_command_engine_deinit(sli_command_engine_t *instance);

/**
 * @brief Adds an extended packet type configuration to the command engine instance.
 *
 * This function registers a new extended packet type with the specified configuration
 * to the given command engine instance. The packet type and its configuration are used
 * to handle custom packet processing within the command engine.
 *
 * @param[in] instance      Pointer to the command engine instance.
 * @param[in] packet_type   The identifier for the extended packet type to add.
 * @param[in] packet_config Pointer to the configuration structure for the packet type.
 *
 * @return
 *   - SL_STATUS_OK if the packet type was successfully added.
 *   - Appropriate error code otherwise.
 */
sl_status_t sli_command_engine_add_packet_type(sli_command_engine_t *instance,
                                               uint8_t packet_type,
                                               sli_command_engine_packet_type_configuration_t *packet_config);

/**
 * @brief Removes an extended packet type from the command engine instance.
 *
 * This function unregisters the specified extended packet type from the given
 * command engine instance, preventing further processing of packets of this type.
 *
 * @param[in] instance      Pointer to the command engine instance.
 * @param[in] packet_type   The extended packet type to remove.
 *
 * @return
 *   - SL_STATUS_OK if the packet type was successfully removed.
 *   - Appropriate error code otherwise.
 */
sl_status_t sli_command_engine_remove_packet_type(sli_command_engine_t *instance, uint8_t packet_type);

/**
 * @brief Query whether a command engine instance is currently idle.
 *
 * This helper inspects the internal state machine of the specified command
 * engine and reports its idleness state via the returned status code.
 *
 * Typical usage:
 *  - Call before queueing a new command to avoid contention.
 *  - Use to decide when it is safe to shut down, reset, or reconfigure
 *    the engine.
 *
 * @param instance Pointer to an initialized command engine instance.
 *                 Must not be NULL.
 *
 * @return sl_status_t
 *  - SL_STATUS_OK          The engine is idle (no command in progress).
 *  - SL_STATUS_BUSY        The engine is currently executing a command.
 *  - SL_STATUS_INVALID_PARAMETER  instance was NULL or invalid.
 *  - SL_STATUS_NOT_READY   The engine has not completed initialization.
 *  - Other implementation-specific error codes may be returned to
 *    indicate internal failures.
 */
sl_status_t sli_command_engine_is_idle(sli_command_engine_t *instance);

/**
 * @brief Send a packet to the command engine instance.
 *
 * @param[in] instance Pointer to the command engine instance.
 * @param[in] tx_info Pointer to the transmission info structure.
 *            The parameter packet_id is updated by this API to track 
 *            and identify the packet. The implementation can use this 
 *            packet_id to track the response packet from the command engine.
 * @return Status of the operation.
 *         - SL_STATUS_OK: Packet sent successfully.
 *         - SL_STATUS_FAIL: Failed to send packet.
 */
sl_status_t sli_command_engine_send_packet(sli_command_engine_t *instance, sli_command_engine_tx_info_t *tx_info);

/**
 * @brief Receive a packet for the specified command engine instance.
 *
 * Handles the reception of a packet for the given command engine instance.
 *
 * @param[in] instance Pointer to the command engine instance.
 * @param[in] data Pointer to the received packet data.
 * @return Status code indicating the result of the operation.
 *         - SL_STATUS_OK: Packet received successfully.
 *         - SL_STATUS_FAIL: Failed to receive the packet.
 */
sl_status_t sli_command_engine_receive_packet(sli_command_engine_t *instance, void *data);

/** 
 * @brief Notify the command engine of the transmission status of a packet.
 *
 * @param packet_type The type of the packet being reported.
 * @param status The status of the packet transmission.
 * @param context Pointer to user-defined context associated with the packet.
 */
void sli_command_engine_send_packet_tx_status(uint16_t packet_type, sl_status_t status, void *context);

#endif // SLI_COMMAND_ENGINE_H
