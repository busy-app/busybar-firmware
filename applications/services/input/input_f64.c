#include "input_f64.h"

#include <furi.h>
#include <furi_hal_qei.h>
#include <furi_hal_resources.h>
#include <toolbox/api_lock.h>

#ifdef SRV_INTERCOM
#include <intercom/intercom.h>
#endif

#define TAG "Input"

#define INPUT_DEBOUNCE_TIMEOUT    2
#define INPUT_DEBOUNCE_TICKS      10
#define INPUT_QUEUE_SIZE          32
#define REQUEST_QUEUE_SIZE        4
#define INPUT_INTERCOM_TIMEOUT_MS 100

#ifdef INPUT_DEBUG
#define INPUT_LOG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define INPUT_LOG(...)
#endif

typedef enum {
    InputEventFlagActivity = 1 << 0,
} InputEventFlag;

typedef struct {
    const InputPin* pin;
    uint16_t debounce_count;
    bool level;
} InputKeyState;

struct Input {
    FuriPubSub* event_pubsub;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* debounce_timer;
    InputKeyState* key_states;
    FuriState* absolute_state;
#ifdef SRV_INTERCOM
    IntercomChannel* intercom_ch;
    _Atomic IntercomStatus intercom_status;
#endif // SRV_INTERCOM
};

static void input_send(Input* instance, const InputPin* pin, InputAction input_action);

static void input_isr_key(void* context) {
    Input* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, InputEventFlagActivity);
}

static void input_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Input* instance = context;

    if(events & InputEventFlagActivity) {
        if(!furi_event_loop_timer_is_running(instance->debounce_timer)) {
            furi_event_loop_timer_start(instance->debounce_timer, INPUT_DEBOUNCE_TIMEOUT);
        }
    }
}

static void input_send(Input* instance, const InputPin* pin, InputAction input_action) {
    InputCommonEvent event;
    event.device = pin->device;

    if(pin->device == InputDeviceSwitch) {
        if(input_action == InputActionPress) {
            event.switch_position = pin->pos;
            with_furi_state(instance->absolute_state, InputAbsoluteState * st, {
                st->switch_position = pin->pos;
            });
            furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
        }
    } else {
        event.button_event.button = pin->button;
        event.button_event.action = input_action;

        if(input_action == InputActionPress) {
            with_furi_state(instance->absolute_state, InputAbsoluteState * st, {
                st->buttons |= (1 << pin->button);
            });
        } else {
            with_furi_state(instance->absolute_state, InputAbsoluteState * st, {
                st->buttons &= ~(1 << pin->button);
            });
        }

        furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
    }
}

static FURI_ALWAYS_INLINE bool input_get_pin_level(const InputPin* input_pin) {
    return !furi_hal_gpio_read(input_pin->gpio);
}

static void input_debounce_timer_callback(void* context) {
    furi_assert(context);

    Input* instance = context;
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
                instance, state->pin, current_level ? InputActionPress : InputActionRelease);
        }
    }

    if(!is_changing) {
        furi_event_loop_timer_stop(instance->debounce_timer);
    }
}

static void input_qei_callback(int16_t delta_pos, void* context) {
    Input* instance = context;

    InputCommonEvent event = {
        .device = InputDeviceEncoder,
        .encoder_delta = delta_pos,
    };

    furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
}

#ifdef SRV_INTERCOM

static void input_intercom_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    Input* instance = context;
    instance->intercom_status = *(IntercomStatus*)item;
}

static void input_init_intercom(Input* instance) {
    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(intercom, IntercomChannelIdInput, NULL, NULL);

    FuriState* intercom_state = intercom_get_state(intercom);
    furi_state_subscribe(intercom_state, input_intercom_state_callback, instance);

    for(size_t i = 0; i < input_pins_count; i++) {
        InputKeyState* state = &instance->key_states[i];

        if(state->level) {
            input_send(instance, instance->key_states[i].pin, InputActionPress);
        }
    }
}

static void input_send_to_intercom(Input* instance, const InputCommonEvent* event) {
    if(instance->intercom_status != IntercomStatusOk) {
        return;
    }

    size_t tx_size;
    do {
        tx_size = intercom_tx(
            instance->intercom_ch, event, sizeof(InputCommonEvent), INPUT_INTERCOM_TIMEOUT_MS);
    } while(tx_size != sizeof(InputCommonEvent));
}

#else
#define input_init_intercom(...)
#define input_send_to_intercom(...)
#endif // SRV_INTERCOM

static void input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Input* instance = context;
    furi_assert(object == instance->input_queue);

    InputCommonEvent event;
    furi_check(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk);

    furi_pubsub_publish(instance->event_pubsub, &event);

#ifdef INPUT_DEBUG

    if(event.device == InputDeviceButton) {
        const InputButton button = event.button_event.button;
        const InputAction action = event.button_event.action;

        const char* name = input_pins[button].name;
        const char* action_str = action == InputActionPress ? "press" : "release";

        INPUT_LOG("Key %s, event %s", name, action_str);

    } else if(event.device == InputDeviceEncoder) {
        INPUT_LOG("Encoder turn %d", event.encoder_delta);

    } else if(event.device == InputDeviceSwitch) {
        const InputSwitchPosition pos = event.switch_position;
        const char* name = input_pins[pos + InputButtonMAX].name;

        INPUT_LOG("Switch %s %d, event %s", name, pos, "press");

    } else {
        furi_crash();
    }

#endif // INPUT_DEBUG

    input_send_to_intercom(instance, &event);
}

int32_t input_srv(void* p) {
    UNUSED(p);

    INPUT_LOG("Starting");

    Input* instance = malloc(sizeof(Input));

    instance->event_pubsub = furi_pubsub_alloc();
    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_SIZE, sizeof(InputCommonEvent));
    instance->event_loop = furi_event_loop_alloc();
    instance->debounce_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        input_debounce_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);
    instance->key_states = malloc(sizeof(InputKeyState) * input_pins_count);
    instance->absolute_state = furi_state_alloc(sizeof(InputAbsoluteState));

    furi_record_create(RECORD_INPUT, instance);
    furi_record_create(RECORD_INPUT_EVENTS, instance->event_pubsub);

    for(size_t i = 0; i < input_pins_count; i++) {
        const InputPin* pin = &input_pins[i];
        InputKeyState* state = &instance->key_states[i];

        state->pin = pin;
        state->level = input_get_pin_level(pin);

        if(state->level) {
            input_send(instance, state->pin, InputActionPress);
        }

        furi_hal_gpio_add_int_callback(pin->gpio, pin->cond, input_isr_key, instance);
    }

    input_init_intercom(instance);

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

InputAbsoluteState input_get_absolute_state(Input* instance) {
    furi_assert(instance);

    InputAbsoluteState state;
    furi_state_get(instance->absolute_state, &state);

    return state;
}
