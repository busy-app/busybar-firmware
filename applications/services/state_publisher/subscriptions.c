#include "state_publisher_i.h"

#include <brightness_control/brightness_control.h>
#include <time/time.h>
#include <wifi/wifi.h>
#include <wifi/wifi_util.h>
#include <matter/matter.h>
#include <input/input.h>
#include <gui/gui.h>
#include <furi_hal_rtc.h>
#include <tzutil.h>

static void brightness_state_callback(const void* item, void* context);
static void time_settings_state_callback(const void* item, void* context);
static void wifi_info_state_callback(const void* item, void* context);
static void updater_update_state_callback(const void* item, void* context);
static void updater_check_state_callback(const void* item, void* context);
static void updater_settings_state_callback(const void* item, void* context);

static void power_pubsub_callback(const void* message, void* context);
static void audio_pubsub_callback(const void* message, void* context);
static void device_name_state_callback(const void* message, void* context);
static void matter_pubsub_callback(const void* message, void* context);
static void input_event_pubsub_callback(const void* message, void* context);
static void busy_timer_pubsub_callback(const void* message, void* context);
static void ble_pubsub_callback(const void* message, void* context);

void state_publisher_subscribe(StatePublisher* instance) {
    {
        const BrightnessControl* brightness_control = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
        instance->state_brightness = brightness_control_get_state(brightness_control);
        furi_state_subscribe(instance->state_brightness, brightness_state_callback, instance);
    }
    {
        instance->power = furi_record_open(RECORD_POWER);
        FuriPubSub* pubsub = power_get_pubsub(instance->power);
        furi_pubsub_subscribe(pubsub, power_pubsub_callback, instance);
    }
    {
        Time* time = furi_record_open(RECORD_TIME);
        instance->state_time_settings = time_get_settings_state(time);
        furi_state_subscribe(
            instance->state_time_settings, time_settings_state_callback, instance);
    }
    {
        instance->audio = furi_record_open(RECORD_AUDIO);
        FuriPubSub* pubsub = audio_get_pubsub(instance->audio);
        furi_pubsub_subscribe(pubsub, audio_pubsub_callback, instance);
    }
    {
        instance->device_name = furi_record_open(RECORD_DEVICE_NAME);
        FuriState* state_device_name = device_name_get_state(instance->device_name);
        furi_state_subscribe(state_device_name, device_name_state_callback, instance);
    }
    {
        Wifi* wifi = furi_record_open(RECORD_WIFI);
        instance->state_wifi = wifi_get_state(wifi);
        furi_state_subscribe(instance->state_wifi, wifi_info_state_callback, instance);
    }
    {
        instance->matter = furi_record_open(RECORD_MATTER);
        FuriPubSub* pubsub = matter_get_pubsub(instance->matter);
        furi_pubsub_subscribe(pubsub, matter_pubsub_callback, instance);
    }
    {
        instance->updater = furi_record_open(RECORD_UPDATER);
        FuriState* update_state = updater_get_update_state(instance->updater);
        FuriState* check_state = updater_get_check_state(instance->updater);
        FuriState* settings_state = updater_get_settings_state(instance->updater);
        furi_state_subscribe(update_state, updater_update_state_callback, instance);
        furi_state_subscribe(check_state, updater_check_state_callback, instance);
        furi_state_subscribe(settings_state, updater_settings_state_callback, instance);
    }
    {
        FuriPubSub* input_pubsub = furi_record_open(RECORD_INPUT_EVENTS);
        furi_pubsub_subscribe(input_pubsub, input_event_pubsub_callback, instance);
    }
    {
        Input* input = furi_record_open(RECORD_INPUT);
        instance->state_switch_pos = input_get_switch_pos(input);
    }
    {
        instance->busy_timer = furi_record_open(RECORD_BUSY_TIMER);
        FuriPubSub* pubsub = busy_timer_get_pubsub(instance->busy_timer);
        furi_pubsub_subscribe(pubsub, busy_timer_pubsub_callback, instance);
    }
    {
        instance->ble = furi_record_open(RECORD_BLE);
        FuriPubSub* pubsub = ble_get_pubsub(instance->ble);
        furi_pubsub_subscribe(pubsub, ble_pubsub_callback, instance);
    }
}

static BSB_State_StateUpdate* collect_brightness(const BrightnessControlState* state) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_brightness_tag;
    switch(state->mode) {
    case BrightnessControlBrightnessModeAuto:
        update->state.brightness.which_setting = BSB_State_Brightness_automatic_tag;
        break;
    case BrightnessControlBrightnessModeManual:
        update->state.brightness.which_setting = BSB_State_Brightness_manual_tag;
        update->state.brightness.setting.manual.brightness = state->brightness_setting;
        break;
    default:
        furi_assert(false);
    }

    update->state.brightness.actual_brightness = state->effective_brightness;

    return update;
}

