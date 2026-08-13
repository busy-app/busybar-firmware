#include "ble_nwp_core_callbacks.h"
#include "../../ble_log.h"

#define TAG "BleNWP"

#define BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED() BLE_LOG_W("%s - not implemented!", __func__)

static BleIncomingNwpEventProcessor* event_processor = NULL;
static BleTransmitter* transport = NULL;

static void rsi_ble_gap_on_adv_report_event(rsi_ble_event_adv_report_t* adv_report) {
    UNUSED(adv_report);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void
    rsi_ble_gap_on_disconnect_event(rsi_ble_event_disconnect_t* resp_disconnect, uint16_t reason) {
    UNUSED(reason);
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeDisconnected,
        sizeof(rsi_ble_event_disconnect_t),
        resp_disconnect);
}

static void rsi_ble_gap_event_le_ping_time_expired_event_dummy(
    rsi_ble_event_le_ping_time_expired_t* rsi_ble_event_timeout_expired) {
    UNUSED(rsi_ble_event_timeout_expired);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gap_on_phy_update_complete_event(
    rsi_ble_event_phy_update_t* rsi_ble_event_phy_update_complete) {
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypePhyUpdateComplete,
        sizeof(rsi_ble_event_phy_update_t),
        rsi_ble_event_phy_update_complete);
}

static void rsi_ble_gap_on_data_length_update_event(
    rsi_ble_event_data_length_update_t* rsi_ble_data_length_update) {
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeDataLengthChange,
        sizeof(rsi_ble_event_data_length_update_t),
        rsi_ble_data_length_update);
}

static void
    rsi_ble_gap_on_enhance_connect_event(rsi_ble_event_enhance_conn_status_t* resp_enh_conn) {
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeConnected,
        sizeof(rsi_ble_event_enhance_conn_status_t),
        resp_enh_conn);
}

static void rsi_ble_gap_on_conn_update_complete_event(
    rsi_ble_event_conn_update_t* rsi_ble_event_conn_update_complete,
    uint16_t resp_status) {
    UNUSED(resp_status);

    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeConnUpdate,
        sizeof(rsi_ble_event_conn_update_t),
        rsi_ble_event_conn_update_complete);
}

