/***************************************************************************/ /**
 * @file sli_command_engine.c
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

#include <stdint.h>
#include <string.h>
#include "cmsis_os2.h"
#include "sl_cmsis_utility.h"
#include "sl_constants.h"
#include "sl_core.h"
#include "sli_command_engine.h"
#include "sli_queue_manager.h"
#include "sli_routing_utility.h"

/******************************************************
  *               Macro Definitions
  ******************************************************/
// Event flag for thread termination notification
#define SLI_COMMAND_ENGINE_THREAD_TERMINATE_EVENT (1 << 23)

// Event flag for thread termination acknowledgment
#define SLI_COMMAND_ENGINE_THREAD_TERMINATE_ACK_EVENT (1 << 22)

// Event flag for packet TX acknowledgment
#define SLI_COMMAND_ENGINE_PACKET_TX_ACK_EVENT (1 << 21)

// Event flag for packet RX
#define SLI_COMMAND_ENGINE_PACKET_RX_EVENT (1 << 20)

// Event flag for packet configuration
#define SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT (1 << 19)

// Event flag for packet TX
#define SLI_COMMAND_ENGINE_PACKET_TX_EVENT (1 << 18)

// Event flag for dynamic packet TX
#define SLI_COMMAND_ENGINE_DYNAMIC_PACKET_TYPE_TX_EVENT (1 << 17)

// Event flags for command engine to wait on
#define SLI_COMMAND_ENGINE_EVENTS_TO_WAIT_ON                                                           \
  (SLI_COMMAND_ENGINE_DYNAMIC_PACKET_TYPE_TX_EVENT          /* dynamic packet type TX pending */       \
   | SLI_COMMAND_ENGINE_PACKET_TX_EVENT                     /* static packet type TX pending */        \
   | SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT /* packet type (un)registration request */ \
   | SLI_COMMAND_ENGINE_PACKET_RX_EVENT                     /* RX packet available */                  \
   | SLI_COMMAND_ENGINE_PACKET_TX_ACK_EVENT                 /* TX completion/ack available */          \
   | SLI_COMMAND_ENGINE_THREAD_TERMINATE_EVENT)             /* thread terminate request */

/******************************************************
  *               Local Type Definitions
  ******************************************************/
//------------------------------------------------------------------------------
// @brief  Packet type configuration request kinds for dynamic packet handling.
//         These values are pushed into the command engine control queue to
//         (un)register additional packet types at runtime.
//------------------------------------------------------------------------------
typedef enum {
  SLI_COMMAND_ENGINE_REGISTER_PACKET_TYPE,   // Request to add/register a new dynamic packet type
  SLI_COMMAND_ENGINE_UNREGISTER_PACKET_TYPE, // Request to remove/unregister an existing dynamic packet type
  SLI_COMMAND_ENGINE_PACKET_TYPE_REQUEST_MAX // Sentinel / upper bound (not used as a request)
} sli_command_engine_packet_configuration_request_type_t;

//------------------------------------------------------------------------------
// @brief  Request object placed on the command engine control queue to
//         (un)register a dynamic packet type at runtime.
//------------------------------------------------------------------------------
typedef struct {
  sli_command_engine_packet_configuration_request_type_t request_type; // Kind of request (register / unregister)
  uint8_t packet_type;                                                 // Packet type value to (un)register
  osThreadId_t thread_id;                                              // Requesting thread (to signal completion)
  sli_command_engine_packet_type_configuration_node_t
    *packet_type_config; // Pointer to new node when registering, NULL when unregistering
} sli_command_engine_packet_type_configuration_request_t;

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *              Static Function Declarations
 ******************************************************/
//------------------------------------------------------------------------------
// @brief Wait (with timeout) for any of the specified command engine events.
// @param command_engine_eventId  Event flags object to wait on.
// @param event_mask              Bitmask of events of interest (any one will satisfy the wait).
// @param timeout                 Timeout in RTOS ticks (osWaitForever for indefinite wait).
// @return Bitmask of events that became active, or 0 on timeout / error.
//------------------------------------------------------------------------------
static uint32_t sli_command_engine_wait_for_event(osEventFlagsId_t command_engine_eventId,
                                                  uint32_t event_mask,
                                                  uint32_t timeout)
{
  // Wait for ANY of the bits in event_mask to be set.
  // CMSIS-RTOS2 returns the complete current flags value on success,
  // or a (negative) error code cast to uint32_t on failure/timeout.
  if (timeout != osWaitForever) {
    timeout = SLI_SYSTEM_MS_TO_TICKS(timeout);
  }

  uint32_t result = osEventFlagsWait(command_engine_eventId, event_mask, osFlagsWaitAny, timeout);

  // Translate timeout or resource error to 0 (caller treats 0 as "no event").
  if ((result == (uint32_t)osErrorTimeout) || (result == (uint32_t)osErrorResource)) {
    return 0;
  }
  return result; // Return the flags that satisfied the wait.
}

//------------------------------------------------------------------------------
// @brief  Set (OR) the specified event flags on a command engine event group.
// @param  command_engine_eventId  Event flags object handle.
// @param  event_mask              Bit mask of flags to set.
//------------------------------------------------------------------------------
static void sli_command_engine_set_event(osEventFlagsId_t command_engine_eventId, uint32_t event_mask)
{
  // No return value needed; errors (if any) are ignored intentionally.
  (void)osEventFlagsSet(command_engine_eventId, event_mask);
}

//------------------------------------------------------------------------------
// @brief  Set (OR) the specified event flags for a specific thread.
// @param  thread_id   Target thread to receive the flags.
// @param  event_mask  Bit mask of thread flags to set.
//------------------------------------------------------------------------------
static void sli_command_engine_set_thread_event(osThreadId_t thread_id, uint32_t event_mask)
{
  // Post flags to the target thread; ignoring error for lightweight usage.
  (void)osThreadFlagsSet(thread_id, event_mask);
}