static void brightness_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const BrightnessControlState* state = item;

    BSB_State_StateUpdate* update = collect_brightness(state);
    state_publisher_schedule_state_update(instance, update, StreamFlagAll);
}

static BSB_State_StateUpdate* collect_power(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    PowerInfo power_info;
    power_get_info(instance->power, &power_info);

    update->which_state = BSB_State_StateUpdate_power_tag;

    update->state.power.which_state = BSB_State_Power_known_tag;
    if(power_info.is_charging) {
        if(power_info.is_full_charged) {
            update->state.power.state.known.battery_status = BSB_State_BatteryStatus_CHARGED;
        } else {
            update->state.power.state.known.battery_status = BSB_State_BatteryStatus_CHARGING;
        }
    } else {
        update->state.power.state.known.battery_status = BSB_State_BatteryStatus_DISCHARGING;
    }
    update->state.power.state.known.battery_charge_percent = power_info.charge;
    update->state.power.state.known.battery_voltage_mv = power_info.voltage_battery;
    update->state.power.state.known.usb_voltage_mv = power_info.voltage_usb;
    update->state.power.state.known.battery_current_ma = power_info.current_battery;
    return update;
}

void state_publisher_store_power(StatePublisher* instance) {
    BSB_State_StateUpdate* update = collect_power(instance);

    state_publisher_store_state_update(instance, update, StreamFlagAll);
}

static BSB_State_StateUpdate* collect_audio(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));

    float volume = audio_get_volume(instance->audio);

    update->which_state = BSB_State_StateUpdate_audio_volume_tag;

    update->state.audio_volume.volume = (uint8_t)roundf(volume * 100.0f);

    return update;
}

void state_publisher_store_audio(StatePublisher* instance) {
    BSB_State_StateUpdate* update = collect_audio(instance);

    state_publisher_store_state_update(instance, update, StreamFlagAll);
}

static BSB_State_StateUpdate* collect_matter(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));

    MatterCommissionedFabrics info;
    matter_get_commissioned_fabrics(instance->matter, &info);

    update->which_state = BSB_State_StateUpdate_matter_tag;

    update->state.matter.fabric_count = info.count;

    if(info.last_status_at) {
        update->state.matter.has_state = true;
        static const BSB_State_MatterCommissioningStatus lookup[] = {
            [MatterCommissioningStatusNeverStarted] =
                BSB_State_MatterCommissioningStatus_NEVER_STARTED,
            [MatterCommissioningStatusStarted] = BSB_State_MatterCommissioningStatus_STARTED,
            [MatterCommissioningStatusComplete] =
                BSB_State_MatterCommissioningStatus_COMPLETED_SUCCESSFULLY,
            [MatterCommissioningStatusFailed] = BSB_State_MatterCommissioningStatus_FAILED};
        static_assert(COUNT_OF(lookup) == MatterCommissioningStatusMAX);
        update->state.matter.state.status = lookup[info.last_status];
        update->state.matter.state.timestamp = info.last_status_at;
    } else {
        update->state.matter.has_state = false;
    }

    return update;
}

void state_publisher_store_matter(StatePublisher* instance) {
    BSB_State_StateUpdate* update = collect_matter(instance);

    state_publisher_store_state_update(instance, update, StreamFlagAll);
}