static void rsi_ble_gap_on_remote_conn_params_request_event_dummy(
    rsi_ble_event_remote_conn_param_req_t* rsi_ble_event_remote_conn_param,
    uint16_t resp_status) {
    UNUSED(rsi_ble_event_remote_conn_param);
    UNUSED(resp_status);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gap_on_directed_adv_report_event_dummy(
    rsi_ble_event_directedadv_report_t* rsi_ble_event_directed) {
    UNUSED(rsi_ble_event_directed);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

void rsi_ble_gap_ext_on_remote_features_event(
    rsi_ble_event_remote_features_t* rsi_ble_event_remote_features) {
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeReceiveRemoteFeatures,
        sizeof(rsi_ble_event_remote_features_t),
        rsi_ble_event_remote_features);
}

static void rsi_ble_gap_ext_on_le_more_data_request_event(
    rsi_ble_event_le_dev_buf_ind_t* rsi_ble_more_data_evt) {
    UNUSED(rsi_ble_more_data_evt);

    if(transport) ble_transmitter_need_more_data(transport);
}

static void rsi_ble_gatt_profiles_list_resp_event_dummy(
    uint16_t resp_status,
    rsi_ble_resp_profiles_list_t* rsi_ble_resp_profiles) {
    UNUSED(resp_status);
    UNUSED(rsi_ble_resp_profiles);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_profile_resp_event_dummy(
    uint16_t resp_status,
    profile_descriptors_t* rsi_ble_resp_profile) {
    UNUSED(resp_status);
    UNUSED(rsi_ble_resp_profile);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_char_services_resp_event_dummy(
    uint16_t resp_status,
    rsi_ble_resp_char_services_t* rsi_ble_resp_char_serv) {
    UNUSED(resp_status);
    UNUSED(rsi_ble_resp_char_serv);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_inc_services_resp_event_dummy(
    uint16_t resp_status,
    rsi_ble_resp_inc_services_t* rsi_ble_resp_inc_serv) {
    UNUSED(resp_status);
    UNUSED(rsi_ble_resp_inc_serv);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_att_desc_resp_dummy(
    uint16_t resp_status,
    rsi_ble_resp_att_descs_t* rsi_ble_resp_att_desc) {
    UNUSED(resp_status);
    UNUSED(rsi_ble_resp_att_desc);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_read_resp_event_dummy(
    uint16_t resp_status,
    uint16_t resp_id,
    rsi_ble_resp_att_value_t* rsi_ble_resp_att_val) {
    UNUSED(resp_status);
    UNUSED(resp_id);
    UNUSED(rsi_ble_resp_att_val);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_write_resp_event_dummy(uint16_t resp_status, uint16_t resp_id) {
    UNUSED(resp_status);
    UNUSED(resp_id);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_write_event(uint16_t event_id, rsi_ble_event_write_t* rsi_ble_write) {
    UNUSED(event_id);

    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeWrite,
        sizeof(rsi_ble_event_write_t),
        rsi_ble_write);
}

static void rsi_ble_gatt_on_prepare_write_event_dummy(
    uint16_t event_id,
    rsi_ble_event_prepare_write_t* rsi_ble_write) {
    UNUSED(event_id);
    UNUSED(rsi_ble_write);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_execute_write_event_dummy(
    uint16_t event_id,
    rsi_ble_execute_write_t* rsi_ble_execute_write) {
    UNUSED(event_id);
    UNUSED(rsi_ble_execute_write);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void
    rsi_ble_gatt_read_request_event(uint16_t event_id, rsi_ble_read_req_t* rsi_ble_read_req) {
    UNUSED(event_id);

    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeReadRequest,
        sizeof(rsi_ble_read_req_t),
        rsi_ble_read_req);
}

static void rsi_ble_gatt_on_mtu_event(rsi_ble_event_mtu_t* rsi_ble_mtu) {
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor, BleIncomingNwpEventTypeMtu, sizeof(rsi_ble_event_mtu_t), rsi_ble_mtu);
}

static void rsi_ble_gatt_on_error_resp_dummy(
    uint16_t event_status,
    rsi_ble_event_error_resp_t* rsi_ble_gatt_error) {
    UNUSED(event_status);
    UNUSED(rsi_ble_gatt_error);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_desc_val_event_dummy(
    uint16_t event_status,
    rsi_ble_event_gatt_desc_t* rsi_ble_gatt_desc_val) {
    UNUSED(event_status);
    UNUSED(rsi_ble_gatt_desc_val);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_event_profiles_list_dummy(
    uint16_t event_status,
    rsi_ble_event_profiles_list_t* rsi_ble_event_profiles) {
    UNUSED(event_status);
    UNUSED(rsi_ble_event_profiles);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_event_profile_by_uuid_dummy(
    uint16_t event_status,
    rsi_ble_event_profile_by_uuid_t* rsi_ble_event_profile) {
    UNUSED(event_status);
    UNUSED(rsi_ble_event_profile);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_event_read_by_char_services_dummy(
    uint16_t event_status,
    rsi_ble_event_read_by_type1_t* rsi_ble_event_read_type1) {
    UNUSED(event_status);
    UNUSED(rsi_ble_event_read_type1);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_event_read_by_inc_services_dummy(
    uint16_t event_status,
    rsi_ble_event_read_by_type2_t* rsi_ble_event_read_type2) {
    UNUSED(event_status);
    UNUSED(rsi_ble_event_read_type2);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_event_read_att_value_dummy(
    uint16_t event_status,
    rsi_ble_event_read_by_type3_t* rsi_ble_event_read_type3) {
    UNUSED(event_status);
    UNUSED(rsi_ble_event_read_type3);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_event_read_resp_dummy(
    uint16_t event_status,
    rsi_ble_event_att_value_t* rsi_ble_event_att_val) {
    UNUSED(event_status);
    UNUSED(rsi_ble_event_att_val);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_event_write_resp_dummy(
    uint16_t event_status,
    rsi_ble_set_att_resp_t* rsi_ble_event_set_att_rsp) {
    UNUSED(event_status);
    UNUSED(rsi_ble_event_set_att_rsp);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_gatt_on_event_indicate_confirmation_event(
    uint16_t event_status,
    rsi_ble_set_att_resp_t* rsi_ble_event_set_att_rsp) {
    UNUSED(rsi_ble_event_set_att_rsp);

    if(event_status != 0) BLE_LOG_W("Indicate_CB: %d", event_status);

    if(transport) ble_transmitter_indication_done(transport);
}

static void rsi_ble_gatt_on_event_prepare_write_resp_dummy(
    uint16_t event_status,
    rsi_ble_prepare_write_resp_t* rsi_ble_event_prepare_write) {
    UNUSED(event_status);
    UNUSED(rsi_ble_event_prepare_write);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_smp_on_request_dummy(rsi_bt_event_smp_req_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_smp_on_response(rsi_bt_event_smp_resp_t* resp) {
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor, BleIncomingNwpEventTypeSmpResponse, sizeof(rsi_bt_event_smp_resp_t), resp);
}

static void rsi_ble_smp_on_passkey_dummy(rsi_bt_event_smp_passkey_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void
    rsi_ble_smp_on_failed(uint16_t resp_status, rsi_bt_event_smp_failed_t* remote_dev_address) {
    UNUSED(resp_status);
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeSmpPairingFailed,
        sizeof(rsi_bt_event_smp_failed_t),
        remote_dev_address);
}

static void rsi_ble_smp_on_encrypt_started(
    uint16_t resp_status,
    rsi_bt_event_encryption_enabled_t* enc_enabled) {
    UNUSED(resp_status);

    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeSmpEncryptStarted,
        sizeof(rsi_bt_event_encryption_enabled_t),
        enc_enabled);
}

static void
    rsi_ble_smp_on_passkey_display_dummy(rsi_bt_event_smp_passkey_display_t* smp_passkey_display) {
    UNUSED(smp_passkey_display);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_smp_on_sc_passkey_dummy(rsi_bt_event_sc_passkey_t* sc_passkey) {
    UNUSED(sc_passkey);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void
    rsi_ble_smp_on_le_ltk_req_event(rsi_bt_event_le_ltk_request_t* rsi_ble_event_le_ltk_request) {
    UNUSED(rsi_ble_event_le_ltk_request);

    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeSmpLtkRequest,
        sizeof(rsi_bt_event_le_ltk_request_t),
        rsi_ble_event_le_ltk_request);
}

static void rsi_ble_smp_on_le_security_keys(
    rsi_bt_event_le_security_keys_t* rsi_ble_event_le_security_keys) {
    ble_incoming_nwp_event_processor_spawn_event(
        event_processor,
        BleIncomingNwpEventTypeSmpSecurityKeys,
        sizeof(rsi_bt_event_le_security_keys_t),
        rsi_ble_event_le_security_keys);
}

static void
    rsi_ble_smp_on_cli_smp_response_event_dummy(rsi_bt_event_smp_resp_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

static void rsi_ble_smp_on_sc_method_dummy(rsi_bt_event_sc_method_t* scmethod) {
    UNUSED(scmethod);
    BLE_LOG_NWP_EVENT_NOT_IMPLEMENTED();
}

void ble_nwp_core_config_callbacks(
    BleIncomingNwpEventProcessor* event_processor_instance,
    BleTransmitter* transport_instance) {
    furi_assert(event_processor_instance);
    furi_assert(transport_instance);

    event_processor = event_processor_instance;
    transport = transport_instance;

    rsi_ble_gap_register_callbacks(
        rsi_ble_gap_on_adv_report_event,
        NULL,
        rsi_ble_gap_on_disconnect_event,
        rsi_ble_gap_event_le_ping_time_expired_event_dummy,
        rsi_ble_gap_on_phy_update_complete_event,
        rsi_ble_gap_on_data_length_update_event,
        rsi_ble_gap_on_enhance_connect_event,
        rsi_ble_gap_on_directed_adv_report_event_dummy,
        rsi_ble_gap_on_conn_update_complete_event,
        rsi_ble_gap_on_remote_conn_params_request_event_dummy);

    rsi_ble_gap_extended_register_callbacks(
        rsi_ble_gap_ext_on_remote_features_event, rsi_ble_gap_ext_on_le_more_data_request_event);

    rsi_ble_gatt_register_callbacks(
        rsi_ble_gatt_profiles_list_resp_event_dummy,
        rsi_ble_gatt_profile_resp_event_dummy,
        rsi_ble_gatt_on_char_services_resp_event_dummy,
        rsi_ble_gatt_inc_services_resp_event_dummy,
        rsi_ble_gatt_on_att_desc_resp_dummy,
        rsi_ble_gatt_read_resp_event_dummy,
        rsi_ble_gatt_write_resp_event_dummy,
        rsi_ble_gatt_on_write_event,
        rsi_ble_gatt_on_prepare_write_event_dummy,
        rsi_ble_gatt_on_execute_write_event_dummy,
        rsi_ble_gatt_read_request_event,
        rsi_ble_gatt_on_mtu_event,
        rsi_ble_gatt_on_error_resp_dummy,
        rsi_ble_gatt_on_desc_val_event_dummy,
        rsi_ble_gatt_on_event_profiles_list_dummy,
        rsi_ble_gatt_on_event_profile_by_uuid_dummy,
        rsi_ble_gatt_on_event_read_by_char_services_dummy,
        rsi_ble_gatt_on_event_read_by_inc_services_dummy,
        rsi_ble_gatt_on_event_read_att_value_dummy,
        rsi_ble_gatt_on_event_read_resp_dummy,
        rsi_ble_gatt_on_event_write_resp_dummy,
        rsi_ble_gatt_on_event_indicate_confirmation_event,
        rsi_ble_gatt_on_event_prepare_write_resp_dummy);

    rsi_ble_smp_register_callbacks(
        rsi_ble_smp_on_request_dummy,
        rsi_ble_smp_on_response,
        rsi_ble_smp_on_passkey_dummy,
        rsi_ble_smp_on_failed,
        rsi_ble_smp_on_encrypt_started,
        rsi_ble_smp_on_passkey_display_dummy,
        rsi_ble_smp_on_sc_passkey_dummy,
        rsi_ble_smp_on_le_ltk_req_event,
        rsi_ble_smp_on_le_security_keys,
        rsi_ble_smp_on_cli_smp_response_event_dummy,
        rsi_ble_smp_on_sc_method_dummy);
}
