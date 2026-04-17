/***************************************************************************//**
 * @brief Adaptation for running Bluetooth in RTOS
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cmsis_os2.h>
#include "sl_core.h"
#include "sl_bluetooth.h"
#include "sli_bgapi.h"
#include "sli_bt_api.h"
#include "sl_bt_stack_config.h"
#include "sl_bt_rtos_config.h"
#include "sl_bt_rtos_adaptation.h"
#include "sli_bgapi_rtos_adaptation.h"
#include "sl_common.h"
#include "sl_component_catalog.h"
#include "sl_memory_manager.h"

#ifdef CONFIGURATION_HEADER
#include CONFIGURATION_HEADER
#endif // CONFIGURATION_HEADER

#if defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
#include "sli_bt_event_system.h"
#endif

#if defined(SL_CATALOG_BGAPI_TRACE_PRESENT)
#include "sli_bgapi_trace.h"
#endif

// Definitions for Bluetooth state flags that are needed already before the RTOS
// kernel has been started, i.e. if the application calls
// `sl_bt_system_start_bluetooth()` from `app_init()`. These flags are used with
// the static state variable `bluetooth_state_flags`.
#define SLI_BT_RTOS_STATE_FLAG_KERNEL_STARTED         0x01U  // The RTOS kernel has been started
#define SLI_BT_RTOS_STATE_FLAG_STARTING               0x02U  // The Bluetooth stack is starting
#define SLI_BT_RTOS_STATE_FLAG_STARTED                0x04U  // The Bluetooth stack has successfully started
#define SLI_BT_RTOS_STATE_FLAG_STOPPING               0x08U  // The Bluetooth stack is stopping
#define SLI_BT_RTOS_STATE_FLAG_INTERNAL_CALLS_ENABLED 0x10U  // Task internal calls have been enabled

// The event flags defined below are specific to a thread and are used with the
// thread context's `event_flags` field. The values would not necessarily need
// to be unique between threads. We still choose unique values to make it easier
// to detect if a flag of one thread is accidentally used with another.

// Common event flags used with @ref wait_bluetooth_rtos_thread_event_flags
#define SLI_BT_RTOS_EVENT_FLAG_ERROR                       0x80000000U  // An error has occurred while waiting for events

// Event flags for the Bluetooth thread
#define SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_START            0x00000001U  // Bluetooth task is instructed to start
#define SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_WAKEUP           0x00000002U  // Bluetooth task needs an update
#define SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_COMMAND_WAITING  0x00000004U  // Bluetooth command is waiting to be processed
#define SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_EVENT_HANDLED    0x00000008U  // Bluetooth event posted to the event handler thread has been handled
#define SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_STOPPED          0x00000010U  // Bluetooth stack has been stopped

// Event flags for the Link Layer thread
#define SLI_BT_RTOS_LINKLAYER_THREAD_FLAG_WAKEUP           0x00000100U  // Linklayer task needs an update

// Event flags for the event handler thread
#define SLI_BT_RTOS_EVENT_THREAD_FLAG_EVENT_WAITING        0x00010000U  // Bluetooth event is waiting to be processed

static volatile uint8_t bluetooth_state_flags = 0;

static volatile sl_bt_msg_t bluetooth_evt_instance;

// When we commit to processing a BGAPI command, we store the relevant
// information while running in the calling task's context. This information is
// read by the Bluetooth stack task when it executes the command.
static volatile sl_bgapi_handler current_bgapi_handler = NULL;
static volatile const void *current_command_buf = NULL;
static volatile void *current_response_buf = NULL;
static volatile size_t current_response_buf_size = 0;
static volatile sl_status_t current_command_execution_status;

/**
 * @brief Structure to represent a thread in the Bluetooth RTOS adaptation
 */
typedef struct {
  osThreadId_t       thread_id;           ///< The ID of the thread
  osSemaphoreId_t    wakeup_semaphore_id; ///< The ID of the wakeup semaphore
  volatile uint32_t  event_flags;         ///< Event flags to be handled after wakeup
} sli_bt_rtos_thread_t;

//Bluetooth stack thread
static void bluetooth_thread(void *p_arg);
static sli_bt_rtos_thread_t thread_bluetooth;
static const osThreadAttr_t thread_bluetooth_attr = {
  .name = "Bluetooth stack",
  .stack_size = SL_BT_RTOS_HOST_STACK_TASK_STACK_SIZE,
  .priority = (osPriority_t) SL_BT_RTOS_HOST_STACK_TASK_PRIORITY
};
static const osSemaphoreAttr_t semaphore_bluetooth_attr = {
  .name = "Bluetooth stack wakeup"
};

