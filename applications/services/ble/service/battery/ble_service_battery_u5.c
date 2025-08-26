#include "ble_service_battery_i.h"

#include "../../../power/power_service/power.h"

#define TAG "BleBatteryU5"

typedef struct {
    PowerEventType event_type;
    FuriThread* thr;
} BleBatteryServiceContext;

void ble_service_prepare_char_frame(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    uint8_t command_event,
    uint8_t ch_index,
    size_t data_size,
    const void* data) {
    BleIntercomFrameGeneric* frame = (BleIntercomFrameGeneric*)instance->output;
    frame->header.frame_type = frame_type;
    frame->header.command = command_event;
    frame->header.service_index = instance->desc->index;
    frame->header.data_size = sizeof(BleCharacteristicDataHeader) + data_size;

    BleCharacteristicData* ch_data = (BleCharacteristicData*)frame->data;
    ch_data->header.index = ch_index;
    ch_data->header.data_size = data_size;
    memcpy(ch_data->data, data, data_size);
}

static void ble_power_pubsub_message_callback(const void* message, void* context) {
    BLE_LOG_D("Battery event!");

    BleServiceObject* instance = context;
    const PowerEvent* event = message;

    if(ble_service_lock(instance)) {
        BleBatteryServiceContext* data_ctx = instance->data_context;
        data_ctx->event_type = event->type;
        furi_thread_flags_set(furi_thread_get_id(data_ctx->thr), 1);
        ble_service_unlock(instance);
    }
}

static int32_t bat_test_thread(void* context) {
    BleServiceObject* instance = context;

    while(1) {
        furi_thread_flags_wait(1, FuriFlagWaitAny, FuriWaitForever);

        if(ble_service_lock(instance)) {
            BleBatteryServiceContext* data_ctx = instance->data_context;
            BleCharacteristicObject* ch_bat_lvl =
                instance->chars[BleSrvBatteryCharacterIndexBatteryLevel];
            BleCharacteristicObject* ch_bat_status =
                instance->chars[BleSrvBatteryCharacterIndexBatteryStatus];

            BatteryStatusInfo battery_status = {0};
            memcpy(
                &battery_status,
                ble_characteristic_get_data(ch_bat_status),
                sizeof(BatteryStatusInfo));

            if(data_ctx->event_type == PowerEventBatteryPresent) {
                battery_status.state.fields.battery_present = 1;
            } else if(data_ctx->event_type == PowerEventBatteryPresent) {
                battery_status.state.fields.battery_present = 0;
            } else if(data_ctx->event_type == PowerEventChargeAmountUpdate) {
                PowerInfo info = {0};
                Power* power = furi_record_open(RECORD_POWER);
                power_get_info(power, &info);
                furi_record_close(RECORD_POWER);
                BLE_LOG_D("Charge: %d charge_enabled: %d", info.charge, info.charge_enabled);
                // data_ctx->battery_level = info.charge;

                battery_status.state.fields.wired_source_present = info.is_charging;
                battery_status.state.fields.wireless_source_present = 0;
                battery_status.state.fields.battery_charge_state = info.is_charging ? 1 : 2;
                battery_status.state.fields.battery_charge_level = 1;
                battery_status.state.fields.charging_type = 2;
                battery_status.state.fields.charging_fault_reason = 0;

                ble_characteristic_set_data(ch_bat_lvl, &info.charge, sizeof(info.charge));
                ble_characteristic_set_data(
                    ch_bat_status, &battery_status, sizeof(BatteryStatusInfo));
            }

            ///TDOO: All this code must be moved to common layers
            const void* data = ble_characteristic_get_data(ch_bat_lvl);
            size_t data_size = ble_characteristic_get_data_size(ch_bat_lvl);

            ble_service_prepare_char_frame(
                instance,
                BleIntercomFrameTypeNotification,
                BleCommandServiceNotify,
                BleSrvBatteryCharacterIndexBatteryLevel,
                data_size,
                data);
            ble_service_send_intercom_frame(instance);

            data = ble_characteristic_get_data(ch_bat_status);
            data_size = ble_characteristic_get_data_size(ch_bat_status);

            ble_service_prepare_char_frame(
                instance,
                BleIntercomFrameTypeNotification,
                BleCommandServiceNotify,
                BleSrvBatteryCharacterIndexBatteryStatus,
                data_size,
                data);
            ble_service_send_intercom_frame(instance);

            ble_service_unlock(instance);
        }
    }
    return 0;
}

bool ble_service_battery_init(void* object) {
    furi_assert(object);

    BLE_LOG_D("battery_init");

    BleServiceObject* instance = object;

    BleBatteryServiceContext* context = malloc(sizeof(BleBatteryServiceContext));
    instance->data_context = context;

    Power* power = furi_record_open(RECORD_POWER);
    PowerInfo info = {0};
    power_get_info(power, &info);

    BleCharacteristicObject* ch = instance->chars[BleSrvBatteryCharacterIndexBatteryLevel];

    ble_characteristic_set_data(ch, &info.charge, sizeof(info.charge));

    BatteryStatusInfo battery_status = {0};

    battery_status.flags = 0;
    battery_status.state.fields.battery_present = 1;
    battery_status.state.fields.wired_source_present = info.is_charging;
    battery_status.state.fields.wireless_source_present = 0;
    battery_status.state.fields.battery_charge_state = info.is_charging ? 1 : 2;
    battery_status.state.fields.battery_charge_level = 1;
    battery_status.state.fields.charging_type = 2;
    battery_status.state.fields.charging_fault_reason = 0;

    ch = instance->chars[BleSrvBatteryCharacterIndexBatteryStatus];
    ble_characteristic_set_data(ch, &battery_status, sizeof(BatteryStatusInfo));

    furi_pubsub_subscribe(power_get_pubsub(power), ble_power_pubsub_message_callback, instance);

    furi_record_close(RECORD_POWER);

    context->thr = furi_thread_alloc_ex("BatTest", 1024, bat_test_thread, instance);
    furi_thread_start(context->thr);

    return true;
}
