/***************************************************************************/ /**
 * @file sli_event_engine.c
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
#include "sli_event_engine.h"
#include "sli_queue_manager.h"
#include "sl_constants.h"
#include "cmsis_os2.h"
#include "sl_cmsis_utility.h"

/******************************************************
   *               Macro Definitions
   ******************************************************/
// Event flag for thread termination notification
#define SLI_EVENT_ENGINE_THREAD_TERMINATE_EVENT (1 << 23)

// Event flag for thread termination acknowledgment
#define SLI_EVENT_ENGINE_THREAD_TERMINATE_ACK_EVENT (1 << 22)

// Event flag for Event Handler Registration
#define SLI_EVENT_ENGINE_EVENT_HANDLER_REGISTRATION_EVENT (1 << 21)

// Event flag for Event Handler Registration Success notification
#define SLI_EVENT_ENGINE_REGISTER_EVENT_HANDLER_SUCCESS_EVENT (1 << 20)

// Event flag for Event Handler Registration Failure notification
#define SLI_EVENT_ENGINE_REGISTER_EVENT_HANDLER_FAILED_EVENT (1 << 19)

// Combined event flags that the event engine thread blocks on in its wait loop.
// Includes async processing, handler registration, and termination.
// Add new events here if the thread must react to them without polling.
#define SLI_EVENT_ENGINE_EVENTS_TO_WAIT_ON                                                   \
  (SLI_EVENT_ENGINE_ASYNC_EVENT                        /* Pending async events */            \
   | SLI_EVENT_ENGINE_EVENT_HANDLER_REGISTRATION_EVENT /* New handler registration queued */ \
   | SLI_EVENT_ENGINE_THREAD_TERMINATE_EVENT           /* Request to terminate thread */     \
  )

/******************************************************
   *               Local Type Definitions
  ******************************************************/
// Node in the singly linked list of registered event handlers.
// Each node represents one event type, its queue holding pending event data,
// and the callback to invoke when events for that type are processed.
typedef struct sli_event_engine_handler_node_s {
  struct sli_event_engine_handler_node_s *next; // Pointer to next handler node in list (NULL if last)
  sli_event_engine_handler_t handler;           // Callback executed for each dequeued event
  sli_queue_t *event_queue;                     // Queue containing pending event data items
  uint32_t event;                               // Event identifier associated with this handler
} sli_event_engine_handler_node_t;

/******************************************************
  *               Variable Definitions
  ******************************************************/
static osThreadId_t event_handler_thread_id         = NULL;  // Global instance of event handler thread ID
static osEventFlagsId_t event_engine_Id             = NULL;  // Global instance of event engine event ID
static sli_queue_t event_handler_registration_queue = { 0 }; // Global instance of event handler registration queue
static sli_event_engine_handler_node_t *event_handler_list = NULL; // Global instance of event handler list

/****************************************************** 
  *              Static Function Declarations
  ******************************************************/
// Wait for a specific event or timeout
static uint32_t sli_event_handler_wait_for_event(osEventFlagsId_t eventId, uint32_t event_mask, uint32_t timeout)
{
  if (timeout != osWaitForever) {
    timeout = SLI_SYSTEM_MS_TO_TICKS(timeout);
  }

  // Wait for any of the specified event flags to be set, with a timeout
  uint32_t result = osEventFlagsWait(eventId, event_mask, osFlagsWaitAny, timeout);

  // Return 0 if timeout or resource error occurred
  if (result == (uint32_t)osErrorTimeout || result == (uint32_t)osErrorResource) {
    return 0;
  }
  // Otherwise, return the received event flags
  return result;
}

// Set a specific event flag
static void sli_event_handler_set_event(osEventFlagsId_t eventId, uint32_t event_mask)
{
  // Set the specified event flag for the given event ID
  osEventFlagsSet(eventId, event_mask);
  return;
}

// Flush all the packets in event queues
static void sli_event_engine_queue_flush_handler(sli_queue_t *handle, void *data, void *context)
{
  UNUSED_PARAMETER(handle);
  UNUSED_PARAMETER(context);

  sli_buffer_manager_free_buffer(data);

  return;
}

