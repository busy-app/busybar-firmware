/***************************************************************************//**
 * @file
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

#ifndef SL_BT_RTOS_ADAPTATION_H
#define SL_BT_RTOS_ADAPTATION_H

#include "sl_bt_api.h"
#include "sli_bgapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup bluetooth_rtos_adaptation Bluetooth RTOS adaptation
 * @{
 *
 * @brief Bluetooth RTOS adaptation
 *
 * The Bluetooth RTOS adaptation component implements the relevant interfaces
 * for running the Bluetooth stack in an RTOS. The component creates the
 * required RTOS synchronization primitives and tasks for the Bluetooth stack
 * and handles the inter-process communication (IPC) required when the
 * application calls a BGAPI command.
 *
 * If the Bluetooth Event System IPC component (`bluetooth_event_system_ipc`) is
 * included in the application, Bluetooth events are delivered using the IPC
 * mechanism provided by the `event_system` component. See @ref
 * bluetooth_event_system for the description of that mechanism. In this
 * configuration no event task is created by the RTOS adaptation.
 *
 * If the Bluetooth Event System IPC component (`bluetooth_event_system_ipc`) is
 * not included in the application, the RTOS adaptation component creates an
 * event task that delivers the events to the application as direct callbacks to
 * sl_bt_process_event() and sl_bt_on_event() functions.
 *
 * To guarantee thread safety and avoid the risk of deadlocks, the Bluetooth
 * RTOS adaptation implementation makes the following promises with regard to
 * the locking mechanism provided in the API:
 *
 * 1. The stack will never directly call sl_bt_process_event() or
 *    sl_bt_on_event() from within the same callstack that is calling a command
 *    function. The callbacks always come from a processing loop in an event
 *    task created for this purpose.
 *
 * 2. The stack uses `sli_bgapi_lock()` and `sli_bgapi_unlock()` to
 *    synchronize the handling of individual BGAPI commands, and the application
 *    must never directly call these. Individual BGAPI commands are safe to be
 *    called from multiple threads without additional locking. See @ref
 *    sl_bt_bluetooth_pend() for description of when an application needs to use
 *    additional locking to guarantee atomicity across multiple commands.
 *
 * 3. The stack will never internally obtain the @ref sl_bt_bluetooth_pend()
 *    lock. It is safe for the application to obtain the lock also from within
 *    the sl_bt_on_event() callback.
 */

/**
 * @brief Make permanent memory allocations for Bluetooth RTOS adaptation.
 *
 * Do not call this function directly from the application. The Bluetooth host
 * stack components call this function automatically in the right stage of
 * initialization.
 *
 * @return SL_STATUS_OK if succeeds, otherwise error
 */
sl_status_t sli_bt_rtos_adaptation_permanent_allocation(void);

/**
 * @brief BGAPI component initialization function for Bluetooth RTOS adaptation
 *
 * Do not call this function directly from the application. The Bluetooth host
 * stack components call this function automatically in the right stage of
 * initialization.
 */
sli_bgapi_component_init_func_t sli_bt_rtos_adaptation_init;

/**
 * @brief Start the Bluetooth RTOS adaptation
 *
 * Do not call this function directly from the application. The Bluetooth host
 * stack components call this function automatically when it's time to start the
 * RTOS adaptation.
 *
 * @return SL_STATUS_OK if succeeds, otherwise error
 */
sl_status_t sli_bt_rtos_adaptation_start(void);

/**
 * @brief Prepare to stop the Bluetooth RTOS adaptation
 *
 * Do not call this function directly from the application. The Bluetooth host
 * stack components call this function automatically when it's time to prepare
 * for stopping the RTOS adaptation.
 */
void sli_bt_rtos_adaptation_prepare_to_stop(void);

/**
 * @brief Stop the Bluetooth RTOS adaptation
 *
 * Do not call this function directly from the application. The Bluetooth host
 * stack components call this function automatically when it's time to stop the
 * RTOS adaptation.
 */
void sli_bt_rtos_adaptation_stop(void);

/**
 * @brief BGAPI component de-init function for Bluetooth RTOS adaptation
 *
 * Do not call this function directly from the application. The Bluetooth host
 * stack components call this function automatically in the right stage of
 * de-initialization.
 */
sli_bgapi_component_deinit_func_t sli_bt_rtos_adaptation_deinit;

/**
 * @brief Obtain the Bluetooth host stack command lock
 *
 * Starting from Gecko SDK v3.1.2, all BGAPI command functions have automatic
 * locking to make them thread-safe. Using @ref sl_bt_bluetooth_pend() and @ref
 * sl_bt_bluetooth_post() is therefore no longer required for individual calls
 * to the BGAPI.
 *
 * The application only needs to use @ref sl_bt_bluetooth_pend() and @ref
 * sl_bt_bluetooth_post() to protect sections of code where multiple commands
 * need to be performed atomically in a thread-safe manner. This includes cases
 * such as using @ref sl_bt_system_data_buffer_write() to write data to the
 * system buffer followed by a call to @ref
 * sl_bt_extended_advertiser_set_long_data() to set that data to an advertiser
 * set. To synchronize access to the shared system buffer, the application would
 * need to lock by calling @ref sl_bt_bluetooth_pend() before @ref
 * sl_bt_system_data_buffer_write(), and release the lock by calling @ref
 * sl_bt_bluetooth_post() after @ref sl_bt_extended_advertiser_set_long_data().
 *
 * @return SL_STATUS_OK if mutex has been obtained
 */
sl_status_t sl_bt_bluetooth_pend();

/**
 * @brief Release the Bluetooth host stack command lock
 *
 * See @ref sl_bt_bluetooth_pend() for description of how an application needs
 * to use the locking to guarantee thread-safety.
 *
 * @return SL_STATUS_OK the mutex has been released
 */
sl_status_t sl_bt_bluetooth_post();

/** @cond DOXYGEN_INCLUDE_INTERNAL */

/**
 * @brief Hooks for API, called from tasks using Bluetooth API
 */
sli_bgapi_cmd_handler_delegate_t sli_bt_cmd_handler_rtos_delegate;

/**
 * @brief Called by Bluetooth stack to wake up the link layer task
 */
void sli_bt_rtos_ll_callback(void);

/**
 * @brief Called by Bluetooth stack to wake up the host stack task
 */
void sli_bt_rtos_stack_callback(void);

/**
 * @brief Called by system initialization when kernel is starting.
 */
void sli_bt_rtos_adaptation_kernel_start();

/**
 * @brief Called by Bluetooth Event System IPC to mark an event as handled
 */
void sli_bt_rtos_set_event_handled();

/** @endcond */ // DOXYGEN_INCLUDE_INTERNAL

/** @} end bluetooth_rtos_adaptation */

#ifdef __cplusplus
}
#endif

#endif //SL_BT_RTOS_ADAPTATION_H