void state_publisher_store_update_check(StatePublisher* instance) {
    FuriState* check_state = updater_get_check_state(instance->updater);
    UpdaterCheckState info;
    furi_state_get(check_state, &info);
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_update_check_tag;

    static const BSB_Update_CheckEvent check_event_lookup[] = {
        [UpdaterCheckEventStart] = BSB_Update_CheckEvent_START,
        [UpdaterCheckEventStop] = BSB_Update_CheckEvent_STOP,
        [UpdaterCheckEventNone] = BSB_Update_CheckEvent_NONE,
    };
    static_assert(COUNT_OF(check_event_lookup) == UpdaterCheckEventsCount);
    update->state.update_check.event = check_event_lookup[info.event];

    switch(info.result) {
    case UpdaterCheckResultAvailable: {
        update->state.update_check.which_status = BSB_Update_CheckState_available_tag;
        FuriString* version = furi_string_alloc();
        updater_get_check_info(
            instance->updater,
            &(UpdateCheckInfo){
                .version = version,
            });
        strlcpy(
            update->state.update_check.status.available.version,
            furi_string_get_cstr(version),
            sizeof(update->state.update_check.status.available.version));
        furi_string_free(version);
        break;
    }
    case UpdaterCheckResultNotAvailable:
        update->state.update_check.status.unavailable.reason = BSB_Update_CheckError_NOT_AVAILABLE;
        update->state.update_check.which_status = BSB_Update_CheckState_unavailable_tag;
        break;
    case UpdaterCheckResultFailure:
        update->state.update_check.status.unavailable.reason = BSB_Update_CheckError_FAILURE;
        update->state.update_check.which_status = BSB_Update_CheckState_unavailable_tag;
        break;
    case UpdaterCheckResultNone:
        update->state.update_check.status.unavailable.reason = BSB_Update_CheckError_IDLE;
        update->state.update_check.which_status = BSB_Update_CheckState_unavailable_tag;
        break;
    default:
        furi_assert(false);
        break;
    }
    state_publisher_store_state_update(instance, update, StreamFlagAll);
}

static BSB_State_StateUpdate* collect_autoupdate(const UpdaterSettings* settings) {
    BSB_State_StateUpdate* update = malloc(sizeof(*update));
    update->which_state = BSB_State_StateUpdate_auto_update_state_tag;
    update->state.auto_update_state.enabled = settings->autoupdate_enabled;

    update->state.auto_update_state.has_interval = true;
    update->state.auto_update_state.interval.start = settings->autoupdate_interval_start;
    update->state.auto_update_state.interval.end = settings->autoupdate_interval_end;
    return update;
}

void state_publisher_store_autoupdate(StatePublisher* instance) {
    UpdaterSettings settings;
    updater_get_settings(instance->updater, &settings);
    BSB_State_StateUpdate* update = collect_autoupdate(&settings);
    state_publisher_store_state_update(instance, update, StreamFlagAll);
}

static void set_json_from_text(BSB_Util_Json* json, const char* text) {
    json->compression = BSB_Util_Compression_PLAIN;
    size_t len = strlen(text);
    json->data = malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(len));
    json->data->size = len;
    memcpy(&json->data->bytes, text, len);
}

static BSB_State_StateUpdate* collect_busy_timer(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));

    update->which_state = BSB_State_StateUpdate_timer_tag;

    update->state.timer.has_json = true;

    BusyTimerSnapshot snapshot;
    busy_timer_get_snapshot(instance->busy_timer, &snapshot);
    char* json_text = busy_timer_snapshot_serialize(&snapshot);
    set_json_from_text(&update->state.timer.json, json_text);
    free(json_text);

    return update;
}

void state_publisher_store_busy_timer(StatePublisher* instance) {
    BSB_State_StateUpdate* update = collect_busy_timer(instance);
    state_publisher_store_state_update(instance, update, StreamFlagAll);
}

static BSB_State_StateUpdate* collect_busy_timer_profiles(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));

    update->which_state = BSB_State_StateUpdate_timer_profiles_tag;

    update->state.timer_profiles.profiles_count = BusyTimerProfileIdMax;
    update->state.timer_profiles.profiles =
        malloc(sizeof(BSB_Timer_Profile) * BusyTimerProfileIdMax);

    for(BusyTimerProfileId id = 0; id != BusyTimerProfileIdMax; ++id) {
        BSB_Timer_Profile* profile_rec = update->state.timer_profiles.profiles + id;

        strlcpy(profile_rec->name, busy_timer_get_profile_name(id), sizeof(profile_rec->name));

        BusyTimerProfile profile;
        busy_timer_get_profile(instance->busy_timer, id, &profile);

        char* json_text = busy_timer_profile_serialize(&profile);
        profile_rec->has_json = true;
        set_json_from_text(&profile_rec->json, json_text);
        free(json_text);
    }
    return update;
}

void state_publisher_store_busy_timer_profiles(StatePublisher* instance) {
    BSB_State_StateUpdate* update = collect_busy_timer_profiles(instance);

    state_publisher_store_state_update(instance, update, StreamFlagAll);
}