//Bluetooth linklayer thread
static void linklayer_thread(void *p_arg);
static sli_bt_rtos_thread_t thread_linklayer;
static const osThreadAttr_t thread_linklayer_attr = {
  .name = "Bluetooth linklayer",
  .stack_size = SL_BT_RTOS_LINK_LAYER_TASK_STACK_SIZE,
  .priority = (osPriority_t) SL_BT_RTOS_LINK_LAYER_TASK_PRIORITY
};
static const osSemaphoreAttr_t semaphore_linklayer_attr = {
  .name = "Bluetooth linklayer wakeup"
};

// The Bluetooth event handler thread is only used when Event System IPC is not
// included
#if !defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
static sli_bt_rtos_thread_t thread_event_handler;
static void event_handler_thread(void *p_arg);
static const osThreadAttr_t thread_event_handler_attr = {
  .name = "Bluetooth event handler",
  .stack_size = SL_BT_RTOS_EVENT_HANDLER_STACK_SIZE,
  .priority = (osPriority_t) SL_BT_RTOS_EVENT_HANDLER_TASK_PRIORITY
};
static const osSemaphoreAttr_t semaphore_event_handler_attr = {
  .name = "Bluetooth event handler wakeup"
};
#endif // !defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)

static volatile osMutexId_t bluetooth_mutex_id;
static const osMutexAttr_t bluetooth_mutex_attr = {
  .name = "Bluetooth Mutex",
  .attr_bits = osMutexRecursive | osMutexPrioInherit,
};

static volatile osMutexId_t bgapi_mutex_id;
static const osMutexAttr_t bgapi_mutex_attr = {
  .name = "BGAPI Mutex",
  .attr_bits = osMutexRecursive | osMutexPrioInherit,
};

static volatile osSemaphoreId_t bgapi_response_semaphore_id;
static const osSemaphoreAttr_t bgapi_response_semaphore_attr = {
  .name = "BGAPI response"
};

/**
 * @brief Convert CMSIS-RTOS2 osStatus_t to @ref sl_status_t
 */
static sl_status_t os2sl_status(osStatus_t ret)
{
  switch (ret) {
    case osOK:
      return SL_STATUS_OK;

    case osErrorTimeout:
      return SL_STATUS_TIMEOUT;

    case osErrorISR:
      return SL_STATUS_ISR;

    case osErrorParameter:
      return SL_STATUS_INVALID_PARAMETER;

    default:
      return SL_STATUS_FAIL;
  }
}

/**
 * @brief Delete a Bluetooth RTOS adaptation thread
 *
 * @param[out] thread The thread structure that records the info for the thread
 *   to delete
 */
static void delete_bluetooth_rtos_thread(sli_bt_rtos_thread_t *thread)
{
  EFM_ASSERT(thread != NULL);

  if (thread->thread_id != NULL) {
    (void) osThreadTerminate(thread->thread_id);
    thread->thread_id = NULL;
  }

  if (thread->wakeup_semaphore_id != NULL) {
    (void) osSemaphoreDelete(thread->wakeup_semaphore_id);
    thread->wakeup_semaphore_id = NULL;
  }
}

/**
 * @brief Create a Bluetooth RTOS adaptation thread
 *
 * This function creates a wakeup semaphore and the corresponding thread to use
 * for Bluetooth RTOS adaptation. The thread function @p thread_func will
 * receive a pointer to @p thread as the thread argument.
 *
 * To support the special case where the event thread is disabled by
 * configuration, this function may be called with @p thread_func set to NULL.
 * In this case the thread is not created, but the wakeup semaphore and the
 * event flags can be used by any calling thread to wait for an event.
 *
 * @param[out] thread          The thread structure that records the info for this thread
 * @param[in]  semaphore_attr  Attributes of the wakeup semaphore
 * @param[in]  thread_func     Thread function
 * @param[in]  thread_attr     Attributes of the thread to create
 */
static sl_status_t create_bluetooth_rtos_thread(sli_bt_rtos_thread_t    *thread,
                                                const osSemaphoreAttr_t *semaphore_attr,
                                                osThreadFunc_t           thread_func,
                                                const osThreadAttr_t    *thread_attr)
{
  // Parameters must have been supplied and we can't yet have the RTOS objects
  EFM_ASSERT(thread != NULL);
  EFM_ASSERT(thread->thread_id == NULL);
  EFM_ASSERT(thread->wakeup_semaphore_id == NULL);

  // Thread will typically immmediately start waiting on the wakeup semaphore,
  // so init event flags and create the semaphore first
  thread->event_flags = 0;
  uint32_t max_count = 1;
  uint32_t initial_count = 0;
  thread->wakeup_semaphore_id = osSemaphoreNew(max_count, initial_count, semaphore_attr);
  if (thread->wakeup_semaphore_id == NULL) {
    return SL_STATUS_FAIL;
  }

  // Create the thread itself, if a function was supplied
  if (thread_func != NULL) {
    thread->thread_id = osThreadNew(thread_func, thread, thread_attr);
    if (thread->thread_id == NULL) {
      (void) osSemaphoreDelete(thread->wakeup_semaphore_id);
      thread->wakeup_semaphore_id = NULL;
      return SL_STATUS_FAIL;
    }
  } else {
    thread->thread_id = NULL;
  }

  return SL_STATUS_OK;
}

