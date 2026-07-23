#include "ble_transmitter_i.h"

#include "ble_transmitter_set.h"
#include "ble_transmitter_indicate.h"

#define TAG "BleTransmitter"

typedef BleTransmitterGeneric* (*BleTransmitterHandlerAlloc)();
typedef void (*BleTransmitterHandlerFree)(BleTransmitterGeneric* instance);

typedef bool (*BleTransmitterHandlerSend)(
    BleTransmitterGeneric* instance,
    const uint8_t* dev_addr,
    const uint16_t handle,
    const uint16_t data_size,
    const uint8_t* data);

typedef void (*BleTransmitterHandlerSendDone)(BleTransmitterGeneric* instance);

typedef void (*BleTransmitterHandlerReset)(BleTransmitterGeneric* instance);

typedef void (*BleTransmitterHandlerEventLoopSubscribe)(
    BleTransmitterGeneric* instance,
    FuriEventLoop* event_loop,
    void* context);

typedef void (*BleTransmitterHandlerEventLoopUnsubscribe)(
    BleTransmitterGeneric* instance,
    FuriEventLoop* event_loop);

typedef struct {
    BleTransmitterHandlerAlloc alloc;
    BleTransmitterHandlerFree free;
    BleTransmitterHandlerSend send;
    BleTransmitterHandlerSendDone done;
    BleTransmitterHandlerReset reset;
    BleTransmitterHandlerEventLoopSubscribe subscribe;
    BleTransmitterHandlerEventLoopUnsubscribe unsubscribe;
} BleTransmitterContract;

static const BleTransmitterContract ble_transmitters[BleTransmitterTypeCount] = {
    [BleTransmitterTypeIndication] =
        {
            .alloc = ble_transmitter_indicate_alloc,
            .free = ble_transmitter_indicate_free,
            .send = ble_transmitter_indicate_chunk,
            .done = ble_transmitter_indicate_done,
            .reset = ble_transmitter_indicate_reset,
            .subscribe = NULL,
            .unsubscribe = NULL,
        },
    [BleTransmitterTypeSet] =
        {
            .alloc = ble_transmitter_set_alloc,
            .free = ble_transmitter_set_free,
            .send = ble_transmitter_set_chunk,
            .done = ble_transmitter_set_more_data,
            .reset = ble_transmitter_set_reset,
            .subscribe = ble_transmitter_set_subscribe,
            .unsubscribe = ble_transmitter_set_unsubscribe,
        },
};

BleTransmitter* ble_transmitter_alloc() {
    BleTransmitter* instance = malloc(sizeof(BleTransmitter));

    for(uint8_t i = 0; i < BleTransmitterTypeCount; i++) {
        furi_assert(ble_transmitters[i].alloc);
        instance->context[i] = ble_transmitters[i].alloc();
    }

    return instance;
}

void ble_transmitter_free(BleTransmitter* instance) {
    furi_assert(instance);

    for(uint8_t i = 0; i < BleTransmitterTypeCount; i++) {
        furi_assert(ble_transmitters[i].free);
        ble_transmitters[i].free(instance->context[i]);
    }
}

bool ble_transmitter_send_chunk(
    BleTransmitter* instance,
    const uint8_t* dev_addr,
    uint16_t handle,
    uint16_t data_size,
    const uint8_t* data,
    uint16_t cccd_value) {
    furi_assert(instance);

    BleTransmitterType type = BLE_CCCD_INDICATION_ENABLED(cccd_value) ?
                                  BleTransmitterTypeIndication :
                                  BleTransmitterTypeSet;
    return ble_transmitters[type].send(instance->context[type], dev_addr, handle, data_size, data);
}

static inline void ble_transmitter_done(BleTransmitter* instance, BleTransmitterType type) {
    if(ble_transmitters[type].done) {
        ble_transmitters[type].done(instance->context[type]);
    } else {
        BLE_LOG_W("Type: %d done not implemented", type);
    }
}

void ble_transmitter_indication_done(BleTransmitter* instance) {
    furi_assert(instance);
    ble_transmitter_done(instance, BleTransmitterTypeIndication);
}

void ble_transmitter_need_more_data(BleTransmitter* instance) {
    furi_assert(instance);
    ble_transmitter_done(instance, BleTransmitterTypeSet);
}

void ble_transmitter_reset(BleTransmitter* instance) {
    furi_assert(instance);
    for(uint8_t i = 0; i < BleTransmitterTypeCount; i++) {
        if(ble_transmitters[i].reset == NULL) continue;
        ble_transmitters[i].reset(instance->context[i]);
    }
}

void ble_transmitter_subscribe(BleTransmitter* instance, FuriEventLoop* event_loop, void* context) {
    furi_assert(instance);
    furi_assert(event_loop);
    furi_assert(context);
    for(uint8_t i = 0; i < BleTransmitterTypeCount; i++) {
        if(ble_transmitters[i].subscribe == NULL) continue;
        ble_transmitters[i].subscribe(instance->context[i], event_loop, context);
    }
}

void ble_transmitter_unsubscribe(BleTransmitter* instance, FuriEventLoop* event_loop) {
    furi_assert(instance);
    furi_assert(event_loop);
    for(uint8_t i = 0; i < BleTransmitterTypeCount; i++) {
        if(ble_transmitters[i].unsubscribe == NULL) continue;
        ble_transmitters[i].unsubscribe(instance->context[i], event_loop);
    }
}

void ble_transmitter_enable_notifications(BleTransmitter* instance) {
    furi_assert(instance);
    BLE_LOG_I("Notifications enabled");
    ble_transmitter_set_enable(instance->context[BleTransmitterTypeSet]);
}
