/****************************************************************************//*
 * @file
 * @brief Event System API for Inter-process Communication.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_event_system.h"
#include "sl_event_system_config.h"
#include "sl_component_catalog.h"
#include "sl_assert.h"
#include "sl_core.h"
#include "sl_memory_manager.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// IRQ event notification (first bit).
#define  SLI_EVENT_IRQ_NOTIFICATION  (1 << 0)

static bool is_event_system_initialized = false;
static sl_slist_node_t *publishers;

#if defined(SL_CATALOG_EVENT_SYSTEM_SUPERVISOR_MODE_PRESENT)
// Publisher for IRQ events.
static sl_event_publisher_t irq_event_publisher;
static sl_event_queue_t supervisor_queue;
#endif

static sl_status_t sli_event_publish(sl_event_publisher_t *publisher, uint32_t event_mask, uint8_t event_prio, sl_event_t* event, void *event_data);
static sl_event_publisher_t* sli_event_find_publisher(sl_event_class_t event_class);
static sl_event_subscriber_t* sli_event_find_subscriber(sl_event_publisher_t* publisher, sl_event_queue_t event_queue);
static sl_status_t sli_event_push_to_subscriber_queues(sl_event_publisher_t* publisher, sl_event_t *event, uint32_t event_mask);

/*******************************************************************************
 * @brief
 *  Initialize the event system.
 ******************************************************************************/
void sl_event_system_init(void)
{
  sl_slist_init(&publishers);
  is_event_system_initialized = true;
#if defined(SL_CATALOG_EVENT_SYSTEM_SUPERVISOR_MODE_PRESENT)
  // Create supervisor queue.
  sl_event_queue_create(SL_EVENT_SUPERVISOR_QUEUE_COUNT, &supervisor_queue);
#endif
}

/*******************************************************************************
 * @brief
 *  Initialize a publisher context and register it in the event system
 *  with a given event class.
 ******************************************************************************/
