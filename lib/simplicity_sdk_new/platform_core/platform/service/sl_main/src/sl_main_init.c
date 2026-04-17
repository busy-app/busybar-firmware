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
#include <stdbool.h>
#include "sl_common.h"
#include "sl_event_handler.h"
#include "sl_assert.h"
#include "sl_event_handler.h"
#include "sl_main_init.h"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#if defined(SL_CATALOG_HAL_SYSTEM_PRESENT)
#include "sl_hal_system.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_DCDC_PRESENT)
#include "sl_device_init_dcdc.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_LFXO_PRESENT)
#include "sl_device_init_lfxo.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_HFRCO_PRESENT)
#include "sl_device_init_hfrco.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_HFXO_PRESENT)
#include "sl_device_init_hfxo.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_LFRCO_PRESENT)
#include "sl_device_init_lfrco.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_DPLL_PRESENT)
#include "sl_device_init_dpll.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_USBPLL_PRESENT)
#include "sl_device_init_usbpll.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_CLOCKS_PRESENT)
#include "sl_device_init_clocks.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_RFFPLL_PRESENT)
#include "sl_device_init_rffpll.h"
#endif

#if defined(SL_CATALOG_DEVICE_INIT_EMU_PRESENT)
#include "sl_device_init_emu.h"
#endif

#if defined(SL_CATALOG_MPA_MANAGER_PRESENT)
#include "sl_mpa_manager.h"
#endif

#if defined(SL_CATALOG_MPU_PRESENT)
#include "sl_mpu.h"
#endif

#include "sli_main_init_memory.h"

#if defined(SL_CATALOG_INTERRUPT_MANAGER_PRESENT)
#include "sl_interrupt_manager.h"
#endif

#if defined(SL_CATALOG_SLEEPTIMER_PRESENT)
#include "sl_sleeptimer.h"
#endif

#if (defined(SL_CATALOG_MEMORY_MANAGER_PRESENT) && !defined(SL_CATALOG_CPP_SUPPORT_PRESENT)) \
  || defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT) || defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)
#include "sl_memory_manager.h"
#endif

#if defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
#include "sl_clock_manager_init.h"
#endif

#if defined(SL_SI91X_SOC_MODE)
#include "system_si91x.h"
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_main_start_task_config.h"
#include "cmsis_os2.h"
#include "sli_main_kernel.h"
#endif

#ifdef SL_CATALOG_LEGACY_HAL_SOC_PRESENT
#include "hal.h"
#endif

#if defined(SL_CATALOG_METRIC_EVENT_HANDLER_PRESENT)
#include "sli_metric_event_handler.h"
#else
#define SLI_METRIC_EVENT_HANDLER_START()
#define SLI_METRIC_EVENT_HANDLER_SAVE(func)
#endif

/******************************************************************************
 * @brief User-defined function for pre-clock app initialization.
 *
 * @details This function is called from sli_main_init():
 * - Before essential infrastructure components are initialized
 * (memory management, clock, oscillators, interrupt management, etc.).
 *****************************************************************************/
SL_WEAK void app_init_pre_clock(void)
{
  // User defined.
}

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
SL_WEAK void app_init_early(void)
{
  // User defined.
}

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
SL_WEAK void app_init_post_platform(void)
{
  // User defined.
}

#if !defined(SL_CATALOG_SL_SYSTEM_COMPATIBILITY_PRESENT)
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
SL_WEAK void app_init(void)
{
  // User defined.
}
#endif

#if !defined(SL_CATALOG_SL_SYSTEM_COMPATIBILITY_PRESENT)
/******************************************************************************
 * @brief User-defined function to process a baremetal action periodically.
 *
 * @details Only used when the firmware type is baremetal.
 * This function will be called every iteration of the main loop.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  // User defined.
}
#endif

/******************************************************************************
 * @brief Internal function used to aggregate driver, services stacks and internal apps
 * initialization function.
 *
 * @details If a kernel is present, these components init functions will be
 * executed in the start task.
 *****************************************************************************/
void sl_main_second_stage_init(void)
{
  sl_platform_init();
  sl_driver_init();
  sl_service_init();
  app_init_post_platform();
  sl_stack_init();
  sl_internal_app_init();
#if defined(SL_CATALOG_KERNEL_PRESENT) && (SL_MAIN_ENABLE_START_TASK_PRIORITY_CHANGE == 1)
  osThreadSetPriority(osThreadGetId(), SL_MAIN_START_TASK_PRIORITY);
#endif
}

