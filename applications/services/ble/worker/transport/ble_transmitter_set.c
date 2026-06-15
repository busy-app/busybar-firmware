
#include "ble_transmitter_i.h"

#define BLE_TX_QUEUE_SIZE            (20)
#define BLE_TX_QUEUE_PUT_TIMEOUT     (300)
#define BLE_TRANSMIT_FAILURE_TIMEOUT (500)

typedef struct {
    FuriMessageQueue* tx_queue;
    FuriSemaphore* more_data_sem;
    bool send_buffer_error;
    bool enabled;
} BleTransmitterSetContext;

typedef struct {
    uint16_t handle;
    size_t data_size;
    uint8_t remote_dev_address[6];
} BleDataHeader;

typedef struct {
    BleDataHeader header;
    uint8_t data[];
} BleDataItem;

typedef BleDataItem* BleDataItemPtr;

#define TAG "BleTransmitterSet"

static bool
    ble_transmitter_send_item(BleTransmitterSetContext* instance, const BleDataItemPtr item) {
    bool result = false;
    do {
        sl_status_t status = rsi_ble_notify_value(
            item->header.remote_dev_address,
            item->header.handle,
            item->header.data_size,
            item->data);

        if(status == RSI_SUCCESS) {
            result = true;
            break;
        }

        if((int32_t)status != RSI_ERROR_BLE_DEV_BUF_FULL) {
            BLE_LOG_W("Notify error: %08lX", status);
            break;
        }

        if(furi_semaphore_acquire(instance->more_data_sem, BLE_TRANSMIT_FAILURE_TIMEOUT) !=
           FuriStatusOk) {
            BLE_LOG_W("Notify timeout");
            instance->send_buffer_error = true;
            break;
        }

    } while(true);

    return result;
}

static void ble_transmitter_tx_queue_handler(FuriEventLoopObject* object, void* context) {
    UNUSED(object);
    furi_assert(context);
    BleTransmitterSetContext* instance = context;

    BleDataItemPtr item = NULL;
    while(furi_message_queue_get(instance->tx_queue, &item, 0) == FuriStatusOk) {
        if(!instance->send_buffer_error) {
            ble_transmitter_send_item(instance, item);
        }
        free(item);
        item = NULL;
    }
}

void ble_transmitter_set_enable(BleTransmitterGeneric* transport) {
    BleTransmitterSetContext* instance = transport;
    instance->enabled = true;
}

bool ble_transmitter_set_chunk(
    BleTransmitterGeneric* transport,
    const uint8_t* dev_addr,
    const uint16_t handle,
    const uint16_t data_size,
    const uint8_t* data) {
    BleTransmitterSetContext* instance = transport;

    if(!instance->enabled) {
        BLE_LOG_W("Notification drop");
        return false;
    }

    BleDataItemPtr item = malloc(sizeof(BleDataHeader) + data_size);
    item->header.data_size = data_size;
    item->header.handle = handle;
    memcpy(item->header.remote_dev_address, dev_addr, sizeof(item->header.remote_dev_address));
    memcpy(item->data, data, data_size);

    FuriStatus status =
        furi_message_queue_put(instance->tx_queue, &item, BLE_TX_QUEUE_PUT_TIMEOUT);

    if(status != FuriStatusOk) {
        BLE_LOG_W("[%04X] - failed to put in queue", handle);
        free(item);
    }
    return status == FuriStatusOk;
}

void ble_transmitter_set_reset(BleTransmitterGeneric* transport) {
    BleTransmitterSetContext* instance = transport;

    BleDataItemPtr item;
    while(furi_message_queue_get(instance->tx_queue, &item, 0) == FuriStatusOk) {
        free(item);
    }
    instance->enabled = false;
    instance->send_buffer_error = false;
}

BleTransmitterGeneric* ble_transmitter_set_alloc() {
    BleTransmitterSetContext* instance = malloc(sizeof(BleTransmitterSetContext));

    instance->more_data_sem = furi_semaphore_alloc(1, 0);
    instance->tx_queue = furi_message_queue_alloc(BLE_TX_QUEUE_SIZE, sizeof(BleDataItemPtr));

    return instance;
}

void ble_transmitter_set_free(BleTransmitterGeneric* transport) {
    furi_assert(transport);

    BleTransmitterSetContext* instance = transport;
    furi_semaphore_free(instance->more_data_sem);
    furi_message_queue_free(instance->tx_queue);
    free(instance);
}

void ble_transmitter_set_more_data(BleTransmitterGeneric* transport) {
    furi_assert(transport);
    BleTransmitterSetContext* instance = transport;

    furi_semaphore_release(instance->more_data_sem);
    if(instance->send_buffer_error) {
        instance->send_buffer_error = false;
        BLE_LOG_W("Restore more data");
    }
}

void ble_transmitter_set_subscribe(
    BleTransmitterGeneric* transport,
    FuriEventLoop* event_loop,
    void* context) {
    UNUSED(context);
    BleTransmitterSetContext* instance = transport;
    furi_event_loop_subscribe_message_queue(
        event_loop,
        instance->tx_queue,
        FuriEventLoopEventIn,
        ble_transmitter_tx_queue_handler,
        instance);
}

void ble_transmitter_set_unsubscribe(BleTransmitterGeneric* transport, FuriEventLoop* event_loop) {
    BleTransmitterSetContext* instance = transport;
    furi_event_loop_unsubscribe(event_loop, instance->tx_queue);
}
