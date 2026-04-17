#ifndef BLE_AUTO_GEN_H
#define BLE_AUTO_GEN_H
#include "event_loop.h"
#include "ble_private.h"

#ifdef FRDM_K28F
#define APP_WEAK __attribute__((weak))
#else
#ifdef MXRT_595s
#define APP_WEAK __weak
#else
#define APP_WEAK __attribute__((weak))
#endif
#endif

void sl_btc_ble_gap_register_callbacks_wrapper();
void sl_btc_ble_gatt_register_callbacks_wrapper();
void sl_btc_ble_gatt_extended_register_callbacks_wrapper();
void sl_btc_ble_gap_extended_register_callbacks_wrapper();
void sl_btc_ble_smp_register_callbacks_wrapper();
void sl_btc_ble_smp_extended_register_callbacks_wrapper();
void sl_btc_ble_adv_extended_register_callbacks_wrapper();

void rsi_ble_event_smp_pending_driver_callback(uint8_t *conn_id);

//! Enumerations
enum ble_enum_ids {
  event_adv_report_event_id,
  event_disconnect_event_id,
  event_conn_status_event_id,
  event_enhance_conn_status_event_id,
  event_mtu_exchange_information_event_id,
  event_mtu_event_id,
  event_le_dev_buf_ind_event_id,
  event_remote_features_event_id,
  event_data_length_update_event_id,
  event_phy_update_event_id,
  event_remote_conn_param_req_event_id,
  event_conn_update_event_id,
  read_req_event_id,
  event_error_resp_event_id,
  event_gatt_desc_event_id,
  event_read_by_type1_event_id,
  event_profile_by_uuid_event_id,
  event_profiles_list_event_id,
  event_write_event_id,
  event_prepare_write_event_id,
  execute_write_event_id,
  set_att_resp_event_id,
  prepare_write_resp_event_id,
  event_att_value_event_id,
  indication_confirmation_event_id,
  ae_adv_report_event_id,
  per_adv_sync_estbl_event_id,
  per_adv_report_event_id,
  per_adv_sync_lost_event_id,
  scan_timeout_event_id,
  adv_set_terminated_event_id,
  scan_req_recvd_event_id,
  event_smp_req_event_id,
  event_smp_resp_event_id,
  event_smp_passkey_event_id,
  event_smp_failed_event_id,
  event_encryption_enabled_event_id,
  event_smp_passkey_display_event_id,
  event_sc_passkey_event_id,
  event_le_ltk_request_event_id,
  event_le_security_keys_event_id,
  data_transmit_event_id,
  smp_pending_event_id,
  advertisement_restart_event_id,
  scan_restart_event_id,
  on_data_recieve_event_id,
  conn_update_req_event_id,
  set_buffer_config_event_id,
  select_data_transfer_event_id,
  profile_discovery_event_id,
  profile_dummy_event_id,
  max_ble_event_id
};