// Can return NULL
static BSB_State_StateUpdate* collect_ble(StatePublisher* instance) {
    BleState ble_state;
    bool ok = ble_get_state(instance->ble, &ble_state);

    if(ok) {
        BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));

        static BSB_State_Ble_ServiceStatus lookup[] = {
            [BleServiceStatusReset] = BSB_State_Ble_ServiceStatus_RESET,
            [BleServiceStatusInitialization] = BSB_State_Ble_ServiceStatus_INITIALIZATION,
            [BleServiceStatusReady] = BSB_State_Ble_ServiceStatus_READY,
            [BleServiceStatusAdvertising] = BSB_State_Ble_ServiceStatus_ADVERTISING,
            [BleServiceStatusConnectable] = BSB_State_Ble_ServiceStatus_CONNECTABLE,
            [BleServiceStatusConnected] = BSB_State_Ble_ServiceStatus_CONNECTED,
            [BleServiceStatusError] = BSB_State_Ble_ServiceStatus_ERROR,
        };
        static_assert(COUNT_OF(lookup) == BleServiceStatusCount);

        update->which_state = BSB_State_StateUpdate_ble_tag;
        update->state.ble.status = lookup[ble_state.status];

        if(ble_state.status == BleServiceStatusConnected) {
            update->state.ble.has_remote_address = true;
            strlcpy(
                update->state.ble.remote_address,
                (const char*)ble_state.remote_device_address,
                sizeof(update->state.ble.remote_address));
        } else {
            update->state.ble.has_remote_address = false;
        }

        return update;
    } else {
        return NULL;
    }
}

void state_publisher_store_ble(StatePublisher* instance) {
    BSB_State_StateUpdate* update = collect_ble(instance);
    if(update) {
        state_publisher_store_state_update(instance, update, StreamFlagAll);
    }
}

static void power_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    // dispatch because power_get_info cannot be called from power task
    furi_event_flag_set(instance->fetch_flags, FetchFlagPowerEvent);
}

static void audio_pubsub_callback(const void* message, void* context) {
    StatePublisher* instance = context;
    const AudioEvent* event = message;

    if(event->type == AudioEventVolumeUpdate) {
        // dispatch because audio_get_volume cannot be called from audio task
        furi_event_flag_set(instance->fetch_flags, FetchFlagAudioEvent);
    }
}

static BSB_State_StateUpdate* collect_device_name(const char* name) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));

    update->which_state = BSB_State_StateUpdate_device_name_tag;
    static_assert(
        sizeof(update->state.device_name.name) > sizeof(void*)); // make sure it's an array
    strlcpy(update->state.device_name.name, name, sizeof(update->state.device_name.name));
    return update;
}

static void device_name_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const DeviceNameInfo* device_name_info = item;

    BSB_State_StateUpdate* update = collect_device_name(device_name_info->name);
    state_publisher_schedule_state_update(instance, update, StreamFlagAll);
}

static void matter_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    furi_event_flag_set(instance->fetch_flags, FetchFlagMatterEvent);
}

static void input_event_pubsub_callback(const void* message, void* context) {
    StatePublisher* instance = context;
    const InputEvent* event = message;

    // Only publish "press" and "release" events
    if(event->type != InputTypePress && event->type != InputTypeRelease) {
        return;
    } else if(event->type == InputTypeRelease) {
        // release event is handled for buttons only
        switch(event->key) {
        case InputKeyOk:
        case InputKeyBack:
        case InputKeyStart:
            break;
        default:
            return;
        }
    }

    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));

    update->which_state = BSB_State_StateUpdate_input_tag;

    static const BSB_Input_ButtonAction action_lookup[] = {
        [InputTypePress] = BSB_Input_ButtonAction_PRESS,
        [InputTypeRelease] = BSB_Input_ButtonAction_RELEASE,
    };

    static const BSB_Input_SwitchPosition switch_lookup[] = {
        [InputKeyBusy] = BSB_Input_SwitchPosition_BUSY,
        [InputKeyCustom] = BSB_Input_SwitchPosition_CUSTOM,
        [InputKeyOff] = BSB_Input_SwitchPosition_OFF,
        [InputKeyApps] = BSB_Input_SwitchPosition_APPS,
        [InputKeySettings] = BSB_Input_SwitchPosition_SETTINGS,
    };

    static const BSB_Input_Button button_lookup[] = {
        [InputKeyOk] = BSB_Input_Button_OK,
        [InputKeyBack] = BSB_Input_Button_BACK,
        [InputKeyStart] = BSB_Input_Button_START,
    };

    update->state.input.event.encoder_event.delta = 0;
    switch(event->key) {
    case InputKeyUp: // encoder +1
        update->state.input.event.encoder_event.delta = 2;
        // fall-through
    case InputKeyDown: // encoder -1
        update->state.input.event.encoder_event.delta -= 1;
        update->state.input.which_event = BSB_Input_InputEvent_encoder_event_tag;
        break;
    case InputKeyOk:
    case InputKeyBack:
    case InputKeyStart:
        update->state.input.which_event = BSB_Input_InputEvent_button_event_tag;
        update->state.input.event.button_event.button = button_lookup[event->key];
        update->state.input.event.button_event.action = action_lookup[event->type];
        break;
    case InputKeyBusy:
    case InputKeyCustom:
    case InputKeyOff:
    case InputKeyApps:
    case InputKeySettings:
        update->state.input.which_event = BSB_Input_InputEvent_switch_event_tag;
        update->state.input.event.switch_event.position = switch_lookup[event->key];
        break;
    default:
        furi_assert(false);
        break;
    }

    state_publisher_schedule_state_update(instance, update, StreamFlagAll);
}

