/***************************************************************************//**
 * @file sl_connect_rail_dmp_freertos.c
 * @brief Source file for the rail app task initialization and handling in
 *       the Silicon Labs Connect RAIL DMP application with FreeRTOS.
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
#include "sl_component_catalog.h"
#include "sl_connect_rail_dmp.h"
#ifdef SL_CATALOG_APP_ASSERT_PRESENT
#include "app_assert.h"
#endif
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

// Rail Application task
#define RAIL_APP_TASK_PRIO         6U
// Rail Application task stack size
#define RAIL_APP_TASK_STACK_SIZE   320U

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// RAIL Application task buffer and stack allocation
static StaticTask_t rail_app_task_buffer;
static StackType_t  rail_app_task_stack[RAIL_APP_TASK_STACK_SIZE];
// RAIL Application task max blocking time
static const TickType_t xMaxBlockTime = pdMS_TO_TICKS(1000);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// OS Event Flag Group
TaskHandle_t rail_task_notify = NULL;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/***************************************************************************//**
 * Initialize the RAIL DMP application task.
 ******************************************************************************/
void rail_app_task_init(void)
{
  TaskHandle_t rail_task_handle = NULL;
  // Create RAIL App Task without using any dynamic memory allocation
  rail_task_handle = xTaskCreateStatic(rail_app_task,
                                       "RAIL App Task",
                                       RAIL_APP_TASK_STACK_SIZE,
                                       NULL,
                                       RAIL_APP_TASK_PRIO,
                                       rail_app_task_stack,
                                       &rail_app_task_buffer);

  // Since rail_app_task_stack and rail_app_task_buffer parameters are not NULL,
  // it is impossible for rail_task_handle to be null. This check is for
  // rigorous example demonstration.
#ifdef SL_CATALOG_APP_ASSERT_PRESENT
  app_assert(rail_task_handle != NULL, "FreeRTOS task creation have failed\n");
#else
  while (rail_task_handle == NULL) {
  }
#endif
  //Save task address for notification
  rail_task_notify = rail_task_handle;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

void handle_os_pending(void)
{
  uint32_t ulNotificationValue;
  ulNotificationValue = ulTaskNotifyTake(pdFALSE, xMaxBlockTime);
  if ( ulNotificationValue == 1 ) {
    /* The transmission ended as expected. */
  } else {
    /* The call to ulTaskNotifyTake() timed out. */
  }
}

void post_os_flag(void)
{
  if (rail_task_notify != NULL) {
    vTaskNotifyGiveFromISR(rail_task_notify, false);
  }
}
