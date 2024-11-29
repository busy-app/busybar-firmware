#include "input.h"

#include <furi.h>
#include <furi_hal_qei.h>
#include <furi_hal_resources.h>

#include <rpc/rpc_i.h>
#include <intercom/intercom_rpc.h>

#include <main.pb.h>

#define TAG "Input"

#define GPIO_Read(input_pin) (furi_hal_gpio_read(input_pin.pin->gpio) ^ (input_pin.pin->inverted))

#define INPUT_DEBOUNCE_TIMER_TICKS 1 //ms
#define INPUT_QUEUE_SIZE           15

#ifdef INPUT_DEBUG
#define INPUT_LOG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define INPUT_LOG(...)
#endif

typedef enum {
    InputSrvKeyEvent = 1 << 0,
} InputSrvEvent;

typedef struct {
    const InputPin* pin;
    uint16_t debounce_count;
    bool state;
} InputSrvKeyState;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriEventLoopTimer* debounce_timer;
    InputSrvKeyState* key_state;
    RpcSession* intercom_session;
} InputSrv;

static void input_isr_key(void* context) {
    InputSrv* instance = context;
    furi_event_loop_set_custom_event(instance->event_loop, InputSrvKeyEvent);
}

static void input_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    InputSrv* instance = context;

    if(events & InputSrvKeyEvent) {
        if(!furi_event_loop_timer_is_running(instance->debounce_timer)) {
            furi_event_loop_timer_start(instance->debounce_timer, INPUT_DEBOUNCE_TIMER_TICKS);
        }
    }
}

static void input_send(InputSrv* instance, uint32_t num_pin, InputType input_type) {
    InputEvent event;

    event.key = instance->key_state[num_pin].pin->key;

    if((instance->key_state[num_pin].pin->key == InputKeySwitch)) {
        if(input_type == InputTypePress) {
            event.type = InputTypeSwitch;
            event.position = instance->key_state[num_pin].pin->switch_position;
            furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
        }
    } else {
        event.type = input_type;
        furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
    }
}

static void input_debounce_timer_callback(void* context) {
    furi_assert(context);
    InputSrv* instance = context;
    bool is_changing = false;

    for(size_t i = 0; i < input_pins_count; i++) {
        bool state = GPIO_Read(instance->key_state[i]);

        if(state) {
            if(instance->key_state[i].debounce_count < INPUT_DEBOUNCE_TICKS) {
                instance->key_state[i].debounce_count++;
                is_changing = true;
            }
        } else if(instance->key_state[i].debounce_count > 0) {
            instance->key_state[i].debounce_count--;
            is_changing = true;
        }

        if(!is_changing && instance->key_state[i].state != state) {
            instance->key_state[i].state = state;

            if(state) {
                input_send(instance, i, InputTypePress);
            } else {
                input_send(instance, i, InputTypeRelease);
            }
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
        .type = InputTypeTurn,
        .delta = delta_pos,
    };
    furi_check(furi_message_queue_put(instance->input_queue, &event, 0) == FuriStatusOk);
}

static void input_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    InputSrv* instance = context;
    furi_assert(object == instance->input_queue);

    InputEvent event;
    furi_check(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk);

    PB_Main msg = {0};

    if(event.type == InputTypeTurn) {
        msg.which_content = PB_Main_encoder_event_tag;
        msg.content.encoder_event.delta = event.delta;

        INPUT_LOG("Encoder turn %d", event.delta);

    } else if(event.type == InputTypeSwitch) {
        msg.which_content = PB_Main_switch_event_tag;
        msg.content.switch_event.position = (PB_Input_SwitchPosition)event.position;

        INPUT_LOG(
            "Switch %s %d, event %s",
            input_pins[event.position + InputKeySwitch].name,
            event.position,
            "press");

    } else {
        msg.which_content = PB_Main_button_event_tag;
        msg.content.button_event.button = (PB_Input_Button)event.key;
        msg.content.button_event.action = (PB_Input_ButtonAction)event.type;

        INPUT_LOG(
            "Key %s, event %s",
            input_pins[event.key].name,
            event.type == InputTypePress ? "press" : "release");
    }

    rpc_send_and_release(instance->intercom_session, &msg);
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

    instance->key_state = malloc(sizeof(InputSrvKeyState) * input_pins_count);
    instance->intercom_session = furi_record_open(RECORD_INTERCOM_RPC);

    for(size_t i = 0; i < input_pins_count; i++) {
        furi_hal_gpio_add_int_callback(
            input_pins[i].gpio, input_pins[i].condition, input_isr_key, instance);
        instance->key_state[i].pin = &input_pins[i];
        instance->key_state[i].state = GPIO_Read(instance->key_state[i]);
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
