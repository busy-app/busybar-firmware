/***************************************************************************//**
 * @file
 * @brief CLI for the Reporting plugin.
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
#include "app/util/serial/sl_zigbee_command_interpreter.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/plugin/reporting/reporting.h"

#include "app/util/serial/sl_zigbee_command_interpreter.h"

// plugin reporting print
void sli_zigbee_af_reporting_cli_print(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint16_t i;
  for (i = 0; i < sli_zigbee_af_reporting_num_entries(); i++) {
    sl_zigbee_af_plugin_reporting_entry_t entry;
    sli_zigbee_af_reporting_get_entry(i, &entry);
    sl_zigbee_af_reporting_print("%02X:", i);
    if (entry.endpoint != SL_ZIGBEE_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID) {
      sl_zigbee_af_reporting_print("ep %02X clus %04X attr %04X svr %c",
                                   entry.endpoint,
                                   entry.clusterId,
                                   entry.attributeId,
                                   (entry.mask == CLUSTER_MASK_SERVER ? 'y' : 'n'));
      if (entry.manufacturerCode != SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE) {
        sl_zigbee_af_reporting_print(" mfg %02X", entry.manufacturerCode);
      }
      if (entry.direction == SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED) {
        sl_zigbee_af_reporting_print(" report min %04X max %04X rpt-chg %08X",
                                     entry.data.reported.minInterval,
                                     entry.data.reported.maxInterval,
                                     entry.data.reported.reportableChange);
        sl_zigbee_af_reporting_flush();
      } else {
        sl_zigbee_af_reporting_print(" receive from %04X ep %02X timeout %04X",
                                     entry.data.received.source,
                                     entry.data.received.endpoint,
                                     entry.data.received.timeout);
      }
    }
    sl_zigbee_af_reporting_println("");
    sl_zigbee_af_reporting_flush();
    // EMZIGBEE-5125: apps with lot's of endpoints/reporting table configs will
    // watchdog when printing
    halResetWatchdog();
  }
}

// plugin reporting clear
void sli_zigbee_af_reporting_cli_clear(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t status = sl_zigbee_af_clear_report_table_cb();
  sl_zigbee_af_reporting_println("%s 0x%02X", "clear", status);
}

// plugin reporting remove <index:1>
void sli_zigbee_af_reporting_cli_remove(sl_cli_command_arg_t *arguments)
{
  sl_status_t status = sli_zigbee_af_reporting_remove_entry(sl_cli_get_argument_uint16(arguments, 0));
  sl_zigbee_af_reporting_println("%s 0x%02X", "remove", status);
}

// plugin reporting add <endpoint:1> <cluster id:2> <attribute id:2> ...
// ... <mask:1> <min interval:2> <max interval:2> <reportable change:4>
void sli_zigbee_af_reporting_cli_add(sl_cli_command_arg_t *arguments)
{
  sl_zigbee_af_status_t status = SL_ZIGBEE_ZCL_STATUS_SUCCESS;
  sl_zigbee_af_plugin_reporting_entry_t entry = {
    .endpoint = (int8u)sl_cli_get_argument_uint8(arguments, 0),
    .clusterId = (sl_zigbee_af_cluster_id_t)sl_cli_get_argument_uint16(arguments, 1),
    .attributeId = (sl_zigbee_af_attribute_id_t)sl_cli_get_argument_uint16(arguments, 2),
    .mask = (int8u)(sl_cli_get_argument_uint8(arguments, 3) == 0
                    ? CLUSTER_MASK_CLIENT
                    : CLUSTER_MASK_SERVER),
    .manufacturerCode = SL_ZIGBEE_AF_NULL_MANUFACTURER_CODE,
    .data.reported.minInterval = (int16u)sl_cli_get_argument_uint16(arguments, 4),
    .data.reported.maxInterval = (int16u)sl_cli_get_argument_uint16(arguments, 5),
    .data.reported.reportableChange = (int32u)sl_cli_get_argument_uint32(arguments, 6),
    .direction = SL_ZIGBEE_ZCL_REPORTING_DIRECTION_REPORTED,
  };
  UNUSED_VAR(status);
  status = sl_zigbee_af_reporting_configure_reported_attribute(&entry);

  sl_zigbee_af_reporting_println("%s 0x%02X", "add", status);
}

// plugin reporting add clear-last-report-time
void sli_zigbee_af_reporting_cli_clear_last_report_time(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint16_t i;
  for (i = 0; i < REPORT_TABLE_SIZE; i++) {
    sli_zigbee_af_report_volatile_data[i].lastReportTimeMs = halCommonGetInt32uMillisecondTick();
  }
  sl_zigbee_af_reporting_println("clearing last report time of all attributes");
}

void sl_zigbee_af_reporting_init_cb(uint8_t init_level);

static uint16_t rawTableScan(uint16_t numEntries)
{
  uint16_t i, acc;
  sl_zigbee_af_plugin_reporting_entry_t entry;
  acc = 0;
  for (i = 0; i < numEntries; i++) {
    sli_zigbee_af_reporting_get_entry(i, &entry);
    acc++;
  }
  return acc;
}

// plugin reporting test-timing
void sli_zigbee_af_reporting_cli_test_timing(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  uint32_t before, after;
  sl_zigbee_af_reporting_println("testing timing of various reporting operations");
  sl_zigbee_af_reporting_println("-----------------------------------------------");

  sl_zigbee_af_reporting_println("--table reads - full table--");
  before = halCommonGetInt32uMillisecondTick();
  rawTableScan(REPORT_TABLE_SIZE);
  after = halCommonGetInt32uMillisecondTick();
  sl_zigbee_af_reporting_println("result = %d ms", after - before);

  sl_zigbee_af_reporting_println("--init / schedule tick---");
  before = halCommonGetInt32uMillisecondTick();
  sl_zigbee_af_reporting_init_cb(SL_ZIGBEE_INIT_LEVEL_LOCAL_DATA);
  after = halCommonGetInt32uMillisecondTick();
  sl_zigbee_af_reporting_println("result = %d ms", after - before);

  sl_zigbee_af_reporting_println("--load configuration defaults--");
  before = halCommonGetInt32uMillisecondTick();
  sl_zigbee_af_reporting_load_reporting_config_defaults();
  after = halCommonGetInt32uMillisecondTick();
  sl_zigbee_af_reporting_println("result = %d ms", after - before);
}