static BSB_State_StateUpdate* collect_switch_position(InputSwitchPosition position) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));

    static const BSB_Input_SwitchPosition lookup[] = {
        [InputSwitchPositionBusy] = BSB_Input_SwitchPosition_BUSY,
        [InputSwitchPositionStatus] = BSB_Input_SwitchPosition_CUSTOM,
        [InputSwitchPositionOff] = BSB_Input_SwitchPosition_OFF,
        [InputSwitchPositionApps] = BSB_Input_SwitchPosition_APPS,
        [InputSwitchPositionSettings] = BSB_Input_SwitchPosition_SETTINGS,
    };
    static_assert(COUNT_OF(lookup) == InputSwitchPositionMAX);

    update->which_state = BSB_State_StateUpdate_input_tag;
    update->state.input.which_event = BSB_Input_InputEvent_switch_event_tag;
    update->state.input.event.switch_event.position = lookup[position];

    return update;
}

static void busy_timer_pubsub_callback(const void* message, void* context) {
    const BusyTimerEvent* event = message;
    StatePublisher* instance = context;

    switch(event->type) {
    case BusyTimerEventTypeSnapshotCreated:
        furi_event_flag_set(instance->fetch_flags, FetchFlagBusyTimer);
        break;
    case BusyTimerEventTypeProfileChanged:
        furi_event_flag_set(instance->fetch_flags, FetchFlagBusyTimerProfiles);
        break;
    default:
        break;
    }
}

static void ble_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    furi_event_flag_set(instance->fetch_flags, FetchFlagBle);
}

static BSB_State_StateUpdate* collect_time_settings(const TimeSettings* settings) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_timezone_tag;

    static_assert(sizeof(update->state.timezone.name) > sizeof(void*)); // make sure it's an array
    strlcpy(
        update->state.timezone.name, settings->timezone.name, sizeof(update->state.timezone.name));
    update->state.timezone.offset =
        settings->timezone.offset.hours * 60 + settings->timezone.offset.minutes;

    DateTime now = furi_hal_rtc_get_datetime().dt;
    TzutilTzInfo info;
    if(tzutil_get_info_by_name(settings->timezone.name, &now, &info)) {
        tzutil_get_abbr(&info, update->state.timezone.abbr, sizeof(update->state.timezone.abbr));
    } else {
        update->state.timezone.abbr[0] = 0;
    }
    return update;
}

static void time_settings_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const TimeSettings* settings = item;

    BSB_State_StateUpdate* update = collect_time_settings(settings);

    state_publisher_schedule_state_update(instance, update, StreamFlagAll);
}

static BSB_State_IpConfigurationMethod convert_ip_configuration_method(WifiIpManagement method) {
    switch(method) {
    case WifiIpManagementStatic:
        return BSB_State_IpConfigurationMethod_STATIC;
    case WifiIpManagementDynamic:
        return BSB_State_IpConfigurationMethod_DHCP;
    default:
        furi_assert(false);
        return BSB_State_IpConfigurationMethod_STATIC;
    }
}

static void convert_ip_config(BSB_State_Wifi* dst, const WifiIpConfig* ip_config) {
    switch(ip_config->type) {
    case WifiIpTypeV4:
        dst->ip_addresses_count = 1;
        dst->ip_addresses[0].method = convert_ip_configuration_method(ip_config->mgmt);
        dst->ip_addresses[0].protocol = BSB_State_IpProtocol_IPV4;
        wifi_format_ipv4(
            &ip_config->ip4.address,
            dst->ip_addresses[0].address,
            sizeof(dst->ip_addresses[0].address));
        wifi_format_ipv4(
            &ip_config->ip4.mask,
            dst->ip_addresses[0].netmask,
            sizeof(dst->ip_addresses[0].netmask));
        wifi_format_ipv4(
            &ip_config->ip4.gateway,
            dst->ip_addresses[0].gateway,
            sizeof(dst->ip_addresses[0].gateway));
        break;
    case WifiIpTypeV6:
        // No IPv6
        dst->ip_addresses_count = 0;
        break;
    default:
        furi_assert(false);
        break;
    }
}

