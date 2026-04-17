/***************************************************************************//**
 * @file sl_connect_rail_dmp.h
 * @brief Header file for the proprietary task initialization and handling in
 *        the Silicon Labs Connect RAIL DMP application.
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

/**
 * @addtogroup connect_rail_dmp
 * @brief Sets of API for Connect RAIL Dynamic Multi Protocol component
 *
 * The Connect-RAIL DMP component is designed to facilitate communication
 * using the RAIL (Radio Abstraction Interface Layer) within the Connect framework.
 * This component allows for both transmission and reception of packets using RAIL.
 *
 * @{
 */

#ifndef SL_CONNECT_RAIL_DMP_H
#define SL_CONNECT_RAIL_DMP_H

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------

#include "sl_rail_types.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/**
 * @brief Scheduler priority for sending operations.
 *
 * Priority level for scheduling send operations within the application.
 * A lower value indicates a higher priority.
 * (should be lower than BLE priority and higher than CONNECT priority)
 */
#define SL_RAIL_SCHEDULER_PRIORITY_SEND 50

/**
 * @brief Scheduler priority for receive operations.
 *
 * Priority level for scheduling receive operations within the application.
 * A lower value indicates a higher priority.
 * (should be lower than BLE priority and higher than CONNECT priority)
 */
#define SL_RAIL_SCHEDULER_PRIORITY_RECEIVE 100

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Declarations
// -----------------------------------------------------------------------------
/***************************************************************************//**
 * Initialize the RAIL DMP application task.
 ******************************************************************************/
void rail_app_task_init(void);

/***************************************************************************//**
 * RAIL DMP application task.
 *
 * @param p_arg Pointer to the task argument.
 ******************************************************************************/
void rail_app_task(void *p_arg);

/***************************************************************************//**
 * Handle the pending OS events.
 ******************************************************************************/
void handle_os_pending(void);

/***************************************************************************//**
 * Post the OS flag.
 ******************************************************************************/
void post_os_flag(void);

/***************************************************************************//**
 * Calculate the transaction time.
 ******************************************************************************/
sl_rail_time_t calculate_transaction_time(sl_rail_handle_t rail_handle);

#endif //SL_CONNECT_RAIL_DMP_H
