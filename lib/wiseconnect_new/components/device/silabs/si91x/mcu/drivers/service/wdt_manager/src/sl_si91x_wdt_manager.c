/************************************************************************************
 * @file sl_si91x_wdt_manager.c
 * @brief Watchdog Manager Service API implementation
 ************************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ************************************************************************************
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
 ************************************************************************************/

#include "sl_si91x_wdt_manager.h"
#if defined SL_WDT_MANAGER_PRESENT_M4_UC_RC
#include "sl_si91x_wdt_rc_config.h"
#elif defined SL_WDT_MANAGER_PRESENT_M4_UC_XTAL
#include "sl_si91x_wdt_xtal_config.h"
#endif

#if defined(SLI_SI91X_ENABLE_OS)
#include "rsi_os.h"
#include "cmsis_os2.h"
#endif
#include "rsi_debug.h"

/************************************************************************************
 *************************  DEFINES / MACROS  ***************************************
 ************************************************************************************/
#if defined(SLI_SI91X_ENABLE_OS)
osSemaphoreId_t sl_wdt_task_semaphore_id;
#define SL_WDT_TASK_STACK_SIZE 512
const osThreadAttr_t wdt_thread_attributes = {
  .name       = "WDT_Task", // Name of thread
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = SL_WDT_TASK_STACK_SIZE, // Stack size of WDT task
  .priority   = osPriorityHigh7,        // Priority of WDT task
  .tz_module  = 0,
  .reserved   = 0,
};
#endif //SLI_SI91X_ENABLE_OS

/************************************************************************************
 *************************  LOCAL VARIABLES  ****************************************
 ************************************************************************************/

/************************************************************************************
 *************************  LOCAL TYPE DEFINITIONS  *********************************
 ************************************************************************************/

/************************************************************************************
 *************************  LOCAL FUNCTION PROTOTYPES  ******************************
 ************************************************************************************/
/************************************************************************************
 *************************  GLOBAL FUNCTION DEFINITIONS  ****************************
 ************************************************************************************/

/*******************************************************************************
 * Callback function of watchdog-timer
 * @param data (void pointer)
 * @return none
 ******************************************************************************/
static void on_timeout_callback(void)
{
#if defined(SLI_SI91X_ENABLE_OS)

  osStatus_t sl_semrel_status;
  sl_semrel_status = osSemaphoreRelease(sl_wdt_task_semaphore_id);
  if (sl_semrel_status != osOK) {
    // osSemaphoreRelease failed
  }
#else
  // kick the watchdog timer
  sl_si91x_watchdog_restart_timer();
#endif
}
#if defined(SLI_SI91X_ENABLE_OS)

/*******************************************************************************
 * Watchdog-timer task to kick the watchdog
 * @param *argument -Attributes structure for thread.
 * @return none
 ******************************************************************************/
static void WDT_Task(void *argument)
{
  UNUSED_PARAMETER(argument);
  osStatus_t sl_sem_wdt_taskacq_status;

  osSemaphoreAttr_t sl_wdt_semaphore_attr_st;
  sl_wdt_semaphore_attr_st.attr_bits = 0U;
  sl_wdt_semaphore_attr_st.cb_mem    = NULL;
  sl_wdt_semaphore_attr_st.cb_size   = 0U;
  sl_wdt_semaphore_attr_st.name      = NULL;

  sl_wdt_task_semaphore_id = osSemaphoreNew(1U, 0U, &sl_wdt_semaphore_attr_st);
  while (1) {
    // waiting for the semaphore release
    sl_sem_wdt_taskacq_status = osSemaphoreAcquire(sl_wdt_task_semaphore_id, osWaitForever);
    if (sl_sem_wdt_taskacq_status != osOK) {
      // osSemaphoreAcquire failed
    } else {
      //semaphore released
      sl_si91x_watchdog_restart_timer();
    }
  }
}
#endif

/*******************************************************************************
 * It retrieve the system reset reason.
 * Returns the reason for the system being reset.
 *******************************************************************************/
