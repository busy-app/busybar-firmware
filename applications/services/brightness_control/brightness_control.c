#include "brightness_control.h"
#include "brightness_conv.h"
#include "brightness_ramp_i.h"
#include "settings/settings.h"
#include <furi/furi.h>
#include <light_sensor/light_sensor.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#if defined(SRV_STATUS_LIGHTS)
#include <status_lights/status_lights.h>
#endif

#define TAG "BrightCtrl"

#define MAX_MESSAGES                      4
#define LIGHT_SENSOR_UPDATE_TIMEOUT       10
#define BRIGHTNESS_TRANSITION_INTERVAL_MS (16U)

#define DEFAULT_BRIGHTNESS \
    (UserBrightness) {     \
        50                 \
    }

struct BrightnessControl {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    FuriEventLoopTimer* transition_timer;
    BrightnessRamp ramp;
    bool has_applied_output[BrightnessControlModuleMax];
    uint8_t applied_output[BrightnessControlModuleMax];
    bool has_published_state;
    BrightnessControlState published_state;

    FrontDisplaySrv* front_display;
    BackDisplaySrv* back_display;
#if defined(SRV_STATUS_LIGHTS)
    StatusLights* status_lights;
#endif

    FuriState* state;

    SettingProvider* setting_provider;

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

static void light_sensor_state_callback(const void* message, void* context);

void brightness_control_set_auto_brightness(BrightnessControl* instance) {
    Message msg = {
        .type = MessageTypeSetAutoBrightness,
    };

    furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever);
}

void brightness_control_set_manual_brightness(BrightnessControl* instance, uint8_t brightness) {
    Message msg = {
        .type = MessageTypeSetManualBrightness,
        .user_brightness = brightness_conv_int_to_user_clamped(brightness),
    };

    furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever);
}

FuriState* brightness_control_get_state(const BrightnessControl* instance) {
    return instance->state;
}

void brightness_control_set_brightness_override(
    BrightnessControl* instance,
    BrightnessControlModule module,
    uint8_t override) {
    UserBrightness user_brightness = brightness_conv_int_to_user_clamped(override);
    Message msg = {
        .type = MessageTypeSetOverride,
        .override =
            {
                .enabled = true,
                .module = module,
                .value = brightness_conv_user_to_internal(user_brightness),
            },
    };

    furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever);
}

void brightness_control_reset_brightness_override(
    BrightnessControl* instance,
    BrightnessControlModule module) {
    Message msg = {
        .type = MessageTypeSetOverride,
        .override =
            {
                .enabled = false,
                .module = module,
            },
    };

    furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever);
}

static void message_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    BrightnessControl* instance = context;
    UNUSED(instance);

    Message message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    message_handlers[message.type](instance, &message);
}

static void apply_brightness(BrightnessControl* instance);
static void brightness_control_transition_timer_callback(void* context);
static void brightness_control_apply_immediate(BrightnessControl* instance);
static void load_config(BrightnessControl* instance);
static void save_config(const BrightnessControl* instance);
static InternalBrightness get_effective_brightness(const BrightnessControl* instance);
static void update_state(BrightnessControl* instance);

static BrightnessControl* brightness_control_alloc(void) {
    BrightnessControl* instance = malloc(sizeof(BrightnessControl));

    instance->event_loop = furi_event_loop_alloc();
    instance->transition_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        brightness_control_transition_timer_callback,
        FuriEventLoopTimerTypeOnce,
        instance);
    instance->message_queue = furi_message_queue_alloc(MAX_MESSAGES, sizeof(Message));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        message_queue_callback,
        instance);

    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);
#if defined(SRV_STATUS_LIGHTS)
    instance->status_lights = furi_record_open(RECORD_STATUS_LIGHTS);
#endif

#if defined(SRV_LIGHT_SENSOR)
    LightSensor* light_sensor = furi_record_open(RECORD_LIGHT_SENSOR);
    LightSensorState light_sensor_state;
    furi_state_get_subscribe(
        light_sensor_get_state(light_sensor),
        &light_sensor_state,
        light_sensor_state_callback,
        instance);
    instance->last_light_sensor_level = light_sensor_state.level;
