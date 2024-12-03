#include "input.h"

#include <furi.h>

#define INPUT_PRESS_TICKS       150
#define INPUT_LONG_PRESS_COUNTS 2

#define INPUT_KEY_PRESS(key)   (1UL << key)
#define INPUT_KEY_RELEASE(key) (1UL << (key + InputKeyMAX))

/** Input pin state */
typedef struct {
    FuriEventLoopTimer* press_timer;
    volatile uint32_t sequence;
    InputKey key;
    volatile uint8_t press_counter;
} InputPinState;

/** Input state */
typedef struct {
    FuriPubSub* event_pubsub;
    FuriEventLoop* event_loop;
    InputPinState* pin_states;
    volatile uint32_t sequence;
} Input;

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

static Input* input = NULL;

void input_key_press(InputKey key) {
    furi_event_loop_set_custom_event(input->event_loop, INPUT_KEY_PRESS(key));
}

void input_key_release(InputKey key) {
    furi_event_loop_set_custom_event(input->event_loop, INPUT_KEY_RELEASE(key));
}

void input_key_toggle(InputKey key) {
    furi_event_loop_set_custom_event(
        input->event_loop, INPUT_KEY_PRESS(key) | INPUT_KEY_RELEASE(key));
}

static void input_press_timer_callback(void* context) {
    InputPinState* state = context;

    InputEvent event;

    event.sequence_source = INPUT_SEQUENCE_SOURCE_HARDWARE;
    event.sequence_counter = state->sequence;
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

            event.sequence_counter = state->sequence;
            event.type = InputTypePress;

            furi_event_loop_timer_start(state->press_timer, INPUT_PRESS_TICKS);

            furi_pubsub_publish(input->event_pubsub, &event);
        }

        if(events & INPUT_KEY_RELEASE(i)) {
            event.sequence_counter = state->sequence;

            furi_event_loop_timer_stop(state->press_timer);

            if(state->press_counter < INPUT_LONG_PRESS_COUNTS) {
                event.type = InputTypeShort;
                furi_pubsub_publish(input->event_pubsub, &event);
            }

            state->press_counter = 0;
            event.type = InputTypeRelease;

            furi_pubsub_publish(input->event_pubsub, &event);
        }
    }
}

int32_t input_srv(void* p) {
    UNUSED(p);

    input = malloc(sizeof(Input));
    input->event_pubsub = furi_pubsub_alloc();
    input->event_loop = furi_event_loop_alloc();

    furi_record_create(RECORD_INPUT_EVENTS, input->event_pubsub);

#ifdef SRV_CLI
#if 0
    input->cli = furi_record_open(RECORD_CLI);
    cli_add_command(input->cli, "input", CliCommandFlagParallelSafe, input_cli, input);
#endif
#endif

    input->pin_states = malloc(sizeof(InputPinState) * InputKeyMAX);

    for(size_t i = 0; i < InputKeyMAX; i++) {
        InputPinState* state = &input->pin_states[i];

        state->key = i;
        state->press_timer = furi_event_loop_timer_alloc(
            input->event_loop, input_press_timer_callback, FuriEventLoopTimerTypePeriodic, state);
        state->press_counter = 0;
    }

    furi_event_loop_set_custom_event_callback(
        input->event_loop, input_custom_event_callback, input);

    furi_event_loop_run(input->event_loop);

    return 0;
}
