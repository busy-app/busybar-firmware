#include "ble_device.h"

#include "ble_advertise.h"
#include "ble_service_registry.h"
#include "../transport/ble_receiver.h"

#include "../_nwp_callbacks/ble_nwp_headers.h"
#include "../../ble_common.h"

#define TAG "BleDevice"

#define BLE_MAX_MTU_SIZE     (240)
#define BLE_ATTR_HEADER_SIZE (3)

#define BLE_DEVICE_MITM_REQ          (1)
#define BLE_DEVICE_SMP_IO_CAPABILITY (0x03)

struct BleDevice {
    BleDeviceBase* base;
    BleDeviceState state;

    uint16_t mtu_size;
    uint16_t max_payload_size; // calculate from mtu

    BleConnectionContext* connection;
    BleDeviceBase* peer;
    BleServiceRegistry* registry;

    BleSecurityData* security_data;
    BleAdvertiseContext* advertise;
    BleTransmitter* transmitter;
    BleReceiverContext* receiver;
};

BleDevice* ble_device_alloc(BleTransmitter* transmitter) {
    furi_assert(transmitter);
    BleDevice* instance = malloc(sizeof(BleDevice));
    instance->transmitter = transmitter;
    instance->state = BleDeviceStateIdle;
    instance->base = ble_device_base_alloc(BleDeviceRoleRemote);
    instance->registry = ble_service_registry_alloc();

    ble_device_set_mtu(instance, BLE_MAX_MTU_SIZE);

    instance->security_data = ble_security_alloc();
    if(!ble_security_init(instance->security_data)) {
        BLE_LOG_W("Device not paired");
    }

    uint8_t addr_buf[BLE_DEVICE_ADDRESS_LEN] = {0};
    sl_status_t status = rsi_bt_get_local_device_address(addr_buf);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Get local device address failed = 0x%08lx", status);
        instance->state = BleDeviceStateError;
    } else {
        ble_device_base_set_address(instance->base, BleDeviceAddressTypeOrigin, addr_buf);
    }

    ///We need this for advertising module. Without this, advertise doesn't want to work
    status = rsi_ble_set_random_address_with_value(addr_buf);
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
    ble_service_registry_free(instance->registry);
    ble_advertise_free(instance->advertise);
    ble_security_free(instance->security_data);
    ble_device_base_free(instance->base);
    free(instance);
}

bool ble_device_register_service(BleDevice* instance, BleServiceObject* service) {
    furi_assert(instance);
    furi_assert(service);
    return ble_service_registry_add_service_entry(instance->registry, service);
}

BleConnectionContext* ble_device_get_connection_context(BleDevice* instance) {
    furi_assert(instance);
    return instance->connection;
}

bool ble_device_is_connected(BleDevice* instance) {
    return instance->connection != NULL;
}

bool ble_device_connection_open(
    BleDevice* instance,
    BleDeviceAddressType type,
    const uint8_t* peer_addr) {
    furi_assert(instance);
    furi_assert(peer_addr);

    bool result = false;
    if(instance->state == BleDeviceStateConnected) {
        BLE_LOG_W("Already connected with remote");
    } else {
        instance->connection = ble_connection_alloc(type, peer_addr);
        instance->receiver = ble_receiver_alloc(peer_addr);
        instance->state = BleDeviceStateConnected;
        instance->peer = ble_connection_get_peer(instance->connection);
        result = true;
    }

    return result;
}

bool ble_device_connection_close(BleDevice* instance) {
    furi_assert(instance);
    bool result = false;

    if(instance->state == BleDeviceStateIdle) {
        BLE_LOG_W("Already disconnected");
    } else {
        ble_transmitter_reset(instance->transmitter);
        ble_connection_free(instance->connection);
        ble_receiver_free(instance->receiver);
        instance->connection = NULL;
        instance->receiver = NULL;

        instance->state = BleDeviceStateIdle;
        ble_service_registry_reset_cccds(instance->registry);

        result = ble_device_start(instance);
    }

    return result;
}

bool ble_device_disconnect(BleDevice* instance) {
    furi_assert(instance);

    bool result = false;
    if(instance->state == BleDeviceStateConnected) {
        const uint8_t* addr =
            ble_device_base_get_address(instance->peer, BleDeviceAddressTypeOrigin);

        sl_status_t status = rsi_ble_disconnect((const int8_t*)addr);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("Failed to disconnect, error code : 0x%08lx", status);
        }

        result = status == RSI_SUCCESS;
    } else if(instance->state != BleDeviceStateError) {
        result = true;
    }
    return result;
}

