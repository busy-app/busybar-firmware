#include "busy_timer_saved_state_interface_v1.h"

#include "../busy_timer_snapshot_i.h"

static bool busy_timer_saved_state_snapshot_is_valid_callback(
    const SettingProviderSetting* setting,
    const void* value) {
    UNUSED(setting);
    const BusyTimerSnapshot* snapshot = value;
    return busy_timer_snapshot_is_valid(snapshot);
}

static bool busy_timer_saved_state_snapshot_serialize_callback(
    const SettingProviderSetting* setting,
    const void* value,
    cJSON* json_node) {
    UNUSED(setting);
    const BusyTimerSnapshot* snapshot = value;
    return busy_timer_snapshot_serialize_raw(snapshot, json_node);
}

static bool busy_timer_saved_state_snapshot_deserialize_callback(
    const SettingProviderSetting* setting,
    const cJSON* json_node,
    void* value) {
    UNUSED(setting);
    BusyTimerSnapshot* snapshot = value;
    return busy_timer_snapshot_deserialize_raw(snapshot, json_node);
}

static const SettingProviderSetting busy_timer_saved_state_v1[] = {
    [BusyTimerSavedStateV1IdxSnapshot] =
        {
            .name = "snapshot",
            .interface =
                &(const SettingProviderRawInterface){
                    .is_valid_callback = busy_timer_saved_state_snapshot_is_valid_callback,
                    .serialize_callback = busy_timer_saved_state_snapshot_serialize_callback,
                    .deserialize_callback = busy_timer_saved_state_snapshot_deserialize_callback,
                    .default_value =
                        &(const BusyTimerSnapshot){
                            .type = BusyTimerSnapshotTypeNotStarted,
                            .common =
                                {
                                    .card_id = "00000000-0000-0000-0000-000000000000",
                                },
                        },
                    .default_value_size = sizeof(BusyTimerSnapshot),
                },
            .field_offset = offsetof(BusyTimerSavedStateV1, snapshot),
            .type = SettingProviderSettingTypeRaw,
        },
};

const SettingProviderSetting busy_timer_saved_state_v1_root = {
    .name = NULL,
    .interface =
        &(const SettingProviderStructInterface){
            .inner_settings = busy_timer_saved_state_v1,
            .inner_settings_count = COUNT_OF(busy_timer_saved_state_v1),
        },
    .type = SettingProviderSettingTypeStruct,
};

static_assert(COUNT_OF(busy_timer_saved_state_v1) == BusyTimerSavedStateV1IdxMax);
