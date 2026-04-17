/***************************************************************************//**
 * @file
 * @brief CLI for the Green Power Server plugin.
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
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#ifdef SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "app/framework/include/af.h"
#else // !SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-adapter.h"
#endif //SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT
#include "green-power-server.h"
#include "green-power-common.h"

#ifndef EZSP_HOST
#include "stack/include/gp-sink-table.h"
#endif //!EZSP_HOST

#include "green-power-server-config.h"

sl_zigbee_af_zcl_request_status_t sli_zigbee_af_green_power_server_gp_sink_commissioning_mode_command_handler(uint8_t options,
                                                                                                              uint16_t gpmAddrForSecurity,
                                                                                                              uint16_t gpmAddrForPairing,
                                                                                                              uint8_t sinkEndpoint);

// Sink Commissioning Mode Enter
void sl_zigbee_af_green_power_server_commissioning_mode(SL_CLI_COMMAND_ARG)
{
  uint8_t options = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t gpmAddressSecurity = sl_cli_get_argument_uint16(arguments, 1);
  uint16_t gpmAddressPairing = sl_cli_get_argument_uint16(arguments, 2);
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 3);
  sli_zigbee_af_green_power_server_gp_sink_commissioning_mode_command_handler(options,
                                                                              gpmAddressSecurity,
                                                                              gpmAddressPairing,
                                                                              endpoint);
}

// Sink Commissioning Window Extend
void sl_zigbee_af_green_power_cluster_gp_sink_cli_commissioning_window_extend(SL_CLI_COMMAND_ARG)
{
  uint16_t commissioningWindow = sl_cli_get_argument_uint16(arguments, 0);
  sl_zigbee_af_green_power_cluster_gp_sink_commissioning_window_extend(commissioningWindow);
}

// Sink Commissioning Window Extend
void sl_zigbee_af_green_power_cluster_gp_sink_cli_commissioning_window_close(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  sl_zigbee_af_green_power_cluster_gp_sink_close_commissioning_window();
}

void sl_zigbee_af_green_power_server_cli_clear_sink_table(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  sl_zigbee_gp_sink_table_clear_all();
}

void sl_zigbee_af_green_power_server_cli_sink_table_print(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
  bool tableEmpty = true;
  uint16_t sinkTableSize = 0;
#ifdef EZSP_HOST
  sl_status_t status = sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_GP_SINK_TABLE_SIZE, (uint16_t*)&sinkTableSize);
  if (status != SL_ZIGBEE_EZSP_SUCCESS) {
    sl_zigbee_af_green_power_cluster_println("ERR: Cannot get the sink table size from GP stack, error code: %02X", status);
    return;
  }
#else
  sinkTableSize = SL_ZIGBEE_GP_SINK_TABLE_SIZE;
#endif // EZSP_HOST

  sl_zigbee_af_core_println("\n\rSt Optn Ap SourceId Ep Dv Alis Gr So FrameCtr");
  for (uint8_t index = 0; index < (uint8_t)sinkTableSize; index++) {
    sl_zigbee_gp_sink_table_entry_t entry = { 0 };
    sl_status_t status = sl_zigbee_gp_sink_table_get_entry(index, &entry);
    if (status == SL_STATUS_OK
        && entry.status == SL_ZIGBEE_GP_SINK_TABLE_ENTRY_STATUS_ACTIVE) {
      tableEmpty = false;
      if (entry.gpd.applicationId == 0) {
        sl_zigbee_af_core_println("%02X %04X %02X %08X %s %02X %04X %02X %02X %08X",
                                  entry.status,
                                  entry.options,
                                  entry.gpd.applicationId,
                                  entry.gpd.id.sourceId,
                                  "--",
                                  entry.deviceId,
                                  entry.assignedAlias,
                                  entry.groupcastRadius,
                                  entry.securityOptions,
                                  entry.gpdSecurityFrameCounter);
      } else {
        sl_zigbee_af_core_println("%02X %04X %02X %08X %02X %02X %04X %02X %02X %08X",
                                  entry.status,
                                  entry.options,
                                  entry.gpd.applicationId,
                                  entry.gpd.id.sourceId,
                                  entry.gpd.endpoint,
                                  entry.deviceId,
                                  entry.assignedAlias,
                                  entry.groupcastRadius,
                                  entry.securityOptions,
                                  entry.gpdSecurityFrameCounter);
      }
    }
  }
  if (tableEmpty) {
    sl_zigbee_af_core_println("\n\rSink Table Empty");
  }
}

void sl_zigbee_af_green_power_server_cli_run_test_vectors(SL_CLI_COMMAND_ARG)
{
  (void)arguments;
#if (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_SECURITY_TEST_VECTORS == 1)
  // A test to run the security test vectors upon reset
  sl_status_t status = sli_zigbee_af_gp_test_security();
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_green_power_cluster_println("~~ gp-test-vectors PASS ~~");
  } else {
    sl_zigbee_af_green_power_cluster_println("~~ gp-test-vectors FAIL ~~");
  }
#else // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_SECURITY_TEST_VECTORS == 0)
  sl_zigbee_af_core_println("Command is not supported");
#endif // (SL_ZIGBEE_AF_PLUGIN_GREEN_POWER_SERVER_SECURITY_TEST_VECTORS == 1)
}