static bool ble_device_start_advertise_with_value(
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

static void ble_device_start_advertise(BleDevice* instance) {
    const rsi_bt_event_le_security_keys_t* rpa =
        ble_security_get_rpa_data(instance->security_data);
    bool is_paired = ble_device_is_paired(instance);
    instance->state = ble_device_start_advertise_with_value(is_paired, rpa, instance->advertise) ?
                          BleDeviceStateAdvertising :
                          BleDeviceStateError;
}

static bool ble_device_stop_advertise(BleDevice* instance) {
    bool result = false;

    if(instance->state == BleDeviceStateAdvertising) {
        sl_status_t status = rsi_ble_stop_advertising();

        if(status != RSI_SUCCESS) {
            BLE_LOG_W("Failed to stop advertising, error code : 0x%08lx", status);
        }

        result = status == RSI_SUCCESS;
    } else if(instance->state == BleDeviceStateConnected) {
        BLE_LOG_W("Skip stop advertising, device is connected");
        result = true;
    }
    return result;
}

bool ble_device_start(BleDevice* instance) {
    furi_assert(instance);

    if(instance->state == BleDeviceStateIdle) {
        ble_device_start_advertise(instance);
    } else {
        BLE_LOG_W("BLE not in Idle state, skip start");
    }

    return instance->state == BleDeviceStateAdvertising;
}

bool ble_device_stop(BleDevice* instance) {
    furi_assert(instance);

    bool result = false;
    do {
        if(!ble_device_disconnect(instance)) break;
        if(!ble_device_stop_advertise(instance)) break;

        instance->state = BleDeviceStateIdle;

        result = true;
    } while(false);
    return result;
}

void ble_device_set_mtu(BleDevice* instance, uint16_t mtu) {
    furi_assert(instance);

    instance->mtu_size = mtu;
    instance->max_payload_size = instance->mtu_size - BLE_ATTR_HEADER_SIZE;
    BLE_LOG_I("MTU size: %u, max payload size: %u", mtu, instance->max_payload_size);
}

void ble_device_set_name(BleDevice* instance, const char* name) {
    furi_assert(instance);

    BleDeviceState prev_state = instance->state;

    if(prev_state == BleDeviceStateAdvertising) {
        ble_device_stop_advertise(instance);
    }

    ble_advertise_set_name(instance->advertise, name);

    if(prev_state == BleDeviceStateAdvertising) {
        ble_device_start_advertise(instance);
    }
    // ble_advertise_print_data(instance->advertise);
}

BleAdvertiseContext* ble_device_get_advertise_context(BleDevice* instance) {
    furi_assert(instance);
    return instance->advertise;
}

bool ble_device_is_paired(BleDevice* instance) {
    furi_assert(instance);
    return ble_security_pairing_present(instance->security_data);
}

bool ble_device_forget_paired(BleDevice* instance) {
    furi_assert(instance);

    BleDeviceState prev_state = instance->state;
    if(prev_state == BleDeviceStateAdvertising) {
        ble_device_stop_advertise(instance);
    }

    ble_security_rpa_disable();

    bool result = ble_security_delete_data(instance->security_data);

    if(prev_state == BleDeviceStateAdvertising) {
        ble_device_start_advertise(instance);
    }

    if(result) BLE_LOG_I("Security data removed");
    return result;
}

BleSecurityData* ble_device_get_security_data(BleDevice* instance) {
    furi_assert(instance);
    return instance->security_data;
}

void ble_device_response_pairing_capabilities(BleDevice* instance) {
    furi_assert(instance);

    const uint8_t* addr = ble_device_base_get_address(instance->peer, BleDeviceAddressTypeOrigin);
    sl_status_t status = rsi_ble_smp_pair_response(
        (uint8_t*)addr, BLE_DEVICE_SMP_IO_CAPABILITY, BLE_DEVICE_MITM_REQ);

    if(status != SL_STATUS_OK) {
        BLE_LOG_W("Failed to send pairing capabilities: %lX", status);
    }
}

void ble_device_request_pairing(BleDevice* instance) {
    furi_assert(instance);
    BLE_LOG_I("Request pairing...");
    const uint8_t* addr = ble_device_base_get_address(instance->peer, BleDeviceAddressTypeOrigin);
    sl_status_t status = rsi_ble_smp_pair_request(
        (uint8_t*)addr, BLE_DEVICE_SMP_IO_CAPABILITY, BLE_DEVICE_MITM_REQ);

    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Request pairing failed: %08lX", status);
    }
}

static bool ble_device_send_encryption_resp(
    const uint8_t* addr,
    uint8_t response_type,
    const uint8_t* localltk) {
    sl_status_t status = rsi_ble_ltk_req_reply((uint8_t*)addr, response_type, localltk);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Encryption response failed: %08lX", status);
    }
    return status == RSI_SUCCESS;
}

