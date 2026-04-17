//- defined under BLE_PRIVATE_H macro
#ifndef BLE_PRIVATE_H
#define BLE_PRIVATE_H
#include "rsi_common_app.h"
#include "event_loop.h"
#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ble_config.h"
#include "rsi_ble.h"
#include "rsi_ble_apis.h"

typedef struct rsi_ble_conn_config_s {
  uint8_t conn_id;
  bool smp_enable;
  bool add_to_acceptlist;
  bool profile_discovery;
  bool data_transfer;
  //  bool bidir_datatransfer;
  bool rx_notifications;
  //    bool rx_write;
  //    bool rx_write_no_response;
  bool rx_indications;
  bool tx_notifications;
  bool tx_write;
  bool tx_write_no_response;
  bool tx_indications;
  uint16_t tx_write_clientservice_uuid;
  uint16_t tx_write_client_char_uuid;
  uint16_t tx_wnr_client_service_uuid;
  uint16_t tx_wnr_client_char_uuid;
  uint16_t rx_indi_client_service_uuid;
  uint16_t rx_indi_client_char_uuid;
  uint16_t rx_notif_client_service_uuid;
  uint16_t rx_notif_client_char_uuid;
  struct buff_mode_sel_s {
    bool buffer_mode;
    uint8_t buffer_cnt;
    uint8_t max_data_length;
  } buff_mode_sel;
  struct conn_param_update_s {
    uint16_t conn_int;
    uint8_t conn_latncy;
    uint16_t supervision_to;
  } conn_param_update;
} rsi_ble_conn_config_t;

typedef struct ble_confg_info_s {
  rsi_ble_conn_config_t ble_conn_configuration[TOTAL_CONNECTIONS];
} ble_confg_info_t;

typedef struct rsi_ble_profile_list_by_conn_s {
  profile_descriptors_t *profile_desc;
  rsi_ble_event_profile_by_uuid_t *profile_info_uuid;
  rsi_ble_event_read_by_type1_t *profile_char_info;
} rsi_ble_profile_list_by_conn_t;

typedef enum ble_connection_state_s {
  disconnected_state,      //! ACL link does not exist
  initiated_connect_state, //! Application triggered connect request, waiting on_connect
  on_connect_state,        //! received on_connect, security exchanges ongoing
  connected_state,         //! auth complete
} ble_connection_state_t;

typedef enum adv_state_s {
  adv_disabled, //! adv is off
  adv_enabled,  //! adv enabled
  adv_enabled_non_connectable,

} adv_state_t;

typedef enum scan_state_s {
  scan_off,             //! scan_off
  connectable_scan,     //! scan on and connection is possible
  non_connectable_scan, //! scan on and connection is not possible
} scan_state_t;

typedef enum mtu_state_s {
  mtu_exchange_info_state,
  mtu_exchange_done_state,
} mtu_state_t;

typedef enum smp_state_s {
  smp_enable,
  smp_triggered,
  smp_pending,
  smp_process_done,
} smp_state_t;

typedef struct more_data_state_beta_s {
  uint8_t data_transmit;

} more_data_state_beta_t;

typedef struct profile_dummy_data_s {
  uint8_t ble_con_id;
  uint8_t swtch_cnt;
} profile_dummy_data_t;

