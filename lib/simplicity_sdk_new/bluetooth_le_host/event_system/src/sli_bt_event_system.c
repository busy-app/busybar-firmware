/***************************************************************************//**
 * @file
 * @brief Bluetooth Event System IPC
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_bt_event_system.h"
#include "sl_bt_rtos_adaptation.h"
#include "sl_event_system.h"
#include "sl_assert.h"
#include "sl_component_catalog.h"
#include "sl_bt_api.h"

#if defined(SL_CATALOG_BTMESH_PRESENT)
#include "sl_btmesh_api.h"
#include "sl_btmesh_bgapi.h"
#endif

// -----------------------------------------------------------------------------
// Types and data private to this file

/**
 * @brief The event publisher used for Bluetooth host stack events
 */
static sl_event_publisher_t bt_event_publisher;

#if defined(SL_CATALOG_BTMESH_PRESENT)
/**
 * @brief The event publisher used for Bluetooth mesh stack events
 */
static sl_event_publisher_t btmesh_event_publisher;
#endif

/**
 * @brief The pre-allocated event that we publish and re-use
 */
static sl_event_t *preallocated_event = NULL;

/**
 * @brief Points to the published event data
 *
 * This is set when an event is published and cleared (set to NULL) when the
 * event has been freed. This enables reliably marking the event as handled
 * exactly once, and makes it possible to check that the RTOS adaptation is not
 * trying to publish more than one event at a time.
 */
static void *published_event_data = NULL;

// -----------------------------------------------------------------------------
// Functions private to this file

/**
 * @brief Called by Event System to free processed data
 *
 * @param[in] data The event data that has been processed
 */
static void event_free_data_callback(void *data)
{
  // Make sure we only free once
  if (published_event_data != NULL) {
    // The data must be what we published
    EFM_ASSERT(data == published_event_data);

    // Mark the event as handled
    published_event_data = NULL;
    sli_bt_rtos_set_event_handled();
  }
}

// -----------------------------------------------------------------------------
// Implementation of the API declared in `sli_bt_event_system.h`

/**************************************************************************//**
 * Perform functional init for Bluetooth support for Event System IPC.
 *****************************************************************************/
sl_status_t sli_bt_event_system_permanent_allocations(void)
{
  // Allocate the event we'll keep reusing
  return sl_event_alloc(&preallocated_event);
}

/**************************************************************************//**
 * Perform functional init for Bluetooth support for Event System IPC.
 *****************************************************************************/
sl_status_t sli_bt_event_system_functional_init(void)
{
  // No event has been published yet
  published_event_data = NULL;

  // Register our event publishers
  sl_status_t status = sl_event_publisher_register(&bt_event_publisher,
                                                   SL_EVENT_CLASS_BLUETOOTH,
                                                   event_free_data_callback);

#if defined(SL_CATALOG_BTMESH_PRESENT)
  if (status == SL_STATUS_OK) {
    status = sl_event_publisher_register(&btmesh_event_publisher,
                                         SL_EVENT_CLASS_BLUETOOTH_MESH,
                                         event_free_data_callback);
  }
#endif

  return status;
}

/**************************************************************************//**
 * Publish a BGAPI event to Event System IPC.
 *****************************************************************************/
sl_status_t sli_bt_publish_to_event_system(sl_bt_msg_t *event)
{
  // RTOS adaptation must not publish more than one event at a time
  EFM_ASSERT(published_event_data == NULL);
  published_event_data = event;

  // Choose the right publisher and event mask
  sl_event_publisher_t *event_publisher = NULL;
  uint32_t event_mask = 0;
  switch (SL_BGAPI_MSG_DEVICE_TYPE(event->header)) {
    case sl_bgapi_dev_type_bt:
      event_publisher = &bt_event_publisher;
      event_mask = SL_BT_EVENT_MASK_PUBLIC;
      break;

#if defined(SL_CATALOG_BTMESH_PRESENT)
    case sl_bgapi_dev_type_btmesh:
      event_publisher = &btmesh_event_publisher;
      event_mask = SL_BTMESH_EVENT_MASK_PUBLIC;
      break;
#endif

    default:
      // The device type was unknown. We leave `event_publisher` NULL so that
      // we'll skip publishing and free the event below.
      break;
  }

  // Publish to Event System
  sl_status_t status = SL_STATUS_INVALID_PARAMETER;
  if (event_publisher != NULL) {
    uint8_t event_prio = 0;
    status = sl_event_publish_static(event_publisher,
                                     event_mask,
                                     event_prio,
                                     preallocated_event,
                                     event);
  }

  // If anything failed, we cannot rely on getting the callback to free the
  // event. Call the callback directly from here.
  if (status != SL_STATUS_OK) {
    event_free_data_callback(event);
  }

  return status;
}
