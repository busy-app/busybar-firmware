#include "ble_connection.h"

// typedef struct {
//     uint8_t dev_addr[BLE_DEVICE_ADDRESS_LEN];
//     uint8_t features[8];

// } BlePeerDevice;

struct BleConnectionContext {
    BleConnectionTimings timings;
    BleConnectionDataLength data_length_params;
    BlePhy TxPhy;
    BlePhy RxPhy;

    BleDeviceBase* peer;
};

BleConnectionContext*
    ble_connection_alloc(BleDeviceAddressType type, const uint8_t* const peer_address) {
    furi_assert(peer_address);

    BleConnectionContext* instance = malloc(sizeof(BleConnectionContext));
    instance->peer = ble_device_base_alloc(BleDeviceRoleCentral);
    ble_device_base_set_address(instance->peer, type, peer_address);

    return instance;
}

// void ble_connection_update_timings(BleConnectionContext* instance) {
//     furi_assert(instance);

// }

void ble_connection_free(BleConnectionContext* instance) {
    furi_assert(instance);

    free(instance->peer);
    free(instance);
}

BleDeviceBase* ble_connection_get_peer(BleConnectionContext* instance) {
    furi_assert(instance);
    return instance->peer;
}

const BleConnectionTimings* ble_connection_get_timings(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->timings;
}

void ble_connection_set_timings(
    BleConnectionContext* instance,
    const BleConnectionTimings* const timings) {
    furi_assert(instance);
    furi_assert(timings);
    memcpy(&instance->timings, timings, sizeof(BleConnectionTimings));
}

const BleConnectionDataLength* ble_connection_get_data_length(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->data_length_params;
}

void ble_connection_set_data_length(
    BleConnectionContext* instance,
    const BleConnectionDataLength* const data_length) {
    furi_assert(instance);
    furi_assert(data_length);
    memcpy(&instance->data_length_params, data_length, sizeof(BleConnectionDataLength));
}

const BlePhy* ble_connection_get_tx_phy(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->TxPhy;
}

void ble_connection_set_tx_phy(BleConnectionContext* instance, const uint8_t value) {
    furi_assert(instance);
    instance->TxPhy.value = value;
}

const BlePhy* ble_connection_get_rx_phy(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->TxPhy;
}

void ble_connection_set_rx_phy(BleConnectionContext* instance, const uint8_t value) {
    furi_assert(instance);
    instance->TxPhy.value = value;
}

// typedef enum {
//     BleConnectionParameterTypeTimingLatency,

// } BleConnectionParameterType;

// typedef struct {
//     BleConnectionParameterType type;
//     size_t data_size;
//     void* data;
// } BleConnectionParameter;

// typedef void (*BleConnectionParamSetter)(void* data, size_t data_size);
// typedef void (*BleConnectionParamGetter)(void* data, size_t data_size);

// void ble_connection_update_parameters(
//     BleConnectionContext* instance,
//     size_t params_count,
//     const BleConnectionParameter* const params) {
//     furi_assert(instance);
//     furi_assert(params_count);
//     furi_assert(params_count);
// }
