/***************************************************************************//**
 * @file app.c
 * @brief main app.c for ZigbeeMinimalRTOS
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

#include PLATFORM_HEADER
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif

#include "sl_zigbee.h"

#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"

#if defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)
#include "sl_iostream.h"
#endif

#include "sl_zigbee_system_common.h"
#include "sl_zigbee_rtos_task_config.h"
#include "zigbee_ipc_command_messages.h"
#include "sl_event_system.h"
#include "sl_memory_manager.h"
#include "zigbee_rtos_adaptation.h"
#include "zigbee_sleep_config.h"
#include "hal.h"
#include "sl_zigbee.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_system_common.h"
#include "app.h"
#include "stack/core/sl_zigbee_multi_network.h"

#include "sl_event_system.h"

#ifdef SL_CATALOG_MEMORY_MANAGER_PRESENT
#include "sl_memory_manager.h"
#else
#include "sl_malloc.h"
#endif // SL_CATALOG_MEMORY_MANAGER_PRESENT

#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"

//------------------------------------------------------------------------------
// required: test endpoint creation and assignment

#define TEST_PROFILE_ID 0x7F01  // Per 064166r00
// Need this variable so applications can have access to it.
const uint16_t testProfileId = TEST_PROFILE_ID;

const uint16_t testDriverInClusters[] = { TEST_DRIVER_IN_CLUSTERS };
const uint16_t testDriverOutClusters[] = { TEST_DRIVER_OUT_CLUSTERS };

sl_zigbee_endpoint_description_t const testDriverDescription = {
  TEST_PROFILE_ID,  // Test Profile #2 profile ID.
  0x0000,  // Test Driver device ID.
  0,       // No complex or user descriptor.
  sizeof(testDriverInClusters) >> 1,
  sizeof(testDriverOutClusters) >> 1
};

const uint16_t fullDeviceInClusters[] = { FULL_DEVICE_IN_CLUSTERS };
const uint16_t fullDeviceOutClusters[] = { FULL_DEVICE_OUT_CLUSTERS };

sl_zigbee_endpoint_description_t const fullDeviceDescription = {
  TEST_PROFILE_ID,  // Test Profile #2 profile ID.
  0xFFFF,  // Full Device Under Test device ID.
  0,       // No complex or user descriptor.
  sizeof(fullDeviceInClusters) >> 1,
  sizeof(fullDeviceOutClusters) >> 1
};

uint8_t sl_zigbee_endpoint_count = 3;

sl_zigbee_endpoint_t sl_zigbee_endpoints[] = {
  ENDPOINT_TEST_DRIVER_DEFINITION,
  ENDPOINT_FULL_DEVICE_1_DEFINITION,
  ENDPOINT_FULL_DEVICE_2_DEFINITION
};

//------------------------------------------------------------------------------
// required global
uint8_t joinDecision = SL_ZIGBEE_SEND_KEY_IN_THE_CLEAR;

//------------------------------------------------------------------------------
// Forward declarations

//task size is specified in bytes and is word aligned
static osThreadId_t custom_app_task_id;
__ALIGNED(8) static uint8_t custom_app_task_stack[SL_ZIGBEE_APP_FRAMEWORK_RTOS_TASK_STACK_SIZE];
__ALIGNED(4) static uint8_t custom_app_task_cb[osThreadCbSize];
static osThreadAttr_t custom_app_task_attr;

static void custom_app_task(void *p_arg);

sl_event_queue_t callback_event_queue;

void custom_app_subscribe_to_callback_event_queue(void)
{
  // Subscribe to publisher and set up queue for receiving events in the application framework task
  sl_event_queue_create(100, &callback_event_queue);
  sl_event_subscribe(SL_EVENT_CLASS_ZIGBEE, //event_class
                     0xFFFFFFFF,            //event_mask
                     callback_event_queue);
}

void sli_zigbee_app_rtos_task_init_cb(void)
{
  // Create Custom app task.
  custom_app_task_attr.name = "Custom App task";
  custom_app_task_attr.stack_mem = &custom_app_task_stack[0];
  custom_app_task_attr.stack_size = sizeof(custom_app_task_stack);
  custom_app_task_attr.cb_mem = custom_app_task_cb;
  custom_app_task_attr.cb_size = osThreadCbSize;
  custom_app_task_attr.priority = (osPriority_t)SL_ZIGBEE_APP_FRAMEWORK_RTOS_TASK_PRIORITY;
  custom_app_task_attr.attr_bits = 0;
  custom_app_task_attr.tz_module = 0;

  custom_app_task_id = osThreadNew(custom_app_task,
                                   NULL,
                                   &custom_app_task_attr);
  assert(custom_app_task_id != NULL);

  custom_app_subscribe_to_callback_event_queue();
}

void sl_zigbee_af_main_init_cb(void) //part of the call to sli_zigbee_app_framework_init_callback,
                                     // which is called for all apps, and not just app_framework apps
{
  sl_zigbee_app_debug_println("BOOT: Simple ZigBee App with RTOS");
}
//------------------------------------------------------------------------------
// Stack callbacks
void sl_zigbee_stack_status_handler(
  sl_status_t status)
{
  (void)status;
  sl_zigbee_app_debug_println("Stack status handler, 0x%08X", status);
}

static void app_internal_message_handler(sl_802154_short_addr_t sender,
                                         sl_zigbee_aps_frame_t *apsFrame,
                                         uint8_t messageLength,
                                         uint8_t *message)
{
  (void)messageLength;
  switch (apsFrame->clusterId) {
    case CLUSTER_BUFFER_TEST_REQUEST: {
      sl_status_t status;
      sl_zigbee_aps_frame_t outApsFrame;
      uint8_t sequenceLength = message[0];
      uint8_t *response = (uint8_t*)sl_malloc(sequenceLength + 2);

      outApsFrame.sourceEndpoint = apsFrame->destinationEndpoint;
      outApsFrame.destinationEndpoint = apsFrame->sourceEndpoint;
      outApsFrame.clusterId = CLUSTER_BUFFER_TEST_RESPONSE;
      outApsFrame.profileId = TEST_PROFILE_ID;
      outApsFrame.options = apsFrame->options;

      response[0] = sequenceLength;
      response[1] = 0x00; // status: success
      for (uint8_t i = 0; i < sequenceLength; i++) {
        response[2 + i] = i;
      }

      status = sl_zigbee_send_unicast(SL_ZIGBEE_OUTGOING_DIRECT,
                                      sender,
                                      &outApsFrame,
                                      0x00, // tag
                                      sequenceLength + 2,
                                      response,
                                      NULL);
      sl_free(response);

      if (status != SL_STATUS_OK) {
        sl_zigbee_app_debug_println("sl_zigbee_send_unicast fails");
        return;
      }
      sl_zigbee_app_debug_println("Responded to buffer request, length 0x%02X", sequenceLength);
      break;
    }

    case CLUSTER_BUFFER_TEST_RESPONSE: {
      sl_zigbee_app_debug_println("Buffer response: length 0x%02X, status 0x%02X",
                                  message[0], message[1]);
      break;
    }

    default:
      break;
  }
}

// The entry functions to send message broadcast, multicast or unicast
#define ZA_MAX_HOPS 12
#define INVALID_MESSAGE_TAG 0x0000

void sl_zigbee_incoming_message_handler(
  sl_zigbee_incoming_message_type_t type,
  sl_zigbee_aps_frame_t *apsFrame,
  sl_zigbee_rx_packet_info_t *packetInfo,
  uint8_t messageLength,
  uint8_t *message)
{
  (void)type;
  (void)packetInfo;

  switch (apsFrame->profileId) {
    case TEST_PROFILE_ID:
      app_internal_message_handler(packetInfo->sender_short_id, apsFrame, messageLength, message);
      break;
    default:
      break;
  }
}

//-------------------------------------------------------------------------------
// rtos related functionality

extern void sli_zigbee_process_stack_callbacks_event(sl_event_t *cb_event);

//------------------------------------------------------------------------------
// Static functions.
void sl_zigbee_message_sent_handler(
  sl_status_t status,
  sl_zigbee_outgoing_message_type_t type,
  uint16_t indexOrDestination,
  sl_zigbee_aps_frame_t *apsFrame,
  uint16_t messageTag,
  uint8_t messageLength,
  uint8_t *message)
{
  (void)type;
  (void)indexOrDestination;
  (void)apsFrame;
  (void)messageTag;
  (void)messageLength;
  (void)message;

  sl_zigbee_app_debug_println("app_message_sent_cb, 0x%08X", status);
}
// sli_zigbee_process_stack_callbacks_event could be replaced with another function similar to the following ( in case a more limited main loop is desired)
// static void app_process_event(sl_event_t *sys_event)
// {
//   sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) sys_event->event_data;
//   switch (cb_event->tag) {
//     case SLI_ZIGBEE_STACK_STACK_STATUS_HANDLER_IPC_EVENT_TYPE:
//       app_stack_status_cb(cb_event->data.stack_status_handler.status);
//       break;
//     default:
//       /* do nothing */
//       break;
//   }
// }

static void custom_app_task(void *p_arg)
{
  (void)p_arg;
  uint32_t yield_time_ms = 10;

  sli_zigbee_app_framework_init_callback(); // sl_system_common subscription, not dependent on app_framework

  while (true) {
    sli_zigbee_app_framework_tick_callback(); // sl_system_common subscription, not dependent on app_framework

    sl_event_t *event_ptr;

    //Process the events in the event queue
    if ( SL_STATUS_OK == sl_event_queue_get(callback_event_queue, 0, yield_time_ms, &event_ptr)) {
      sli_zigbee_process_stack_callbacks_event(event_ptr);
      sl_event_process(&event_ptr);
    }
  }
}