//------------------------------------------------------------------------------
// @brief Send an error event to the command engine's error handler.
//
// Tries to allocate a buffer for the error status. If allocation fails, it
// falls back to a pre-reserved emergency buffer (instance->error_buffer).
// The error status is then enqueued to the configured error queue and the
// associated error event flag is set to wake the error handler.
//------------------------------------------------------------------------------
static sl_status_t sli_command_engine_send_error_event(sli_command_engine_t *instance,
                                                       sl_command_engine_error_status_t error)
{
  sl_status_t status                             = SL_STATUS_OK;
  sl_command_engine_error_status_t *error_status = NULL;

  // Validate error queue and event id configuration.
  if (NULL == instance->config.error_event_queue) {
    return SL_STATUS_FAIL;
  }
  if (NULL == instance->config.error_event_id) {
    return SL_STATUS_FAIL;
  }

  // Try to allocate a buffer for the error status.
  status = sli_buffer_manager_allocate_buffer(instance->config.error_buffer_pool_type,
                                              SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                              1000,
                                              (sli_buffer_t *)&error_status);
  if (SL_STATUS_OK != status) {
    // Allocation failed: use the emergency buffer (if available) and mark the error.
    error_status           = instance->error_buffer;
    instance->error_buffer = NULL; // Consume the fallback buffer.
    *error_status          = SLI_COMMAND_ENGINE_STATUS_MEMORY_ERROR;
    status                 = SL_STATUS_ALLOCATION_FAILED;
  }

  // Store the actual error code (overwrites memory error only if allocation succeeded).
  *error_status = error;

  // Enqueue the error object so the error handler can process it.
  status = sli_queue_manager_enqueue(instance->config.error_event_queue, (void *)error_status);

  // Signal the error handler via its event flags.
  sli_command_engine_set_event(*(instance->config.error_event_id), instance->config.error_event);

  return status;
}

// Identify if an in-flight metadata node matches the received packet metadata
// Match criteria: packet_type and frame_id must both be equal
static bool rx_packet_identity_handler(sli_queue_t *handle, void *data, void *node_match_data)
{
  UNUSED_PARAMETER(handle); // Not used in this matcher
  // node_match_data carries the received packet temporary metadata
  sli_command_engine_metadata_t *rx_metadata = (sli_command_engine_metadata_t *)node_match_data;
  // data is the current in-flight metadata node being checked
  sli_command_engine_metadata_t *metadata = (sli_command_engine_metadata_t *)data;

  // Return true only if both packet_type and frame_id match
  if ((rx_metadata->tx_info.packet_type == metadata->tx_info.packet_type)
      && (rx_metadata->tx_info.frame_id == metadata->tx_info.frame_id)) {
    return true;
  }
  return false; // No match
}

// Retrieve the queue info and (optionally) the packet type configuration for a dynamic packet type.
// Returns SL_STATUS_OK if found, SL_STATUS_NOT_FOUND otherwise.
static sl_status_t sli_command_engine_get_dynamic_packet_info(
  sli_command_engine_t *instance,               // Command engine instance
  uint16_t packet_type,                         // Dynamic packet type to lookup
  sli_command_engine_queue_info_t **queue_info, // OUT: queue info for this packet type (if non-NULL)
  sli_command_engine_packet_type_configuration_t **packet_type_configuration) // OUT: packet config (if non-NULL)
{
  sli_command_engine_packet_type_configuration_node_t *node = NULL;

  // No dynamic packet types registered
  if (NULL == instance->dynamic_packet_type) {
    return SL_STATUS_NOT_FOUND;
  }

  // Traverse the singly-linked list of dynamic packet type nodes
  node = instance->dynamic_packet_type;
  while (NULL != node) {
    // Match on packet type
    if (node->packet_type == packet_type) {
      // Return queue info if caller requested it
      if (NULL != queue_info) {
        *queue_info = &(node->queue_info);
      }

      // Return packet configuration if caller requested it
      if (NULL != packet_type_configuration) {
        *packet_type_configuration = &(node->packet_config);
      }
      return SL_STATUS_OK; // Found
    }
    node = node->next; // Advance to next node
  }

  // Not found in dynamic list
  return SL_STATUS_NOT_FOUND;
}

