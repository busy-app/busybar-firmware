#include "ble_connection.h"

#include "ble_device_common.h"

// typedef BleDeviceCommon BlePeerDevice;

typedef struct {
    uint16_t interval;
    uint16_t latency;
    uint16_t timeout;
} BleConnectionTimings;

typedef struct {
    /**Maximum TX Octets to be transmitted*/
    uint16_t MaxTxOctets;
    /**Maximum TX time to transmit the MaxTxOctets*/
    uint16_t MaxTxTime;
    /**Maximum Rx Octets to be received*/
    uint16_t MaxRxOctets;
    /**Maximum Rx time to receive the MaxRxOctets*/
    uint16_t MaxRxTime;
} BleConnectionDataLength;

typedef union {
    struct {
        bool phy_le_1m    : 1;
        bool phy_le_2m    : 1;
        bool phy_le_coded : 1;
        uint8_t reserved  : 5;
    } flags;
    uint8_t value;
} BlePhy;

// typedef struct {
//     uint8_t dev_addr[BLE_DEVICE_ADDRESS_LEN];
//     uint8_t features[8];

// } BlePeerDevice;

struct BleConnectionContext {
    BleConnectionTimings timings;
    BleConnectionDataLength data_length_params;
    BlePhy TxPhy;
    BlePhy RxPhy;

    BleDeviceCommon* peer;
};

BleConnectionContext* ble_connection_alloc(const uint8_t* const peer_address) {
    furi_assert(peer_address);

    BleConnectionContext* instance = malloc(sizeof(BleConnectionContext));
    instance->peer = malloc(sizeof(BleDeviceCommon));
    memcpy(instance->peer->dev_addr, peer_address, BLE_DEVICE_ADDRESS_LEN);

    return instance;
}

void ble_connection_free(BleConnectionContext* instance) {
    furi_assert(instance);

    free(instance->peer);
    free(instance);
}

const uint8_t* ble_connection_get_peer_address(BleConnectionContext* instance) {
    furi_assert(instance);
    return instance->peer->dev_addr;
}