/**
 * @brief Set event flags to the specifed Bluetooth RTOS adaptation thread
 *
 * This function sets the specified event flags to a Bluetooth RTOS adaptation
 * thread. The flags are bitwise OR'ed, i.e. flag bits that are zero do not
 * affect flags that have already been set. Flag bits that are set to one are
 * set to the thread and the target thread is woken up.
 *
 * @param[in,out] thread       The thread structure that records the info for this thread
 * @param[in]     event_flags  The event flags to set
 */
static void set_bluetooth_rtos_thread_event_flags(sli_bt_rtos_thread_t *thread,
                                                  uint32_t              event_flags)
{
  CORE_DECLARE_IRQ_STATE;

  // Set the flags in an atomic section
  CORE_ENTER_ATOMIC();
  thread->event_flags |= event_flags;
  CORE_EXIT_ATOMIC();

  // To avoid any risk of a race condition, we err on the side of caution and
  // always wakeup the target thread. The threads don't react to the number of
  // wakeups, but to seeing flags set, so redundant wakeups are not a problem.
  // Also, since we're using a binary semaphore, the number of wakeups won't
  // accumulate. If we wake up the thread multiple times, the semaphore clamps
  // at one token, the thread wakes up once, and sees all the flags accumulated
  // so far.
  //
  // We deliberately don't check the return code when we release the semaphore.
  // The only possible errors documented by CMSIS-RTOS2 is reaching the maximum
  // count (this is OK for us, as we're deliberately using a binary semaphore
  // that clamps to one token), and having a NULL or invalid semaphore ID. That
  // can only happen if someone else corrupts our memory, as the semaphore
  // creation is checked for success before the threads start using it. That
  // error would be unrecoverable anyway.
  (void) osSemaphoreRelease(thread->wakeup_semaphore_id);
}

/**
 * @brief Pop the current event flags in the specifed Bluetooth RTOS adaptation thread
 *
 * This function returns a bitmask of event flags that were set and clears those
 * bits in the thread structure.
 *
 * @param[in,out] thread    The thread structure that records the info for this thread
 * @param[in]  event_flags  The event flags to wait for
 *
 * @return The event flags that were set and have been cleared
 */
static uint32_t pop_bluetooth_rtos_thread_event_flags(sli_bt_rtos_thread_t *thread)
{
  CORE_DECLARE_IRQ_STATE;

  // Check and clear the flags atomically
  CORE_ENTER_ATOMIC();
  uint32_t event_flags = thread->event_flags;
  if (event_flags != 0) {
    thread->event_flags = 0;
  }
  CORE_EXIT_ATOMIC();

  return event_flags;
}

/**
 * @brief Wait for event flags in the specifed Bluetooth RTOS adaptation thread
 *
 * This function waits until at least one event flag has become set for the
 * thread. If flags are set already, the function returns immediately. The
 * function returns a bitmask of event flags that were set and clears those bits
 * in the thread structure.
 *
 * @param[in,out] thread    The thread structure that records the info for this thread
 * @param[in]  event_flags  The event flags to wait for
 *
 * @return The event flags that were set and have been cleared. If an error
 *   occurs, the flag bit @ref SLI_BT_RTOS_EVENT_FLAG_ERROR is set in the return
 *   value.
 */
static uint32_t wait_bluetooth_rtos_thread_event_flags(sli_bt_rtos_thread_t *thread)
{
  // Loop until we see a flag set or an error occurs
  while (1) {
    // Pop the current flags
    uint32_t event_flags = pop_bluetooth_rtos_thread_event_flags(thread);

    // If any flags were set, we're done
    if (event_flags != 0) {
      return event_flags;
    }

    // Wait on the wakeup semaphore
    osStatus_t os_status = osSemaphoreAcquire(thread->wakeup_semaphore_id, osWaitForever);
    if (os_status != osOK) {
      // An error has occurred. Set the error bit and store the `sl_status_t` in
      // the other bits.
      sl_status_t status = os2sl_status(os_status);
      return SLI_BT_RTOS_EVENT_FLAG_ERROR | (uint32_t) status;
    }
  }
}