//------------------------------------------------------------------------------
// @brief  Dequeue next packet metadata, drop any that have timed out,
//         invoke optional pre-TX hook, and route (transmit) the packet.
//         If transmission is asynchronous (returns IN_PROGRESS) we keep
//         metadata until TX completion callback moves it; otherwise on
//         immediate success we free the data buffer. For command packets
//         expecting synchronous response we move metadata to the in‑flight
//         queue so the RX path can correlate the response.
// @param  instance                  Command engine instance
// @param  queue_info                Per-packet-type queue state
// @param  packet_type_configuration Static/dynamic packet type configuration
// @param  packet_type               Packet type value
// @return SL_STATUS_OK (always unless internal helper reports fatal error)
//------------------------------------------------------------------------------
static sl_status_t sli_command_engine_handle_packet_tx(
  sli_command_engine_t *instance,
  sli_command_engine_queue_info_t *queue_info,
  sli_command_engine_packet_type_configuration_t *packet_type_configuration,
  uint16_t packet_type)
{
  sl_status_t status                      = SL_STATUS_OK;
  sli_command_engine_metadata_t *metadata = NULL;
  uint32_t time_elapsed                   = 0;

  do {
    // Pull next queued command
    status = sli_queue_manager_dequeue(&(queue_info->packet_queue), (void **)&metadata);
    if (SL_STATUS_OK != status) {
      // Dequeue failure: raise fatal error (queue/state corruption) and exit gracefully
      status = sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
      if (SL_STATUS_OK != status) {
        return status;
      }
      return SL_STATUS_OK;
    }

    // Compute time elapsed since packet queued to detect timeout
    time_elapsed = (osKernelGetTickCount() - metadata->packet_start_tickcount);
    if ((time_elapsed > metadata->tx_info.timeout) && (metadata->tx_info.timeout)) {
      // Drop timed out packet metadata and try next one (if any)
      sli_buffer_manager_free_buffer(metadata);
      metadata = NULL;
    }
    // Continue looping while:
    //  - We have not found a non-expired metadata (metadata == NULL)
    //  - And there are still entries in the queue to inspect
  } while ((NULL == metadata) && (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&(queue_info->packet_queue))));

  // All entries were timed out (or queue empty after drops)
  if (NULL == metadata) {
    return SL_STATUS_OK;
  }

  // Invoke optional pre-transmit hook (e.g., header fixups, logging)
  if (NULL != packet_type_configuration->pre_tx_handler) {
    status = packet_type_configuration->pre_tx_handler(instance, packet_type, (void *)metadata);
    if (SL_STATUS_OK != status) {
      // Pre-TX hook failure -> fatal error notification then graceful return
      status = sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
      if (SL_STATUS_OK != status) {
        return status;
      }
      return SL_STATUS_OK;
    }
  }

  // Route (send) the packet via routing utility (may be async)
  status = sli_routing_utility_route_packet(instance->config.routing_table,
                                            packet_type_configuration->route_packet_type,
                                            metadata->tx_info.data_packet,
                                            metadata->tx_info.data_packet_length,
                                            metadata);
  if (SL_STATUS_IN_PROGRESS == status) {
    // Async TX started: completion path will enqueue metadata for ACK processing
    return SL_STATUS_OK;
  } else if (SL_STATUS_OK != status) {
    // Immediate TX failure: report command TX failure and return
    status = sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_COMMAND_TX_FAILED);
    if (SL_STATUS_OK != status) {
      return status;
    }
    return SL_STATUS_OK;
  } else {
    // Immediate success (synchronous send). Data buffer no longer needed.
    sli_buffer_manager_free_buffer(metadata->tx_info.data_packet);
  }

  // For command packets expecting a synchronous response we retain metadata
  if ((SLI_COMMAND_ENGINE_COMMAND_PACKET == (SLI_COMMAND_ENGINE_COMMAND_PACKET & metadata->tx_info.flags))
      && ((SLI_COMMAND_ENGINE_SYNC_RESPONSE_STATUS_PACKET
           == (SLI_COMMAND_ENGINE_SYNC_RESPONSE_STATUS_PACKET & metadata->tx_info.flags))
          || (SLI_COMMAND_ENGINE_SYNC_RESPONSE_DATA_PACKET
              == (SLI_COMMAND_ENGINE_SYNC_RESPONSE_DATA_PACKET & metadata->tx_info.flags)))) {
    metadata->tx_info.data_packet        = NULL;
    metadata->tx_info.data_packet_length = 0;

    SL_DEBUG_LOG("Adding meta data : 0x%X\n", (unsigned int)metadata);

    // Move metadata to in-flight queue for response correlation
    status = sli_queue_manager_enqueue(&(queue_info->inflight_packet_queue), (void *)metadata);
    if (SL_STATUS_OK != status) {
      // Could not enqueue -> fatal error
      sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
      return status;
    }

    metadata = NULL;
    queue_info->in_flight_command_count++;
  } else {
    // No synchronous response expected: release metadata structure
    sli_buffer_manager_free_buffer(metadata);
    metadata = NULL;
  }

  return SL_STATUS_OK;
}

// Flush handler used when de‑initializing queues or removing nodes.
// This function is called for every node remaining in a queue so that
// any allocated packet (data) buffer is released to avoid memory leaks.
static void sli_command_engine_queue_flush_handler(sli_queue_t *handle, void *data, void *context)
{
  UNUSED_PARAMETER(handle);  // Queue handle not needed for simple free
  UNUSED_PARAMETER(context); // No extra context required

  // Release the buffer associated with this queue node
  sli_buffer_manager_free_buffer(data);

  return; // Explicit for clarity
}