sl_watchdog_manager_reset_reason_t sl_wdt_manager_get_system_reset_status(void)
{
  uint32_t get_status = 0;
  get_status          = RSI_PS_GetComnIntrSts();

  switch (SL_SI91X_WDT_RESET & get_status) {
    case SL_SI91X_MCU_WWD_WINDOW_RESET:
    case SL_SI91X_MCU_WWD_RESET:
      DEBUGOUT("\r\n MCU WDT System Reset  \r\n");
      return SL_SI91X_WATCHDOG_MANAGER_RESET_WATCHDOG;

    case SL_SI91X_NWP_WWD_WINDOW_RESET:
    case SL_SI91X_NWP_WWD_RESET:
      DEBUGOUT("\r\n NWP WDT System Reset  \r\n");
      return SL_SI91X_WATCHDOG_MANAGER_RESET_WATCHDOG;

    case SL_SI91X_HOST_RESET_REQUEST:
      DEBUGOUT("\r\n Software System Reset \r\n");
      return SL_SI91X_WATCHDOG_MANAGER_RESET_SOFTWARE;

    default:
      DEBUGOUT("\r\n Power-on Reset \r\n");
      return SL_SI91X_WATCHDOG_MANAGER_RESET_POR;
  }
}

/*******************************************************************************
 * Initializes the Watchdog Manager Service
 * It initializes call back handler to WDT
 * It enables the WDT during the sleep
 * It configures WDT as wakeup source 
 ******************************************************************************/
sl_status_t sl_watchdog_manager_init(void)
{
  sl_status_t status                               = SL_STATUS_OK;
  watchdog_timer_config_t watchdog_config_internal = { 0 }; // Initialized the watch dog structure with zero

  // Power-Up WDT domain
  RSI_PS_NpssPeriPowerUp(SLPSS_PWRGATE_ULP_MCUWDT);

#if defined(SLI_SI91X_ENABLE_OS)
  osThreadId_t si91x_thread = 0;

  // Create WDT task
  si91x_thread = osThreadNew((osThreadFunc_t)WDT_Task, NULL, &wdt_thread_attributes);
  if (si91x_thread == NULL) {
    return SL_STATUS_FAIL;
  }
#endif

  // Initializing watchdog-timer
  sl_si91x_watchdog_init_timer();

  // Enable the Watchdog timer to reset the system on processor lockup
  sl_si91x_watchdog_enable_system_reset_on_processor_lockup();

  // Update system reset value based on configured interrupt time
#if defined(SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_PERIOD)
  watchdog_config_internal.interrupt_time = SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_PERIOD;

  // Assigning system reset value to 4sec ,if WDT is not kicked at programmed interval it resets the system after 4sec(reset counter starts counting after the interrupt time expiry)
  watchdog_config_internal.system_reset_time = SL_SI91X_WATCHDOG_MANAGER_TIMEOUT_INDEX_17;
#endif
  // Configuring watchdog-timer
  status = sl_si91x_watchdog_set_configuration(&watchdog_config_internal);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Registering timeout callback
  status = sl_si91x_watchdog_register_timeout_callback(on_timeout_callback);
  if (status != SL_STATUS_OK) {
    return status;
    //Failed to register callback
  }
#if defined(SL_SI91X_WATCHDOG_MANAGER_ENABLE_DURING_SLEEP) && (SL_SI91X_WATCHDOG_MANAGER_ENABLE_DURING_SLEEP == ENABLE)
  // Enable the WDT in sleep
  sl_watchdog_enable_wdt_in_sleep();
#else
  // Disable the WDT in sleep
  sl_watchdog_disable_wdt_in_sleep();
#endif //SL_ENABLE_WDT_DURING_SLEEP

  return status;
}

/*******************************************************************************
 * Starts the Watchdog Manager Service
 ******************************************************************************/
sl_status_t sl_watchdog_manager_start(void)
{
  // Start WDT timer
  sl_si91x_watchdog_start_timer();

  return SL_STATUS_OK;
}

/*******************************************************************************
 * Disables the WDT during sleep or shutdown states
 *******************************************************************************/
void sl_watchdog_disable_wdt_in_sleep(void)
{
  MCU_FSM->MCU_FSM_CRTL_PDM_AND_ENABLES_b.ENABLE_WDT_IN_SLEEP_b = 0;
}

/*******************************************************************************
 * Enables the WDT during sleep or shutdown states
 * It sets WDT as a wake-up source to the system
 *******************************************************************************/
void sl_watchdog_enable_wdt_in_sleep(void)
{
  // Enable WDT as a wakeup source to kick the WDT
  RSI_PS_SetWkpSources(WDT_INTR_BASED_WAKEUP);
  // Enable WDT during sleep
  MCU_FSM->MCU_FSM_CRTL_PDM_AND_ENABLES_b.ENABLE_WDT_IN_SLEEP_b = 1;
}