static BSB_State_StateUpdate* collect_wifi_info(const WifiInfo* info) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_wifi_tag;

    switch(info->state) {
    case WifiStateUnknown:
        update->state.wifi.which_wifi_state = BSB_State_Wifi_unknown_tag;
        break;
    case WifiStateDisconnected:
        update->state.wifi.which_wifi_state = BSB_State_Wifi_inactive_tag;
        break;
    case WifiStateConnected:
    case WifiStateConnecting:
    case WifiStateDisconnecting:
    case WifiStateReconnecting: {
        update->state.wifi.which_wifi_state = BSB_State_Wifi_active_tag;

        // status
        static const BSB_State_WifiConnectionStatus status_lookup[] = {
            [WifiStateConnected] = BSB_State_WifiConnectionStatus_CONNECTED,
            [WifiStateConnecting] = BSB_State_WifiConnectionStatus_CONNECTING,
            [WifiStateDisconnecting] = BSB_State_WifiConnectionStatus_DISCONNECTING,
            [WifiStateReconnecting] = BSB_State_WifiConnectionStatus_RECONNECTING,
        };
        update->state.wifi.wifi_state.active.status = status_lookup[info->state];

        // SSID
        static_assert(
            sizeof(update->state.wifi.wifi_state.active.ssid) >
            sizeof(void*)); // make sure it's an array
        strlcpy(
            update->state.wifi.wifi_state.active.ssid,
            info->ssid,
            sizeof(update->state.wifi.wifi_state.active.ssid));

        // BSSID
        static_assert(
            sizeof(update->state.wifi.wifi_state.active.bssid) >
            sizeof(void*)); // make sure it's an array
        wifi_format_bssid(
            info->bssid,
            update->state.wifi.wifi_state.active.bssid,
            sizeof(update->state.wifi.wifi_state.active.bssid));

        // channel
        update->state.wifi.wifi_state.active.channel = info->channel;

        // rssi
        update->state.wifi.wifi_state.active.rssi = info->rssi;

        // security
        static const BSB_State_WifiSecurity security_lookup[] = {
            [WifiSecurityModeOpen] = BSB_State_WifiSecurity_OPEN,
            [WifiSecurityModeWpa] = BSB_State_WifiSecurity_WPA,
            [WifiSecurityModeWpa2] = BSB_State_WifiSecurity_WPA2,
            [WifiSecurityModeWep] = BSB_State_WifiSecurity_WEP,
            [WifiSecurityModeWpaWpa2Mixed] = BSB_State_WifiSecurity_WPA_WPA2,
            [WifiSecurityModeWpa3] = BSB_State_WifiSecurity_WPA3,
            [WifiSecurityModeWpa3Transition] = BSB_State_WifiSecurity_WPA2_WPA3,
            [WifiSecurityModeUnsupported] = BSB_State_WifiSecurity_UNKNOWN,
        };
        static_assert(COUNT_OF(security_lookup) == WifiSecurityModeMax);
        update->state.wifi.wifi_state.active.security = security_lookup[info->security_mode];

        // IP
        convert_ip_config(&update->state.wifi, &info->ip_config);

        break;
    }
    default:
        furi_assert(false);
        break;
    }
    return update;
}

static void wifi_info_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const WifiInfo* info = item;

    BSB_State_StateUpdate* update = collect_wifi_info(info);

    state_publisher_schedule_state_update(instance, update, StreamFlagAll);
}

