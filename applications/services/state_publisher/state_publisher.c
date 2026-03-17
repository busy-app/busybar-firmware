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
#include <updater/updater.h>
#include <input/input.h>
#include <busy_timer/busy_timer.h>
#include <gui/gui.h>
#include "screen_streamer.h"

#include <nanopb/pb.h>
#include <nanopb/pb_encode.h>
#include <state.pb.h>

#define TAG "StPubSrv"

#define MAX_MESSAGES 16

#define MAX_TRANSPORTS 16

typedef enum {
    StreamFlagMQTT = 1 << StatePublisherTransportMQTT,
    StreamFlagWebSocket = 1 << StatePublisherTransportWebSocket,
    StreamFlagBLE = 1 << StatePublisherTransportBLE,

    StreamFlagAll = StreamFlagMQTT | StreamFlagWebSocket | StreamFlagBLE
} StreamFlag;

typedef struct Transport {
    bool valid;
    StreamFlag flags;
    StatePublisherPublishCb cb;
    void* cb_context;
} Transport;

struct StatePublisher {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    FuriThreadId main_thread_id;

    ScreenStreamer* screen_streamer_front;
    ScreenStreamer* screen_streamer_back;

    Power* power;
    Audio* audio;
    MatterSrv* matter;
    Updater* updater;
    BusyTimer* busy_timer;
    Gui* gui;

    FuriMutex* transports_mutex;
    Transport transports[MAX_TRANSPORTS];
};

typedef enum {
    MessageTypePublishUpdate,
    MessageTypePowerEvent,
    MessageTypeAudioEvent,
    MessageTypeMatterEvent,
    MessageTypeUpdaterCheckEvent,
    MessageTypeBusyTimer,

    MessageTypesCount,
} MessageType;

typedef struct {
    MessageType type;
    union {
        struct {
            BSB_State_StateUpdate* data; // allocated on the heap
            StreamFlag stream_flags;
        } update;
        UpdaterCheckState updater_check_state;
    };
} Message;

typedef bool (*MessageHandler)(StatePublisher* instance, const Message* message);

static const MessageHandler message_handlers[];

static void brightness_state_callback(const void* item, void* context);
static void sntp_settings_state_callback(const void* item, void* context);
static void wifi_info_state_callback(const void* item, void* context);
static void updater_update_state_callback(const void* item, void* context);
static void updater_check_state_callback(const void* item, void* context);

static void power_pubsub_callback(const void* message, void* context);
static void audio_pubsub_callback(const void* message, void* context);
static void device_name_pubsub_callback(const void* message, void* context);
static void matter_pubsub_callback(const void* message, void* context);
static void input_event_pubsub_callback(const void* message, void* context);
static void busy_timer_pubsub_callback(const void* message, void* context);

static void publish_power(StatePublisher* instance);
static void publish_audio(StatePublisher* instance);
static void publish_matter(StatePublisher* instance);
static void publish_update_check(StatePublisher* instance, const UpdaterCheckState* check_state);
static void publish_busy_timer(StatePublisher* instance);

void screen_streamer_callback(
    GuiDisplayId display,
    const ScreenStreamerFrame* frame,
    uint8_t stream_flags,
    void* context);

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    StatePublisher* instance = context;

    Message message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    message_handlers[message.type](instance, &message);
}

static void send_message(StatePublisher* instance, const Message* message) {
    if(furi_thread_get_current_id() == instance->main_thread_id) {
        message_handlers[message->type](instance, message);
    } else {
        furi_check(
            furi_message_queue_put(instance->message_queue, message, FuriWaitForever) ==
            FuriStatusOk);
    }
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
    {
        instance->updater = furi_record_open(RECORD_UPDATER);
        FuriState* update_state = updater_get_update_state(instance->updater);
        FuriState* check_state = updater_get_check_state(instance->updater);
        furi_state_subscribe(update_state, updater_update_state_callback, instance);
        furi_state_subscribe(check_state, updater_check_state_callback, instance);
    }
    {
        FuriPubSub* input_pubsub = furi_record_open(RECORD_INPUT_EVENTS);
        furi_pubsub_subscribe(input_pubsub, input_event_pubsub_callback, instance);
    }
    {
        instance->busy_timer = furi_record_open(RECORD_BUSY_TIMER);
        FuriPubSub* pubsub = busy_timer_get_pubsub(instance->busy_timer);
        furi_pubsub_subscribe(pubsub, busy_timer_pubsub_callback, instance);
    }
    {
        screen_streamer_add_stream(instance->screen_streamer_front, 5000, 1);
        screen_streamer_add_stream(instance->screen_streamer_front, 1300, 2);
    }
}

