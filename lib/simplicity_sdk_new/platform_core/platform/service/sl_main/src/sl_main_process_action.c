/***************************************************************************//**
 * @file
 * @brief Main process action.
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
#include "sl_event_handler.h"

/******************************************************************************
 * @brief Action(s) to perform periodically from the main loop.
 *
 * @note (1) For baremetal applications, this function is called from the main function
 *           in an infinite loop.
 *
 * @note (2) For RTOS applications, after configuring the start task to continue
 *           executing, this function is called from the main function in an infinite
 *           loop.
 *****************************************************************************/
void sl_main_process_action(void)
{
  sli_platform_process_action();
  sli_service_process_action();
  sli_stack_process_action();
  sli_internal_app_process_action();
}
