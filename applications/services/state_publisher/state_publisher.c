#include "state_publisher.h"
#include <furi/furi.h>
#include <dyn_buffer.h>
#include <brightness_control/brightness_control.h>
#include <power/power_service/power.h>
#include <sntp/sntp.h>
#include <sntp/sntp.h>
#include <nanopb/pb.h>
#include <nanopb/pb_encode.h>
#include <state.pb.h>

#define TAG "StPubSrv"

#define MAX_MESSAGES 16

struct StatePublisher {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    Power* power;
};

typedef enum {
    MessageTypePublishUpdate,
    MessageTypePowerEvent,

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
static void power_pubsub_callback(const void* message, void* context);

static void publish_power(StatePublisher* instance);

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
        FuriState* brightness_state = brightness_control_get_state(brightness_control);
        furi_state_subscribe(brightness_state, brightness_state_callback, instance);
    }
    {
        instance->power = furi_record_open(RECORD_POWER);
        FuriPubSub* power_pubsub = power_get_pubsub(instance->power);
        furi_pubsub_subscribe(power_pubsub, power_pubsub_callback, instance);
    }
    {
        Sntp* sntp = furi_record_open(RECORD_SNTP);
        FuriState* state = sntp_get_settings_state(sntp);
        furi_state_subscribe(state, sntp_settings_state_callback, instance);
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

static const MessageHandler message_handlers[] = {
    [MessageTypePublishUpdate] = handle_publish_update,
    [MessageTypePowerEvent] = handle_power_event,
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

static void power_pubsub_callback(const void* message, void* context) {
    UNUSED(message);
    StatePublisher* instance = context;

    // dispatch because power_get_info cannot be called from power task
    Message msg = {
        .type = MessageTypePowerEvent,
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