//! structures
typedef struct rsi_ble_event_adv_report_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_adv_report_t rsi_ble_event_adv_report;
} rsi_ble_event_adv_report_msg_t;
typedef struct rsi_ble_event_disconnect_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_disconnect_t rsi_ble_event_disconnect;
} rsi_ble_event_disconnect_msg_t;
typedef struct rsi_ble_event_conn_status_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_conn_status_t rsi_ble_event_conn_status;
} rsi_ble_event_conn_status_msg_t;
typedef struct rsi_ble_event_enhance_conn_status_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_enhance_conn_status_t rsi_ble_event_enhance_conn_status;
} rsi_ble_event_enhance_conn_status_msg_t;
typedef struct rsi_ble_event_mtu_exchange_information_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_mtu_exchange_information_t rsi_ble_event_mtu_exchange_information;
} rsi_ble_event_mtu_exchange_information_msg_t;
typedef struct rsi_ble_event_mtu_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_mtu_t rsi_ble_event_mtu;
} rsi_ble_event_mtu_msg_t;
typedef struct rsi_ble_event_le_dev_buf_ind_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_le_dev_buf_ind_t rsi_ble_event_le_dev_buf_ind;
} rsi_ble_event_le_dev_buf_ind_msg_t;
typedef struct rsi_ble_event_remote_features_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_remote_features_t rsi_ble_event_remote_features;
} rsi_ble_event_remote_features_msg_t;
typedef struct rsi_ble_event_data_length_update_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_data_length_update_t rsi_ble_event_data_length_update;
} rsi_ble_event_data_length_update_msg_t;
typedef struct rsi_ble_event_phy_update_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_phy_update_t rsi_ble_event_phy_update;
} rsi_ble_event_phy_update_msg_t;
typedef struct rsi_ble_event_remote_conn_param_req_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_remote_conn_param_req_t rsi_ble_event_remote_conn_param_req;
} rsi_ble_event_remote_conn_param_req_msg_t;
typedef struct rsi_ble_event_conn_update_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_conn_update_t rsi_ble_event_conn_update;
} rsi_ble_event_conn_update_msg_t;
typedef struct rsi_ble_read_req_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_read_req_t rsi_ble_read_req;
} rsi_ble_read_req_msg_t;
typedef struct rsi_ble_event_error_resp_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_error_resp_t rsi_ble_event_error_resp;
} rsi_ble_event_error_resp_msg_t;
typedef struct rsi_ble_event_gatt_desc_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_gatt_desc_t rsi_ble_event_gatt_desc;
} rsi_ble_event_gatt_desc_msg_t;
typedef struct rsi_ble_event_read_by_type1_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_read_by_type1_t rsi_ble_event_read_by_type1;
} rsi_ble_event_read_by_type1_msg_t;
typedef struct rsi_ble_event_profile_by_uuid_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_profile_by_uuid_t rsi_ble_event_profile_by_uuid;
} rsi_ble_event_profile_by_uuid_msg_t;
typedef struct rsi_ble_event_profiles_list_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_profiles_list_t rsi_ble_event_profiles_list;
} rsi_ble_event_profiles_list_msg_t;
typedef struct rsi_ble_event_write_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_write_t rsi_ble_event_write;
} rsi_ble_event_write_msg_t;
typedef struct rsi_ble_event_prepare_write_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_prepare_write_t rsi_ble_event_prepare_write;
} rsi_ble_event_prepare_write_msg_t;
typedef struct rsi_ble_execute_write_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_execute_write_t rsi_ble_execute_write;
} rsi_ble_execute_write_msg_t;
typedef struct rsi_ble_set_att_resp_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_set_att_resp_t rsi_ble_set_att_resp;
} rsi_ble_set_att_resp_msg_t;
typedef struct rsi_ble_prepare_write_resp_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_prepare_write_resp_t rsi_ble_prepare_write_resp;
} rsi_ble_prepare_write_resp_msg_t;
typedef struct rsi_ble_event_att_value_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_event_att_value_t rsi_ble_event_att_value;
} rsi_ble_event_att_value_msg_t;
typedef struct rsi_ble_ae_adv_report_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_ae_adv_report_t rsi_ble_ae_adv_report;
} rsi_ble_ae_adv_report_msg_t;
typedef struct rsi_ble_per_adv_sync_estbl_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_per_adv_sync_estbl_t rsi_ble_per_adv_sync_estbl;
} rsi_ble_per_adv_sync_estbl_msg_t;
typedef struct rsi_ble_per_adv_report_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_per_adv_report_t rsi_ble_per_adv_report;
} rsi_ble_per_adv_report_msg_t;
typedef struct rsi_ble_per_adv_sync_lost_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_per_adv_sync_lost_t rsi_ble_per_adv_sync_lost;
} rsi_ble_per_adv_sync_lost_msg_t;
typedef struct rsi_ble_scan_timeout_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_scan_timeout_t rsi_ble_scan_timeout;
} rsi_ble_scan_timeout_msg_t;
typedef struct rsi_ble_adv_set_terminated_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_adv_set_terminated_t rsi_ble_adv_set_terminated;
} rsi_ble_adv_set_terminated_msg_t;
typedef struct rsi_ble_scan_req_recvd_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_ble_scan_req_recvd_t rsi_ble_scan_req_recvd;
} rsi_ble_scan_req_recvd_msg_t;
typedef struct rsi_ble_event_smp_req_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_smp_req_t rsi_ble_event_smp_req;
} rsi_ble_event_smp_req_msg_t;
typedef struct rsi_ble_event_smp_resp_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_smp_resp_t rsi_ble_event_smp_resp;
} rsi_ble_event_smp_resp_msg_t;
typedef struct rsi_ble_event_smp_passkey_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_smp_passkey_t rsi_ble_event_smp_passkey;
} rsi_ble_event_smp_passkey_msg_t;
typedef struct rsi_ble_event_smp_failed_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_smp_failed_t rsi_ble_event_smp_failed;
} rsi_ble_event_smp_failed_msg_t;
typedef struct rsi_ble_event_encryption_enabled_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_encryption_enabled_t rsi_ble_event_encryption_enabled;
} rsi_ble_event_encryption_enabled_msg_t;
typedef struct rsi_ble_event_smp_passkey_display_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_smp_passkey_display_t rsi_ble_event_smp_passkey_display;
} rsi_ble_event_smp_passkey_display_msg_t;
typedef struct rsi_ble_event_sc_passkey_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_sc_passkey_t rsi_ble_event_sc_passkey;
} rsi_ble_event_sc_passkey_msg_t;
typedef struct rsi_ble_event_le_ltk_request_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_le_ltk_request_t rsi_ble_event_le_ltk_request;
} rsi_ble_event_le_ltk_request_msg_t;
typedef struct rsi_ble_event_le_security_keys_msg_s {
  void *next;
  uint16_t status;
  void (*free_callback)(void *);
  rsi_bt_event_le_security_keys_t rsi_ble_event_le_security_keys;
} rsi_ble_event_le_security_keys_msg_t;

