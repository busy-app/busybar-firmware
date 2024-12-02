#include "input.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <furi.h>

#define INPUT_DEBOUNCE_TICKS      4
#define INPUT_DEBOUNCE_TICKS_HALF (INPUT_DEBOUNCE_TICKS / 2)
#define INPUT_PRESS_TICKS         150
#define INPUT_LONG_PRESS_COUNTS   2
#define INPUT_THREAD_FLAG_ISR     0x00000001

/** Input pin state */
typedef struct {
    // State
    InputKey key;
    volatile bool state;
    volatile uint8_t debounce;
    FuriTimer* press_timer;
    volatile uint8_t press_counter;
    volatile uint32_t counter;
} InputPinState;

/** Input state */
typedef struct {
    FuriThreadId thread_id;
    FuriPubSub* event_pubsub;
    InputPinState* pin_states;
    volatile uint32_t counter;
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
    case InputKeyMAX:
        return "InputKeyMAX";
    }

    return "Unknown";
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
    case InputTypeMAX:
        return "InputTypeMAX";
    }

    return "Unknown";
}

static Input* input = NULL;

static uint8_t key_state = 0;

static bool key_state_read(uint8_t key) {
    return key_state & (1 << key);
}

void input_key_press(InputKey key) {
    uint8_t tmp_state = key_state;
    tmp_state |= 1 << key;

    if(tmp_state != key_state) {
        key_state = tmp_state;
        furi_thread_flags_set(input->thread_id, INPUT_THREAD_FLAG_ISR);
    }
}

void input_key_release(InputKey key) {
    uint8_t tmp_state = key_state;
    tmp_state &= ~(1 << key);

    if(tmp_state != key_state) {
        key_state = tmp_state;
        furi_thread_flags_set(input->thread_id, INPUT_THREAD_FLAG_ISR);
    }
}

static void input_press_timer_callback(void* arg) {
    InputPinState* input_pin = arg;
    InputEvent event;
    event.sequence_source = INPUT_SEQUENCE_SOURCE_HARDWARE;
    event.sequence_counter = input_pin->counter;
    event.key = input_pin->key;
    input_pin->press_counter++;
    if(input_pin->press_counter == INPUT_LONG_PRESS_COUNTS) {
        event.type = InputTypeLong;
        furi_pubsub_publish(input->event_pubsub, &event);
    } else if(input_pin->press_counter > INPUT_LONG_PRESS_COUNTS) {
        input_pin->press_counter--;
        event.type = InputTypeRepeat;
        furi_pubsub_publish(input->event_pubsub, &event);
    }
}

int32_t input_srv(void* p) {
    UNUSED(p);
    input = malloc(sizeof(Input));
    input->thread_id = furi_thread_get_current_id();
    input->event_pubsub = furi_pubsub_alloc();
    furi_record_create(RECORD_INPUT_EVENTS, input->event_pubsub);

#ifdef INPUT_DEBUG
    furi_hal_gpio_init_simple(&gpio_ext_pa4, GpioModeOutputPushPull);
#endif

#ifdef SRV_CLI
    // input->cli = furi_record_open(RECORD_CLI);
    // cli_add_command(input->cli, "input", CliCommandFlagParallelSafe, input_cli, input);
#endif

    input->pin_states = malloc(InputKeyMAX * sizeof(InputPinState));

    for(size_t i = 0; i < InputKeyMAX; i++) {
        // furi_hal_gpio_add_int_callback(input_pins[i].gpio, input_isr, NULL);
        // input->pin_states[i].pin = &input_pins[i];
        input->pin_states[i].key = i;
        input->pin_states[i].state = key_state_read(i);
        input->pin_states[i].debounce = INPUT_DEBOUNCE_TICKS_HALF;
        input->pin_states[i].press_timer = furi_timer_alloc(
            input_press_timer_callback, FuriTimerTypePeriodic, &input->pin_states[i]);
        input->pin_states[i].press_counter = 0;
    }

    while(1) {
        bool is_changing = false;
        for(size_t i = 0; i < InputKeyMAX; i++) {
            bool state = key_state_read(i);
            if(state) {
                if(input->pin_states[i].debounce < INPUT_DEBOUNCE_TICKS)
                    input->pin_states[i].debounce += 1;
            } else {
                if(input->pin_states[i].debounce > 0) input->pin_states[i].debounce -= 1;
            }

            if(input->pin_states[i].debounce > 0 &&
               input->pin_states[i].debounce < INPUT_DEBOUNCE_TICKS) {
                is_changing = true;
            } else if(input->pin_states[i].state != state) {
                input->pin_states[i].state = state;

                // Common state info
                InputEvent event;
                event.sequence_source = INPUT_SEQUENCE_SOURCE_HARDWARE;
                event.key = i;

                // Short / Long / Repeat timer routine
                if(state) {
                    input->counter++;
                    input->pin_states[i].counter = input->counter;
                    event.sequence_counter = input->pin_states[i].counter;
                    furi_timer_start(input->pin_states[i].press_timer, INPUT_PRESS_TICKS);
                } else {
                    event.sequence_counter = input->pin_states[i].counter;
                    furi_timer_stop(input->pin_states[i].press_timer);
                    while(furi_timer_is_running(input->pin_states[i].press_timer))
                        furi_delay_tick(1);
                    if(input->pin_states[i].press_counter < INPUT_LONG_PRESS_COUNTS) {
                        event.type = InputTypeShort;
                        furi_pubsub_publish(input->event_pubsub, &event);
                    }
                    input->pin_states[i].press_counter = 0;
                }

                // Send Press/Release event
                event.type = input->pin_states[i].state ? InputTypePress : InputTypeRelease;
                furi_pubsub_publish(input->event_pubsub, &event);
            }
        }

        if(is_changing) {
#ifdef INPUT_DEBUG
            furi_hal_gpio_write(&gpio_ext_pa4, 1);
#endif
            furi_delay_tick(1);
        } else {
#ifdef INPUT_DEBUG
            furi_hal_gpio_write(&gpio_ext_pa4, 0);
#endif
            furi_thread_flags_wait(INPUT_THREAD_FLAG_ISR, FuriFlagWaitAny, FuriWaitForever);
        }
    }

    return 0;
}