/******************************************************************************
 * @brief Core system components initialization.
 *
 * @details MCU and software components initialization  will be executed in
 * order to have a working system after exit. If an operating system is
 * selected, software components will be initialized just after the kernel
 * is started and before any other tasks are able to execute.
 *****************************************************************************/
void sl_main_init(void)
{
  SLI_METRIC_EVENT_HANDLER_START();

#if defined(SL_CATALOG_MEMORY_MANAGER_PRESENT) && !defined(SL_CATALOG_CPP_SUPPORT_PRESENT)
  sl_memory_init();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_memory_init");
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_PSRAM_PRESENT)
  sl_memory_init_psram();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_memory_init_psram");
#endif

#if defined(SL_CATALOG_MEMORY_MANAGER_DTCM_PRESENT)
  sl_memory_init_dtcm();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_memory_init_dtcm");
#endif

#if defined(SL_CATALOG_HAL_SYSTEM_PRESENT)
  sl_hal_system_init();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_hal_system_init");
#endif

  // Pre-clock application initialization.
  app_init_pre_clock();
  SLI_METRIC_EVENT_HANDLER_SAVE("app_init_pre_clock");

#if defined(SL_CATALOG_INTERRUPT_MANAGER_PRESENT)
  sl_interrupt_manager_init();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_interrupt_manager_init");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_DCDC_PRESENT)
  sl_device_init_dcdc();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_dcdc");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_LFXO_PRESENT)
  sl_device_init_lfxo();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_lfxo");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_HFRCO_PRESENT)
  sl_device_init_hfrco();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_hfrco");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_HFXO_PRESENT)
  sl_device_init_hfxo();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_hfxo");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_LFRCO_PRESENT)
  sl_device_init_lfrco();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_lfrco");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_DPLL_PRESENT)
  sl_device_init_dpll();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_dpll");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_USBPLL_PRESENT)
  sl_device_init_usbpll();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_usbpll");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_CLOCKS_PRESENT)
  sl_device_init_clocks();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_clocks");
#endif

#if defined(SL_CATALOG_DEVICE_INIT_RFFPLL_PRESENT)
  sl_device_init_rffpll();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_rffpll");
#endif

#if defined(SL_CATALOG_CLOCK_MANAGER_PRESENT)
  sl_clock_manager_init();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_clock_manager_init");
#endif

#if defined(SL_SI91X_SOC_MODE)
  SystemCoreClockUpdate();
#endif

#if defined(SL_CATALOG_DEVICE_INIT_EMU_PRESENT)
  sl_device_init_emu();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_device_init_emu");
#endif

#if defined(SL_CATALOG_SLEEPTIMER_PRESENT)
  sl_sleeptimer_init();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_sleeptimer_init");
#endif

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
  sl_power_manager_init();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_power_manager_init");
#endif

#if defined(SL_CATALOG_MPA_MANAGER_PRESENT)
  sl_mpa_manager_init();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_mpa_manager_init");
#endif

#if defined(SL_CATALOG_MPU_PRESENT)
  sl_mpu_disable_execute_from_ram();
  SLI_METRIC_EVENT_HANDLER_SAVE("sl_mpu_disable_execute_from_ram");
#endif

  // Early application initialization (post-system init).
  app_init_early();
  SLI_METRIC_EVENT_HANDLER_SAVE("app_init_early");

#ifdef SL_CATALOG_LEGACY_HAL_SOC_PRESENT
  halInit();
  SLI_METRIC_EVENT_HANDLER_SAVE("halInit");
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
  osKernelInitialize();
  SLI_METRIC_EVENT_HANDLER_SAVE("osKernelInitialize");
#endif

  sli_allocate_permanent_memory();
  SLI_METRIC_EVENT_HANDLER_SAVE("sli_allocate_permanent_memory");

  // Internal early init event.
  sli_internal_init_early();
  SLI_METRIC_EVENT_HANDLER_SAVE("sli_internal_init_early");

#if defined(SL_CATALOG_KERNEL_PRESENT)
  sli_main_kernel_start_task_initialize();
  SLI_METRIC_EVENT_HANDLER_SAVE("sli_main_kernel_start_task_initialize");
#endif

#if !defined(SL_CATALOG_KERNEL_PRESENT)
  sl_main_second_stage_init();
#endif
}
