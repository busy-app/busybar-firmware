/***************************************************************************//**
 * @brief Zigbee Application Framework sleep code.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include PLATFORM_HEADER

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)

#include PLATFORM_HEADER
#include "sl_cli.h"
#include "sl_zigbee_system_common.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

#if defined(_SILICON_LABS_32B_SERIES_3)
#include "sl_device_clock.h"
#include "sl_clock_manager.h"
#endif //defined(_SILICON_LABS_32B_SERIES_3)

extern bool sli_zigbee_af_stay_awake_when_not_joined;
extern bool sli_zigbee_af_force_end_device_to_stay_awake;
extern void sl_zigbee_af_force_end_device_to_stay_awake(bool stayAwake);

void sl_zigbee_af_idle_sleep_status_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_zigbee_core_debug_println("Stay awake when not joined: %s",
                               (sli_zigbee_af_stay_awake_when_not_joined ? "yes" : "no"));
  sl_zigbee_core_debug_println("Forced stay awake:          %s",
                               (sli_zigbee_af_force_end_device_to_stay_awake ? "yes" : "no"));
}

void sl_zigbee_af_idle_sleep_stay_awake_command(sl_cli_command_arg_t *arguments)
{
  bool stayAwake = (bool)sl_cli_get_argument_uint8(arguments, 0);
  if (stayAwake) {
    sl_zigbee_core_debug_println("Forcing device to stay awake");
  } else {
    sl_zigbee_core_debug_println("Allowing device to go to sleep");
  }
  sl_zigbee_af_force_end_device_to_stay_awake(stayAwake);
}

void sl_zigbee_af_idle_sleep_awake_when_not_joined_command(sl_cli_command_arg_t *arguments)
{
  sli_zigbee_af_stay_awake_when_not_joined = (bool)sl_cli_get_argument_uint8(arguments, 0);
}

#if defined(_SILICON_LABS_32B_SERIES_3)
void sl_zigbee_af_idle_sleep_power_mode_performance_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status;
  status = sl_clock_manager_set_ext_flash_clk(SL_OSCILLATOR_FLPLL);
  sl_zigbee_core_debug_println("Switched to performance mode with status %02X", status);
}

void sl_zigbee_af_idle_sleep_power_mode_eco_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status;
  status = sl_clock_manager_set_ext_flash_clk(SL_OSCILLATOR_FSRCO);
  sl_zigbee_core_debug_println("Switched to power save mode with status %02X", status);
}
#if defined(SL_CATALOG_SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_PRESENT)
void sl_zigbee_af_radio_rx_duty_cycle_command(sl_cli_command_arg_t *arguments)
{
  sl_rail_handle_t *rail_handle = (sl_rail_handle_t *)sl_zigbee_get_rail_handle();
  sl_rail_status_t status = SL_RAIL_STATUS_INVALID_STATE;
  if (rail_handle) {
    status = sl_rail_ieee802154_config_2p4_ghz_radio_rx_duty_cycling(*rail_handle);
  }
  if (status == SL_RAIL_STATUS_NO_ERROR) {
    sl_zigbee_core_debug_println("Started radio RX duty cycle, status %02X", status);
  } else {
    sl_zigbee_core_debug_println("Failed to start RX duty cycle, status: %02X", status);
  }
}
#endif //SL_CATALOG_SL_RAIL_UTIL_IEEE802154_RX_DUTY_CYCLING_PRESENT
#endif //_SILICON_LABS_32B_SERIES_3

#endif //#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
