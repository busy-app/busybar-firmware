#pragma once

#include "ble_transmitter_i.h"

BleTransmitterGeneric* ble_transmitter_indicate_alloc();
void ble_transmitter_indicate_free(BleTransmitterGeneric* transport);

bool ble_transmitter_indicate_chunk(
    BleTransmitterGeneric* transport,
    const uint8_t* dev_addr,
    const uint16_t handle,
    const uint16_t data_size,
    const uint8_t* data);

void ble_transmitter_indicate_done(BleTransmitterGeneric* transport);
