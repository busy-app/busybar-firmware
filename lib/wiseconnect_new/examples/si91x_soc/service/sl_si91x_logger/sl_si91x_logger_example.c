/***************************************************************************/ /**
 * @file sl_si91x_logger_example_example.c
 * @brief logger example
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
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_logger_example.h"
#include "FreeRTOS.h"
#include "sl_log_helper.h"

/*******************************************************************************
 ***************************  Defines / Macros  ********************************
 ******************************************************************************/
// Mask of all power‑state transition events we want the Power Manager to report
#define PS_EVENT_MASK                                                                                           \
  (SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS4 | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS4   \
   | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS3 | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS3 \
   | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_SLEEP)

// Number of M4/TA state combinations cycled through
#define MAX_COMBINATIONS 8

/*******************************************************************************
 *************************** LOCAL VARIABLES   *********************************
 ******************************************************************************/

// Semaphores used to serialize the M4, TA, and app tasks
static osSemaphoreId_t sem_m4_turn;
static osSemaphoreId_t sem_ta_turn;
static osSemaphoreId_t sem_app_turn;

// Thread attributes for App, TA, and M4 tasks
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .stack_size = 3072,
  .priority   = osPriorityLow3,
};

const osThreadAttr_t ta_thread_attributes = {
  .name       = "TA task",
  .stack_size = 3072,
  .priority   = osPriorityLow1,
};

const osThreadAttr_t m4_thread_attributes = {
  .name       = "M4 task",
  .stack_size = 3072,
  .priority   = osPriorityLow2,
};

// Structure grouping an M4 power state with a TA Wi-Fi performance profile
typedef struct {
  sl_power_state_t m4_state;
  sl_wifi_performance_profile_v2_t ta_profile;
} sl_m4_ta_power_t;

// Predefined sequence of M4/TA combinations the app will iterate through
sl_m4_ta_power_t m4_ta_combinations[MAX_COMBINATIONS] = {
  { .m4_state = SL_SI91X_POWER_MANAGER_PS4, .ta_profile.profile = HIGH_PERFORMANCE },
  { .m4_state = SL_SI91X_POWER_MANAGER_PS3, .ta_profile.profile = HIGH_PERFORMANCE },
  { .m4_state = SL_SI91X_POWER_MANAGER_PS4, .ta_profile.profile = DEEP_SLEEP_WITH_RAM_RETENTION },
  { .m4_state = SL_SI91X_POWER_MANAGER_PS3, .ta_profile.profile = DEEP_SLEEP_WITH_RAM_RETENTION },
  { .m4_state = SL_SI91X_POWER_MANAGER_PS4, .ta_profile.profile = HIGH_PERFORMANCE },
  { .m4_state = SL_SI91X_POWER_MANAGER_PS4, .ta_profile.profile = DEEP_SLEEP_WITH_RAM_RETENTION },
  { .m4_state = SL_SI91X_POWER_MANAGER_PS3, .ta_profile.profile = HIGH_PERFORMANCE },
  { .m4_state = SL_SI91X_POWER_MANAGER_PS3, .ta_profile.profile = DEEP_SLEEP_WITH_RAM_RETENTION }
};
// prototypes
static void application_start(void *argument);
static sl_status_t initialize_wireless(void);
static void transition_callback(sl_power_state_t from, sl_power_state_t to);

// Global that the app task updates to signal the next desired M4/TA state
static sl_m4_ta_power_t sl_log_m4_ta_power_state = { .m4_state           = SL_SI91X_POWER_MANAGER_PS4,
                                                     .ta_profile.profile = HIGH_PERFORMANCE };

/*******************************************************************************
 **************************** Helper Functions *********************************
 ******************************************************************************/

// Attempts to move the M4 core to the given power state
static sl_status_t set_m4_power_state(sl_power_state_t target)
{
  SL_PRINT_STRING_DEBUG("ENTER: set_m4_power_state");

  sl_status_t status       = SL_STATUS_OK;
  sl_power_state_t current = sl_si91x_power_manager_get_current_state();

  // If already in requested state, log a warning and exit
  if (current == target) {
    SL_PRINT_STRING_WARN("M4 already in requested state %lu", target);
    return SL_STATUS_OK;
  }

  SL_PRINT_STRING_DEBUG("M4 current state %lu Target state %lu", current, target);

  // Add and remove the PS requirement to trigger transition
  status = sl_si91x_power_manager_add_ps_requirement(target);
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("Add PS req fail %lu", status);
    return status;
  }

  status = sl_si91x_power_manager_remove_ps_requirement(target);
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("Remove PS req fail %lu", status);
    return status;
  }

  SL_PRINT_STRING_INFO("M4 change state success, current state %lu", target);

  osDelay(10); // Allow small delay for state change to settle

  SL_PRINT_STRING_DEBUG("EXIT: set_m4_power_state");
  return status;
}

