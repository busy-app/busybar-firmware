#include "ble_service_battery_i.h"

#include "../../../power/power_service/power.h"

#define TAG "BleBatteryU5"

typedef struct {
    FuriPubSubSubscription* pubsub_subscription;
    uint8_t prev_charge;
    BatteryStatusInfo prev_status;
} BleBatteryServiceContext;

static void ble_power_pubsub_message_callback(const void* message, void* context) {
    UNUSED(message);

    BleServiceObject* instance = context;
    ble_service_enqueue_run(instance);
}

static void ble_service_battery_update(BleServiceObject* instance) {
    PowerInfo info = {0};
    Power* power = furi_record_open(RECORD_POWER);
    power_get_info(power, &info);
    furi_record_close(RECORD_POWER);
    BleBatteryServiceContext* ctx = instance->context;

    if(info.charge != ctx->prev_charge) {
        BleCharacteristicObject* ch = instance->chars[BleSrvBatteryCharacterIndexBatteryLevel];
        ble_characteristic_set_data(ch, &info.charge, sizeof(info.charge));
        ctx->prev_charge = info.charge;
    }

    BatteryStatusInfo battery_status = {0};
    battery_status.flags = 0;
    battery_status.state.fields.battery_present = 1;
    battery_status.state.fields.wired_source_present = info.is_charging;
    battery_status.state.fields.wireless_source_present = 0;
    battery_status.state.fields.battery_charge_state = info.is_charging ? 1 : 2;
    battery_status.state.fields.battery_charge_level = 1;
    battery_status.state.fields.charging_type = 2;
    battery_status.state.fields.charging_fault_reason = 0;

    if(battery_status.state.value != ctx->prev_status.state.value) {
        BleCharacteristicObject* ch = instance->chars[BleSrvBatteryCharacterIndexBatteryStatus];
        ble_characteristic_set_data(ch, &battery_status, sizeof(BatteryStatusInfo));
        ctx->prev_status.flags = 0;
        ctx->prev_status.state.value = battery_status.state.value;
    }
}

bool ble_service_battery_init(void* object) {
    furi_assert(object);
    BleServiceObject* instance = object;

    BleBatteryServiceContext* context = malloc(sizeof(BleBatteryServiceContext));
    instance->context = context;

    ble_service_battery_update(instance);

    Power* power = furi_record_open(RECORD_POWER);
    context->pubsub_subscription = furi_pubsub_subscribe(
        power_get_pubsub(power), ble_power_pubsub_message_callback, instance);
    furi_record_close(RECORD_POWER);

    return true;
}

bool ble_service_battery_run(void* object, size_t data_size, const void* data) {
    UNUSED(data_size);
    UNUSED(data);

    furi_assert(object);
    BleServiceObject* instance = object;

    ble_service_battery_update(instance);
    return true;
}
