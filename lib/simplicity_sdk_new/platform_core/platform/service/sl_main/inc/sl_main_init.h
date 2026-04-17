/***************************************************************************//**
 * @file
 * @brief Main Initialization.
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
#ifndef _SL_MAIN_INIT_H
#define _SL_MAIN_INIT_H

/***************************************************************************//**
 * @addtogroup sl_main System Setup (sl_main)
 * @brief System Setup (sl_main)
 * @details
 * ##Overview
 *
 * The sl_main component manages the initialization phase of SiSDK embedded
 * firmware. This component aggregates, organizes and executes all the different
 * steps required for initializing the firmware so that every component in your
 * .slcp file and their dependencies are in a ready state for proper execution.
 * Application developers may also initialize different parts of their application
 * by re-implementing hooks that are called at key points within the sl_main
 * initialization sequence.
 *
 * ##Configuration
 *
 * The only configurations available to sl_main are for the start task in:
 *   - \b sl_main_start_task_config.h
 *
 * This header files offer a few configurations for sl_main when using an RTOS.
 * They use the CMSIS Configuration Wizard Annotations that can be rendered
 * by Simplicity Studio to set graphically the configuration settings value.
 *
 * Here is a list of the available configurations:
 *
 * \a SL_MAIN_START_TASK_STACK_SIZE_BYTES allows to configure the stack size for
 *    the start task. The default value of 4096 bytes.
 *
 * \a SL_MAIN_ENABLE_START_TASK_PRIORITY_CHANGE allows the start task to change it's
 *    priority after the initialization process is done.
 *
 *     @note The new priority will take effect just before the call to `app_init()`.
 *           The start task will always start at maximum priority (`osPriorityRealtime7`)
 *           regardless of this configuration option.
 *
 * \a SL_MAIN_START_TASK_PRIORITY Sets the priority of the start task after initialization is done.
 *
 *     @note This configuration is only available when
 *           SL_MAIN_ENABLE_START_TASK_PRIORITY_CHANGE is set to 1.
 *           The start task will always start at maximum priority (`osPriorityRealtime7`)
 *           regardless of this configuration option.
 *
 * ### Main Init
 * Main Init provides a function for initializing the system and the products:
 *
 *   - sl_main_init().
 *
 * This function calls a hardcoded sequence of functions that are executed in order.
 *
 *   - sl_main_second_stage_init().
 *
 * This function is called the second stage init, it calls a set of functions
 * that are automatically generated and located in `autogen/sl_event_handler.c`.
 * Handlers can be registered for the following events using the Event Handler
 * API provided by the Event Handler component:
 *
 *   - platform_init      -> sl_platform_init()
 *   - driver_init        -> sl_driver_init()
 *   - service_init       -> sl_service_init()
 *   - stack_init         -> sl_stack_init()
 *   - internal_app_init  -> sl_internal_app_init()
 *
 * These events are fired in the order listed above when `sl_main_init()`
 * is called. If an RTOS is included in the project the sl_main_second_stage_init
 * function will be called after the OS kernel is started.
 *
 * ### Main Kernel
 *
 * Main Kernel component provides an API to:
 *
 *    - sl_main_kernel_start(): Starts the kernel
 *    - sl_main_start_task_should_continue(): Determines if the start task should
 *                                            continue after the initialization
 *                                            process is done. (templated main only)
 *
 *  sl_main_kernel_start() function calls a function that is automatically generated
 *  and located in `$autogen/sl_event_handler.c`. Handlers can be registered
 *  for the following events using the Event Handler API provided by the
 *  Event Handler component:
 *
 *    - kernel_start -> sl_kernel_start()
 *
 *  The event is fired when `sl_main_kernel_start()` is called.
 *
 *  sl_main_kernel_start() is called automatically by the main re-target
 *  from main_retarget.c. Users should not call this API themselves.
 *
 *  sl_main_start_task_should_continue() returns false by default to stop the start
 *  task after the initialization process is done. It can be redefined by the user
 *  to allow the start task to continue executing after the initialization process is
 *  done. Keep in mind that this function is called in the templated main function
 *  provided by sl_main, if you removed the call to `sl_main_start_task_should_continue`
 *  in your main function then re-implementing this function will have no effect.
 *
 *      @note By default, when using Micrium OS, the start task's resources - such as
 *            its stack and Task Control Block (TCB) - are not released upon task exit,
 *            which can result in a memory leak. To avoid this, enable Micrium OS garbage
 *            collection. Note that enabling garbage collection may significantly increase
 *            both RAM and flash usage.
 *
 *  The following code snippet shows how to implement this function to allow the
 *  start task to continue:
 *  @code{.c}
 *  bool sl_main_start_task_should_continue(void)
 *  {
 *      return true; // start task will continue after initialization.
 *  }
 *  @endcode
 *
 * ### Main Process Action
 *
 * Main Process Action component provides a function for running
 * the products from a super loop:
 *
 *   - sl_main_process_action().
 *
 * This function calls a set of functions that are automatically generated
 * and located in `$autogen/sl_event_handler.c`. Handlers can be registered
 * for the following events using the Event Handler API provided by the
 * Event Handler component:
 *
 *   - platform_process_action     -> sli_platform_process_action()
 *   - service_process_action      -> sli_service_process_action()
 *   - stack_process_action        -> sli_stack_process_action()
 *   - internal_app_process_action -> sli_internal_app_process_action()
 *
 * These events are fired in the order listed above when `sl_main_process_action()`
 * is called.
 *
 * Usage example in baremetal application:
 *
 * @code{.c}
 * #include "sl_component_catalog.h"
 * #include "sl_main_init.h"
 *
 * #if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
 * #include "sl_power_manager.h"
 * #endif
 *
 * #include "sl_main_process_action.h"
 *
 * int main(void)
 * {
 *   // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
 *   // Note that if the kernel is present, the start task will be started and software
 *   // component initialization will take place there.
 *   sl_main_init();
 *
 *   // User provided code
 *   app_init();
 *
 *   while (1) {
 *     // Silicon Labs components process action routine
 *     // must be called from the super loop.
 *     sl_main_process_action();
 *
 *     // User provided code. Application process.
 *     app_process_action();
 *
 * #if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
 *     // Let the CPU go to sleep if the system allows it.
 *     sl_power_manager_sleep();
 * #endif
 *   }
 * #endif // SL_CATALOG_KERNEL_PRESENT
 * }
 * @endcode
 * In RTOS applications, this API is never called:
 *
 * @code{.c}
 * #include "sl_main_init.h"
 * #include "sl_main_kernel.h"
 *
 * int main(void)
 * {
 *   // In case of RTOS, only the second stage (initialize stage) would be performed.
 *   sl_main_second_stage_init();
 *
 *   // User provided code
 *   app_init();
 *
 *   while (1) {

 *   }
 * #endif // SL_CATALOG_KERNEL_PRESENT
 * }
 * @endcode
 * @{
 *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * @brief User-defined function for pre-clock app initialization.
 *
 * @details This function is called from sli_main_init():
 * - Before essential infrastructure components are initialized
 * (memory management, clock, oscillators, interrupt management, etc.).
 *****************************************************************************/