// Event Handler thread function
static void sli_event_handler_thread(void *args)
{
  UNUSED_PARAMETER(args);
  uint32_t events_received = 0;
  sl_status_t status       = SL_STATUS_FAIL;
  void *data               = NULL;

  SL_DEBUG_LOG("Event Engine thread Started\n");

  while (1) {
    SL_DEBUG_LOG("Event Engine thread waiting for events\n");
    // Wait for any of the events
    events_received |=
      sli_event_handler_wait_for_event(event_engine_Id, SLI_EVENT_ENGINE_EVENTS_TO_WAIT_ON, osWaitForever);

    SL_DEBUG_LOG("Got events : 0x%lX in event engine thread\n", events_received);

    if (events_received & SLI_EVENT_ENGINE_THREAD_TERMINATE_EVENT) {
      // Clear the termination event flag
      events_received &= ~SLI_EVENT_ENGINE_THREAD_TERMINATE_EVENT;

      // Acknowledge the termination request
      sli_event_handler_set_event(event_engine_Id, SLI_EVENT_ENGINE_THREAD_TERMINATE_ACK_EVENT);

      // Terminate the current thread
      osThreadTerminate(osThreadGetId());
    }

    if (events_received & SLI_EVENT_ENGINE_EVENT_HANDLER_REGISTRATION_EVENT) {
      SL_DEBUG_LOG("Handling : SLI_EVENT_ENGINE_EVENT_HANDLER_REGISTRATION_EVENT.\n");
      // Clear the registration event flag
      events_received &= ~SLI_EVENT_ENGINE_EVENT_HANDLER_REGISTRATION_EVENT;

      // Add the received handler to the list
      // Process all pending event handler registrations in the queue
      while (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(&event_handler_registration_queue)) {
        sli_event_engine_handler_node_t *new_node = NULL;
        // Dequeue a new handler node from the registration queue
        status = sli_queue_manager_dequeue(&event_handler_registration_queue, (void **)&new_node);
        if (SL_STATUS_OK != status) {
          sli_event_handler_set_event(event_engine_Id, SLI_EVENT_ENGINE_REGISTER_EVENT_HANDLER_FAILED_EVENT);
          continue;
        }

        // Add the new handler node to the event handler list
        if (event_handler_list == NULL) {
          // If the list is empty, set the new node as the head
          event_handler_list = new_node;
        } else {
          // Otherwise, insert the new node at the beginning of the list
          new_node->next     = event_handler_list;
          event_handler_list = new_node;
        }

        sli_event_handler_set_event(event_engine_Id, SLI_EVENT_ENGINE_REGISTER_EVENT_HANDLER_SUCCESS_EVENT);
      }
    }

    if (events_received & SLI_EVENT_ENGINE_ASYNC_EVENT) {
      SL_DEBUG_LOG("Handling : SLI_EVENT_ENGINE_ASYNC_EVENT.\n");
      bool event_queues_empty               = true;
      sli_event_engine_handler_node_t *list = event_handler_list;
      // Clear the async event flag
      events_received &= ~SLI_EVENT_ENGINE_ASYNC_EVENT;

      while (NULL != list) {
        if (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(list->event_queue)) {
          // Dequeue the next event from the handler's event queue
          status = sli_queue_manager_dequeue(list->event_queue, &data);
          if (SL_STATUS_OK != status) {
            // ToDo: Treat the error as fatal and handle it appropriately
            continue;
          }

          // Handle the event
          list->handler(list->event, data);

          // Check if the event queue is not empty before processing
          if (!SLI_QUEUE_MANAGER_IS_QUEUE_EMPTY(list->event_queue)) {
            // Mark that at least one event queue is not empty
            event_queues_empty = false;
          }
        }

        if ((false == event_queues_empty) && (NULL == list->next)) {
          // We have iterated through all queues and there are more
          // events to be processed in one or more queues yet so loop
          // again from the beginning.
          list = event_handler_list;

          // Reset the event queues empty flag
          event_queues_empty = true;
        }
        list = list->next;
      }
    }
  }

  return;
}

/******************************************************
  *             Internal Function Definitions
  ******************************************************/
