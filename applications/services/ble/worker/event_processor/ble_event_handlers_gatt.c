#include "../ble_worker_i.h"

#define TAG "BleGATTEvent"

#define BLE_WORKER_RX_TIMEOUT_MS (10000)

#define BLE_NORDIC_UART_TX_HANDLE  (0x001D)
#define BLE_NORDIC_UART_CNT_HANDLE (0x001F)

bool ble_event_handler_gatt_mtu(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BLE_LOG_D("ble_event_handler_gatt_mtu");

    BleWorker* instance = context;
    rsi_ble_event_mtu_t* rsi_ble_mtu = data;

    ///TODO: maybe settings of mtu should be done by connection instance instead of device
    ///because in fact this parameter should be the same between both devices, but for now
    ///it's ok
    ble_device_set_mtu(instance->device, rsi_ble_mtu->mtu_size);

    return true;
}

bool ble_event_handler_gatt_write_event(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    bool result = false;
    BleWorker* instance = context;

    const rsi_ble_event_write_t* app_ble_write_event = data;
    if(app_ble_write_event->pkt_type == RSI_BLE_WRITE_REQUEST_EVENT) {
        const uint16_t handle = *(uint16_t*)app_ble_write_event->handle;
        result = ble_device_process_write_request(
            instance->device,
            app_ble_write_event->dev_addr,
            handle,
            app_ble_write_event->length,
            app_ble_write_event->att_value);

        if(handle == BLE_NORDIC_UART_TX_HANDLE) BLE_LOG_W("Subscribed!");
        if(handle == BLE_NORDIC_UART_CNT_HANDLE) BLE_LOG_W("Session modified!");

    } else if(app_ble_write_event->pkt_type == RSI_BLE_NOTIFICATION_EVENT) {
        BLE_LOG_W("Notification event");
    } else if(app_ble_write_event->pkt_type == RSI_BLE_INDICATION_EVENT) {
        BLE_LOG_W("Indication event");
    } else if(app_ble_write_event->pkt_type == RSI_BLE_WRITE_CMD_EVENT) {
        BLE_LOG_W("CMD event");
    }

    return result;
}

bool ble_event_handler_gatt_read_request_event(size_t data_size, void* data, void* context) {
    UNUSED(data_size);

    BleWorker* instance = context;

    rsi_ble_read_req_t* read_request = data;

    bool result = ble_device_process_read_request(
        instance->device,
        read_request->dev_addr,
        read_request->type,
        read_request->handle,
        read_request->offset);
    return result;
}
