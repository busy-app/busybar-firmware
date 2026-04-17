/*******************************************************************************
 * @file sl_si91x_power_manager.c
 * @brief Power Manager Service API implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_si91x_power_manager.h"
#if (SL_SI91X_TICKLESS_MODE == 1)
#include "sl_wifi.h"
#endif
#include "sli_si91x_clock_manager.h"
#if SL_WIFI_COMPONENT_INCLUDED
#include "sl_rsi_utility.h"
#endif
#include "sl_component_catalog.h"

#ifdef SL_CATALOG_LOGGER_COMPONENT_PRESENT
#include "sl_log_platform_specific.h"
#endif
/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/

#define POWER_STATE_TABLE_SIZE 5 // Power State Requirement Table size
#define PS_MIN_COUNTER         0 // Minimum Power State count
#define PS_MAX_COUNTER         5 // Maximum Power State count

/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/
static sl_power_state_t current_state                          = SL_SI91X_POWER_MANAGER_PS3;
static boolean_t is_initialized                                = false;
static sl_slist_node_t *power_manager_ps_transition_event_list = NULL;
static sl_clock_scaling_t clock_scaling_mode                   = SL_SI91X_POWER_MANAGER_POWERSAVE;

// Indicates whether the M4 core is ready to transition to the PS1 sleep state.
// PS1 sleep will be triggered when the application is in the IDLE state and this flag is set.
static bool sli_si91x_pm_ps1_state_active = false;
// Indicates whether the M4 core is ready to transition to the standby state.
// Standby state will be triggered when the application is in the IDLE state and this flag is set.
static bool sli_si91x_pm_standby_state_active = false;

// Table of power state counters. Each counter indicates the presence (not zero)
// or absence (zero) of requirements on a given power state.
static uint8_t requirement_ps_table[POWER_STATE_TABLE_SIZE] = {
  0, // SL_SI91X_POWER_MANAGER_PS0 requirement counter
  0, // SL_SI91X_POWER_MANAGER_PS1 requirement counter
  0, // SL_SI91X_POWER_MANAGER_PS2 requirement counter
  0, // SL_SI91X_POWER_MANAGER_PS3 requirement counter
  0, // SL_SI91X_POWER_MANAGER_PS4 requirement counter
};
/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  Hook References ********************************
 ******************************************************************************/
boolean_t sl_si91x_power_manager_sleep_on_isr_exit(void);

/*******************************************************************************
 ********  Mandatory callback that allows to cancel sleeping action. ********
 ******************************************************************************/
boolean_t sl_si91x_power_manager_is_ok_to_sleep(void);

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void notify_power_state_transition(sl_power_state_t from, sl_power_state_t to);