typedef struct rsi_ble_conn_info_s {
  uint8_t conn_id;
  uint8_t conn_status;

  ble_connection_state_t conn_state;
  adv_state_t adv_state;
  scan_state_t scan_state;
  bool buff_config_done;
  bool conn_param_req_given;
  bool char_resp_recvd;
  bool char_desc_resp_recvd;
  bool write_handle_found;
  rsi_ble_conn_config_t *ble_conn_conf;
  bool smp_pairing_initated;
  bool smp_pairing_request_received;
  uint16_t write_cnt;
  bool write_wwr_handle_found;
  int16_t write_handle;
  uint16_t wwr_count;
  bool notify_handle_found;
  uint16_t notify_handle;
  bool indication_handle_found;
  uint16_t indication_handle;
  int16_t write_wwr_handle;
  bool transmit;
  uint8_t l_num_of_services;
  uint8_t l_char_property;
  uint8_t char_for_serv_cnt;
  uint8_t char_desc_cnt;
  uint8_t profile_index_for_char_query;
  uint8_t temp1;
  uint8_t temp2;
  uint16_t temp_prepare_write_value_len;

  uint8_t temp_prepare_write_value[250];
  uint8_t mtu_exchange_done;

  uint8_t prep_write_err;
  uint8_t first_connect;
  uint8_t smp_done;
  uint8_t neg_rply;
  uint16_t offset;
  uint16_t handle;
  uint8_t type;
  uint8_t read_data1[230];
  uint8_t max_data_length;
  bool rsi_rx_from_rem_dev;
  bool rsi_tx_to_rem_dev;
  uint16_t indication_cnt;
  uint16_t notfy_cnt;
  uint8_t prof_cnt;
  uint8_t char_cnt;

  bool notification_received;
  uint8_t str_remote_address[RSI_REM_DEV_ADDR_LEN];
  bool done_profiles_query;
  bool prof_resp_recvd;

  uint8_t no_of_profiles;
  uint8_t total_remote_profiles;

  uint8_t profs_evt_cnt, prof_evt_cnt;

  uint16_t profiles_endhandle;
  uint8_t rsi_connected_dev_addr[RSI_DEV_ADDR_LEN];
  uint8_t skip_end_profile;
  uint8_t switch_case_count;
  rsi_ble_profile_list_by_conn_t rsi_ble_profile_list_by_conn;
  bool profile_mem_init;
  bool service_char_mem_init;

  uint8_t remote_device_role;                         //! 1 - remote device is peripheral, 2 - remote device is central
  uint8_t remote_dev_addr[18];                        //! 18 = RSI_REM_DEV_ADDR_LEN
  rsi_ble_event_remote_features_t remote_dev_feature; //! -- rsi_ble_simple_peripheral_on_remote_features_event()
  rsi_ble_event_adv_report_t rsi_app_adv_reports_to_app; //! -- rsi_ble_simple_central_on_adv_report_event()
  rsi_ble_ae_adv_report_t rsi_app_ae_adv_reports_to_app; //! -- rsi_ble_simple_central_on_ae_adv_report_event()
  rsi_ble_event_conn_update_t conn_update_resp;          //! -- ble_on_conn_update_complete_event()
  rsi_ble_event_remote_conn_param_req_t
    rsi_app_remote_device_conn_params; //! -- rsi_ble_on_remote_conn_params_request_event()
  rsi_ble_event_conn_status_t
    conn_event_to_app; //! -- rsi_ble_on_connect_event()  rsi_ble_on_disconnect_event() rsi_ble_on_enhance_conn_status_event()
  rsi_ble_event_enhance_conn_status_t rsi_enhc_conn_status;
  bool is_enhanced_conn;                     //! -- flag to indicate enhanced connection(1) or normal connection(0)
  rsi_ble_event_write_t app_ble_write_event; //! -- rsi_ble_on_gatt_write_event()
  rsi_ble_event_prepare_write_t app_ble_prepared_write_event;
  rsi_ble_execute_write_t app_ble_execute_write_event;
  rsi_ble_set_att_resp_t rsi_ble_write_resp_event;
  rsi_ble_read_req_t app_ble_read_event;                      //! -- rsi_ble_on_read_req_event()
  rsi_ble_event_mtu_t app_ble_mtu_event;                      //! -- rsi_ble_on_mtu_event()
  rsi_ble_event_mtu_exchange_information_t mtu_exchange_info; //! --rsi_ble_on_mtu_exchange_info_t
  rsi_ble_event_disconnect_t rsi_ble_disconn_resp;            //! -- rsi_ble_on_disconnect_event()
  rsi_ble_event_profiles_list_t get_allprofiles;              //! -- rsi_ble_profiles_list_event()
  rsi_ble_event_profile_by_uuid_t get_profile;                //! -- rsi_ble_profile_event()
  rsi_ble_event_read_by_type1_t get_char_services;            //! -- rsi_ble_char_services_event()
  rsi_ble_event_att_value_t rsi_char_descriptors;
  rsi_ble_event_error_resp_t rsi_ble_gatt_err_resp;
  rsi_bt_event_smp_resp_t rsi_ble_event_smp_resp;
  rsi_bt_event_smp_req_t rsi_ble_event_smp_req;
  rsi_bt_event_smp_passkey_t rsi_ble_event_smp_passkey;
  rsi_bt_event_smp_passkey_display_t rsi_ble_smp_passkey_display;
  rsi_bt_event_sc_passkey_t rsi_event_sc_passkey;
  rsi_bt_event_smp_failed_t rsi_ble_smp_failed;
  rsi_bt_event_encryption_enabled_t rsi_encryption_enabled;
  rsi_bt_event_le_ltk_request_t rsi_le_ltk_resp;
  rsi_bt_event_le_security_keys_t rsi_le_security_keys;
  rsi_bt_event_encryption_enabled_t l_rsi_encryption_enabled;
#if (CONNECT_OPTION == CONN_BY_NAME)
  uint8_t *rsi_remote_name;
#endif
  uuid_t search_serv_conn[10];

  mtu_state_t mtu_state;
  smp_state_t smp_state;
  bool notifications_enabled;
  bool indications_enabled;
} rsi_ble_conn_info_t;

