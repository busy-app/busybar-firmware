/***************************************************************************/ /**
 * @file power_manager_tickless_idle_example.c
 * @brief Power Manager example functions
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "cmsis_os2.h"
#include "sl_wifi.h"
#include "sl_si91x_driver.h"
#include "sl_net.h"
#include "sl_si91x_power_manager.h"
#include "power_manager_tickless_idle_example.h"
#include "rsi_debug.h"
#include "rsi_m4.h"
#include "sl_si91x_button_instances.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "sl_ulp_timer_instances.h"
#include "sl_si91x_ulp_timer_common_config.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
// Set SL_SI91X_STANDBY to 0 to enable active to sleep state transition.
// Set SL_SI91X_STANDBY to 1 to enable active to standby transition.
#define SL_SI91X_STANDBY 0

#define PS_EVENT_MASK                                                                                           \
  (SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS4 | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS4   \
   | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS3 | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS3 \
   | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS2 | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS2 \
   | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_SLEEP) // Ored value of event for which callback is subscribed

#define ULP_TIMER_MATCH_VALUE 10000000 // Timer match value for down-counter type with 20mhz clock for 500 ms.
#define ULP_TIMER_INSTANCE \
  SL_ULP_TIMER_TIMER0 // ulp-timer instance to be used, user can pass selected timer-number in place of '0'

#define BUTTON_INSTANCE_0 button_btn0
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
// This enum is used in the state transition
typedef enum {
  ADD_REQ,
  REM_REQ,
} ps_transition;
TimerHandle_t PM_timer_handle;
static ps_transition transition = ADD_REQ;
static boolean_t change_state   = false;
osSemaphoreId_t timer_semaphore;
// Thread attributes which are required for creating a thread
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

/*******************************************************************************
 **********************  Local Function prototypes   ***************************
 ******************************************************************************/
static void application_start(void *argument);
static sl_status_t initialize_wireless(void);
static void wireless_sleep(boolean_t sleep_with_retention);
static void transition_callback(sl_power_state_t from, sl_power_state_t to);
void pm_timer_callback(TimerHandle_t xTimer);

#if !SL_SI91X_STANDBY
static void set_ulp_timer_configuration(void);
static void clear_ulp_timer_configuration(void);
static void ulp_timer_callback(void);
#endif

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/
/*******************************************************************************
 * Initializes the thread.
 * New thread is created with the application_start function.
 ******************************************************************************/
void power_manager_example_init(void)
{
  // Initialization of thread with the application_start function
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

/*******************************************************************************
 * - This function is called in the thread.
 * - All the activities are performed in this function.
 * - The wireless interface is initialized first and then the power manager service is initialized.
 * - Events are subscribed to get the callbacks.
 * - This function transits between different power states and sleep.
 ******************************************************************************/
static void application_start(void *argument)
{
  UNUSED_VARIABLE(argument);
  timer_semaphore = osSemaphoreNew(1U, 0U, NULL);
  sl_power_manager_ps_transition_event_handle_t handle;
  sl_power_manager_ps_transition_event_info_t info = { .event_mask = PS_EVENT_MASK, .on_event = transition_callback };
  sl_status_t status;
  osStatus_t timer_status;
  // Initialize the wireless interface and put the NWP in Standby with RAM retention mode.
  status = initialize_wireless();
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("Wireless API initialization failed, Error Code: 0x%lX \n", status);
    return;
  }
  DEBUGOUT("Wireless is successfully initialized and NWP is in Sleep \n");

  // Subscribe the state transition callback events, the ored value of flag and function pointer is passed in this API.
  status = sl_si91x_power_manager_subscribe_ps_transition_event(&handle, &info);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("Power Manager transition event subscription failed, Error Code: 0x%lX \n", status);
    return;
  }
  DEBUGOUT("Power Manager transition event is subscribed \n");

  change_state = false;

  // Create software timer
  PM_timer_handle = xTimerCreate("PM_app_os_timer", 500, 1, NULL, pm_timer_callback);
  // start software timer
  timer_status = xTimerStart(PM_timer_handle, portMAX_DELAY);

  if (timer_status != pdPASS) {
  }
  while (true) {
    power_manager_example_process_action();
  }
}