// Initialize the command engine
sl_status_t sli_event_engine_init(osEventFlagsId_t *event_engine_eventId)
{
  sl_status_t status = SL_STATUS_FAIL;

  // Create event flags for the command engine event handler
  event_engine_Id = osEventFlagsNew(NULL);
  if (NULL == event_engine_Id) {
    return SL_STATUS_NO_MORE_RESOURCE;
  }
  *event_engine_eventId = event_engine_Id;

  // Initialize each queue in the event handler queue
  status = sli_queue_manager_init(&event_handler_registration_queue, SLI_BUFFER_MANAGER_QUEUE_NODE_POOL);
  VERIFY_STATUS_AND_RETURN(status);

  // Thread attributes
  const osThreadAttr_t attr = {
    .name       = "event engine",
    .priority   = osPriorityRealtime1,
    .stack_mem  = 0,
    .stack_size = 1536,
    .cb_mem     = 0,
    .cb_size    = 0,
    .attr_bits  = 0u,
    .tz_module  = 0u,
  };

  // Create a new thread for the command engine event handler
  event_handler_thread_id = osThreadNew(&sli_event_handler_thread, NULL, &attr);
  if (NULL == event_handler_thread_id) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

// Deinitialize the command engine
sl_status_t sli_event_engine_deinit(void)
{
  uint32_t events_received = 0; // Holds received event flags (ack or timeout)

  if (NULL == event_handler_thread_id) {
    return SL_STATUS_INVALID_MODE; // Thread not running; cannot deinit
  }

  // Request the event engine thread to terminate
  sli_event_handler_set_event(event_engine_Id, SLI_EVENT_ENGINE_THREAD_TERMINATE_EVENT);

  // Wait (up to 5s) for the thread to acknowledge termination
  events_received =
    sli_event_handler_wait_for_event(event_engine_Id, SLI_EVENT_ENGINE_THREAD_TERMINATE_ACK_EVENT, 5000);
  if (0 == events_received) {
    return SL_STATUS_TIMEOUT; // Did not receive ack in time
  }
  event_handler_thread_id = NULL; // Mark thread as no longer valid

  // Free all registered event handler nodes and deinitialize their queues
  while (NULL != event_handler_list) {
    sli_event_engine_handler_node_t *temp = event_handler_list; // Take current head

    // Flush and deinit the event queue associated with this handler
    sli_queue_manager_deinit(temp->event_queue, sli_event_engine_queue_flush_handler);

    event_handler_list = event_handler_list->next; // Advance to next node
    free(temp);                                    // Release node memory
  }

  return SL_STATUS_OK; // Deinitialization successful
}

// Register an event handler for a specific event
sl_status_t sli_event_engine_register_event(sli_queue_t *event_queue,
                                            uint32_t event,
                                            sli_event_engine_handler_t handler)
{
  sl_status_t status                    = SL_STATUS_FAIL; // Status of queue operations
  sli_event_engine_handler_node_t *node = NULL;           // Newly allocated handler registration node
  uint32_t events_received              = 0;              // Event flags received while waiting for ack

  if (NULL == event_queue) { // Validate event queue pointer
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (NULL == handler) { // Validate handler callback
    return SL_STATUS_INVALID_PARAMETER;
  }

  node = (sli_event_engine_handler_node_t *)malloc(sizeof(sli_event_engine_handler_node_t)); // Allocate node
  if (NULL == node) { // Allocation failure handling
    return SL_STATUS_NO_MORE_RESOURCE;
  }

  node->event       = event;       // Store event identifier
  node->event_queue = event_queue; // Associate queue with this handler
  node->handler     = handler;     // Store handler callback
  node->next        = NULL;        // Initialize next pointer

  // Enqueue the handler node into the registration queue for the event engine thread to process
  status = sli_queue_manager_enqueue(&event_handler_registration_queue, (void *)node);
  if (SL_STATUS_OK != status) { // If enqueue fails release memory
    free(node);
    return status;
  }

  SL_DEBUG_LOG("Triggering : 0x%X event for event engine to register events\n",
               SLI_EVENT_ENGINE_EVENT_HANDLER_REGISTRATION_EVENT);
  // Signal event engine thread that a handler registration is pending
  sli_event_handler_set_event(event_engine_Id, SLI_EVENT_ENGINE_EVENT_HANDLER_REGISTRATION_EVENT);

  // Wait indefinitely for success or failure acknowledgment from the event engine thread
  events_received = sli_event_handler_wait_for_event(
    event_engine_Id,
    SLI_EVENT_ENGINE_REGISTER_EVENT_HANDLER_SUCCESS_EVENT | SLI_EVENT_ENGINE_REGISTER_EVENT_HANDLER_FAILED_EVENT,
    osWaitForever);

  // Check if success flag was set; if not, treat as failure
  if (SLI_EVENT_ENGINE_REGISTER_EVENT_HANDLER_SUCCESS_EVENT
      != (events_received & SLI_EVENT_ENGINE_REGISTER_EVENT_HANDLER_SUCCESS_EVENT)) {
    free(node); // Free node on failure (thread did not adopt it)
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK; // Registration succeeded
}