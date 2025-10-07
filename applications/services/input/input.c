#include "input.h"
#include "input_common_f64.h"

#include <furi.h>
#include <toolbox/api_lock.h>

#ifdef SRV_INTERCOM
#include <intercom/intercom.h>
#endif

#define INPUT_PRESS_TICKS       150
#define INPUT_LONG_PRESS_COUNTS 2

#define INPUT_KEY_PRESS(key)   (1UL << key)
#define INPUT_KEY_RELEASE(key) (1UL << (key + InputKeyMAX))

#define INPUT_REQUEST_QUEUE_SIZE 4

/** Input pin state */
typedef struct {
    FuriEventLoopTimer* press_timer;
    volatile uint32_t sequence;
    InputKey key;
    volatile uint8_t press_counter;
    Input* input;
} InputPinState;

/** Input state */
struct Input {
    FuriPubSub* event_pubsub;
    FuriEventLoop* event_loop;
    FuriMessageQueue* request_queue;
    InputPinState* pin_states;
    volatile uint32_t sequence;
    InputAbsoluteState absolute_state;
};

typedef enum {
    InputRequestTypeGetAbsState,
} InputRequestType;

typedef struct {
    InputRequestType type;
    FuriApiLock lock;
    union {
        InputAbsoluteState abs_state;
    };
} InputRequest;

const char* input_get_key_name(InputKey key) {
    switch(key) {
    case InputKeyUp:
        return "InputKeyUp";
    case InputKeyDown:
        return "InputKeyDown";
    case InputKeyRight:
        return "InputKeyRight";
    case InputKeyLeft:
        return "InputKeyLeft";
    case InputKeyOk:
        return "InputKeyOk";
    case InputKeyBack:
        return "InputKeyBack";
    case InputKeyStart:
        return "InputKeyStart";
    case InputKeyBusy:
        return "InputKeyBusy";
    case InputKeyStatus:
        return "InputKeyStatus";
    case InputKeyOff:
        return "InputKeyOff";
    case InputKeyApps:
        return "InputKeyApps";
    case InputKeySettings:
        return "InputKeySettings";
    default:
        furi_crash();
    }
}

const char* input_get_type_name(InputType type) {
    switch(type) {
    case InputTypePress:
        return "InputTypePress";
    case InputTypeRelease:
        return "InputTypeRelease";
    case InputTypeShort:
        return "InputTypeShort";
    case InputTypeLong:
        return "InputTypeLong";
    case InputTypeRepeat:
        return "InputTypeRepeat";
    default:
        furi_crash();
    }
}

#define INPUT_BUTTON_RANGE_START InputKeyOk
#define INPUT_BUTTON_RANGE_END   InputKeyStart
#define INPUT_SWITCH_RANGE_START InputKeyBusy
#define INPUT_SWITCH_RANGE_END   InputKeySettings

static void input_update_absolute_state(Input* input, InputEvent* event) {
    InputKey key = event->key;
    InputType type = event->type;
    furi_check(type == InputTypePress || type == InputTypeRelease);

    if(key >= INPUT_BUTTON_RANGE_START && key <= INPUT_BUTTON_RANGE_END) {
        size_t offset = key - INPUT_BUTTON_RANGE_START;
        InputButtonMask* buttons = &input->absolute_state.buttons;
        size_t button_value = type == InputTypePress;
        *buttons = (*buttons & ~(1 << offset)) | (button_value << offset);

    } else if(key >= INPUT_SWITCH_RANGE_START && key <= INPUT_SWITCH_RANGE_END) {
        if(type == InputTypePress) {
            input->absolute_state.switch_position = key - INPUT_SWITCH_RANGE_START;
        }

    } else {
        // not an absolute control
    }
}

static void input_press_timer_callback(void* context) {
    InputPinState* state = context;
    Input* input = state->input;

    InputEvent event;

    event.sequence_source = INPUT_SEQUENCE_SOURCE_HARDWARE;
    event.sequence_number = state->sequence;
    event.key = state->key;

    state->press_counter++;

    if(state->press_counter == INPUT_LONG_PRESS_COUNTS) {
        event.type = InputTypeLong;
        furi_pubsub_publish(input->event_pubsub, &event);

    } else if(state->press_counter > INPUT_LONG_PRESS_COUNTS) {
        state->press_counter--;
        event.type = InputTypeRepeat;
        furi_pubsub_publish(input->event_pubsub, &event);
    }
}