/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/
bool sli_si91x_is_sdk_ok_to_sleep();
bool sli_si91x_ta_packet_initiated_to_m4(void);
/*******************************************************************************
 * Initializes the Power Manager Service.
 * If service is not already initialized, then initialize the linked list and
 * clears the requirement table.
 * It also initializes the hardware: Sets the system clock to 100 MHz and changes the 
 * state to PS4.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_init(void)
{
  if (!is_initialized) {
    SL_SI91X_POWER_MANAGER_CORE_ENTER_CRITICAL;
    // If power manager is not initialized, resets the linked list
    // and requirement table.
    sl_slist_init(&power_manager_ps_transition_event_list);
    memset(requirement_ps_table, 0, sizeof(requirement_ps_table));
    // Configures the clock to power save mode
    sli_si91x_power_manager_init_hardware();
    is_initialized = true;

#if defined(SL_SI91X_POWER_MANAGER_DEBUG) && (SL_SI91X_POWER_MANAGER_DEBUG == ENABLE)
    sli_si91x_power_manager_init_debug();
#endif
    SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
    return SL_STATUS_OK;
  }
  SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
  return SL_STATUS_ALREADY_INITIALIZED;
}

/*******************************************************************************
 * Adds the peripheral requirement.
 * Calls the internal peripheral update function which validates the peripheral
 * and enables them.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_add_peripheral_requirement(sl_power_peripheral_t *peripheral)
{
  sl_status_t status;
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (peripheral == NULL) {
    // Validates the peripheral, if null pointer, returns error code.
    return SL_STATUS_NULL_POINTER;
  }
  // returns the error code for peripheral update, if conditions are fulfilled,
  // updated the peripheral requirements.
  status = sli_power_manager_update_peripheral(peripheral, true);
  return status;
}

/*******************************************************************************
 * Removes the peripheral requirement.
 * Calls the internal peripheral update function which validates the peripheral
 * and disables them.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_remove_peripheral_requirement(sl_power_peripheral_t *peripheral)
{
  sl_status_t status;
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (peripheral == NULL) {
    // Validates the peripheral, if null pointer, returns error code.
    return SL_STATUS_NULL_POINTER;
  }
  // returns the error code for peripheral update, if conditions are fulfilled,
  // updated the peripheral requirements.
  status = sli_power_manager_update_peripheral(peripheral, false);
  return status;
}

/*******************************************************************************
 * Registers a callback to be called on given Power state transition(s).
 * New node is added to the linked list for handling the state transition callbacks.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_subscribe_ps_transition_event(
  sl_power_manager_ps_transition_event_handle_t *event_handle,
  const sl_power_manager_ps_transition_event_info_t *event_info)
{
  if ((event_handle) == NULL || (event_info == NULL)) {
    // Validates the null pointer, if incorrect returns the error code.
    return SL_STATUS_NULL_POINTER;
  }
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }
  SL_SI91X_POWER_MANAGER_CORE_ENTER_CRITICAL;
  event_handle->info = (sl_power_manager_ps_transition_event_info_t *)event_info;
  // Push the data into the linked list.
  sl_slist_push(&power_manager_ps_transition_event_list, &event_handle->node);
  SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
  // If it reaches here, then returns SL_STATUS_OK
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Un-registers a callback to be called on given Power state transition(s).
 * Existing node is deleted from the linked list for handling the state transition callbacks.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_unsubscribe_ps_transition_event(
  sl_power_manager_ps_transition_event_handle_t *event_handle,
  const sl_power_manager_ps_transition_event_info_t *event_info)
{
  if ((event_handle) == NULL || (event_info == NULL)) {
    // Validates the null pointer, if incorrect returns the error code.
    return SL_STATUS_NULL_POINTER;
  }
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }
  SL_SI91X_POWER_MANAGER_CORE_ENTER_CRITICAL;
  // Pops out the data from the linked list.
  sl_slist_remove(&power_manager_ps_transition_event_list, &event_handle->node);
  SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
  // If it reaches here, then returns SL_STATUS_OK
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Transit to sleep mode and waits for the peripheral to configured a wakeup source
 * to wakeup the m4 soc.
 * If the sleep mode transition is not successful, then it returns error code.
 * Configuration of sleep parameters are handled in the internal function.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_sleep(void)
{
  sl_status_t status;

  if (!sli_si91x_power_manager_is_valid_transition(current_state, SL_SI91X_POWER_MANAGER_SLEEP)) {
    // Validates the state transition for sleep, if invalid returns error code.
    return SL_STATUS_INVALID_STATE;
  }
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }
#if (SL_SI91X_TICKLESS_MODE == 0)
  if (!sl_si91x_power_manager_is_ok_to_sleep()) {
    return SL_STATUS_BUSY;
  }
#endif
#ifdef SL_CATALOG_LOGGER_COMPONENT_PRESENT
  sl_log_api_core_t *sl_log_core_api = sl_log_get_api_core();
  sl_log_core_api->pre_sleep_process(NULL);
#endif
  do {
    // Internal function to change active mode to sleep mode is called.
    // It sets the required configurations and goes into sleep mode.
    status = sli_si91x_power_manager_set_sleep_configuration(current_state);
    if (status != SL_STATUS_OK) {
      return status;
    }
  } while (sl_si91x_power_manager_sleep_on_isr_exit());

  // After wakeup, clock is set to the particular PS4/PS3/PS2 mode.
  status = sl_si91x_power_manager_set_clock_scaling(clock_scaling_mode);
  if (status != SL_STATUS_OK) {
    return status;
  }
#ifdef SL_CATALOG_LOGGER_COMPONENT_PRESENT
  sl_log_core_api->post_sleep_process(NULL);
#endif
  // Notifies the state transition who has subscribed to it.
  notify_power_state_transition(SL_SI91X_POWER_MANAGER_SLEEP, current_state);
  // If it reaches here, then returns SL_STATUS_OK
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Transit to standby state and will wait for the interrupt.
 * WFI instruction is called to move the soc power mode to standby mode.
 ******************************************************************************/