typedef struct rsi_parsed_conf_s {
  struct rsi_protocol_sel_t {
    bool is_ble_enabled;
    bool is_bt_enabled;
    bool is_ant_enabled;
    bool is_wifi_enabled;
  } rsi_protocol_sel;
  struct rsi_ble_config_t {
    uint8_t no_of_peripherals;
    uint8_t no_of_centrals;
    uint8_t conn_by_name;
    uint8_t peripheral1_name[RSI_REM_DEV_NAME_LEN];
    uint8_t peripheral2_name[RSI_REM_DEV_NAME_LEN];
    uint8_t peripheral3_name[RSI_REM_DEV_NAME_LEN];
    uint8_t peripheral1_addr[RSI_REM_DEV_ADDR_LEN];
    uint8_t peripheral2_addr[RSI_REM_DEV_ADDR_LEN];
    uint8_t peripheral3_addr[RSI_REM_DEV_ADDR_LEN];
    rsi_ble_conn_config_t rsi_ble_conn_config[TOTAL_CONNECTIONS];
  } rsi_ble_config;

} rsi_parsed_conf_t;

/***********************************************************************************************************************************************/
//! user defined structure
/***********************************************************************************************************************************************/
typedef struct rsi_ble_att_list_s {
  uuid_t char_uuid;
  uint16_t handle;
  uint16_t value_len;
  uint16_t max_value_len;
  uint8_t char_val_prop;
  void *value;
} rsi_ble_att_list_t;

typedef struct rsi_ble_s {
  uint8_t DATA[BLE_ATT_REC_SIZE];
  uint16_t DATA_ix;
  uint16_t att_rec_list_count;
  rsi_ble_att_list_t att_rec_list[NO_OF_VAL_ATT];
} rsi_ble_t;

//LE resolvlist.
typedef struct rsi_ble_dev_ltk_list_s {

  uint8_t enc_enable;
  uint8_t sc_enable;
  uint8_t remote_dev_addr_type;
  uint8_t remote_dev_addr[6];
  uint8_t peer_irk[16];
  uint8_t local_irk[16];
  uint16_t remote_ediv;
  uint16_t local_ediv;
  uint8_t remote_rand[8];
  uint8_t localrand[8];
  uint8_t remote_ltk[16];
  uint8_t localltk[16];
  uint8_t Identity_addr_type;
  uint8_t Identity_addr[6];
  uint8_t used;
  uint8_t derived_linkkey[16];

} rsi_ble_dev_ltk_list_t;

typedef struct ble_peer_info_s {
  rsi_ble_conn_info_t ble_peers[5];
} ble_peer_info_t;

typedef struct tx_generic_event_message_s {
  void *next;
  void (*free_callback)(void *); //! handler to free command message
  uint16_t status;
  uint32_t event_id;
  uint8_t event_data;
} tx_generic_event_message_t;

extern generic_task_cb_t ble_generic_cb;
extern more_data_state_beta_t more_data_state_beta[TOTAL_CONNECTIONS];
extern rsi_ble_req_adv_t change_adv_param;
extern rsi_ble_req_scan_t change_scan_param;
extern rsi_ble_conn_info_t rsi_ble_conn_info[TOTAL_CONNECTIONS];

extern uint8_t rsi_check_dev_list_driver(uint8_t *remote_dev_name, uint8_t *adv_dev_addr);

extern uint8_t peripheral_con_req_pending;

extern volatile uint16_t rsi_ble_att1_val_hndl;

extern rsi_ble_t att_list;
extern rsi_ble_dev_ltk_list_t ble_dev_ltk_list[TOTAL_CONNECTIONS];
extern int32_t status;

extern rsi_ble_att_list_t *rsi_gatt_get_attribute_from_list(rsi_ble_t *p_val, uint16_t handle);
extern uint8_t rsi_get_remote_device_role(uint8_t *remote_dev_addr);
extern bool rsi_parse_conn_conf(uint8_t *recv_buffer, uint8_t line_no, uint8_t temp_len);
extern bool rsi_parse_input(uint8_t *recv_buffer, uint8_t parse_line);

#if (CONNECT_OPTION == CONN_BY_NAME)
extern uint8_t rsi_get_ble_conn_id(uint8_t *remote_dev_addr, uint8_t *remote_name, uint8_t size);
#else
extern uint8_t rsi_get_ble_conn_id(uint8_t *remote_dev_addr);
#endif

#if (CONNECT_OPTION == CONN_BY_NAME)
extern uint8_t rsi_add_ble_conn_id(uint8_t *remote_dev_addr, uint8_t *remote_name, uint8_t size);
#else
extern uint8_t rsi_add_ble_conn_id(uint8_t *remote_dev_addr);
#endif

extern uint8_t i, temp, smp_in_progress;
extern volatile uint16_t rsi_ble_att1_val_hndl;
extern volatile uint16_t rsi_ble_att2_val_hndl;
extern volatile uint16_t rsi_ble_att3_val_hndl;
extern uint8_t adv_pkt_processing_pending;
extern uint8_t num_of_connected_peripheral_devices;
extern rsi_ble_conn_config_t ble_conn_configuration[TOTAL_CONNECTIONS];
extern adv_state_t adv_state_dut;
extern scan_state_t scan_state_dut;
extern uint8_t peripheral_count;
extern uint8_t central_count;

//! function protoypes

extern osSemaphoreId_t ble_wait_on_connect;
#endif
