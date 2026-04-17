/***************************************************************************//**
 * @brief Adaptation for running Bluetooth Mesh in RTOS on the Si91X
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

#ifndef SL_BTMESH_SI91X_RTOS_ADAPTATION_H
#define SL_BTMESH_SI91X_RTOS_ADAPTATION_H

#include "sl_btmesh_api.h"

/***************************************************************************//**
 * @addtogroup bluetooth_rtos_adaptation
 * @{
 *
 * @brief Bluetooth RTOS adaptation
 *
 * The Bluetooth RTOS adaptation component implements the relevant interfaces
 * for running the Bluetooth stack in an RTOS. When initialized in @ref
 * sl_bt_rtos_init(), the component creates the required RTOS tasks for the
 * Bluetooth stack itself, and an event task that is used to deliver
 * sl_bt_process_event() and sl_bt_on_event() callbacks to the application.
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
 * 2. The stack uses @ref sl_bgapi_lock() and @ref sl_bgapi_unlock() to
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
 * @brief Hooks for API, called from tasks using Bluetooth API
 */
void sli_bt_cmd_handler_rtos_delegate(uint32_t header, sl_bgapi_handler handler, const void* payload);

/**
 * @brief Called by system initialization when kernel is starting.
 */
void sli_bt_rtos_adaptation_kernel_start();

/**
 * @brief Mutex functions for using Bluetooth from multiple tasks
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
 * system buffer followed by a call to @ref sl_bt_advertiser_set_long_data() to
 * set that data to an advertiser set. To synchronize access to the shared
 * system buffer, the application would need to lock by calling @ref
 * sl_bt_bluetooth_pend() before @ref sl_bt_system_data_buffer_write(), and
 * release the lock by calling @ref sl_bt_bluetooth_post() after @ref
 * sl_bt_advertiser_set_long_data().
 *
 * @return SL_STATUS_OK if mutex has been obtained
 */
sl_status_t sl_bt_bluetooth_pend();

/**
 * @brief Mutex functions for using Bluetooth from multiple tasks
 *
 * See @ref sl_bt_bluetooth_pend() for description of how an application needs
 * to use the locking to guarantee thread-safety.
 *
 * @return SL_STATUS_OK the mutex has been released
 */
sl_status_t sl_bt_bluetooth_post();

/**
 * @brief Initialize Bluetooth for running in RTOS.
 * @return SL_STATUS_OK if succeeds, otherwise error
 */
sl_status_t sl_bt_rtos_init();

/** @} end bluetooth_rtos_adaptation */

#endif //SL_BTMESH_SI91X_RTOS_ADAPTATION_H
