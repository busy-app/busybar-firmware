#include "ble_event_handlers_gap.h"

#include "../ble_worker_i.h"

#define TAG "BleGAPEvent"

#define BLE_ADJUST_CONNECTION_PARAMETERS_TIMEOUT (500)

bool ble_event_handler_gap_connected(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BleWorker* instance = context;

    rsi_ble_event_enhance_conn_status_t* resp_enh_conn = data;

    BleDeviceAddressType type = BleDeviceAddressTypeOrigin;
    bool result = ble_device_connection_open(instance->device, type, resp_enh_conn->dev_addr);

    BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
    BleConnectionTimings timings = {
        .interval = resp_enh_conn->conn_interval,
        .latency = resp_enh_conn->conn_latency,
        .timeout = resp_enh_conn->supervision_timeout};
    ble_connection_set_timings(conn, &timings);

    FuriString* addr = furi_string_alloc();
    BleDeviceBase* peer = ble_connection_get_peer(conn);
    ble_device_base_format_address(peer, BleDeviceAddressTypeOrigin, addr);
    BLE_LOG_I("Connected, address : %s", furi_string_get_cstr(addr));
    furi_string_free(addr);

    // const BleDeviceCommon* peer = ble_connection_get_peer()
    // ble_device_set_address

    ///TODO: Commented due to issues with connect to different phones remove when interaction logic will be finalized
    //! Setting MTU Exchange event
    // status =
    //     rsi_ble_mtu_exchange_event(instance->remote_dev_address, BLE_WORKER_MAX_MTU_SIZE);
    // if(status != RSI_SUCCESS) {
    //     BLE_LOG_W("MTU request cmd failed with error code = 0x%08lx", status);
    //     furi_crash();
    // } else {
    //     BLE_LOG_I("MTU sent");
    // }
#ifdef BLE_DEBUG_ADVERTISE_FORCE_PUBLIC
    if(result) {
        ble_worker_invoke_connect_callback(instance);
    }
#endif
    return result;
}

bool ble_event_handler_gap_disconnected(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);

    BleWorker* instance = context;

    //! event invokes when disconnection was completed
    BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
    FuriString* addr = furi_string_alloc();
    BleDeviceBase* peer = ble_connection_get_peer(conn);
    ble_device_base_format_address(peer, BleDeviceAddressTypeOrigin, addr);
    BLE_LOG_I("Disconnect, address : %s", furi_string_get_cstr(addr));
    furi_string_free(addr);

    furi_event_loop_timer_stop(instance->update_param_timer);

    bool result = ble_device_connection_close(instance->device);

    ble_worker_invoke_disconnect_callback(instance);

    return result;
}

bool ble_event_handler_gap_phy_update_complete(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BLE_LOG_I("%s", __func__);

    rsi_ble_event_phy_update_t* new_phy = data;

    BleWorker* instance = context;
    BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
    ble_connection_set_phy(conn, new_phy->TxPhy, new_phy->RxPhy);

    BLE_LOG_I("Tx Phy rate = 0x%x and Rx Phy rate = 0x%x", new_phy->TxPhy, new_phy->RxPhy);
    return true;
}

bool ble_event_handler_gap_connection_update(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BLE_LOG_I("%s", __func__);

    rsi_ble_event_conn_update_t* con_upd = data;

    BleWorker* instance = context;
    BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
    BleConnectionTimings timings = {
        .interval = con_upd->conn_interval,
        .latency = con_upd->conn_latency,
        .timeout = con_upd->timeout,
    };
    ble_connection_set_timings(conn, &timings);

    BLE_LOG_I(
        "Connection parameters update completed \r\n Connection interval = %d, Latency = %d, Supervision Timeout = %d",
        con_upd->conn_interval,
        con_upd->conn_latency,
        con_upd->timeout);
    return true;
}

bool ble_event_handler_gap_length_change(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BLE_LOG_I("%s", __func__);
    rsi_ble_event_data_length_update_t* rsi_len = data;

    BleWorker* instance = context;
    BleConnectionContext* conn = ble_device_get_connection_context(instance->device);

    BleConnectionDataLength len = {
        .MaxRxOctets = rsi_len->MaxRxOctets,
        .MaxTxOctets = rsi_len->MaxTxOctets,
        .MaxRxTime = rsi_len->MaxRxTime,
        .MaxTxTime = rsi_len->MaxTxTime,
    };

    ble_connection_set_data_length(conn, &len);

    return true;
}

bool ble_event_handler_gap_receive_remote_features(size_t data_size, void* data, void* context) {
    BLE_LOG_I("%s", __func__);
    UNUSED(data_size);
    rsi_ble_event_remote_features_t* features = data;

    BleWorker* instance = context;

    BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
    BleDeviceBase* peer = ble_connection_get_peer(conn);
    ble_device_base_set_features(peer, features->remote_features);

    return true;
}

//------------------------------------------------------------------------------------
///TODO: Move this handlers to commands folder
bool ble_event_handler_gap_exit(size_t data_size, void* data, void* context) {
    BLE_LOG_I("ble_event_handler_gap_exit");
    UNUSED(data_size);
    UNUSED(data);
    BleWorker* instance = context;

    ble_device_stop(instance->device);

    furi_event_loop_stop(instance->event_loop);
    return true;
}

bool ble_event_handler_gap_adjust_connection_request(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("%s", __func__);
    BleWorker* instance = context;
    BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
    bool all_done = (conn != NULL) && ble_connection_update_phy_and_data_length_by_timer(conn);
    if(!all_done) {
        furi_event_loop_timer_start(
            instance->update_param_timer, BLE_ADJUST_CONNECTION_PARAMETERS_TIMEOUT);
    }
    return true;
}