// Command engine main thread loop.
// Responsibilities:
//  - Waits on command engine event flags
//  - Handles thread termination
//  - Processes dynamic packet type (un)registration requests
//  - Processes TX completion (ACK) events
//  - Processes RX packets (routes to sync / async handlers)
//  - Dequeues and transmits queued TX packets (static + dynamic types)
//  - Enforces per‑packet-type in‑flight command limits
static void sli_command_engine_thread(void *args)
{
  sli_command_engine_t *instance                                            = (sli_command_engine_t *)args;
  uint32_t events_received                                                  = 0; // Accumulated pending events
  sl_status_t status                                                        = SL_STATUS_FAIL;
  sli_command_engine_metadata_t rx_metadata                                 = { 0 }; // Temp RX metadata (stack)
  sli_command_engine_metadata_t *metadata                                   = NULL;  // Generic metadata pointer
  sli_command_engine_queue_info_t *queue_info                               = NULL;  // Per packet type queue state
  sli_command_engine_packet_type_configuration_t *packet_type_configuration = NULL;  // Static/dynamic type config
  uint16_t packet_type                                                      = 0; // Current packet type being processed
  void *data                                                                = NULL; // Generic packet pointer
  osThreadId_t resp_thread_id;                                                      // Waiting thread for sync response

  SL_DEBUG_LOG("%s task started \n", instance->config.name);

  while (1) {
    // Block until at least one awaited event occurs; OR in new events with any still pending
    events_received |= sli_command_engine_wait_for_event(instance->command_engine_eventId,
                                                         SLI_COMMAND_ENGINE_EVENTS_TO_WAIT_ON,
                                                         osWaitForever);

    // ---------------- Thread termination handling ----------------
    if (events_received & SLI_COMMAND_ENGINE_THREAD_TERMINATE_EVENT) {
      events_received &= ~SLI_COMMAND_ENGINE_THREAD_TERMINATE_EVENT; // Clear request
      // Acknowledge so deinit can continue
      sli_command_engine_set_event(instance->command_engine_eventId, SLI_COMMAND_ENGINE_THREAD_TERMINATE_ACK_EVENT);
      osThreadTerminate(osThreadGetId()); // Self terminate
    }

    // ---------------- Dynamic packet type configuration requests ----------------
    if (events_received & SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT) {
      SL_DEBUG_LOG("Handling : SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT.\n");
      events_received &= ~SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT;

      // Drain all control requests (register/unregister)
      while (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&instance->control_queue)) {
        sli_command_engine_packet_type_configuration_request_t *request = NULL;
        sli_command_engine_packet_type_configuration_node_t *new_node   = NULL;

        status = sli_queue_manager_dequeue(&instance->control_queue, (void **)&request);
        if (SL_STATUS_OK != status) {
          continue; // Skip on dequeue error
        }

        if (request->request_type == SLI_COMMAND_ENGINE_REGISTER_PACKET_TYPE) {
          new_node = request->packet_type_config; // Node prepared by requester (already alloc+inited)
          if (instance->dynamic_packet_type == NULL) {
            instance->dynamic_packet_type = new_node; // First dynamic packet type in list
          } else {
            new_node->next                = instance->dynamic_packet_type; // Insert new node at head for O(1) add
            instance->dynamic_packet_type = new_node;
          }
        } else { // Unregister path
          sli_command_engine_packet_type_configuration_node_t *prev = NULL;
          sli_command_engine_packet_type_configuration_node_t *node = instance->dynamic_packet_type;
          sli_command_engine_packet_type_configuration_node_t *ntbr = NULL; // Node to be removed

          while (node != NULL) {
            if (node->packet_type == request->packet_type) { // Match target packet type
              // Unlink located node
              ntbr = node;
              if (NULL != prev) {
                prev->next = node->next; // Bypass node in middle/end
              } else {
                instance->dynamic_packet_type = node->next; // Removing head updates list start
              }

              // Clean queues then free
              sli_queue_manager_deinit(&(ntbr->queue_info.packet_queue), sli_command_engine_queue_flush_handler);
              sli_queue_manager_deinit(&(ntbr->queue_info.inflight_packet_queue),
                                       sli_command_engine_queue_flush_handler);
              free(ntbr); // Release node memory
              break;      // Removal complete
            }
            prev = node;       // Track previous for unlink
            node = node->next; // Advance traversal
          }
        }

        // Notify requesting thread its request has been processed
        sli_command_engine_set_thread_event(request->thread_id, SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT);
        free(request);
      }
    }

    // ---------------- TX completion (ACK) handling ----------------
    if (events_received & SLI_COMMAND_ENGINE_PACKET_TX_ACK_EVENT) {
      while (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&instance->tx_status_packet_queue)) {
        status = sli_queue_manager_dequeue(&(instance->tx_status_packet_queue), (void **)(&metadata));
        if (SL_STATUS_OK != status) {
          // Queue corruption / unexpected failure
          sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
          continue;
        }

        // Data buffer is no longer needed post transmit
        sli_buffer_manager_free_buffer(metadata->tx_info.data_packet);
        metadata->tx_info.data_packet        = NULL;
        metadata->tx_info.data_packet_length = 0;

        // For command packets expecting sync response, move to in‑flight queue
        if ((SLI_COMMAND_ENGINE_COMMAND_PACKET
             == (SLI_COMMAND_ENGINE_COMMAND_PACKET & metadata->tx_info.flags)) // Is a command packet
            && ((SLI_COMMAND_ENGINE_SYNC_RESPONSE_STATUS_PACKET
                 == (SLI_COMMAND_ENGINE_SYNC_RESPONSE_STATUS_PACKET
                     & metadata->tx_info.flags)) // Needs sync status response
                || (SLI_COMMAND_ENGINE_SYNC_RESPONSE_DATA_PACKET
                    == (SLI_COMMAND_ENGINE_SYNC_RESPONSE_DATA_PACKET
                        & metadata->tx_info.flags)))) { // Or needs sync data response
          if (metadata->tx_info.packet_type < instance->config.packet_type_count) {
            queue_info = &(instance->queue_info[metadata->tx_info.packet_type]); // Static packet type queues
            packet_type_configuration =
              &(instance->config.packet_type_configuration[metadata->tx_info.packet_type]); // Static config
          } else {
            // Lookup dynamic packet info (packet_type variable contains metadata->tx_info.packet_type value)
            status = sli_command_engine_get_dynamic_packet_info(instance,
                                                                packet_type,
                                                                &queue_info,
                                                                &packet_type_configuration);
            if ((SL_STATUS_OK != status) || (NULL == queue_info) || (NULL == packet_type_configuration)) {
              sli_buffer_manager_free_buffer(metadata); // Cannot correlate -> drop
              continue;
            }
          }

          SL_DEBUG_LOG("Adding meta data : 0x%X\n", (unsigned int)metadata);
          status =
            sli_queue_manager_enqueue(&(queue_info->inflight_packet_queue), (void *)metadata); // Move to in-flight list
          if (SL_STATUS_OK != status) {
            sli_command_engine_send_error_event(instance,
                                                SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR); // Queue failure -> report
            continue;
          }
          metadata = NULL;                       // Ownership transferred
          queue_info->in_flight_command_count++; // Track outstanding sync command
        } else {
          // No sync response expected; free metadata (TX completed and not waiting for anything)
          sli_buffer_manager_free_buffer(metadata);
          metadata = NULL;
        }
      }
    }

    // ---------------- RX packet handling ----------------
    if (events_received & SLI_COMMAND_ENGINE_PACKET_RX_EVENT) {
      // Dequeue one RX packet from the RX queue
      status = sli_queue_manager_dequeue(&(instance->rx_packet_queue), (void **)&data);
      if (SL_STATUS_OK != status) {
        // If dequeue fails, send a fatal error event and break if error sending fails
        status = sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
        if (SL_STATUS_OK != status) {
          break; // Exit loop on fatal path
        }
        continue;
      }

      // If RX queue is now empty, clear the RX event bit
      if (SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&(instance->rx_packet_queue))) {
        events_received &= ~(SLI_COMMAND_ENGINE_PACKET_RX_EVENT);
      }

      // Fill temporary RX metadata using user callback
      rx_metadata.tx_info.data_packet = data;
      status                          = instance->config.get_packet_metadata(instance, data, &rx_metadata);
      if (SL_STATUS_OK != status) {
        // If metadata extraction fails, free the data buffer and continue
        sli_buffer_manager_free_buffer(data);
        continue;
      }
      packet_type = rx_metadata.tx_info.packet_type;

      // Resolve queue and configuration for static or dynamic packet type
      if (packet_type < instance->config.packet_type_count) {
        queue_info                = &(instance->queue_info[packet_type]);
        packet_type_configuration = &(instance->config.packet_type_configuration[packet_type]);
      } else {
        // Lookup dynamic packet info for dynamic packet types
        status =
          sli_command_engine_get_dynamic_packet_info(instance, packet_type, &queue_info, &packet_type_configuration);
        if ((SL_STATUS_OK != status) || (NULL == queue_info) || (NULL == packet_type_configuration)) {
          // If not found, free the data buffer and continue
          sli_buffer_manager_free_buffer(data);
          continue;
        }
      }

      // Decrement in-flight command counter if any commands are outstanding
      if (queue_info->in_flight_command_count > 0) {
        queue_info->in_flight_command_count--;
      }

      // Call RX event handler if configured (e.g., for parsing/classification)
      if (NULL != packet_type_configuration->rx_event_handler) {
        status = packet_type_configuration->rx_event_handler(instance, packet_type, (void *)data);
        if (SL_STATUS_OK != status) {
          // On handler failure, send fatal error and free buffer
          status = sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
          if (SL_STATUS_OK != status) {
            break;
          }
          sli_buffer_manager_free_buffer(data);
          continue;
        }
      }

      // Try to locate matching in-flight metadata for synchronous response
      status = sli_queue_manager_remove_node_from_queue(
        &(instance->queue_info[rx_metadata.tx_info.packet_type].inflight_packet_queue),
        rx_packet_identity_handler,
        (void *)&rx_metadata,
        (void **)&metadata);

      if ((SL_STATUS_NOT_FOUND == status) | (SL_STATUS_EMPTY == status)) {
        // If not found, treat as async response: enqueue to async queue and signal event
        SL_DEBUG_LOG("Sending data pointer : 0x%X to async event handler for packet type : %u\n",
                     (unsigned int)data,
                     packet_type);

        status = sli_queue_manager_enqueue(packet_type_configuration->async_response_queue, (void *)data);
        if (SL_STATUS_OK != status) {
          // On enqueue failure, send fatal error event
          SL_DEBUG_LOG("Async enqueue failed : %lu", status);
          status = sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
          if (SL_STATUS_OK != status) {
            break;
          }
          continue;
        }

        SL_DEBUG_LOG("Triggering event 0x%lX on event Id : 0x%X.\n",
                     packet_type_configuration->async_response_event,
                     (unsigned int)packet_type_configuration->async_response_event_id);

        // Notify async consumer via event flag
        sli_command_engine_set_event(*(packet_type_configuration->async_response_event_id),
                                     packet_type_configuration->async_response_event);
      } else if (SL_STATUS_OK != status) {
        // Unexpected queue error, send fatal error event
        status = sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
        if (SL_STATUS_OK != status) {
          break;
        }
        continue;
      } else {
        // Sync response: complete metadata and enqueue for waiting thread
        SL_DEBUG_LOG("Found meta data : 0x%X\n", (unsigned int)metadata);
        metadata->tx_info.data_packet        = data;
        metadata->tx_info.data_packet_length = rx_metadata.tx_info.data_packet_length;
        resp_thread_id                       = metadata->sync_resp_thread_id;

        SL_DEBUG_LOG("Adding %u command packet to queue 0x%X\n",
                     rx_metadata.tx_info.packet_type,
                     (unsigned int)packet_type_configuration->sync_response_queue);

        status = sli_queue_manager_enqueue(packet_type_configuration->sync_response_queue, (void *)metadata);
        if (SL_STATUS_OK != status) {
          // On enqueue failure, send fatal error event
          status = sli_command_engine_send_error_event(instance, SLI_COMMAND_ENGINE_STATUS_FATAL_ERROR);
          if (SL_STATUS_OK != status) {
            break;
          }
          continue;
        }
        SL_DEBUG_LOG("Sending Event: 0x%X on event id : 0x%X for queue 0x%X\n",
                     (unsigned int)packet_type_configuration->sync_response_event,
                     (unsigned int)*(packet_type_configuration->sync_response_event_id),
                     (unsigned int)packet_type_configuration->sync_response_queue);

        // Signal waiting thread (if original context still alive) else global event
        if (NULL != resp_thread_id) {
          sli_command_engine_set_thread_event(resp_thread_id, packet_type_configuration->sync_response_event);
        } else {
          sli_command_engine_set_event(*(packet_type_configuration->sync_response_event_id),
                                       packet_type_configuration->sync_response_event);
        }

        metadata = NULL;
      }

      // Reset temporary RX metadata container for next use
      memset((void *)&rx_metadata, 0, sizeof(sli_command_engine_metadata_t));
    }

    // ---------------- Static packet type TX scheduling ----------------
    if (events_received & SLI_COMMAND_ENGINE_PACKET_TX_EVENT) {
      bool tx_queues_empty = true;

      for (uint8_t i = 0; i < instance->config.packet_type_count; i++) {
        // Eligible to send if queue is not empty and below in-flight limit
        if (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&(instance->queue_info[i].packet_queue))
            && (instance->queue_info[i].in_flight_command_count
                < instance->config.packet_type_configuration[i].max_in_flight_command_count)) {

          status = sli_command_engine_handle_packet_tx(instance,
                                                       &(instance->queue_info[i]),
                                                       &(instance->config.packet_type_configuration[i]),
                                                       i);
          if (SL_STATUS_OK != status) {
            break;
          }
        }

        // Track if any queues still have pending items
        if (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&(instance->queue_info[i].packet_queue))) {
          tx_queues_empty = false;
        }
      }

      // Clear event if all queues drained
      if (true == tx_queues_empty) {
        events_received &= ~(SLI_COMMAND_ENGINE_PACKET_TX_EVENT);
      }
    }

    // ---------------- Dynamic packet type TX scheduling ----------------
    if (events_received & SLI_COMMAND_ENGINE_DYNAMIC_PACKET_TYPE_TX_EVENT) {
      bool dynamic_queues_empty                                 = true;
      sli_command_engine_packet_type_configuration_node_t *node = instance->dynamic_packet_type;

      while (NULL != node) {
        if (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&(node->queue_info.packet_queue))
            && (node->queue_info.in_flight_command_count < node->packet_config.max_in_flight_command_count)) {

          status = sli_command_engine_handle_packet_tx(instance,
                                                       &(node->queue_info),
                                                       &(node->packet_config),
                                                       node->packet_type);
          if (SL_STATUS_OK != status) {
            break;
          }
        }

        if (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&(node->queue_info.packet_queue))) {
          dynamic_queues_empty = false;
        }

        node = node->next;
      }

      if (true == dynamic_queues_empty) {
        events_received &= ~(SLI_COMMAND_ENGINE_DYNAMIC_PACKET_TYPE_TX_EVENT);
      }
    }
  }

  return;
}