sl_status_t sl_event_publisher_register(sl_event_publisher_t *publisher,
                                        sl_event_class_t event_class,
                                        sl_event_free_data_cb_t free_data_callback)
{
  sl_event_publisher_t *publisher_entry;

  // Only register publishers if the event system has been initialized
  EFM_ASSERT(is_event_system_initialized == true);

  // Parameter validation.
  if (publisher == NULL || event_class >= SL_EVENT_CLASS_MAX
      || free_data_callback == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  // Only one publisher context can publish for a given event class.
  SL_SLIST_FOR_EACH_ENTRY(publishers, publisher_entry,
                          sl_event_publisher_t, node) {
    if (publisher_entry->event_class == event_class) {
      CORE_EXIT_ATOMIC();
      return SL_STATUS_ALREADY_EXISTS;
    }
  }

  // Register publisher context with the given event class and free data
  // callback.
  publisher->event_class = event_class;
  publisher->free_data_callback = free_data_callback;
  publisher->is_registered = true;
  sl_slist_push(&publishers, &publisher->node);

#if defined(SL_CATALOG_EVENT_SYSTEM_SUPERVISOR_MODE_PRESENT)
  sl_event_subscribe(event_class, 0xFFFFFFFFUL, supervisor_queue);
#endif

  CORE_EXIT_ATOMIC();
  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief
 *  Unregister a publisher context from its event class.
 ******************************************************************************/
sl_status_t sl_event_publisher_unregister(sl_event_publisher_t *publisher)
{
  // The publisher context cannot be NULL.
  if (publisher == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Only unregister publishers if the event system has been initialized
  EFM_ASSERT(is_event_system_initialized == true);

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  // Remove the publisher context from the publishers list. The publisher
  // context can be freed in user code with sl_event_publisher_free.
  publisher->is_registered = false;
  sl_slist_remove(&publishers, &publisher->node);
  CORE_EXIT_ATOMIC();

  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief
 *  Publish an event, with data, within the event class of the publisher.
 ******************************************************************************/
sl_status_t sl_event_publish(sl_event_publisher_t *publisher,
                             uint32_t event_mask,
                             uint8_t event_prio,
                             void *event_data)
{
  return sli_event_publish(publisher, event_mask, event_prio, NULL, event_data);
}

/*******************************************************************************
 * @brief
 *  Publish an event, with data, with a pre-allocated event handle, within the
 *  event class of the publisher.
 ******************************************************************************/
sl_status_t sl_event_publish_static(sl_event_publisher_t *publisher,
                                    uint32_t event_mask,
                                    uint8_t event_prio,
                                    sl_event_t* event,
                                    void *event_data)
{
  // The pre-allocated event handle cannot be NULL.
  if (event == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  return sli_event_publish(publisher, event_mask, event_prio, event, event_data);
}

/*******************************************************************************
 * @brief
 *  Subscribe to one or more events for a given event class. The subscribed
 *  event(s) is/are placed in the queue identified by event_queue.
 ******************************************************************************/
sl_status_t sl_event_subscribe(sl_event_class_t event_class,
                               uint32_t event_mask,
                               sl_event_queue_t event_queue)
{
  sl_status_t status = SL_STATUS_OK;
  sl_event_publisher_t *publisher;
  sl_event_subscriber_t *subscriber;

  // Only subscribe to events if the event system has been initialized.
  EFM_ASSERT(is_event_system_initialized == true);

  // Input validation
  if (event_class >= SL_EVENT_CLASS_MAX || event_queue == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  publisher = sli_event_find_publisher(event_class);
  if (publisher != NULL) {
    subscriber = sli_event_find_subscriber(publisher, event_queue);
    if (subscriber != NULL) {
      // Update subscriber's event_mask, adding any events in the provided
      // event_mask that are not already subscribed to.
      subscriber->event_mask = subscriber->event_mask | event_mask;
    } else {
      // Register a subscriber in the publisher context with the provided
      // event_queue, and subscribe to the events indicated by the
      // provided event_mask.
      status = sl_memory_alloc(sizeof(sl_event_subscriber_t),
                               BLOCK_TYPE_LONG_TERM, (void **)&subscriber);
      if (status != SL_STATUS_OK) {
        CORE_EXIT_ATOMIC();
        return status;
      }

      subscriber->event_mask = event_mask;
      subscriber->event_queue = event_queue;

      EFM_ASSERT(publisher->subscriber_count != UINT8_MAX);
      sl_slist_push(&publisher->subscribers, &subscriber->node);
      publisher->subscriber_count++;
    }
  } else {
    // There is no publisher registered with the provided event_class.
    status = SL_STATUS_NOT_FOUND;
  }

  CORE_EXIT_ATOMIC();
  return status;
}

/*******************************************************************************
 * @brief
 *  Unsubscribe from one or more events for a given event class. The
 *  unsubscribed event(s) will no longer be placed in the queue identified
 *  by event_queue.
 ******************************************************************************/
sl_status_t sl_event_unsubscribe(sl_event_class_t event_class,
                                 uint32_t event_mask,
                                 sl_event_queue_t event_queue)
{
  sl_status_t status = SL_STATUS_OK;
  sl_event_publisher_t *publisher;
  sl_event_subscriber_t *subscriber;

  // Only unsubscribe from events if the event system has been initialized
  EFM_ASSERT(is_event_system_initialized == true);

  // Input validation
  if (event_class >= SL_EVENT_CLASS_MAX || event_mask == 0
      || event_queue == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  publisher = sli_event_find_publisher(event_class);
  if (publisher != NULL) {
    subscriber = sli_event_find_subscriber(publisher, event_queue);
    if (subscriber != NULL) {
      // Update subscriber event_mask, removing any events currently
      // subscribed to that are indicated in the provided event_mask.
      subscriber->event_mask = subscriber->event_mask & ~event_mask;
      if (subscriber->event_mask == 0x0UL) {
        sl_slist_remove(&publisher->subscribers, &subscriber->node);
        publisher->subscriber_count--;
        sl_memory_free(subscriber);
      }
    }
  } else {
    // There is no publisher registered with the provided event_class.
    status = SL_STATUS_NOT_FOUND;
  }

  CORE_EXIT_ATOMIC();
  return status;
}

/*******************************************************************************
 * @brief
 *  Signals to the event system that a subscriber has processed an event. This
 *  must be called by subscribers after consuming an event so that the event
 *  data may eventually be freed. The event reference passed to this function
 *  is nullified before returning.
 ******************************************************************************/
sl_status_t sl_event_process(sl_event_t **event)
{
  // The event reference cannot be null
  if (*event == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Decrement reference count of event, and call the publisher's free
  // data callback if the number of references is 0.
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  (*event)->reference_count--;
  if ((*event)->reference_count == 0) {
    (*event)->free_data_callback((*event)->event_data);
    if (!(*event)->pre_allocated) {
      sl_memory_free((void *)(*event));
    }
  }
  CORE_EXIT_ATOMIC();

  // Nullify the event reference held by the callee.
  *event = NULL;

  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief
 *  Create an event queue.
 ******************************************************************************/
sl_status_t sl_event_queue_create(uint32_t event_count,
                                  sl_event_queue_t *event_queue)
{
  *event_queue = osMessageQueueNew(event_count, sizeof(sl_event_t *), NULL);
  if (*event_queue == NULL) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/*******************************************************************************
 * @brief
 *  Delete an event queue.
 ******************************************************************************/
sl_status_t sl_event_queue_delete(sl_event_queue_t event_queue)
{
  if (event_queue == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_event_publisher_t* publisher_entry;
  sl_event_subscriber_t* subscriber;
  sl_event_t* event;
  osStatus_t os_status;

  // Loop through all publishers subscribers list.
  SL_SLIST_FOR_EACH_ENTRY(publishers, publisher_entry,
                          sl_event_publisher_t, node) {
    // Loop through all subscribers queues.
    SL_SLIST_FOR_EACH_ENTRY(publisher_entry->subscribers, subscriber,
                            sl_event_subscriber_t, node) {
      // If subscriber's event queue is the queue we want to delete
      // remove the subscriber  from the subscribers list.
      if (event_queue == subscriber->event_queue) {
        sl_slist_remove(&publisher_entry->subscribers, &subscriber->node);
      }
    }
  }

  // Read event from the queue.
  os_status = osMessageQueueGet(event_queue, &event, NULL, 0);

  // Process each event in the queue, until the end of the queue has been reached.
  while (os_status != osErrorResource) {
    if (os_status == osOK) {
      sl_event_process(&event);
    } else if (os_status == osErrorParameter) {
      return SL_STATUS_INVALID_PARAMETER;
    } else {
      return SL_STATUS_FAIL;
    }
    // Read next event from the queue.
    os_status = osMessageQueueGet(event_queue, &event, NULL, 0);
  }

  // Delete the queue.
  os_status = osMessageQueueDelete(event_queue);
  if (os_status == osOK) {
    return SL_STATUS_OK;
  } else if (os_status == osErrorParameter) {
    return SL_STATUS_INVALID_PARAMETER;
  } else if (os_status == osErrorResource) {
    return SL_STATUS_EMPTY;
  } else {
    return SL_STATUS_FAIL;
  }
}

/*******************************************************************************
 * @brief
 *  Get an event from an event queue.
 ******************************************************************************/
sl_status_t sl_event_queue_get(sl_event_queue_t event_queue,
                               uint8_t *event_prio, uint32_t timeout,
                               sl_event_t **event)
{
  // Implementation doesn't currently use message priorities.
  (void)event_prio;
  sl_status_t status;
  osStatus_t os_status;

  // Wait to receive an event in the event queue for the specified timeout time.
  os_status = osMessageQueueGet(event_queue, event, NULL, timeout);
  switch (os_status) {
    case osOK:
      status = SL_STATUS_OK;
      break;
    case osErrorResource:
      status = SL_STATUS_EMPTY;
      break;
    case osErrorParameter:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    default:
      status = SL_STATUS_FAIL;
      break;
  }

  return status;
}

/*******************************************************************************
 * @brief
 *  Get the current number of events in queue.
 ******************************************************************************/
uint32_t sl_event_queue_get_count(sl_event_queue_t event_queue)
{
  return osMessageQueueGetCount(event_queue);
}

/*******************************************************************************
 * @brief
 *  Get the size of the event publisher structure.
 ******************************************************************************/
size_t sl_event_publisher_get_size(void)
{
  return sizeof(sl_event_publisher_t);
}

/*******************************************************************************
 * @brief
 *  Allocate the publisher structure to the heap using the common memory
 *  manager with a long-term lifespan.
 ******************************************************************************/
sl_status_t sl_event_publisher_alloc(sl_event_publisher_t **publisher)
{
  sl_status_t status;
  status = sl_memory_alloc(sizeof(sl_event_publisher_t), BLOCK_TYPE_LONG_TERM,
                           (void **)publisher);

  if ( status == SL_STATUS_OK ) {
    // Initialize publisher context's subscribers list
    (*publisher)->subscriber_count = 0;
    (*publisher)->is_registered = false;
    sl_slist_init(&(*publisher)->subscribers);
  }

  return status;
}

/*******************************************************************************
 * @brief
 *  Free the publisher context structure from the heap, as well as its list of
 *  subscriber entries.
 ******************************************************************************/
sl_status_t sl_event_publisher_free(sl_event_publisher_t *publisher)
{
  sl_event_subscriber_t *subscriber;

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  // Free the publisher's subscriber entries.
  SL_SLIST_FOR_EACH_ENTRY(publisher->subscribers, subscriber,
                          sl_event_subscriber_t, node) {
    sl_memory_free((void *)subscriber);
  }
  CORE_EXIT_ATOMIC();

  return sl_memory_free((void *)publisher);
}

/*******************************************************************************
 * @brief
 *  Get the size of the event structure.
 ******************************************************************************/
size_t sl_event_get_size(void)
{
  return sizeof(sl_event_t);
}

/*******************************************************************************
 * @brief
 *  Allocate the event structure to the heap using the common memory
 *  manager with a long-term lifespan.
 ******************************************************************************/
sl_status_t sl_event_alloc(sl_event_t **event)
{
  sl_status_t status;
  status = sl_memory_alloc(sizeof(sl_event_t), BLOCK_TYPE_LONG_TERM,
                           (void **)event);

  if ( status == SL_STATUS_OK ) {
    // Initialize event reference count.
    (*event)->reference_count = 0;
    // Set the pre-allocated flag.
    (*event)->pre_allocated = true;
  }

  return status;
}

/*******************************************************************************
 * @brief
 *  Free an event structure from the heap using the common memory manager.
 ******************************************************************************/
sl_status_t sl_event_free(sl_event_t *event)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  if ( event->reference_count > 0 ) {
    CORE_EXIT_ATOMIC();
    return SL_STATUS_NOT_READY;
  }
  CORE_EXIT_ATOMIC();

  return sl_memory_free((void *)event);
}

/*******************************************************************************
 *  Publish an event, with data, within the event class of the publisher.
 *  Handle with or without a pre-allocated event handle.
 ******************************************************************************/
static sl_status_t sli_event_publish(sl_event_publisher_t *publisher,
                                     uint32_t event_mask,
                                     uint8_t event_prio,
                                     sl_event_t* event,
                                     void *event_data)
{
  // Implementation doesn't currently use message priorities.
  (void)event_prio;
  sl_status_t status = SL_STATUS_OK;
  bool event_pre_allocated = (event == NULL) ? false : true;

  // Only publish events if the event system has been initialized
  EFM_ASSERT(is_event_system_initialized == true);

  // The event data cannot be NULL.
  if (publisher == NULL || event_mask == 0 || event_data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  if (event_pre_allocated && (event->reference_count > 0)) {
    // The event is still waiting to be consumed by all subscribers.
    CORE_EXIT_ATOMIC();
    return SL_STATUS_NOT_READY;
  }

  if (publisher->is_registered) {
    if (publisher->subscriber_count) {
      if (!event_pre_allocated) {
        // Event context is allocated.
        status = sl_memory_alloc(sizeof(sl_event_t), BLOCK_TYPE_SHORT_TERM,
                                 (void **)&event);
        if (status != SL_STATUS_OK) {
          CORE_EXIT_ATOMIC();
          return status;
        }
      }

      // Initialize event context
      event->event_class = publisher->event_class;
      event->free_data_callback = publisher->free_data_callback;
      event->event_data = event_data;
      event->reference_count = publisher->subscriber_count;
      event->pre_allocated = event_pre_allocated;

      status = sli_event_push_to_subscriber_queues(publisher, event, event_mask);
    } else {
      // No subscribers are listening, so the publish is skipped.
      publisher->free_data_callback(event_data);
    }
  } else {
    status = SL_STATUS_NOT_FOUND;
  }

  CORE_EXIT_ATOMIC();
  return status;
}

/*******************************************************************************
 *  Find a publisher registered to the given event_class. Returns NULL if none
 *  found.
 ******************************************************************************/
static sl_event_publisher_t* sli_event_find_publisher(sl_event_class_t event_class)
{
  sl_event_publisher_t* publisher;
  SL_SLIST_FOR_EACH_ENTRY(publishers, publisher, sl_event_publisher_t, node) {
    if (event_class == publisher->event_class) {
      return publisher;
    }
  }
  return NULL;
}

/*******************************************************************************
 *  Find a subscriber, subscribed to a given publisher, with the given
 *  event_queue. Returns NULL if none found.
 ******************************************************************************/
static sl_event_subscriber_t* sli_event_find_subscriber(sl_event_publisher_t* publisher, sl_event_queue_t event_queue)
{
  sl_event_subscriber_t* subscriber;
  SL_SLIST_FOR_EACH_ENTRY(publisher->subscribers, subscriber, sl_event_subscriber_t,
                          node) {
    if (event_queue == subscriber->event_queue) {
      return subscriber;
    }
  }
  return NULL;
}

sl_status_t sli_event_push_to_subscriber_queues(sl_event_publisher_t* publisher, sl_event_t *event, uint32_t event_mask)
{
  sl_event_subscriber_t* subscriber;
  sl_event_t* temp;
  sl_status_t status = SL_STATUS_OK;

  // Publish event to subscriber event queues.
  SL_SLIST_FOR_EACH_ENTRY(publisher->subscribers, subscriber,
                          sl_event_subscriber_t, node) {
    // Using a temp object since sl_event_process() nullfies the event
    // reference.
    temp = event;
    if ((event_mask & subscriber->event_mask) != 0) {
      // Implementation doesn't currently use message priorities.
      switch (osMessageQueuePut(subscriber->event_queue, &event, 0, 0)) {
        case osOK:
          // The event is sent to this subscriber.
          break;
        case osErrorResource:
          // The event_queue is full, the event will not be sent to this subscriber.
          // This is considered a failure to publish.
          status = SL_STATUS_FULL;
          sl_event_process(&temp);
          break;
        case osErrorParameter:
          // The event_queue is invalid. The publish fails but there is no
          // recovering from this.
          EFM_ASSERT(false);
          break;
        case osErrorTimeout:
          // A timeout should not occur. The publish fails but there is no
          // recovering from this.
          EFM_ASSERT(false);
          break;
        default:
          // An unknown error occurred. The publish fails but there is no
          // recovering from this.
          EFM_ASSERT(false);
          break;
      }
    } else {
      // The subscriber does not listen to this event, let's consider it
      // processed.
      sl_event_process(&temp);
    }
  }
  return status;
}

#if defined(SL_CATALOG_EVENT_SYSTEM_SUPERVISOR_MODE_PRESENT)
/*******************************************************************************
 * @brief
 *  Get an event from the supervisor event queue.
 ******************************************************************************/
sl_status_t sl_event_supervisor_queue_get(sl_event_t **event)
{
  return sl_event_queue_get(supervisor_queue, NULL, 0, event);
}

/*******************************************************************************
 * @brief
 *  Free data callback for IRQ events.
 *
 * @param[in] data  Pointer to the event data to free.
 ******************************************************************************/
static void irq_event_free_data_cb(void *data)
{
  (void)data; // Suppress unused parameter warning
}

/*******************************************************************************
 * @brief
 *  Initialize the IRQ event system. This must be called after sl_event_system_init().
 ******************************************************************************/
sl_status_t sl_event_irq_publisher_init(void)
{
  sl_status_t status;
  // Make sure the event system has been initialized
  EFM_ASSERT(is_event_system_initialized == true);

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();

  // Initialize the IRQ event publisher
  irq_event_publisher.subscriber_count = 0;
  irq_event_publisher.is_registered = false;
  sl_slist_init(&irq_event_publisher.subscribers);

  // Register the ISR event publisher
  status = sl_event_publisher_register(&irq_event_publisher,
                                       SL_EVENT_CLASS_IRQ,
                                       irq_event_free_data_cb);

  CORE_EXIT_ATOMIC();

  return status;
}

/*******************************************************************************
 * @brief
 *  De-initialize the IRQ event system.
 ******************************************************************************/
sl_status_t sl_event_irq_publisher_deinit(void)
{
  // Unregister the IRQ event publisher
  return sl_event_publisher_unregister(&irq_event_publisher);
}

/*******************************************************************************
 * @brief
 *  Publish an event from an interrupt service routine (ISR).
 *
 * @description
 *  This function should only be called from IRQ contexts to publish events to
 *  subscribers of the IRQ event class. It's a simplified wrapper around
 *  sl_event_publish designed for use in interrupt handlers.
 *
 * @param[in] irq_number    The IRQ number associated with the event
 *
 * @return
 *    SL_STATUS_OK if successful, otherwise an error code is returned.
 *
 * @note
 *    This function must only be called from IRQ contexts. For non-IRQ contexts,
 *    use the standard event publishing mechanisms.
 ******************************************************************************/
sl_status_t sl_event_irq_publish(uint32_t irq_number)
{
  // Always use priority 0 for IRQ events.
  return sl_event_publish(&irq_event_publisher, SLI_EVENT_IRQ_NOTIFICATION, 0, (void *)(uintptr_t)irq_number);
}

/*******************************************************************************
 * @brief
 *  Decode the IRQ event from a given event structure.
 ******************************************************************************/
uint32_t sl_event_irq_decode(sl_event_t *event)
{
  if (event == NULL || event->event_class != SL_EVENT_CLASS_IRQ) {
    return 0xFFFFFFFF;
  }

  // Cast pointer to get irq number.
  return (uint32_t)(uintptr_t)(event->event_data);
}
#endif // SL_CATALOG_EVENT_SYSTEM_SUPERVISOR_MODE_PRESENT

#ifdef __cplusplus
}
#endif
