/***************************************************************************//**
 * @brief ZigBee Application Framework common code.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "hal.h"
#include "stack/include/sl_zigbee.h"
#include "stack/include/sl_zigbee_token.h"
#include "sl_zigbee_system_common.h"
#include "zigbee_app_framework_callback.h"
#ifndef EZSP_HOST
#include "stack/internal/inc/network-formation-internal-def.h"
#endif
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "sl_code_classification.h"

#if (defined(SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT) || (defined(SL_ZIGBEE_SCRIPTED_TEST))) \
  || (defined(SL_ZIGBEE_AF_NCP) && defined(SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT))
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_push_endpoint_network_index(uint8_t endpoint);
extern sl_status_t sl_zigbee_af_pop_network_index(void);
extern uint8_t sl_zigbee_af_index_from_endpoint(uint8_t endpoint);
#else
#define sl_zigbee_af_push_network_index(networkIndex)
#define sl_zigbee_af_push_endpoint_network_index(endpoint)
#define sl_zigbee_af_pop_network_index()
#define sl_zigbee_af_index_from_endpoint(endpoint) (0xFF)
#endif

extern void sl_zigbee_af_main_init_cb(void);
extern void sl_zigbee_af_main_tick_cb(void);

sli_zigbee_event_queue_t sli_zigbee_af_app_event_queue;
#if defined(SL_CATALOG_KERNEL_PRESENT)
osMutexId_t sli_zigbee_af_mutex_id;
static const osMutexAttr_t app_framework_mutex_attr = {
  .name = "Zigbee App Framework Mutex",
  .attr_bits = osMutexRecursive | osMutexPrioInherit,
};
extern void sl_zigbee_af_acquire_lock();
extern void sl_zigbee_af_release_lock();
#endif // defined(SL_CATALOG_KERNEL_PRESENT)

//------------------------------------------------------------------------------
// Init and tick callbacks

#ifndef EZSP_HOST
void sli_zigbee_stack_init_callback(void)
{
  sl_status_t status;
  #if (!defined(SL_CATALOG_SL_MAIN_PRESENT) && !defined(SL_ZIGBEE_SCRIPTED_TEST))
  sli_legacy_buffer_manager_initialize_buffers();
  #endif
  // Initialize the radio and the stack.  If this fails, we have to assert
  // because something is wrong.
  status = sli_zigbee_stack_init();
  assert(status == SL_STATUS_OK);
}
#endif // EZSP_HOST

extern void sli_zigbee_af_event_init(void);
extern void sli_zigbee_af_local_data_init(void);
extern void sli_zigbee_af_init_done(void);

#if (defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && !defined(SL_ZIGBEE_TEST) && !defined(EZSP_HOST) && !defined(PRO_COMPLIANCE))
#define EXTENDED_RESET_INFO
#include "cortexm3/diagnostic.h"
#include "sl_zigbee_debug_print.h"
#endif // (defined(SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT) && !defined(SL_ZIGBEE_TEST) && !defined(EZSP_HOST) && !defined(PRO_COMPLIANCE))
#ifdef EZSP_HOST
extern sl_status_t halStackInitTokens(void);
#endif // EZSP_HOST

#ifdef EZSP_HOST
extern sl_status_t halStackInitTokens(void);
#endif // EZSP_HOST

void sli_zigbee_app_framework_init_callback(void)
{
#ifdef EZSP_HOST
  // Initialize all tokens, which includes 'stack' tokens
  // (while stack tokens are initialized on host, they have no use apart from their
  // existence to remain backwards-compatible with older host NVM files)
  // On SoC/NCP, stack tokens are initialized in the zigbee stack library
  sl_status_t status = halStackInitTokens();
  assert(status == SL_STATUS_OK);
#endif // EZSP_HOST

  // Init the event queue.
  sli_zigbee_initialize_event_queue(&sli_zigbee_af_app_event_queue);

#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Create mutex for Zigbee App Framework Event
  sli_zigbee_af_mutex_id = osMutexNew(&app_framework_mutex_attr);
  if (sli_zigbee_af_mutex_id == NULL) {
    // Unable to create mutex
    assert(0);
  }
#endif // defined(SL_CATALOG_KERNEL_PRESENT)

  // Call the application init callback.
  sl_zigbee_af_main_init_cb();

  // Call the generated init callbacks.
  sli_zigbee_af_event_init();
  sli_zigbee_af_local_data_init();
  sli_zigbee_af_init_done();

#if defined(EXTENDED_RESET_INFO)
  #ifndef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
  sl_zigbee_app_debug_println("Reset info: 0x%02X (%s)",
                              halGetResetInfo(),
                              halGetResetString());
  #endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT

  sl_zigbee_app_debug_println("Extended Reset info: 0x%04X (%s)",
                              halGetExtendedResetInfo(),
                              halGetExtendedResetString());

  if (halResetWasCrash()) {
#if defined(SL_CATALOG_IOSTREAM_UART_COMMON_PRESENT)
    // The crash printing is implemented for IOSTREAM UART in diagnostic module.
    // We pass port 0 here though this parameter is unused in the legacy HAL
    // version of the diagnostic code.
    halPrintCrashSummary(0);
    halPrintCrashDetails(0);
    halPrintCrashData(0);
#endif
  }
#endif // EXTENDED_RESET_INFO
}

#ifndef EZSP_HOST
void sli_zigbee_stack_tick_callback(void)
{
  // Pet the watchdog.
  halResetWatchdog();
  // Call the stack tick API.
  sl_zigbee_tick();
}
#endif // EZSP_HOST

extern void sli_zigbee_af_tick(void);

void sli_zigbee_app_framework_tick_callback(void)
{
  // Pet the watchdog.
  halResetWatchdog();

  // Call the application tick callback.
  sl_zigbee_af_main_tick_cb();

  // Call the generated tick callback (tick the components that subscribed to it).
  sli_zigbee_af_tick();

  // Run the application event queue.
  sli_zigbee_af_run_events();
}

//------------------------------------------------------------------------------
// Event System

static void event_common_handler(sl_zigbee_af_event_t *event)
{
  bool is_network_event = sli_zigbee_af_event_is_network_event(event);
  bool is_endpoint_event = sli_zigbee_af_event_is_endpoint_event(event);

  assert(!(is_network_event && is_endpoint_event));

  if (is_network_event) {
    sl_zigbee_af_push_network_index(sli_zigbee_af_event_get_network_index(event));
  } else if (is_endpoint_event) {
    sl_zigbee_af_push_endpoint_network_index(sli_zigbee_af_event_get_endpoint(event));
  }

  assert(event->dataPtr);

  sli_zigbee_event_set_inactive(event);

  // Endpoint events pass the endpoint to the handler, other events pass the
  // event itself.
  // TODO: we should clean this up by always pass the event end let the code
  // in the handler retrieve the endpoint.
  if (is_endpoint_event) {
    ((void (*)(uint8_t))(event->dataPtr))(sli_zigbee_af_event_get_endpoint(event));
  } else {
    ((void (*)(sl_zigbee_af_event_t *))(event->dataPtr))(event);
  }

  if (is_network_event || is_endpoint_event) {
    sl_zigbee_af_pop_network_index();
  }
}

void sli_zigbee_af_event_internal_init(sl_zigbee_af_event_t *event,
                                       const char *event_name,
                                       void *handler,
                                       uint8_t network_index,
                                       uint8_t endpoint)
{
  // We do not support events that are multiplexed by network and endpoint
  // simultaneously.
  assert(!(network_index < 0xFF && endpoint < 0xFF));

  event->next = NULL;
  event->actions.queue = &sli_zigbee_af_app_event_queue;
  event->actions.handler = event_common_handler;
  event->actions.marker = NULL;
  event->dataPtr = handler;
  event->data = 0;
  event->actions.name = event_name;

  if (network_index < 0xFF) {
    sli_zigbee_af_event_set_network_event(event);
    sli_zigbee_af_event_set_network_index(event, network_index);
  }
  if (endpoint < 0xFF) {
    sli_zigbee_af_event_set_endpoint_event(event);
    sli_zigbee_af_event_set_endpoint(event, endpoint);
  }
}

void sli_zigbee_af_event_network_event_init(sl_zigbee_af_event_t *event,
                                            const char *event_name,
                                            void (*handler)(sl_zigbee_af_event_t *))
{
  uint8_t i;
  for (i = 0; i < SL_ZIGBEE_SUPPORTED_NETWORKS; i++) {
    sli_zigbee_af_event_internal_init(event, event_name, (void*)handler, i, 0xFF);
    event++;
  }
}

sl_zigbee_af_event_t* sli_zigbee_af_event_get_event_ptr(sl_zigbee_af_event_t *event,
                                                        uint8_t endpoint)
{
  bool is_network_event = sli_zigbee_af_event_is_network_event(event);
  bool is_endpoint_event = sli_zigbee_af_event_is_endpoint_event(event);

  // We do not support events that are multiplexed by network and endpoint
  // simultaneously.
  assert(!(is_network_event && is_endpoint_event));

  if (endpoint < 0xFF) {
    // If we are passing a valid endpoint to this API, we enforce that the event
    // is actually an endpoint event.
    assert(is_endpoint_event);

    uint8_t endpoint_index = sl_zigbee_af_index_from_endpoint(endpoint);
    assert(endpoint_index < 0xFF);
    return event + endpoint_index;
  }

  if (is_network_event) {
#if SL_ZIGBEE_SUPPORTED_NETWORKS == 1
    // We enforce that the network-0 event is the one passed in.
    assert(sli_zigbee_af_event_get_network_index(event) == 0);
#endif // SL_ZIGBEE_SUPPORTED_NETWORKS == 1
    event = event + sl_zigbee_get_current_network();
  }

  return event;
}

#ifndef SL_CATALOG_KERNEL_PRESENT
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
WEAK(void sl_zigbee_wakeup_common_task(void))
{
}
#endif
void sl_zigbee_wakeup_common_task(void);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
void sli_zigbee_stack_rtos_stack_wakeup_isr_handler(void)
{
  sl_zigbee_wakeup_common_task();
  sl_zigbee_rtos_stack_wakeup_isr_handler();
}

// Event initialisation routine for the event that gets activated from ISRs.
void sl_zigbee_af_isr_event_init(sl_zigbee_af_event_t *event,
                                 void (*handler)(sl_zigbee_af_event_t *))
{
  sl_zigbee_af_event_init(event, handler);
  event->actions.marker = sli_zigbee_isr_event_marker;
}

void sli_zigbee_af_event_set_delay_ms(sl_zigbee_af_event_t *event, uint8_t endpoint, uint32_t delay)
{
  sli_zigbee_event_set_delay_ms(sli_zigbee_af_event_get_event_ptr(event, endpoint), delay);
}

void sli_zigbee_af_event_set_active(sl_zigbee_af_event_t *event, uint8_t endpoint)
{
  sli_zigbee_event_set_active(sli_zigbee_af_event_get_event_ptr(event, endpoint));
}

void sli_zigbee_af_event_set_inactive(sl_zigbee_af_event_t *event, uint8_t endpoint)
{
  sli_zigbee_event_set_inactive(sli_zigbee_af_event_get_event_ptr(event, endpoint));
}

bool sli_zigbee_af_event_is_scheduled(sl_zigbee_af_event_t *event, uint8_t endpoint)
{
  bool result = false;
  result = sli_zigbee_event_is_scheduled(sli_zigbee_af_event_get_event_ptr(event, endpoint));
  return result;
}

uint32_t sli_zigbee_af_event_get_remaining_ms(sl_zigbee_af_event_t *event, uint8_t endpoint)
{
  uint32_t remaining_ms = 0;
  remaining_ms = sli_zigbee_event_get_remaining_ms(sli_zigbee_af_event_get_event_ptr(event, endpoint));
  return remaining_ms;
}

uint32_t sli_zigbee_af_ms_to_next_event()
{
  uint32_t next_ms = 0;
  next_ms = sli_zigbee_ms_to_next_queue_event(&sli_zigbee_af_app_event_queue);
  return next_ms;
}
void sli_zigbee_af_run_events()
{
  sli_zigbee_run_event_queue(&sli_zigbee_af_app_event_queue);
}

//------------------------------------------------------------------------------
// Callbacks stubs

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_ZIGBEE_STACK, SL_CODE_CLASS_TIME_CRITICAL)
WEAK(void sl_zigbee_rtos_stack_wakeup_isr_handler(void))
{
}

WEAK(void sl_zigbee_af_main_init_cb(void))
{
}

WEAK(void sl_zigbee_af_main_tick_cb(void))
{
}
