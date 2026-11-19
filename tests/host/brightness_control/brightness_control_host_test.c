#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "brightness_control.c"

struct FuriState {
    BrightnessControlState value;
    unsigned acquires;
    unsigned publishes;
};
struct FuriEventLoopTimer {
    bool is_running;
    unsigned starts;
    uint32_t interval;
    uint32_t arm_tick;
    unsigned arm_sequence;
    int type;
    void (*callback)(void*);
    void* context;
};
static uint32_t now;
static unsigned writes[BrightnessControlModuleMax];
static uint8_t outputs[BrightnessControlModuleMax];
static unsigned saves;
static unsigned sequence;
static unsigned output_sequence;
static unsigned state_sequence;
static uint32_t sink_delay;
static FuriEventLoopTimer timer;
static FuriState state;
static BrightnessControl instance;
const SettingProviderSetting brightness_v1_settings_root = {0};

FuriEventLoop* furi_event_loop_alloc(void) {
    return NULL;
}
FuriMessageQueue* furi_message_queue_alloc(size_t count, size_t size) {
    UNUSED(count);
    UNUSED(size);

    return NULL;
}
void furi_event_loop_subscribe_message_queue(
    FuriEventLoop* loop,
    FuriMessageQueue* queue,
    int event,
    void (*callback)(FuriEventLoopObject*, void*),
    void* context) {
    UNUSED(loop);
    UNUSED(queue);
    UNUSED(event);
    UNUSED(callback);
    UNUSED(context);
}
int furi_message_queue_put(FuriMessageQueue* queue, const void* item, uint32_t timeout) {
    UNUSED(queue);
    UNUSED(item);
    UNUSED(timeout);

    return FuriStatusOk;
}
int furi_message_queue_get(FuriMessageQueue* queue, void* item, uint32_t timeout) {
    UNUSED(queue);
    UNUSED(item);
    UNUSED(timeout);

    return FuriStatusOk;
}
void furi_event_loop_run(FuriEventLoop* loop) {
    UNUSED(loop);
}
void* furi_record_open(const char* name) {
    UNUSED(name);

    return NULL;
}
void furi_record_create(const char* name, void* record) {
    UNUSED(name);
    UNUSED(record);
}
FuriEventLoopTimer* furi_event_loop_timer_alloc(
    FuriEventLoop* loop,
    void (*callback)(void*),
    int type,
    void* context) {
    UNUSED(loop);
    timer.type = type;

    timer.callback = callback;
    timer.context = context;
    return &timer;
}
void furi_event_loop_timer_start(FuriEventLoopTimer* value, uint32_t interval) {
    value->interval = interval;
    value->arm_tick = now;
    value->arm_sequence = ++sequence;
    value->is_running = true;
    ++value->starts;
}
void furi_event_loop_timer_stop(FuriEventLoopTimer* value) {
    value->is_running = false;
}
bool furi_event_loop_timer_is_running(const FuriEventLoopTimer* value) {
    return value->is_running;
}
uint32_t furi_get_tick(void) {
    return now;
}
uint32_t furi_ms_to_ticks(uint32_t ms) {
    return ms;
}
FuriState* furi_state_alloc(size_t size) {
    assert(size == sizeof(state.value));
    return &state;
}
void* furi_state_acquire(FuriState* value) {
    ++value->acquires;
    return &value->value;
}
void furi_state_release(FuriState* value) {
    ++value->publishes;
    state_sequence = ++sequence;
}
void furi_state_get(FuriState* value, void* out) {
    memcpy(out, &value->value, sizeof(value->value));
}
void* furi_state_get_subscribe(
    FuriState* value,
    void* out,
    FuriStateCallback callback,
    void* context) {
    UNUSED(value);
    UNUSED(callback);
    UNUSED(context);

    *(LightSensorState*)out = (LightSensorState){.level = {0}};
    return NULL;
}
FuriState* light_sensor_get_state(LightSensor* sensor) {
    UNUSED(sensor);

    return NULL;
}
SettingProvider*
    setting_provider_alloc(const char* path, unsigned version, void* migration, size_t count) {
    UNUSED(path);
    UNUSED(version);
    UNUSED(migration);
    UNUSED(count);

    return NULL;
}
void setting_provider_load(
    SettingProvider* provider,
    const SettingProviderSetting* schema,
    void* out) {
    UNUSED(provider);
    UNUSED(schema);

    *(BrightnessSettings*)out =
        (BrightnessSettings){.mode = BrightnessControlBrightnessModeAuto, .brightness = 37};
}
void setting_provider_save(
    SettingProvider* provider,
    const SettingProviderSetting* schema,
    const void* value) {
    UNUSED(provider);
    UNUSED(schema);
    UNUSED(value);

    ++saves;
}
static void output(BrightnessControlModule module, uint8_t value) {
    output_sequence = ++sequence;
    ++writes[module];
    outputs[module] = value;
}
void front_display_set_brightness(FrontDisplaySrv* display, FrontDisplayBrightness value) {
    UNUSED(display);

    output(BrightnessControlModuleFrontDisplay, value.val);
    now += sink_delay;
}
void back_display_set_contrast(BackDisplaySrv* display, BackDisplayContrast value) {
    UNUSED(display);

    output(BrightnessControlModuleBackDisplay, value.val);
}
#if defined(SRV_STATUS_LIGHTS)
static StatusLightsStatus status_result;
static unsigned status_attempts;
StatusLightsStatus
    status_lights_set_brightness(StatusLights* lights, StatusLightsBrightness value) {
    UNUSED(lights);

    ++status_attempts;
    if(status_result == StatusLightsStatusOk) {
        output(BrightnessControlModuleStatusLights, value.val);
    }
    return status_result;
}
#endif

