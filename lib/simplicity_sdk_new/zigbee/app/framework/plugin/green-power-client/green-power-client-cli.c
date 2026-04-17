/***************************************************************************//**
 * @file
 * @brief CLI for the Green Power Client plugin.
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

#include "stack/gp/sli-gp-proxy-table.h"
#include "stack/include/gp-proxy-table.h"
#include "green-power-client.h"
#include "app/framework/plugin/green-power-common/green-power-common.h"
#include "app/util/serial/sl_zigbee_command_interpreter.h"

#ifndef SL_ZIGBEE_AF_GENERATE_CLI
#error The Green Power Client plugin is not compatible with the legacy CLI.
#endif

extern uint8_t sli_zigbee_gp_proxy_table_size;
extern sl_zigbee_gp_proxy_table_entry_t sli_zigbee_gp_proxy_table[];

void sl_zigbee_af_green_power_client_set_proxy_entry(SL_CLI_COMMAND_ARG)
{
#ifndef EZSP_HOST
  uint8_t index = sl_cli_get_argument_uint8(arguments, 0);
  uint32_t srcID = sl_cli_get_argument_uint32(arguments, 1);
  uint16_t sinkNodeID = sl_cli_get_argument_uint16(arguments, 2);
  sl_zigbee_gp_proxy_table_entry_t entry;

  entry.gpd.applicationId = 0;
  entry.gpd.id.sourceId = srcID;
  entry.options = sl_cli_get_argument_uint32(arguments, 3);
  entry.status = SL_ZIGBEE_GP_PROXY_TABLE_ENTRY_STATUS_ACTIVE;
  entry.securityOptions = 0;
  entry.sinkList[0].type = SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST;
  entry.sinkList[1].type = SL_ZIGBEE_GP_SINK_TYPE_UNUSED;
  entry.sinkList[0].target.unicast.sinkNodeId = sinkNodeID;
  entry.sinkList[0].target.unicast.sinkEUI[0] = 0x84;
  entry.sinkList[0].target.unicast.sinkEUI[1] = 0x40;
  entry.sinkList[0].target.unicast.sinkEUI[2] = 0x18;
  entry.sinkList[0].target.unicast.sinkEUI[3] = 0x00;
  entry.sinkList[0].target.unicast.sinkEUI[4] = 0x00;
  entry.sinkList[0].target.unicast.sinkEUI[5] = 0x00;
  entry.sinkList[0].target.unicast.sinkEUI[6] = 0x00;
  entry.sinkList[0].target.unicast.sinkEUI[7] = 0x00;
  entry.gpdSecurityFrameCounter = 0x00;

  sli_zigbee_gp_proxy_table_set_entry(index, &entry);
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_af_green_power_client_add_sink(SL_CLI_COMMAND_ARG)
{
#ifndef EZSP_HOST
  sl_zigbee_gp_address_t addr;
  uint32_t srcID = sl_cli_get_argument_uint32(arguments, 0);
  sl_802154_long_addr_t eui64;
  sl_zigbee_copy_eui64_arg(arguments, 1, eui64, true);
  addr.applicationId = 0;
  addr.id.sourceId = srcID;

  uint8_t index = sli_zigbee_gp_proxy_table_find_or_allocate_entry(&addr);
  if (index != 0xFF) {
    sli_zigbee_gp_proxy_table_add_sink(index,
                                       SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST,
                                       eui64,
                                       SL_ZIGBEE_UNKNOWN_NODE_ID,
                                       0,
                                       0xFFFF);
  }
#else
  UNUSED_VAR(arguments);
#endif
}
void sl_zigbee_af_green_power_client_add_groupcast_sink(SL_CLI_COMMAND_ARG)
{
#ifndef EZSP_HOST
  sl_zigbee_gp_address_t addr;
  uint32_t srcID = sl_cli_get_argument_uint32(arguments, 0);
  uint16_t groupID = sl_cli_get_argument_uint16(arguments, 1);
  addr.applicationId = 0;
  addr.id.sourceId = srcID;

  uint8_t index = sli_zigbee_gp_proxy_table_find_or_allocate_entry(&addr);
  if (index != 0xFF) {
    sli_zigbee_gp_proxy_table_add_sink(index,
                                       SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST,
                                       0,
                                       SL_ZIGBEE_UNKNOWN_NODE_ID,
                                       groupID,
                                       0xFFFF);
  }
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_af_green_power_client_remove_proxy_table_entry(SL_CLI_COMMAND_ARG)
{
  sl_zigbee_gp_address_t addr;
  uint32_t srcID = sl_cli_get_argument_uint32(arguments, 0);
  addr.applicationId = 0;
  addr.id.sourceId = srcID;
  sl_zigbee_gp_proxy_table_remove_entry(sl_zigbee_gp_proxy_table_lookup(&addr));
}

void sl_zigbee_af_green_power_client_print_proxy_table(SL_CLI_COMMAND_ARG)
{
  UNUSED_VAR(arguments);
#ifndef EZSP_HOST
  uint8_t i, j;
  uint8_t cleared = 0;
  sl_zigbee_af_green_power_cluster_print("Proxy Table:\n");
  for (i = 0; i < sli_zigbee_gp_proxy_table_size; i++) {
    if (sli_zigbee_gp_proxy_table_entry_in_use(i)) {
      sl_zigbee_af_core_print("%d opt:%08X seco:%02X srcID:%08X ", i, sli_zigbee_gp_proxy_table[i].options, sli_zigbee_gp_proxy_table[i].securityOptions, sli_zigbee_gp_proxy_table[i].gpd.id.sourceId);
      for (j = 0; j < 2; j++) {
        if (sli_zigbee_gp_proxy_table[i].sinkList[j].type == SL_ZIGBEE_GP_SINK_TYPE_UNUSED) {
          sl_zigbee_af_core_print("unused");
        } else if (sli_zigbee_gp_proxy_table[i].sinkList[j].type == SL_ZIGBEE_GP_SINK_TYPE_GROUPCAST) {
          sl_zigbee_af_core_print("GC %04X", sli_zigbee_gp_proxy_table[i].sinkList[j].target.groupcast.groupID);
        } else if (sli_zigbee_gp_proxy_table[i].sinkList[j].type == SL_ZIGBEE_GP_SINK_TYPE_LW_UNICAST) {
          sl_zigbee_af_core_print("LU:");
          sl_zigbee_af_print_big_endian_eui64(sli_zigbee_gp_proxy_table[i].sinkList[j].target.unicast.sinkEUI);
        }
        sl_zigbee_af_core_print(" ");
      }
      sl_zigbee_af_core_print(" ");
      for (j = 0; j < SL_ZIGBEE_ENCRYPTION_KEY_SIZE; j++) {
        sl_zigbee_af_core_print("%02X", sli_zigbee_gp_proxy_table[i].gpdKey.contents[j]);
      }
      sl_zigbee_af_core_print(" ");
      sl_zigbee_af_core_print("%02X", sli_zigbee_gp_proxy_table[i].gpdSecurityFrameCounter);
      sl_zigbee_af_core_print("\n");
    } else {
      cleared++;
    }
  }
  if (cleared == sli_zigbee_gp_proxy_table_size) {
    sl_zigbee_af_core_println("No Proxy Entries");
  }
#else
  uint16_t proxyTableSize = 0;
  sl_zigbee_gp_proxy_table_entry_t entry;
  sl_status_t status = sl_zigbee_ezsp_get_configuration_value(SL_ZIGBEE_EZSP_CONFIG_GP_PROXY_TABLE_SIZE, (uint16_t*)&proxyTableSize);
  if (status != SL_ZIGBEE_EZSP_SUCCESS) {
    sl_zigbee_af_green_power_cluster_println("ERR: Cannot get the proxy table size from GP stack, error code: %02X", status);
    return;
  }
  uint8_t i, j;
  sl_zigbee_af_green_power_cluster_print("Proxy Table:\n");
  for (i = 0; i < (uint8_t)proxyTableSize; i++) {
    sl_zigbee_gp_proxy_table_get_entry(i, &entry);
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

#endif
}

void sl_zigbee_af_green_power_client_duplicate_filtering_test(SL_CLI_COMMAND_ARG)
{
#ifndef EZSP_HOST
  sl_zigbee_gp_address_t sender;
  sender.endpoint = sl_cli_get_argument_uint8(arguments, 0);
  sender.applicationId = SL_ZIGBEE_GP_APPLICATION_SOURCE_ID;
  sender.id.sourceId = sl_cli_get_argument_uint32(arguments, 1);
  sli_zigbee_af_gp_message_checking(&sender, sl_cli_get_argument_uint8(arguments, 2));
  //sli_zigbee_af_green_power_find_duplicate_mac_seq_num(&sender, (uint8_t)sl_zigbee_unsigned_command_argument(2));
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_af_green_power_client_set_key(SL_CLI_COMMAND_ARG)
{
#ifndef EZSP_HOST
  uint8_t index;
  sl_zigbee_key_data_t keyData;
  index = sl_cli_get_argument_uint8(arguments, 0);
  sl_zigbee_copy_hex_arg(arguments,                        \
                         1,                                \
                         sl_zigbee_key_contents(&keyData), \
                         SL_ZIGBEE_ENCRYPTION_KEY_SIZE,    \
                         true);
  sli_zigbee_gp_proxy_table_set_key(index, (keyData.contents), 0 /*key type TODO*/);
#else
  UNUSED_VAR(arguments);
#endif
}

void sl_zigbee_af_green_power_client_spoof_dev_annce(SL_CLI_COMMAND_ARG)
{
  uint16_t nodeId = sl_cli_get_argument_uint16(arguments, 0);
  sl_802154_long_addr_t eui64;

  sl_zigbee_copy_eui64_arg(arguments, 1, eui64, true);
  uint8_t capabilities = sl_cli_get_argument_uint8(arguments, 2);
  sli_zigbee_af_gp_spoof_device_annce(nodeId,
                                      eui64,
                                      capabilities);
}
void sl_zigbee_af_green_power_client_clear_proxy_table_command(SL_CLI_COMMAND_ARG)
{
  UNUSED_VAR(arguments);
  sl_zigbee_af_green_power_client_clear_proxy_table();
}
