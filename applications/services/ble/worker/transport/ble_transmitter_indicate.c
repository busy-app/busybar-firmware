#include "ble_transmitter_indicate.h"

#include "../../util/ble_canary.h"

#define BLE_WORKER_INDICATE_RETRY_DELAY_MS (100)
#define BLE_WORKER_TX_TIMEOUT_MS           (1000)

#define TAG "BleTransmitterIndicate"

typedef struct {
    uint16_t tx_pending_handle;
    FuriSemaphore* indication_sem;
    BleDebugCanary* indicate_error_canary;
} BleTransmitterIndicateContext;

static inline bool ble_transmitter_indicate_retry(
    BleTransmitterIndicateContext* instance,
    const uint8_t* dev_addr,
    const uint16_t handle,
    const uint16_t data_size,
    const uint8_t* data,
    const uint8_t max_retries) {
    uint8_t retry_count = 0;
    int32_t status;
    do {
        status = rsi_ble_indicate_value(dev_addr, handle, data_size, data);
        if(status == RSI_SUCCESS) break;

        if(status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) {
            furi_delay_ms(BLE_WORKER_INDICATE_RETRY_DELAY_MS);
            ble_debug_canary_test_log(
                instance->indicate_error_canary, TAG, "Indicate retry: %04X", handle);
        }
        retry_count += 1;
    } while((status == RSI_ERROR_BLE_ATT_CMD_IN_PROGRESS) && (retry_count < max_retries));

    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Indication failed: %08lX", status);
    }

    return status == RSI_SUCCESS;
}

bool ble_transmitter_indicate_chunk(
    BleTransmitterGeneric* transport,
    const uint8_t* dev_addr,
    const uint16_t handle,
    const uint16_t data_size,
    const uint8_t* data) {
    furi_assert(transport);
    BleTransmitterIndicateContext* instance = transport;

    bool result = false;

    do {
        const uint8_t indication_retry_count = 4;
        instance->tx_pending_handle = handle;
        if(!ble_transmitter_indicate_retry(
               transport, dev_addr, handle, data_size, data, indication_retry_count))
            break;

        if(furi_semaphore_acquire(instance->indication_sem, BLE_WORKER_TX_TIMEOUT_MS) !=
           FuriStatusOk) {
            BLE_LOG_W("Indicate timeout expired");
            break;
        }

        result = true;
    } while(false);
    return result;
}

static inline void
    ble_transmitter_indicate_release_if_pending(BleTransmitterIndicateContext* instance) {
    if(instance->tx_pending_handle) {
        furi_semaphore_release(instance->indication_sem);
        instance->tx_pending_handle = 0;
    }
}

void ble_transmitter_indicate_reset(BleTransmitterGeneric* transport) {
    furi_assert(transport);
    BleTransmitterIndicateContext* instance = transport;

    ble_transmitter_indicate_done(instance);
    ble_debug_canary_reset(instance->indicate_error_canary);
}

void ble_transmitter_indicate_done(BleTransmitterGeneric* transport) {
    furi_assert(transport);
    BleTransmitterIndicateContext* instance = transport;
    ble_transmitter_indicate_release_if_pending(instance);
}

BleTransmitterGeneric* ble_transmitter_indicate_alloc() {
    BleTransmitterIndicateContext* instance = malloc(sizeof(BleTransmitterIndicateContext));
    instance->indication_sem = furi_semaphore_alloc(1, 0);
    instance->indicate_error_canary = ble_debug_canary_alloc(BleCanaryTypeHitOnce);
    instance->tx_pending_handle = 0;
    return instance;
}

void ble_transmitter_indicate_free(BleTransmitterGeneric* transport) {
    furi_assert(transport);
    BleTransmitterIndicateContext* instance = transport;
    furi_semaphore_free(instance->indication_sem);
    ble_debug_canary_free(instance->indicate_error_canary);
    free(instance);
}
