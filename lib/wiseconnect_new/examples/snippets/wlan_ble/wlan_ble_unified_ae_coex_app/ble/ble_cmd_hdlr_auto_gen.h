#ifndef BLE_CMD_HDLR_AUTO_GEN_H
#define BLE_CMD_HDLR_AUTO_GEN_H
#include "event_loop.h"

#ifdef FRDM_K28F
#define APP_WEAK __attribute__((weak))
#else
#ifdef MXRT_595s
#define APP_WEAK __weak
#else
#define APP_WEAK __attribute__((weak))
#endif
#endif

#define FIXED_LEN_RESPONSE_BUFFER_VALID    1
#define VARIABLE_LEN_RESPONSE_BUFFER_VALID 2

//! Enumerations
enum ble_cmd_ids {
  req_adv_command_id,
  req_scan_command_id,
  req_conn_command_id,
  req_adv_stop_command_id,
  req_scan_stop_command_id,
  req_disconnect_command_id,
  get_dev_state_command_id,
  cmd_conn_params_update_command_id,
  req_start_encryption_command_id,
  req_smp_pair_command_id,
  smp_pair_response_command_id,
  smp_passkey_command_id,
  req_profiles_command_id,
  req_profile_command_id,
  req_char_services_command_id,
  req_inc_services_command_id,
  req_read_by_uuid_command_id,
  req_desc_command_id,
  req_read_val_command_id,
  req_multiple_read_command_id,
  req_long_read_command_id,
  req_write_command_id,
  req_write_no_ack_command_id,
  req_long_write_command_id,
  req_prepare_write_command_id,
  req_execute_write_command_id,
  add_service_command_id,
  add_attribute_command_id,
  set_local_att_value_command_id,
  get_local_att_value_command_id,
  cmd_notify_command_id,
  cmd_indicate_command_id,
  set_advertise_data_command_id,
  get_le_ping_command_id,
  set_le_ping_command_id,
  set_random_address_command_id,
  encrypt_command_id,
  cmd_read_resp_command_id,
  set_scan_response_data_command_id,
  le_accept_list_command_id,
  cmd_remove_service_command_id,
  cmd_remove_attribute_command_id,
  process_resolv_list_command_id,
  get_resolving_list_size_command_id,
  set_address_resolution_enable_command_id,
  req_read_phy_command_id,
  req_set_phy_command_id,
  set_data_len_command_id,
  read_max_data_len_command_id,
  set_privacy_mode_command_id,
  cbfc_conn_req_command_id,
  cbfc_conn_resp_command_id,
  cbfc_tx_data_command_id,
  cbfc_disconn_command_id,
  le_ltk_req_reply_command_id,
  rx_test_mode_command_id,
  tx_test_mode_command_id,
  end_test_mode_command_id,
  per_tx_mode_command_id,
  per_rx_mode_command_id,
  cmd_att_error_command_id,
  req_profiles_async_command_id,
  req_profile_async_command_id,
  get_charservices_async_command_id,
  get_includeservices_async_command_id,
  readcharvaluebyuuid_async_command_id,
  get_attribute_async_command_id,
  get_descriptorvalue_async_command_id,
  get_multiplevalues_async_command_id,
  get_longdescvalues_async_command_id,
  set_descvalue_async_command_id,
  set_preparewrite_async_command_id,
  execute_longdescwrite_async_command_id,
  set_smp_pairing_capability_data_command_id,
  conn_param_resp_cmd_command_id,
  cmd_indicate_confirmation_command_id,
  mtu_exchange_request_command_id,
  cmd_set_wwo_resp_notify_buf_info_command_id,
  cmd_write_resp_command_id,
  cmd_prepare_write_resp_command_id,
  cmd_set_local_irk_command_id,
  cmd_set_prop_protocol_ble_bandedge_txpower_command_id,
  cmd_mtu_exchange_resp_command_id,
  cmd_set_ble_tx_power_command_id,
  cmd_indicate_sync_command_id,
  prop_protocol_cmd_command_id,
  prop_protocol_cmd_per_command_id,
  max_ble_command_id
};

