#include "brightness_control.h"
#include <furi/furi.h>
#include <light_sensor/light_sensor.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#include <storage/storage.h>
#include <json_helper.h>

#define TAG "BrightCtrl"

#define CONFIG_FILE APP_DATA_PATH("config.json")

#define MAX_MESSAGES                4
#define LIGHT_SENSOR_UPDATE_TIMEOUT 10

#define DEFAULT_BRIGHTNESS \
    (UserBrightness) {     \
        50                 \
    }

struct BrightnessControl {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;

    FrontDisplaySrv* front_display;
    BackDisplaySrv* back_display;
    StatusLights* status_lights;

    FuriPubSub* light_sensor_events;

    FuriState* state;

    bool is_auto;
    UserBrightness manual_brightness;
    LightSensorLevel last_light_sensor_level;

    bool is_overridden[BrightnessControlModuleMax];
    InternalBrightness brightness_override[BrightnessControlModuleMax];
};

typedef enum {
    MessageTypeSetManualBrightness,
    MessageTypeSetAutoBrightness,
    MessageTypeLightSensor,
    MessageTypeSetOverride,

    MessageTypesCount,
} MessageType;

typedef struct Message {
    MessageType type;

    union {
        InternalBrightness brightness;
        UserBrightness user_brightness;
        LightSensorLevel light_sensor_level;
        struct {
            bool enabled;
            BrightnessControlModule module;
            InternalBrightness value;
        } override;
    };
} Message;

typedef void (*MessageHandler)(BrightnessControl* instance, Message* message);

static const MessageHandler message_handlers[];

static void light_sensor_event(const void* message, void* context);

// uint8_t brightness_control_get_brightness(BrightnessControl* inst);

void brightness_control_set_auto_brightness(BrightnessControl* inst) {
    Message msg = {
        .type = MessageTypeSetAutoBrightness,
    };

    furi_message_queue_put(inst->message_queue, &msg, FuriWaitForever);
}

void brightness_control_set_manual_brightness(BrightnessControl* inst, UserBrightness brightness) {
    Message msg = {
        .type = MessageTypeSetManualBrightness,
        .user_brightness = brightness,
    };

    furi_message_queue_put(inst->message_queue, &msg, FuriWaitForever);
}

FuriState* brightness_control_get_state(const BrightnessControl* instance) {
    return instance->state;
}

void brightness_control_set_brightness_override(
    BrightnessControl* inst,
    BrightnessControlModule module,
    const UserBrightness* override) {
    Message msg = {
        .type = MessageTypeSetOverride,
        .override =
            {
                .enabled = override != NULL,
                .module = module,
                .value = override ? brightness_conv_user_to_internal(*override) :
                                    (InternalBrightness){0},
            },
    };

    furi_message_queue_put(inst->message_queue, &msg, FuriWaitForever);
}

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    BrightnessControl* inst = context;
    UNUSED(inst);

    Message message;
    furi_check(furi_message_queue_get(inst->message_queue, &message, 0) == FuriStatusOk);

    message_handlers[message.type](inst, &message);
}

static void apply_brightness(const BrightnessControl* inst);
static void load_config(BrightnessControl* inst);
static void save_config(const BrightnessControl* inst);
static InternalBrightness get_effective_brightness(const BrightnessControl* inst);

static BrightnessControl* brightness_control_alloc(void) {
    BrightnessControl* inst = malloc(sizeof(BrightnessControl));

    inst->event_loop = furi_event_loop_alloc();
    inst->message_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(Message));
    furi_event_loop_subscribe_message_queue(
        inst->event_loop, inst->message_queue, FuriEventLoopEventIn, message_queue_callback, inst);

    inst->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    inst->back_display = furi_record_open(RECORD_BACK_DISPLAY);
    inst->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);

#if defined(SRV_LIGHT_SENSOR)
    inst->light_sensor_events = furi_record_open(RECORD_LIGHT_SENSOR_EVENTS);
    furi_pubsub_subscribe(inst->light_sensor_events, light_sensor_event, inst);
    inst->last_light_sensor_level = light_sensor_get_light_level();
#else
    UNUSED(light_sensor_event);
#endif
    load_config(inst);

    bzero(inst->is_overridden, sizeof(inst->is_overridden));

    inst->state = furi_state_alloc(sizeof(BrightnessControlState));
    furi_state_set(
        inst->state,
        &(BrightnessControlState){
            .mode = inst->is_auto ? BrightnessControlBrightnessModeAuto :
                                    BrightnessControlBrightnessModeManual,
            .brightness_setting = inst->manual_brightness,
            .effective_brightness = get_effective_brightness(inst)});

    apply_brightness(inst);

    return inst;
}

int brightness_control_srv(void* arg) {
    UNUSED(arg);

    BrightnessControl* inst = brightness_control_alloc();
    furi_record_create(RECORD_BRIGHTNESS_CONTROL, inst);
    furi_event_loop_run(inst->event_loop);

    return 0;
}

