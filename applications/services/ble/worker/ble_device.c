#include "ble_device.h"

// #include "ble_advertise.h"
// #include "ble_security.h"
#include "ble_connection.h"
#include "ble_device_common.h"

#include "_nwp_callbacks/ble_nwp_headers.h"
#include "../ble_common.h"

#define TAG "BleDevice"

struct BleDevice {
    BleDeviceCommon base;
    BleDeviceState state;

    bool pairing_info_available;
    uint16_t mtu_size;
    uint16_t max_payload_size; // calculate from mtu

    BleConnectionContext* connection;
    // BleServiceEntryDict_t service_dict;

    BleSecurityData* security_data;
    BleAdvertiseContext* advertise;
};

BleDevice* ble_device_alloc(/*BleDeviceType*/) {
    BleDevice* instance = malloc(sizeof(BleDevice));
    instance->state = BleDeviceStateIdle;
    instance->security_data = ble_security_alloc();
    instance->pairing_info_available = ble_security_init(instance->security_data);

    sl_status_t status = rsi_bt_get_local_device_address(instance->base.dev_addr);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Get local device address failed = 0x%08lx", status);
        instance->state = BleDeviceStateError;
    }

    ///We need this for advertising module. Without this, advertise doesn't want to work
    status = rsi_ble_set_random_address_with_value(instance->base.dev_addr);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set address: %08lX", status);
        instance->state = BleDeviceStateError;
    }

    instance->advertise = ble_advertise_alloc();

    ///TODO: Read bsb ble features once we find out how to do that
    //rsi_ble_get_local_features()

    return instance;
}

void ble_device_free(BleDevice* instance) {
    furi_assert(instance);
    ble_security_free(instance->security_data);
    free(instance);
}

///TODO: Maybe Replace by checking state
bool ble_device_is_connected(BleDevice* instance) {
    return instance->connection != NULL;
}

bool ble_device_connect(BleDevice* instance, const uint8_t* const peer_address) {
    furi_assert(instance);
    furi_assert(peer_address);

    bool result = false;
    if(ble_device_is_connected(instance)) {
        BLE_LOG_W("Already connected with remote");
    } else {
        instance->connection = ble_connection_alloc(peer_address);
        result = true;
    }

    return result;
}

bool ble_device_disconnect(BleDevice* instance) {
    furi_assert(instance);

    bool result = false;
    if(!ble_device_is_connected(instance)) {
        BLE_LOG_W("Already disconnected");
    } else {
        ble_connection_free(instance->connection);
        result = true;
    }
    return result;
}

void ble_device_set_name(BleDevice* instance, const char* name) {
    furi_assert(instance);
    //! Set local name
    sl_status_t status = rsi_bt_set_local_name((const uint8_t*)name);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set default local name, error code : 0x%08lx", status);
    }

    ble_advertise_set_name(instance->advertise, name);

    ble_advertise_print_data(instance->advertise);
}

static bool ble_start_advertise(
    bool advertise_to_paired_only,
    const rsi_bt_event_le_security_keys_t* key,
    const BleAdvertiseContext* advertise) {
    rsi_ble_req_adv_t ble_adv = {0};

#ifdef BLE_DEBUG_ADVERTISE_FORCE_PUBLIC
    BLE_LOG_W("Public advertise forced!");
    advertise_to_paired_only = false;
#endif

    ble_adv.status = RSI_BLE_START_ADV;
    ///TODO: This is blocked because it doesn't work on IPhone. It just doesn't see
    ///BSB in case of direct advertise.
    // ble_adv.adv_type = advertise_to_paired_only ? DIR_CONN_LOW_DUTY_CYCLE : UNDIR_CONN;
    ble_adv.adv_type = UNDIR_CONN;

    ble_adv.adv_int_min = RSI_BLE_ADV_INT_MIN;
    ble_adv.adv_int_max = RSI_BLE_ADV_INT_MAX;
    ble_adv.adv_channel_map = RSI_BLE_ADV_CHANNEL_MAP;

    rsi_ble_clear_acceptlist();
    BLE_LOG_W("advertise_to_paired_only = %d", advertise_to_paired_only);
    if(advertise_to_paired_only) {
        rsi_ble_addto_acceptlist((int8_t*)key->Identity_addr, key->Identity_addr_type);
        ble_adv.filter_type = ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST;
        ble_adv.own_addr_type = LE_RESOLVABLE_RANDOM_ADDRESS;
        memcpy(ble_adv.direct_addr, key->Identity_addr, 6);
        ble_adv.direct_addr_type = key->Identity_addr_type;
    } else {
        ble_adv.filter_type = RSI_BLE_ADV_FILTER_TYPE;
        ble_adv.own_addr_type = LE_PUBLIC_ADDRESS;
    }

    ble_advertise_refresh_data(advertise);

    sl_status_t status = rsi_ble_start_advertising_with_values(&ble_adv);

    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to start advertising, error code : 0x%08lx", status);
    } else {
        BLE_LOG_I("Start advertising...");
    }

    return status == RSI_SUCCESS;
}

bool ble_device_start_advertise(BleDevice* instance) {
    furi_assert(instance);

    if(instance->state == BleDeviceStateIdle) {
        const rsi_bt_event_le_security_keys_t* rpa =
            ble_security_get_rpa_data(instance->security_data);
        instance->state =
            ble_start_advertise(instance->pairing_info_available, rpa, instance->advertise) ?
                BleDeviceStateAdvertising :
                BleDeviceStateError;
    } else {
        BLE_LOG_W("BLE not in Idle state, skip advertise start");
    }

    return instance->state == BleDeviceStateAdvertising;
}

// bool ble_device_stop_advertise(BleDevice* instance) {

// }

BleAdvertiseContext* ble_device_get_advertise_context(BleDevice* instance) {
    furi_assert(instance);
    return instance->advertise;
}

bool ble_device_is_paired(BleDevice* instance) {
    furi_assert(instance);
    return instance->pairing_info_available;
}

BleSecurityData* ble_device_get_security_data(BleDevice* instance) {
    furi_assert(instance);
    return instance->security_data;
}
// BleAdvertiseContext* advertise;