/*******************************************************************************
 * Initialization of wireless APIs.
 * M4-NWP handshake is performed.
 * NWP is send to standby with RAM retention mode if SWITCH_TO_PS0 is disabled.
 * NWP is send to standby without RAM retention mode if SWITCH_TO_PS0 is enabled.
 ******************************************************************************/
static sl_status_t initialize_wireless(void)
{
  // For M4-sleep wakeup, and to achieve minimum current in powersave application,
  // wifi is initialized, then
  // get the mac address and firmware version.
  // NWP powersave profile is updated sleep with/without retention as per
  // requirements.
  // Wifi device configuration
  static const sl_wifi_device_configuration_t station_init_configuration = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
    .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                     .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                     .feature_bit_map =
                       (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
#ifdef SLI_SI91X_MCU_INTERFACE
                        | SL_WIFI_FEAT_WPS_DISABLE
#endif
                        ),
                     .tcp_ip_feature_bit_map =
                       (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                     .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                     .ext_custom_feature_bit_map = (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                    | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                                                    | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                                                    ),
                     .bt_feature_bit_map         = 0,
                     .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                     .ble_feature_bit_map        = 0,
                     .ble_ext_feature_bit_map    = 0,
                     .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
  };
  sl_status_t status;
  sl_wifi_firmware_version_t version = { 0 };
  sl_mac_address_t mac_addr          = { 0 };
  // Initialize the wifi interface.
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("Failed to start Wi-Fi Client interface: 0x%lx\r\n", status);
    return status;
  }
  // Get the mac address.
  status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
  if (status == SL_STATUS_OK) {
    DEBUGOUT("Device MAC address: %x:%x:%x:%x:%x:%x\r\n",
             mac_addr.octet[0],
             mac_addr.octet[1],
             mac_addr.octet[2],
             mac_addr.octet[3],
             mac_addr.octet[4],
             mac_addr.octet[5]);
  } else {
    DEBUGOUT("Failed to get mac address: 0x%lx\r\n", status);
  }
  // Get the firmware version.
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\nFailed to fetch firmware version: 0x%lx\r\n", status);
  } else {
    print_firmware_version(&version);
  }
  // Wireless Sleep with ram retention
  wireless_sleep(true);
  // If reaches here, returns SL_STATUS_OK.
  return SL_STATUS_OK;
}

/*******************************************************************************
 * After PS2 to PS4 transition, flash is initialized and to initialize flash
 * wireless processor is set to active mode.
 * This function sends the wireless processor to sleep with retention.
 ******************************************************************************/
static void wireless_sleep(boolean_t sleep_with_retention)
{
  sl_status_t status;
  // Wifi Profile (NWP Mode) is set to High Performance.
  sl_wifi_performance_profile_v2_t ta_performance_profile = { .profile = HIGH_PERFORMANCE };

  status = sl_wifi_set_performance_profile_v2(&ta_performance_profile);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_wifi_set_performance_profile_v2 failed, Error Code: 0x%lX \n", status);
    return;
  }
  if (sleep_with_retention) {
    // Wifi Profile (NWP Mode) is set to standby power save with RAM retention.
    ta_performance_profile.profile = DEEP_SLEEP_WITH_RAM_RETENTION;
  } else {
    ta_performance_profile.profile = DEEP_SLEEP_WITHOUT_RAM_RETENTION;
  }
  // Wifi Profile (NWP Mode) is set to standby power save with RAM retention.
  status = sl_wifi_set_performance_profile_v2(&ta_performance_profile);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_wifi_set_performance_profile_v2 failed, Error Code: 0x%lX \n", status);
    return;
  }
}
/*******************************************************************************
 * Function will run continuously in while loop
 ******************************************************************************/
