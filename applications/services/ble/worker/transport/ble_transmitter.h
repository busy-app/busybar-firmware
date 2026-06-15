#pragma once

#include <furi.h>

typedef struct BleTransmitter BleTransmitter;

BleTransmitter* ble_transmitter_alloc();
void ble_transmitter_free(BleTransmitter* instance);

void ble_transmitter_enable_notifications(BleTransmitter* instance);

bool ble_transmitter_send_chunk(
    BleTransmitter* instance,
    const uint8_t* dev_addr,
    uint16_t handle,
    uint16_t data_size,
    const uint8_t* data,
    uint16_t cccd_value);

void ble_transmitter_reset(BleTransmitter* instance);

void ble_transmitter_indication_done(BleTransmitter* instance);
void ble_transmitter_need_more_data(BleTransmitter* instance);

void ble_transmitter_subscribe(BleTransmitter* instance, FuriEventLoop* event_loop, void* context);
void ble_transmitter_unsubscribe(BleTransmitter* instance, FuriEventLoop* event_loop);
