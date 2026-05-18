#include "ble_nwp_core_callbacks.h"
#include "ble_worker_event.h"
#include "../ble_common.h"

#define TAG "BleCbcks"

//==========================================================
static BleEventQueuePtr ble_event_queue = NULL;
/*==============================================*/
/**
 * @fn         ble_worker_echo_app_on_adv_report_event
 * @brief      invoked when advertise report event is received
 * @param[in]  adv_report, pointer to the received advertising report
 * @return     none.
 * @section description
 * This callback function updates the scanned remote devices list
 */
static void ble_worker_on_adv_report_event(rsi_ble_event_adv_report_t* adv_report) {
    BLE_LOG_W("ble_worker_on_adv_report_event");

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeAdvReport,
        sizeof(rsi_ble_event_adv_report_t),
        adv_report);

    //     if(ble_worker_instance->device_found == 1) {
    //         return;
    //     }

    //     memset(&ble_worker_instance->remote_name, 0, sizeof(ble_worker_instance->remote_name));
    //     BT_LE_ADPacketExtract(
    //         ble_worker_instance->remote_name, adv_report->adv_data, adv_report->adv_data_len);

    //     ble_worker_instance->remote_addr_type = adv_report->dev_addr_type;
    //     rsi_6byte_dev_address_to_ascii(
    //         ble_worker_instance->remote_dev_str_addr, (uint8_t*)adv_report->dev_addr);
    //     memcpy((int8_t*)ble_worker_instance->remote_dev_bd_addr, (uint8_t*)adv_report->dev_addr, 6);

    // #if(CONNECT_OPTION == CONN_BY_NAME)
    //     if((ble_worker_instance->device_found == 0) &&
    //        ((strcmp((const char*)ble_worker_instance->remote_name, RSI_REMOTE_DEVICE_NAME)) == 0)) {
    //         ble_worker_instance->device_found = 1;

    //         furi_thread_flags_set(
    //             furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtAdvReport);
    //         return;
    //     }
    // #elif(CONNECT_OPTION == CONN_BY_ADDR)
    //     if((!strcmp(RSI_BLE_REMOTE_DEV_ADDR, (char*)ble_worker_instance->remote_dev_str_addr))) {
    //         ble_worker_instance->device_found = 1;
    //         furi_thread_flags_set(
    //             furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtAdvReport);
    //     }
    // #endif

    // return;
}

/**
 * @fn         ble_worker_on_disconnect_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This callback function indicates disconnected device information and status
 */
static void
    ble_worker_on_disconnect_event(rsi_ble_event_disconnect_t* resp_disconnect, uint16_t reason) {
    //This statement is added only to resolve compilation warning, value is unchanged
    UNUSED(reason);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeDisconnected,
        sizeof(rsi_ble_event_disconnect_t),
        resp_disconnect);

    // memcpy(ble_worker_instance->remote_dev_address, resp_disconnect->dev_addr, 6);
    // rsi_6byte_dev_address_to_ascii(
    //     ble_worker_instance->str_remote_address, resp_disconnect->dev_addr);

    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtDisconnected);
}

/**
 * @fn         ble_worker_phy_update_complete_event
 * @brief      invoked when disconnection event is received
 * @param[in]  resp_disconnect, disconnected remote device information
 * @param[in]  reason, reason for disconnection.
 * @return     none.
 * @section description
 * This Callback function indicates disconnected device information and status
 */
static void ble_worker_phy_update_complete_event(
    rsi_ble_event_phy_update_t* rsi_ble_event_phy_update_complete) {
    BLE_LOG_D("ble_worker_phy_update_complete_event");
    // memcpy(
    //     &ble_worker_instance->app_phy_update_complete,
    //     rsi_ble_event_phy_update_complete,
    //     sizeof(rsi_ble_event_phy_update_t));
    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtPhyUpdateComplete);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypePhyUpdateComplete,
        sizeof(rsi_ble_event_phy_update_t),
        rsi_ble_event_phy_update_complete);
}

/**
 * @fn         ble_worker_data_length_change_event
 * @brief      invoked when data length is set
 * @section description
 * This Callback function indicates data length is set
 */