//! function declarations
void ui_task_req_adv_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_scan_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_conn_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_adv_stop_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_scan_stop_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_disconnect_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_dev_state_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_conn_params_update_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_start_encryption_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_smp_pair_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_smp_pair_response_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_smp_passkey_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_profiles_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_profile_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_char_services_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_inc_services_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_read_by_uuid_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_desc_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_read_val_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_multiple_read_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_long_read_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_write_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_write_no_ack_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_long_write_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_prepare_write_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_execute_write_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_add_service_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_add_attribute_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_local_att_value_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_local_att_value_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_notify_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_indicate_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_advertise_data_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_le_ping_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_le_ping_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_random_address_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_encrypt_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_read_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_scan_response_data_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_le_accept_list_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_remove_service_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_remove_attribute_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_process_resolv_list_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_resolving_list_size_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_address_resolution_enable_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_read_phy_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_set_phy_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_data_len_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_read_max_data_len_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_privacy_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cbfc_conn_req_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cbfc_conn_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cbfc_tx_data_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cbfc_disconn_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_le_ltk_req_reply_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_rx_test_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_tx_test_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_end_test_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_per_tx_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_per_rx_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_att_error_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_profiles_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_req_profile_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_charservices_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_includeservices_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_readcharvaluebyuuid_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_attribute_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_descriptorvalue_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_multiplevalues_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_get_longdescvalues_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_descvalue_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_preparewrite_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_execute_longdescwrite_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_set_smp_pairing_capability_data_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_conn_param_resp_cmd_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_indicate_confirmation_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_mtu_exchange_request_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_set_wwo_resp_notify_buf_info_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_write_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_prepare_write_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_set_local_irk_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_mtu_exchange_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_set_ble_tx_power_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_cmd_indicate_sync_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_prop_protocol_cmd_cmd_handler(uint8_t argc, uint8_t *argv[]);
void ui_task_prop_protocol_cmd_per_cmd_handler(uint8_t argc, uint8_t *argv[]);

void ble_module_req_adv_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_scan_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_conn_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_adv_stop_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_scan_stop_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_disconnect_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_dev_state_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_conn_params_update_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_start_encryption_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_smp_pair_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_smp_pair_response_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_smp_passkey_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_profiles_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_profile_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_char_services_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_inc_services_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_read_by_uuid_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_desc_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_read_val_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_multiple_read_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_long_read_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_write_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_write_no_ack_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_long_write_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_prepare_write_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_execute_write_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_add_service_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_add_attribute_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_local_att_value_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_local_att_value_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_notify_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_indicate_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_advertise_data_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_le_ping_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_le_ping_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_random_address_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_encrypt_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_read_resp_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_scan_response_data_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_le_accept_list_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_remove_service_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_remove_attribute_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_process_resolv_list_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_resolving_list_size_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_address_resolution_enable_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_read_phy_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_set_phy_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_data_len_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_read_max_data_len_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_privacy_mode_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cbfc_conn_req_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cbfc_conn_resp_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cbfc_tx_data_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cbfc_disconn_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_le_ltk_req_reply_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_rx_test_mode_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_tx_test_mode_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_end_test_mode_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_per_tx_mode_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_per_rx_mode_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_att_error_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_profiles_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_req_profile_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_charservices_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_includeservices_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_readcharvaluebyuuid_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_attribute_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_descriptorvalue_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_multiplevalues_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_get_longdescvalues_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_descvalue_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_preparewrite_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_execute_longdescwrite_async_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_set_smp_pairing_capability_data_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_conn_param_resp_cmd_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_indicate_confirmation_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_mtu_exchange_request_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_write_resp_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_prepare_write_resp_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_set_local_irk_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler(
  generic_command_message_t *generic_command_message);
void ble_module_cmd_mtu_exchange_resp_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_set_ble_tx_power_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_cmd_indicate_sync_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_prop_protocol_cmd_cmd_handler(generic_command_message_t *generic_command_message);
void ble_module_prop_protocol_cmd_per_cmd_handler(generic_command_message_t *generic_command_message);