#else
    UNUSED(light_sensor_state_callback);
    instance->last_light_sensor_level = (LightSensorLevel){0};
#endif

    instance->setting_provider = setting_provider_alloc(
        BRIGHTNESS_SETTINGS_FILE_PATH, BRIGHTNESS_SETTINGS_VERSION, NULL, 0);
    load_config(instance);

    instance->state = furi_state_alloc(sizeof(BrightnessControlState));

    brightness_control_apply_immediate(instance);

    return instance;
}

int brightness_control_srv(void* arg) {
    UNUSED(arg);

    BrightnessControl* instance = brightness_control_alloc();
    furi_record_create(RECORD_BRIGHTNESS_CONTROL, instance);
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static void light_sensor_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    BrightnessControl* instance = context;
    const LightSensorState* state = item;

    const Message msg = {
        .type = MessageTypeLightSensor,
        .light_sensor_level = state->level,
    };

    furi_message_queue_put(instance->message_queue, &msg, LIGHT_SENSOR_UPDATE_TIMEOUT);
}

static void load_config(BrightnessControl* instance) {
    BrightnessSettings settings;

    setting_provider_load(instance->setting_provider, &BRIGHTNESS_SETTINGS_ROOT, &settings);

#if defined(SRV_LIGHT_SENSOR)
    instance->is_auto = settings.mode == BrightnessControlBrightnessModeAuto;
#else
    instance->is_auto = false;
#endif
    instance->manual_brightness = brightness_conv_int_to_user_clamped(settings.brightness);
}

static void save_config(const BrightnessControl* instance) {
    BrightnessSettings settings = {
        .brightness = instance->manual_brightness.val,
        .mode = instance->is_auto ? BrightnessControlBrightnessModeAuto :
                                    BrightnessControlBrightnessModeManual,
    };

    setting_provider_save(instance->setting_provider, &BRIGHTNESS_SETTINGS_ROOT, &settings);
}

static InternalBrightness get_effective_brightness(const BrightnessControl* instance) {
#if defined(SRV_LIGHT_SENSOR)
    if(instance->is_auto) {
        return brightness_conv_light_sensor_to_internal(instance->last_light_sensor_level);
    } else {
        return brightness_conv_user_to_internal(instance->manual_brightness);
    }
#else
    return brightness_conv_user_to_internal(instance->manual_brightness);
#endif
}

static InternalBrightness apply_override(
    const BrightnessControl* instance,
    InternalBrightness br,
    BrightnessControlModule module) {
    if(instance->is_overridden[module]) {
        return instance->brightness_override[module];
    } else {
        return br;
    }
}

static bool brightness_control_output_changed(
    BrightnessControl* instance,
    BrightnessControlModule module,
    uint8_t value) {
    const bool has_changed = !instance->has_applied_output[module] ||
                             instance->applied_output[module] != value;
    instance->has_applied_output[module] = true;
    instance->applied_output[module] = value;
    return has_changed;
}

static void apply_brightness(BrightnessControl* instance) {
    const InternalBrightness br = instance->ramp.current;
    const FrontDisplayBrightness front = brightness_conv_internal_to_front(
        apply_override(instance, br, BrightnessControlModuleFrontDisplay));
    if(brightness_control_output_changed(
           instance, BrightnessControlModuleFrontDisplay, front.val)) {
        front_display_set_brightness(instance->front_display, front);
    }
    const BackDisplayContrast back = brightness_conv_internal_to_back(
        apply_override(instance, br, BrightnessControlModuleBackDisplay));
    if(brightness_control_output_changed(instance, BrightnessControlModuleBackDisplay, back.val)) {
        back_display_set_contrast(instance->back_display, back);
    }
#if defined(SRV_STATUS_LIGHTS)
    const StatusLightsBrightness status = brightness_conv_internal_to_status(
        apply_override(instance, br, BrightnessControlModuleStatusLights));
    if(brightness_control_output_changed(
           instance, BrightnessControlModuleStatusLights, status.val)) {
        if(status_lights_set_brightness(instance->status_lights, status) != StatusLightsStatusOk) {
            instance->has_applied_output[BrightnessControlModuleStatusLights] = false;
        }
    }
#endif
}

