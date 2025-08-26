#include "ble_service_target.h"
#include "../../worker/ble_worker.h"

#define TAG "BleService917"

bool ble_service_target_init(BleServiceObject* instance) {
    BLE_LOG_D("%s - ble_service_target_init", instance->desc->name);

    BleServiceState state = BleServiceStateReady;

    const BleIntercomFrameServiceConfig* frame =
        (BleIntercomFrameServiceConfig*)instance->frame_buf;

    // if(instance->desc->init(instance))

    const BleIntercomServiceData* service_config = &frame->service_init;
    BLE_LOG_D("%s - config char_count: %d", instance->desc->name, service_config->char_count);
    uint8_t offset = 0;

    for(size_t i = 0; i < service_config->char_count; i++) {
        const BleCharacteristicData* char_init =
            (BleCharacteristicData*)((uint8_t*)service_config->chars_config + offset);
        size_t data_size = char_init->header.data_size;

        BLE_LOG_D(
            "%s - char: %d data_size: %d",
            instance->desc->name,
            char_init->header.index,
            data_size);
        BleCharacteristicObject* ch = instance->chars[char_init->header.index];
        ble_characteristic_set_data(ch, char_init->data, data_size);

        offset += (data_size + sizeof(BleCharacteristicDataHeader));
    }

    if(ble_worker_register_service(instance)) {
        ble_service_switch_state(instance, state);
    }

    ble_service_prepare_frame(
        instance, BleIntercomFrameTypeResponse, BleCommandServiceInit, 0, NULL);

    return true;
}

bool ble_service_target_process_response(BleServiceObject* instance) {
    UNUSED(instance);
    return true;
}

void ble_service_target_notify(
    BleServiceObject* instance,
    uint8_t ch_index,
    void* data,
    size_t data_size) {
    BLE_LOG_D("%s - ble_service_target_notify", instance->desc->name);
    BleCharacteristicObject* ch = instance->chars[ch_index];
    ble_characteristic_set_data(ch, data, data_size);
    const uint16_t handle = ble_characteristic_get_handle(ch);
    ble_worker_notify(handle, data_size, data);
}
