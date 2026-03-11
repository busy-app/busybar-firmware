#include "state_publisher.h"
#include <furi/furi.h>
#include <dyn_buffer.h>

#include <brightness_control/brightness_control.h>
#include <power/power_service/power.h>
#include <audio/audio.h>
#include <sntp/sntp.h>
#include <device_name/device_name.h>
#include <wifi/wifi.h>
#include <wifi/wifi_util.h>
#include <matter/matter.h>

#include <nanopb/pb.h>
#include <nanopb/pb_encode.h>
#include <state.pb.h>

#define TAG "StPubSrv"

#define MAX_MESSAGES 16

struct StatePublisher {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    Power* power;
    Audio* audio;
    MatterSrv* matter;
};

typedef enum {
    MessageTypePublishUpdate,
    MessageTypePowerEvent,
    MessageTypeAudioEvent,
    MessageTypeMatterEvent,

    MessageTypesCount,
} MessageType;

typedef struct {
    MessageType type;
    union {
        BSB_State_StateUpdate* update; // allocated on the heap
    };
} Message;

typedef bool (*MessageHandler)(StatePublisher* instance, const Message* message);

static const MessageHandler message_handlers[];

static void brightness_state_callback(const void* item, void* context);
static void sntp_settings_state_callback(const void* item, void* context);
static void wifi_info_state_callback(const void* item, void* context);
static void power_pubsub_callback(const void* message, void* context);
static void audio_pubsub_callback(const void* message, void* context);
static void device_name_pubsub_callback(const void* message, void* context);
static void matter_pubsub_callback(const void* message, void* context);

static void publish_power(StatePublisher* instance);
static void publish_audio(StatePublisher* instance);
static void publish_matter(StatePublisher* instance);

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    StatePublisher* instance = context;

    Message message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    message_handlers[message.type](instance, &message);
}

static void send_message(StatePublisher* instance, const Message* message) {
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);
}

static void subscribe(StatePublisher* instance) {
    {
        const BrightnessControl* brightness_control = furi_record_open(RECORD_BRIGHTNESS_CONTROL);
        FuriState* stat = brightness_control_get_state(brightness_control);
        furi_state_subscribe(stat, brightness_state_callback, instance);
    }
    {
        instance->power = furi_record_open(RECORD_POWER);
        FuriPubSub* pubsub = power_get_pubsub(instance->power);
        furi_pubsub_subscribe(pubsub, power_pubsub_callback, instance);
    }
    {
        Sntp* sntp = furi_record_open(RECORD_SNTP);
        FuriState* state = sntp_get_settings_state(sntp);
        furi_state_subscribe(state, sntp_settings_state_callback, instance);
    }
    {
        instance->audio = furi_record_open(RECORD_AUDIO);
        FuriPubSub* pubsub = audio_get_pubsub(instance->audio);
        furi_pubsub_subscribe(pubsub, audio_pubsub_callback, instance);
    }
    {
        DeviceName* device_name = furi_record_open(RECORD_DEVICE_NAME);
        FuriPubSub* pubsub = device_name_get_pubsub(device_name);
        furi_pubsub_subscribe(pubsub, device_name_pubsub_callback, instance);
    }
    {
        Wifi* wifi = furi_record_open(RECORD_WIFI);
        FuriState* state = wifi_get_state(wifi);
        furi_state_subscribe(state, wifi_info_state_callback, instance);
    }
    {
        instance->matter = furi_record_open(RECORD_MATTER);
        FuriPubSub* pubsub = matter_get_pubsub(instance->matter);
        furi_pubsub_subscribe(pubsub, matter_pubsub_callback, instance);
    }
}

static StatePublisher* state_publisher_alloc(void) {
    StatePublisher* instance = malloc(sizeof(StatePublisher));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(Message));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    subscribe(instance);

    furi_record_create(RECORD_STATE_PUBLISHER, instance);

    return instance;
}