//! function declarations
/*==============================================*/

void rsi_ble_event_adv_report_driver_callback(rsi_ble_event_adv_report_t *rsi_ble_event_adv_report);

/*==============================================*/

void rsi_ble_event_disconnect_driver_callback(rsi_ble_event_disconnect_t *rsi_ble_event_disconnect, uint16_t status);

/*==============================================*/

void rsi_ble_event_conn_status_driver_callback(rsi_ble_event_conn_status_t *rsi_ble_event_conn_status);

/*==============================================*/

void rsi_ble_event_enhance_conn_status_driver_callback(
  rsi_ble_event_enhance_conn_status_t *rsi_ble_event_enhance_conn_status);

/*==============================================*/

void rsi_ble_event_mtu_exchange_information_driver_callback(
  rsi_ble_event_mtu_exchange_information_t *rsi_ble_event_mtu_exchange_information);

/*==============================================*/

void rsi_ble_event_mtu_driver_callback(rsi_ble_event_mtu_t *rsi_ble_event_mtu);

/*==============================================*/

void rsi_ble_event_le_dev_buf_ind_driver_callback(rsi_ble_event_le_dev_buf_ind_t *rsi_ble_event_le_dev_buf_ind);

/*==============================================*/

void rsi_ble_event_remote_features_driver_callback(rsi_ble_event_remote_features_t *rsi_ble_event_remote_features);

/*==============================================*/

void rsi_ble_event_data_length_update_driver_callback(
  rsi_ble_event_data_length_update_t *rsi_ble_event_data_length_update);

/*==============================================*/

void rsi_ble_event_phy_update_driver_callback(rsi_ble_event_phy_update_t *rsi_ble_event_phy_update);

/*==============================================*/

void rsi_ble_event_remote_conn_param_req_driver_callback(
  rsi_ble_event_remote_conn_param_req_t *rsi_ble_event_remote_conn_param_req,
  uint16_t status);

/*==============================================*/

void rsi_ble_event_conn_update_driver_callback(rsi_ble_event_conn_update_t *rsi_ble_event_conn_update, uint16_t status);

/*==============================================*/

void rsi_ble_read_req_driver_callback(uint16_t status, rsi_ble_read_req_t *rsi_ble_read_req);

/*==============================================*/

void rsi_ble_event_error_resp_driver_callback(uint16_t status, rsi_ble_event_error_resp_t *rsi_ble_event_error_resp);

/*==============================================*/

