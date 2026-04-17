/***************************************************************************//**
 * @file
 * @brief Bluetooth Mesh RTOS configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in a
 *    product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_BTMESH_SI91X_RTOS_CONFIG_H
#define SL_BTMESH_SI91X_RTOS_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Priority Configuration for Bluetooth Mesh RTOS Tasks

// <o SL_BTMESH_RTOS_TASK_PRIORITY> Bluetooth mesh stack task priority
// <i> Default: 51 (CMSIS-RTOS2 osPriorityRealtime3)
// <i> Define the priority of the Bluetooth mesh stack task. This must be a
// <i> valid priority value from CMSIS-RTOS2 osPriority_t definition.
#define SL_BTMESH_RTOS_TASK_PRIORITY     (51)

// <o SL_BTMESH_RTOS_TASK_STACK_SIZE> Bluetooth mesh stack task stack size in bytes
// <i> Default: 3000
// <i> Define the stack size of the Bluetooth mesh stack task. The value is in bytes
// <i> and will be word aligned when it is applied at the task creation.
#define SL_BTMESH_RTOS_TASK_STACK_SIZE   (3000)

// <o SL_BTMESH_RTOS_EVENT_HANDLER_TASK_PRIORITY> Bluetooth mesh event handler task priority
// <i> Default: 50 (CMSIS-RTOS2 osPriorityRealtime2)
// <i> Define the priority of the Bluetooth mesh event handler task. This must be a
// <i> valid priority value from CMSIS-RTOS2 osPriority_t definition. The event
// <i> handler task must have the lowest priority of these two Bluetooth RTOS tasks.
#define SL_BTMESH_RTOS_EVENT_HANDLER_TASK_PRIORITY  (50)

// <o SL_BTMESH_RTOS_EVENT_HANDLER_TASK_STACK_SIZE> Bluetooth mesh event handler task stack size in bytes
// <i> Default: 1000
// <i> Define the stack size of the Bluetooth mesh event handler task. The value is in bytes
// <i> and will be word aligned when it is applied at the task creation.
#define SL_BTMESH_RTOS_EVENT_HANDLER_TASK_STACK_SIZE     (1000)

// </h> End Priority Configuration for Bluetooth Mesh RTOS Tasks

// <<< end of configuration section >>>

#endif // SL_BTMESH_SI91X_RTOS_CONFIG_H