void sl_si91x_power_manager_standby(void)
{
  if (sli_si91x_power_manager_is_valid_transition(current_state, SL_SI91X_POWER_MANAGER_STANDBY)) {
    // Validates the state transition for sleep, if valid it transits to standby mode.
    __WFI();
    notify_power_state_transition(SL_SI91X_POWER_MANAGER_STANDBY, current_state);
  }
}

/*******************************************************************************
 * Configures the wakeup sources.
 * Configuration of wakeup source is handled in the internal function.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_set_wakeup_sources(uint32_t source, boolean_t add)
{
  sl_status_t status;
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }

  status = sli_si91x_power_configure_wakeup_resource(source, add);
  return status;
}

/*******************************************************************************
 * Retains the RAM in low power state either by using size or RAM bank as input parameter.
 * All the RAM retention configuration parameters are handled in the internal function
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_configure_ram_retention(sl_power_ram_retention_config_t *config)
{
  sl_status_t status;
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }

  if (config == NULL) {
    // Validates the config, if null pointer, returns error code.
    return SL_STATUS_NULL_POINTER;
  }
  status = sli_si91x_power_manager_set_ram_retention_configuration(config);
  return status;
}

/*******************************************************************************
 * Configures the clock scaling.
 * According to the mode, clock scaling is configured.
 * Internal function handles the clock configuration as per the paramater passed
 * in it.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_set_clock_scaling(sl_clock_scaling_t mode)
{
  sl_status_t status;
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (mode >= LAST_ENUM_CLOCK_SCALING) {
    // Validates the clock scaling enum, if not in range returns error code.
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (mode == SL_SI91X_POWER_MANAGER_POWERSAVE) {
    SL_SI91X_POWER_MANAGER_CORE_ENTER_CRITICAL;
    // For powersave current state with false flag is passed as parameter
    clock_scaling_mode = SL_SI91X_POWER_MANAGER_POWERSAVE;
    // to the internal function.
    status = sli_si91x_clock_manager_config_clks_on_ps_change(current_state, clock_scaling_mode);
    SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
    return status;
  }
  if (mode == SL_SI91X_POWER_MANAGER_PERFORMANCE) {
    SL_SI91X_POWER_MANAGER_CORE_ENTER_CRITICAL;
    // For performance current state with true flag is passed as parameter
    clock_scaling_mode = SL_SI91X_POWER_MANAGER_PERFORMANCE;
    // to the internal function.
    status = sli_si91x_clock_manager_config_clks_on_ps_change(current_state, clock_scaling_mode);
    SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
    return status;
  }
  // If it reaches here, then the entered mode is invalid.
  return SL_STATUS_INVALID_PARAMETER;
}
/*******************************************************************************
 * Returns the current clock scaling mode of power state which is stored in static variable.
 ******************************************************************************/
sl_clock_scaling_t sl_si91x_power_manager_get_clock_scaling(void)
{
  return clock_scaling_mode;
}
/*******************************************************************************
 * Returns the current power state of SoC which is stored in static variable.
 ******************************************************************************/
sl_power_state_t sl_si91x_power_manager_get_current_state(void)
{
  return current_state;
}

/*******************************************************************************
 * Returns the pointer to the array of requirement table.
 ******************************************************************************/
uint8_t *sl_si91x_power_manager_get_requirement_table(void)
{
  return requirement_ps_table;
}

/*******************************************************************************
 * De-initialize the power manager service.
 * It clears the requirement table and the is_initialized flag.
 ******************************************************************************/