/** Extern functions  **/
//! function declarations
extern void ui_task_req_adv_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_scan_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_conn_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_adv_stop_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_scan_stop_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_disconnect_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_dev_state_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_conn_params_update_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_start_encryption_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_smp_pair_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_smp_pair_response_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_smp_passkey_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_profiles_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_profile_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_char_services_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_inc_services_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_read_by_uuid_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_desc_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_read_val_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_multiple_read_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_long_read_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_write_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_write_no_ack_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_long_write_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_prepare_write_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_execute_write_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_add_service_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_add_attribute_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_local_att_value_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_local_att_value_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_notify_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_indicate_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_advertise_data_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_le_ping_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_le_ping_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_random_address_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_encrypt_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_read_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_scan_response_data_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_le_accept_list_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_remove_service_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_remove_attribute_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_process_resolv_list_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_resolving_list_size_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_address_resolution_enable_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_read_phy_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_set_phy_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_data_len_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_read_max_data_len_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_privacy_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cbfc_conn_req_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cbfc_conn_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cbfc_tx_data_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cbfc_disconn_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_le_ltk_req_reply_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_rx_test_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_tx_test_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_end_test_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_per_tx_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_per_rx_mode_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_att_error_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_profiles_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_req_profile_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_charservices_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_includeservices_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_readcharvaluebyuuid_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_attribute_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_descriptorvalue_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_multiplevalues_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_get_longdescvalues_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_descvalue_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_preparewrite_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_execute_longdescwrite_async_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_set_smp_pairing_capability_data_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_conn_param_resp_cmd_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_indicate_confirmation_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_mtu_exchange_request_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_set_wwo_resp_notify_buf_info_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_write_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_prepare_write_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_set_local_irk_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_mtu_exchange_resp_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_set_ble_tx_power_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_cmd_indicate_sync_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_prop_protocol_cmd_cmd_handler(uint8_t argc, uint8_t *argv[]);
extern void ui_task_prop_protocol_cmd_per_cmd_handler(uint8_t argc, uint8_t *argv[]);

extern void ble_module_req_adv_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_scan_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_conn_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_adv_stop_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_scan_stop_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_disconnect_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_dev_state_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_conn_params_update_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_start_encryption_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_smp_pair_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_smp_pair_response_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_smp_passkey_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_profiles_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_profile_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_char_services_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_inc_services_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_read_by_uuid_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_desc_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_read_val_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_multiple_read_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_long_read_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_write_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_write_no_ack_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_long_write_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_prepare_write_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_execute_write_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_add_service_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_add_attribute_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_local_att_value_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_local_att_value_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_notify_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_indicate_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_advertise_data_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_le_ping_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_le_ping_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_random_address_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_encrypt_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_read_resp_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_scan_response_data_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_le_accept_list_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_remove_service_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_remove_attribute_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_process_resolv_list_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_resolving_list_size_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_address_resolution_enable_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_read_phy_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_set_phy_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_data_len_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_read_max_data_len_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_privacy_mode_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cbfc_conn_req_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cbfc_conn_resp_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cbfc_tx_data_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cbfc_disconn_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_le_ltk_req_reply_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_rx_test_mode_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_tx_test_mode_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_end_test_mode_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_per_tx_mode_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_per_rx_mode_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_att_error_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_profiles_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_req_profile_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_charservices_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_includeservices_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_readcharvaluebyuuid_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_attribute_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_descriptorvalue_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_multiplevalues_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_get_longdescvalues_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_descvalue_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_preparewrite_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_execute_longdescwrite_async_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_set_smp_pairing_capability_data_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_conn_param_resp_cmd_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_indicate_confirmation_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_mtu_exchange_request_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_set_wwo_resp_notify_buf_info_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_write_resp_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_prepare_write_resp_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_set_local_irk_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_set_prop_protocol_ble_bandedge_txpower_cmd_handler(
  generic_command_message_t *generic_command_message);
extern void ble_module_cmd_mtu_exchange_resp_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_set_ble_tx_power_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_cmd_indicate_sync_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_prop_protocol_cmd_cmd_handler(generic_command_message_t *generic_command_message);
extern void ble_module_prop_protocol_cmd_per_cmd_handler(generic_command_message_t *generic_command_message);

/** Extern variables  **/
extern generic_task_cb_t ble_generic_cb;
extern osSemaphoreId_t ui_task_response_semaphore;
extern generic_command_handler_lut_entry_t ble_commands_hdlr_lut[];
#endif