static void ble_worker_data_length_change_event(
    rsi_ble_event_data_length_update_t* rsi_ble_data_length_update) {
    // memcpy(
    //     &ble_worker_instance->data_length_update,
    //     rsi_ble_data_length_update,
    //     sizeof(rsi_ble_event_data_length_update_t));

    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtDataLengthChange);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeDataLengthChange,
        sizeof(rsi_ble_event_data_length_update_t),
        rsi_ble_data_length_update);
}

/**
 * @fn         ble_worker_on_enhance_conn_status_event
 * @brief      invoked when enhanced connection complete event is received
 * @param[out] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
static void
    ble_worker_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t* resp_enh_conn) {
    // memcpy(ble_worker_instance->remote_dev_address, resp_enh_conn->dev_addr, 6);
    // rsi_6byte_dev_address_to_ascii(
    //     ble_worker_instance->str_remote_address, resp_enh_conn->dev_addr);
    // furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtConnected);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeConnected,
        sizeof(rsi_ble_event_enhance_conn_status_t),
        resp_enh_conn);
}

static void ble_worker_on_conn_update_complete_event(
    rsi_ble_event_conn_update_t* rsi_ble_event_conn_update_complete,
    uint16_t resp_status) {
    UNUSED(resp_status);
    // memcpy(
    //     &ble_worker_instance->event_conn_update_complete,
    //     rsi_ble_event_conn_update_complete,
    //     sizeof(rsi_ble_event_conn_update_t));
    // memcpy(
    //     ble_worker_instance->remote_dev_address, rsi_ble_event_conn_update_complete->dev_addr, 6);

    // furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtConnUpdate);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeConnUpdate,
        sizeof(rsi_ble_event_conn_update_t),
        rsi_ble_event_conn_update_complete);
}

static void rsi_ble_on_remote_conn_params_request(
    rsi_ble_event_remote_conn_param_req_t* rsi_ble_event_remote_conn_param,
    uint16_t resp_status) {
    UNUSED(rsi_ble_event_remote_conn_param);
    BLE_LOG_W("rsi_ble_on_remote_conn_params_request: %04X", resp_status);
}

static void rsi_ble_on_directed_adv_report_event(
    rsi_ble_event_directedadv_report_t* rsi_ble_event_directed) {
    BLE_LOG_W("rsi_ble_on_directed_adv_report_event");
    UNUSED(rsi_ble_event_directed);
}
/*==============================================*/
/**
 * @fn         ble_worker_simple_peripheral_on_remote_features_event
 * @brief      invoked when LE remote features event is received.
 * @param[in] resp_conn, connected remote device information
 * @return     none.
 * @section description
 * This callback function indicates the status of the connection
 */
void ble_worker_simple_peripheral_on_remote_features_event(
    rsi_ble_event_remote_features_t* rsi_ble_event_remote_features) {
    // memcpy(
    //     &ble_worker_instance->remote_dev_feature,
    //     rsi_ble_event_remote_features,
    //     sizeof(rsi_ble_event_remote_features_t));
    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtReceiveRemoteFeatures);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeReceiveRemoteFeatures,
        sizeof(rsi_ble_event_remote_features_t),
        rsi_ble_event_remote_features);
}

static void ble_worker_more_data_req_event(rsi_ble_event_le_dev_buf_ind_t* rsi_ble_more_data_evt) {
    UNUSED(rsi_ble_more_data_evt);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeMoreDataRequest,
        sizeof(rsi_ble_event_le_dev_buf_ind_t),
        rsi_ble_more_data_evt);

    return;
}

/*==============================================*/
/**
 * @fn         ble_worker_on_gatt_write_event
 * @brief      its invoked when write/notify/indication events are received.
 * @param[in]  event_id, it indicates write/notification event id.
 * @param[in]  rsi_ble_write, write event parameters.
 * @return     none.
 * @section description
 * This callback function is invoked when write/notify/indication events are received
 */
static void
    ble_worker_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t* rsi_ble_write) {
    UNUSED(event_id);
    // memcpy(
    //     &ble_worker_instance->app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));
    // furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtWrite);

    ble_worker_spawn_event(
        ble_event_queue, BleWorkerEventTypeWrite, sizeof(rsi_ble_event_write_t), rsi_ble_write);
}

