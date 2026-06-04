#include "ble_worker_gap_events.h"

#include "../../ble_worker_i.h"

#define TAG "BleGAPEvent"

bool ble_worker_event_handler_connected(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BleWorker* instance = context;
    rsi_ble_event_enhance_conn_status_t* resp_enh_conn = data;

    memcpy(instance->remote_dev_address, resp_enh_conn->dev_addr, 6);
    rsi_6byte_dev_address_to_ascii(instance->str_remote_address, resp_enh_conn->dev_addr);
    BLE_LOG_I("Connected, str_remote_address : %s", instance->str_remote_address);

    instance->state = BleWorkerStateConnected;
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
    instance->connected = true;
#ifdef BLE_DEBUG_ADVERTISE_FORCE_PUBLIC
    instance->on_connection_changed_cb(
        instance->on_connection_changed_ctx, instance->connected, instance->str_remote_address);
#endif
    return true;
}

bool ble_worker_event_handler_disconnected(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);

    BleWorker* instance = context;

    //! event invokes when disconnection was completed
    BLE_LOG_I("Disconnected, str_remote_address : %s", instance->str_remote_address);

    instance->device_found = 0;
    instance->conn_params_updated = 0;
    instance->connected = false;
    if(instance->rx_pending_handle) {
        BLE_LOG_W("Rx confirm not sent!");
        furi_semaphore_release(instance->receive_sem);
        instance->rx_pending_handle = 0;
    }

    ble_transmitter_reset(instance->transport);

    BleServiceEntryDict_it_t entry_iter;
    for(BleServiceEntryDict_it(entry_iter, instance->service_dict);
        !BleServiceEntryDict_end_p(entry_iter);
        BleServiceEntryDict_next(entry_iter)) {
        BleServiceEntryDict_itref_t* entry_ref = BleServiceEntryDict_ref(entry_iter);

        BleServiceEntry* entry = &entry_ref->value;
        BleServiceObject* service = entry->service;
        if(ble_service_lock(service)) {
            BleCharacteristicObject* ch = service->chars[entry->char_index];
            ble_characteristic_set_cccd_value(ch, 0);
            ble_service_unlock(service);
        }
    }

    //! start advertising
    const rsi_bt_event_le_security_keys_t* rpa =
        ble_security_get_rpa_data(instance->security_data);
    instance->state =
        ble_worker_start_advertising(instance->pairing_info_available, rpa, instance->advertise) ?
            BleWorkerStateAdvertising :
            BleWorkerStateError;

    memset(instance->str_remote_address, 0, BLE_REMOTE_ADDRESS_STRING_SIZE);
    instance->on_connection_changed_cb(
        instance->on_connection_changed_ctx, instance->connected, instance->str_remote_address);

    return true;
}

bool ble_worker_event_handler_phy_update_complete(size_t data_size, void* data, void* context) {
    BLE_LOG_I("ble_worker_event_handler_phy_update_complete");

    BleWorker* instance = context;
    memcpy(&instance->app_phy_update_complete, data, data_size);

    BLE_LOG_I(
        "Tx Phy rate = 0x%x  and Rx Phy rate = 0x%x",
        instance->app_phy_update_complete.TxPhy,
        instance->app_phy_update_complete.RxPhy);
    return true;
}

bool ble_worker_event_handler_connection_update(size_t data_size, void* data, void* context) {
    BLE_LOG_I("ble_worker_event_handler_connection_update");

    BleWorker* instance = context;
    memcpy(&instance->event_conn_update_complete, data, data_size);

    BLE_LOG_I(
        "Connection parameters update completed \r\n Connection interval = %d, Latency = %d, Supervision Timeout = %d",
        instance->event_conn_update_complete.conn_interval,
        instance->event_conn_update_complete.conn_latency,
        instance->event_conn_update_complete.timeout);
    return true;
}

bool ble_worker_event_handler_length_change(size_t data_size, void* data, void* context) {
    BLE_LOG_I("ble_worker_event_handler_length_change");
    BleWorker* instance = context;
    if(data_size > 0) memcpy(&instance->data_length_update, data, data_size);

    UNUSED(instance);
    bool result = false;
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

    return result;
}

bool ble_worker_event_handler_receive_remote_features(size_t data_size, void* data, void* context) {
    BLE_LOG_I("ble_worker_event_handler_receive_remote_features");
    BleWorker* instance = context;
    memcpy(&instance->remote_dev_feature, data, data_size);

    BLE_LOG_I(
        "Feature received is 0x%04X", *(uint16_t*)instance->remote_dev_feature.remote_features);

    return true;
}

bool ble_worker_event_handler_more_data_request(size_t data_size, void* data, void* context) {
    BLE_LOG_W("ble_worker_event_handler_more_data_request");
    UNUSED(data_size);
    UNUSED(data);
    UNUSED(context);
    return false;
}
//------------------------------------------------------------------------------------
///TODO: Move this handlers to commands folder
bool ble_worker_event_handler_exit(size_t data_size, void* data, void* context) {
    BLE_LOG_I("ble_worker_event_handler_exit");
    UNUSED(data_size);
    UNUSED(data);
    BleWorker* instance = context;

    instance->state = ble_worker_stop_advertising() ? BleWorkerStateIdle : BleWorkerStateError;
    furi_event_loop_stop(instance->event_loop);
    return true;
}

bool ble_worker_event_handler_adjust_connection_request(
    size_t data_size,
    void* data,
    void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("ble_worker_event_handler_adjust_connection_request");
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