void sl_si91x_power_manager_deinit(void)
{
  // Clears the requirement table and disable the initialization flag.
  memset(requirement_ps_table, 0, sizeof(requirement_ps_table));
  is_initialized = false;
}

/*******************************************************************************
 * Updates the power state requirement, requirement table and current state variable.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_update_ps_requirement(sl_power_state_t state, boolean_t add)
{
#if (SL_SI91X_TICKLESS_MODE == 1)
  sl_wifi_performance_profile_v2_t pm_ta_performance_profile;
  sl_wifi_get_performance_profile_v2(&pm_ta_performance_profile);
#endif
  if (!is_initialized) {
    // Validate the status of power manager service, if not initialized
    // returns error code.
    return SL_STATUS_NOT_INITIALIZED;
  }
  if (state > SL_SI91X_POWER_MANAGER_PS4) {
    // Validate the power state, if not in range returns error code.
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validates the transition, if incorrect returns error code.
  if ((requirement_ps_table[state] == PS_MIN_COUNTER) && !add) {
    // If requirement is to remove when it 0, i.e., user tries to
    // make the requirement less than 0 (wrap around not allowed), returns error code.
    return SL_STATUS_INVALID_PARAMETER;
  }
  if ((requirement_ps_table[state] == UINT8_MAX) && add) {
    // If requirement is to add when it is 255, i.e., user tries to
    // make the requirement more than 255 (wrap around not allowed), returns error code.
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Updates the requirement table.
  requirement_ps_table[state] += (uint8_t)((add) ? 1 : -1);
  state = sl_si91x_get_lowest_ps();
  if ((current_state != state) && (state != SL_SI91X_POWER_MANAGER_SLEEP)) {
    if (sl_si91x_power_manager_get_ps1_state_status() == true) {
      // Notifies the state transition who has subscribed to it.
      notify_power_state_transition(SL_SI91X_POWER_MANAGER_PS2, SL_SI91X_POWER_MANAGER_PS1);
    }
    // It updates the power state using internal api.
    if (sli_si91x_power_manager_change_power_state(current_state, state) != SL_STATUS_OK) {
      return SL_STATUS_INVALID_PARAMETER;
    }
    if (current_state == SL_SI91X_POWER_MANAGER_PS2 && state == SL_SI91X_POWER_MANAGER_PS1) {
      // If transition is to PS1 state, its requirement needs to clear after wakeup.
      // It goes to sleep with retention with valid ULP peripheral based wakeup source.
      // Notifies the state transition who has subscribed to it.
      notify_power_state_transition(SL_SI91X_POWER_MANAGER_PS1, SL_SI91X_POWER_MANAGER_PS2);
      // Clears the PS1 requirement
      requirement_ps_table[SL_SI91X_POWER_MANAGER_PS1] -= 1;
      return SL_STATUS_OK;
    }

    // Notifies the state transition who has subscribed to it.
    notify_power_state_transition(current_state, state);
    // Updates the current state variable.
    current_state = state;
    // If it reaches here, then returns SL_STATUS_OK
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Notifies the power state transition to the event subscribers.
 * It creates a uint32_t variable named transition which consists the ored value of
 * the states, "From" and "To" transitions.
 * It searches the linked list and finds the subscribers who has subscribed for that
 * event. 
 * It masks the transition variable with the subscribers event mask and if it matches,
 * the callback function is called.
 ******************************************************************************/