// Updates the Wi‑Fi performance profile of the TA (network processor)
static sl_status_t set_ta_profile(sl_wifi_performance_profile_v2_t *profile)
{
  SL_PRINT_STRING_DEBUG("ENTER: set_ta_profile");

  sl_wifi_performance_profile_v2_t current;
  sl_wifi_get_performance_profile_v2(&current);

  // Warn if requested profile is already active
  if (current.profile == profile->profile) {
    SL_PRINT_STRING_WARN("TA already in requested state %lu", profile->profile);
    return SL_STATUS_OK;
  }

  SL_PRINT_STRING_DEBUG("TA Current state %lu Target state %lu", current.profile, profile->profile);

  sl_status_t status = sl_wifi_set_performance_profile_v2(profile);
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("TA set profile failed %lu", status);
  }

  SL_PRINT_STRING_INFO("TA set profile success, current profile %lu", profile->profile);

  osDelay(10); // Allow small delay for NWP state application

  SL_PRINT_STRING_DEBUG("EXIT: set_ta_profile");
  return status;
}

/*******************************************************************************
 **************************** TA Task ******************************************
 ******************************************************************************/
// The TA task initializes wireless, then waits for semaphore signal from app task
static void ta_task_start(void *arg)
{
  SL_PRINT_STRING_DEBUG("ENTER: ta_task_start");
  (void)arg;

  // Initialize Wi‑Fi and network processor
  sl_status_t status = initialize_wireless();
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("Wireless init fail 0x%lX", status);
    return;
  }

  // Main TA‑task loop
  for (;;) {
    // Wait until orchestrator signals TA turn
    osSemaphoreAcquire(sem_ta_turn, osWaitForever);

    // Apply TA profile requested by the app task
    set_ta_profile(&sl_log_m4_ta_power_state.ta_profile);

    // Signal back to app task that TA update is complete
    osSemaphoreRelease(sem_app_turn);
  }
}

/*******************************************************************************
 **************************** M4 Task ******************************************
 ******************************************************************************/
// M4 task waits for semaphore, updates M4 power state, then triggers TA task
static void m4_task_start(void *arg)
{
  SL_PRINT_STRING_DEBUG("ENTER: m4_task_start");
  (void)arg;

  // Subscribe to power‑state transition events for logging
  sl_power_manager_ps_transition_event_handle_t handle;
  sl_power_manager_ps_transition_event_info_t info = { .event_mask = PS_EVENT_MASK, .on_event = transition_callback };

  if (sl_si91x_power_manager_subscribe_ps_transition_event(&handle, &info) != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("PM event subscribe fail");
  }

  // Main M4‑task loop
  for (;;) {
    // Wait for orchestrator to signal M4 turn
    osSemaphoreAcquire(sem_m4_turn, osWaitForever);

    // Apply requested M4 power state
    set_m4_power_state(sl_log_m4_ta_power_state.m4_state);

    // Signal TA task that M4 update is complete
    osSemaphoreRelease(sem_ta_turn);
  }
}

/*******************************************************************************
 **************************** App (Orchestrator) *******************************
 ******************************************************************************/
