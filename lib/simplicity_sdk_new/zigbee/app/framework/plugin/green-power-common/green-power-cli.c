/***************************************************************************//**
 * @file
 * @brief A sample of custom EZSP protocol.
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
#endif // SL_COMPONENT_CATALOG_PRESENT
#include "app/framework/include/af.h"

#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_CLIENT_CLI_PRESENT
// Green Power Client CLIs
WEAK(void sl_zigbee_af_green_power_client_set_proxy_entry(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_client_add_sink(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_client_add_groupcast_sink(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_client_remove_proxy_table_entry(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_client_duplicate_filtering_test(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_client_set_key(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_client_print_proxy_table(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
#ifdef SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
  uint16_t proxyTableSize = 0;
  sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_GP_PROXY_TABLE_SIZE, &proxyTableSize);
  uint8_t i, j;

  sl_zigbee_af_green_power_cluster_print("Proxy Table:\n");
  for (i = 0; i < proxyTableSize; i++) {
    sl_zigbee_gp_proxy_table_entry_t entry = { 0 };
    if (sl_zigbee_ezsp_gp_proxy_table_get_entry(i, &entry) != SL_STATUS_OK) {
      return;
    }
    if (entry.status == SL_ZIGBEE_GP_PROXY_TABLE_ENTRY_STATUS_ACTIVE) {
      sl_zigbee_af_core_print("%d opt:%08X seco:%02X srcID:%08X ", i, entry.options, entry.securityOptions, entry.gpd.id.sourceId);
      for (j = 0; j < 2; j++) {
        if (entry.sinkList[j].type == SL_ZIGBEE_GP_SINK_TYPE_UNUSED) {
          sl_zigbee_af_core_print("unused");
        } else if (entry.sinkList[j].type == SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST) {
          sl_zigbee_af_core_print("GC %04X", entry.sinkList[j].target.groupcast.groupID);
        } else if (entry.sinkList[j].type == SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST) {
          sl_zigbee_af_core_print("LU:");
          sl_zigbee_af_print_big_endian_eui64(entry.sinkList[j].target.unicast.sinkEUI);
        }
        sl_zigbee_af_core_print(" ");
      }
      sl_zigbee_af_core_print(" ");
      for (j = 0; j < SL_ZIGBEE_ENCRYPTION_KEY_SIZE; j++) {
        sl_zigbee_af_core_print("%02X", entry.gpdKey.contents[j]);
      }
      sl_zigbee_af_core_print(" ");
      sl_zigbee_af_core_print("%02X", entry.gpdSecurityFrameCounter);
      sl_zigbee_af_core_print("\n");
    }
  }
#endif // SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
}

WEAK(void sl_zigbee_af_green_power_client_clear_proxy_table_command(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
#ifdef SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
  uint16_t proxyTableSize = 0;
  sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_GP_PROXY_TABLE_SIZE, &proxyTableSize);

  uint8_t entryIndex;
  for (entryIndex = 0; entryIndex < proxyTableSize; entryIndex++) {
    sl_zigbee_gp_proxy_table_entry_t entry = { 0 };
    if (sl_zigbee_ezsp_gp_proxy_table_get_entry(entryIndex, &entry) != SL_STATUS_OK) {
      return;
    }
    uint8_t allZeroesIeeeAddress[17];
    if (entry.status == SL_ZIGBEE_GP_PROXY_TABLE_ENTRY_STATUS_ACTIVE) {
      sl_zigbee_ezsp_gp_proxy_table_process_gp_pairing(SL_ZIGBEE_AF_GP_PAIRING_OPTION_REMOVE_GPD,
                                                       &(entry.gpd),
                                                       0, // commMode,
                                                       SL_ZIGBEE_NULL_NODE_ID,
                                                       0, // sinkGroupId,
                                                       SL_ZIGBEE_NULL_NODE_ID, // assignedAlias,
                                                       allZeroesIeeeAddress, //sinkIeeeAddress,
                                                       (sl_zigbee_key_data_t *) allZeroesIeeeAddress, // sl_zigbee_key_data_t* gpdKey,
                                                       0, // gpdSecurityFrameCounter,
                                                       0); // forwardingRadius)
    }
  }
#endif // SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
}

#endif //SL_CATALOG_ZIGBEE_GREEN_POWER_CLIENT_CLI_PRESENT

#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_CLI_PRESENT
// Green Power Server CLIs

// Sink Commissioning Window Extend
WEAK(void sl_zigbee_af_green_power_cluster_gp_sink_cli_commissioning_window_extend(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_cluster_gp_sink_cli_commissioning_window_close(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
  // Stub function.
}

WEAK(void sl_zigbee_af_green_power_server_commissioning_mode(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
#ifdef SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
  uint8_t options = sl_cli_get_argument_uint8(arguments, 0);
  uint16_t gpmAddressSecurity = sl_cli_get_argument_uint16(arguments, 1);
  uint16_t gpmAddressPairing = sl_cli_get_argument_uint16(arguments, 2);
  uint8_t endpoint = sl_cli_get_argument_uint8(arguments, 3);
  sl_status_t status = sl_zigbee_ezsp_gp_sink_commission(options,
                                                         gpmAddressSecurity,
                                                         gpmAddressPairing,
                                                         endpoint);
  sl_zigbee_af_core_println("\n\rSink Commissioning Enter %s",
                            (status == SL_STATUS_OK) ? "success" : "error");
#endif // SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
}

// Sink Table Print
WEAK(void sl_zigbee_af_green_power_server_cli_sink_table_print(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
#ifdef SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
  uint16_t sinkTableSize = 0;
  sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_GP_SINK_TABLE_SIZE, &sinkTableSize);
  bool tableEmpty = true;
  sl_zigbee_af_core_println("\n\rSink Table Size: %d", sinkTableSize);
  for (uint8_t index = 0; index < sinkTableSize; index++) {
    sl_zigbee_gp_sink_table_entry_t entry = { 0 };
    sl_status_t status = sl_zigbee_ezsp_gp_sink_table_get_entry(index, &entry);
    if (status == SL_STATUS_OK
        && entry.status == SL_ZIGBEE_GP_SINK_TABLE_ENTRY_STATUS_ACTIVE) {
      tableEmpty = false;
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
  if (tableEmpty) {
    sl_zigbee_af_core_println("\n\rSink Table Empty");
  }
#endif // SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_sink_table(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
#ifdef SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
  sl_zigbee_ezsp_gp_sink_table_clear_all();
#endif // SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
}

WEAK(void sl_zigbee_af_green_power_server_cli_run_test_vectors(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
  // A test to run the security test vectors upon reset
  sl_status_t status = sl_zigbee_ezsp_gp_security_test_vectors();
  if (status == SL_STATUS_OK) {
    sl_zigbee_af_green_power_cluster_println("~~ gp-test-vectors PASS ~~");
  } else if (status == SL_STATUS_NOT_AVAILABLE) {
    sl_zigbee_af_green_power_cluster_println("Command is not supported");
  } else {
    sl_zigbee_af_green_power_cluster_println("~~ gp-test-vectors FAIL ~~");
  }
}
#endif //SL_CATALOG_ZIGBEE_GREEN_POWER_SERVER_CLI_PRESENT

#ifdef SL_CATALOG_ZIGBEE_GREEN_POWER_TRANSLATION_TABLE_CLI_PRESENT
// Green Power Translation Table CLIs

WEAK(void sli_zigbee_gp_print_additional_info_block(uint8_t gpdCommand, uint8_t addInfoOffset))
{
  (void)gpdCommand;
  (void)addInfoOffset;
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_additional_info_block_table(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_customized_table(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_additional_info_block_table_entry(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_customized_table_entry(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_translation_table_entry(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_set_additional_info_block_table_entry(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_set_customized_table_entry(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_set_translation_table(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_additional_info_block_table_print(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_customized_table_print(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_translation_table_print(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_remove_gpd_endpoint(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
}

WEAK(void sl_zigbee_af_green_power_server_cli_clear_translation_translation_table(SL_CLI_COMMAND_ARG))
{
  (void)arguments;
#ifdef SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
  sl_zigbee_ezsp_gp_translation_table_clear();
#endif // SL_CATALOG_ZIGBEE_AF_SUPPORT_PRESENT
}
#endif //SL_CATALOG_ZIGBEE_GREEN_POWER_TRANSLATION_TABLE_CLI_PRESENT