static void notify_power_state_transition(sl_power_state_t from, sl_power_state_t to)
{
  sl_power_manager_ps_transition_event_handle_t *handle;
  sl_power_manager_ps_transition_event_t transition = 0;

  switch (to) {
    // Updates the transition variable with "To" which state the transition happened.
    case SL_SI91X_POWER_MANAGER_PS4:
      // Updates the transition variable with entering PS4 state flag.
      transition = SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS4;
      break;
    case SL_SI91X_POWER_MANAGER_PS3:
      // Updates the transition variable with entering PS3 state flag.
      transition = SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS3;
      break;
    case SL_SI91X_POWER_MANAGER_PS2:
      // Updates the transition variable with entering PS2 state flag.
      transition = SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS2;
      break;
    default:
      break;
  }

  switch (from) {
    // Updates the transition variable with "From" which state the transition happened.
    case SL_SI91X_POWER_MANAGER_PS4:
      // Updates the transition variable with leaving PS4 state flag.
      transition |= SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS4;
      break;
    case SL_SI91X_POWER_MANAGER_PS3:
      // Updates the transition variable with leaving PS3 state flag.
      transition |= SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS3;
      break;
    case SL_SI91X_POWER_MANAGER_PS2:
      // Updates the transition variable with leaving PS2 state flag.
      transition |= SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS2;
      break;
    case SL_SI91X_POWER_MANAGER_PS1:
      // Updates the transition variable with leaving PS1 state flag.
      transition |= SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS1;
      break;
    case SL_SI91X_POWER_MANAGER_SLEEP:
      // Updates the transition variable with leaving Sleep state flag.
      transition |= SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_SLEEP;
      break;
    case SL_SI91X_POWER_MANAGER_STANDBY:
      // Updates the transition variable with leaving Standby state flag.
      transition |= SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_STANDBY;
      break;
    default:
      break;
  }
  // For loop to search each entry of linked list for the notification of transition
  SL_SLIST_FOR_EACH_ENTRY(power_manager_ps_transition_event_list,
                          handle,
                          sl_power_manager_ps_transition_event_handle_t,
                          node)
  {
    if ((handle->info->event_mask & transition) > 0) {
      // Verifies if the subscription and event are same, if same callback function is called.
      handle->info->on_event(from, to);
    }
  }
}

/***************************************************************************/ /**
 * Last-chance check before sleep.
 *
 * @return  True, if the system should actually sleep.
 *          False, if not.
 *
 * @note This is the fall back implementation of the callback, it can be
 *       overridden by the application or other components.
 ******************************************************************************/
boolean_t sl_si91x_power_manager_is_ok_to_sleep(void)
{
  boolean_t is_sleep_ready = false;
#if (SL_SI91X_TICKLESS_MODE == 1)
  sl_wifi_performance_profile_v2_t pm_ta_performance_profile;
  sl_wifi_get_performance_profile_v2(&pm_ta_performance_profile);
  if (pm_ta_performance_profile.profile != DEEP_SLEEP_WITHOUT_RAM_RETENTION) {
    if ((SL_SI91X_POWER_MANAGER_SLEEP == sl_si91x_get_lowest_ps()) && (sli_si91x_is_sdk_ok_to_sleep())
        && (sli_si91x_ta_packet_initiated_to_m4())) {
      is_sleep_ready = true;
    } else {
    }
  }
#else
  is_sleep_ready = true;
#endif
  return is_sleep_ready;
}

/***************************************************************************/ /**
* Returns the Lowest possible power state from the requirement table.
* It validates all the power state requirements and return the lowest possible state transition.
* If power manager service is not initialized then it returns SL_STATUS_NOT_INITIALIZED, to initialize call \ref sl_si91x_power_manager_init.
*
* @pre Pre-conditions:
* - \ref sl_si91x_power_manager_init
*
*
* @param[in] none
* @return sl_power_state_t values are returned:
* - none
******************************************************************************/
sl_power_state_t sl_si91x_get_lowest_ps(void)
{
  uint8_t ps_counter;
  for (ps_counter = PS_MAX_COUNTER - 1;
       (ps_counter > PS_MIN_COUNTER) && (requirement_ps_table[ps_counter] == PS_MIN_COUNTER);
       ps_counter--) {
  }
  if ((ps_counter == PS_MIN_COUNTER) && (!requirement_ps_table[ps_counter])) {
    return SL_SI91X_POWER_MANAGER_SLEEP;
  } else {
    return (sl_power_state_t)ps_counter;
  }
}

/***************************************************************************/ /**
 * Check if the MCU can sleep after an interrupt.
 *
 * @return  True, if the system can sleep after the interrupt.
 *          False, otherwise.
 *
 * @note This is the fallback implementation of the callback, it can be
 *       overridden by the application or other components.
 ******************************************************************************/
