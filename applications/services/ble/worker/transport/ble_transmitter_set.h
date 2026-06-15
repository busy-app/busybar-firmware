#pragma once

#include "ble_transmitter_i.h"

BleTransmitterGeneric* ble_transmitter_set_alloc();
void ble_transmitter_set_free(BleTransmitterGeneric* transport);

void ble_transmitter_set_enable(BleTransmitterGeneric* transport);

bool ble_transmitter_set_chunk(
    BleTransmitterGeneric* transport,
    const uint8_t* dev_addr,
    const uint16_t handle,
    const uint16_t data_size,
    const uint8_t* data);

void ble_transmitter_set_more_data(BleTransmitterGeneric* transport);

void ble_transmitter_set_reset(BleTransmitterGeneric* transport);

void ble_transmitter_set_subscribe(
    BleTransmitterGeneric* transport,
    FuriEventLoop* event_loop,
    void* context);

void ble_transmitter_set_unsubscribe(BleTransmitterGeneric* transport, FuriEventLoop* event_loop);