static void rsi_ble_on_gatt_prepare_write_event(
    uint16_t event_id,
    rsi_ble_event_prepare_write_t* rsi_ble_write) {
    UNUSED(rsi_ble_write);
    BLE_LOG_W("Prep: %04X", event_id);
}

static void rsi_ble_on_execute_write_event(
    uint16_t event_id,
    rsi_ble_execute_write_t* rsi_ble_execute_write) {
    UNUSED(rsi_ble_execute_write);
    BLE_LOG_W("Exec: %04X", event_id);
}

static void ble_worker_on_indicate_confirmation_event(
    uint16_t event_status,
    rsi_ble_set_att_resp_t* rsi_ble_event_set_att_rsp) {
    UNUSED(rsi_ble_event_set_att_rsp);

    if(event_status != 0) BLE_LOG_W("Indicate_CB: %d", event_status);

    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtIndicateConfirm);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeIndicateConfirm,
        sizeof(rsi_ble_set_att_resp_t),
        rsi_ble_event_set_att_rsp);
}

static void ble_worker_on_mtu_event(rsi_ble_event_mtu_t* rsi_ble_mtu) {
    // memcpy(&ble_worker_instance->app_ble_mtu_event, rsi_ble_mtu, sizeof(rsi_ble_event_mtu_t));

    // furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerEvtMtu);
    ble_worker_spawn_event(
        ble_event_queue, BleWorkerEventTypeMtu, sizeof(rsi_ble_event_mtu_t), rsi_ble_mtu);
}

static void rsi_ble_on_att_desc_resp(
    uint16_t resp_status,
    rsi_ble_resp_att_descs_t* rsi_ble_resp_att_desc) {
    UNUSED(resp_status);
    UNUSED(rsi_ble_resp_att_desc);
    BLE_LOG_W("rsi_ble_on_att_desc_resp_t");
}

static void rsi_ble_on_gatt_error_resp(
    uint16_t event_status,
    rsi_ble_event_error_resp_t* rsi_ble_gatt_error) {
    uint16_t error = *(uint16_t*)rsi_ble_gatt_error->error;
    uint16_t handle = *(uint16_t*)rsi_ble_gatt_error->handle;
    BLE_LOG_W("Status: %04X, err: %04X, handle: %04X", event_status, error, handle);
}

void rsi_ble_on_char_services_resp(
    uint16_t resp_status,
    rsi_ble_resp_char_services_t* rsi_ble_resp_char_serv) {
    UNUSED(rsi_ble_resp_char_serv);
    BLE_LOG_W("rsi_ble_on_char_services_resp status: %04X", resp_status);
}
//===========================================================================================

static void rsi_ble_on_smp_request(rsi_bt_event_smp_req_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_W("rsi_ble_on_smp_request");
}

static void rsi_ble_on_smp_response(rsi_bt_event_smp_resp_t* resp) {
    BLE_LOG_D("rsi_ble_on_smp_response");
    // memcpy(&ble_worker_instance->rsi_bt_event_smp_resp, resp, sizeof(rsi_bt_event_smp_resp_t));
    // furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerSmpResponse);

    ble_worker_spawn_event(
        ble_event_queue, BleWorkerEventTypeSmpResponse, sizeof(rsi_bt_event_smp_resp_t), resp);
}

static void rsi_ble_on_smp_passkey(rsi_bt_event_smp_passkey_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_W("rsi_ble_on_smp_passkey");
}

static void
    rsi_ble_on_smp_failed(uint16_t resp_status, rsi_bt_event_smp_failed_t* remote_dev_address) {
    UNUSED(resp_status);
    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_worker_instance->thread), BleWorkerSmpPairingFailed);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeSmpPairingFailed,
        sizeof(rsi_bt_event_smp_failed_t),
        remote_dev_address);
}

