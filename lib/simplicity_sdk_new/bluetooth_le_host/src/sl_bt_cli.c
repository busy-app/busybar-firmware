/***************************************************************************//**
 * @brief SL_BT_API commands and Bluetooth stack C APIs
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_bt_api.h"
#include "sl_cli.h"
#include "sl_bt_cli.h"
#include "printf.h"
#define MAX_P_SIZE (255)
static void print_hex(uint8_t * addr,size_t len)
{
  printf("{ ");
  while(len--)
  {
    printf("%02x ",*addr++);
  }
  printf("} ");
}
void sli_bt_cli_system_hello(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_system_hello(
  );

    printf("rsp_system_hello 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_start_bluetooth(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_system_start_bluetooth(
  );

    printf("rsp_system_start_bluetooth 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_stop_bluetooth(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_system_stop_bluetooth(
  );

    printf("rsp_system_stop_bluetooth 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_forcefully_stop_bluetooth(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_system_forcefully_stop_bluetooth(
  );

    printf("rsp_system_forcefully_stop_bluetooth 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_get_version(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  uint16_t major;
  uint16_t minor;
  uint16_t patch;
  uint16_t build;
  uint32_t bootloader;
  uint32_t hash;
  status=sl_bt_system_get_version(
  &major,
  &minor,
  &patch,
  &build,
  &bootloader,
  &hash
  );

    printf("rsp_system_get_version 0x%lx ",status);
    printf("0x%x ",major);
    printf("0x%x ",minor);
    printf("0x%x ",patch);
    printf("0x%x ",build);
    printf("0x%x ",bootloader);
    printf("0x%x ",hash);
    printf("\n");
}
void sli_bt_cli_system_reboot(sl_cli_command_arg_t *arguments)
{

  (void)(arguments);
  // parameters
  //return values
sl_bt_system_reboot(
  );

}
void sli_bt_cli_system_halt(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t halt=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_system_halt(
  halt
  );

    printf("rsp_system_halt 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_linklayer_configure(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t key=sl_cli_get_argument_uint8(arguments,0);
  size_t data_len;
  uint8_t *data=sl_cli_get_argument_hex(arguments,1,&data_len);
  //return values
  status=sl_bt_system_linklayer_configure(
  key,
  data_len,
  data
  );

    printf("rsp_system_linklayer_configure 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_set_tx_power(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  int16_t min_power=sl_cli_get_argument_int16(arguments,0);
  int16_t max_power=sl_cli_get_argument_int16(arguments,1);
  //return values
  int16_t set_min;
  int16_t set_max;
  status=sl_bt_system_set_tx_power(
  min_power,
  max_power,
  &set_min,
  &set_max
  );

    printf("rsp_system_set_tx_power 0x%lx ",status);
    printf("%d ",set_min);
    printf("%d ",set_max);
    printf("\n");
}
void sli_bt_cli_system_get_tx_power_setting(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  int16_t support_min;
  int16_t support_max;
  int16_t set_min;
  int16_t set_max;
  int16_t rf_path_gain;
  status=sl_bt_system_get_tx_power_setting(
  &support_min,
  &support_max,
  &set_min,
  &set_max,
  &rf_path_gain
  );

    printf("rsp_system_get_tx_power_setting 0x%lx ",status);
    printf("%d ",support_min);
    printf("%d ",support_max);
    printf("%d ",set_min);
    printf("%d ",set_max);
    printf("%d ",rf_path_gain);
    printf("\n");
}
void sli_bt_cli_system_set_identity_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t type=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_system_set_identity_address(
  address,
  type
  );

    printf("rsp_system_set_identity_address 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_get_identity_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  bd_addr address;
  uint8_t type;
  status=sl_bt_system_get_identity_address(
  &address,
  &type
  );

    printf("rsp_system_get_identity_address 0x%lx ",status);
    print_hex(address.addr,sizeof(address.addr));
    printf("0x%x ",type);
    printf("\n");
}
void sli_bt_cli_system_get_random_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t length=sl_cli_get_argument_uint8(arguments,0);
  //return values
  size_t data_len;
  uint8_t data[MAX_P_SIZE];
  status=sl_bt_system_get_random_data(
  length,
  MAX_P_SIZE,
  &data_len,
  data
  );

    printf("rsp_system_get_random_data 0x%lx ",status);
    print_hex(data,data_len);
    printf("\n");
}
void sli_bt_cli_system_data_buffer_write(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t data_len;
  uint8_t *data=sl_cli_get_argument_hex(arguments,0,&data_len);
  //return values
  status=sl_bt_system_data_buffer_write(
  data_len,
  data
  );

    printf("rsp_system_data_buffer_write 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_data_buffer_clear(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_system_data_buffer_clear(
  );

    printf("rsp_system_data_buffer_clear 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_system_get_counters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t reset=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint16_t tx_packets;
  uint16_t rx_packets;
  uint16_t crc_errors;
  uint16_t failures;
  status=sl_bt_system_get_counters(
  reset,
  &tx_packets,
  &rx_packets,
  &crc_errors,
  &failures
  );

    printf("rsp_system_get_counters 0x%lx ",status);
    printf("0x%x ",tx_packets);
    printf("0x%x ",rx_packets);
    printf("0x%x ",crc_errors);
    printf("0x%x ",failures);
    printf("\n");
}
void sli_bt_cli_system_set_lazy_soft_timer(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t time=sl_cli_get_argument_uint32(arguments,0);
  uint32_t slack=sl_cli_get_argument_uint32(arguments,1);
  uint8_t handle=sl_cli_get_argument_uint8(arguments,2);
  uint8_t single_shot=sl_cli_get_argument_uint8(arguments,3);
  //return values
  status=sl_bt_system_set_lazy_soft_timer(
  time,
  slack,
  handle,
  single_shot
  );

    printf("rsp_system_set_lazy_soft_timer 0x%lx ",status);
    printf("\n");
}
#ifdef SL_CATALOG_BLUETOOTH_FEATURE_LINKLAYER_INTERFACE_PRESENT
void sli_bt_cli_linklayer_event_info_reporting_enable(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t enable=sl_cli_get_argument_uint8(arguments,0);
  uint32_t configuration=sl_cli_get_argument_uint32(arguments,1);
  uint8_t procedure_type=sl_cli_get_argument_uint8(arguments,2);
  size_t procedure_identifier_len;
  uint8_t *procedure_identifier=sl_cli_get_argument_hex(arguments,3,&procedure_identifier_len);
  //return values
  status=sl_bt_linklayer_event_info_reporting_enable(
  enable,
  configuration,
  procedure_type,
  procedure_identifier_len,
  procedure_identifier
  );

    printf("rsp_linklayer_event_info_reporting_enable 0x%lx ",status);
    printf("\n");
}
#endif // SL_CATALOG_BLUETOOTH_FEATURE_LINKLAYER_INTERFACE_PRESENT
void sli_bt_cli_resource_get_status(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  uint32_t total_bytes;
  uint32_t free_bytes;
  status=sl_bt_resource_get_status(
  &total_bytes,
  &free_bytes
  );

    printf("rsp_resource_get_status 0x%lx ",status);
    printf("0x%x ",total_bytes);
    printf("0x%x ",free_bytes);
    printf("\n");
}
void sli_bt_cli_resource_set_report_threshold(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t low=sl_cli_get_argument_uint32(arguments,0);
  uint32_t high=sl_cli_get_argument_uint32(arguments,1);
  //return values
  status=sl_bt_resource_set_report_threshold(
  low,
  high
  );

    printf("rsp_resource_set_report_threshold 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_resource_enable_connection_tx_report(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t packet_count=sl_cli_get_argument_uint16(arguments,0);
  //return values
  status=sl_bt_resource_enable_connection_tx_report(
  packet_count
  );

    printf("rsp_resource_enable_connection_tx_report 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_resource_get_connection_tx_status(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint16_t flags;
  uint16_t packet_count;
  uint32_t data_len;
  status=sl_bt_resource_get_connection_tx_status(
  connection,
  &flags,
  &packet_count,
  &data_len
  );

    printf("rsp_resource_get_connection_tx_status 0x%lx ",status);
    printf("0x%x ",flags);
    printf("0x%x ",packet_count);
    printf("0x%x ",data_len);
    printf("\n");
}
void sli_bt_cli_resource_disable_connection_tx_report(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_resource_disable_connection_tx_report(
  );

    printf("rsp_resource_disable_connection_tx_report 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gap_set_privacy_mode(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t privacy=sl_cli_get_argument_uint8(arguments,0);
  uint8_t interval=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_gap_set_privacy_mode(
  privacy,
  interval
  );

    printf("rsp_gap_set_privacy_mode 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gap_set_data_channel_classification(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t channel_map_len;
  uint8_t *channel_map=sl_cli_get_argument_hex(arguments,0,&channel_map_len);
  //return values
  status=sl_bt_gap_set_data_channel_classification(
  channel_map_len,
  channel_map
  );

    printf("rsp_gap_set_data_channel_classification 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gap_set_identity_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t addr_type=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_gap_set_identity_address(
  address,
  addr_type
  );

    printf("rsp_gap_set_identity_address 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gap_get_identity_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  bd_addr address;
  uint8_t type;
  status=sl_bt_gap_get_identity_address(
  &address,
  &type
  );

    printf("rsp_gap_get_identity_address 0x%lx ",status);
    print_hex(address.addr,sizeof(address.addr));
    printf("0x%x ",type);
    printf("\n");
}
void sli_bt_cli_gap_get_max_connections(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  uint8_t num_connections;
  status=sl_bt_gap_get_max_connections(
  &num_connections
  );

    printf("rsp_gap_get_max_connections 0x%lx ",status);
    printf("0x%x ",num_connections);
    printf("\n");
}
void sli_bt_cli_advertiser_create_set(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  uint8_t handle;
  status=sl_bt_advertiser_create_set(
  &handle
  );

    printf("rsp_advertiser_create_set 0x%lx ",status);
    printf("0x%x ",handle);
    printf("\n");
}
void sli_bt_cli_advertiser_configure(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint32_t flags=sl_cli_get_argument_uint32(arguments,1);
  //return values
  status=sl_bt_advertiser_configure(
  advertising_set,
  flags
  );

    printf("rsp_advertiser_configure 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_advertiser_set_timing(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint32_t interval_min=sl_cli_get_argument_uint32(arguments,1);
  uint32_t interval_max=sl_cli_get_argument_uint32(arguments,2);
  uint16_t duration=sl_cli_get_argument_uint16(arguments,3);
  uint8_t maxevents=sl_cli_get_argument_uint8(arguments,4);
  //return values
  status=sl_bt_advertiser_set_timing(
  advertising_set,
  interval_min,
  interval_max,
  duration,
  maxevents
  );

    printf("rsp_advertiser_set_timing 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_advertiser_set_channel_map(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t channel_map=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_advertiser_set_channel_map(
  advertising_set,
  channel_map
  );

    printf("rsp_advertiser_set_channel_map 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_advertiser_set_tx_power(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  int16_t power=sl_cli_get_argument_int16(arguments,1);
  //return values
  int16_t set_power;
  status=sl_bt_advertiser_set_tx_power(
  advertising_set,
  power,
  &set_power
  );

    printf("rsp_advertiser_set_tx_power 0x%lx ",status);
    printf("%d ",set_power);
    printf("\n");
}
void sli_bt_cli_advertiser_set_report_scan_request(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t report_scan_req=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_advertiser_set_report_scan_request(
  advertising_set,
  report_scan_req
  );

    printf("rsp_advertiser_set_report_scan_request 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_advertiser_set_random_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t addr_type=sl_cli_get_argument_uint8(arguments,1);
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,2,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  //return values
  bd_addr address_out;
  status=sl_bt_advertiser_set_random_address(
  advertising_set,
  addr_type,
  address,
  &address_out
  );

    printf("rsp_advertiser_set_random_address 0x%lx ",status);
    print_hex(address_out.addr,sizeof(address_out.addr));
    printf("\n");
}
void sli_bt_cli_advertiser_clear_random_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_advertiser_clear_random_address(
  advertising_set
  );

    printf("rsp_advertiser_clear_random_address 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_advertiser_stop(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_advertiser_stop(
  advertising_set
  );

    printf("rsp_advertiser_stop 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_advertiser_delete_set(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_advertiser_delete_set(
  advertising_set
  );

    printf("rsp_advertiser_delete_set 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_legacy_advertiser_set_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t type=sl_cli_get_argument_uint8(arguments,1);
  size_t data_len;
  uint8_t *data=sl_cli_get_argument_hex(arguments,2,&data_len);
  //return values
  status=sl_bt_legacy_advertiser_set_data(
  advertising_set,
  type,
  data_len,
  data
  );

    printf("rsp_legacy_advertiser_set_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_legacy_advertiser_generate_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t discover=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_legacy_advertiser_generate_data(
  advertising_set,
  discover
  );

    printf("rsp_legacy_advertiser_generate_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_legacy_advertiser_start(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t connect=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_legacy_advertiser_start(
  advertising_set,
  connect
  );

    printf("rsp_legacy_advertiser_start 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_legacy_advertiser_start_directed(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t connect=sl_cli_get_argument_uint8(arguments,1);
  size_t _peer_addr_len;
  uint8_t *_peer_addr=sl_cli_get_argument_hex(arguments,2,&_peer_addr_len);
  bd_addr peer_addr;
  memcpy(&peer_addr,_peer_addr,sizeof(peer_addr));
  uint8_t peer_addr_type=sl_cli_get_argument_uint8(arguments,3);
  //return values
  status=sl_bt_legacy_advertiser_start_directed(
  advertising_set,
  connect,
  peer_addr,
  peer_addr_type
  );

    printf("rsp_legacy_advertiser_start_directed 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_extended_advertiser_set_phy(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t primary_phy=sl_cli_get_argument_uint8(arguments,1);
  uint8_t secondary_phy=sl_cli_get_argument_uint8(arguments,2);
  //return values
  status=sl_bt_extended_advertiser_set_phy(
  advertising_set,
  primary_phy,
  secondary_phy
  );

    printf("rsp_extended_advertiser_set_phy 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_extended_advertiser_set_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  size_t data_len;
  uint8_t *data=sl_cli_get_argument_hex(arguments,1,&data_len);
  //return values
  status=sl_bt_extended_advertiser_set_data(
  advertising_set,
  data_len,
  data
  );

    printf("rsp_extended_advertiser_set_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_extended_advertiser_set_long_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_extended_advertiser_set_long_data(
  advertising_set
  );

    printf("rsp_extended_advertiser_set_long_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_extended_advertiser_generate_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t discover=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_extended_advertiser_generate_data(
  advertising_set,
  discover
  );

    printf("rsp_extended_advertiser_generate_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_extended_advertiser_start(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t connect=sl_cli_get_argument_uint8(arguments,1);
  uint32_t flags=sl_cli_get_argument_uint32(arguments,2);
  //return values
  status=sl_bt_extended_advertiser_start(
  advertising_set,
  connect,
  flags
  );

    printf("rsp_extended_advertiser_start 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_extended_advertiser_start_directed(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t connect=sl_cli_get_argument_uint8(arguments,1);
  uint32_t flags=sl_cli_get_argument_uint32(arguments,2);
  size_t _peer_addr_len;
  uint8_t *_peer_addr=sl_cli_get_argument_hex(arguments,3,&_peer_addr_len);
  bd_addr peer_addr;
  memcpy(&peer_addr,_peer_addr,sizeof(peer_addr));
  uint8_t peer_addr_type=sl_cli_get_argument_uint8(arguments,4);
  //return values
  status=sl_bt_extended_advertiser_start_directed(
  advertising_set,
  connect,
  flags,
  peer_addr,
  peer_addr_type
  );

    printf("rsp_extended_advertiser_start_directed 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_periodic_advertiser_set_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  size_t data_len;
  uint8_t *data=sl_cli_get_argument_hex(arguments,1,&data_len);
  //return values
  status=sl_bt_periodic_advertiser_set_data(
  advertising_set,
  data_len,
  data
  );

    printf("rsp_periodic_advertiser_set_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_periodic_advertiser_set_long_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_periodic_advertiser_set_long_data(
  advertising_set
  );

    printf("rsp_periodic_advertiser_set_long_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_periodic_advertiser_start(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint16_t interval_min=sl_cli_get_argument_uint16(arguments,1);
  uint16_t interval_max=sl_cli_get_argument_uint16(arguments,2);
  uint32_t flags=sl_cli_get_argument_uint32(arguments,3);
  //return values
  status=sl_bt_periodic_advertiser_start(
  advertising_set,
  interval_min,
  interval_max,
  flags
  );

    printf("rsp_periodic_advertiser_start 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_periodic_advertiser_stop(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_periodic_advertiser_stop(
  advertising_set
  );

    printf("rsp_periodic_advertiser_stop 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_scanner_set_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t mode=sl_cli_get_argument_uint8(arguments,0);
  uint16_t interval=sl_cli_get_argument_uint16(arguments,1);
  uint16_t window=sl_cli_get_argument_uint16(arguments,2);
  //return values
  status=sl_bt_scanner_set_parameters(
  mode,
  interval,
  window
  );

    printf("rsp_scanner_set_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_scanner_set_parameters_and_filter(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t mode=sl_cli_get_argument_uint8(arguments,0);
  uint16_t interval=sl_cli_get_argument_uint16(arguments,1);
  uint16_t window=sl_cli_get_argument_uint16(arguments,2);
  uint32_t flags=sl_cli_get_argument_uint32(arguments,3);
  uint8_t filter_policy=sl_cli_get_argument_uint8(arguments,4);
  //return values
  status=sl_bt_scanner_set_parameters_and_filter(
  mode,
  interval,
  window,
  flags,
  filter_policy
  );

    printf("rsp_scanner_set_parameters_and_filter 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_scanner_start(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t scanning_phy=sl_cli_get_argument_uint8(arguments,0);
  uint8_t discover_mode=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_scanner_start(
  scanning_phy,
  discover_mode
  );

    printf("rsp_scanner_start 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_scanner_stop(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_scanner_stop(
  );

    printf("rsp_scanner_stop 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sync_set_reporting_mode(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t sync=sl_cli_get_argument_uint16(arguments,0);
  uint8_t reporting_mode=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_sync_set_reporting_mode(
  sync,
  reporting_mode
  );

    printf("rsp_sync_set_reporting_mode 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sync_update_sync_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t sync=sl_cli_get_argument_uint16(arguments,0);
  uint16_t skip=sl_cli_get_argument_uint16(arguments,1);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,2);
  //return values
  status=sl_bt_sync_update_sync_parameters(
  sync,
  skip,
  timeout
  );

    printf("rsp_sync_update_sync_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sync_close(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t sync=sl_cli_get_argument_uint16(arguments,0);
  //return values
  status=sl_bt_sync_close(
  sync
  );

    printf("rsp_sync_close 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sync_scanner_set_sync_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t skip=sl_cli_get_argument_uint16(arguments,0);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,1);
  uint8_t reporting_mode=sl_cli_get_argument_uint8(arguments,2);
  //return values
  status=sl_bt_sync_scanner_set_sync_parameters(
  skip,
  timeout,
  reporting_mode
  );

    printf("rsp_sync_scanner_set_sync_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sync_scanner_open(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t address_type=sl_cli_get_argument_uint8(arguments,1);
  uint8_t adv_sid=sl_cli_get_argument_uint8(arguments,2);
  //return values
  uint16_t sync;
  status=sl_bt_sync_scanner_open(
  address,
  address_type,
  adv_sid,
  &sync
  );

    printf("rsp_sync_scanner_open 0x%lx ",status);
    printf("0x%x ",sync);
    printf("\n");
}
void sli_bt_cli_past_receiver_set_default_sync_receive_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t mode=sl_cli_get_argument_uint8(arguments,0);
  uint16_t skip=sl_cli_get_argument_uint16(arguments,1);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,2);
  uint8_t reporting_mode=sl_cli_get_argument_uint8(arguments,3);
  //return values
  status=sl_bt_past_receiver_set_default_sync_receive_parameters(
  mode,
  skip,
  timeout,
  reporting_mode
  );

    printf("rsp_past_receiver_set_default_sync_receive_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_past_receiver_set_sync_receive_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t mode=sl_cli_get_argument_uint8(arguments,1);
  uint16_t skip=sl_cli_get_argument_uint16(arguments,2);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,3);
  uint8_t reporting_mode=sl_cli_get_argument_uint8(arguments,4);
  //return values
  status=sl_bt_past_receiver_set_sync_receive_parameters(
  connection,
  mode,
  skip,
  timeout,
  reporting_mode
  );

    printf("rsp_past_receiver_set_sync_receive_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_past_receiver_set_default_sync_receive_over_sync_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t mode=sl_cli_get_argument_uint8(arguments,0);
  uint16_t skip=sl_cli_get_argument_uint16(arguments,1);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,2);
  uint8_t reporting_mode=sl_cli_get_argument_uint8(arguments,3);
  //return values
  status=sl_bt_past_receiver_set_default_sync_receive_over_sync_parameters(
  mode,
  skip,
  timeout,
  reporting_mode
  );

    printf("rsp_past_receiver_set_default_sync_receive_over_sync_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_past_receiver_set_sync_receive_over_sync_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t sync=sl_cli_get_argument_uint16(arguments,0);
  uint8_t mode=sl_cli_get_argument_uint8(arguments,1);
  uint16_t skip=sl_cli_get_argument_uint16(arguments,2);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,3);
  uint8_t reporting_mode=sl_cli_get_argument_uint8(arguments,4);
  //return values
  status=sl_bt_past_receiver_set_sync_receive_over_sync_parameters(
  sync,
  mode,
  skip,
  timeout,
  reporting_mode
  );

    printf("rsp_past_receiver_set_sync_receive_over_sync_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_advertiser_past_transfer(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t service_data=sl_cli_get_argument_uint16(arguments,1);
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,2);
  //return values
  status=sl_bt_advertiser_past_transfer(
  connection,
  service_data,
  advertising_set
  );

    printf("rsp_advertiser_past_transfer 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_advertiser_past_transfer_over_pawr_advertiser(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t transferring_advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint16_t service_data=sl_cli_get_argument_uint16(arguments,1);
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,2);
  uint8_t repeat_count=sl_cli_get_argument_uint8(arguments,3);
  size_t subevents_len;
  uint8_t *subevents=sl_cli_get_argument_hex(arguments,4,&subevents_len);
  //return values
  status=sl_bt_advertiser_past_transfer_over_pawr_advertiser(
  transferring_advertising_set,
  service_data,
  advertising_set,
  repeat_count,
  subevents_len,
  subevents
  );

    printf("rsp_advertiser_past_transfer_over_pawr_advertiser 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sync_past_transfer(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t service_data=sl_cli_get_argument_uint16(arguments,1);
  uint16_t sync=sl_cli_get_argument_uint16(arguments,2);
  //return values
  status=sl_bt_sync_past_transfer(
  connection,
  service_data,
  sync
  );

    printf("rsp_sync_past_transfer 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_pawr_sync_set_sync_subevents(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t sync=sl_cli_get_argument_uint16(arguments,0);
  size_t subevents_len;
  uint8_t *subevents=sl_cli_get_argument_hex(arguments,1,&subevents_len);
  //return values
  status=sl_bt_pawr_sync_set_sync_subevents(
  sync,
  subevents_len,
  subevents
  );

    printf("rsp_pawr_sync_set_sync_subevents 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_pawr_sync_set_response_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t sync=sl_cli_get_argument_uint16(arguments,0);
  uint16_t request_event=sl_cli_get_argument_uint16(arguments,1);
  uint8_t request_subevent=sl_cli_get_argument_uint8(arguments,2);
  uint8_t response_subevent=sl_cli_get_argument_uint8(arguments,3);
  uint8_t response_slot=sl_cli_get_argument_uint8(arguments,4);
  size_t response_data_len;
  uint8_t *response_data=sl_cli_get_argument_hex(arguments,5,&response_data_len);
  //return values
  status=sl_bt_pawr_sync_set_response_data(
  sync,
  request_event,
  request_subevent,
  response_subevent,
  response_slot,
  response_data_len,
  response_data
  );

    printf("rsp_pawr_sync_set_response_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_pawr_advertiser_start(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint16_t interval_min=sl_cli_get_argument_uint16(arguments,1);
  uint16_t interval_max=sl_cli_get_argument_uint16(arguments,2);
  uint32_t flags=sl_cli_get_argument_uint32(arguments,3);
  uint8_t num_subevents=sl_cli_get_argument_uint8(arguments,4);
  uint8_t subevent_interval=sl_cli_get_argument_uint8(arguments,5);
  uint8_t response_slot_delay=sl_cli_get_argument_uint8(arguments,6);
  uint8_t response_slot_spacing=sl_cli_get_argument_uint8(arguments,7);
  uint8_t response_slots=sl_cli_get_argument_uint8(arguments,8);
  //return values
  status=sl_bt_pawr_advertiser_start(
  advertising_set,
  interval_min,
  interval_max,
  flags,
  num_subevents,
  subevent_interval,
  response_slot_delay,
  response_slot_spacing,
  response_slots
  );

    printf("rsp_pawr_advertiser_start 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_pawr_advertiser_change_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint16_t interval_min=sl_cli_get_argument_uint16(arguments,1);
  uint16_t interval_max=sl_cli_get_argument_uint16(arguments,2);
  uint32_t flags=sl_cli_get_argument_uint32(arguments,3);
  uint8_t num_subevents=sl_cli_get_argument_uint8(arguments,4);
  uint8_t subevent_interval=sl_cli_get_argument_uint8(arguments,5);
  uint8_t response_slot_delay=sl_cli_get_argument_uint8(arguments,6);
  uint8_t response_slot_spacing=sl_cli_get_argument_uint8(arguments,7);
  uint8_t response_slots=sl_cli_get_argument_uint8(arguments,8);
  uint8_t phy=sl_cli_get_argument_uint8(arguments,9);
  uint8_t repeat_count=sl_cli_get_argument_uint8(arguments,10);
  //return values
  status=sl_bt_pawr_advertiser_change_parameters(
  advertising_set,
  interval_min,
  interval_max,
  flags,
  num_subevents,
  subevent_interval,
  response_slot_delay,
  response_slot_spacing,
  response_slots,
  phy,
  repeat_count
  );

    printf("rsp_pawr_advertiser_change_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_pawr_advertiser_set_subevent_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t subevent=sl_cli_get_argument_uint8(arguments,1);
  uint8_t response_slot_start=sl_cli_get_argument_uint8(arguments,2);
  uint8_t response_slot_count=sl_cli_get_argument_uint8(arguments,3);
  size_t adv_data_len;
  uint8_t *adv_data=sl_cli_get_argument_hex(arguments,4,&adv_data_len);
  //return values
  status=sl_bt_pawr_advertiser_set_subevent_data(
  advertising_set,
  subevent,
  response_slot_start,
  response_slot_count,
  adv_data_len,
  adv_data
  );

    printf("rsp_pawr_advertiser_set_subevent_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_pawr_advertiser_create_connection(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  uint8_t subevent=sl_cli_get_argument_uint8(arguments,1);
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,2,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t address_type=sl_cli_get_argument_uint8(arguments,3);
  //return values
  uint8_t connection;
  status=sl_bt_pawr_advertiser_create_connection(
  advertising_set,
  subevent,
  address,
  address_type,
  &connection
  );

    printf("rsp_pawr_advertiser_create_connection 0x%lx ",status);
    printf("0x%x ",connection);
    printf("\n");
}
void sli_bt_cli_pawr_advertiser_stop(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t advertising_set=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_pawr_advertiser_stop(
  advertising_set
  );

    printf("rsp_pawr_advertiser_stop 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_set_default_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t min_interval=sl_cli_get_argument_uint16(arguments,0);
  uint16_t max_interval=sl_cli_get_argument_uint16(arguments,1);
  uint16_t latency=sl_cli_get_argument_uint16(arguments,2);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,3);
  uint16_t min_ce_length=sl_cli_get_argument_uint16(arguments,4);
  uint16_t max_ce_length=sl_cli_get_argument_uint16(arguments,5);
  //return values
  status=sl_bt_connection_set_default_parameters(
  min_interval,
  max_interval,
  latency,
  timeout,
  min_ce_length,
  max_ce_length
  );

    printf("rsp_connection_set_default_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_set_default_preferred_phy(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t preferred_phy=sl_cli_get_argument_uint8(arguments,0);
  uint8_t accepted_phy=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_connection_set_default_preferred_phy(
  preferred_phy,
  accepted_phy
  );

    printf("rsp_connection_set_default_preferred_phy 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_set_default_data_length(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t tx_data_len=sl_cli_get_argument_uint16(arguments,0);
  //return values
  status=sl_bt_connection_set_default_data_length(
  tx_data_len
  );

    printf("rsp_connection_set_default_data_length 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_set_default_acceptable_subrate(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t min_subrate=sl_cli_get_argument_uint16(arguments,0);
  uint16_t max_subrate=sl_cli_get_argument_uint16(arguments,1);
  uint16_t max_latency=sl_cli_get_argument_uint16(arguments,2);
  uint16_t continuation_number=sl_cli_get_argument_uint16(arguments,3);
  uint16_t max_timeout=sl_cli_get_argument_uint16(arguments,4);
  //return values
  status=sl_bt_connection_set_default_acceptable_subrate(
  min_subrate,
  max_subrate,
  max_latency,
  continuation_number,
  max_timeout
  );

    printf("rsp_connection_set_default_acceptable_subrate 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_open(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t address_type=sl_cli_get_argument_uint8(arguments,1);
  uint8_t initiating_phy=sl_cli_get_argument_uint8(arguments,2);
  //return values
  uint8_t connection;
  status=sl_bt_connection_open(
  address,
  address_type,
  initiating_phy,
  &connection
  );

    printf("rsp_connection_open 0x%lx ",status);
    printf("0x%x ",connection);
    printf("\n");
}
void sli_bt_cli_connection_open_with_accept_list(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t initiating_phy=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint8_t connection;
  status=sl_bt_connection_open_with_accept_list(
  initiating_phy,
  &connection
  );

    printf("rsp_connection_open_with_accept_list 0x%lx ",status);
    printf("0x%x ",connection);
    printf("\n");
}
void sli_bt_cli_connection_set_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t min_interval=sl_cli_get_argument_uint16(arguments,1);
  uint16_t max_interval=sl_cli_get_argument_uint16(arguments,2);
  uint16_t latency=sl_cli_get_argument_uint16(arguments,3);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,4);
  uint16_t min_ce_length=sl_cli_get_argument_uint16(arguments,5);
  uint16_t max_ce_length=sl_cli_get_argument_uint16(arguments,6);
  //return values
  status=sl_bt_connection_set_parameters(
  connection,
  min_interval,
  max_interval,
  latency,
  timeout,
  min_ce_length,
  max_ce_length
  );

    printf("rsp_connection_set_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_set_preferred_phy(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t preferred_phy=sl_cli_get_argument_uint8(arguments,1);
  uint8_t accepted_phy=sl_cli_get_argument_uint8(arguments,2);
  //return values
  status=sl_bt_connection_set_preferred_phy(
  connection,
  preferred_phy,
  accepted_phy
  );

    printf("rsp_connection_set_preferred_phy 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_disable_slave_latency(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t disable=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_connection_disable_slave_latency(
  connection,
  disable
  );

    printf("rsp_connection_disable_slave_latency 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_get_median_rssi(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  int8_t rssi;
  status=sl_bt_connection_get_median_rssi(
  connection,
  &rssi
  );

    printf("rsp_connection_get_median_rssi 0x%lx ",status);
    printf("%d ",rssi);
    printf("\n");
}
void sli_bt_cli_connection_read_channel_map(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  size_t channel_map_len;
  uint8_t channel_map[MAX_P_SIZE];
  status=sl_bt_connection_read_channel_map(
  connection,
  MAX_P_SIZE,
  &channel_map_len,
  channel_map
  );

    printf("rsp_connection_read_channel_map 0x%lx ",status);
    print_hex(channel_map,channel_map_len);
    printf("\n");
}
void sli_bt_cli_connection_set_power_reporting(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t mode=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_connection_set_power_reporting(
  connection,
  mode
  );

    printf("rsp_connection_set_power_reporting 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_set_remote_power_reporting(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t mode=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_connection_set_remote_power_reporting(
  connection,
  mode
  );

    printf("rsp_connection_set_remote_power_reporting 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_get_tx_power(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t phy=sl_cli_get_argument_uint8(arguments,1);
  //return values
  int8_t current_level;
  int8_t max_level;
  status=sl_bt_connection_get_tx_power(
  connection,
  phy,
  &current_level,
  &max_level
  );

    printf("rsp_connection_get_tx_power 0x%lx ",status);
    printf("%d ",current_level);
    printf("%d ",max_level);
    printf("\n");
}
void sli_bt_cli_connection_get_remote_tx_power(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t phy=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_connection_get_remote_tx_power(
  connection,
  phy
  );

    printf("rsp_connection_get_remote_tx_power 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_set_tx_power(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  int16_t tx_power=sl_cli_get_argument_int16(arguments,1);
  //return values
  int16_t tx_power_out;
  status=sl_bt_connection_set_tx_power(
  connection,
  tx_power,
  &tx_power_out
  );

    printf("rsp_connection_set_tx_power 0x%lx ",status);
    printf("%d ",tx_power_out);
    printf("\n");
}
void sli_bt_cli_connection_read_remote_used_features(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_connection_read_remote_used_features(
  connection
  );

    printf("rsp_connection_read_remote_used_features 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_get_security_status(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint8_t security_mode;
  uint8_t key_size;
  uint8_t bonding_handle;
  status=sl_bt_connection_get_security_status(
  connection,
  &security_mode,
  &key_size,
  &bonding_handle
  );

    printf("rsp_connection_get_security_status 0x%lx ",status);
    printf("0x%x ",security_mode);
    printf("0x%x ",key_size);
    printf("0x%x ",bonding_handle);
    printf("\n");
}
void sli_bt_cli_connection_set_data_length(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t tx_data_len=sl_cli_get_argument_uint16(arguments,1);
  uint16_t tx_time_us=sl_cli_get_argument_uint16(arguments,2);
  //return values
  status=sl_bt_connection_set_data_length(
  connection,
  tx_data_len,
  tx_time_us
  );

    printf("rsp_connection_set_data_length 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_read_statistics(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t reset=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_connection_read_statistics(
  connection,
  reset
  );

    printf("rsp_connection_read_statistics 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_get_scheduling_details(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint32_t access_address;
  uint8_t role;
  uint32_t crc_init;
  uint16_t interval;
  uint16_t supervision_timeout;
  uint8_t central_clock_accuracy;
  uint8_t central_phy;
  uint8_t peripheral_phy;
  uint8_t channel_selection_algorithm;
  uint8_t hop;
  sl_bt_connection_channel_map_t channel_map;
  uint8_t channel;
  uint16_t event_counter;
  uint32_t start_time_us;
  status=sl_bt_connection_get_scheduling_details(
  connection,
  &access_address,
  &role,
  &crc_init,
  &interval,
  &supervision_timeout,
  &central_clock_accuracy,
  &central_phy,
  &peripheral_phy,
  &channel_selection_algorithm,
  &hop,
  &channel_map,
  &channel,
  &event_counter,
  &start_time_us
  );

    printf("rsp_connection_get_scheduling_details 0x%lx ",status);
    printf("0x%x ",access_address);
    printf("0x%x ",role);
    printf("0x%x ",crc_init);
    printf("0x%x ",interval);
    printf("0x%x ",supervision_timeout);
    printf("0x%x ",central_clock_accuracy);
    printf("0x%x ",central_phy);
    printf("0x%x ",peripheral_phy);
    printf("0x%x ",channel_selection_algorithm);
    printf("0x%x ",hop);
    print_hex(channel_map.data,sizeof(channel_map.data));
    printf("0x%x ",channel);
    printf("0x%x ",event_counter);
    printf("0x%x ",start_time_us);
    printf("\n");
}
void sli_bt_cli_connection_get_remote_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  bd_addr address;
  uint8_t address_type;
  status=sl_bt_connection_get_remote_address(
  connection,
  &address,
  &address_type
  );

    printf("rsp_connection_get_remote_address 0x%lx ",status);
    print_hex(address.addr,sizeof(address.addr));
    printf("0x%x ",address_type);
    printf("\n");
}
void sli_bt_cli_connection_request_subrate(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t min_subrate=sl_cli_get_argument_uint16(arguments,1);
  uint16_t max_subrate=sl_cli_get_argument_uint16(arguments,2);
  uint16_t max_latency=sl_cli_get_argument_uint16(arguments,3);
  uint16_t continuation_number=sl_cli_get_argument_uint16(arguments,4);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,5);
  //return values
  status=sl_bt_connection_request_subrate(
  connection,
  min_subrate,
  max_subrate,
  max_latency,
  continuation_number,
  timeout
  );

    printf("rsp_connection_request_subrate 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_get_state(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint8_t state;
  status=sl_bt_connection_get_state(
  connection,
  &state
  );

    printf("rsp_connection_get_state 0x%lx ",status);
    printf("0x%x ",state);
    printf("\n");
}
void sli_bt_cli_connection_close(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_connection_close(
  connection
  );

    printf("rsp_connection_close 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_connection_forcefully_close(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_connection_forcefully_close(
  connection
  );

    printf("rsp_connection_forcefully_close 0x%lx ",status);
    printf("\n");
}
#ifdef SL_CATALOG_BLUETOOTH_FEATURE_GATT_PRESENT
void sli_bt_cli_gatt_set_max_mtu(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t max_mtu=sl_cli_get_argument_uint16(arguments,0);
  //return values
  uint16_t max_mtu_out;
  status=sl_bt_gatt_set_max_mtu(
  max_mtu,
  &max_mtu_out
  );

    printf("rsp_gatt_set_max_mtu 0x%lx ",status);
    printf("0x%x ",max_mtu_out);
    printf("\n");
}
void sli_bt_cli_gatt_discover_primary_services(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_gatt_discover_primary_services(
  connection
  );

    printf("rsp_gatt_discover_primary_services 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_discover_primary_services_by_uuid(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  size_t uuid_len;
  uint8_t *uuid=sl_cli_get_argument_hex(arguments,1,&uuid_len);
  //return values
  status=sl_bt_gatt_discover_primary_services_by_uuid(
  connection,
  uuid_len,
  uuid
  );

    printf("rsp_gatt_discover_primary_services_by_uuid 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_find_included_services(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint32_t service=sl_cli_get_argument_uint32(arguments,1);
  //return values
  status=sl_bt_gatt_find_included_services(
  connection,
  service
  );

    printf("rsp_gatt_find_included_services 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_discover_characteristics(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint32_t service=sl_cli_get_argument_uint32(arguments,1);
  //return values
  status=sl_bt_gatt_discover_characteristics(
  connection,
  service
  );

    printf("rsp_gatt_discover_characteristics 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_discover_characteristics_by_uuid(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint32_t service=sl_cli_get_argument_uint32(arguments,1);
  size_t uuid_len;
  uint8_t *uuid=sl_cli_get_argument_hex(arguments,2,&uuid_len);
  //return values
  status=sl_bt_gatt_discover_characteristics_by_uuid(
  connection,
  service,
  uuid_len,
  uuid
  );

    printf("rsp_gatt_discover_characteristics_by_uuid 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_discover_descriptors(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gatt_discover_descriptors(
  connection,
  characteristic
  );

    printf("rsp_gatt_discover_descriptors 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_discover_characteristic_descriptors(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t start=sl_cli_get_argument_uint16(arguments,1);
  uint16_t end=sl_cli_get_argument_uint16(arguments,2);
  //return values
  status=sl_bt_gatt_discover_characteristic_descriptors(
  connection,
  start,
  end
  );

    printf("rsp_gatt_discover_characteristic_descriptors 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_set_characteristic_notification(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint8_t flags=sl_cli_get_argument_uint8(arguments,2);
  //return values
  status=sl_bt_gatt_set_characteristic_notification(
  connection,
  characteristic,
  flags
  );

    printf("rsp_gatt_set_characteristic_notification 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_send_characteristic_confirmation(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_gatt_send_characteristic_confirmation(
  connection
  );

    printf("rsp_gatt_send_characteristic_confirmation 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_read_characteristic_value(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gatt_read_characteristic_value(
  connection,
  characteristic
  );

    printf("rsp_gatt_read_characteristic_value 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_read_characteristic_value_from_offset(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint16_t offset=sl_cli_get_argument_uint16(arguments,2);
  uint16_t maxlen=sl_cli_get_argument_uint16(arguments,3);
  //return values
  status=sl_bt_gatt_read_characteristic_value_from_offset(
  connection,
  characteristic,
  offset,
  maxlen
  );

    printf("rsp_gatt_read_characteristic_value_from_offset 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_read_multiple_characteristic_values(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  size_t characteristic_list_len;
  uint8_t *characteristic_list=sl_cli_get_argument_hex(arguments,1,&characteristic_list_len);
  //return values
  status=sl_bt_gatt_read_multiple_characteristic_values(
  connection,
  characteristic_list_len,
  characteristic_list
  );

    printf("rsp_gatt_read_multiple_characteristic_values 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_read_variable_length_characteristic_values(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  size_t characteristic_list_len;
  uint8_t *characteristic_list=sl_cli_get_argument_hex(arguments,1,&characteristic_list_len);
  //return values
  status=sl_bt_gatt_read_variable_length_characteristic_values(
  connection,
  characteristic_list_len,
  characteristic_list
  );

    printf("rsp_gatt_read_variable_length_characteristic_values 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_read_characteristic_value_by_uuid(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint32_t service=sl_cli_get_argument_uint32(arguments,1);
  size_t uuid_len;
  uint8_t *uuid=sl_cli_get_argument_hex(arguments,2,&uuid_len);
  //return values
  status=sl_bt_gatt_read_characteristic_value_by_uuid(
  connection,
  service,
  uuid_len,
  uuid
  );

    printf("rsp_gatt_read_characteristic_value_by_uuid 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_write_characteristic_value(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,2,&value_len);
  //return values
  status=sl_bt_gatt_write_characteristic_value(
  connection,
  characteristic,
  value_len,
  value
  );

    printf("rsp_gatt_write_characteristic_value 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_write_characteristic_value_without_response(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,2,&value_len);
  //return values
  uint16_t sent_len;
  status=sl_bt_gatt_write_characteristic_value_without_response(
  connection,
  characteristic,
  value_len,
  value,
  &sent_len
  );

    printf("rsp_gatt_write_characteristic_value_without_response 0x%lx ",status);
    printf("0x%x ",sent_len);
    printf("\n");
}
void sli_bt_cli_gatt_prepare_characteristic_value_write(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint16_t offset=sl_cli_get_argument_uint16(arguments,2);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,3,&value_len);
  //return values
  uint16_t sent_len;
  status=sl_bt_gatt_prepare_characteristic_value_write(
  connection,
  characteristic,
  offset,
  value_len,
  value,
  &sent_len
  );

    printf("rsp_gatt_prepare_characteristic_value_write 0x%lx ",status);
    printf("0x%x ",sent_len);
    printf("\n");
}
void sli_bt_cli_gatt_prepare_characteristic_value_reliable_write(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint16_t offset=sl_cli_get_argument_uint16(arguments,2);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,3,&value_len);
  //return values
  uint16_t sent_len;
  status=sl_bt_gatt_prepare_characteristic_value_reliable_write(
  connection,
  characteristic,
  offset,
  value_len,
  value,
  &sent_len
  );

    printf("rsp_gatt_prepare_characteristic_value_reliable_write 0x%lx ",status);
    printf("0x%x ",sent_len);
    printf("\n");
}
void sli_bt_cli_gatt_execute_characteristic_value_write(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t flags=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_gatt_execute_characteristic_value_write(
  connection,
  flags
  );

    printf("rsp_gatt_execute_characteristic_value_write 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_read_descriptor_value(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t descriptor=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gatt_read_descriptor_value(
  connection,
  descriptor
  );

    printf("rsp_gatt_read_descriptor_value 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_write_descriptor_value(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t descriptor=sl_cli_get_argument_uint16(arguments,1);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,2,&value_len);
  //return values
  status=sl_bt_gatt_write_descriptor_value(
  connection,
  descriptor,
  value_len,
  value
  );

    printf("rsp_gatt_write_descriptor_value 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_get_mtu(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint16_t mtu;
  status=sl_bt_gatt_get_mtu(
  connection,
  &mtu
  );

    printf("rsp_gatt_get_mtu 0x%lx ",status);
    printf("0x%x ",mtu);
    printf("\n");
}
#endif // SL_CATALOG_BLUETOOTH_FEATURE_GATT_PRESENT
void sli_bt_cli_gattdb_new_session(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  uint16_t session;
  status=sl_bt_gattdb_new_session(
  &session
  );

    printf("rsp_gattdb_new_session 0x%lx ",status);
    printf("0x%x ",session);
    printf("\n");
}
void sli_bt_cli_gattdb_add_service(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint8_t type=sl_cli_get_argument_uint8(arguments,1);
  uint8_t property=sl_cli_get_argument_uint8(arguments,2);
  size_t uuid_len;
  uint8_t *uuid=sl_cli_get_argument_hex(arguments,3,&uuid_len);
  //return values
  uint16_t service;
  status=sl_bt_gattdb_add_service(
  session,
  type,
  property,
  uuid_len,
  uuid,
  &service
  );

    printf("rsp_gattdb_add_service 0x%lx ",status);
    printf("0x%x ",service);
    printf("\n");
}
void sli_bt_cli_gattdb_remove_service(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t service=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gattdb_remove_service(
  session,
  service
  );

    printf("rsp_gattdb_remove_service 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_add_included_service(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t service=sl_cli_get_argument_uint16(arguments,1);
  uint16_t included_service=sl_cli_get_argument_uint16(arguments,2);
  //return values
  uint16_t attribute;
  status=sl_bt_gattdb_add_included_service(
  session,
  service,
  included_service,
  &attribute
  );

    printf("rsp_gattdb_add_included_service 0x%lx ",status);
    printf("0x%x ",attribute);
    printf("\n");
}
void sli_bt_cli_gattdb_remove_included_service(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t attribute=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gattdb_remove_included_service(
  session,
  attribute
  );

    printf("rsp_gattdb_remove_included_service 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_add_uuid16_characteristic(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t service=sl_cli_get_argument_uint16(arguments,1);
  uint16_t property=sl_cli_get_argument_uint16(arguments,2);
  uint16_t security=sl_cli_get_argument_uint16(arguments,3);
  uint8_t flag=sl_cli_get_argument_uint8(arguments,4);
  size_t _uuid_len;
  uint8_t *_uuid=sl_cli_get_argument_hex(arguments,5,&_uuid_len);
  sl_bt_uuid_16_t uuid;
  memcpy(&uuid,_uuid,sizeof(uuid));
  uint8_t value_type=sl_cli_get_argument_uint8(arguments,6);
  uint16_t maxlen=sl_cli_get_argument_uint16(arguments,7);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,8,&value_len);
  //return values
  uint16_t characteristic;
  status=sl_bt_gattdb_add_uuid16_characteristic(
  session,
  service,
  property,
  security,
  flag,
  uuid,
  value_type,
  maxlen,
  value_len,
  value,
  &characteristic
  );

    printf("rsp_gattdb_add_uuid16_characteristic 0x%lx ",status);
    printf("0x%x ",characteristic);
    printf("\n");
}
void sli_bt_cli_gattdb_add_uuid128_characteristic(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t service=sl_cli_get_argument_uint16(arguments,1);
  uint16_t property=sl_cli_get_argument_uint16(arguments,2);
  uint16_t security=sl_cli_get_argument_uint16(arguments,3);
  uint8_t flag=sl_cli_get_argument_uint8(arguments,4);
  size_t _uuid_len;
  uint8_t *_uuid=sl_cli_get_argument_hex(arguments,5,&_uuid_len);
  uuid_128 uuid;
  memcpy(&uuid,_uuid,sizeof(uuid));
  uint8_t value_type=sl_cli_get_argument_uint8(arguments,6);
  uint16_t maxlen=sl_cli_get_argument_uint16(arguments,7);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,8,&value_len);
  //return values
  uint16_t characteristic;
  status=sl_bt_gattdb_add_uuid128_characteristic(
  session,
  service,
  property,
  security,
  flag,
  uuid,
  value_type,
  maxlen,
  value_len,
  value,
  &characteristic
  );

    printf("rsp_gattdb_add_uuid128_characteristic 0x%lx ",status);
    printf("0x%x ",characteristic);
    printf("\n");
}
void sli_bt_cli_gattdb_remove_characteristic(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gattdb_remove_characteristic(
  session,
  characteristic
  );

    printf("rsp_gattdb_remove_characteristic 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_add_uuid16_descriptor(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint16_t property=sl_cli_get_argument_uint16(arguments,2);
  uint16_t security=sl_cli_get_argument_uint16(arguments,3);
  size_t _uuid_len;
  uint8_t *_uuid=sl_cli_get_argument_hex(arguments,4,&_uuid_len);
  sl_bt_uuid_16_t uuid;
  memcpy(&uuid,_uuid,sizeof(uuid));
  uint8_t value_type=sl_cli_get_argument_uint8(arguments,5);
  uint16_t maxlen=sl_cli_get_argument_uint16(arguments,6);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,7,&value_len);
  //return values
  uint16_t descriptor;
  status=sl_bt_gattdb_add_uuid16_descriptor(
  session,
  characteristic,
  property,
  security,
  uuid,
  value_type,
  maxlen,
  value_len,
  value,
  &descriptor
  );

    printf("rsp_gattdb_add_uuid16_descriptor 0x%lx ",status);
    printf("0x%x ",descriptor);
    printf("\n");
}
void sli_bt_cli_gattdb_add_uuid128_descriptor(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint16_t property=sl_cli_get_argument_uint16(arguments,2);
  uint16_t security=sl_cli_get_argument_uint16(arguments,3);
  size_t _uuid_len;
  uint8_t *_uuid=sl_cli_get_argument_hex(arguments,4,&_uuid_len);
  uuid_128 uuid;
  memcpy(&uuid,_uuid,sizeof(uuid));
  uint8_t value_type=sl_cli_get_argument_uint8(arguments,5);
  uint16_t maxlen=sl_cli_get_argument_uint16(arguments,6);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,7,&value_len);
  //return values
  uint16_t descriptor;
  status=sl_bt_gattdb_add_uuid128_descriptor(
  session,
  characteristic,
  property,
  security,
  uuid,
  value_type,
  maxlen,
  value_len,
  value,
  &descriptor
  );

    printf("rsp_gattdb_add_uuid128_descriptor 0x%lx ",status);
    printf("0x%x ",descriptor);
    printf("\n");
}
void sli_bt_cli_gattdb_remove_descriptor(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t descriptor=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gattdb_remove_descriptor(
  session,
  descriptor
  );

    printf("rsp_gattdb_remove_descriptor 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_start_service(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t service=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gattdb_start_service(
  session,
  service
  );

    printf("rsp_gattdb_start_service 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_stop_service(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t service=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gattdb_stop_service(
  session,
  service
  );

    printf("rsp_gattdb_stop_service 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_start_characteristic(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gattdb_start_characteristic(
  session,
  characteristic
  );

    printf("rsp_gattdb_start_characteristic 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_stop_characteristic(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_gattdb_stop_characteristic(
  session,
  characteristic
  );

    printf("rsp_gattdb_stop_characteristic 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_commit(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  //return values
  status=sl_bt_gattdb_commit(
  session
  );

    printf("rsp_gattdb_commit 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_abort(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t session=sl_cli_get_argument_uint16(arguments,0);
  //return values
  status=sl_bt_gattdb_abort(
  session
  );

    printf("rsp_gattdb_abort 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gattdb_get_attribute_state(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t attribute=sl_cli_get_argument_uint16(arguments,0);
  //return values
  uint8_t state;
  status=sl_bt_gattdb_get_attribute_state(
  attribute,
  &state
  );

    printf("rsp_gattdb_get_attribute_state 0x%lx ",status);
    printf("0x%x ",state);
    printf("\n");
}
void sli_bt_cli_gatt_server_set_max_mtu(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t max_mtu=sl_cli_get_argument_uint16(arguments,0);
  //return values
  uint16_t max_mtu_out;
  status=sl_bt_gatt_server_set_max_mtu(
  max_mtu,
  &max_mtu_out
  );

    printf("rsp_gatt_server_set_max_mtu 0x%lx ",status);
    printf("0x%x ",max_mtu_out);
    printf("\n");
}
void sli_bt_cli_gatt_server_get_mtu(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint16_t mtu;
  status=sl_bt_gatt_server_get_mtu(
  connection,
  &mtu
  );

    printf("rsp_gatt_server_get_mtu 0x%lx ",status);
    printf("0x%x ",mtu);
    printf("\n");
}
void sli_bt_cli_gatt_server_find_attribute(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t start=sl_cli_get_argument_uint16(arguments,0);
  size_t type_len;
  uint8_t *type=sl_cli_get_argument_hex(arguments,1,&type_len);
  //return values
  uint16_t attribute;
  status=sl_bt_gatt_server_find_attribute(
  start,
  type_len,
  type,
  &attribute
  );

    printf("rsp_gatt_server_find_attribute 0x%lx ",status);
    printf("0x%x ",attribute);
    printf("\n");
}
void sli_bt_cli_gatt_server_find_primary_service(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t start=sl_cli_get_argument_uint16(arguments,0);
  size_t uuid_len;
  uint8_t *uuid=sl_cli_get_argument_hex(arguments,1,&uuid_len);
  //return values
  uint16_t start_out;
  uint16_t end_out;
  status=sl_bt_gatt_server_find_primary_service(
  start,
  uuid_len,
  uuid,
  &start_out,
  &end_out
  );

    printf("rsp_gatt_server_find_primary_service 0x%lx ",status);
    printf("0x%x ",start_out);
    printf("0x%x ",end_out);
    printf("\n");
}
void sli_bt_cli_gatt_server_read_attribute_value(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t attribute=sl_cli_get_argument_uint16(arguments,0);
  uint16_t offset=sl_cli_get_argument_uint16(arguments,1);
  //return values
  size_t value_len;
  uint8_t value[MAX_P_SIZE];
  status=sl_bt_gatt_server_read_attribute_value(
  attribute,
  offset,
  MAX_P_SIZE,
  &value_len,
  value
  );

    printf("rsp_gatt_server_read_attribute_value 0x%lx ",status);
    print_hex(value,value_len);
    printf("\n");
}
void sli_bt_cli_gatt_server_read_attribute_type(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t attribute=sl_cli_get_argument_uint16(arguments,0);
  //return values
  size_t type_len;
  uint8_t type[MAX_P_SIZE];
  status=sl_bt_gatt_server_read_attribute_type(
  attribute,
  MAX_P_SIZE,
  &type_len,
  type
  );

    printf("rsp_gatt_server_read_attribute_type 0x%lx ",status);
    print_hex(type,type_len);
    printf("\n");
}
void sli_bt_cli_gatt_server_read_attribute_properties(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t attribute=sl_cli_get_argument_uint16(arguments,0);
  //return values
  uint8_t category;
  uint16_t security;
  uint16_t properties;
  uint8_t value_type;
  uint16_t len;
  uint16_t max_writable_len;
  status=sl_bt_gatt_server_read_attribute_properties(
  attribute,
  &category,
  &security,
  &properties,
  &value_type,
  &len,
  &max_writable_len
  );

    printf("rsp_gatt_server_read_attribute_properties 0x%lx ",status);
    printf("0x%x ",category);
    printf("0x%x ",security);
    printf("0x%x ",properties);
    printf("0x%x ",value_type);
    printf("0x%x ",len);
    printf("0x%x ",max_writable_len);
    printf("\n");
}
void sli_bt_cli_gatt_server_write_attribute_value(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t attribute=sl_cli_get_argument_uint16(arguments,0);
  uint16_t offset=sl_cli_get_argument_uint16(arguments,1);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,2,&value_len);
  //return values
  status=sl_bt_gatt_server_write_attribute_value(
  attribute,
  offset,
  value_len,
  value
  );

    printf("rsp_gatt_server_write_attribute_value 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_send_user_read_response(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint8_t att_errorcode=sl_cli_get_argument_uint8(arguments,2);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,3,&value_len);
  //return values
  uint16_t sent_len;
  status=sl_bt_gatt_server_send_user_read_response(
  connection,
  characteristic,
  att_errorcode,
  value_len,
  value,
  &sent_len
  );

    printf("rsp_gatt_server_send_user_read_response 0x%lx ",status);
    printf("0x%x ",sent_len);
    printf("\n");
}
void sli_bt_cli_gatt_server_send_user_write_response(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint8_t att_errorcode=sl_cli_get_argument_uint8(arguments,2);
  //return values
  status=sl_bt_gatt_server_send_user_write_response(
  connection,
  characteristic,
  att_errorcode
  );

    printf("rsp_gatt_server_send_user_write_response 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_send_notification(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,2,&value_len);
  //return values
  status=sl_bt_gatt_server_send_notification(
  connection,
  characteristic,
  value_len,
  value
  );

    printf("rsp_gatt_server_send_notification 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_send_notification_with_options(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint32_t options=sl_cli_get_argument_uint32(arguments,2);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,3,&value_len);
  //return values
  status=sl_bt_gatt_server_send_notification_with_options(
  connection,
  characteristic,
  options,
  value_len,
  value
  );

    printf("rsp_gatt_server_send_notification_with_options 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_send_indication(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,2,&value_len);
  //return values
  status=sl_bt_gatt_server_send_indication(
  connection,
  characteristic,
  value_len,
  value
  );

    printf("rsp_gatt_server_send_indication 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_send_indication_with_options(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint32_t options=sl_cli_get_argument_uint32(arguments,2);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,3,&value_len);
  //return values
  status=sl_bt_gatt_server_send_indication_with_options(
  connection,
  characteristic,
  options,
  value_len,
  value
  );

    printf("rsp_gatt_server_send_indication_with_options 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_notify_all(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,0);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,1,&value_len);
  //return values
  status=sl_bt_gatt_server_notify_all(
  characteristic,
  value_len,
  value
  );

    printf("rsp_gatt_server_notify_all 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_read_client_configuration(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  //return values
  uint16_t client_config_flags;
  status=sl_bt_gatt_server_read_client_configuration(
  connection,
  characteristic,
  &client_config_flags
  );

    printf("rsp_gatt_server_read_client_configuration 0x%lx ",status);
    printf("0x%x ",client_config_flags);
    printf("\n");
}
void sli_bt_cli_gatt_server_send_user_prepare_write_response(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t characteristic=sl_cli_get_argument_uint16(arguments,1);
  uint8_t att_errorcode=sl_cli_get_argument_uint8(arguments,2);
  uint16_t offset=sl_cli_get_argument_uint16(arguments,3);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,4,&value_len);
  //return values
  status=sl_bt_gatt_server_send_user_prepare_write_response(
  connection,
  characteristic,
  att_errorcode,
  offset,
  value_len,
  value
  );

    printf("rsp_gatt_server_send_user_prepare_write_response 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_set_capabilities(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t caps=sl_cli_get_argument_uint32(arguments,0);
  uint32_t reserved=sl_cli_get_argument_uint32(arguments,1);
  //return values
  status=sl_bt_gatt_server_set_capabilities(
  caps,
  reserved
  );

    printf("rsp_gatt_server_set_capabilities 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_enable_capabilities(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t caps=sl_cli_get_argument_uint32(arguments,0);
  //return values
  status=sl_bt_gatt_server_enable_capabilities(
  caps
  );

    printf("rsp_gatt_server_enable_capabilities 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_disable_capabilities(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t caps=sl_cli_get_argument_uint32(arguments,0);
  //return values
  status=sl_bt_gatt_server_disable_capabilities(
  caps
  );

    printf("rsp_gatt_server_disable_capabilities 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_gatt_server_get_enabled_capabilities(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  uint32_t caps;
  status=sl_bt_gatt_server_get_enabled_capabilities(
  &caps
  );

    printf("rsp_gatt_server_get_enabled_capabilities 0x%lx ",status);
    printf("0x%x ",caps);
    printf("\n");
}
void sli_bt_cli_gatt_server_read_client_supported_features(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  uint8_t client_features;
  status=sl_bt_gatt_server_read_client_supported_features(
  connection,
  &client_features
  );

    printf("rsp_gatt_server_read_client_supported_features 0x%lx ",status);
    printf("0x%x ",client_features);
    printf("\n");
}
void sli_bt_cli_nvm_save(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t key=sl_cli_get_argument_uint16(arguments,0);
  size_t value_len;
  uint8_t *value=sl_cli_get_argument_hex(arguments,1,&value_len);
  //return values
  status=sl_bt_nvm_save(
  key,
  value_len,
  value
  );

    printf("rsp_nvm_save 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_nvm_load(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t key=sl_cli_get_argument_uint16(arguments,0);
  //return values
  size_t value_len;
  uint8_t value[MAX_P_SIZE];
  status=sl_bt_nvm_load(
  key,
  MAX_P_SIZE,
  &value_len,
  value
  );

    printf("rsp_nvm_load 0x%lx ",status);
    print_hex(value,value_len);
    printf("\n");
}
void sli_bt_cli_nvm_erase(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t key=sl_cli_get_argument_uint16(arguments,0);
  //return values
  status=sl_bt_nvm_erase(
  key
  );

    printf("rsp_nvm_erase 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_nvm_erase_all(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_nvm_erase_all(
  );

    printf("rsp_nvm_erase_all 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_test_dtm_tx_v4(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t packet_type=sl_cli_get_argument_uint8(arguments,0);
  uint8_t length=sl_cli_get_argument_uint8(arguments,1);
  uint8_t channel=sl_cli_get_argument_uint8(arguments,2);
  uint8_t phy=sl_cli_get_argument_uint8(arguments,3);
  int8_t power_level=sl_cli_get_argument_int8(arguments,4);
  //return values
  status=sl_bt_test_dtm_tx_v4(
  packet_type,
  length,
  channel,
  phy,
  power_level
  );

    printf("rsp_test_dtm_tx_v4 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_test_dtm_tx_cw(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t packet_type=sl_cli_get_argument_uint8(arguments,0);
  uint8_t channel=sl_cli_get_argument_uint8(arguments,1);
  uint8_t phy=sl_cli_get_argument_uint8(arguments,2);
  int16_t power_level=sl_cli_get_argument_int16(arguments,3);
  //return values
  status=sl_bt_test_dtm_tx_cw(
  packet_type,
  channel,
  phy,
  power_level
  );

    printf("rsp_test_dtm_tx_cw 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_test_dtm_rx(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t channel=sl_cli_get_argument_uint8(arguments,0);
  uint8_t phy=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_test_dtm_rx(
  channel,
  phy
  );

    printf("rsp_test_dtm_rx 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_test_dtm_end(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_test_dtm_end(
  );

    printf("rsp_test_dtm_end 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_configure(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t flags=sl_cli_get_argument_uint8(arguments,0);
  uint8_t io_capabilities=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_sm_configure(
  flags,
  io_capabilities
  );

    printf("rsp_sm_configure 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_set_minimum_key_size(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t minimum_key_size=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_sm_set_minimum_key_size(
  minimum_key_size
  );

    printf("rsp_sm_set_minimum_key_size 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_set_debug_mode(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_sm_set_debug_mode(
  );

    printf("rsp_sm_set_debug_mode 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_store_bonding_configuration(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t max_bonding_count=sl_cli_get_argument_uint8(arguments,0);
  uint8_t policy_flags=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_sm_store_bonding_configuration(
  max_bonding_count,
  policy_flags
  );

    printf("rsp_sm_store_bonding_configuration 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_set_bondable_mode(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t bondable=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_sm_set_bondable_mode(
  bondable
  );

    printf("rsp_sm_set_bondable_mode 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_set_passkey(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  int32_t passkey=sl_cli_get_argument_int32(arguments,0);
  //return values
  status=sl_bt_sm_set_passkey(
  passkey
  );

    printf("rsp_sm_set_passkey 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_increase_security(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_sm_increase_security(
  connection
  );

    printf("rsp_sm_increase_security 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_enter_passkey(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  int32_t passkey=sl_cli_get_argument_int32(arguments,1);
  //return values
  status=sl_bt_sm_enter_passkey(
  connection,
  passkey
  );

    printf("rsp_sm_enter_passkey 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_passkey_confirm(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t confirm=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_sm_passkey_confirm(
  connection,
  confirm
  );

    printf("rsp_sm_passkey_confirm 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_bonding_confirm(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t confirm=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_sm_bonding_confirm(
  connection,
  confirm
  );

    printf("rsp_sm_bonding_confirm 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_delete_bonding(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t bonding=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_sm_delete_bonding(
  bonding
  );

    printf("rsp_sm_delete_bonding 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_delete_bondings(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_sm_delete_bondings(
  );

    printf("rsp_sm_delete_bondings 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_get_bonding_handles(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t reserved=sl_cli_get_argument_uint32(arguments,0);
  //return values
  uint32_t num_bondings;
  size_t bondings_len;
  uint8_t bondings[MAX_P_SIZE];
  status=sl_bt_sm_get_bonding_handles(
  reserved,
  &num_bondings,
  MAX_P_SIZE,
  &bondings_len,
  bondings
  );

    printf("rsp_sm_get_bonding_handles 0x%lx ",status);
    printf("0x%x ",num_bondings);
    print_hex(bondings,bondings_len);
    printf("\n");
}
void sli_bt_cli_sm_get_bonding_details(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t bonding=sl_cli_get_argument_uint32(arguments,0);
  //return values
  bd_addr address;
  uint8_t address_type;
  uint8_t security_mode;
  uint8_t key_size;
  status=sl_bt_sm_get_bonding_details(
  bonding,
  &address,
  &address_type,
  &security_mode,
  &key_size
  );

    printf("rsp_sm_get_bonding_details 0x%lx ",status);
    print_hex(address.addr,sizeof(address.addr));
    printf("0x%x ",address_type);
    printf("0x%x ",security_mode);
    printf("0x%x ",key_size);
    printf("\n");
}
void sli_bt_cli_sm_find_bonding_by_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  //return values
  uint32_t bonding;
  uint8_t security_mode;
  uint8_t key_size;
  status=sl_bt_sm_find_bonding_by_address(
  address,
  &bonding,
  &security_mode,
  &key_size
  );

    printf("rsp_sm_find_bonding_by_address 0x%lx ",status);
    printf("0x%x ",bonding);
    printf("0x%x ",security_mode);
    printf("0x%x ",key_size);
    printf("\n");
}
void sli_bt_cli_sm_resolve_rpa(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _rpa_len;
  uint8_t *_rpa=sl_cli_get_argument_hex(arguments,0,&_rpa_len);
  bd_addr rpa;
  memcpy(&rpa,_rpa,sizeof(rpa));
  //return values
  bd_addr address;
  uint8_t address_type;
  uint32_t bonding;
  status=sl_bt_sm_resolve_rpa(
  rpa,
  &address,
  &address_type,
  &bonding
  );

    printf("rsp_sm_resolve_rpa 0x%lx ",status);
    print_hex(address.addr,sizeof(address.addr));
    printf("0x%x ",address_type);
    printf("0x%x ",bonding);
    printf("\n");
}
void sli_bt_cli_sm_set_legacy_oob(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t enable=sl_cli_get_argument_uint8(arguments,0);
  size_t _oob_data_len;
  uint8_t *_oob_data=sl_cli_get_argument_hex(arguments,1,&_oob_data_len);
  aes_key_128 oob_data;
  memcpy(&oob_data,_oob_data,sizeof(oob_data));
  //return values
  status=sl_bt_sm_set_legacy_oob(
  enable,
  oob_data
  );

    printf("rsp_sm_set_legacy_oob 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_sm_set_oob(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t enable=sl_cli_get_argument_uint8(arguments,0);
  //return values
  aes_key_128 random;
  aes_key_128 confirm;
  status=sl_bt_sm_set_oob(
  enable,
  &random,
  &confirm
  );

    printf("rsp_sm_set_oob 0x%lx ",status);
    print_hex(random.data,sizeof(random.data));
    print_hex(confirm.data,sizeof(confirm.data));
    printf("\n");
}
void sli_bt_cli_sm_set_remote_oob(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t enable=sl_cli_get_argument_uint8(arguments,0);
  size_t _random_len;
  uint8_t *_random=sl_cli_get_argument_hex(arguments,1,&_random_len);
  aes_key_128 random;
  memcpy(&random,_random,sizeof(random));
  size_t _confirm_len;
  uint8_t *_confirm=sl_cli_get_argument_hex(arguments,2,&_confirm_len);
  aes_key_128 confirm;
  memcpy(&confirm,_confirm,sizeof(confirm));
  //return values
  status=sl_bt_sm_set_remote_oob(
  enable,
  random,
  confirm
  );

    printf("rsp_sm_set_remote_oob 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_external_bondingdb_set_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t type=sl_cli_get_argument_uint8(arguments,1);
  size_t data_len;
  uint8_t *data=sl_cli_get_argument_hex(arguments,2,&data_len);
  //return values
  status=sl_bt_external_bondingdb_set_data(
  connection,
  type,
  data_len,
  data
  );

    printf("rsp_external_bondingdb_set_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_external_bondingdb_set_local_irk(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t irk_len;
  uint8_t *irk=sl_cli_get_argument_hex(arguments,0,&irk_len);
  //return values
  status=sl_bt_external_bondingdb_set_local_irk(
  irk_len,
  irk
  );

    printf("rsp_external_bondingdb_set_local_irk 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_resolving_list_add_device_by_bonding(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t bonding=sl_cli_get_argument_uint32(arguments,0);
  uint8_t privacy_mode=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_resolving_list_add_device_by_bonding(
  bonding,
  privacy_mode
  );

    printf("rsp_resolving_list_add_device_by_bonding 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_resolving_list_add_device_by_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t address_type=sl_cli_get_argument_uint8(arguments,1);
  size_t _key_len;
  uint8_t *_key=sl_cli_get_argument_hex(arguments,2,&_key_len);
  aes_key_128 key;
  memcpy(&key,_key,sizeof(key));
  uint8_t privacy_mode=sl_cli_get_argument_uint8(arguments,3);
  //return values
  status=sl_bt_resolving_list_add_device_by_address(
  address,
  address_type,
  key,
  privacy_mode
  );

    printf("rsp_resolving_list_add_device_by_address 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_resolving_list_remove_device_by_bonding(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t bonding=sl_cli_get_argument_uint32(arguments,0);
  //return values
  status=sl_bt_resolving_list_remove_device_by_bonding(
  bonding
  );

    printf("rsp_resolving_list_remove_device_by_bonding 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_resolving_list_remove_device_by_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t address_type=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_resolving_list_remove_device_by_address(
  address,
  address_type
  );

    printf("rsp_resolving_list_remove_device_by_address 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_resolving_list_remove_all_devices(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_resolving_list_remove_all_devices(
  );

    printf("rsp_resolving_list_remove_all_devices 0x%lx ",status);
    printf("\n");
}
#ifdef SL_CATALOG_BLUETOOTH_FEATURE_ACCEPT_LIST_PRESENT
void sli_bt_cli_accept_list_add_device_by_bonding(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t bonding=sl_cli_get_argument_uint32(arguments,0);
  //return values
  status=sl_bt_accept_list_add_device_by_bonding(
  bonding
  );

    printf("rsp_accept_list_add_device_by_bonding 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_accept_list_add_device_by_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t address_type=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_accept_list_add_device_by_address(
  address,
  address_type
  );

    printf("rsp_accept_list_add_device_by_address 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_accept_list_remove_device_by_bonding(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t bonding=sl_cli_get_argument_uint32(arguments,0);
  //return values
  status=sl_bt_accept_list_remove_device_by_bonding(
  bonding
  );

    printf("rsp_accept_list_remove_device_by_bonding 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_accept_list_remove_device_by_address(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _address_len;
  uint8_t *_address=sl_cli_get_argument_hex(arguments,0,&_address_len);
  bd_addr address;
  memcpy(&address,_address,sizeof(address));
  uint8_t address_type=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_accept_list_remove_device_by_address(
  address,
  address_type
  );

    printf("rsp_accept_list_remove_device_by_address 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_accept_list_remove_all_devices(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_accept_list_remove_all_devices(
  );

    printf("rsp_accept_list_remove_all_devices 0x%lx ",status);
    printf("\n");
}
#endif // SL_CATALOG_BLUETOOTH_FEATURE_ACCEPT_LIST_PRESENT
void sli_bt_cli_coex_set_options(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t mask=sl_cli_get_argument_uint32(arguments,0);
  uint32_t options=sl_cli_get_argument_uint32(arguments,1);
  //return values
  status=sl_bt_coex_set_options(
  mask,
  options
  );

    printf("rsp_coex_set_options 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_coex_set_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t priority=sl_cli_get_argument_uint8(arguments,0);
  uint8_t request=sl_cli_get_argument_uint8(arguments,1);
  uint8_t pwm_period=sl_cli_get_argument_uint8(arguments,2);
  uint8_t pwm_dutycycle=sl_cli_get_argument_uint8(arguments,3);
  //return values
  status=sl_bt_coex_set_parameters(
  priority,
  request,
  pwm_period,
  pwm_dutycycle
  );

    printf("rsp_coex_set_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_coex_set_directional_priority_pulse(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t pulse=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_coex_set_directional_priority_pulse(
  pulse
  );

    printf("rsp_coex_set_directional_priority_pulse 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_coex_get_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  uint8_t priority;
  uint8_t request;
  uint8_t pwm_period;
  uint8_t pwm_dutycycle;
  status=sl_bt_coex_get_parameters(
  &priority,
  &request,
  &pwm_period,
  &pwm_dutycycle
  );

    printf("rsp_coex_get_parameters 0x%lx ",status);
    printf("0x%x ",priority);
    printf("0x%x ",request);
    printf("0x%x ",pwm_period);
    printf("0x%x ",pwm_dutycycle);
    printf("\n");
}
void sli_bt_cli_coex_get_counters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t reset=sl_cli_get_argument_uint8(arguments,0);
  //return values
  size_t counters_len;
  uint8_t counters[MAX_P_SIZE];
  status=sl_bt_coex_get_counters(
  reset,
  MAX_P_SIZE,
  &counters_len,
  counters
  );

    printf("rsp_coex_get_counters 0x%lx ",status);
    print_hex(counters,counters_len);
    printf("\n");
}
#ifdef SL_CATALOG_BLUETOOTH_CS_SUPPORT_PRESENT
void sli_bt_cli_cs_security_enable(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_cs_security_enable(
  connection
  );

    printf("rsp_cs_security_enable 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_set_default_settings(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t initiator_status=sl_cli_get_argument_uint8(arguments,1);
  uint8_t reflector_status=sl_cli_get_argument_uint8(arguments,2);
  uint8_t antenna_identifier=sl_cli_get_argument_uint8(arguments,3);
  int8_t max_tx_power=sl_cli_get_argument_int8(arguments,4);
  //return values
  status=sl_bt_cs_set_default_settings(
  connection,
  initiator_status,
  reflector_status,
  antenna_identifier,
  max_tx_power
  );

    printf("rsp_cs_set_default_settings 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_create_config(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t config_id=sl_cli_get_argument_uint8(arguments,1);
  uint8_t create_context=sl_cli_get_argument_uint8(arguments,2);
  uint8_t main_mode_type=sl_cli_get_argument_uint8(arguments,3);
  uint8_t sub_mode_type=sl_cli_get_argument_uint8(arguments,4);
  uint8_t min_main_mode_steps=sl_cli_get_argument_uint8(arguments,5);
  uint8_t max_main_mode_steps=sl_cli_get_argument_uint8(arguments,6);
  uint8_t main_mode_repetition=sl_cli_get_argument_uint8(arguments,7);
  uint8_t mode_calibration_steps=sl_cli_get_argument_uint8(arguments,8);
  uint8_t role=sl_cli_get_argument_uint8(arguments,9);
  uint8_t rtt_type=sl_cli_get_argument_uint8(arguments,10);
  uint8_t cs_sync_phy=sl_cli_get_argument_uint8(arguments,11);
  size_t _channel_map_len;
  uint8_t *_channel_map=sl_cli_get_argument_hex(arguments,12,&_channel_map_len);
  sl_bt_cs_channel_map_t channel_map;
  memcpy(&channel_map,_channel_map,sizeof(channel_map));
  uint8_t channel_map_repetition=sl_cli_get_argument_uint8(arguments,13);
  uint8_t channel_selection_type=sl_cli_get_argument_uint8(arguments,14);
  uint8_t ch3c_shape=sl_cli_get_argument_uint8(arguments,15);
  uint8_t ch3c_jump=sl_cli_get_argument_uint8(arguments,16);
  uint8_t reserved=sl_cli_get_argument_uint8(arguments,17);
  //return values
  status=sl_bt_cs_create_config(
  connection,
  config_id,
  create_context,
  main_mode_type,
  sub_mode_type,
  min_main_mode_steps,
  max_main_mode_steps,
  main_mode_repetition,
  mode_calibration_steps,
  role,
  rtt_type,
  cs_sync_phy,
  &channel_map,
  channel_map_repetition,
  channel_selection_type,
  ch3c_shape,
  ch3c_jump,
  reserved
  );

    printf("rsp_cs_create_config 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_remove_config(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t config_id=sl_cli_get_argument_uint8(arguments,1);
  //return values
  status=sl_bt_cs_remove_config(
  connection,
  config_id
  );

    printf("rsp_cs_remove_config 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_set_channel_classification(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t _channel_map_len;
  uint8_t *_channel_map=sl_cli_get_argument_hex(arguments,0,&_channel_map_len);
  sl_bt_cs_channel_map_t channel_map;
  memcpy(&channel_map,_channel_map,sizeof(channel_map));
  //return values
  status=sl_bt_cs_set_channel_classification(
  &channel_map
  );

    printf("rsp_cs_set_channel_classification 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_set_procedure_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t config_id=sl_cli_get_argument_uint8(arguments,1);
  uint16_t max_procedure_len=sl_cli_get_argument_uint16(arguments,2);
  uint16_t min_procedure_interval=sl_cli_get_argument_uint16(arguments,3);
  uint16_t max_procedure_interval=sl_cli_get_argument_uint16(arguments,4);
  uint16_t max_procedure_count=sl_cli_get_argument_uint16(arguments,5);
  uint32_t min_subevent_len=sl_cli_get_argument_uint32(arguments,6);
  uint32_t max_subevent_len=sl_cli_get_argument_uint32(arguments,7);
  uint8_t tone_antenna_config_selection=sl_cli_get_argument_uint8(arguments,8);
  uint8_t phy=sl_cli_get_argument_uint8(arguments,9);
  int8_t tx_pwr_delta=sl_cli_get_argument_int8(arguments,10);
  uint8_t preferred_peer_antenna=sl_cli_get_argument_uint8(arguments,11);
  uint8_t snr_control_initiator=sl_cli_get_argument_uint8(arguments,12);
  uint8_t snr_control_reflector=sl_cli_get_argument_uint8(arguments,13);
  //return values
  status=sl_bt_cs_set_procedure_parameters(
  connection,
  config_id,
  max_procedure_len,
  min_procedure_interval,
  max_procedure_interval,
  max_procedure_count,
  min_subevent_len,
  max_subevent_len,
  tone_antenna_config_selection,
  phy,
  tx_pwr_delta,
  preferred_peer_antenna,
  snr_control_initiator,
  snr_control_reflector
  );

    printf("rsp_cs_set_procedure_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_procedure_enable(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t enable=sl_cli_get_argument_uint8(arguments,1);
  uint8_t config_id=sl_cli_get_argument_uint8(arguments,2);
  //return values
  status=sl_bt_cs_procedure_enable(
  connection,
  enable,
  config_id
  );

    printf("rsp_cs_procedure_enable 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_set_antenna_configuration(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  size_t antenna_element_offset_len;
  uint8_t *antenna_element_offset=sl_cli_get_argument_hex(arguments,0,&antenna_element_offset_len);
  //return values
  status=sl_bt_cs_set_antenna_configuration(
  antenna_element_offset_len,
  antenna_element_offset
  );

    printf("rsp_cs_set_antenna_configuration 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_read_local_supported_capabilities(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  uint8_t num_config;
  uint16_t max_consecutive_procedures;
  uint8_t num_antennas;
  uint8_t max_antenna_paths;
  uint8_t roles;
  uint8_t modes;
  uint8_t rtt_capability;
  uint8_t rtt_aa_only;
  uint8_t rtt_sounding;
  uint8_t rtt_random_payload;
  uint16_t nadm_sounding_capability;
  uint16_t nadm_random_capability;
  uint8_t cs_sync_phys;
  uint16_t subfeatures;
  uint16_t t_ip1_times;
  uint16_t t_ip2_times;
  uint16_t t_fcs_times;
  uint16_t t_pm_times;
  uint8_t t_sw_times;
  uint8_t tx_snr_capability;
  status=sl_bt_cs_read_local_supported_capabilities(
  &num_config,
  &max_consecutive_procedures,
  &num_antennas,
  &max_antenna_paths,
  &roles,
  &modes,
  &rtt_capability,
  &rtt_aa_only,
  &rtt_sounding,
  &rtt_random_payload,
  &nadm_sounding_capability,
  &nadm_random_capability,
  &cs_sync_phys,
  &subfeatures,
  &t_ip1_times,
  &t_ip2_times,
  &t_fcs_times,
  &t_pm_times,
  &t_sw_times,
  &tx_snr_capability
  );

    printf("rsp_cs_read_local_supported_capabilities 0x%lx ",status);
    printf("0x%x ",num_config);
    printf("0x%x ",max_consecutive_procedures);
    printf("0x%x ",num_antennas);
    printf("0x%x ",max_antenna_paths);
    printf("0x%x ",roles);
    printf("0x%x ",modes);
    printf("0x%x ",rtt_capability);
    printf("0x%x ",rtt_aa_only);
    printf("0x%x ",rtt_sounding);
    printf("0x%x ",rtt_random_payload);
    printf("0x%x ",nadm_sounding_capability);
    printf("0x%x ",nadm_random_capability);
    printf("0x%x ",cs_sync_phys);
    printf("0x%x ",subfeatures);
    printf("0x%x ",t_ip1_times);
    printf("0x%x ",t_ip2_times);
    printf("0x%x ",t_fcs_times);
    printf("0x%x ",t_pm_times);
    printf("0x%x ",t_sw_times);
    printf("0x%x ",tx_snr_capability);
    printf("\n");
}
void sli_bt_cli_cs_read_remote_supported_capabilities(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_cs_read_remote_supported_capabilities(
  connection
  );

    printf("rsp_cs_read_remote_supported_capabilities 0x%lx ",status);
    printf("\n");
}
#endif // SL_CATALOG_BLUETOOTH_CS_SUPPORT_PRESENT
#ifdef SL_CATALOG_BLUETOOTH_CS_SUPPORT_PRESENT
void sli_bt_cli_cs_test_start(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t main_mode_type=sl_cli_get_argument_uint8(arguments,0);
  uint8_t sub_mode_type=sl_cli_get_argument_uint8(arguments,1);
  uint8_t main_mode_repetition=sl_cli_get_argument_uint8(arguments,2);
  uint8_t mode_calibration_steps=sl_cli_get_argument_uint8(arguments,3);
  uint8_t role=sl_cli_get_argument_uint8(arguments,4);
  uint8_t rtt_type=sl_cli_get_argument_uint8(arguments,5);
  uint8_t cs_sync_phy=sl_cli_get_argument_uint8(arguments,6);
  uint8_t antenna_selection=sl_cli_get_argument_uint8(arguments,7);
  size_t _subevent_len_len;
  uint8_t *_subevent_len=sl_cli_get_argument_hex(arguments,8,&_subevent_len_len);
  sl_bt_cs_subevent_length_t subevent_len;
  memcpy(&subevent_len,_subevent_len,sizeof(subevent_len));
  uint16_t subevent_interval=sl_cli_get_argument_uint16(arguments,9);
  uint8_t max_num_subevents=sl_cli_get_argument_uint8(arguments,10);
  int8_t tx_power=sl_cli_get_argument_int8(arguments,11);
  uint8_t t_ip1_time=sl_cli_get_argument_uint8(arguments,12);
  uint8_t t_ip2_time=sl_cli_get_argument_uint8(arguments,13);
  uint8_t t_fcs_time=sl_cli_get_argument_uint8(arguments,14);
  uint8_t t_pm_time=sl_cli_get_argument_uint8(arguments,15);
  uint8_t t_sw_time=sl_cli_get_argument_uint8(arguments,16);
  uint8_t tone_antenna_config=sl_cli_get_argument_uint8(arguments,17);
  uint8_t reserved=sl_cli_get_argument_uint8(arguments,18);
  uint8_t snr_control_initiator=sl_cli_get_argument_uint8(arguments,19);
  uint8_t snr_control_reflector=sl_cli_get_argument_uint8(arguments,20);
  uint16_t drbg_nonce=sl_cli_get_argument_uint16(arguments,21);
  uint8_t channel_map_repetition=sl_cli_get_argument_uint8(arguments,22);
  uint16_t override_config=sl_cli_get_argument_uint16(arguments,23);
  size_t override_parameters_len;
  uint8_t *override_parameters=sl_cli_get_argument_hex(arguments,24,&override_parameters_len);
  //return values
  status=sl_bt_cs_test_start(
  main_mode_type,
  sub_mode_type,
  main_mode_repetition,
  mode_calibration_steps,
  role,
  rtt_type,
  cs_sync_phy,
  antenna_selection,
  &subevent_len,
  subevent_interval,
  max_num_subevents,
  tx_power,
  t_ip1_time,
  t_ip2_time,
  t_fcs_time,
  t_pm_time,
  t_sw_time,
  tone_antenna_config,
  reserved,
  snr_control_initiator,
  snr_control_reflector,
  drbg_nonce,
  channel_map_repetition,
  override_config,
  override_parameters_len,
  override_parameters
  );

    printf("rsp_cs_test_start 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cs_test_end(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_cs_test_end(
  );

    printf("rsp_cs_test_end 0x%lx ",status);
    printf("\n");
}
#endif // SL_CATALOG_BLUETOOTH_CS_SUPPORT_PRESENT
void sli_bt_cli_l2cap_open_le_channel(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t spsm=sl_cli_get_argument_uint16(arguments,1);
  uint16_t max_sdu=sl_cli_get_argument_uint16(arguments,2);
  uint16_t max_pdu=sl_cli_get_argument_uint16(arguments,3);
  uint16_t credit=sl_cli_get_argument_uint16(arguments,4);
  //return values
  uint16_t cid;
  status=sl_bt_l2cap_open_le_channel(
  connection,
  spsm,
  max_sdu,
  max_pdu,
  credit,
  &cid
  );

    printf("rsp_l2cap_open_le_channel 0x%lx ",status);
    printf("0x%x ",cid);
    printf("\n");
}
void sli_bt_cli_l2cap_send_le_channel_open_response(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t cid=sl_cli_get_argument_uint16(arguments,1);
  uint16_t max_sdu=sl_cli_get_argument_uint16(arguments,2);
  uint16_t max_pdu=sl_cli_get_argument_uint16(arguments,3);
  uint16_t credit=sl_cli_get_argument_uint16(arguments,4);
  uint16_t errorcode=sl_cli_get_argument_uint16(arguments,5);
  //return values
  status=sl_bt_l2cap_send_le_channel_open_response(
  connection,
  cid,
  max_sdu,
  max_pdu,
  credit,
  errorcode
  );

    printf("rsp_l2cap_send_le_channel_open_response 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_l2cap_channel_send_data(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t cid=sl_cli_get_argument_uint16(arguments,1);
  size_t data_len;
  uint8_t *data=sl_cli_get_argument_hex(arguments,2,&data_len);
  //return values
  status=sl_bt_l2cap_channel_send_data(
  connection,
  cid,
  data_len,
  data
  );

    printf("rsp_l2cap_channel_send_data 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_l2cap_channel_send_credit(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t cid=sl_cli_get_argument_uint16(arguments,1);
  uint16_t credit=sl_cli_get_argument_uint16(arguments,2);
  //return values
  status=sl_bt_l2cap_channel_send_credit(
  connection,
  cid,
  credit
  );

    printf("rsp_l2cap_channel_send_credit 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_l2cap_close_channel(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t cid=sl_cli_get_argument_uint16(arguments,1);
  //return values
  status=sl_bt_l2cap_close_channel(
  connection,
  cid
  );

    printf("rsp_l2cap_close_channel 0x%lx ",status);
    printf("\n");
}
#ifdef SL_CATALOG_BLUETOOTH_FEATURE_CTE_TRANSMITTER_PRESENT
void sli_bt_cli_cte_transmitter_set_dtm_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t cte_length=sl_cli_get_argument_uint8(arguments,0);
  uint8_t cte_type=sl_cli_get_argument_uint8(arguments,1);
  size_t switching_pattern_len;
  uint8_t *switching_pattern=sl_cli_get_argument_hex(arguments,2,&switching_pattern_len);
  //return values
  status=sl_bt_cte_transmitter_set_dtm_parameters(
  cte_length,
  cte_type,
  switching_pattern_len,
  switching_pattern
  );

    printf("rsp_cte_transmitter_set_dtm_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_transmitter_clear_dtm_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_cte_transmitter_clear_dtm_parameters(
  );

    printf("rsp_cte_transmitter_clear_dtm_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_transmitter_enable_connection_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t cte_types=sl_cli_get_argument_uint8(arguments,1);
  size_t switching_pattern_len;
  uint8_t *switching_pattern=sl_cli_get_argument_hex(arguments,2,&switching_pattern_len);
  //return values
  status=sl_bt_cte_transmitter_enable_connection_cte(
  connection,
  cte_types,
  switching_pattern_len,
  switching_pattern
  );

    printf("rsp_cte_transmitter_enable_connection_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_transmitter_disable_connection_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_cte_transmitter_disable_connection_cte(
  connection
  );

    printf("rsp_cte_transmitter_disable_connection_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_transmitter_enable_connectionless_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t handle=sl_cli_get_argument_uint8(arguments,0);
  uint8_t cte_length=sl_cli_get_argument_uint8(arguments,1);
  uint8_t cte_type=sl_cli_get_argument_uint8(arguments,2);
  uint8_t cte_count=sl_cli_get_argument_uint8(arguments,3);
  size_t switching_pattern_len;
  uint8_t *switching_pattern=sl_cli_get_argument_hex(arguments,4,&switching_pattern_len);
  //return values
  status=sl_bt_cte_transmitter_enable_connectionless_cte(
  handle,
  cte_length,
  cte_type,
  cte_count,
  switching_pattern_len,
  switching_pattern
  );

    printf("rsp_cte_transmitter_enable_connectionless_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_transmitter_disable_connectionless_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t handle=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_cte_transmitter_disable_connectionless_cte(
  handle
  );

    printf("rsp_cte_transmitter_disable_connectionless_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_transmitter_enable_silabs_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t handle=sl_cli_get_argument_uint8(arguments,0);
  uint8_t cte_length=sl_cli_get_argument_uint8(arguments,1);
  uint8_t cte_type=sl_cli_get_argument_uint8(arguments,2);
  uint8_t cte_count=sl_cli_get_argument_uint8(arguments,3);
  size_t switching_pattern_len;
  uint8_t *switching_pattern=sl_cli_get_argument_hex(arguments,4,&switching_pattern_len);
  //return values
  status=sl_bt_cte_transmitter_enable_silabs_cte(
  handle,
  cte_length,
  cte_type,
  cte_count,
  switching_pattern_len,
  switching_pattern
  );

    printf("rsp_cte_transmitter_enable_silabs_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_transmitter_disable_silabs_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t handle=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_cte_transmitter_disable_silabs_cte(
  handle
  );

    printf("rsp_cte_transmitter_disable_silabs_cte 0x%lx ",status);
    printf("\n");
}
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CTE_TRANSMITTER_PRESENT
#ifdef SL_CATALOG_BLUETOOTH_FEATURE_CTE_RECEIVER_PRESENT
void sli_bt_cli_cte_receiver_set_dtm_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t cte_length=sl_cli_get_argument_uint8(arguments,0);
  uint8_t cte_type=sl_cli_get_argument_uint8(arguments,1);
  uint8_t slot_durations=sl_cli_get_argument_uint8(arguments,2);
  size_t switching_pattern_len;
  uint8_t *switching_pattern=sl_cli_get_argument_hex(arguments,3,&switching_pattern_len);
  //return values
  status=sl_bt_cte_receiver_set_dtm_parameters(
  cte_length,
  cte_type,
  slot_durations,
  switching_pattern_len,
  switching_pattern
  );

    printf("rsp_cte_receiver_set_dtm_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_clear_dtm_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_cte_receiver_clear_dtm_parameters(
  );

    printf("rsp_cte_receiver_clear_dtm_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_set_sync_cte_type(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t sync_cte_type=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_cte_receiver_set_sync_cte_type(
  sync_cte_type
  );

    printf("rsp_cte_receiver_set_sync_cte_type 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_set_default_sync_receive_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t mode=sl_cli_get_argument_uint8(arguments,0);
  uint16_t skip=sl_cli_get_argument_uint16(arguments,1);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,2);
  uint8_t sync_cte_type=sl_cli_get_argument_uint8(arguments,3);
  uint8_t reporting_mode=sl_cli_get_argument_uint8(arguments,4);
  //return values
  status=sl_bt_cte_receiver_set_default_sync_receive_parameters(
  mode,
  skip,
  timeout,
  sync_cte_type,
  reporting_mode
  );

    printf("rsp_cte_receiver_set_default_sync_receive_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_set_sync_receive_parameters(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint8_t mode=sl_cli_get_argument_uint8(arguments,1);
  uint16_t skip=sl_cli_get_argument_uint16(arguments,2);
  uint16_t timeout=sl_cli_get_argument_uint16(arguments,3);
  uint8_t sync_cte_type=sl_cli_get_argument_uint8(arguments,4);
  uint8_t reporting_mode=sl_cli_get_argument_uint8(arguments,5);
  //return values
  status=sl_bt_cte_receiver_set_sync_receive_parameters(
  connection,
  mode,
  skip,
  timeout,
  sync_cte_type,
  reporting_mode
  );

    printf("rsp_cte_receiver_set_sync_receive_parameters 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_configure(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t flags=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_cte_receiver_configure(
  flags
  );

    printf("rsp_cte_receiver_configure 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_enable_connection_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  uint16_t interval=sl_cli_get_argument_uint16(arguments,1);
  uint8_t cte_length=sl_cli_get_argument_uint8(arguments,2);
  uint8_t cte_type=sl_cli_get_argument_uint8(arguments,3);
  uint8_t slot_durations=sl_cli_get_argument_uint8(arguments,4);
  size_t switching_pattern_len;
  uint8_t *switching_pattern=sl_cli_get_argument_hex(arguments,5,&switching_pattern_len);
  //return values
  status=sl_bt_cte_receiver_enable_connection_cte(
  connection,
  interval,
  cte_length,
  cte_type,
  slot_durations,
  switching_pattern_len,
  switching_pattern
  );

    printf("rsp_cte_receiver_enable_connection_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_disable_connection_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t connection=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_cte_receiver_disable_connection_cte(
  connection
  );

    printf("rsp_cte_receiver_disable_connection_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_enable_connectionless_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t sync=sl_cli_get_argument_uint16(arguments,0);
  uint8_t slot_durations=sl_cli_get_argument_uint8(arguments,1);
  uint8_t cte_count=sl_cli_get_argument_uint8(arguments,2);
  size_t switching_pattern_len;
  uint8_t *switching_pattern=sl_cli_get_argument_hex(arguments,3,&switching_pattern_len);
  //return values
  status=sl_bt_cte_receiver_enable_connectionless_cte(
  sync,
  slot_durations,
  cte_count,
  switching_pattern_len,
  switching_pattern
  );

    printf("rsp_cte_receiver_enable_connectionless_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_disable_connectionless_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint16_t sync=sl_cli_get_argument_uint16(arguments,0);
  //return values
  status=sl_bt_cte_receiver_disable_connectionless_cte(
  sync
  );

    printf("rsp_cte_receiver_disable_connectionless_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_enable_silabs_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t slot_durations=sl_cli_get_argument_uint8(arguments,0);
  uint8_t cte_count=sl_cli_get_argument_uint8(arguments,1);
  size_t switching_pattern_len;
  uint8_t *switching_pattern=sl_cli_get_argument_hex(arguments,2,&switching_pattern_len);
  //return values
  status=sl_bt_cte_receiver_enable_silabs_cte(
  slot_durations,
  cte_count,
  switching_pattern_len,
  switching_pattern
  );

    printf("rsp_cte_receiver_enable_silabs_cte 0x%lx ",status);
    printf("\n");
}
void sli_bt_cli_cte_receiver_disable_silabs_cte(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  //return values
  status=sl_bt_cte_receiver_disable_silabs_cte(
  );

    printf("rsp_cte_receiver_disable_silabs_cte 0x%lx ",status);
    printf("\n");
}
#endif // SL_CATALOG_BLUETOOTH_FEATURE_CTE_RECEIVER_PRESENT
void sli_bt_cli_connection_analyzer_start(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint32_t access_address=sl_cli_get_argument_uint32(arguments,0);
  uint32_t crc_init=sl_cli_get_argument_uint32(arguments,1);
  uint16_t interval=sl_cli_get_argument_uint16(arguments,2);
  uint16_t supervision_timeout=sl_cli_get_argument_uint16(arguments,3);
  uint8_t central_clock_accuracy=sl_cli_get_argument_uint8(arguments,4);
  uint8_t central_phy=sl_cli_get_argument_uint8(arguments,5);
  uint8_t peripheral_phy=sl_cli_get_argument_uint8(arguments,6);
  uint8_t channel_selection_algorithm=sl_cli_get_argument_uint8(arguments,7);
  uint8_t hop=sl_cli_get_argument_uint8(arguments,8);
  size_t _channel_map_len;
  uint8_t *_channel_map=sl_cli_get_argument_hex(arguments,9,&_channel_map_len);
  sl_bt_connection_channel_map_t channel_map;
  memcpy(&channel_map,_channel_map,sizeof(channel_map));
  uint8_t channel=sl_cli_get_argument_uint8(arguments,10);
  uint16_t event_counter=sl_cli_get_argument_uint16(arguments,11);
  int32_t start_time_us=sl_cli_get_argument_int32(arguments,12);
  uint32_t flags=sl_cli_get_argument_uint32(arguments,13);
  //return values
  uint8_t analyzer;
  status=sl_bt_connection_analyzer_start(
  access_address,
  crc_init,
  interval,
  supervision_timeout,
  central_clock_accuracy,
  central_phy,
  peripheral_phy,
  channel_selection_algorithm,
  hop,
  &channel_map,
  channel,
  event_counter,
  start_time_us,
  flags,
  &analyzer
  );

    printf("rsp_connection_analyzer_start 0x%lx ",status);
    printf("0x%x ",analyzer);
    printf("\n");
}
void sli_bt_cli_connection_analyzer_stop(sl_cli_command_arg_t *arguments)
{

  sl_status_t status;
  (void)(arguments);
  // parameters
  uint8_t analyzer=sl_cli_get_argument_uint8(arguments,0);
  //return values
  status=sl_bt_connection_analyzer_stop(
  analyzer
  );

    printf("rsp_connection_analyzer_stop 0x%lx ",status);
    printf("\n");
}


void sl_bt_cli_on_event(sl_bt_msg_t* evt)
{
 switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      printf("sl_bt_evt_system_boot ");
      printf("0x%x ",evt->data.evt_system_boot.major);
      printf("0x%x ",evt->data.evt_system_boot.minor);
      printf("0x%x ",evt->data.evt_system_boot.patch);
      printf("0x%x ",evt->data.evt_system_boot.build);
      printf("0x%x ",evt->data.evt_system_boot.bootloader);
      printf("0x%x ",evt->data.evt_system_boot.hw);
      printf("0x%x ",evt->data.evt_system_boot.hash);
      printf("\n");
    break;
    case sl_bt_evt_system_stopped_id:
      printf("sl_bt_evt_system_stopped ");
      printf("\n");
    break;
    case sl_bt_evt_system_error_id:
      printf("sl_bt_evt_system_error ");
      printf("0x%x ",evt->data.evt_system_error.reason);
      print_hex(evt->data.evt_system_error.data.data,evt->data.evt_system_error.data.len);
      printf("\n");
    break;
    case sl_bt_evt_system_resource_exhausted_id:
      printf("sl_bt_evt_system_resource_exhausted ");
      printf("0x%x ",evt->data.evt_system_resource_exhausted.num_buffers_discarded);
      printf("0x%x ",evt->data.evt_system_resource_exhausted.num_buffer_allocation_failures);
      printf("0x%x ",evt->data.evt_system_resource_exhausted.num_heap_allocation_failures);
      printf("0x%x ",evt->data.evt_system_resource_exhausted.num_message_allocation_failures);
      printf("\n");
    break;
    case sl_bt_evt_system_external_signal_id:
      printf("sl_bt_evt_system_external_signal ");
      printf("0x%x ",evt->data.evt_system_external_signal.extsignals);
      printf("\n");
    break;
    case sl_bt_evt_system_awake_id:
      printf("sl_bt_evt_system_awake ");
      printf("\n");
    break;
    case sl_bt_evt_system_soft_timer_id:
      printf("sl_bt_evt_system_soft_timer ");
      printf("0x%x ",evt->data.evt_system_soft_timer.handle);
      printf("\n");
    break;
    case sl_bt_evt_linklayer_event_info_report_id:
      printf("sl_bt_evt_linklayer_event_info_report ");
      printf("0x%x ",evt->data.evt_linklayer_event_info_report.configuration);
      printf("0x%x ",evt->data.evt_linklayer_event_info_report.procedure_type);
      print_hex(evt->data.evt_linklayer_event_info_report.data.data,evt->data.evt_linklayer_event_info_report.data.len);
      printf("\n");
    break;
    case sl_bt_evt_resource_status_id:
      printf("sl_bt_evt_resource_status ");
      printf("0x%x ",evt->data.evt_resource_status.free_bytes);
      printf("\n");
    break;
    case sl_bt_evt_advertiser_timeout_id:
      printf("sl_bt_evt_advertiser_timeout ");
      printf("0x%x ",evt->data.evt_advertiser_timeout.handle);
      printf("\n");
    break;
    case sl_bt_evt_advertiser_scan_request_id:
      printf("sl_bt_evt_advertiser_scan_request ");
      printf("0x%x ",evt->data.evt_advertiser_scan_request.handle);
      print_hex(evt->data.evt_advertiser_scan_request.address.addr,sizeof(evt->data.evt_advertiser_scan_request.address.addr));
      printf("0x%x ",evt->data.evt_advertiser_scan_request.address_type);
      printf("0x%x ",evt->data.evt_advertiser_scan_request.bonding);
      printf("\n");
    break;
    case sl_bt_evt_periodic_advertiser_status_id:
      printf("sl_bt_evt_periodic_advertiser_status ");
      printf("0x%x ",evt->data.evt_periodic_advertiser_status.advertising_set);
      printf("0x%x ",evt->data.evt_periodic_advertiser_status.status);
      printf("0x%x ",evt->data.evt_periodic_advertiser_status.event_counter);
      printf("\n");
    break;
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      printf("sl_bt_evt_scanner_legacy_advertisement_report ");
      printf("0x%x ",evt->data.evt_scanner_legacy_advertisement_report.event_flags);
      print_hex(evt->data.evt_scanner_legacy_advertisement_report.address.addr,sizeof(evt->data.evt_scanner_legacy_advertisement_report.address.addr));
      printf("0x%x ",evt->data.evt_scanner_legacy_advertisement_report.address_type);
      printf("0x%x ",evt->data.evt_scanner_legacy_advertisement_report.bonding);
      printf("%d ",evt->data.evt_scanner_legacy_advertisement_report.rssi);
      printf("0x%x ",evt->data.evt_scanner_legacy_advertisement_report.channel);
      print_hex(evt->data.evt_scanner_legacy_advertisement_report.target_address.addr,sizeof(evt->data.evt_scanner_legacy_advertisement_report.target_address.addr));
      printf("0x%x ",evt->data.evt_scanner_legacy_advertisement_report.target_address_type);
      print_hex(evt->data.evt_scanner_legacy_advertisement_report.data.data,evt->data.evt_scanner_legacy_advertisement_report.data.len);
      printf("\n");
    break;
    case sl_bt_evt_scanner_extended_advertisement_report_id:
      printf("sl_bt_evt_scanner_extended_advertisement_report ");
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.event_flags);
      print_hex(evt->data.evt_scanner_extended_advertisement_report.address.addr,sizeof(evt->data.evt_scanner_extended_advertisement_report.address.addr));
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.address_type);
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.bonding);
      printf("%d ",evt->data.evt_scanner_extended_advertisement_report.rssi);
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.channel);
      print_hex(evt->data.evt_scanner_extended_advertisement_report.target_address.addr,sizeof(evt->data.evt_scanner_extended_advertisement_report.target_address.addr));
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.target_address_type);
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.adv_sid);
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.primary_phy);
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.secondary_phy);
      printf("%d ",evt->data.evt_scanner_extended_advertisement_report.tx_power);
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.periodic_interval);
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.data_completeness);
      printf("0x%x ",evt->data.evt_scanner_extended_advertisement_report.counter);
      print_hex(evt->data.evt_scanner_extended_advertisement_report.data.data,evt->data.evt_scanner_extended_advertisement_report.data.len);
      printf("\n");
    break;
    case sl_bt_evt_sync_closed_id:
      printf("sl_bt_evt_sync_closed ");
      printf("0x%x ",evt->data.evt_sync_closed.reason);
      printf("0x%x ",evt->data.evt_sync_closed.sync);
      printf("\n");
    break;
    case sl_bt_evt_periodic_sync_opened_id:
      printf("sl_bt_evt_periodic_sync_opened ");
      printf("0x%x ",evt->data.evt_periodic_sync_opened.sync);
      printf("0x%x ",evt->data.evt_periodic_sync_opened.adv_sid);
      print_hex(evt->data.evt_periodic_sync_opened.address.addr,sizeof(evt->data.evt_periodic_sync_opened.address.addr));
      printf("0x%x ",evt->data.evt_periodic_sync_opened.address_type);
      printf("0x%x ",evt->data.evt_periodic_sync_opened.adv_phy);
      printf("0x%x ",evt->data.evt_periodic_sync_opened.adv_interval);
      printf("0x%x ",evt->data.evt_periodic_sync_opened.clock_accuracy);
      printf("0x%x ",evt->data.evt_periodic_sync_opened.bonding);
      printf("\n");
    break;
    case sl_bt_evt_periodic_sync_transfer_received_id:
      printf("sl_bt_evt_periodic_sync_transfer_received ");
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.status);
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.sync);
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.service_data);
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.connection);
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.adv_sid);
      print_hex(evt->data.evt_periodic_sync_transfer_received.address.addr,sizeof(evt->data.evt_periodic_sync_transfer_received.address.addr));
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.address_type);
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.adv_phy);
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.adv_interval);
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.clock_accuracy);
      printf("0x%x ",evt->data.evt_periodic_sync_transfer_received.bonding);
      printf("\n");
    break;
    case sl_bt_evt_periodic_sync_report_id:
      printf("sl_bt_evt_periodic_sync_report ");
      printf("0x%x ",evt->data.evt_periodic_sync_report.sync);
      printf("%d ",evt->data.evt_periodic_sync_report.tx_power);
      printf("%d ",evt->data.evt_periodic_sync_report.rssi);
      printf("0x%x ",evt->data.evt_periodic_sync_report.cte_type);
      printf("0x%x ",evt->data.evt_periodic_sync_report.data_status);
      printf("0x%x ",evt->data.evt_periodic_sync_report.counter);
      print_hex(evt->data.evt_periodic_sync_report.data.data,evt->data.evt_periodic_sync_report.data.len);
      printf("\n");
    break;
    case sl_bt_evt_pawr_sync_opened_id:
      printf("sl_bt_evt_pawr_sync_opened ");
      printf("0x%x ",evt->data.evt_pawr_sync_opened.sync);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.adv_sid);
      print_hex(evt->data.evt_pawr_sync_opened.address.addr,sizeof(evt->data.evt_pawr_sync_opened.address.addr));
      printf("0x%x ",evt->data.evt_pawr_sync_opened.address_type);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.adv_phy);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.adv_interval);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.clock_accuracy);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.num_subevents);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.subevent_interval);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.response_slot_delay);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.response_slot_spacing);
      printf("0x%x ",evt->data.evt_pawr_sync_opened.bonding);
      printf("\n");
    break;
    case sl_bt_evt_pawr_sync_transfer_received_id:
      printf("sl_bt_evt_pawr_sync_transfer_received ");
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.status);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.sync);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.service_data);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.connection);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.adv_sid);
      print_hex(evt->data.evt_pawr_sync_transfer_received.address.addr,sizeof(evt->data.evt_pawr_sync_transfer_received.address.addr));
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.address_type);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.adv_phy);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.adv_interval);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.clock_accuracy);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.num_subevents);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.subevent_interval);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.response_slot_delay);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.response_slot_spacing);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.bonding);
      printf("0x%x ",evt->data.evt_pawr_sync_transfer_received.receiving_sync);
      printf("\n");
    break;
    case sl_bt_evt_pawr_sync_subevent_report_id:
      printf("sl_bt_evt_pawr_sync_subevent_report ");
      printf("0x%x ",evt->data.evt_pawr_sync_subevent_report.sync);
      printf("%d ",evt->data.evt_pawr_sync_subevent_report.tx_power);
      printf("%d ",evt->data.evt_pawr_sync_subevent_report.rssi);
      printf("0x%x ",evt->data.evt_pawr_sync_subevent_report.cte_type);
      printf("0x%x ",evt->data.evt_pawr_sync_subevent_report.event_counter);
      printf("0x%x ",evt->data.evt_pawr_sync_subevent_report.subevent);
      printf("0x%x ",evt->data.evt_pawr_sync_subevent_report.data_status);
      printf("0x%x ",evt->data.evt_pawr_sync_subevent_report.counter);
      print_hex(evt->data.evt_pawr_sync_subevent_report.data.data,evt->data.evt_pawr_sync_subevent_report.data.len);
      printf("\n");
    break;
    case sl_bt_evt_pawr_advertiser_subevent_data_request_id:
      printf("sl_bt_evt_pawr_advertiser_subevent_data_request ");
      printf("0x%x ",evt->data.evt_pawr_advertiser_subevent_data_request.advertising_set);
      printf("0x%x ",evt->data.evt_pawr_advertiser_subevent_data_request.subevent_start);
      printf("0x%x ",evt->data.evt_pawr_advertiser_subevent_data_request.subevent_data_count);
      printf("\n");
    break;
    case sl_bt_evt_pawr_advertiser_subevent_tx_failed_id:
      printf("sl_bt_evt_pawr_advertiser_subevent_tx_failed ");
      printf("0x%x ",evt->data.evt_pawr_advertiser_subevent_tx_failed.advertising_set);
      printf("0x%x ",evt->data.evt_pawr_advertiser_subevent_tx_failed.subevent);
      printf("\n");
    break;
    case sl_bt_evt_pawr_advertiser_response_report_id:
      printf("sl_bt_evt_pawr_advertiser_response_report ");
      printf("0x%x ",evt->data.evt_pawr_advertiser_response_report.advertising_set);
      printf("0x%x ",evt->data.evt_pawr_advertiser_response_report.subevent);
      printf("%d ",evt->data.evt_pawr_advertiser_response_report.tx_power);
      printf("%d ",evt->data.evt_pawr_advertiser_response_report.rssi);
      printf("0x%x ",evt->data.evt_pawr_advertiser_response_report.cte_type);
      printf("0x%x ",evt->data.evt_pawr_advertiser_response_report.response_slot);
      printf("0x%x ",evt->data.evt_pawr_advertiser_response_report.data_status);
      printf("0x%x ",evt->data.evt_pawr_advertiser_response_report.counter);
      print_hex(evt->data.evt_pawr_advertiser_response_report.data.data,evt->data.evt_pawr_advertiser_response_report.data.len);
      printf("\n");
    break;
    case sl_bt_evt_connection_opened_id:
      printf("sl_bt_evt_connection_opened ");
      print_hex(evt->data.evt_connection_opened.address.addr,sizeof(evt->data.evt_connection_opened.address.addr));
      printf("0x%x ",evt->data.evt_connection_opened.address_type);
      printf("0x%x ",evt->data.evt_connection_opened.role);
      printf("0x%x ",evt->data.evt_connection_opened.connection);
      printf("0x%x ",evt->data.evt_connection_opened.bonding);
      printf("0x%x ",evt->data.evt_connection_opened.advertiser);
      printf("0x%x ",evt->data.evt_connection_opened.sync);
      printf("\n");
    break;
    case sl_bt_evt_connection_parameters_id:
      printf("sl_bt_evt_connection_parameters ");
      printf("0x%x ",evt->data.evt_connection_parameters.connection);
      printf("0x%x ",evt->data.evt_connection_parameters.interval);
      printf("0x%x ",evt->data.evt_connection_parameters.latency);
      printf("0x%x ",evt->data.evt_connection_parameters.timeout);
      printf("0x%x ",evt->data.evt_connection_parameters.security_mode);
      printf("\n");
    break;
    case sl_bt_evt_connection_set_parameters_failed_id:
      printf("sl_bt_evt_connection_set_parameters_failed ");
      printf("0x%x ",evt->data.evt_connection_set_parameters_failed.connection);
      printf("\n");
    break;
    case sl_bt_evt_connection_phy_status_id:
      printf("sl_bt_evt_connection_phy_status ");
      printf("0x%x ",evt->data.evt_connection_phy_status.connection);
      printf("0x%x ",evt->data.evt_connection_phy_status.phy);
      printf("\n");
    break;
    case sl_bt_evt_connection_get_remote_tx_power_completed_id:
      printf("sl_bt_evt_connection_get_remote_tx_power_completed ");
      printf("0x%x ",evt->data.evt_connection_get_remote_tx_power_completed.status);
      printf("0x%x ",evt->data.evt_connection_get_remote_tx_power_completed.connection);
      printf("0x%x ",evt->data.evt_connection_get_remote_tx_power_completed.phy);
      printf("%d ",evt->data.evt_connection_get_remote_tx_power_completed.power_level);
      printf("0x%x ",evt->data.evt_connection_get_remote_tx_power_completed.flags);
      printf("%d ",evt->data.evt_connection_get_remote_tx_power_completed.delta);
      printf("\n");
    break;
    case sl_bt_evt_connection_tx_power_id:
      printf("sl_bt_evt_connection_tx_power ");
      printf("0x%x ",evt->data.evt_connection_tx_power.connection);
      printf("0x%x ",evt->data.evt_connection_tx_power.phy);
      printf("%d ",evt->data.evt_connection_tx_power.power_level);
      printf("0x%x ",evt->data.evt_connection_tx_power.flags);
      printf("%d ",evt->data.evt_connection_tx_power.delta);
      printf("\n");
    break;
    case sl_bt_evt_connection_remote_tx_power_id:
      printf("sl_bt_evt_connection_remote_tx_power ");
      printf("0x%x ",evt->data.evt_connection_remote_tx_power.connection);
      printf("0x%x ",evt->data.evt_connection_remote_tx_power.phy);
      printf("%d ",evt->data.evt_connection_remote_tx_power.power_level);
      printf("0x%x ",evt->data.evt_connection_remote_tx_power.flags);
      printf("%d ",evt->data.evt_connection_remote_tx_power.delta);
      printf("\n");
    break;
    case sl_bt_evt_connection_remote_used_features_id:
      printf("sl_bt_evt_connection_remote_used_features ");
      printf("0x%x ",evt->data.evt_connection_remote_used_features.connection);
      print_hex(evt->data.evt_connection_remote_used_features.features.data,evt->data.evt_connection_remote_used_features.features.len);
      printf("\n");
    break;
    case sl_bt_evt_connection_data_length_id:
      printf("sl_bt_evt_connection_data_length ");
      printf("0x%x ",evt->data.evt_connection_data_length.connection);
      printf("0x%x ",evt->data.evt_connection_data_length.tx_data_len);
      printf("0x%x ",evt->data.evt_connection_data_length.tx_time_us);
      printf("0x%x ",evt->data.evt_connection_data_length.rx_data_len);
      printf("0x%x ",evt->data.evt_connection_data_length.rx_time_us);
      printf("\n");
    break;
    case sl_bt_evt_connection_statistics_id:
      printf("sl_bt_evt_connection_statistics ");
      printf("0x%x ",evt->data.evt_connection_statistics.connection);
      printf("%d ",evt->data.evt_connection_statistics.rssi_min);
      printf("%d ",evt->data.evt_connection_statistics.rssi_max);
      printf("0x%x ",evt->data.evt_connection_statistics.num_total_connection_events);
      printf("0x%x ",evt->data.evt_connection_statistics.num_missed_connection_events);
      printf("0x%x ",evt->data.evt_connection_statistics.num_successful_connection_events);
      printf("0x%x ",evt->data.evt_connection_statistics.num_crc_errors);
      printf("\n");
    break;
    case sl_bt_evt_connection_request_subrate_failed_id:
      printf("sl_bt_evt_connection_request_subrate_failed ");
      printf("0x%x ",evt->data.evt_connection_request_subrate_failed.connection);
      printf("0x%x ",evt->data.evt_connection_request_subrate_failed.result);
      printf("\n");
    break;
    case sl_bt_evt_connection_subrate_changed_id:
      printf("sl_bt_evt_connection_subrate_changed ");
      printf("0x%x ",evt->data.evt_connection_subrate_changed.connection);
      printf("0x%x ",evt->data.evt_connection_subrate_changed.subrate_factor);
      printf("0x%x ",evt->data.evt_connection_subrate_changed.latency);
      printf("0x%x ",evt->data.evt_connection_subrate_changed.continuation_number);
      printf("0x%x ",evt->data.evt_connection_subrate_changed.timeout);
      printf("\n");
    break;
    case sl_bt_evt_connection_channel_classification_id:
      printf("sl_bt_evt_connection_channel_classification ");
      printf("0x%x ",evt->data.evt_connection_channel_classification.connection);
      print_hex(evt->data.evt_connection_channel_classification.classification_map.data,sizeof(evt->data.evt_connection_channel_classification.classification_map.data));
      printf("\n");
    break;
    case sl_bt_evt_connection_closed_id:
      printf("sl_bt_evt_connection_closed ");
      printf("0x%x ",evt->data.evt_connection_closed.reason);
      printf("0x%x ",evt->data.evt_connection_closed.connection);
      printf("\n");
    break;
    case sl_bt_evt_gatt_mtu_exchanged_id:
      printf("sl_bt_evt_gatt_mtu_exchanged ");
      printf("0x%x ",evt->data.evt_gatt_mtu_exchanged.connection);
      printf("0x%x ",evt->data.evt_gatt_mtu_exchanged.mtu);
      printf("\n");
    break;
    case sl_bt_evt_gatt_service_id:
      printf("sl_bt_evt_gatt_service ");
      printf("0x%x ",evt->data.evt_gatt_service.connection);
      printf("0x%x ",evt->data.evt_gatt_service.service);
      print_hex(evt->data.evt_gatt_service.uuid.data,evt->data.evt_gatt_service.uuid.len);
      printf("\n");
    break;
    case sl_bt_evt_gatt_characteristic_id:
      printf("sl_bt_evt_gatt_characteristic ");
      printf("0x%x ",evt->data.evt_gatt_characteristic.connection);
      printf("0x%x ",evt->data.evt_gatt_characteristic.characteristic);
      printf("0x%x ",evt->data.evt_gatt_characteristic.properties);
      print_hex(evt->data.evt_gatt_characteristic.uuid.data,evt->data.evt_gatt_characteristic.uuid.len);
      printf("\n");
    break;
    case sl_bt_evt_gatt_descriptor_id:
      printf("sl_bt_evt_gatt_descriptor ");
      printf("0x%x ",evt->data.evt_gatt_descriptor.connection);
      printf("0x%x ",evt->data.evt_gatt_descriptor.descriptor);
      print_hex(evt->data.evt_gatt_descriptor.uuid.data,evt->data.evt_gatt_descriptor.uuid.len);
      printf("\n");
    break;
    case sl_bt_evt_gatt_characteristic_value_id:
      printf("sl_bt_evt_gatt_characteristic_value ");
      printf("0x%x ",evt->data.evt_gatt_characteristic_value.connection);
      printf("0x%x ",evt->data.evt_gatt_characteristic_value.characteristic);
      printf("0x%x ",evt->data.evt_gatt_characteristic_value.att_opcode);
      printf("0x%x ",evt->data.evt_gatt_characteristic_value.offset);
      print_hex(evt->data.evt_gatt_characteristic_value.value.data,evt->data.evt_gatt_characteristic_value.value.len);
      printf("\n");
    break;
    case sl_bt_evt_gatt_descriptor_value_id:
      printf("sl_bt_evt_gatt_descriptor_value ");
      printf("0x%x ",evt->data.evt_gatt_descriptor_value.connection);
      printf("0x%x ",evt->data.evt_gatt_descriptor_value.descriptor);
      printf("0x%x ",evt->data.evt_gatt_descriptor_value.offset);
      print_hex(evt->data.evt_gatt_descriptor_value.value.data,evt->data.evt_gatt_descriptor_value.value.len);
      printf("\n");
    break;
    case sl_bt_evt_gatt_procedure_completed_id:
      printf("sl_bt_evt_gatt_procedure_completed ");
      printf("0x%x ",evt->data.evt_gatt_procedure_completed.connection);
      printf("0x%x ",evt->data.evt_gatt_procedure_completed.result);
      printf("\n");
    break;
    case sl_bt_evt_gatt_server_attribute_value_id:
      printf("sl_bt_evt_gatt_server_attribute_value ");
      printf("0x%x ",evt->data.evt_gatt_server_attribute_value.connection);
      printf("0x%x ",evt->data.evt_gatt_server_attribute_value.attribute);
      printf("0x%x ",evt->data.evt_gatt_server_attribute_value.att_opcode);
      printf("0x%x ",evt->data.evt_gatt_server_attribute_value.offset);
      print_hex(evt->data.evt_gatt_server_attribute_value.value.data,evt->data.evt_gatt_server_attribute_value.value.len);
      printf("\n");
    break;
    case sl_bt_evt_gatt_server_user_read_request_id:
      printf("sl_bt_evt_gatt_server_user_read_request ");
      printf("0x%x ",evt->data.evt_gatt_server_user_read_request.connection);
      printf("0x%x ",evt->data.evt_gatt_server_user_read_request.characteristic);
      printf("0x%x ",evt->data.evt_gatt_server_user_read_request.att_opcode);
      printf("0x%x ",evt->data.evt_gatt_server_user_read_request.offset);
      printf("\n");
    break;
    case sl_bt_evt_gatt_server_user_write_request_id:
      printf("sl_bt_evt_gatt_server_user_write_request ");
      printf("0x%x ",evt->data.evt_gatt_server_user_write_request.connection);
      printf("0x%x ",evt->data.evt_gatt_server_user_write_request.characteristic);
      printf("0x%x ",evt->data.evt_gatt_server_user_write_request.att_opcode);
      printf("0x%x ",evt->data.evt_gatt_server_user_write_request.offset);
      print_hex(evt->data.evt_gatt_server_user_write_request.value.data,evt->data.evt_gatt_server_user_write_request.value.len);
      printf("\n");
    break;
    case sl_bt_evt_gatt_server_characteristic_status_id:
      printf("sl_bt_evt_gatt_server_characteristic_status ");
      printf("0x%x ",evt->data.evt_gatt_server_characteristic_status.connection);
      printf("0x%x ",evt->data.evt_gatt_server_characteristic_status.characteristic);
      printf("0x%x ",evt->data.evt_gatt_server_characteristic_status.status_flags);
      printf("0x%x ",evt->data.evt_gatt_server_characteristic_status.client_config_flags);
      printf("0x%x ",evt->data.evt_gatt_server_characteristic_status.client_config);
      printf("\n");
    break;
    case sl_bt_evt_gatt_server_execute_write_completed_id:
      printf("sl_bt_evt_gatt_server_execute_write_completed ");
      printf("0x%x ",evt->data.evt_gatt_server_execute_write_completed.connection);
      printf("0x%x ",evt->data.evt_gatt_server_execute_write_completed.result);
      printf("\n");
    break;
    case sl_bt_evt_gatt_server_indication_timeout_id:
      printf("sl_bt_evt_gatt_server_indication_timeout ");
      printf("0x%x ",evt->data.evt_gatt_server_indication_timeout.connection);
      printf("\n");
    break;
    case sl_bt_evt_gatt_server_notification_tx_completed_id:
      printf("sl_bt_evt_gatt_server_notification_tx_completed ");
      printf("0x%x ",evt->data.evt_gatt_server_notification_tx_completed.connection);
      printf("0x%x ",evt->data.evt_gatt_server_notification_tx_completed.count);
      printf("\n");
    break;
    case sl_bt_evt_test_dtm_completed_id:
      printf("sl_bt_evt_test_dtm_completed ");
      printf("0x%x ",evt->data.evt_test_dtm_completed.result);
      printf("0x%x ",evt->data.evt_test_dtm_completed.number_of_packets);
      printf("\n");
    break;
    case sl_bt_evt_sm_passkey_display_id:
      printf("sl_bt_evt_sm_passkey_display ");
      printf("0x%x ",evt->data.evt_sm_passkey_display.connection);
      printf("0x%x ",evt->data.evt_sm_passkey_display.passkey);
      printf("\n");
    break;
    case sl_bt_evt_sm_passkey_request_id:
      printf("sl_bt_evt_sm_passkey_request ");
      printf("0x%x ",evt->data.evt_sm_passkey_request.connection);
      printf("\n");
    break;
    case sl_bt_evt_sm_confirm_passkey_id:
      printf("sl_bt_evt_sm_confirm_passkey ");
      printf("0x%x ",evt->data.evt_sm_confirm_passkey.connection);
      printf("0x%x ",evt->data.evt_sm_confirm_passkey.passkey);
      printf("\n");
    break;
    case sl_bt_evt_sm_bonded_id:
      printf("sl_bt_evt_sm_bonded ");
      printf("0x%x ",evt->data.evt_sm_bonded.connection);
      printf("0x%x ",evt->data.evt_sm_bonded.bonding);
      printf("0x%x ",evt->data.evt_sm_bonded.security_mode);
      printf("\n");
    break;
    case sl_bt_evt_sm_bonding_failed_id:
      printf("sl_bt_evt_sm_bonding_failed ");
      printf("0x%x ",evt->data.evt_sm_bonding_failed.connection);
      printf("0x%x ",evt->data.evt_sm_bonding_failed.reason);
      printf("\n");
    break;
    case sl_bt_evt_sm_confirm_bonding_id:
      printf("sl_bt_evt_sm_confirm_bonding ");
      printf("0x%x ",evt->data.evt_sm_confirm_bonding.connection);
      printf("0x%x ",evt->data.evt_sm_confirm_bonding.bonding_handle);
      printf("\n");
    break;
    case sl_bt_evt_external_bondingdb_data_request_id:
      printf("sl_bt_evt_external_bondingdb_data_request ");
      printf("0x%x ",evt->data.evt_external_bondingdb_data_request.connection);
      printf("0x%x ",evt->data.evt_external_bondingdb_data_request.type);
      printf("\n");
    break;
    case sl_bt_evt_external_bondingdb_data_id:
      printf("sl_bt_evt_external_bondingdb_data ");
      printf("0x%x ",evt->data.evt_external_bondingdb_data.connection);
      printf("0x%x ",evt->data.evt_external_bondingdb_data.type);
      print_hex(evt->data.evt_external_bondingdb_data.data.data,evt->data.evt_external_bondingdb_data.data.len);
      printf("\n");
    break;
    case sl_bt_evt_external_bondingdb_data_ready_id:
      printf("sl_bt_evt_external_bondingdb_data_ready ");
      printf("0x%x ",evt->data.evt_external_bondingdb_data_ready.connection);
      printf("\n");
    break;
    case sl_bt_evt_external_bondingdb_local_irk_request_id:
      printf("sl_bt_evt_external_bondingdb_local_irk_request ");
      printf("\n");
    break;
    case sl_bt_evt_external_bondingdb_local_irk_id:
      printf("sl_bt_evt_external_bondingdb_local_irk ");
      print_hex(evt->data.evt_external_bondingdb_local_irk.data.data,evt->data.evt_external_bondingdb_local_irk.data.len);
      printf("\n");
    break;
    case sl_bt_evt_cs_security_enable_complete_id:
      printf("sl_bt_evt_cs_security_enable_complete ");
      printf("0x%x ",evt->data.evt_cs_security_enable_complete.connection);
      printf("\n");
    break;
    case sl_bt_evt_cs_config_complete_id:
      printf("sl_bt_evt_cs_config_complete ");
      printf("0x%x ",evt->data.evt_cs_config_complete.connection);
      printf("0x%x ",evt->data.evt_cs_config_complete.config_id);
      printf("0x%x ",evt->data.evt_cs_config_complete.status);
      printf("0x%x ",evt->data.evt_cs_config_complete.config_state);
      printf("0x%x ",evt->data.evt_cs_config_complete.main_mode_type);
      printf("0x%x ",evt->data.evt_cs_config_complete.sub_mode_type);
      printf("0x%x ",evt->data.evt_cs_config_complete.min_main_mode_steps);
      printf("0x%x ",evt->data.evt_cs_config_complete.max_main_mode_steps);
      printf("0x%x ",evt->data.evt_cs_config_complete.main_mode_repetition);
      printf("0x%x ",evt->data.evt_cs_config_complete.mode_calibration_steps);
      printf("0x%x ",evt->data.evt_cs_config_complete.role);
      printf("0x%x ",evt->data.evt_cs_config_complete.rtt_type);
      printf("0x%x ",evt->data.evt_cs_config_complete.cs_sync_phy);
      print_hex(evt->data.evt_cs_config_complete.channel_map.data,sizeof(evt->data.evt_cs_config_complete.channel_map.data));
      printf("0x%x ",evt->data.evt_cs_config_complete.channel_map_repetition);
      printf("0x%x ",evt->data.evt_cs_config_complete.channel_selection_type);
      printf("0x%x ",evt->data.evt_cs_config_complete.ch3c_shape);
      printf("0x%x ",evt->data.evt_cs_config_complete.ch3c_jump);
      printf("0x%x ",evt->data.evt_cs_config_complete.reserved);
      printf("0x%x ",evt->data.evt_cs_config_complete.t_ip1_time);
      printf("0x%x ",evt->data.evt_cs_config_complete.t_ip2_time);
      printf("0x%x ",evt->data.evt_cs_config_complete.t_fcs_time);
      printf("0x%x ",evt->data.evt_cs_config_complete.t_pm_time);
      printf("\n");
    break;
    case sl_bt_evt_cs_procedure_enable_complete_id:
      printf("sl_bt_evt_cs_procedure_enable_complete ");
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.connection);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.config_id);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.status);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.state);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.antenna_config);
      printf("%d ",evt->data.evt_cs_procedure_enable_complete.tx_power);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.subevent_len);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.subevents_per_event);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.subevent_interval);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.event_interval);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.procedure_interval);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.procedure_count);
      printf("0x%x ",evt->data.evt_cs_procedure_enable_complete.max_procedure_len);
      printf("\n");
    break;
    case sl_bt_evt_cs_result_id:
      printf("sl_bt_evt_cs_result ");
      printf("0x%x ",evt->data.evt_cs_result.connection);
      printf("0x%x ",evt->data.evt_cs_result.config_id);
      printf("0x%x ",evt->data.evt_cs_result.start_acl_conn_event);
      printf("0x%x ",evt->data.evt_cs_result.procedure_counter);
      printf("%d ",evt->data.evt_cs_result.frequency_compensation);
      printf("%d ",evt->data.evt_cs_result.reference_power_level);
      printf("0x%x ",evt->data.evt_cs_result.procedure_done_status);
      printf("0x%x ",evt->data.evt_cs_result.subevent_done_status);
      printf("0x%x ",evt->data.evt_cs_result.abort_reason);
      printf("0x%x ",evt->data.evt_cs_result.num_antenna_paths);
      printf("0x%x ",evt->data.evt_cs_result.num_steps);
      print_hex(evt->data.evt_cs_result.data.data,evt->data.evt_cs_result.data.len);
      printf("\n");
    break;
    case sl_bt_evt_cs_result_continue_id:
      printf("sl_bt_evt_cs_result_continue ");
      printf("0x%x ",evt->data.evt_cs_result_continue.connection);
      printf("0x%x ",evt->data.evt_cs_result_continue.config_id);
      printf("0x%x ",evt->data.evt_cs_result_continue.procedure_done_status);
      printf("0x%x ",evt->data.evt_cs_result_continue.subevent_done_status);
      printf("0x%x ",evt->data.evt_cs_result_continue.abort_reason);
      printf("0x%x ",evt->data.evt_cs_result_continue.num_antenna_paths);
      printf("0x%x ",evt->data.evt_cs_result_continue.num_steps);
      print_hex(evt->data.evt_cs_result_continue.data.data,evt->data.evt_cs_result_continue.data.len);
      printf("\n");
    break;
    case sl_bt_evt_cs_read_remote_supported_capabilities_complete_id:
      printf("sl_bt_evt_cs_read_remote_supported_capabilities_complete ");
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.connection);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.status);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.num_config);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.max_consecutive_procedures);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.num_antennas);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.max_antenna_paths);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.roles);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.modes);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.rtt_capability);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.rtt_aa_only);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.rtt_sounding);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.rtt_random_payload);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.nadm_sounding_capability);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.nadm_random_capability);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.cs_sync_phys);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.subfeatures);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.t_ip1_times);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.t_ip2_times);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.t_fcs_times);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.t_pm_times);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.t_sw_times);
      printf("0x%x ",evt->data.evt_cs_read_remote_supported_capabilities_complete.tx_snr_capability);
      printf("\n");
    break;
    case sl_bt_evt_cs_test_end_completed_id:
      printf("sl_bt_evt_cs_test_end_completed ");
      printf("0x%x ",evt->data.evt_cs_test_end_completed.status);
      printf("\n");
    break;
    case sl_bt_evt_l2cap_le_channel_open_request_id:
      printf("sl_bt_evt_l2cap_le_channel_open_request ");
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_request.connection);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_request.spsm);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_request.cid);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_request.max_sdu);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_request.max_pdu);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_request.credit);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_request.remote_cid);
      printf("\n");
    break;
    case sl_bt_evt_l2cap_le_channel_open_response_id:
      printf("sl_bt_evt_l2cap_le_channel_open_response ");
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_response.connection);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_response.cid);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_response.max_sdu);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_response.max_pdu);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_response.credit);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_response.errorcode);
      printf("0x%x ",evt->data.evt_l2cap_le_channel_open_response.remote_cid);
      printf("\n");
    break;
    case sl_bt_evt_l2cap_channel_data_id:
      printf("sl_bt_evt_l2cap_channel_data ");
      printf("0x%x ",evt->data.evt_l2cap_channel_data.connection);
      printf("0x%x ",evt->data.evt_l2cap_channel_data.cid);
      print_hex(evt->data.evt_l2cap_channel_data.data.data,evt->data.evt_l2cap_channel_data.data.len);
      printf("\n");
    break;
    case sl_bt_evt_l2cap_channel_credit_id:
      printf("sl_bt_evt_l2cap_channel_credit ");
      printf("0x%x ",evt->data.evt_l2cap_channel_credit.connection);
      printf("0x%x ",evt->data.evt_l2cap_channel_credit.cid);
      printf("0x%x ",evt->data.evt_l2cap_channel_credit.credit);
      printf("\n");
    break;
    case sl_bt_evt_l2cap_channel_closed_id:
      printf("sl_bt_evt_l2cap_channel_closed ");
      printf("0x%x ",evt->data.evt_l2cap_channel_closed.connection);
      printf("0x%x ",evt->data.evt_l2cap_channel_closed.cid);
      printf("0x%x ",evt->data.evt_l2cap_channel_closed.reason);
      printf("\n");
    break;
    case sl_bt_evt_l2cap_command_rejected_id:
      printf("sl_bt_evt_l2cap_command_rejected ");
      printf("0x%x ",evt->data.evt_l2cap_command_rejected.connection);
      printf("0x%x ",evt->data.evt_l2cap_command_rejected.code);
      printf("0x%x ",evt->data.evt_l2cap_command_rejected.reason);
      printf("0x%x ",evt->data.evt_l2cap_command_rejected.cid);
      printf("\n");
    break;
    case sl_bt_evt_cte_receiver_dtm_iq_report_id:
      printf("sl_bt_evt_cte_receiver_dtm_iq_report ");
      printf("0x%x ",evt->data.evt_cte_receiver_dtm_iq_report.status);
      printf("0x%x ",evt->data.evt_cte_receiver_dtm_iq_report.channel);
      printf("%d ",evt->data.evt_cte_receiver_dtm_iq_report.rssi);
      printf("0x%x ",evt->data.evt_cte_receiver_dtm_iq_report.rssi_antenna_id);
      printf("0x%x ",evt->data.evt_cte_receiver_dtm_iq_report.cte_type);
      printf("0x%x ",evt->data.evt_cte_receiver_dtm_iq_report.slot_durations);
      printf("0x%x ",evt->data.evt_cte_receiver_dtm_iq_report.event_counter);
      print_hex(evt->data.evt_cte_receiver_dtm_iq_report.samples.data,evt->data.evt_cte_receiver_dtm_iq_report.samples.len);
      printf("\n");
    break;
    case sl_bt_evt_cte_receiver_connection_iq_report_id:
      printf("sl_bt_evt_cte_receiver_connection_iq_report ");
      printf("0x%x ",evt->data.evt_cte_receiver_connection_iq_report.status);
      printf("0x%x ",evt->data.evt_cte_receiver_connection_iq_report.connection);
      printf("0x%x ",evt->data.evt_cte_receiver_connection_iq_report.phy);
      printf("0x%x ",evt->data.evt_cte_receiver_connection_iq_report.channel);
      printf("%d ",evt->data.evt_cte_receiver_connection_iq_report.rssi);
      printf("0x%x ",evt->data.evt_cte_receiver_connection_iq_report.rssi_antenna_id);
      printf("0x%x ",evt->data.evt_cte_receiver_connection_iq_report.cte_type);
      printf("0x%x ",evt->data.evt_cte_receiver_connection_iq_report.slot_durations);
      printf("0x%x ",evt->data.evt_cte_receiver_connection_iq_report.event_counter);
      print_hex(evt->data.evt_cte_receiver_connection_iq_report.samples.data,evt->data.evt_cte_receiver_connection_iq_report.samples.len);
      printf("\n");
    break;
    case sl_bt_evt_cte_receiver_connectionless_iq_report_id:
      printf("sl_bt_evt_cte_receiver_connectionless_iq_report ");
      printf("0x%x ",evt->data.evt_cte_receiver_connectionless_iq_report.status);
      printf("0x%x ",evt->data.evt_cte_receiver_connectionless_iq_report.sync);
      printf("0x%x ",evt->data.evt_cte_receiver_connectionless_iq_report.channel);
      printf("%d ",evt->data.evt_cte_receiver_connectionless_iq_report.rssi);
      printf("0x%x ",evt->data.evt_cte_receiver_connectionless_iq_report.rssi_antenna_id);
      printf("0x%x ",evt->data.evt_cte_receiver_connectionless_iq_report.cte_type);
      printf("0x%x ",evt->data.evt_cte_receiver_connectionless_iq_report.slot_durations);
      printf("0x%x ",evt->data.evt_cte_receiver_connectionless_iq_report.event_counter);
      print_hex(evt->data.evt_cte_receiver_connectionless_iq_report.samples.data,evt->data.evt_cte_receiver_connectionless_iq_report.samples.len);
      printf("\n");
    break;
    case sl_bt_evt_cte_receiver_silabs_iq_report_id:
      printf("sl_bt_evt_cte_receiver_silabs_iq_report ");
      printf("0x%x ",evt->data.evt_cte_receiver_silabs_iq_report.status);
      print_hex(evt->data.evt_cte_receiver_silabs_iq_report.address.addr,sizeof(evt->data.evt_cte_receiver_silabs_iq_report.address.addr));
      printf("0x%x ",evt->data.evt_cte_receiver_silabs_iq_report.address_type);
      printf("0x%x ",evt->data.evt_cte_receiver_silabs_iq_report.phy);
      printf("0x%x ",evt->data.evt_cte_receiver_silabs_iq_report.channel);
      printf("%d ",evt->data.evt_cte_receiver_silabs_iq_report.rssi);
      printf("0x%x ",evt->data.evt_cte_receiver_silabs_iq_report.rssi_antenna_id);
      printf("0x%x ",evt->data.evt_cte_receiver_silabs_iq_report.cte_type);
      printf("0x%x ",evt->data.evt_cte_receiver_silabs_iq_report.slot_durations);
      printf("0x%x ",evt->data.evt_cte_receiver_silabs_iq_report.packet_counter);
      print_hex(evt->data.evt_cte_receiver_silabs_iq_report.samples.data,evt->data.evt_cte_receiver_silabs_iq_report.samples.len);
      printf("\n");
    break;
    case sl_bt_evt_connection_analyzer_report_id:
      printf("sl_bt_evt_connection_analyzer_report ");
      printf("0x%x ",evt->data.evt_connection_analyzer_report.analyzer);
      printf("%d ",evt->data.evt_connection_analyzer_report.central_rssi);
      printf("%d ",evt->data.evt_connection_analyzer_report.peripheral_rssi);
      printf("\n");
    break;
    case sl_bt_evt_connection_analyzer_completed_id:
      printf("sl_bt_evt_connection_analyzer_completed ");
      printf("0x%x ",evt->data.evt_connection_analyzer_completed.analyzer);
      printf("0x%x ",evt->data.evt_connection_analyzer_completed.reason);
      printf("\n");
    break;

 }
}