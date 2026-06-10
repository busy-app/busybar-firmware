#include "ble_event_handlers_gap.h"

#include "../../ble_worker_i.h"

#define TAG "BleGAPEvent"

static void ble_print_enh_conn_data(const rsi_ble_event_enhance_conn_status_t* const info) {
    uint8_t buf[18] = {0};
    rsi_6byte_dev_address_to_ascii(buf, info->dev_addr);
    BLE_LOG_W("Addr: %s type: %d role: %d", buf, info->dev_addr_type, info->role);

    rsi_6byte_dev_address_to_ascii(buf, info->local_resolvlable_addr);
    BLE_LOG_W("Local resolvable addr: %s", buf);

    rsi_6byte_dev_address_to_ascii(buf, info->peer_resolvlable_addr);
    BLE_LOG_W("Peer resolvable addr: %s", buf);

    BLE_LOG_W(
        "Interval: %d Latency: %d Timeout: %d",
        info->conn_interval,
        info->conn_latency,
        info->supervision_timeout);
}

bool ble_event_handler_gap_connected(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BleWorker* instance = context;

    rsi_ble_event_enhance_conn_status_t* resp_enh_conn = data;
    memcpy(instance->remote_dev_address, resp_enh_conn->dev_addr, 6);

    ble_print_enh_conn_data(resp_enh_conn);

    // BleDeviceAddressType type = resp_enh_conn->dev_addr_type ==
    BLE_LOG_W("SET PROPER DEVICE ADDR TYPE!!");
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
        instance->on_connection_changed_cb(
            instance->on_connection_changed_ctx, result, instance->str_remote_address);
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

    bool result = ble_device_connection_close(instance->device);

    bool connected = !result; //ble_device_is_connected(instance->device);
    uint8_t dummy[BLE_REMOTE_ADDRESS_STRING_SIZE] = {0};
    instance->on_connection_changed_cb(instance->on_connection_changed_ctx, connected, dummy);

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

    // bool result = false;
    // if(instance->remote_dev_feature.remote_features[1] & 0x01) {
    //     BLE_LOG_I("[BLEWorkerEvtDataLengthChange] rsi_ble_setphy");
    //     sl_status_t status = rsi_ble_setphy(
    //         (int8_t*)instance->remote_dev_address, TX_PHY_RATE, RX_PHY_RATE, CODDED_PHY_RATE);
    //     if(status != RSI_SUCCESS) {
    //         if(status == BLE_WORKER_BT_HCI_COMMAND_DISALLOWED) {
    //             //retry the same command
    //             BLE_LOG_W("Retry setphy");
    //             furi_timer_start(
    //                 instance->retry_phy_timer, furi_ms_to_ticks(BLE_WORKER_RETRY_PHY_TIMEOUT_MS));
    //         } else {
    //             BLE_LOG_W("Failed to set phy, error code : 0x%08lx", status);
    //         }
    //     } else {
    //         BLE_LOG_I(
    //             "PHY set done max_tx_octets: %d\r\nMax_tx_time: %d\r\nMax_rx_octets: %d\r\nMax_rx_time: %d",
    //             instance->data_length_update.MaxTxOctets,
    //             instance->data_length_update.MaxTxTime,
    //             instance->data_length_update.MaxRxOctets,
    //             instance->data_length_update.MaxRxTime);
    //         result = true;
    //     }
    // } else {
    //     BLE_LOG_W("[BLEWorkerEvtDataLengthChange] 2M Phy not supported");
    // }

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

bool ble_event_handler_gap_more_data_request(size_t data_size, void* data, void* context) {
    BLE_LOG_W("ble_event_handler_gap_more_data_request");
    UNUSED(data_size);
    UNUSED(data);
    UNUSED(context);
    return false;
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

// bool ble_worker_event_set_name(size_t data_size, void* data, void* context) {
//     BLE_LOG_I("ble_worker_event_set_name");
//     UNUSED(data_size);

//     SyncEventContext* ctx = data;
//     BleWorker* instance = context;

//     const char* name = ctx->data;
//     BLE_LOG_I("NAME IN TREAD: %s", name);
//     ble_device_set_name(instance->device, name);

//     api_lock_unlock(ctx->lock);

//     return true;
// }

bool ble_event_handler_gap_adjust_connection_request(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("ble_event_handler_gap_adjust_connection_request");
    BleWorker* instance = context;

    if(instance->remote_dev_feature.remote_features[0] & 0x20) {
        BLE_LOG_I("[BLEWorkerReconfigure] rsi_ble_set_data_len");
        sl_status_t status = rsi_ble_set_data_len(instance->remote_dev_address, TX_LEN, TX_TIME);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("Failed to set data length, error code : 0x%08lx", status);
        } else
            BLE_LOG_I("LEN set done");
    } else {
        ble_incoming_nwp_event_processor_spawn_event(
            instance->event_proc, BleIncomingNwpEventTypeDataLengthChange, 0, NULL);
    }
    return true;
}