static void rsi_ble_on_encrypt_started(
    uint16_t resp_status,
    rsi_bt_event_encryption_enabled_t* enc_enabled) {
    UNUSED(resp_status);
    BLE_LOG_D("rsi_ble_on_encrypt_started status: %X", resp_status);
    // ble_security_set_pairing_data(ble_worker_instance->security_data, enc_enabled);

    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_worker_instance->thread), BLEWorkerSmpEncryptStarted);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeSmpEncryptStarted,
        sizeof(rsi_bt_event_encryption_enabled_t),
        enc_enabled);
}

static void
    rsi_ble_on_le_ltk_req_event(rsi_bt_event_le_ltk_request_t* rsi_ble_event_le_ltk_request) {
    UNUSED(rsi_ble_event_le_ltk_request);

    BLE_LOG_D("rsi_ble_on_le_ltk_req_event");
    // memcpy(
    //     &ble_worker_instance->ble_ltk_req,
    //     rsi_ble_event_le_ltk_request,
    //     sizeof(rsi_bt_event_le_ltk_request_t));
    // furi_thread_flags_set(furi_thread_get_id(ble_worker_instance->thread), BLEWorkerSmpLtkRequest);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeSmpLtkRequest,
        sizeof(rsi_bt_event_le_ltk_request_t),
        rsi_ble_event_le_ltk_request);
}

static void
    rsi_ble_on_le_security_keys(rsi_bt_event_le_security_keys_t* rsi_ble_event_le_security_keys) {
    BLE_LOG_D("rsi_ble_on_le_security_keys");
    // ble_security_set_rpa_data(ble_worker_instance->security_data, rsi_ble_event_le_security_keys);

    // furi_thread_flags_set(
    //     furi_thread_get_id(ble_worker_instance->thread), BLEWorkerSmpSecurityKeys);

    ble_worker_spawn_event(
        ble_event_queue,
        BleWorkerEventTypeSmpSecurityKeys,
        sizeof(rsi_bt_event_le_security_keys_t),
        rsi_ble_event_le_security_keys);
}

static void ble_on_cli_smp_response_event(rsi_bt_event_smp_resp_t* remote_dev_address) {
    UNUSED(remote_dev_address);
    BLE_LOG_W("ble_on_cli_smp_response_event");
}

static void rsi_ble_on_sc_method(rsi_bt_event_sc_method_t* scmethod) {
    UNUSED(scmethod);
    BLE_LOG_W("rsi_ble_on_sc_method");
}
//===========================================================================================

void ble_nwp_core_config_callbacks(BleEventQueuePtr queue) {
    furi_assert(queue);

    ble_event_queue = queue;

    rsi_ble_gap_register_callbacks(
        ble_worker_on_adv_report_event,
        NULL,
        ble_worker_on_disconnect_event,
        NULL,
        ble_worker_phy_update_complete_event,
        ble_worker_data_length_change_event,
        ble_worker_on_enhance_conn_status_event,
        rsi_ble_on_directed_adv_report_event,
        ble_worker_on_conn_update_complete_event,
        rsi_ble_on_remote_conn_params_request);

    rsi_ble_smp_register_callbacks(
        rsi_ble_on_smp_request,
        rsi_ble_on_smp_response,
        rsi_ble_on_smp_passkey,
        rsi_ble_on_smp_failed,
        rsi_ble_on_encrypt_started,
        NULL,
        NULL,
        rsi_ble_on_le_ltk_req_event,
        rsi_ble_on_le_security_keys,
        ble_on_cli_smp_response_event,
        rsi_ble_on_sc_method);

    rsi_ble_gap_extended_register_callbacks(
        ble_worker_simple_peripheral_on_remote_features_event, ble_worker_more_data_req_event);

    rsi_ble_gatt_register_callbacks(
        NULL,
        NULL,
        rsi_ble_on_char_services_resp,
        NULL,
        rsi_ble_on_att_desc_resp,
        NULL,
        NULL,
        ble_worker_on_gatt_write_event,
        rsi_ble_on_gatt_prepare_write_event,
        rsi_ble_on_execute_write_event,
        NULL,
        ble_worker_on_mtu_event,
        rsi_ble_on_gatt_error_resp,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        ble_worker_on_indicate_confirmation_event,
        NULL);
}