static void light_sensor_event(const void* message, void* context) {
    UNUSED(message);
    furi_assert(context);

    BrightnessControl* inst = context;

    const LightSensorEvent* event = message;
    if(event->type != LightSensorEventTypeLightLevelChanged) {
        return;
    }

    Message msg = {
        .type = MessageTypeLightSensor,
        .light_sensor_level = event->light_level,
    };

    furi_message_queue_put(inst->message_queue, &msg, LIGHT_SENSOR_UPDATE_TIMEOUT);
}

static void load_config(BrightnessControl* inst) {
    inst->is_auto = false;
    inst->manual_brightness = DEFAULT_BRIGHTNESS;

    JsonConfig* cfg = json_config_alloc();

    do {
        if(json_config_open(cfg, CONFIG_FILE) == JsonConfigStatusError) {
            // be resilient and not crash
            FURI_LOG_E(TAG, "Cannot open config file");
            break;
        }

#if defined(SRV_LIGHT_SENSOR)
        FuriString* mode = furi_string_alloc();
        json_config_read_str(cfg, "mode", mode, "manual");
        if(furi_string_cmp_str(mode, "auto") == 0) {
            inst->is_auto = true;
        }
        furi_string_free(mode);
#endif

        int brightness = DEFAULT_BRIGHTNESS.val;
        json_config_read_int(cfg, "brightness", &brightness, NULL);
        inst->manual_brightness = brightness_conv_int_to_user(brightness);
    } while(false);

    json_config_free(cfg);
}

static void save_config(const BrightnessControl* inst) {
    JsonConfig* cfg = json_config_alloc();

    do {
        if(json_config_open(cfg, CONFIG_FILE) == JsonConfigStatusError) {
            // be resilient and not crash
            FURI_LOG_E(TAG, "Cannot open config file");
            break;
        }

        json_config_write_str(cfg, "mode", inst->is_auto ? "auto" : "manual");
        json_config_write_int(cfg, "brightness", inst->manual_brightness.val);
    } while(false);

    json_config_free(cfg);
}

static InternalBrightness get_effective_brightness(const BrightnessControl* inst) {
    if(inst->is_auto) {
        return brightness_conv_light_sensor_to_internal(inst->last_light_sensor_level);
    } else {
        return brightness_conv_user_to_internal(inst->manual_brightness);
    }
}

static InternalBrightness apply_override(
    const BrightnessControl* inst,
    InternalBrightness br,
    BrightnessControlModule module) {
    if(inst->is_overridden[module]) {
        return inst->brightness_override[module];
    } else {
        return br;
    }
}

static void apply_brightness(const BrightnessControl* inst) {
    InternalBrightness br = get_effective_brightness(inst);

    FURI_LOG_D(TAG, "Set brightness %hhu", br.val);
    front_display_set_brightness(
        inst->front_display,
        brightness_conv_internal_to_front(
            apply_override(inst, br, BrightnessControlModuleFrontDisplay)));
    back_display_set_contrast(
        inst->back_display,
        brightness_conv_internal_to_back(
            apply_override(inst, br, BrightnessControlModuleBackDisplay)));
    status_lights_set_brightness(
        inst->status_lights,
        brightness_conv_internal_to_status(
            apply_override(inst, br, BrightnessControlModuleStatusLights)));
}

static void update_state(const BrightnessControl* inst) {
    with_furi_state(inst->state, BrightnessControlState * state, {
        state->mode = inst->is_auto ? BrightnessControlBrightnessModeAuto :
                                      BrightnessControlBrightnessModeManual;
        state->effective_brightness = get_effective_brightness(inst);
        state->brightness_setting = inst->manual_brightness;
    });
}

static void do_set_manual_brightness(BrightnessControl* inst, Message* message) {
    furi_assert(message->type == MessageTypeSetManualBrightness);

    inst->is_auto = false;
    inst->manual_brightness = message->user_brightness;
    apply_brightness(inst);
    save_config(inst);
    update_state(inst);
}

static void do_set_auto_brightness(BrightnessControl* inst, Message* message) {
    furi_assert(message->type == MessageTypeSetAutoBrightness);

#if defined(SRV_LIGHT_SENSOR)
    inst->is_auto = true;
    apply_brightness(inst);
    save_config(inst);
    update_state(inst);
#else
    UNUSED(inst);
#endif
}

static void do_process_light_sensor(BrightnessControl* inst, Message* message) {
    furi_assert(message->type == MessageTypeLightSensor);

#if defined(SRV_LIGHT_SENSOR)
    FURI_LOG_I(TAG, "Light sensor brightness: %hhu", message->light_sensor_level.val);
    inst->last_light_sensor_level = message->light_sensor_level;
    if(inst->is_auto) {
        apply_brightness(inst);
        update_state(inst);
    }
#else
    UNUSED(inst);
#endif
}

static void do_set_override(BrightnessControl* inst, Message* message) {
    furi_assert(message->type == MessageTypeSetOverride);

    inst->is_overridden[message->override.module] = message->override.enabled;
    inst->brightness_override[message->override.module] = message->override.value;

    apply_brightness(inst);
}

static const MessageHandler message_handlers[] = {
    [MessageTypeSetManualBrightness] = do_set_manual_brightness,
    [MessageTypeSetAutoBrightness] = do_set_auto_brightness,
    [MessageTypeLightSensor] = do_process_light_sensor,
    [MessageTypeSetOverride] = do_set_override,
};
