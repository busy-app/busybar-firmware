#pragma once

#include <furi.h>
#include "../../service/target/ble_service_target.h"

typedef struct BleReceiverContext BleReceiverContext;

BleReceiverContext* ble_receiver_alloc(const uint8_t* peer_addr);

void ble_receiver_free(BleReceiverContext* instance);

bool ble_receiver_process_write_request(
    BleReceiverContext* instance,
    BleServiceObject* service,
    const uint8_t char_index,
    const uint16_t handle,
    const size_t data_size,
    const void* data);

void ble_receiver_transfer_confirm(
    BleReceiverContext* instance,
    uint16_t handle,
    uint8_t cccd_value);
