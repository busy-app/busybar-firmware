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

typedef enum {
    InputRequestTypeGetAbsState,
} InputRequestType;

typedef struct {
    InputRequestType type;
    FuriApiLock lock;
    union {
        InputAbsoluteState* abs_state;
    };
} InputRequest;

struct Input {
    FuriPubSub* event_pubsub;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* debounce_timer;
    InputKeyState* key_states;
#ifdef SRV_INTERCOM
    Intercom* intercom_srv;
    IntercomChannel* intercom;
#endif
    FuriMessageQueue* request_queue;
    InputAbsoluteState absolute_state;
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

static void input_request_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);
    furi_assert(context);

    Input* instance = context;

    InputRequest request;
    furi_check(furi_message_queue_get(instance->request_queue, &request, 0) == FuriStatusOk);

    switch(request.type) {
    case InputRequestTypeGetAbsState:
        *request.abs_state = instance->absolute_state;
        break;
    default:
        furi_crash();
    }

    if(request.lock) {
        api_lock_unlock(request.lock);
    }
}

static void input_send(Input* instance, const InputPin* pin, InputAction input_action) {
    InputCommonEvent event;
    event.device = pin->device;

    if(pin->device == InputDeviceSwitch) {
        if(input_action == InputActionPress) {
            event.switch_position = pin->pos;
            instance->absolute_state.switch_position = pin->pos;
            furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
        }
    } else {
        event.button_event.button = pin->button;
        event.button_event.action = input_action;

        if(input_action == InputActionPress) {
            instance->absolute_state.buttons |= (1 << pin->button);
        } else {
            instance->absolute_state.buttons &= ~(1 << pin->button);
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

#endif

#ifdef SRV_INTERCOM
    while(intercom_is_in_sync(instance->intercom_srv)) {
        size_t sent_size = intercom_tx(
            instance->intercom,
            &event,
            sizeof(InputCommonEvent),
            furi_ms_to_ticks(INPUT_INTERCOM_TIMEOUT_MS));

        if(sent_size == sizeof(InputCommonEvent)) {
            break;
        }
    }
#endif
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
    instance->request_queue = furi_message_queue_alloc(REQUEST_QUEUE_SIZE, sizeof(InputRequest));

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

#ifdef SRV_INTERCOM
    instance->intercom_srv = furi_record_open(RECORD_INTERCOM);
    instance->intercom = intercom_channel_open(
        instance->intercom_srv, IntercomChannelIdInput, FuriWaitForever, NULL, NULL);
    for(size_t i = 0; i < input_pins_count; i++) {
        InputKeyState* state = &instance->key_states[i];

        if(state->level) {
            input_send(instance, instance->key_states[i].pin, InputActionPress);
        }
    }
#endif

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, input_custom_event_callback, instance);
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        input_queue_callback,
        instance);
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->request_queue,
        FuriEventLoopEventIn,
        input_request_callback,
        instance);

    furi_record_create(RECORD_INPUT, instance);
    furi_record_create(RECORD_INPUT_EVENTS, instance->event_pubsub);

    furi_hal_qei_init();
    furi_hal_qei_set_delta_pos_callback(input_qei_callback, instance);

    // Start Input Service
    furi_event_loop_run(instance->event_loop);

    return 0;
}

InputAbsoluteState input_get_absolute_state(Input* instance) {
    furi_assert(instance);

    InputAbsoluteState abs_state;
    InputRequest request = {
        .type = InputRequestTypeGetAbsState,
        .lock = api_lock_alloc_locked(),
        .abs_state = &abs_state,
    };

    furi_check(
        furi_message_queue_put(instance->request_queue, &request, FuriWaitForever) ==
        FuriStatusOk);
    api_lock_wait_unlock_and_free(request.lock);

    return *request.abs_state;
}