static void update_state(BrightnessControl* instance) {
    const BrightnessControlState next = {
        .mode = instance->is_auto ? BrightnessControlBrightnessModeAuto :
                                    BrightnessControlBrightnessModeManual,
        .effective_brightness = brightness_conv_internal_to_user(instance->ramp.current).val,
        .brightness_setting = instance->manual_brightness.val,
    };
    if(!instance->has_published_state || next.mode != instance->published_state.mode ||
       next.effective_brightness != instance->published_state.effective_brightness ||
       next.brightness_setting != instance->published_state.brightness_setting) {
        instance->published_state = next;
        instance->has_published_state = true;
        with_furi_state(instance->state, BrightnessControlState * state, { *state = next; });
    }
}

static void brightness_control_apply_immediate(BrightnessControl* instance) {
    furi_event_loop_timer_stop(instance->transition_timer);
    brightness_ramp_set(&instance->ramp, get_effective_brightness(instance));
    apply_brightness(instance);
    update_state(instance);
}

static void brightness_control_transition_timer_callback(void* context) {
    furi_assert(context);
    BrightnessControl* instance = context;
    brightness_ramp_step(&instance->ramp);
    apply_brightness(instance);
    update_state(instance);
    if(instance->ramp.is_active) {
        furi_event_loop_timer_start(
            instance->transition_timer, furi_ms_to_ticks(BRIGHTNESS_TRANSITION_INTERVAL_MS));
    }
}

static void do_set_manual_brightness(BrightnessControl* instance, Message* message) {
    furi_assert(message->type == MessageTypeSetManualBrightness);

    instance->is_auto = false;
    instance->manual_brightness = message->user_brightness;
    brightness_control_apply_immediate(instance);
    save_config(instance);
}

static void do_set_auto_brightness(BrightnessControl* instance, Message* message) {
    furi_assert(message->type == MessageTypeSetAutoBrightness);

#if defined(SRV_LIGHT_SENSOR)
    instance->is_auto = true;
    brightness_control_apply_immediate(instance);
    save_config(instance);
#else
    UNUSED(instance);
#endif
}

static void do_process_light_sensor(BrightnessControl* instance, Message* message) {
    furi_assert(message->type == MessageTypeLightSensor);

#if defined(SRV_LIGHT_SENSOR)
    if(instance->last_light_sensor_level.val != message->light_sensor_level.val) {
        FURI_LOG_I(TAG, "Light sensor brightness: %hhu", message->light_sensor_level.val);
        instance->last_light_sensor_level = message->light_sensor_level;

        if(instance->is_auto) {
            if(brightness_ramp_target(&instance->ramp, get_effective_brightness(instance))) {
                if(instance->ramp.is_active) {
                    furi_event_loop_timer_start(
                        instance->transition_timer,
                        furi_ms_to_ticks(BRIGHTNESS_TRANSITION_INTERVAL_MS));
                } else {
                    furi_event_loop_timer_stop(instance->transition_timer);
                }
            }
        }
    }
#else
    UNUSED(instance);
#endif
}

static void do_set_override(BrightnessControl* instance, Message* message) {
    furi_assert(message->type == MessageTypeSetOverride);

    instance->is_overridden[message->override.module] = message->override.enabled;
    instance->brightness_override[message->override.module] = message->override.value;

    apply_brightness(instance);
}

static const MessageHandler message_handlers[] = {
    [MessageTypeSetManualBrightness] = do_set_manual_brightness,
    [MessageTypeSetAutoBrightness] = do_set_auto_brightness,
    [MessageTypeLightSensor] = do_process_light_sensor,
    [MessageTypeSetOverride] = do_set_override,
};