int32_t state_publisher_srv(void* p) {
    UNUSED(p);

    FURI_LOG_I(TAG, "Service starting...");

    StatePublisher* instance = state_publisher_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static bool ostream_cb(pb_ostream_t* stream, const pb_byte_t* data, size_t count) {
    DynBuffer* buf = stream->state;
    dyn_buffer_push(buf, data, count);
    return true;
}

static pb_ostream_t ostream_with_buffer(DynBuffer* buf) {
    return (pb_ostream_t){
        .callback = ostream_cb,
        .bytes_written = 0,
        .errmsg = NULL,
        .max_size = SIZE_MAX,
        .state = buf};
}

static bool handle_publish_update(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypePublishUpdate);
    BSB_State_StateUpdate* update = message->update;
    BSB_State_State state = {
        .timestamp = sntp_get_timestamp_ms(),
        .updates_count = 1,
        .updates = (BSB_State_StateUpdate*)update,
    };

    DynBuffer buf = dyn_buffer_init();

    pb_ostream_t stream = ostream_with_buffer(&buf);

    bool result = pb_encode(&stream, BSB_State_State_fields, &state);
    furi_assert(result);
    FuriString* dump = furi_string_alloc();
    for(size_t i = 0; i != buf.size; ++i) {
        furi_string_cat_printf(dump, "%02hhX", buf.data[i]);
    }
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(dump));
    furi_string_free(dump);
    dyn_buffer_destroy(&buf);
    free(update);
    UNUSED(instance);
    return true;
}

static bool handle_power_event(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypePowerEvent);
    publish_power(instance);
    return true;
}

static bool handle_audio_event(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypeAudioEvent);
    publish_audio(instance);
    return true;
}

static bool handle_matter_event(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypeMatterEvent);
    publish_matter(instance);
    return true;
}

static const MessageHandler message_handlers[] = {
    [MessageTypePublishUpdate] = handle_publish_update,
    [MessageTypePowerEvent] = handle_power_event,
    [MessageTypeAudioEvent] = handle_audio_event,
    [MessageTypeMatterEvent] = handle_matter_event,
};

static_assert(COUNT_OF(message_handlers) == MessageTypesCount);

static void schedule_state_update(StatePublisher* instance, BSB_State_StateUpdate* update) {
    Message msg = {
        .type = MessageTypePublishUpdate,
        .update = update,
    };
    send_message(instance, &msg);
}

static void brightness_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const BrightnessControlState* state = item;
    FURI_LOG_D(TAG, "publish brightness");

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

    schedule_state_update(instance, update);
}

static void publish_power(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    FURI_LOG_D(TAG, "publish power");

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

    schedule_state_update(instance, update);
}

static void publish_audio(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    FURI_LOG_D(TAG, "publish audio");

    float volume = audio_get_volume(instance->audio);

    update->which_state = BSB_State_StateUpdate_audio_volume_tag;

    update->state.audio_volume.volume = (uint8_t)roundf(volume * 100.0f);

    schedule_state_update(instance, update);
}

static void publish_matter(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    FURI_LOG_D(TAG, "publish matter");

    MatterCommissionedFabrics info = matter_commissioned_fabrics(instance->matter);
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

    schedule_state_update(instance, update);
}

static void power_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    // dispatch because power_get_info cannot be called from power task
    Message msg = {
        .type = MessageTypePowerEvent,
    };
    send_message(instance, &msg);
}

static void audio_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    // dispatch because audio_get_volume cannot be called from power task
    Message msg = {
        .type = MessageTypeAudioEvent,
    };
    send_message(instance, &msg);
}

static void device_name_pubsub_callback(const void* message, void* context) {
    StatePublisher* instance = context;
    const FuriString* name = message;

    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    FURI_LOG_D(TAG, "publish device name");

    update->which_state = BSB_State_StateUpdate_device_name_tag;
    static_assert(
        sizeof(update->state.device_name.name) > sizeof(void*)); // make sure it's an array
    strlcpy(
        update->state.device_name.name,
        furi_string_get_cstr(name),
        sizeof(update->state.device_name.name));

    schedule_state_update(instance, update);
}

static void matter_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    Message msg = {
        .type = MessageTypeMatterEvent,
    };
    send_message(instance, &msg);
}

