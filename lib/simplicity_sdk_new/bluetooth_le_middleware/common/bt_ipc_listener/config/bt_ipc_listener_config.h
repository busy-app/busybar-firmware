/***************************************************************************//**
 * @file
 * @brief Bluetooth IPC Listener Configuration header
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef BT_IPC_LISTENER_CONFIG_H
#define BT_IPC_LISTENER_CONFIG_H

/***********************************************************************************************//**
 * @addtogroup bt_ipc_listener
 * @{
 **************************************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <o BT_IPC_LISTENER_EVENT_QUEUE_SIZE> Event queue size
// <i> Default: 2
// <i> Define how many events the queue can store at once before processing.
#define BT_IPC_LISTENER_EVENT_QUEUE_SIZE             2

// <o BT_IPC_LISTENER_EVENT_HANDLER_TASK_PRIORITY> Event handler task priority
// <i> Default: 50 (CMSIS-RTOS2 osPriorityRealtime2)
// <i> Define the priority of the event handler task. This must be a
// <i> valid priority value from CMSIS-RTOS2 osPriority_t definition.
#define BT_IPC_LISTENER_EVENT_HANDLER_TASK_PRIORITY  50

// <o BT_IPC_LISTENER_EVENT_HANDLER_STACK_SIZE> Event handler task stack size in bytes
// <i> Default: 1000
// <i> Define the stack size of the Bluetooth event handler task. The value is
// <i> in bytes and will be word aligned when it is applied at the task creation.
#define BT_IPC_LISTENER_EVENT_HANDLER_STACK_SIZE     1000

// <<< end of configuration section >>>

/** @} (end addtogroup bt_ipc_listener) */
#endif // BT_IPC_LISTENER_CONFIG_H
