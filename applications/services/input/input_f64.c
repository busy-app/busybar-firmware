#include <furi.h>

#include <furi_hal_qei.h>
#include <furi_hal_resources.h>

#include <intercom/intercom.h>

#define TAG "Input"

#define INPUT_DEBOUNCE_TIMEOUT 1
#define INPUT_DEBOUNCE_TICKS   4
#define INPUT_QUEUE_SIZE       15

#ifdef INPUT_DEBUG
#define INPUT_LOG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define INPUT_LOG(...)
#endif

typedef enum {
    InputButtonActionPress,
    InputButtonActionRelease,
} InputButtonAction;

typedef struct {
    InputKey key;
    union {
        InputButtonAction button_action;
        InputSwitchPosition switch_position;
        int16_t encoder_delta;
    };
} InputEvent;

typedef enum {
    InputEventFlagActivity = 1 << 0,
} InputEventFlag;

typedef struct {
    const InputPin* pin;
    uint16_t debounce_count;
    bool level;
} InputKeyState;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* debounce_timer;
    InputKeyState* key_states;
    Intercom* intercom;
} InputSrv;

static void input_isr_key(void* context) {
    InputSrv* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, InputEventFlagActivity);
}

static void input_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    InputSrv* instance = context;

    if(events & InputEventFlagActivity) {
        if(!furi_event_loop_timer_is_running(instance->debounce_timer)) {
            furi_event_loop_timer_start(instance->debounce_timer, INPUT_DEBOUNCE_TIMEOUT);
        }
    }
}

static void input_send(InputSrv* instance, const InputPin* pin, InputButtonAction input_type) {
    InputEvent event;
    event.key = pin->key;

    if(pin->key == InputKeySwitch) {
        if(input_type == InputButtonActionPress) {
            event.switch_position = pin->switch_position;
            furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
        }

    } else {
        event.button_action = input_type;
        furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
    }
}

static FURI_ALWAYS_INLINE bool input_get_pin_level(const InputPin* input_pin) {
    return furi_hal_gpio_read(input_pin->gpio) ^ input_pin->inverted;
}

static void input_debounce_timer_callback(void* context) {
    furi_assert(context);

    InputSrv* instance = context;
    bool is_changing = false;

    for(size_t i = 0; i < input_pins_count; i++) {
        InputKeyState* state = &instance->key_states[i];
        const bool current_level = input_get_pin_level(state->pin);

        if(current_level) {
            if(state->debounce_count < INPUT_DEBOUNCE_TICKS) {
                state->debounce_count++;
                is_changing = true;
            }

        } else if(state->debounce_count > 0) {
            state->debounce_count--;
            is_changing = true;
        }

        if(!is_changing && state->level != current_level) {
            state->level = current_level;
            input_send(
                instance,
                state->pin,
                current_level ? InputButtonActionPress : InputButtonActionRelease);
        }
    }

    if(!is_changing) {
        furi_event_loop_timer_stop(instance->debounce_timer);
    }
}

static void input_qei_callback(int16_t delta_pos, void* context) {
    InputSrv* instance = context;

    InputEvent event = {
        .key = InputKeyEncoder,
        .encoder_delta = delta_pos,
    };

    furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
}

static void input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    InputSrv* instance = context;
    furi_assert(object == instance->input_queue);

    InputEvent event;
    furi_check(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk);

#ifdef INPUT_DEBUG
    if(event.key < InputKeySwitch) {
        INPUT_LOG(
            "Key %s, event %s",
            input_pins[event.key].name,
            event.type == InputTypePress ? "press" : "release");

    } else if(event.key == InputKeyEncoder) {
        INPUT_LOG("Encoder turn %d", event.delta);

    } else if(event.key == InputKeySwitch) {
        INPUT_LOG(
            "Switch %s %d, event %s",
            input_pins[event.position + InputKeySwitch].name,
            event.position,
            "press");

    } else {
        furi_crash();
    }
#endif

    intercom_tx(instance->intercom, IntercomChannelInput, &event, sizeof(event), FuriWaitForever);
}

int32_t input_srv(void* p) {
    UNUSED(p);

    INPUT_LOG("Starting");

    InputSrv* instance = malloc(sizeof(InputSrv));

    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_SIZE, sizeof(InputEvent));
    instance->event_loop = furi_event_loop_alloc();
    instance->debounce_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        input_debounce_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    instance->key_states = malloc(sizeof(InputKeyState) * input_pins_count);
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    for(size_t i = 0; i < input_pins_count; i++) {
        const InputPin* pin = &input_pins[i];
        InputKeyState* state = &instance->key_states[i];

        state->pin = pin;
        state->level = input_get_pin_level(pin);

        furi_hal_gpio_add_int_callback(pin->gpio, pin->condition, input_isr_key, instance);
    }

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, input_custom_event_callback, instance);
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        input_queue_callback,
        instance);

    furi_hal_qei_init();
    furi_hal_qei_set_delta_pos_callback(input_qei_callback, instance);

    // Start Input Service
    furi_event_loop_run(instance->event_loop);

    return 0;
}