void rsi_ble_event_gatt_desc_driver_callback(rsi_ble_event_gatt_desc_t *rsi_ble_event_gatt_desc);

/*==============================================*/

void rsi_ble_event_read_by_type1_driver_callback(uint16_t status,
                                                 rsi_ble_event_read_by_type1_t *rsi_ble_event_read_by_type1);

/*==============================================*/

void rsi_ble_event_profile_by_uuid_driver_callback(uint16_t status,
                                                   rsi_ble_event_profile_by_uuid_t *rsi_ble_event_profile_by_uuid);

/*==============================================*/

void rsi_ble_event_profiles_list_driver_callback(uint16_t status,
                                                 rsi_ble_event_profiles_list_t *rsi_ble_event_profiles_list);

/*==============================================*/

void rsi_ble_event_write_driver_callback(uint16_t status, rsi_ble_event_write_t *rsi_ble_event_write);

/*==============================================*/

void rsi_ble_event_prepare_write_driver_callback(uint16_t status,
                                                 rsi_ble_event_prepare_write_t *rsi_ble_event_prepare_write);

/*==============================================*/

void rsi_ble_execute_write_driver_callback(uint16_t status, rsi_ble_execute_write_t *rsi_ble_execute_write);

/*==============================================*/

void rsi_ble_set_att_resp_driver_callback(uint16_t status, rsi_ble_set_att_resp_t *rsi_ble_set_att_resp);

/*==============================================*/

void rsi_ble_prepare_write_resp_driver_callback(rsi_ble_prepare_write_resp_t *rsi_ble_prepare_write_resp);

/*==============================================*/

void rsi_ble_event_att_value_driver_callback(uint16_t status, rsi_ble_event_att_value_t *rsi_ble_event_att_value);

/*==============================================*/

void rsi_ble_event_indication_confirmation_driver_callback(uint16_t status,
                                                           rsi_ble_set_att_resp_t *rsi_ble_event_set_att_rsp);

/*==============================================*/

void rsi_ble_ae_adv_report_driver_callback(uint16_t status, rsi_ble_ae_adv_report_t *rsi_ble_ae_adv_report);

/*==============================================*/

void rsi_ble_per_adv_sync_estbl_driver_callback(uint16_t status,
                                                rsi_ble_per_adv_sync_estbl_t *rsi_ble_per_adv_sync_estbl);

/*==============================================*/

void rsi_ble_per_adv_report_driver_callback(uint16_t status, rsi_ble_per_adv_report_t *rsi_ble_per_adv_report);

/*==============================================*/

void rsi_ble_per_adv_sync_lost_driver_callback(uint16_t status, rsi_ble_per_adv_sync_lost_t *rsi_ble_per_adv_sync_lost);

/*==============================================*/

void rsi_ble_scan_timeout_driver_callback(uint16_t status, rsi_ble_scan_timeout_t *rsi_ble_scan_timeout);

/*==============================================*/

void rsi_ble_adv_set_terminated_driver_callback(uint16_t status,
                                                rsi_ble_adv_set_terminated_t *rsi_ble_adv_set_terminated);

/*==============================================*/

void rsi_ble_scan_req_recvd_driver_callback(uint16_t status, rsi_ble_scan_req_recvd_t *rsi_ble_scan_req_recvd);