static StatePublisher* state_publisher_alloc(void) {
    StatePublisher* instance = malloc(sizeof(StatePublisher));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(Message));
    instance->main_thread_id = furi_thread_get_current_id();

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    instance->gui = furi_record_open(RECORD_GUI);

    instance->screen_streamer_front = screen_streamer_alloc(
        GuiDisplayIdFront, instance->gui, screen_streamer_callback, instance);
    instance->screen_streamer_back =
        screen_streamer_alloc(GuiDisplayIdBack, instance->gui, screen_streamer_callback, instance);

    instance->transports_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    bzero(instance->transports, sizeof(instance->transports));

    subscribe(instance);

    screen_streamer_start(instance->screen_streamer_front);
    screen_streamer_start(instance->screen_streamer_back);

    furi_record_create(RECORD_STATE_PUBLISHER, instance);

    return instance;
}

StatePublisherTransportHandle state_publisher_add_transport(
    StatePublisher* instance,
    StatePublisherTransport transport,
    uint32_t frame_interval_ms,
    StatePublisherPublishCb cb,
    void* context) {
    UNUSED(frame_interval_ms);
    size_t i = 0;
    furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
    for(; i != MAX_TRANSPORTS; ++i) {
        Transport* t = instance->transports + i;
        if(!t->valid) {
            t->valid = true;
            t->flags = 1 << transport;
            t->cb = cb;
            t->cb_context = context;
            break;
        }
    }
    furi_mutex_release(instance->transports_mutex);
    furi_check(i < MAX_TRANSPORTS);
    return i;
}

void state_publisher_del_transport(StatePublisher* instance, StatePublisherTransportHandle handle) {
    furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
    instance->transports[handle].valid = false;
    furi_mutex_release(instance->transports_mutex);
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

static void free_state_update(BSB_State_StateUpdate* update) {
    switch(update->which_state) {
    case BSB_State_StateUpdate_timer_tag:
        free(update->state.timer.json.data);
        break;
    case BSB_State_StateUpdate_frame_tag:
        free(update->state.frame.data);
        break;
    default:
        break;
    }
    free(update);
}

static bool handle_publish_update(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypePublishUpdate);
    BSB_State_StateUpdate* update = message->update.data;
    BSB_State_State state = {
        .timestamp = sntp_get_timestamp_ms(),
        .updates_count = 1,
        .updates = (BSB_State_StateUpdate*)update,
    };

    DynBuffer buf = dyn_buffer_init();

    pb_ostream_t stream = ostream_with_buffer(&buf);

    bool result = pb_encode(&stream, BSB_State_State_fields, &state);
    furi_assert(result);

    FuriString* dump = furi_string_alloc_printf("%hhx: ", message->update.stream_flags);
    for(size_t i = 0; i != buf.size; ++i) {
        furi_string_cat_printf(dump, "%02hhX", buf.data[i]);
    }
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(dump));
    furi_string_free(dump);

    {
        furi_mutex_acquire(instance->transports_mutex, FuriWaitForever);
        for(size_t i = 0; i != MAX_TRANSPORTS; ++i) {
            Transport* t = instance->transports + i;
            if(t->valid && (t->flags & message->update.stream_flags)) {
                t->cb(buf.data, buf.size, t->cb_context);
            }
        }
        furi_mutex_release(instance->transports_mutex);
    }
    dyn_buffer_destroy(&buf);

    free_state_update(update);
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

static bool handle_updater_check_event(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypeUpdaterCheckEvent);
    publish_update_check(instance, &message->updater_check_state);
    return true;
}

static bool handle_busy_timer(StatePublisher* instance, const Message* message) {
    furi_assert(message->type == MessageTypeBusyTimer);
    publish_busy_timer(instance);
    return true;
}

static const MessageHandler message_handlers[] = {
    [MessageTypePublishUpdate] = handle_publish_update,
    [MessageTypePowerEvent] = handle_power_event,
    [MessageTypeAudioEvent] = handle_audio_event,
    [MessageTypeMatterEvent] = handle_matter_event,
    [MessageTypeUpdaterCheckEvent] = handle_updater_check_event,
    [MessageTypeBusyTimer] = handle_busy_timer,
};

static_assert(COUNT_OF(message_handlers) == MessageTypesCount);