void app_init_pre_clock(void);

/******************************************************************************
 * @brief User-defined function for early app initialization.
 *
 * @details This function is called from sli_main_init():
 * - After essential infrastructure components are initialized
 * (memory management, clock, oscillators, interrupt management, etc.).
 * - Before initializing the kernel (if RTOS-based application)
 * - Before the call to sl_main_second_stage_init() doing the other SiSDK
 * modules initilization (e.g. drivers, services, stacks)
 * This function allows the user to perform early initialization required by
 * his application before other SiSDK modules initializations.
 *****************************************************************************/
void app_init_early(void);

/******************************************************************************
 * @brief User-defined function for application initialization after platform 
 * and initialization.
 *
 * @details This function is called from sl_main_second_stage_init():
 * - After sl_platform_init(), sl_driver_init(), and sl_service_init() complete
 * - Before sl_stack_init() is called
 * This function allows the user to perform initialization that depends on
 * platform components, drivers, and services being ready, but needs to run
 * before stacks are initialized.
 *****************************************************************************/
void app_init_post_platform(void);

/******************************************************************************
 * @brief User-defined function for app initialization.
 *
 * @details This function is called from main():
 * - After all SiSDKs modules have been initialized (e.g. memory management,
 * clock, oscillators, interrupt management, drivers, services, stacks).
 * - After initializing and starting the kernel (if RTOS-based application)
 * This function allows the user to perform any initialization required by his
 * application after all SiSDK modules initializations.
 *****************************************************************************/
void app_init(void);

/******************************************************************************
 * @brief User-defined function to process a baremetal action periodically.
 *
 * @details Only used when the firmware type is baremetal.
 * This function will be called every iteration of the main loop.
 *****************************************************************************/
void app_process_action(void);

/******************************************************************************
 * @brief Main entry function calling key platform modules initialization functions.
 *
 * @details This function allows the application to perform an early
 * initialization from within the function app_init_early().
 *****************************************************************************/
void sl_main_init(void);

/******************************************************************************
 * @brief Internal function used to aggregate driver, services stacks and internal
 * apps initialization function.
 *
 * @details In a baremetal application, this function is called from
 * sl_main_init(). If a kernel is present, this function will be automatically
 * called  after Silicon Labs platform and stacks components are initialized.
 * These components init functions will be executed from the start task context.
 *****************************************************************************/
void sl_main_second_stage_init(void);

#ifdef __cplusplus
}
#endif

/** @} (end addtogroup sl_main) */

#endif // _SL_MAIN_INIT_H
