#include "ble_service_battery_i.h"

#include "../../../power/power_service/power.h"

#define TAG "BleBatteryU5"

typedef struct {
    PowerEventType event_type;
    FuriPubSubSubscription* pubsub_subscription;
} BleBatteryServiceContext;

static void ble_power_pubsub_message_callback(const void* message, void* context) {
    BleServiceObject* instance = context;
    const PowerEvent* event = message;

    if(ble_service_lock(instance)) {
        BleBatteryServiceContext* ctx = instance->context;
        ctx->event_type = event->type;
        ble_service_enqueue_run(instance);
        ble_service_unlock(instance);
    }
}

static void ble_service_battery_update(BleServiceObject* instance) {
    PowerInfo info = {0};
    Power* power = furi_record_open(RECORD_POWER);
    power_get_info(power, &info);
    furi_record_close(RECORD_POWER);

    BleCharacteristicObject* ch = instance->chars[BleSrvBatteryCharacterIndexBatteryLevel];
    ble_characteristic_set_data(ch, &info.charge, sizeof(info.charge));

    BleBatteryServiceContext* ctx = instance->context;
    BatteryStatusInfo battery_status = {0};
    battery_status.flags = 0;
    battery_status.state.fields.battery_present =
        (ctx->event_type == PowerEventBatteryNotPresent) ? 0 : 1;
    battery_status.state.fields.wired_source_present = info.is_charging;
    battery_status.state.fields.wireless_source_present = 0;
    battery_status.state.fields.battery_charge_state = info.is_charging ? 1 : 2;
    battery_status.state.fields.battery_charge_level = 1;
    battery_status.state.fields.charging_type = 2;
    battery_status.state.fields.charging_fault_reason = 0;

    ch = instance->chars[BleSrvBatteryCharacterIndexBatteryStatus];
    ble_characteristic_set_data(ch, &battery_status, sizeof(BatteryStatusInfo));
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

bool ble_service_battery_run(void* object) {
    furi_assert(object);
    BleServiceObject* instance = object;

    ble_service_battery_update(instance);
    return true;
}