static void reset(uint32_t tick) {
    now = tick;
    memset(&timer, 0, sizeof(timer));
    memset(&state, 0, sizeof(state));
    memset(writes, 0, sizeof(writes));
    saves = 0;
    sequence = output_sequence = state_sequence = 0;
    sink_delay = 0;
    BrightnessControl* allocated = brightness_control_alloc();
    instance = *allocated;
    free(allocated);
    timer.context = &instance;
    assert(state.value.brightness_setting == 37);
    assert(state.value.effective_brightness == 0);
    assert(writes[BrightnessControlModuleFrontDisplay] == 1);
    assert(writes[BrightnessControlModuleBackDisplay] == 1);
}
static void sensor(uint8_t level) {
    Message message = {.type = MessageTypeLightSensor, .light_sensor_level = {level}};
    do_process_light_sensor(&instance, &message);
}
static void advance(uint32_t tick) {
    now = tick;
    assert(timer.callback);
    timer.is_running = false;
    timer.callback(timer.context);
}
static void manual(uint8_t brightness) {
    Message message = {.type = MessageTypeSetManualBrightness, .user_brightness = {brightness}};
    do_set_manual_brightness(&instance, &message);
}
static void auto_mode(void) {
    Message message = {.type = MessageTypeSetAutoBrightness};
    do_set_auto_brightness(&instance, &message);
}
static void override_module(BrightnessControlModule module, bool enabled, uint8_t brightness) {
    Message message = {
        .type = MessageTypeSetOverride,
        .override = {.module = module, .enabled = enabled, .value = {(float)brightness / 100}}};
    do_set_override(&instance, &message);
}
static void test_delay_one_step_and_rearm(void) {
    reset(0);
    sensor(15);
    sink_delay = 3000;
    advance(10000);
    assert(instance.ramp.current.val == 1.f / 128.f);
    assert(timer.is_running);
    assert(timer.interval == 16);
    assert(timer.type == FuriEventLoopTimerTypeOnce);
    assert(timer.arm_tick == now && now == 13000);
    assert(timer.arm_sequence > output_sequence);
    assert(timer.arm_sequence > state_sequence);
    sink_delay = 0;
    advance(now + 16);
    assert(instance.ramp.current.val == 2.f / 128.f);
}
static void test_full_span_duration(void) {
    reset(0);
    sensor(15);
    for(unsigned direction = 0; direction < 2; ++direction) {
        unsigned callbacks = 0;
        const unsigned front_start = writes[0], back_start = writes[1];
        while(timer.is_running) {
            advance(now + 16);
            assert(++callbacks <= 128);
        }
        assert(callbacks == 128);
        assert(instance.ramp.current.val == (direction ? 0.f : 1.f));
        assert(writes[0] - front_start == 99);
        assert(writes[1] - back_start == 70);
        if(!direction) sensor(0);
    }
    assert(now == 4096);
}
static void test_interior_curve_bounds_and_endpoints(void) {
    const float starts[] = {0.f, .001f, .05f, .123f, .49f, .5f, .777f, .999f, 1.f};
    for(unsigned start = 0; start < sizeof(starts) / sizeof(starts[0]); ++start) {
        for(unsigned level = 0; level <= 15; ++level) {
            reset(0);
            brightness_ramp_set(&instance.ramp, (InternalBrightness){starts[start]});
            apply_brightness(&instance);
            update_state(&instance);
            instance.last_light_sensor_level.val = (level + 1) % 16;
            sensor(level);
            unsigned callbacks = 0;
            while(timer.is_running) {
                uint8_t before[BrightnessControlModuleMax];
                memcpy(before, outputs, sizeof(before));
                const float previous = instance.ramp.current.val;
                advance(now + 16);
                assert(state.value.mode == BrightnessControlBrightnessModeAuto);
                assert(
                    state.value.effective_brightness ==
                    brightness_conv_internal_to_user(instance.ramp.current).val);
                assert(state.value.brightness_setting == 37);
                assert(fabsf(instance.ramp.current.val - previous) <= 1.f / 128.f + .000001f);
                assert(abs((int)outputs[0] - before[0]) <= 1);
                assert(abs((int)outputs[1] - before[1]) <= 1);
#if defined(SRV_STATUS_LIGHTS)
                assert(abs((int)outputs[2] - before[2]) <= 1);
#endif
                assert(++callbacks <= 128);
            }
            const InternalBrightness target =
                brightness_conv_light_sensor_to_internal((LightSensorLevel){level});
            assert(instance.ramp.current.val == target.val);
        }
    }
}
static void test_adjacent_levels(void) {
    reset(0);
    brightness_ramp_set(
        &instance.ramp, brightness_conv_light_sensor_to_internal((LightSensorLevel){7}));
    apply_brightness(&instance);
    sensor(8);
    unsigned callbacks = 0;
    while(timer.is_running) {
        advance(now + 16);
        ++callbacks;
    }
    assert(callbacks == 9 && now == 144);
}
static void test_retarget_reversal_independent_of_clock(void) {
    reset(UINT32_MAX - 8);
    sensor(15);
    advance(7);
    assert(instance.ramp.current.val == 1.f / 128.f);
    const unsigned starts = timer.starts;
    sensor(15);
    assert(timer.starts == starts);
    sensor(0);
    assert(instance.ramp.current.val == 1.f / 128.f);
    advance(23);
    assert(instance.ramp.current.val == 0.f && !timer.is_running);
    sensor(15);
    advance(39);
    sensor(7);
    const float applied = instance.ramp.current.val;
    advance(55);
    assert(instance.ramp.current.val == applied + 1.f / 128.f);
}
static void test_manual_auto_overrides_and_dedup(void) {
    reset(0);
    sensor(15);
    advance(16);
    const float applied = instance.ramp.current.val;
    override_module(BrightnessControlModuleFrontDisplay, true, 80);
    assert(instance.ramp.current.val == applied && timer.is_running);
    const unsigned front = writes[0];
    advance(32);
    assert(writes[0] == front);
    override_module(BrightnessControlModuleFrontDisplay, false, 0);
    assert(outputs[0] == brightness_conv_internal_to_front(instance.ramp.current).val);
    override_module(BrightnessControlModuleBackDisplay, true, 75);
    const unsigned back_writes = writes[1];
    advance(now + 16);
    assert(writes[1] == back_writes && timer.is_running);
    override_module(BrightnessControlModuleBackDisplay, false, 0);
    assert(outputs[1] == brightness_conv_internal_to_back(instance.ramp.current).val);
#if defined(SRV_STATUS_LIGHTS)
    override_module(BrightnessControlModuleStatusLights, true, 75);
    const unsigned status_writes = writes[2];
    advance(now + 16);
    assert(writes[2] == status_writes && timer.is_running);
    override_module(BrightnessControlModuleStatusLights, false, 0);
    assert(outputs[2] == brightness_conv_internal_to_status(instance.ramp.current).val);
#endif
    const unsigned publishes = state.publishes, acquires = state.acquires;
    const unsigned f = writes[0], b = writes[1];
    apply_brightness(&instance);
    update_state(&instance);
    assert(writes[0] == f && writes[1] == b);
    assert(state.publishes == publishes && state.acquires == acquires);
    manual(23);
    assert(!timer.is_running && state.value.brightness_setting == 23);
    assert(state.value.effective_brightness == 23);
    sensor(7);
    assert(state.value.effective_brightness == 23 && !timer.is_running);
    auto_mode();
    assert(!timer.is_running && state.value.effective_brightness == 47);
    sensor(15);
    advance(now + 16);
    assert(state.value.brightness_setting == 23);
    auto_mode();
    assert(!timer.is_running && state.value.effective_brightness == 100);
    assert(state.value.brightness_setting == 23 && saves == 3);
}
#if defined(SRV_STATUS_LIGHTS)
static void test_status_error_retry(void) {
    reset(0);
    const StatusLightsStatus failures[] = {StatusLightsStatusTimeout, StatusLightsStatusError};
    for(unsigned i = 0; i < sizeof(failures) / sizeof(failures[0]); ++i) {
        const unsigned attempts = status_attempts;
        const uint8_t target = 30 + 40 * i;
        status_result = failures[i];
        override_module(BrightnessControlModuleStatusLights, true, target);
        assert(status_attempts == attempts + 1);
        status_result = StatusLightsStatusOk;
        override_module(BrightnessControlModuleStatusLights, true, target);
        assert(status_attempts == attempts + 2);
        override_module(BrightnessControlModuleStatusLights, true, target);
        assert(status_attempts == attempts + 2);
    }
}
#endif
int main(void) {
    test_delay_one_step_and_rearm();
    test_full_span_duration();
    test_interior_curve_bounds_and_endpoints();
    test_adjacent_levels();
    test_retarget_reversal_independent_of_clock();
    test_manual_auto_overrides_and_dedup();
#if defined(SRV_STATUS_LIGHTS)
    test_status_error_retry();
#endif
    puts("adaptive actual-production host tests passed");
    return 0;
}