static void updater_update_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const UpdaterUpdateState* info = item;

    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_update_state_tag;

    static const BSB_Update_UpdateStatus status_lookup[] = {
        [UpdaterStatusOk] = BSB_Update_UpdateStatus_OK,
        [UpdaterStatusBatteryLow] = BSB_Update_UpdateStatus_BATTERY_LOW,
        [UpdaterStatusBusy] = BSB_Update_UpdateStatus_BUSY,
        [UpdaterStatusDownloadFailure] = BSB_Update_UpdateStatus_DOWNLOAD_FAILURE,
        [UpdaterStatusDownloadAbort] = BSB_Update_UpdateStatus_DOWNLOAD_ABORT,
        [UpdaterStatusShaMismatch] = BSB_Update_UpdateStatus_SHA_MISMATCH,
        [UpdaterStatusUnpackCreateStagingDirectoryFailure] =
            BSB_Update_UpdateStatus_UNPACK_CREATE_STAGING_DIRECTORY_FAILURE,
        [UpdaterStatusUnpackArchiveOpenFailure] =
            BSB_Update_UpdateStatus_UNPACK_ARCHIVE_OPEN_FAILURE,
        [UpdaterStatusUnpackArchiveUnpackFailure] =
            BSB_Update_UpdateStatus_UNPACK_ARCHIVE_UNPACK_FAILURE,
        [UpdaterStatusInstallationPrepareManifestNotFound] =
            BSB_Update_UpdateStatus_INSTALLATION_PREPARE_MANIFEST_NOT_FOUND,
        [UpdaterStatusInstallationPrepareManifestInvalid] =
            BSB_Update_UpdateStatus_INSTALLATION_PREPARE_MANIFEST_INVALID,
        [UpdaterStatusInstallationPrepareSessionConfigSetupFailure] =
            BSB_Update_UpdateStatus_INSTALLATION_PREPARE_SESSION_CONFIG_SETUP_FAILURE,
        [UpdaterStatusInstallationPreparePointerSetupFailure] =
            BSB_Update_UpdateStatus_INSTALLATION_PREPARE_POINTER_SETUP_FAILURE,
        [UpdaterStatusUnknownFailure] = BSB_Update_UpdateStatus_UNKNOWN_FAILURE,
    };
    static_assert(COUNT_OF(status_lookup) == UpdaterStatusesCount);

    static const BSB_Update_UpdateAction action_lookup[] = {
        [UpdaterUpdateActionDownload] = BSB_Update_UpdateAction_DOWNLOAD,
        [UpdaterUpdateActionShaVerification] = BSB_Update_UpdateAction_SHA_VERIFICATION,
        [UpdaterUpdateActionUnpack] = BSB_Update_UpdateAction_UNPACK,
        [UpdaterUpdateActionInstallationPrepare] = BSB_Update_UpdateAction_INSTALLATION_PREPARE,
        [UpdaterUpdateActionInstallationApply] = BSB_Update_UpdateAction_INSTALLATION_APPLY,
        [UpdaterUpdateActionNone] = BSB_Update_UpdateAction_ACTION_NONE,
    };
    static_assert(COUNT_OF(action_lookup) == UpdaterUpdateActionsCount);

    static const BSB_Update_UpdateEvent event_lookup[] = {
        [UpdaterUpdateEventSessionStart] = BSB_Update_UpdateEvent_SESSION_START,
        [UpdaterUpdateEventSessionStop] = BSB_Update_UpdateEvent_SESSION_STOP,
        [UpdaterUpdateEventActionBegin] = BSB_Update_UpdateEvent_ACTION_BEGIN,
        [UpdaterUpdateEventActionDone] = BSB_Update_UpdateEvent_ACTION_DONE,
        [UpdaterUpdateEventDetailChange] = BSB_Update_UpdateEvent_DETAIL_CHANGE,
        [UpdaterUpdateEventActionProgress] = BSB_Update_UpdateEvent_ACTION_PROGRESS,
        [UpdaterUpdateEventNone] = BSB_Update_UpdateEvent_EVENT_NONE,
    };
    static_assert(COUNT_OF(event_lookup) == UpdaterUpdateEventsCount);

    update->state.update_state.action = action_lookup[info->action];
    update->state.update_state.status = status_lookup[info->status];
    update->state.update_state.event = event_lookup[info->event];

    state_publisher_schedule_state_update(instance, update, StreamFlagAll);
}

static void updater_check_state_callback(const void* item, void* context) {
    UNUSED(item);
    StatePublisher* instance = context;

    furi_event_flag_set(instance->fetch_flags, FetchFlagUpdaterCheckEvent);
}

static void updater_settings_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const UpdaterSettings* settings = item;

    BSB_State_StateUpdate* update = collect_autoupdate(settings);

    state_publisher_schedule_state_update(instance, update, StreamFlagAll);
}