__WEAK boolean_t sl_si91x_power_manager_sleep_on_isr_exit(void)
{
  return false;
}

/*****************************************************************************
 * @fn      sl_status_t sl_si91x_power_manager_request_ps1_state(void)
 *
 * @brief   Requests the PS1 state requirement during tickless idle mode.
 * 
 * @details This function requests the PS1 power state from the power manager during tickless idle mode. If the system is in the IDLE state, it transitions into the PS1 state.
 *
 * @return  Status code indicating the result:
 *          - SL_STATUS_OK (0x0000) - PS1 state requirement successfully added.
 *          - SL_STATUS_INVALID_STATE (0x0002) - Invalid request to add PS1 state.
 *          - SL_STATUS_NOT_INITIALIZED (0x0011) - Power manager service is not initialized.
 *          - SL_STATUS_INVALID_PARAMETER (0x0021) - Invalid parameter.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_request_ps1_state(void)
{
  sl_status_t status = SL_STATUS_OK;
  if (sli_si91x_pm_standby_state_active) {
    // If the standby state is requested, then it is not possible to add the PS1 state request.
    return SL_STATUS_INVALID_STATE;
  }
  if (!(M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS)) {
    // If the ULP mode is not enabled, then it is not possible to transition to the PS1 state.
    return SL_STATUS_INVALID_STATE;
  }
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_ULPSS_WAKEUP, true);
  if (status != SL_STATUS_OK) {
    // Returns the error code.
    return status;
  }
  sli_si91x_pm_ps1_state_active = true;
  return status;
}

/******************************************************************************
 * @fn      sl_status_t sl_si91x_power_manager_remove_ps1_state_request(void)
 * 
 * @brief   Remove the PS1 state request during tickless idle mode.
 * 
 * @details This function removes the PS1 state request from the power manager during tickless idle mode.
 * 
 * @return  Status code indicating the result:  
 *          - SL_STATUS_OK (0x0000) - Ps1 state requirement successfully removed.
 *          - SL_STATUS_INVALID_STATE (0x0002) - Invalid re quest to remove PS1 state.
 *          - SL_STATUS_NOT_INITIALIZED (0x0011) - Power manager service is not initialized.
 *          - SL_STATUS_INVALID_PARAMETER (0x0021) - Invalid parameter.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_remove_ps1_state_request(void)
{
  sl_status_t status = SL_STATUS_OK;
  if (!sli_si91x_pm_ps1_state_active) {
    // If the PS1 state is not requested, then it is not possible to remove the PS1 state request.
    return SL_STATUS_INVALID_STATE;
  }
  status = sl_si91x_power_manager_set_wakeup_sources(SL_SI91X_POWER_MANAGER_ULPSS_WAKEUP, false);
  if (status != SL_STATUS_OK) {
    // Returns the error code.
    return status;
  }
  sli_si91x_pm_ps1_state_active = false;
  return status;
}

/******************************************************************************
 * @fn      bool sl_si91x_power_manager_get_ps1_state_status(void)
 * 
 * @brief   Get the current ps1 state request status from the power manager during tickless idle mode.
 * 
 * @details This function indicates whether a PS1 state request has been added to the power manager during tickless idle mode.
 *
 * @return  Status code indicating the result:
 *          - true - PS1 state requirement is added.
 *          - false - PS1 state requirement is not added.
 ******************************************************************************/
bool sl_si91x_power_manager_get_ps1_state_status(void)
{
  return sli_si91x_pm_ps1_state_active;
}