bool ble_device_send_encryption_response(BleDevice* instance) {
    furi_assert(instance);

    uint8_t response_type = 0;
    const uint8_t* local_ltk = NULL;

    if(ble_security_pairing_present(instance->security_data)) {
        const rsi_bt_event_encryption_enabled_t* encrypt_keys =
            ble_security_get_pairing_data(instance->security_data);

        response_type = (1 | encrypt_keys->enabled | (encrypt_keys->sc_enable << 7));
        local_ltk = encrypt_keys->localltk;
    } else {
        BLE_LOG_I("Not paired device");
    }

    const uint8_t* addr = ble_device_base_get_address(instance->peer, BleDeviceAddressTypeOrigin);
    return ble_device_send_encryption_resp(addr, response_type, local_ltk);
}

void ble_device_handle_encryption_start(
    BleDevice* instance,
    rsi_bt_event_encryption_enabled_t* encryption_data) {
    if(!ble_security_pairing_present(instance->security_data)) {
        ble_security_set_pairing_data(instance->security_data, encryption_data);
        if(ble_security_save_data(instance->security_data))
            BLE_LOG_I("Security data saved");
        else
            BLE_LOG_W("Failed to save Security");
    } else
        BLE_LOG_I("Encryption start with previously paired device");
}

void ble_device_send_data(
    BleDevice* instance,
    uint16_t handle,
    uint16_t data_size,
    const uint8_t* data,
    uint16_t cccd_value) {
    size_t index = 0;
    size_t total_size = data_size;

    if(instance->state != BleDeviceStateConnected) return;

    const uint8_t* addr = ble_device_base_get_address(instance->peer, BleDeviceAddressTypeOrigin);
    while(total_size) {
        size_t send_size = total_size > instance->max_payload_size ? instance->max_payload_size :
                                                                     total_size;

        bool send_result = ble_transmitter_send_chunk(
            instance->transmitter, addr, handle, send_size, &data[index], cccd_value);

        if(!send_result) {
            BLE_LOG_W("[%04X] - Tx terminated!", handle);
            break;
        }

        index += send_size;
        total_size -= send_size;
    }
}

bool ble_device_process_write_request(
    BleDevice* instance,
    const uint8_t* remote_addr,
    const uint16_t handle,
    const size_t data_size,
    const void* data) {
    UNUSED(remote_addr);
    furi_assert(instance);

    const BleServiceRegistryEntry* entry =
        ble_service_registry_get_service_entry(instance->registry, handle);

    bool result = false;
    if(entry) {
        result = ble_receiver_process_write_request(
            instance->receiver, entry->service, entry->char_index, handle, data_size, data);
    } else {
        BLE_LOG_W("Not found: %04X", handle);
        // ble_receiver_transfer_confirm(BleReceiverContext *instance, uint16_t handle, uint8_t cccd_value)
        // sl_status_t status = ble_worker_write_response(instance->remote_dev_address, 0);
    }

    return result;
}

void ble_device_receive_confirm(BleDevice* instance, uint16_t handle, uint8_t cccd_value) {
    if(instance->state == BleDeviceStateConnected) {
        ble_receiver_transfer_confirm(instance->receiver, handle, cccd_value);
    }
}

static inline bool ble_device_gatt_read_response(
    uint8_t* dev_addr,
    uint8_t read_type,
    uint16_t handle,
    uint16_t offset,
    uint16_t length,
    const uint8_t* p_data) {
    sl_status_t status =
        rsi_ble_gatt_read_response(dev_addr, read_type, handle, offset, length, p_data);

    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Read response failed status: %08lX", status);
    }
    return status == RSI_SUCCESS;
}

bool ble_device_process_read_request(
    BleDevice* instance,
    uint8_t* addr,
    uint8_t type,
    uint16_t handle,
    uint16_t offset) {
    furi_assert(instance);

    const BleServiceRegistryEntry* entry =
        ble_service_registry_get_service_entry(instance->registry, handle);

    bool result = false;
    if(entry) {
        BleServiceObject* service = entry->service;
        if(ble_service_lock(service)) {
            BleCharacteristicObject* ch = service->chars[entry->char_index];
            size_t data_size = ble_characteristic_get_data_size(ch);
            const void* data = ble_characteristic_get_data(ch);

            if(offset < data_size) {
                result = ble_device_gatt_read_response(
                    addr, type, handle, offset, data_size - offset, data + offset);
            } else {
                BLE_LOG_W("Wrong offset");
            }

            ble_service_unlock(service);
        }
    }
    return result;
}
