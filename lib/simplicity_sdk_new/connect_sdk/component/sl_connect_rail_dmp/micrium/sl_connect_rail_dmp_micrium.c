/***************************************************************************//**
 * @file sl_connect_rail_dmp_micrium.c
 * @brief Source file for the rail app task initialization and handling in
 *        the Silicon Labs Connect RAIL DMP application with MicriumOS.
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_connect_rail_dmp.h"
#include "os_cfg.h"
#include "os.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/* OS Event Flag ID */
#define RAIL_FLAG  ((OS_FLAGS)0x01)
// RAIL Application task
#define RAIL_APP_TASK_PRIO         6U
// RAIL Application task stack size
#define RAIL_APP_TASK_STACK_SIZE   (1024 / sizeof(CPU_STK))

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// RAIL Application task buffer and stack allocation
static CPU_STK rail_app_task_stack[RAIL_APP_TASK_STACK_SIZE];
static OS_TCB  rail_app_task_TCB;

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// OS Event Flag Group
OS_FLAG_GRP  rail_event_flags;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void rail_app_task_init(void)
{
  RTOS_ERR err;
  // Create the RAIL Application task
  OSTaskCreate(&rail_app_task_TCB,
               "RAIL App Task",
               rail_app_task,
               0U,
               RAIL_APP_TASK_PRIO,
               &rail_app_task_stack[0U],
               (RAIL_APP_TASK_STACK_SIZE / 10U),
               RAIL_APP_TASK_STACK_SIZE,
               0U,
               0U,
               0U,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);

  // Initialize the flag group for the rail task
  OSFlagCreate(&rail_event_flags, "Rail. flags", (OS_FLAGS) 0, &err);
}

void handle_os_pending(void)
{
  RTOS_ERR err;
  OSFlagPend(&rail_event_flags,
             RAIL_FLAG,
             (OS_TICK) 0,
             OS_OPT_PEND_BLOCKING       \
             + OS_OPT_PEND_FLAG_SET_ANY \
             + OS_OPT_PEND_FLAG_CONSUME,
             NULL,
             &err);
}

void post_os_flag(void)
{
  RTOS_ERR err;
  OSFlagPost(&rail_event_flags, RAIL_FLAG, OS_OPT_POST_FLAG_SET, &err);
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