void rsi_ble_event_adv_report_wrapper(void *msg);
void rsi_ble_event_disconnect_wrapper(void *msg);
void rsi_ble_event_conn_status_wrapper(void *msg);
void rsi_ble_event_enhance_conn_status_wrapper(void *msg);
void rsi_ble_event_mtu_exchange_information_wrapper(void *msg);
void rsi_ble_event_mtu_wrapper(void *msg);
void rsi_ble_event_more_data_req_wrapper(void *msg);
void rsi_ble_simple_peripheral_on_remote_features_event_wrapper(void *msg);
void rsi_ble_event_data_length_update_wrapper(void *msg);
void rsi_ble_event_phy_update_wrapper(void *msg);
void rsi_ble_on_remote_conn_params_request_event_wrapper(void *msg);
void rsi_ble_event_conn_update_wrapper(void *msg);
void rsi_ble_read_req_wrapper(void *msg);
void rsi_ble_gatt_error_event_wrapper(void *msg);
void rsi_ble_event_gatt_desc_wrapper(void *msg);
void rsi_ble_char_services_event_wrapper(void *msg);
void rsi_ble_profile_event_wrapper(void *msg);
void rsi_ble_event_profiles_list_wrapper(void *msg);
void rsi_ble_on_gatt_write_event_wrapper(void *msg);
void rsi_ble_on_gatt_prepare_write_event_wrapper(void *msg);
void rsi_ble_on_execute_write_event_wrapper(void *msg);
void rsi_ble_on_event_write_resp_wrapper(void *msg);
void rsi_ble_prepare_write_resp_wrapper(void *msg);
void rsi_ble_on_read_resp_event_wrapper(void *msg);
void rsi_ble_event_indication_confirmation_wrapper(void *msg);
void rsi_ble_event_ext_adv_report_wrapper(void *msg);
void rsi_ble_event_extended_per_sync_establish_wrapper(void *msg);
void rsi_ble_event_extended_per_adv_report_wrapper(void *msg);
void rsi_ble_event_extended_per_sync_lost_wrapper(void *msg);
void rsi_ble_event_extended_scan_timeout_wrapper(void *msg);
void rsi_ble_on_terminate_wrapper(void *msg);
void rsi_ble_event_extended_scan_req_received_wrapper(void *msg);

void rsi_ble_event_adv_report(uint16_t status, void *ble_event_data);
void rsi_ble_event_disconnect(uint16_t status, void *ble_event_data);
void rsi_ble_event_conn_status(uint16_t status, void *ble_event_data);
void rsi_ble_event_enhance_conn_status(uint16_t status, void *ble_event_data);
void rsi_ble_event_mtu_exchange_information(uint16_t status, void *ble_event_data);
void rsi_ble_event_mtu(uint16_t status, void *ble_event_data);
void rsi_ble_event_more_data_req(uint16_t status, void *ble_event_data);
void rsi_ble_simple_peripheral_on_remote_features_event(uint16_t status, void *ble_event_data);
void rsi_ble_event_data_length_update(uint16_t status, void *ble_event_data);
void rsi_ble_event_phy_update(uint16_t status, void *ble_event_data);
void rsi_ble_on_remote_conn_params_request_event(uint16_t status, void *ble_event_data);
void rsi_ble_event_conn_update(uint16_t status, void *ble_event_data);
void rsi_ble_read_req(uint16_t status, void *ble_event_data);
void rsi_ble_gatt_error_event(uint16_t status, void *ble_event_data);
void rsi_ble_event_gatt_desc(uint16_t status, void *ble_event_data);
void rsi_ble_char_services_event(uint16_t status, void *ble_event_data);
void rsi_ble_profile_event(uint16_t status, void *ble_event_data);
void rsi_ble_event_profiles_list(uint16_t status, void *ble_event_data);
void rsi_ble_on_gatt_write_event(uint16_t status, void *ble_event_data);
void rsi_ble_on_gatt_prepare_write_event(uint16_t status, void *ble_event_data);
void rsi_ble_on_execute_write_event(uint16_t status, void *ble_event_data);
void rsi_ble_on_event_write_resp(uint16_t status, void *ble_event_data);
void rsi_ble_prepare_write_resp(uint16_t status, void *ble_event_data);
void rsi_ble_on_read_resp_event(uint16_t status, void *ble_event_data);
void rsi_ble_event_indication_confirmation(uint16_t status, void *ble_event_data);
void rsi_ble_event_ext_adv_report(uint16_t status, void *ble_event_data);
void rsi_ble_event_extended_per_sync_establish(uint16_t status, void *ble_event_data);
void rsi_ble_event_extended_per_adv_report(uint16_t status, void *ble_event_data);
void rsi_ble_event_extended_per_sync_lost(uint16_t status, void *ble_event_data);
void rsi_ble_event_extended_scan_timeout(uint16_t status, void *ble_event_data);
void rsi_ble_on_terminate(uint16_t status, void *ble_event_data);
void rsi_ble_event_extended_scan_req_received(uint16_t status, void *ble_event_data);
void sl_ble_register_callbacks_wrapper();