static void input_custom_event_callback(uint32_t events, void* context) {
    Input* input = context;

    for(size_t i = 0; i < InputKeyMAX; i++) {
        InputPinState* state = &input->pin_states[i];

        InputEvent event;
        event.sequence_source = INPUT_SEQUENCE_SOURCE_HARDWARE;
        event.key = i;

        if(events & INPUT_KEY_PRESS(i)) {
            input->sequence++;

            state->sequence = input->sequence;

            event.sequence_number = state->sequence;
            event.type = InputTypePress;

            furi_event_loop_timer_start(state->press_timer, INPUT_PRESS_TICKS);

            input_update_absolute_state(input, &event);
            furi_pubsub_publish(input->event_pubsub, &event);
        }

        if(events & INPUT_KEY_RELEASE(i)) {
            event.sequence_number = state->sequence;

            furi_event_loop_timer_stop(state->press_timer);

            if(state->press_counter < INPUT_LONG_PRESS_COUNTS) {
                event.type = InputTypeShort;
                furi_pubsub_publish(input->event_pubsub, &event);
            }

            state->press_counter = 0;
            event.type = InputTypeRelease;

            input_update_absolute_state(input, &event);
            furi_pubsub_publish(input->event_pubsub, &event);
        }
    }
}

#ifdef SRV_INTERCOM
static void input_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    furi_assert(data_size == sizeof(InputCommonEvent));

    Input* input = context;
    const InputCommonEvent* event = data;

    if(event->device == InputDeviceButton) {
        const InputKey key = event->button_event.button + InputKeyOk;

        if(event->button_event.action == InputActionPress) {
            input_key_press(input, key);
        } else {
            input_key_release(input, key);
        }

    } else if(event->device == InputDeviceSwitch) {
        const InputKey key = event->switch_position + InputKeyBusy;
        input_key_toggle(input, key);

    } else if(event->device == InputDeviceEncoder) {
        const InputKey key = event->encoder_delta > 0 ? InputKeyUp : InputKeyDown;
        input_key_toggle(input, key);
    }
}
#endif

static void input_request_handler(FuriEventLoopObject* object, void* context) {
    FuriMessageQueue* queue = object;
    Input* input = context;

    InputRequest* request;
    furi_check(furi_message_queue_get(queue, &request, 0) == FuriStatusOk);

    if(request->type == InputRequestTypeGetAbsState) {
        request->abs_state = input->absolute_state;

    } else {
        furi_crash();
    }

    api_lock_unlock(request->lock);
}

int32_t input_srv(void* p) {
    UNUSED(p);

    Input* input = malloc(sizeof(Input));
    input->event_pubsub = furi_pubsub_alloc();
    input->event_loop = furi_event_loop_alloc();

    input->request_queue =
        furi_message_queue_alloc(INPUT_REQUEST_QUEUE_SIZE, sizeof(InputRequest*));
    furi_event_loop_subscribe_message_queue(
        input->event_loop,
        input->request_queue,
        FuriEventLoopEventIn,
        input_request_handler,
        input);

    furi_record_create(RECORD_INPUT_EVENTS, input->event_pubsub);

    input->pin_states = malloc(sizeof(InputPinState) * InputKeyMAX);

    for(size_t i = 0; i < InputKeyMAX; i++) {
        InputPinState* state = &input->pin_states[i];

        state->input = input;
        state->key = i;
        state->press_timer = furi_event_loop_timer_alloc(
            input->event_loop, input_press_timer_callback, FuriEventLoopTimerTypePeriodic, state);
        state->press_counter = 0;
    }

    furi_event_loop_set_custom_event_callback(
        input->event_loop, input_custom_event_callback, input);

#ifdef SRV_INTERCOM
    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_rx_callback(intercom, IntercomChannelInput, input_intercom_rx_callback, input);
#endif

    furi_record_create(RECORD_INPUT, input);

    furi_event_loop_run(input->event_loop);

    return 0;
}

// ==========
// Public API
// ==========

void input_key_press(Input* input, InputKey key) {
    furi_check(input);
    furi_event_loop_set_custom_event(input->event_loop, INPUT_KEY_PRESS(key));
}

void input_key_release(Input* input, InputKey key) {
    furi_check(input);
    furi_event_loop_set_custom_event(input->event_loop, INPUT_KEY_RELEASE(key));
}

void input_key_toggle(Input* input, InputKey key) {
    furi_check(input);
    furi_event_loop_set_custom_event(
        input->event_loop, INPUT_KEY_PRESS(key) | INPUT_KEY_RELEASE(key));
}

// TODO: generalize this synchronous request pattern in a common lib
static void input_synchronous_request(Input* input, InputRequest* request) {
    furi_check(input);
    request->lock = api_lock_alloc_locked();
    furi_check(
        furi_message_queue_put(input->request_queue, &request, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(request->lock);
}

InputAbsoluteState input_get_absolute_state(Input* input) {
    InputRequest request = {
        .type = InputRequestTypeGetAbsState,
    };
    input_synchronous_request(input, &request);
    return request.abs_state;
}
