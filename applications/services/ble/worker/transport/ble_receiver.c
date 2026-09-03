#include "ble_receiver.h"

#include "../_nwp_callbacks/ble_nwp_headers.h"
#include "../../ble_log.h"

#define BLE_RX_TIMEOUT_MS (10000)

#define TAG "BleReceiver"

struct BleReceiverContext {
    FuriSemaphore* receive_sem;
    uint16_t rx_pending_handle;
    uint16_t rx_pending_cccd;
    uint8_t peer_addr[DEVICE_ADDR_LEN];
    bool enabled;
};

BleReceiverContext* ble_receiver_alloc(const uint8_t* peer_addr) {
    BleReceiverContext* instance = malloc(sizeof(BleReceiverContext));
    instance->receive_sem = furi_semaphore_alloc(1, 1);
    memcpy(instance->peer_addr, peer_addr, DEVICE_ADDR_LEN);
    instance->enabled = false;
    instance->rx_pending_handle = 0;
    instance->rx_pending_cccd = 0;
    return instance;
}

void ble_receiver_free(BleReceiverContext* instance) {
    furi_assert(instance);
    furi_semaphore_free(instance->receive_sem);
    free(instance);
}

void ble_receiver_enable(BleReceiverContext* instance) {
    furi_assert(instance);
    instance->enabled = true;
    BLE_LOG_I("Enabled");
    if(instance->rx_pending_handle != 0) {
        ble_receiver_transfer_confirm(
            instance, instance->rx_pending_handle, instance->rx_pending_cccd);
    }
}

typedef void (*BleReceiverConfirmHandler)(uint8_t* addr, uint8_t type);

static void ble_receiver_confirm_write(uint8_t* addr, uint8_t type) {
    sl_status_t status = rsi_ble_gatt_write_response(addr, type);
    if(status != 0) BLE_LOG_W("Write request confirm fail %08lX", status);
}

static void ble_receiver_confirm_indicate(uint8_t* addr, uint8_t type) {
    UNUSED(type);
    sl_status_t status = rsi_ble_indicate_confirm(addr);
    if(status != 0) BLE_LOG_W("Indicate request confirm fail %08lX", status);
}

static void ble_receiver_send_confirm(
    BleReceiverContext* instance,
    BleReceiverConfirmHandler handler,
    uint8_t type) {
    handler(instance->peer_addr, type);
    furi_semaphore_release(instance->receive_sem);
    instance->rx_pending_handle = 0;
    instance->rx_pending_cccd = 0;
}

bool ble_receiver_process_write_request(
    BleReceiverContext* instance,
    BleServiceObject* service,
    const uint8_t char_index,
    const uint16_t handle,
    const size_t data_size,
    const void* data) {
    if(furi_semaphore_acquire(instance->receive_sem, BLE_RX_TIMEOUT_MS) == FuriStatusOk) {
        bool send_response = ble_service_write_char_data_or_cccd_by_handle(
            service, char_index, handle, data, data_size);

        if(send_response) {
            ble_receiver_send_confirm(instance, ble_receiver_confirm_write, 0);
        } else {
            instance->rx_pending_handle = handle;
        }

        return true;
    } else {
        BLE_LOG_W("receive_sem timeout!");
        return false;
    }
}

void ble_receiver_transfer_confirm(
    BleReceiverContext* instance,
    uint16_t handle,
    uint8_t cccd_value) {
    if(instance->enabled) {
        bool indication = BLE_CCCD_INDICATION_ENABLED(cccd_value);
        BleReceiverConfirmHandler handler = indication ? ble_receiver_confirm_indicate :
                                                         ble_receiver_confirm_write;

        furi_assert(handle == instance->rx_pending_handle);
        ble_receiver_send_confirm(instance, handler, 0);
    } else {
        instance->rx_pending_cccd = cccd_value;
    }
}
