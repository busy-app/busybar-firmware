/***************************************************************************//**
 * @file
 * @brief CLI interface to manage the concentrator's periodic MTORR broadcasts.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/concentrator/concentrator-support.h"
#include "stack/include/source-route.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "concentrator-config.h"

#define SOURCE_ROUTE_NULL_INDEX 0xFF

void sl_zigbee_af_concentrator_print_source_route_table(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint8_t i;
  for (i = 0; i < sl_zigbee_af_get_source_route_table_filled_size(); i++) {
    uint8_t index = i;
    uint8_t closerIndex;
    sl_802154_short_addr_t destination;
    sl_status_t status = sl_zigbee_af_get_source_route_table_entry(index,
                                                                   &destination,
                                                                   &closerIndex);
    if (status != SL_STATUS_OK) {
      sl_zigbee_af_app_println("0 of 0 total entries.");
      return;
    }

    sl_zigbee_af_app_print("%d: 0x%04X -> ",
                           index, destination);

    while (closerIndex != SOURCE_ROUTE_NULL_INDEX) {
      index = closerIndex;
      status = sl_zigbee_af_get_source_route_table_entry(index,
                                                         &destination,
                                                         &closerIndex);
      if (status == SL_STATUS_OK) {
        sl_zigbee_af_app_print("0x%04X -> ", destination);
      } else {
        sl_zigbee_af_app_print("NULL ");
        return;
      }
    }
    sl_zigbee_af_app_println("0x%04X (Me)", sl_zigbee_af_get_node_id());
  }
  sl_zigbee_af_app_println("%d of %d total entries.",
                           sl_zigbee_af_get_source_route_table_filled_size(),
                           sl_zigbee_af_get_source_route_table_total_size());
}

void sl_zigbee_af_concentrator_status(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_af_app_println("Type:  %s RAM",
                           ((SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE
                             == SL_ZIGBEE_LOW_RAM_CONCENTRATOR)
                            ? "Low"
                            : "High"));

  sl_zigbee_af_app_println("Min Time Between Broadcasts (sec): %d",
                           SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS);
  sl_zigbee_af_app_println("Max Time Between Broadcasts (sec): %d",
                           SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS);
  sl_zigbee_af_app_println("Max Hops: %d",
                           (SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MAX_HOPS == 0
                            ? SL_ZIGBEE_MAX_HOPS
                            : SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_MAX_HOPS));
  sl_zigbee_af_app_println("Route Error Threshold:      %d",
                           SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTE_ERROR_THRESHOLD);
  sl_zigbee_af_app_println("Delivery Failure Threshold: %d",
                           SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD);
}

void sl_zigbee_af_concentrator_start_discovery_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint32_t qsLeft =  sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_ON);
  sl_zigbee_af_app_println("%d sec until next MTORR broadcast", (qsLeft >> 2));
}

void sl_zigbee_af_concentrator_stop_discovery_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_OFF);
}

void sl_zigbee_af_concentrator_aggregation_command(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_RESCHEDULE);
}

void sl_zigbee_af_concentrator_set_router_behavior_command(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_plugin_concentrator_router_behavior_t behavior
    = (sl_zigbee_af_plugin_concentrator_router_behavior_t)sl_cli_get_argument_uint8(arguments, 0);

  if (behavior > SL_ZIGBEE_AF_PLUGIN_CONCENTRATOR_ROUTER_BEHAVIOR_MAX) {
    sl_zigbee_af_core_println("Illegal concentrator router behavior: 0x%02X", behavior);
  } else {
    sl_zigbee_af_core_println("Set concentrator router behavior to: 0x%02X", behavior);
    if (behavior != sl_zigbee_af_concentrator_get_router_behavior()) {
      sl_status_t status = sl_zigbee_af_concentrator_change_router_behavior(behavior);
      if (status != SL_STATUS_OK) {
        sl_zigbee_af_core_println("Set concentrator router behavior failed: 0x%02X", status);
      }
    }
  }
}

void sl_zigbee_af_concentrator_start_discovery(void)
{
  uint32_t qsLeft =  sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_ON);
  sl_zigbee_af_app_println("%d sec until next MTORR broadcast", (qsLeft >> 2));
}

void sl_zigbee_af_concentrator_stop_discovery(void)
{
  sl_zigbee_set_source_route_discovery_mode(SL_ZIGBEE_SOURCE_ROUTE_DISCOVERY_OFF);
}
