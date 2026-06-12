#include "ble_connection.h"
#include "../_nwp_callbacks/ble_nwp_headers.h"

#include "../../ble_common.h"

#define TAG "BleConnection"

struct BleConnectionContext {
    BleConnectionTimings timings;
    BleConnectionDataLength data_length_params;
    BlePhy TxPhy;
    BlePhy RxPhy;

    BleDeviceBase* peer;
    bool phy_update_done;
    bool length_update_done;
};

BleConnectionContext*
    ble_connection_alloc(BleDeviceAddressType type, const uint8_t* const peer_address) {
    furi_assert(peer_address);

    BleConnectionContext* instance = malloc(sizeof(BleConnectionContext));
    instance->peer = ble_device_base_alloc(BleDeviceRoleCentral);
    ble_device_base_set_address(instance->peer, type, peer_address);

    return instance;
}

void ble_connection_free(BleConnectionContext* instance) {
    furi_assert(instance);

    ble_device_base_free(instance->peer);
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
    instance->length_update_done = true;
}

const BlePhy* ble_connection_get_tx_phy(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->TxPhy;
}

const BlePhy* ble_connection_get_rx_phy(BleConnectionContext* instance) {
    furi_assert(instance);
    return &instance->RxPhy;
}

void ble_connection_set_phy(
    BleConnectionContext* instance,
    const uint8_t tx_phy,
    const uint8_t rx_phy) {
    furi_assert(instance);
    instance->TxPhy.value = tx_phy;
    instance->RxPhy.value = rx_phy;
    instance->phy_update_done = true;
}

void request_2m_phy_retry(const uint8_t* addr) {
    sl_status_t status =
        rsi_ble_setphy((const int8_t*)addr, TX_PHY_RATE, RX_PHY_RATE, CODDED_PHY_RATE);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set phy, error code : 0x%08lx", status);
    } else {
        BLE_LOG_I("PHY set done");
    }
}

bool ble_connection_update_phy_and_data_length_by_timer(BleConnectionContext* instance) {
    furi_assert(instance);

    bool all_updates_done = instance->phy_update_done && instance->length_update_done;

    do {
        if(all_updates_done) {
            BLE_LOG_I("all_updates_done = 1");
            break;
        }

        BleDeviceBase* peer = instance->peer;
        bool Phy2M_supported =
            ble_device_base_is_feature_supported(peer, BleDeviceFeaturesLE2MPhy);

        const uint8_t* addr = ble_device_base_get_address(peer, BleDeviceAddressTypeOrigin);
        if(!instance->phy_update_done && Phy2M_supported) {
            request_2m_phy_retry(addr);
            break;
        }

        bool length_extension_supported = ble_device_base_is_feature_supported(
            peer, BleDeviceFeaturesLEDataPacketLengthExtension);
        if(!instance->length_update_done && length_extension_supported) {
            sl_status_t status = rsi_ble_set_data_len((uint8_t*)addr, TX_LEN, TX_TIME);
            if(status != RSI_SUCCESS) {
                BLE_LOG_W("\n Set data length cmd failed with error code = %lx \n", status);
            } else {
                BLE_LOG_I("Length set done");
            }

            break;
        }

    } while(false);

    return all_updates_done;
}