// Make permanent memory allocations for Bluetooth RTOS adaptation.
sl_status_t sli_bt_rtos_adaptation_permanent_allocation(void)
{
  // The Bluetooth stack including its RTOS adaptation is started with a BGAPI
  // command. The BGAPI synchronization primitives are therefore always created
  // already at init-time and are never deleted.
  uint32_t max_count = 1;
  uint32_t initial_count = 0;
  bgapi_response_semaphore_id = osSemaphoreNew(max_count, initial_count, &bgapi_response_semaphore_attr);
  if (bgapi_response_semaphore_id == NULL) {
    return SL_STATUS_FAIL;
  }

  bgapi_mutex_id = osMutexNew(&bgapi_mutex_attr);
  if (bgapi_mutex_id == NULL) {
    (void) osSemaphoreDelete(bgapi_response_semaphore_id);
    bgapi_response_semaphore_id = NULL;
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

// BGAPI component initialization function for Bluetooth RTOS adaptation
sl_status_t sli_bt_rtos_adaptation_init(const void *feature_config)
{
  // RTOS adaptation configuration is read directly from the configuration
  // header and the `feature_config` pointer is not used
  (void) feature_config;

  // Ccreate the mutex for Bluetooth stack
  sl_status_t status = SL_STATUS_FAIL;
  EFM_ASSERT(bluetooth_mutex_id == NULL);
  bluetooth_mutex_id = osMutexNew(&bluetooth_mutex_attr);
  if (bluetooth_mutex_id == NULL) {
    return SL_STATUS_FAIL;
  }

  // Create thread for Bluetooth stack
  status = create_bluetooth_rtos_thread(&thread_bluetooth,
                                        &semaphore_bluetooth_attr,
                                        bluetooth_thread,
                                        &thread_bluetooth_attr);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Create thread for Linklayer
  status = create_bluetooth_rtos_thread(&thread_linklayer,
                                        &semaphore_linklayer_attr,
                                        linklayer_thread,
                                        &thread_linklayer_attr);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Create thread for Bluetooth event handler when Event System IPC is not used
#if !defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
  status = create_bluetooth_rtos_thread(&thread_event_handler,
                                        &semaphore_event_handler_attr,
                                        event_handler_thread,
                                        &thread_event_handler_attr);
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif // !defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)

#if defined(SL_CATALOG_BTMESH_PRESENT)
  // Bluetooth Mesh calls Bluetooth's BGAPI commands internally. Enable
  // task-internal calls on the host stack BGAPI device.
  status = sli_bgapi_enable_task_internal_calls(sl_bgapi_dev_type_bt);
  if (status != SL_STATUS_OK) {
    return status;
  }
  bluetooth_state_flags |= SLI_BT_RTOS_STATE_FLAG_INTERNAL_CALLS_ENABLED;
#endif

  // Now we're ready to handle BGAPI commands via the RTOS adaptation. Set the
  // BGAPI delegate.
  sli_bgapi_set_cmd_handler_delegate(sli_bt_cmd_handler_rtos_delegate);

  return SL_STATUS_OK;
}

// Internal de-init called when we're ready to shutdown the stack task
static void deinit_rtos_adaptation(void)
{
  #if defined(SL_CATALOG_BTMESH_PRESENT)
  // The Bluetooth task is about to be deleted, so Mesh stack won't be running
  // in the task anymore. If task-internal calls were enabled successfully,
  // disable them now.
  if (bluetooth_state_flags & SLI_BT_RTOS_STATE_FLAG_INTERNAL_CALLS_ENABLED) {
    sli_bgapi_disable_task_internal_calls(sl_bgapi_dev_type_bt);
    bluetooth_state_flags &= ~SLI_BT_RTOS_STATE_FLAG_INTERNAL_CALLS_ENABLED;
  }
#endif

  // Delete the threads that we created for the link layer and for the event
  // handler
  delete_bluetooth_rtos_thread(&thread_linklayer);
#if !defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
  delete_bluetooth_rtos_thread(&thread_event_handler);
#endif

  // Delete the synchronization primitives
  if (bluetooth_mutex_id != NULL) {
    (void) osMutexDelete(bluetooth_mutex_id);
    bluetooth_mutex_id = NULL;
  }
}

// BGAPI component de-initialization function for Bluetooth RTOS adaptation
void sli_bt_rtos_adaptation_deinit(void)
{
  // If the host stack task has been created and we're in it, we need to let the
  // task itself trigger the deinitialization when the task has finished.
  // Exiting the processing loop is triggered by `sli_bt_rtos_adaptation_stop()`
  // and no de-init happens here.
  if ((thread_bluetooth.thread_id != NULL)
      && (osThreadGetId() == thread_bluetooth.thread_id)) {
    return;
  }

  // We're not in the host stack task, so de-init is happening because
  // initialization has failed and we didn't have a chance to start the task
  // yet. Clear the state flag and de-init here directly.
  bluetooth_state_flags &= ~SLI_BT_RTOS_STATE_FLAG_STARTING;
  deinit_rtos_adaptation();
  delete_bluetooth_rtos_thread(&thread_bluetooth);
}

// This callback is called from interrupt context (Kernel Aware) to wake up the
// link layer task
void sli_bt_rtos_ll_callback()
{
  set_bluetooth_rtos_thread_event_flags(&thread_linklayer,
                                        SLI_BT_RTOS_LINKLAYER_THREAD_FLAG_WAKEUP);
}

// This callback is called by the Bluetooth stack to wakeup the Bluetooth
// thread. This call may come from a thread or from interrupt context.
void sli_bt_rtos_stack_callback()
{
  set_bluetooth_rtos_thread_event_flags(&thread_bluetooth,
                                        SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_WAKEUP);
}

// Called by Bluetooth Event System IPC to mark an event as handled
void sli_bt_rtos_set_event_handled()
{
  // Signal the host stack thread that the event has been handled
  set_bluetooth_rtos_thread_event_flags(&thread_bluetooth,
                                        SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_EVENT_HANDLED);
}

/**
 * Internal stack function to get how long the Bluetooth stack can sleep.
 *
 * @return 0 if the stack cannot sleep;
 * Maximum value of uint32_t if the stack has no task scheduled to process;
 * The ticks (in sleeptimer frequency) the stack needs to wake up to process a task
 */
extern uint32_t sli_bt_can_sleep_ticks();

#if defined(SL_CATALOG_BTMESH_PRESENT)
// This is to avoid casting a function pointer to a different type in the
// event_types table.
static sl_status_t _btmesh_pop_event(sl_bt_msg_t *msg)
{
  return sl_btmesh_pop_event((sl_btmesh_msg_t*) msg);
}
#endif

typedef struct {
  enum sl_bgapi_dev_types dev_type;
  bool (*event_pending_fn)(void);
  sl_status_t (*pop_event_fn)(sl_bt_msg_t *buffer);
} bgapi_device_type;

static const bgapi_device_type bgapi_device_table[] = {
  { sl_bgapi_dev_type_bt, sl_bt_event_pending, sl_bt_pop_event },
#if defined(SL_CATALOG_BTMESH_PRESENT)
  { sl_bgapi_dev_type_btmesh, sl_btmesh_event_pending, _btmesh_pop_event },
#endif
};

#define NUM_BGAPI_DEVICES   (sizeof(bgapi_device_table) / sizeof(bgapi_device_table[0]))

//Bluetooth task, it waits for events from bluetooth and handles them
void bluetooth_thread(void *p_arg)
{
  // The input parameter is our thread structure
  sli_bt_rtos_thread_t *thread = (sli_bt_rtos_thread_t *) p_arg;

  uint32_t flags = SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_EVENT_HANDLED;

  // Wait until the init sequence has completed and we've been instructed to
  // start the host stack
  while ((flags & SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_START) == 0) {
    flags |= wait_bluetooth_rtos_thread_event_flags(thread);
  }

  // Start the Bluetooth stack
  sl_status_t start_status = sli_bt_start_bgapi_device();
  if (start_status == SL_STATUS_OK) {
    // bluetooth mesh must have bluetooth started before it is initalizes
#if defined(SL_CATALOG_BTMESH_PRESENT)
    sl_btmesh_init();
#endif

    // Starting the stack has succeeded and we update the state accordingly.
    uint8_t updated_state_flags = bluetooth_state_flags;
    updated_state_flags &= ~SLI_BT_RTOS_STATE_FLAG_STARTING;
    updated_state_flags |= SLI_BT_RTOS_STATE_FLAG_STARTED;
    bluetooth_state_flags = updated_state_flags;
  } else {
    // We failed to start, so consider the stack stopped already in the local
    // flags. This way we won't enter the processing loop at all, but proceed to
    // cleaning up after the loop. The state flags in `bluetooth_state_flags`
    // continue to show the "starting" state until we've finished cleaning up.
    flags |= SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_STOPPED;
  }

  // We run the command processing loop as long as the stack has not been stopped
  uint8_t next_evt_index_to_check = 0;
  while ((flags & SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_STOPPED) == 0) {
    // Command needs to be sent to Bluetooth stack
    if (flags & SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_COMMAND_WAITING) {
      const void *command_buf = (const void *)current_command_buf;
      void *response_buf = (void *)current_response_buf;
      size_t response_buf_size = current_response_buf_size;
      current_command_execution_status = sli_bgapi_cmd_handler_delegate(current_bgapi_handler,
                                                                        command_buf,
                                                                        response_buf,
                                                                        response_buf_size);
      flags &= ~SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_COMMAND_WAITING;
      osSemaphoreRelease(bgapi_response_semaphore_id);
    }

    //Run Bluetooth stack. Pop the next event for application
    sl_bt_run();
    if (flags & SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_EVENT_HANDLED) {
      bool event_popped = false;
      sl_status_t status = SL_STATUS_OK;
      for (uint8_t i = 0; i < NUM_BGAPI_DEVICES; i++) {
        // Try to get an event of a device type that was _not_ handled previously, in order to
        // prevent one device type from "starving" the other device type(s).
        // For example, Bluetooth scan reports could flood in so fast that we'd never process
        // BT Mesh events, if we always checked for Bluetooth events first.
        uint8_t evt_index = (next_evt_index_to_check + i) % NUM_BGAPI_DEVICES;
        const bgapi_device_type *dev = &bgapi_device_table[evt_index];
        if (dev->event_pending_fn()) {
          status = dev->pop_event_fn((sl_bt_msg_t*) &bluetooth_evt_instance);
          if (status == SL_STATUS_OK) {
            // Next round, start the search from the next event type (wrapping around if needed)
            next_evt_index_to_check = (evt_index + 1) % NUM_BGAPI_DEVICES;
            event_popped = true;
          }
          break;
        }
      }
      if (status != SL_STATUS_OK) {
        continue;
      }
      if (event_popped) {
        // Always clear the local flag once an event was popped. The flag gets
        // set when the event has been handled and the corresponding thread
        // event flag is signalled by `sli_bt_rtos_set_event_handled()`.
        flags &= ~SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_EVENT_HANDLED;

#if defined(SL_CATALOG_BTMESH_PRESENT)
        sl_btmesh_bgapi_listener((sl_bt_msg_t*) &bluetooth_evt_instance);
#endif

#if defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
        // Publish the event to Event System IPC
        sl_status_t publish_status = sli_bt_publish_to_event_system((sl_bt_msg_t*) &bluetooth_evt_instance);
        if (publish_status != SL_STATUS_OK) {
          // Failed to publish the event. This is potentially fatal, but there
          // is not much we can do here. Assert in a debug build.
          EFM_ASSERT(0);
        }
#else // defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
        // Signal the event handler task to deliver the event
        set_bluetooth_rtos_thread_event_flags(&thread_event_handler,
                                              SLI_BT_RTOS_EVENT_THREAD_FLAG_EVENT_WAITING);
#endif // defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
      }
    }

    uint32_t timeout = sli_bt_can_sleep_ticks();
    if (timeout == 0 && (flags & SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_EVENT_HANDLED)) {
      continue;
    }

    // Wait for any flag to get set
    flags |= wait_bluetooth_rtos_thread_event_flags(&thread_bluetooth);
    if ((flags & SLI_BT_RTOS_EVENT_FLAG_ERROR) != 0) {
      // In case of error, reset the flags and continue
      flags = SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_EVENT_HANDLED;
      continue;
    }
  }

  // De-initialize the host stack BGAPI device
  sli_bt_deinit_bgapi_device();

  // De-initialize RTOS adaptation except this thread itself
  deinit_rtos_adaptation();

  // In case we've failed to start, we will directly deliver a system error
  // event to the application in the code below. To allow the application to
  // retry when it gets the error event, we need to clean all global state first
  // and finish the cleanup locally in the current thread when the event handler
  // returns. We take ownership of all remaining RTOS objects (this thread and
  // its wakeup semaphore) and clear the state to indicate the kernel is started
  // but the stack is stopped.
  thread_bluetooth.thread_id = NULL;
  (void) osSemaphoreDelete(thread_bluetooth.wakeup_semaphore_id);
  thread_bluetooth.wakeup_semaphore_id = NULL;
  bluetooth_state_flags = SLI_BT_RTOS_STATE_FLAG_KERNEL_STARTED;

  // Generate the right event to indicate the stack is not running
  if (start_status != SL_STATUS_OK) {
    // We failed to start the stack. Generate a system error event.
    uint32_t evt_len = sizeof(bluetooth_evt_instance.data.evt_system_error);
    bluetooth_evt_instance.header = sl_bt_evt_system_error_id | (evt_len << 8);
    bluetooth_evt_instance.data.evt_system_error.reason = (uint16_t) start_status;
    bluetooth_evt_instance.data.evt_system_error.data.len = 0;
  } else {
    // The stack started successfully, so the stopping has been a deliberate
    // request with the on-demand start feature. Generate the "system stopped"
    // event and deliver it directly to the application with a function call
    // from this thread. Note that the "system stopped" event has no
    // event-specific fields, so only the header needs to be set.
    bluetooth_evt_instance.header = sl_bt_evt_system_stopped_id;
  }

  // Make the event visible in BGAPI Trace, if included in the build
#if defined(SL_CATALOG_BGAPI_TRACE_PRESENT)
  sli_bgapi_trace_output_message(sli_bgapi_trace_message_type_event,
                                 bluetooth_evt_instance.header,
                                 (const void *)&bluetooth_evt_instance.data);
#endif

  // Publish via event system or call the application directly
#if defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
  sli_bt_publish_to_event_system((sl_bt_msg_t*) &bluetooth_evt_instance);
#else
  sl_bt_process_event((sl_bt_msg_t*) &bluetooth_evt_instance);
#endif

  // Finally terminate this thread itself
  (void) osThreadTerminate(osThreadGetId());
}

static void linklayer_thread(void *p_arg)
{
  // The input parameter is our thread structure
  sli_bt_rtos_thread_t *thread = (sli_bt_rtos_thread_t *) p_arg;

  while (1) {
    // Wait for any flag to get set. Only the link layer flag is expected in
    // this thread.
    uint32_t flags = wait_bluetooth_rtos_thread_event_flags(thread);
    EFM_ASSERT(flags == SLI_BT_RTOS_LINKLAYER_THREAD_FLAG_WAKEUP);

    // Handle the event
    sl_bt_priority_handle();
  }
}

// Event task, this calls the application code
#if !defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
static void event_handler_thread(void *p_arg)
{
  // The input parameter is our thread structure
  sli_bt_rtos_thread_t *thread = (sli_bt_rtos_thread_t *) p_arg;

  while (1) {
    // Wait for any flag to get set. Only the "event waiting" flag is expected
    // in this thread.
    uint32_t flags = wait_bluetooth_rtos_thread_event_flags(thread);
    EFM_ASSERT(flags == SLI_BT_RTOS_EVENT_THREAD_FLAG_EVENT_WAITING);

    switch (SL_BGAPI_MSG_DEVICE_TYPE(bluetooth_evt_instance.header)) {
      case sl_bgapi_dev_type_bt:
        sl_bt_process_event((sl_bt_msg_t*) &bluetooth_evt_instance);
        break;
#if defined(SL_CATALOG_BTMESH_PRESENT)
      case sl_bgapi_dev_type_btmesh:
        sl_btmesh_process_event((sl_btmesh_msg_t*) &bluetooth_evt_instance);
        break;
#endif
      default:
        // This should not be possible
        EFM_ASSERT(0);
        break;
    }

    // Signal the host stack thread that the event has been handled
    set_bluetooth_rtos_thread_event_flags(&thread_bluetooth,
                                          SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_EVENT_HANDLED);
  }
}
#endif

//hooks for API
//called from tasks using Bluetooth API
sl_status_t sli_bt_cmd_handler_rtos_delegate(sl_bgapi_handler handler,
                                             const void *command_buf,
                                             void *response_buf,
                                             size_t response_buf_size)
{
  if (osThreadGetId() == thread_bluetooth.thread_id) {
    // If we're already in the Bluetooth stack thread, the BGAPI command will be handled as a direct
    // function call; as opposed to signalling ourselves that there's a command waiting, like what
    // would happen in all other threads
    return sli_bgapi_cmd_handler_delegate(handler, command_buf, response_buf, response_buf_size);
  }

  // We're starting a new command. The response semaphore cannot have an
  // available token yet.
  EFM_ASSERT(osSemaphoreGetCount(bgapi_response_semaphore_id) == 0);

  // Remember the command we're about to execute in the Bluetooth stack thread
  current_bgapi_handler = handler;
  current_command_buf = command_buf;
  current_response_buf = response_buf;
  current_response_buf_size = response_buf_size;
  current_command_execution_status = SL_STATUS_IN_PROGRESS;

  // Command structure is filled, notify the stack
  set_bluetooth_rtos_thread_event_flags(&thread_bluetooth,
                                        SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_COMMAND_WAITING);

  // Wait for response
  osSemaphoreAcquire(bgapi_response_semaphore_id, osWaitForever);

  return current_command_execution_status;
}

// Obtain a buffer that can be used to execute BGAPI or user commands.
sl_status_t sl_bgapi_obtain_message_buffer(size_t max_payload_size,
                                           void **buffer)
{
  return sl_memory_alloc(SL_BGAPI_MSG_HEADER_LEN + max_payload_size,
                         BLOCK_TYPE_LONG_TERM,
                         buffer);
}

// Release a buffer that was obtained with @ref sl_bgapi_obtain_message_buffer.
void sl_bgapi_release_message_buffer(void *buffer)
{
  sl_memory_free(buffer);
}

// Used internally by BGAPI to lock the BGAPI for exclusive access
sl_status_t sli_bgapi_lock(uint32_t command_hdr)
{
  // The BGAPI is not allowed to be called from an ISR
  if (CORE_InIrqContext()) {
    return SL_STATUS_ISR;
  }

  // If the kernel has not been started yet, the BGAPI command call must be a
  // call from `app_init()` to start the Bluetooth stack. The init is
  // single-threaded and we can safely proceed without the lock.
  if ((bluetooth_state_flags & SLI_BT_RTOS_STATE_FLAG_KERNEL_STARTED) == 0) {
    return SL_STATUS_OK;
  }

  // If the calling task is the one that will process the command, return
  // without locking
  if (osThreadGetId() == thread_bluetooth.thread_id) {
    return SL_STATUS_IS_OWNER;
  }

  // Bluetooth locking is not command-specific. Always acquire the singleton
  // BGAPI mutex.
  (void) command_hdr;
  osStatus_t ret = osMutexAcquire(bgapi_mutex_id, osWaitForever);
  return os2sl_status(ret);
}

// Used internally by BGAPI to release the lock obtained by @ref sli_bgapi_lock
void sli_bgapi_unlock(uint32_t command_hdr)
{
  // If the kernel has not been started yet, the lock has not been obtained and
  // the unlock is ignored
  if ((bluetooth_state_flags & SLI_BT_RTOS_STATE_FLAG_KERNEL_STARTED) == 0) {
    return;
  }

  // If the calling task is the one that processed the command, the lock has not
  // been obtained and the unlock is ignored
  if (osThreadGetId() == thread_bluetooth.thread_id) {
    return;
  }

  // Bluetooth locking is not command-specific. Always release the singleton
  // BGAPI mutex that we acquired.
  (void) command_hdr;
  (void) osMutexRelease(bgapi_mutex_id);
}

// Called by system initialization when kernel is starting.
void sli_bt_rtos_adaptation_kernel_start()
{
  bluetooth_state_flags |= SLI_BT_RTOS_STATE_FLAG_KERNEL_STARTED;
}

// Start the Bluetooth RTOS adaptation
sl_status_t sli_bt_rtos_adaptation_start(void)
{
  // If the Bluetooth stack is stopping, we cannot accept a new request to start
  // the stack until the previous stopping has finished
  if (bluetooth_state_flags & SLI_BT_RTOS_STATE_FLAG_STOPPING) {
    return SL_STATUS_INVALID_STATE;
  }

  // If the stack is already starting or started, the new request to start
  // requires no actions
  if (bluetooth_state_flags & (SLI_BT_RTOS_STATE_FLAG_STARTING | SLI_BT_RTOS_STATE_FLAG_STARTED)) {
    return SL_STATUS_OK;
  }

  // We have committed to starting the stack and we expect
  // `bluetooth_thread()` to continue the start-up.
  bluetooth_state_flags |= SLI_BT_RTOS_STATE_FLAG_STARTING;

  // Signal the Bluetooth thread to start the stack
  set_bluetooth_rtos_thread_event_flags(&thread_bluetooth,
                                        SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_START);
  return SL_STATUS_OK;
}

// Prepare for stopping the Bluetooth RTOS adaptation
void sli_bt_rtos_adaptation_prepare_to_stop(void)
{
  // This function is called in the context of the Bluetooth thread when
  // processing the command sl_bt_system_stop_bluetooth(). The BGAPI lock is
  // already held when we enter this function and we can safely manipulate the
  // state flags. The BGAPI command handling verifies the current state and
  // calls this function only when it's really time to begin stopping. We can
  // simply always set the flag.
  bluetooth_state_flags |= SLI_BT_RTOS_STATE_FLAG_STOPPING;

  // We stop processing BGAPI commands via the RTOS adaptation, as the stack has
  // already committed to stopping. The BGAPI device has changed to a state
  // where only commands that are available when the stack is stopped are
  // reachable if the application calls a BGAPI command. It is therefore safe to
  // handle BGAPI commands via the default delegate until the stack is
  // restarted.
  sli_bgapi_set_cmd_handler_delegate(sli_bgapi_cmd_handler_delegate);
}

// Stop the Bluetooth RTOS adaptation
void sli_bt_rtos_adaptation_stop(void)
{
  // Set the event flag to indicate that the stack has been stopped. The
  // Bluetooth thread will cleanup and exit.
  set_bluetooth_rtos_thread_event_flags(&thread_bluetooth,
                                        SLI_BT_RTOS_BLUETOOTH_THREAD_FLAG_STOPPED);
}

sl_status_t sl_bt_bluetooth_pend()
{
  osStatus_t ret = osMutexAcquire(bluetooth_mutex_id, osWaitForever);
  return os2sl_status(ret);
}

sl_status_t sl_bt_bluetooth_post()
{
  osStatus_t ret = osMutexRelease(bluetooth_mutex_id);
  return os2sl_status(ret);
}

const sl_bt_msg_t* sl_bt_rtos_get_event()
{
  return (const sl_bt_msg_t*) &bluetooth_evt_instance;
}