void power_manager_example_process_action(void)
{
  osStatus_t sl_sem_status;
  sl_status_t status;
  switch (transition) {
    case ADD_REQ:
      if (change_state) {
        DEBUGOUT("Current State: PS%d \n", sl_si91x_power_manager_get_current_state());
        if (sl_si91x_power_manager_get_current_state() == SL_SI91X_POWER_MANAGER_PS2) {
#if !SL_SI91X_STANDBY
          if (sl_si91x_power_manager_get_ps1_state_status() == false) {
            // Stop the PM timer.
            xTimerStop(PM_timer_handle, portMAX_DELAY);
            // Add the PS1 state request in the power manager.
            status = sl_si91x_power_manager_request_ps1_state();
            if (status != SL_STATUS_OK) {
              // If status is not OK, display the error info.
              DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
            }
            // Configure the ULP timer to wake up from PS1.
            set_ulp_timer_configuration();
          } else {
            // Start the PM timer.
            xTimerStart(PM_timer_handle, portMAX_DELAY);
            // Remove the PS1 state request in the power manager.
            status = sl_si91x_power_manager_remove_ps1_state_request();
            if (status != SL_STATUS_OK) {
              // If status is not OK, display the error info.
              DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
            }
            // Clear the ULP timer wakeup source.
            clear_ulp_timer_configuration();
#endif
            status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
            if (status != SL_STATUS_OK) {
              // If status is not OK, display the error info.
              DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
            }
#if !SL_SI91X_STANDBY
            // Closing bracket for removing the PS1 state request. This should only be active when Standby mode is disabled.
          }
#endif
        } else if (sl_si91x_power_manager_get_current_state() == SL_SI91X_POWER_MANAGER_PS4) {
          status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS3);
          if (status != SL_STATUS_OK) {
            // If status is not OK, display the error info.
            DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
          }
        } else if (sl_si91x_power_manager_get_current_state() == SL_SI91X_POWER_MANAGER_PS3) {
          while (!sl_si91x_power_manager_ps2_pre_check()) {
            // Wait for the PS2 state to be safe to enter.
            osDelay(5);
          }
          status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
          if (status != SL_STATUS_OK) {
            // If status is not OK, display the error info.
            DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
          }
        }
        // Next transition is from PS3 to PS4 state.
        transition   = REM_REQ;
        change_state = false;
      }
      break;
    case REM_REQ:
      if (change_state) {
        DEBUGOUT("Current State: PS%d \n", sl_si91x_power_manager_get_current_state());
        status = sl_si91x_power_manager_remove_ps_requirement(sl_si91x_power_manager_get_current_state());
        if (status != SL_STATUS_OK) {
          // If status is not OK, display the error info.
          DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
        }
        change_state = false;
        // Next transition is from PS4 to PS3 state.
        transition = ADD_REQ;
      }
      break;
    default:
      break;
  }
#if SL_SI91X_STANDBY
  // Add the standby state request in the power manager.
  status = sl_si91x_power_manager_request_standby_state();
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
  }
#endif
  sl_sem_status = osSemaphoreAcquire(timer_semaphore, osWaitForever);
  if (sl_sem_status != osOK) {
    DEBUGOUT("\r\n osSemaphoreAcquire failed :%d \r\n", sl_sem_status);
  }
#if SL_SI91X_STANDBY
  // Remove the standby state request in the power manager.
  status = sl_si91x_power_manager_remove_standby_state_request();
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status);
  }
#endif
}

/*******************************************************************************
 * Callback function for state transition.
 * Prints the state transition with the parameters from and to.
 ******************************************************************************/