// Main application thread that cycles through power/profile combinations
static void application_start(void *argument)
{
  SL_PRINT_STRING_DEBUG("ENTER: application_start");
  (void)argument;

  osThreadId_t m4_task_thread_id;
  osThreadId_t ta_task_thread_id;

  // Create semaphores used to control task execution order
  sem_m4_turn = osSemaphoreNew(1U, 0U, NULL);
  if (sem_m4_turn == NULL) {
    SL_PRINT_STRING_ERROR("sem_m4_turn semaphore creation failed");
  }
  SL_PRINT_STRING_INFO("sem_m4_turn semaphore created");

  sem_ta_turn = osSemaphoreNew(1U, 0U, NULL);
  if (sem_ta_turn == NULL) {
    SL_PRINT_STRING_ERROR("sem_ta_turn semaphore creation failed");
  }
  SL_PRINT_STRING_INFO("sem_ta_turn semaphore created");

  sem_app_turn = osSemaphoreNew(1U, 0U, NULL);
  if (sem_app_turn == NULL) {
    SL_PRINT_STRING_ERROR("sem_app_turn semaphore creation failed");
  }
  SL_PRINT_STRING_INFO("sem_app_turn semaphore created");

  // Create TA and M4 worker threads
  ta_task_thread_id = osThreadNew(ta_task_start, NULL, &ta_thread_attributes);
  if (ta_task_thread_id == NULL) {
    SL_PRINT_STRING_ERROR("ta_task_start thread creation failed");
    while (1)
      ;
  }
  SL_PRINT_STRING_INFO("ta_task_start thread created");

  m4_task_thread_id = osThreadNew(m4_task_start, NULL, &m4_thread_attributes);
  if (m4_task_thread_id == NULL) {
    SL_PRINT_STRING_ERROR("m4_task_thread_id thread creation failed");
    while (1)
      ;
  }
  SL_PRINT_STRING_INFO("m4_task_start thread created");

  // Iterate through all M4/TA combinations in a loop
  uint32_t combination_index = 0;
  for (;;) {
    // Update the desired global state
    sl_log_m4_ta_power_state = m4_ta_combinations[combination_index];

    osDelay(100); // Allow brief pause before starting next update

    // Start M4 update first
    osSemaphoreRelease(sem_m4_turn);

    // Wait until both M4 and TA updates complete
    osSemaphoreAcquire(sem_app_turn, osWaitForever);

    // Synchronize timestamps across cores
    sl_log_sync_timestamp(0, NULL);
    SL_PRINT_STRING_INFO("Timestamp synchronization complete");

    // Log timestamp counters from both cores
    SL_PRINT_STRING_INFO("M4_TS=%lu TA_TS=%lu", sl_log_get_timestamp_count(1), sl_log_get_timestamp_count(0));

    // Move to next combination, flush logs when finishing full cycle
    combination_index++;
    if (combination_index == MAX_COMBINATIONS) {
      combination_index = 0;
      sl_log_flush(); // Flush the logs to the uart
    }
  }
}

/*******************************************************************************
 **************************** Wireless Init ************************************
 ******************************************************************************/
// Initializes Wi‑Fi subsystem and loads firmware
static sl_status_t initialize_wireless(void)
{
  SL_PRINT_STRING_DEBUG("ENTER: initialize_wireless");

  // Standard STA‑mode configuration with low‑power features
  static const sl_wifi_device_configuration_t station_init_configuration = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
    .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                     .coex_mode = SL_SI91X_WLAN_ONLY_MODE,
                     .feature_bit_map =
                       (SL_WIFI_FEAT_SECURITY_OPEN | SL_WIFI_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE),
                     .tcp_ip_feature_bit_map =
                       (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                     .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                     .ext_custom_feature_bit_map = (SL_WIFI_SYSTEM_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK
                                                    | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG),
                     .bt_feature_bit_map         = 0,
                     .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                     .ble_feature_bit_map        = 0,
                     .ble_ext_feature_bit_map    = 0,
                     .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_WIFI_ENABLE_ENHANCED_MAX_PSP) }
  };

  sl_status_t status;
  sl_wifi_firmware_version_t version = { 0 };
  sl_mac_address_t mac_addr          = { 0 };

  // Initialize Wi‑Fi stack and load firmware
  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &station_init_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("WiFi init fail 0x%lX", status);
    return status;
  }

  // Retrieve MAC address
  status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
  if (status != SL_STATUS_OK)
    SL_PRINT_STRING_ERROR("MAC fail 0x%lX", status);

  // Retrieve firmware version
  status = sl_wifi_get_firmware_version(&version);
  if (status != SL_STATUS_OK) {
    SL_PRINT_STRING_ERROR("FW ver fail 0x%lX", status);
  }

  SL_PRINT_STRING_DEBUG("EXIT: initialize_wireless");
  return SL_STATUS_OK;
}

/*******************************************************************************
 **************************** Transition Callback ******************************
 ******************************************************************************/
// Called when the M4 power manager transitions between PS states
static void transition_callback(sl_power_state_t from, sl_power_state_t to)
{
  SL_PRINT_STRING_DEBUG("ENTER: transition_callback");

  // Log the transition source
  switch (from) {
    case SL_SI91X_POWER_MANAGER_PS4:
      SL_PRINT_STRING_INFO("Leave PS4");
      break;
    case SL_SI91X_POWER_MANAGER_PS3:
      SL_PRINT_STRING_INFO("Leave PS3");
      break;
    default:
      break;
  }

  // Log the transition destination
  switch (to) {
    case SL_SI91X_POWER_MANAGER_PS4:
      SL_PRINT_STRING_INFO("Enter PS4");
      break;
    case SL_SI91X_POWER_MANAGER_PS3:
      SL_PRINT_STRING_INFO("Enter PS3");
      break;
    default:
      break;
  }

  SL_PRINT_STRING_DEBUG("EXIT: transition_callback");
}

/*******************************************************************************
 **************************** Init Entry ****************************************
 ******************************************************************************/
// Public function called by system startup to create the main app task
void logger_example_init(void)
{
  // Start application orchestrator
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}