//! Extern functions
extern void rsi_ble_event_adv_report(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_disconnect(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_conn_status(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_enhance_conn_status(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_mtu_exchange_information(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_mtu(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_more_data_req(uint16_t status, void *ble_event_data);

extern void rsi_ble_simple_peripheral_on_remote_features_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_data_length_update(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_phy_update(uint16_t status, void *ble_event_data);

extern void rsi_ble_on_remote_conn_params_request_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_conn_update(uint16_t status, void *ble_event_data);

extern void rsi_ble_read_req(uint16_t status, void *ble_event_data);

extern void rsi_ble_gatt_error_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_gatt_desc(uint16_t status, void *ble_event_data);

extern void rsi_ble_char_services_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_profile_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_profiles_list(uint16_t status, void *ble_event_data);

extern void rsi_ble_on_gatt_write_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_on_gatt_prepare_write_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_on_execute_write_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_on_event_write_resp(uint16_t status, void *ble_event_data);

extern void rsi_ble_prepare_write_resp(uint16_t status, void *ble_event_data);

extern void rsi_ble_on_read_resp_event(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_indication_confirmation(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_ext_adv_report(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_extended_per_sync_establish(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_extended_per_adv_report(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_extended_per_sync_lost(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_extended_scan_timeout(uint16_t status, void *ble_event_data);

extern void rsi_ble_on_terminate(uint16_t status, void *ble_event_data);

extern void rsi_ble_event_extended_scan_req_received(uint16_t status, void *ble_event_data);

//! function declarations
/*==============================================*/
void rsi_ble_event_data_transmit_driver_callback(uint8_t conn_id);

/*==============================================*/
void rsi_ble_event_smp_pending_driver_callback(uint8_t *conn_id);

/*==============================================*/
void rsi_ble_event_advertisement_restart_driver_callback(void);

/*==============================================*/
void rsi_ble_event_scan_restart_driver_callback(void);

/*==============================================*/
void rsi_ble_event_on_data_recieve_driver_callback(uint8_t conn_id);

/*==============================================*/
void rsi_ble_event_conn_update_req_driver_callback(uint8_t *conn_id);

/*==============================================*/
void rsi_ble_event_set_buffer_config_driver_callback(uint8_t *conn_id);

/*==============================================*/
void rsi_ble_event_select_data_transfer_driver_callback(uint8_t *conn_id);

/*==============================================*/
void rsi_ble_event_profile_discovery_driver_callback(uint8_t *conn_id);

void rsi_ble_event_profile_dummy_driver_callback(profile_dummy_data_t *profile_dummy_data);

void rsi_ble_event_data_transmit(uint16_t status, void *event_data);
void rsi_ble_event_smp_pending(uint16_t status, void *event_data);
void rsi_ble_event_advertisement_restart(uint16_t status, void *event_data);
void rsi_ble_event_scan_restart(uint16_t status, void *event_data);
void rsi_ble_event_on_data_recieve(uint16_t status, void *event_data);
void rsi_ble_event_conn_update_req(uint16_t status, void *event_data);
void rsi_ble_event_set_buffer_config(uint16_t status, void *event_data);
void rsi_ble_event_select_data_transfer(uint16_t status, void *event_data);
void rsi_ble_event_profile_discovery(uint16_t status, void *event_data);
void rsi_ble_event_profile_dummy(uint16_t status, void *event_data);

//! Extern functions
extern void rsi_ble_event_data_transmit(uint16_t status, void *event_data);
extern void rsi_ble_event_smp_pending(uint16_t status, void *event_data);
extern void rsi_ble_event_advertisement_restart(uint16_t status, void *event_data);
extern void rsi_ble_event_scan_restart(uint16_t status, void *event_data);
extern void rsi_ble_event_on_data_recieve(uint16_t status, void *event_data);
extern void rsi_ble_event_conn_update_req(uint16_t status, void *event_data);
extern void rsi_ble_event_set_buffer_config(uint16_t status, void *event_data);
extern void rsi_ble_event_select_data_transfer(uint16_t status, void *event_data);
extern void rsi_ble_event_profile_discovery(uint16_t status, void *event_data);
extern void rsi_ble_event_profile_dummy(uint16_t status, void *event_data);

/*==============================================*/

void rsi_ble_event_smp_req_driver_callback(rsi_bt_event_smp_req_t *rsi_ble_event_smp_req);

/*==============================================*/

void rsi_ble_event_smp_resp_driver_callback(rsi_bt_event_smp_resp_t *rsi_ble_event_smp_resp);

/*==============================================*/

void rsi_ble_event_smp_passkey_driver_callback(rsi_bt_event_smp_passkey_t *rsi_ble_event_smp_passkey);

/*==============================================*/

void rsi_ble_event_smp_failed_driver_callback(uint16_t status, rsi_bt_event_smp_failed_t *rsi_ble_event_smp_failed);

/*==============================================*/

void rsi_ble_event_encryption_enabled_driver_callback(
  uint16_t status,
  rsi_bt_event_encryption_enabled_t *rsi_ble_event_encryption_enabled);

/*==============================================*/

void rsi_ble_event_smp_passkey_display_driver_callback(
  rsi_bt_event_smp_passkey_display_t *rsi_ble_event_smp_passkey_display);

/*==============================================*/

void rsi_ble_event_sc_passkey_driver_callback(rsi_bt_event_sc_passkey_t *rsi_ble_event_sc_passkey);

/*==============================================*/

void rsi_ble_event_le_ltk_request_driver_callback(rsi_bt_event_le_ltk_request_t *rsi_ble_event_le_ltk_request);

/*==============================================*/

void rsi_ble_event_le_security_keys_driver_callback(rsi_bt_event_le_security_keys_t *rsi_ble_event_le_security_keys);

void rsi_bt_event_smp_req_wrapper(void *msg);
void rsi_bt_event_smp_resp_wrapper(void *msg);
void rsi_bt_event_smp_passkey_wrapper(void *msg);
void rsi_bt_event_smp_failed_wrapper(void *msg);
void rsi_bt_event_encryption_enabled_wrapper(void *msg);
void rsi_bt_event_smp_passkey_display_wrapper(void *msg);
void rsi_bt_event_sc_passkey_wrapper(void *msg);
void rsi_bt_event_le_ltk_request_wrapper(void *msg);
void rsi_bt_event_le_security_keys_wrapper(void *msg);

void rsi_bt_event_smp_req(uint16_t status, void *bt_event_data);
void rsi_bt_event_smp_resp(uint16_t status, void *bt_event_data);
void rsi_bt_event_smp_passkey(uint16_t status, void *bt_event_data);
void rsi_bt_event_smp_failed(uint16_t status, void *bt_event_data);
void rsi_bt_event_encryption_enabled(uint16_t status, void *bt_event_data);
void rsi_bt_event_smp_passkey_display(uint16_t status, void *bt_event_data);
void rsi_bt_event_sc_passkey(uint16_t status, void *bt_event_data);
void rsi_bt_event_le_ltk_request(uint16_t status, void *bt_event_data);
void rsi_bt_event_le_security_keys(uint16_t status, void *bt_event_data);
void sl_ble_register_callbacks_wrapper();

//! Extern functions
extern void rsi_bt_event_smp_req(uint16_t status, void *bt_event_data);
extern void rsi_bt_event_smp_resp(uint16_t status, void *bt_event_data);
extern void rsi_bt_event_smp_passkey(uint16_t status, void *bt_event_data);
extern void rsi_bt_event_smp_failed(uint16_t status, void *bt_event_data);
extern void rsi_bt_event_encryption_enabled(uint16_t status, void *bt_event_data);
extern void rsi_bt_event_smp_passkey_display(uint16_t status, void *bt_event_data);
extern void rsi_bt_event_sc_passkey(uint16_t status, void *bt_event_data);
extern void rsi_bt_event_le_ltk_request(uint16_t status, void *bt_event_data);
extern void rsi_bt_event_le_security_keys(uint16_t status, void *bt_event_data);
extern generic_event_handler_lut_entry_t ble_events_lut[];
#endif