static void sntp_settings_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const SntpSettings* settings = item;

    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_timezone_tag;

    static_assert(sizeof(update->state.timezone.name) > sizeof(void*)); // make sure it's an array
    strlcpy(
        update->state.timezone.name, settings->timezone.name, sizeof(update->state.timezone.name));
    update->state.timezone.offset =
        settings->timezone.offset.hours * 60 + settings->timezone.offset.minutes;

    schedule_state_update(instance, update);
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
        if(wifi_ipv6_is_specified(&ip_config->ip6.global)) {
            size_t idx = dst->ip_addresses_count;
            dst->ip_addresses_count += 1;
            dst->ip_addresses[idx].method = convert_ip_configuration_method(ip_config->mgmt);
            dst->ip_addresses[idx].protocol = BSB_State_IpProtocol_IPV6;
            wifi_format_ipv6(
                &ip_config->ip6.global,
                dst->ip_addresses[idx].address,
                sizeof(dst->ip_addresses[idx].address));
            wifi_format_ipv6(
                &ip_config->ip6.gateway,
                dst->ip_addresses[idx].gateway,
                sizeof(dst->ip_addresses[idx].gateway));
            dst->ip_addresses[idx].netmask[0] = 0;
        }
        if(wifi_ipv6_is_specified(&ip_config->ip6.local)) {
            size_t idx = dst->ip_addresses_count;
            dst->ip_addresses_count += 1;
            dst->ip_addresses[idx].method = convert_ip_configuration_method(ip_config->mgmt);
            dst->ip_addresses[idx].protocol = BSB_State_IpProtocol_IPV6;
            wifi_format_ipv6(
                &ip_config->ip6.local,
                dst->ip_addresses[idx].address,
                sizeof(dst->ip_addresses[idx].address));
            strlcpy(dst->ip_addresses[idx].gateway, "::", sizeof(dst->ip_addresses[idx].gateway));
            dst->ip_addresses[idx].netmask[0] = 0;
        }

        break;
    default:
        furi_assert(false);
        break;
    }
}

static void wifi_info_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const WifiInfo* info = item;

    FURI_LOG_D(TAG, "publish wifi");

    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_wifi_tag;

    switch(info->state) {
    case WifiStateUnknown:
        update->state.wifi.which_wifi_state = BSB_State_Wifi_unknown_tag;
        break;
    case WifiStateDisconnected:
        update->state.wifi.which_wifi_state = BSB_State_Wifi_disconnected_tag;
        break;
    case WifiStateConnected:
        update->state.wifi.wifi_state.connected.status = BSB_State_WifiConnectionStatus_CONNECTED;
        // fall-through
    case WifiStateConnecting:
        update->state.wifi.wifi_state.connected.status = BSB_State_WifiConnectionStatus_CONNECTING;
        // fall-through
    case WifiStateDisconnecting:
        update->state.wifi.wifi_state.connected.status =
            BSB_State_WifiConnectionStatus_DISCONNECTING;
        // fall-through
    case WifiStateReconnecting: {
        update->state.wifi.wifi_state.connected.status =
            BSB_State_WifiConnectionStatus_RECONNECTING;
        update->state.wifi.which_wifi_state = BSB_State_Wifi_connected_tag;

        // SSID
        static_assert(
            sizeof(update->state.wifi.wifi_state.connected.ssid) >
            sizeof(void*)); // make sure it's an array
        strlcpy(
            update->state.wifi.wifi_state.connected.ssid,
            info->ssid,
            sizeof(update->state.wifi.wifi_state.connected.ssid));

        // BSSID
        static_assert(
            sizeof(update->state.wifi.wifi_state.connected.bssid) >
            sizeof(void*)); // make sure it's an array
        wifi_format_bssid(
            info->bssid,
            update->state.wifi.wifi_state.connected.bssid,
            sizeof(update->state.wifi.wifi_state.connected.bssid));

        // channel
        update->state.wifi.wifi_state.connected.channel = info->channel;

        // rssi
        update->state.wifi.wifi_state.connected.rssi = info->rssi;

        // security
        static const BSB_State_WifiSecurity lookup[] = {
            [WifiSecurityModeOpen] = BSB_State_WifiSecurity_OPEN,
            [WifiSecurityModeWpa] = BSB_State_WifiSecurity_WPA,
            [WifiSecurityModeWpa2] = BSB_State_WifiSecurity_WPA2,
            [WifiSecurityModeWep] = BSB_State_WifiSecurity_WEP,
            [WifiSecurityModeWpaWpa2Mixed] = BSB_State_WifiSecurity_WPA_WPA2,
            [WifiSecurityModeWpa3] = BSB_State_WifiSecurity_WPA3,
            [WifiSecurityModeWpa3Transition] = BSB_State_WifiSecurity_WPA2_WPA3,
            [WifiSecurityModeUnsupported] = BSB_State_WifiSecurity_UNKNOWN,
        };
        static_assert(COUNT_OF(lookup) == WifiSecurityModeMax);
        update->state.wifi.wifi_state.connected.security = lookup[info->security_mode];

        // IP
        convert_ip_config(&update->state.wifi, &info->ip_config);

        break;
    }
    default:
        furi_assert(false);
        break;
    }

    schedule_state_update(instance, update);
}