/******************************************************************************
 * @fn      sl_status_t sl_si91x_power_manager_request_standby_state(void)
 * 
 * @brief   Requests the standby state during tickless idle mode.
 * 
 * @details This function requests the standby power state from the power manager during tickless idle mode. If the system is in the IDLE state, it transitions into the standby state.
 *
 * @return  Status code indicating the result:
 *          - SL_STATUS_OK (0x0000) - Standby state requirement successfully added.
 *          - SL_STATUS_INVALID_STATE (0x0002) - Invalid request to add standby state.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_request_standby_state(void)
{
  if (sli_si91x_pm_ps1_state_active) {
    // If the PS1 state is requested, then it is not possible to add the standby state request.
    return SL_STATUS_INVALID_STATE;
  }
  sli_si91x_pm_standby_state_active = true;
  return SL_STATUS_OK;
}

/*****************************************************************************
 * @fn      sl_status_t sl_si91x_power_manager_remove_standby_state_request(void)
 * 
 * @brief   Remove the standby state request during tickless idle mode.
 * 
 * @details This function removes the standby state request from the power manager during tickless idle mode.
 *
 * @return  Status code indicating the result:
 *          - SL_STATUS_OK (0x0000) : Standby state requirement successfully removed.
 *          - SL_STATUS_INVALID_STATE (0x0001) - Invalid request to remove standby state.
 ******************************************************************************/
sl_status_t sl_si91x_power_manager_remove_standby_state_request(void)
{
  if (!sli_si91x_pm_standby_state_active) {
    // If the standby state is not requested, then it is not possible to remove the standby state request.
    return SL_STATUS_INVALID_STATE;
  }
  sli_si91x_pm_standby_state_active = false;
  return SL_STATUS_OK;
}

/***************************************************************************
 * @fn      bool sl_si91x_power_manager_get_standby_state_status(void)
 * 
 * @brief   Get the current standby state request status during tickless idle mode.
 * 
 * @details This function indicates whether a standby state request has been added to the power manager during tickless idle mode.
 *
 * @return  Status code indicating the result:
 *          - true : Standby state requirement is added.
 *          - false : Standby state requirement is not added.
 ******************************************************************************/
bool sl_si91x_power_manager_get_standby_state_status(void)
{
  return sli_si91x_pm_standby_state_active;
}

/***************************************************************************
 * @fn      bool sl_si91x_power_manager_is_tx_command_in_progress(void)
 * 
 * @brief   Check if a TX command is currently in progress.
 * 
 * @details This function is used to check the TX command status. It returns true if
 *          a TX command is currently in progress, false otherwise.
 *
 * @pre Pre-conditions:
 * - WiFi component should be added to the project for this function to work properly.
 *   If WiFi component is not included, this function will always return false.
 *
 * @return  bool - true if TX command is in progress, false otherwise.
 *
 * @note    This function is useful for power management decisions and determining
 *          when the system is ready for sleep or other power state transitions.
 ******************************************************************************/
bool sl_si91x_power_manager_is_tx_command_in_progress(void)
{
#if SL_WIFI_COMPONENT_INCLUDED
  return sli_si91x_get_tx_command_status();
#endif
  return false;
}

/*****************************************************************************
 * @fn      bool sl_si91x_power_manager_ps2_pre_check(void)
 * 
 * @brief   Performs pre-transition checks before entering PS2 state.
 * 
 * @details This function verifies whether the Network Wireless Processor (NWP) has any pending 
 *          packet transfers to the M4 core. If pending packets are detected, it returns false 
 *          to prevent PS2 state transition and avoid data loss. Wi-Fi component must be included
 *          in the project for this function to operate correctly.
 *
 * @return  Status code indicating the result:
 *          - true - Safe to enter PS2 state (no pending packets).
 *          - false - Pending packets detected, PS2 transition should be blocked.
 ******************************************************************************/
bool sl_si91x_power_manager_ps2_pre_check(void)
{
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
  __asm volatile("cpsid i" ::: "memory");
  __asm volatile("dsb");
  __asm volatile("isb");
  // Check if the system is safe to enter PS2 state.
  bool is_safe_to_enter_ps2 = (sli_si91x_ta_packet_initiated_to_m4() && sli_si91x_is_sdk_ok_to_sleep());
  if (is_safe_to_enter_ps2 == true) {
    // Disable the 74 IRQ.
    NVIC_DisableIRQ(TASS_P2P_IRQn);
  }
  // Enable the NVIC interrupts.
  __asm volatile("cpsie i" ::: "memory");
  return is_safe_to_enter_ps2;
#else
  return true;
#endif
}