static void transition_callback(sl_power_state_t from, sl_power_state_t to)
{
  // DEBUGINIT needs ULP Uart to be enabled in low power states.
  // This is a demonstration and it is not necessary to have debugout in callback function.
  // This callback function can be used to perform any activities after state transitions.
  DEBUGINIT();
  switch (from) {
    case SL_SI91X_POWER_MANAGER_PS4:
      // Previous state was PS4
      DEBUGOUT("Leaving PS4 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS3:
      // Previous state was PS3
      DEBUGOUT("Leaving PS3 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS2:
      // Previous state was PS2
      DEBUGOUT("Leaving PS2 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS1:
      // Wakeup from PS1
      DEBUGOUT("Leaving PS1 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_SLEEP:
      // Wakeup from sleep
      DEBUGOUT("Leaving Sleep State \n");
      break;
    case SL_SI91X_POWER_MANAGER_STANDBY:
      // Wakeup from standby
      DEBUGOUT("Leaving Standby State \n");
      break;
    default:
      break;
  }

  switch (to) {
    case SL_SI91X_POWER_MANAGER_PS4:
      // Current state is PS4
      DEBUGOUT("Entering PS4 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS3:
      // Current state is PS3
      DEBUGOUT("Entering PS3 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS2:
      // Current state is PS2
      DEBUGOUT("Entering PS2 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS1:
      // Current state is PS2
      DEBUGOUT("Entering PS1 State \n");
      break;
    default:
      break;
  }
}

/*******************************************************************************
 * PM_timer_callback to change the state of flag, used in state machine.
 ******************************************************************************/
void pm_timer_callback(TimerHandle_t xTimer)
{
  (void)xTimer;
  osStatus_t sl_sem_status;
  change_state = true;

  sl_sem_status = osSemaphoreRelease(timer_semaphore);
  if (sl_sem_status != osOK) {
    DEBUGOUT("\r\nosSemaphoreRelease failed :%d \r\n", sl_sem_status);
  }
}

#if !SL_SI91X_STANDBY
/*******************************************************************************
 * Configure and initialize the ulp timer wakeup source.
 * ULP timer interrupt is used to wakeup from PS1.
 * ULP timer is initialzed and configured for 2 seconds.
 * Callback is registered for ULP Timer interrupt.
 ******************************************************************************/
static void set_ulp_timer_configuration(void)
{
  sl_status_t status;
  sl_timer_clk_handle.ulp_timer_clk_input_src = ULP_TIMER_MHZ_RC_CLK_SRC;

  // ULP Timer initialization, the values are fetched from the UC.
  status = sl_si91x_ulp_timer_init(&sl_timer_clk_handle);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_init failed, Error Code: 0x%lX \n", status);
    return;
  }
  // Match value is set to 2 seconds.
  sl_timer_handle_timer0.timer_match_value = ULP_TIMER_MATCH_VALUE;
  // ULP timer configurations are set, the values are fetched from the UC.
  status = sl_si91x_ulp_timer_set_configuration(&sl_timer_handle_timer0);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_set_configuration failed, Error Code: 0x%lX \n", status);
    return;
  }
  // Callback is registered to enable the timer interrupt.
  status = sl_si91x_ulp_timer_register_timeout_callback(ULP_TIMER_INSTANCE, ulp_timer_callback);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_register_timeout_callback failed, Error Code: 0x%lX \n", status);
    return;
  }
  // ULP Timer is started for timer 0.
  status = sl_si91x_ulp_timer_start(ULP_TIMER_INSTANCE);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_start failed, Error Code: 0x%lX \n", status);
    return;
  }
}
/*******************************************************************************
 * Clears ulp timer wakeup source.
 * ULP timer is stopped here.
 * Callback is unregistered for ULP Timer interrupt.
 ******************************************************************************/
static void clear_ulp_timer_configuration(void)
{
  sl_status_t status;
  // After waking up, ulp timer is stopped.
  status = sl_si91x_ulp_timer_stop(ULP_TIMER_INSTANCE);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_stop failed, Error Code: 0x%lX \n", status);
    return;
  }
  // After waking up, ulp timer callback is unregistered.
  status = sl_si91x_ulp_timer_unregister_timeout_callback(ULP_TIMER_INSTANCE);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_ulp_timer_unregister_timeout_callback failed, Error Code: 0x%lX \n", status);
    return;
  }
  // Once the sleep-wakeup is completed, ulp based wakeup source is removed.
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_ULPSS_WAKEUP, false);
  if (status != SL_STATUS_OK) {
    // If status is not OK, display the error info.
    DEBUGOUT("sl_si91x_power_manager_set_wakeup_sources failed, Error Code: 0x%lX \n", status);
    return;
  }
}

/*******************************************************************************
 * ULP Timer callback.
 * Interrupt Clearing is handled in driver layer, so this function is defined
 * to avoid undefined function while registering callback.
 ******************************************************************************/
static void ulp_timer_callback(void)
{
  osStatus_t sl_sem_status;
  change_state  = true;
  transition    = ADD_REQ;
  sl_sem_status = osSemaphoreRelease(timer_semaphore);
  if (sl_sem_status != osOK) {
    DEBUGOUT("\r\nosSemaphoreRelease failed :%d \r\n", sl_sem_status);
  }
}
#endif
