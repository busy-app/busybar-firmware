/***************************************************************************//**
 * @file app_init.c
 * @brief Application init
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
#include <stdio.h>

#include "sl_assert.h"
#include "sl_main_init.h"
#include "cmsis_os2.h"
#include "sl_cmsis_os2_common.h"
#include "app.h"
// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
#define APP_TASK_STACK_SIZE         (500U)
#define APP_SERVICE_TASK_STACK_SIZE (500U)
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void app_init(void)
{
  // Creating App main thread
  const osThreadAttr_t app_task_attr = {
    .name       = "AppMain",
    .attr_bits  = osThreadDetached,
    .cb_mem     = NULL,
    .cb_size    = 0,
    .stack_mem  = NULL,
    .stack_size = (APP_TASK_STACK_SIZE * sizeof(void *)) & 0xFFFFFFF8u,
    .priority   = osPriorityNormal,
    .tz_module  = 0,
    .reserved   = 0
  };

  osThreadId_t app_thr_id = osThreadNew(app_task,
                                        NULL,
                                        &app_task_attr);
  EFM_ASSERT(app_thr_id != NULL);
}

void app_service_task_init(void)
{
  // Creating service task thread
  const osThreadAttr_t service_task_attr = {
    .name       = "AppService",
    .attr_bits  = osThreadDetached,
    .cb_mem     = NULL,
    .cb_size    = 0,
    .stack_mem  = NULL,
    .stack_size = (APP_SERVICE_TASK_STACK_SIZE * sizeof(void *)) & 0xFFFFFFF8u,
    .priority   = osPriorityBelowNormal1,
    .tz_module  = 0,
    .reserved   = 0
  };

  osThreadId_t service_thr_id = osThreadNew(app_service_task,
                                            NULL,
                                            &service_task_attr);
  EFM_ASSERT(service_thr_id != NULL);
}
// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