/******************************************************
 *             Internal Function Definitions
 ******************************************************/
// Initialize the command engine
sl_status_t sli_command_engine_init(sli_command_engine_t *instance,
                                    const sli_command_engine_configuration_t *command_config)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Store the configuration in the instance
  instance->config = *command_config;

  // Allocate memory for the queue information based on the packet type count
  instance->queue_info = (sli_command_engine_queue_info_t *)malloc(sizeof(sli_command_engine_queue_info_t)
                                                                   * command_config->packet_type_count);

  // Check if memory allocation was successful
  if (NULL == (instance->queue_info)) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Initialize each queue in the queue information
  // Initialize each queue for every packet type
  for (uint16_t i = 0; i < instance->config.packet_type_count; i++) {
    memset(&(instance->queue_info[i]), 0, sizeof(sli_command_engine_queue_info_t)); // Clear queue info struct
    status = sli_queue_manager_init(&instance->queue_info[i].packet_queue,
                                    SLI_BUFFER_MANAGER_QUEUE_NODE_POOL); // Init main packet queue
    VERIFY_STATUS_AND_RETURN(status);
    status = sli_queue_manager_init(&instance->queue_info[i].inflight_packet_queue,
                                    SLI_BUFFER_MANAGER_QUEUE_NODE_POOL); // Init in-flight queue
    VERIFY_STATUS_AND_RETURN(status);
  }

  // Initialize RX packet queue
  status = sli_queue_manager_init(&(instance->rx_packet_queue), SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  VERIFY_STATUS_AND_RETURN(status);

  // Initialize TX status packet queue
  status = sli_queue_manager_init(&(instance->tx_status_packet_queue), SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  VERIFY_STATUS_AND_RETURN(status);

  // Initialize control queue for dynamic packet type requests
  status = sli_queue_manager_init(&(instance->control_queue), SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  VERIFY_STATUS_AND_RETURN(status);

  // Create event flags for the command engine
  instance->command_engine_eventId = osEventFlagsNew(NULL);

  // Thread attributes
  const osThreadAttr_t attr = {
    .name       = command_config->name,
    .priority   = command_config->priority,
    .stack_mem  = 0,
    .stack_size = command_config->stack_size,
    .cb_mem     = 0,
    .cb_size    = 0,
    .attr_bits  = 0u,
    .tz_module  = 0u,
  };

  // Create a new thread for the command engine
  instance->command_engine_threadId = osThreadNew(&sli_command_engine_thread, (void *)instance, &attr);
  if (NULL == instance->command_engine_threadId) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

// Deinitialize the command engine
sl_status_t sli_command_engine_deinit(sli_command_engine_t *instance)
{
  uint32_t events_received                                  = 0;
  sli_command_engine_packet_type_configuration_node_t *node = NULL;

  // Validate input instance pointer
  if (NULL == instance) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Ensure event ID object exists before using it
  if (NULL == instance->command_engine_eventId) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Ask the command engine thread to terminate itself (thread will set ACK then self-terminate)
  sli_command_engine_set_event(instance->command_engine_eventId, SLI_COMMAND_ENGINE_THREAD_TERMINATE_EVENT);

  // Wait (with timeout) for the thread to acknowledge termination
  events_received = sli_command_engine_wait_for_event(instance->command_engine_eventId,
                                                      SLI_COMMAND_ENGINE_THREAD_TERMINATE_ACK_EVENT,
                                                      5000);
  if (0 == events_received) {
    // Timed out waiting for acknowledgment -> report timeout
    return SL_STATUS_TIMEOUT;
  }

  // Clear stored thread id (thread has terminated)
  instance->command_engine_threadId = NULL;

  // Delete event flags and clear handle
  osEventFlagsDelete(instance->command_engine_eventId);
  instance->command_engine_eventId = NULL;

  // Deinitialize auxiliary queues
  sli_queue_manager_deinit(&(instance->control_queue), sli_command_engine_queue_flush_handler);
  sli_queue_manager_deinit(&(instance->tx_status_packet_queue), sli_command_engine_queue_flush_handler);
  sli_queue_manager_deinit(&(instance->rx_packet_queue), sli_command_engine_queue_flush_handler);

  // Deinitialize per static packet-type queues
  for (uint16_t i = 0; i < instance->config.packet_type_count; i++) {
    sli_queue_manager_deinit(&(instance->queue_info[i].packet_queue), sli_command_engine_queue_flush_handler);
    sli_queue_manager_deinit(&(instance->queue_info[i].inflight_packet_queue), sli_command_engine_queue_flush_handler);
  }

  // Free static packet type queue info array
  free(instance->queue_info);
  instance->queue_info = NULL;

  // Walk and free all dynamically registered packet types
  while (NULL != instance->dynamic_packet_type) {
    node                          = instance->dynamic_packet_type;       // Take head
    instance->dynamic_packet_type = instance->dynamic_packet_type->next; // Advance list head
    sli_queue_manager_deinit(&(node->queue_info.packet_queue), sli_command_engine_queue_flush_handler);
    sli_queue_manager_deinit(&(node->queue_info.inflight_packet_queue), sli_command_engine_queue_flush_handler);
    free(node); // Free node memory
  }

  return SL_STATUS_OK;
}

// Register (add) a new dynamic packet type at runtime.
// Allocates a node + request object, initializes its queues, enqueues a control
// request to the command engine thread, and waits for completion acknowledgment.
sl_status_t sli_command_engine_add_packet_type(sli_command_engine_t *instance,
                                               uint8_t packet_type,
                                               sli_command_engine_packet_type_configuration_t *packet_config)
{
  sl_status_t status                                              = SL_STATUS_OK;
  uint32_t events_received                                        = 0;
  sli_command_engine_packet_type_configuration_node_t *new_node   = NULL;
  sli_command_engine_packet_type_configuration_request_t *request = NULL;
  osThreadId_t thread_id = osThreadGetId(); // Thread to be notified on completion

  // Validate inputs
  if (NULL == instance) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL == packet_config) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL == thread_id) { // Should not normally be NULL
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Reject if packet_type collides with static range
  if (packet_type < instance->config.packet_type_count) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Allocate node for the dynamic packet type
  new_node = (sli_command_engine_packet_type_configuration_node_t *)malloc(
    sizeof(sli_command_engine_packet_type_configuration_node_t));
  if (NULL == new_node) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Allocate control request object
  request = (sli_command_engine_packet_type_configuration_request_t *)malloc(
    sizeof(sli_command_engine_packet_type_configuration_request_t));
  if (NULL == request) {
    free(new_node);
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Copy user configuration into node
  new_node->packet_config = *packet_config;
  new_node->next          = NULL;

  // Initialize TX queue for this packet type
  status = sli_queue_manager_init(&new_node->queue_info.packet_queue, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  if (SL_STATUS_OK != status) {
    free(new_node);
    free(request);
    VERIFY_STATUS_AND_RETURN(status);
  }

  // Initialize in-flight (awaiting response) queue
  status = sli_queue_manager_init(&new_node->queue_info.inflight_packet_queue, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  if (SL_STATUS_OK != status) {
    sli_queue_manager_deinit(&new_node->queue_info.packet_queue, sli_command_engine_queue_flush_handler);
    VERIFY_STATUS_AND_RETURN(status);
    free(new_node);
    free(request);
    return SL_STATUS_FAIL;
  }

  // Populate request
  request->request_type       = SLI_COMMAND_ENGINE_REGISTER_PACKET_TYPE;
  request->packet_type        = packet_type;
  request->packet_type_config = new_node;
  request->thread_id          = thread_id;

  // Enqueue request to control queue
  status = sli_queue_manager_enqueue(&(instance->control_queue), (void *)request);
  VERIFY_STATUS_AND_RETURN(status);

  // Notify command engine thread
  sli_command_engine_set_event(instance->command_engine_eventId,
                               SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT);

  // Wait for acknowledgment (same flag echoed back to requesting thread)
  events_received = osThreadFlagsWait(SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT, osFlagsWaitAny, 10000);
  if (!(events_received & SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT)) {
    // Timed out / failed: cleanup (thread never took ownership)
    sli_queue_manager_deinit(&new_node->queue_info.packet_queue, sli_command_engine_queue_flush_handler);
    sli_queue_manager_deinit(&new_node->queue_info.inflight_packet_queue, sli_command_engine_queue_flush_handler);
    free(new_node);
    free(request);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

// Unregister (remove) a previously added dynamic packet type.
// Allocates a control request, enqueues it, and waits for acknowledgment.
// The command engine thread will free the node and its queues.
sl_status_t sli_command_engine_remove_packet_type(sli_command_engine_t *instance, uint8_t packet_type)
{
  sl_status_t status                                              = SL_STATUS_OK;
  uint32_t events_received                                        = 0;
  sli_command_engine_packet_type_configuration_request_t *request = NULL;
  osThreadId_t thread_id                                          = osThreadGetId(); // Requesting thread id

  // Validate inputs
  if (NULL == instance) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  if (NULL == thread_id) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Reject if within static packet type range (cannot remove static type)
  if (packet_type < instance->config.packet_type_count) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Allocate control request
  request = (sli_command_engine_packet_type_configuration_request_t *)malloc(
    sizeof(sli_command_engine_packet_type_configuration_request_t));
  if (NULL == request) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  // Populate request (no node pointer for remove)
  request->request_type       = SLI_COMMAND_ENGINE_UNREGISTER_PACKET_TYPE;
  request->packet_type        = packet_type;
  request->packet_type_config = NULL;
  request->thread_id          = thread_id;

  // Enqueue removal request
  status = sli_queue_manager_enqueue(&(instance->control_queue), (void *)request);
  VERIFY_STATUS_AND_RETURN(status);

  // Signal command engine to process request
  sli_command_engine_set_event(instance->command_engine_eventId,
                               SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT);

  // Wait for acknowledgment
  events_received = osThreadFlagsWait(SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT, osFlagsWaitAny, 10000);
  if (!(events_received & SLI_COMMAND_ENGINE_CONFIGURE_PACKET_TYPE_REQUEST_EVENT)) {
    // Thread did not acknowledge in time
    free(request);
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t sli_command_engine_is_idle(sli_command_engine_t *instance)
{
  // Validate inputs
  if (NULL == instance) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (0 != osEventFlagsGet(instance->command_engine_eventId)) {
    return SL_STATUS_BUSY;
  }

  for (uint16_t i = 0; i < instance->config.packet_type_count; i++) {
    // Check if any static packet type queues are non-empty
    if (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&(instance->queue_info[i].packet_queue))) {
      return SL_STATUS_BUSY;
    }
  }

  while (NULL != instance->dynamic_packet_type) {
    // Check if any dynamic packet type queues are non-empty
    if (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&(instance->dynamic_packet_type->queue_info.packet_queue))) {
      return SL_STATUS_BUSY;
    }
    instance->dynamic_packet_type = instance->dynamic_packet_type->next;
  }

  return SL_STATUS_OK;
}

// Send (enqueue) a packet for transmission through the command engine.
// Allocates a metadata container, copies TX info, selects the correct
// queue (static or dynamic), enqueues it, and signals the worker thread.
sl_status_t sli_command_engine_send_packet(sli_command_engine_t *instance, sli_command_engine_tx_info_t *tx_info)
{
  sl_status_t status                          = SL_STATUS_OK;
  sli_command_engine_metadata_t *metadata     = NULL; // Holds per-packet state until completion
  sli_command_engine_queue_info_t *queue_info = NULL; // Queue set chosen based on packet type (static/dynamic)
  uint32_t event_mask                         = 0;    // Event flag to wake TX scheduler

  // Validate inputs
  if ((NULL == instance) || (NULL == tx_info)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Reject packet types that collide with static space (dynamic types handled later in lookup)
  if (tx_info->packet_type >= instance->config.packet_type_count) {
    // Caller attempted to send with an unknown / unregistered packet type
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Allocate metadata buffer (hybrid allocation allows pool + heap fallback)
  status = sli_buffer_manager_allocate_buffer(instance->config.metadata_buffer_pool_type,
                                              SLI_BUFFER_MANAGER_ALLOCATION_TYPE_HYBRID,
                                              1000,
                                              (sli_buffer_t *)&metadata);
  VERIFY_STATUS_AND_RETURN(status);

  // Atomically allocate a monotonically increasing packet_id per packet type
  CORE_irqState_t state = CORE_EnterAtomic();
  tx_info->packet_id    = (instance->queue_info[tx_info->packet_type].packet_id)++;
  CORE_ExitAtomic(state);

  // Populate metadata structure
  metadata->instance               = instance;               // Back reference to engine instance
  metadata->tx_info.packet_type    = tx_info->packet_type;   // Store packet type (redundant but explicit)
  metadata->tx_info.packet_id      = tx_info->packet_id;     // Store assigned packet id
  metadata->tx_info                = *tx_info;               // Copy full TX descriptor
  metadata->packet_start_tickcount = osKernelGetTickCount(); // Timestamp for timeout handling
  metadata->sync_resp_thread_id    = osThreadGetId();        // Thread to signal for sync responses

  // Resolve queue info and appropriate wake event (static vs dynamic)
  if (metadata->tx_info.packet_type < instance->config.packet_type_count) {
    queue_info = &(instance->queue_info[metadata->tx_info.packet_type]); // Static packet type queue
    event_mask = SLI_COMMAND_ENGINE_PACKET_TX_EVENT;                     // Static TX scheduler event
  } else {
    // Dynamic packet type: lookup its queue info
    status = sli_command_engine_get_dynamic_packet_info(instance, tx_info->packet_type, &queue_info, NULL);
    if ((SL_STATUS_OK != status) || (NULL == queue_info)) {
      // Unknown / unregistered dynamic type
      sli_buffer_manager_free_buffer(metadata);
      return SL_STATUS_INVALID_CONFIGURATION;
    }
    event_mask = SLI_COMMAND_ENGINE_DYNAMIC_PACKET_TYPE_TX_EVENT; // Dynamic TX scheduler event
  }

  // Enqueue metadata onto the packet queue (ownership transfers to queue on success)
  status = sli_queue_manager_enqueue(&(queue_info->packet_queue), (sl_slist_node_t *)metadata);
  VERIFY_STATUS_AND_RETURN(status);

  // Wake command engine thread to process TX scheduling
  sli_command_engine_set_event(instance->command_engine_eventId, event_mask);

  return SL_STATUS_OK;
}

// Send a packet through the command engine
sl_status_t sli_command_engine_receive_packet(sli_command_engine_t *instance, void *data)
{
  sl_status_t status = SL_STATUS_OK;

  // Check if the instance or data is NULL
  if ((NULL == instance) || (NULL == data)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Enqueue the data packet into the appropriate queue
  status = sli_queue_manager_enqueue(&(instance->rx_packet_queue), data);
  VERIFY_STATUS_AND_RETURN(status);

  // Signal the thread to handle the packet
  sli_command_engine_set_event(instance->command_engine_eventId, SLI_COMMAND_ENGINE_PACKET_RX_EVENT);

  // Return success status
  return SL_STATUS_OK;
}

//------------------------------------------------------------------------------
// @brief  Callback invoked after a packet transmit attempt completes.
//         Stores the transmit status in the metadata, enqueues the metadata
//         onto the TX status queue, and signals the command engine thread
//         so it can continue processing (e.g. move to in‑flight or free).
// @param  packet_type  Type of the transmitted packet (unused here)
// @param  status       Result of the transmit operation
// @param  context      Pointer to sli_command_engine_metadata_t provided at TX
//------------------------------------------------------------------------------
void sli_command_engine_send_packet_tx_status(uint16_t packet_type, sl_status_t status, void *context)
{
  UNUSED_PARAMETER(packet_type); // Packet type not needed in this callback

  // Recover metadata and owning command engine instance from context
  sli_command_engine_metadata_t *metadata = (sli_command_engine_metadata_t *)context;
  sli_command_engine_t *instance          = metadata->instance;

  // Record the TX completion status in the metadata
  metadata->packet_status = status;

  // Enqueue metadata for the command engine thread to post-process
  status = sli_queue_manager_enqueue(&(instance->tx_status_packet_queue), (void *)metadata);
  if (SL_STATUS_OK != status) {
    // If enqueue fails we silently drop; fatal error path handled elsewhere
    return;
  }

  // Wake the command engine thread to handle TX ACK processing
  sli_command_engine_set_event(instance->command_engine_eventId, SLI_COMMAND_ENGINE_PACKET_TX_ACK_EVENT);
  return;
}