static void schedule_state_update(
    StatePublisher* instance,
    BSB_State_StateUpdate* update,
    StreamFlag flags) {
    Message msg = {
        .type = MessageTypePublishUpdate,
        .update =
            {
                .data = update,
                .stream_flags = flags,
            },
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

    schedule_state_update(instance, update, StreamFlagAll);
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

    schedule_state_update(instance, update, StreamFlagAll);
}

static void publish_audio(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    FURI_LOG_D(TAG, "publish audio");

    float volume = audio_get_volume(instance->audio);

    update->which_state = BSB_State_StateUpdate_audio_volume_tag;

    update->state.audio_volume.volume = (uint8_t)roundf(volume * 100.0f);

    schedule_state_update(instance, update, StreamFlagAll);
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

    schedule_state_update(instance, update, StreamFlagAll);
}

static void publish_update_check(StatePublisher* instance, const UpdaterCheckState* info) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    update->which_state = BSB_State_StateUpdate_update_check_tag;

    FURI_LOG_D(TAG, "publish update check");

    switch(info->result) {
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
    schedule_state_update(instance, update, StreamFlagAll);
}

static void publish_busy_timer(StatePublisher* instance) {
    BSB_State_StateUpdate* update = malloc(sizeof(BSB_State_StateUpdate));
    FURI_LOG_D(TAG, "publish busy timer");

    update->which_state = BSB_State_StateUpdate_timer_tag;

    update->state.timer.has_json = true;
    update->state.timer.json.compression = BSB_Util_Compression_PLAIN;

    BusyTimerSnapshot snapshot;
    busy_timer_get_snapshot(instance->busy_timer, &snapshot);
    char* json_text = busy_timer_snapshot_serialize(&snapshot);
    size_t len = strlen(json_text);
    update->state.timer.json.data = malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(len));
    update->state.timer.json.data->size = len;
    memcpy(&update->state.timer.json.data->bytes, json_text, len);
    free(json_text);

    schedule_state_update(instance, update, StreamFlagAll);
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

    schedule_state_update(instance, update, StreamFlagAll);
}

static void matter_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    Message msg = {
        .type = MessageTypeMatterEvent,
    };
    send_message(instance, &msg);
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
    FURI_LOG_D(TAG, "publish input event");
    FURI_LOG_D(TAG, "input event: type=%u, key=%u", event->type, event->key);

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

    schedule_state_update(instance, update, StreamFlagAll);
}

static void busy_timer_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    Message msg = {
        .type = MessageTypeBusyTimer,
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

    schedule_state_update(instance, update, StreamFlagAll);
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
    case WifiStateConnecting:
    case WifiStateDisconnecting:
    case WifiStateReconnecting: {
        update->state.wifi.which_wifi_state = BSB_State_Wifi_connected_tag;

        // status
        static const BSB_State_WifiConnectionStatus status_lookup[] = {
            [WifiStateConnected] = BSB_State_WifiConnectionStatus_CONNECTED,
            [WifiStateConnecting] = BSB_State_WifiConnectionStatus_CONNECTING,
            [WifiStateDisconnecting] = BSB_State_WifiConnectionStatus_DISCONNECTING,
            [WifiStateReconnecting] = BSB_State_WifiConnectionStatus_RECONNECTING,
        };
        update->state.wifi.wifi_state.connected.status = status_lookup[info->state];

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
        update->state.wifi.wifi_state.connected.security = security_lookup[info->security_mode];

        // IP
        convert_ip_config(&update->state.wifi, &info->ip_config);

        break;
    }
    default:
        furi_assert(false);
        break;
    }

    schedule_state_update(instance, update, StreamFlagAll);
}

static void updater_update_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const UpdaterUpdateState* info = item;

    FURI_LOG_D(TAG, "publish update state");

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

    schedule_state_update(instance, update, StreamFlagAll);
}

static void updater_check_state_callback(const void* item, void* context) {
    StatePublisher* instance = context;
    const UpdaterCheckState* info = item;

    Message msg = {.type = MessageTypeUpdaterCheckEvent, .updater_check_state = *info};
    send_message(instance, &msg);
}

void screen_streamer_callback(
    GuiDisplayId display,
    const ScreenStreamerFrame* frame,
    uint8_t stream_flags,
    void* context) {
    StatePublisher* instance = context;
    FURI_LOG_D(
        TAG,
        "frame for %hhx: %lux%lu (%zu) pf:%u c:%u",
        stream_flags,
        frame->width,
        frame->height,
        frame->data_size,
        frame->pixel_format,
        frame->compression);

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
    schedule_state_update(instance, update, stream_flags);
}