static BSB_State_StateUpdate*
    collect_frame(GuiDisplayId display, const ScreenStreamerFrame* frame) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_frame_tag;

    static const BSB_Frame_Encoding encoding_lookup[] = {
        [ScreenStreamerCompressionPlain] = BSB_Frame_Encoding_PLAIN,
        [ScreenStreamerCompressionRLE] = BSB_Frame_Encoding_RUN_LENGTH,
    };
    static const BSB_Frame_PixelFormat pixel_format_lookup[] = {
        [ScreenStreamerPixelFormatR8G8B8] = BSB_Frame_PixelFormat_RGB888,
        [ScreenStreamerPixelFormatL8] = BSB_Frame_PixelFormat_L8,
        [ScreenStreamerPixelFormatL4] = BSB_Frame_PixelFormat_L4,
    };
    static const BSB_Frame_Screen screen_lookup[] = {
        [GuiDisplayIdFront] = BSB_Frame_Screen_FRONT,
        [GuiDisplayIdBack] = BSB_Frame_Screen_BACK,
    };
    update->state.frame.screen = screen_lookup[display];
    update->state.frame.width = frame->width;
    update->state.frame.height = frame->height;
    update->state.frame.pixel_format = pixel_format_lookup[frame->pixel_format];
    update->state.frame.encoding = encoding_lookup[frame->compression];
    update->state.frame.data = malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(frame->data_size));
    update->state.frame.data->size = frame->data_size;
    memcpy(&update->state.frame.data->bytes, frame->data, frame->data_size);
    return update;
}

void screen_streamer_callback(
    GuiDisplayId display,
    const ScreenStreamerFrame* frame,
    uint8_t stream_flags,
    void* context) {
    StatePublisher* instance = context;

    BSB_State_StateUpdate* update = collect_frame(display, frame);

    state_publisher_schedule_state_update(instance, update, stream_flags);
}

static void get_frame_cb(
    GuiDisplayId display,
    const ScreenStreamerFrame* frame,
    uint8_t stream_flags,
    void* context) {
    UNUSED(stream_flags);
    BSB_State_StateUpdate** update = context;

    *update = collect_frame(display, frame);
}

ARRAY_DEF(StateUpdatePtrArray, BSB_State_StateUpdate*, M_PTR_OPLIST);
BSB_State_State* state_publisher_collect_all(StatePublisher* instance) {
    StateUpdatePtrArray_t updates;
    StateUpdatePtrArray_init(updates);

    {
        BrightnessControlState state;
        furi_state_get(instance->state_brightness, &state);
        StateUpdatePtrArray_push_back(updates, collect_brightness(&state));
    }
    StateUpdatePtrArray_push_back(updates, collect_power(instance));
    StateUpdatePtrArray_push_back(updates, collect_audio(instance));
    StateUpdatePtrArray_push_back(updates, collect_matter(instance));
    {
        UpdaterSettings settings;
        updater_get_settings(instance->updater, &settings);
        StateUpdatePtrArray_push_back(updates, collect_autoupdate(&settings));
    }
    StateUpdatePtrArray_push_back(updates, collect_busy_timer(instance));
    StateUpdatePtrArray_push_back(updates, collect_busy_timer_profiles(instance));
    {
        BSB_State_StateUpdate* update = collect_ble(instance);
        if(update) {
            StateUpdatePtrArray_push_back(updates, update);
        }
    }
    {
        FuriString* name = furi_string_alloc();
        device_name_get(instance->device_name, name);
        StateUpdatePtrArray_push_back(updates, collect_device_name(furi_string_get_cstr(name)));
        furi_string_free(name);
    }
    {
        TimeSettings time_settings;
        furi_state_get(instance->state_time_settings, &time_settings);
        StateUpdatePtrArray_push_back(updates, collect_time_settings(&time_settings));
    }
    {
        WifiInfo wifi_info;
        furi_state_get(instance->state_wifi, &wifi_info);
        StateUpdatePtrArray_push_back(updates, collect_wifi_info(&wifi_info));
    }
    {
        InputSwitchPosition position;
        furi_state_get(instance->state_switch_pos, &position);
        if(position != InputSwitchPositionMAX) {
            StateUpdatePtrArray_push_back(updates, collect_switch_position(position));
        }
    }
    {
        BSB_State_StateUpdate* update = NULL;
        screen_streamer_get_single_frame(instance->screen_streamer_front, get_frame_cb, &update);
        StateUpdatePtrArray_push_back(updates, update);
    }

    BSB_State_State* state = malloc(sizeof(BSB_State_State));
    state->has_error = false;
    state->timestamp = time_get_timestamp_ms();

    state->updates_count = StateUpdatePtrArray_size(updates);
    state->updates = malloc(sizeof(BSB_State_StateUpdate) * state->updates_count);
    for(size_t i = 0; i != state->updates_count; ++i) {
        BSB_State_StateUpdate* update = *StateUpdatePtrArray_cget(updates, i);
        memcpy(state->updates + i, update, sizeof(BSB_State_StateUpdate));
        free(update);
    }
    StateUpdatePtrArray_clear(updates);
    return state;
}
