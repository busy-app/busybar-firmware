#include "ble_worker_gatt_events.h"

#include "../../ble_worker_i.h"

#define TAG "BleGATTEvent"

#define BLE_WORKER_RX_TIMEOUT_MS (10000)

#define BLE_NORDIC_UART_TX_HANDLE  (0x001D)
#define BLE_NORDIC_UART_CNT_HANDLE (0x001F)

bool ble_worker_event_handler_indicate_confirm(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_D("ble_worker_event_handler_indicate_confirm");
    BleWorker* instance = context;
    furi_semaphore_release(instance->indication_sem);
    return true;
}

bool ble_worker_event_handler_mtu(size_t data_size, void* data, void* context) {
    BLE_LOG_I("ble_worker_event_handler_mtu");

    BleWorker* instance = context;
    rsi_ble_event_mtu_t* rsi_ble_mtu = data;

    memcpy(&instance->app_ble_mtu_event, rsi_ble_mtu, data_size);

    BLE_LOG_I(
        "MTU size received from remote device(%s) is %u",
        instance->str_remote_address,
        instance->app_ble_mtu_event.mtu_size);

    instance->max_payload_size =
        instance->app_ble_mtu_event.mtu_size - BLE_WORKER_ATTR_HEADER_SIZE;
    BLE_LOG_I("Max payload size: %u", instance->max_payload_size);

    sl_status_t status = rsi_ble_set_wo_resp_notify_buf_info(
        instance->remote_dev_address, DLE_BUFFER_MODE, DLE_BUFFER_COUNT);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set the buffer configuration mode, error: 0x%08lx", status);
    } else {
        BLE_LOG_I(
            "Buffer configuration done for notify and set_att cmds buf mode = %d , max buff count =%d",
            DLE_BUFFER_MODE,
            DLE_BUFFER_COUNT);
    }

    return true;
}

///TODO: rework this shit to be less messy
bool ble_worker_event_handler_write_event(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BleWorker* instance = context;

    rsi_ble_event_write_t* app_ble_write_event = data;

    uint16_t handle = *(uint16_t*)app_ble_write_event->handle;
    if(app_ble_write_event->pkt_type == RSI_BLE_WRITE_REQUEST_EVENT) {
        const void* data = app_ble_write_event->att_value;
        const size_t data_size = app_ble_write_event->length;

        BleServiceEntry* entry = BleServiceEntryDict_get(instance->service_dict, handle);

        if(entry) {
            if(furi_semaphore_acquire(instance->receive_sem, BLE_WORKER_RX_TIMEOUT_MS) ==
               FuriStatusOk) {
                BLE_LOG_D("Entry present");
                BleServiceObject* service = entry->service;
                if(ble_service_lock(service)) {
                    BleCharacteristicObject* ch = service->chars[entry->char_index];

                    if(ble_characteristic_is_cccd_handle(ch, handle)) {
                        uint8_t ccd_val = *((uint8_t*)data);
                        ble_characteristic_set_cccd_value(ch, ccd_val);
                        sl_status_t status =
                            rsi_ble_gatt_write_response(instance->remote_dev_address, 0);

                        if(status != RSI_SUCCESS) BLE_LOG_W("Response fail");
                        if(handle == BLE_NORDIC_UART_TX_HANDLE) BLE_LOG_W("Subscribed!");

                        furi_semaphore_release(instance->receive_sem);
                    } else {
                        furi_check(data_size > 0);
                        instance->rx_pending_handle = handle;
                        ble_characteristic_set_data(ch, data, data_size);
                        ble_service_enqueue_run(service);
                        if(handle == BLE_NORDIC_UART_CNT_HANDLE) BLE_LOG_W("Session modified!");
                    }

                    ble_service_unlock(service);
                }
            } else {
                BLE_LOG_W("receive_sem timeout!");
            }
        } else {
            BLE_LOG_W("Not found: %04X", handle);
            sl_status_t status = rsi_ble_gatt_write_response(instance->remote_dev_address, 0);
            if(status != RSI_SUCCESS) BLE_LOG_W("Response fail");
        }
    } else if(app_ble_write_event->pkt_type == RSI_BLE_NOTIFICATION_EVENT) {
        BLE_LOG_W("Notification event");
    } else if(app_ble_write_event->pkt_type == RSI_BLE_INDICATION_EVENT) {
        BLE_LOG_W("Indication event");
    } else if(app_ble_write_event->pkt_type == RSI_BLE_WRITE_CMD_EVENT) {
        BLE_LOG_W("CMD event");
    }

    return true;
}
