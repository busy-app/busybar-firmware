#include "rpc_i.h"

#include <furi.h>
#include <input/input.h>

#define TAG "RpcInput"

#ifdef RPC_INPUT_DEBUG
#define RPC_INPUT_LOG_D(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define RPC_INPUT_LOG_D(...)
#endif

typedef struct {
    RpcSession* session;
} RpcSystemInput;

typedef void (*RpcSystemInputKeyFunc)(InputKey);

static const InputKey rpc_system_input_button_map[] = {
    [PB_Input_Button_OK] = InputKeyOk,
    [PB_Input_Button_BACK] = InputKeyBack,
    [PB_Input_Button_START] = InputKeyStart,
};

static void rpc_system_input_process_button_event(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    RpcSystemInput* instance = context;
    RpcSession* session = instance->session;
    furi_assert(session);

    const PB_Input_Button button = request->content.button_event.button;
    const PB_Input_ButtonAction action = request->content.button_event.action;

    const InputKey key = rpc_system_input_button_map[button];

    if(action == PB_Input_ButtonAction_PRESS) {
        input_key_press(key);
    } else {
        input_key_release(key);
    }

    RPC_INPUT_LOG_D(TAG, "Button event: button %d, action %d", button, action);
}

static void rpc_system_input_process_switch_event(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    RpcSystemInput* instance = context;
    RpcSession* session = instance->session;
    furi_assert(session);

    const PB_Input_SwitchPosition position = request->content.switch_event.position;
    input_key_toggle(InputKeyBusy + position);

    RPC_INPUT_LOG_D(TAG, "Switch event: position %d", position);
}

static void rpc_system_input_process_encoder_event(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    RpcSystemInput* instance = context;
    RpcSession* session = instance->session;
    furi_assert(session);

    const int16_t delta = request->content.encoder_event.delta;

    if(delta > 0) {
        input_key_toggle(InputKeyUp);
    } else if(delta < 0) {
        input_key_toggle(InputKeyDown);
    } else {
        furi_crash();
    }

    RPC_INPUT_LOG_D(TAG, "Encoder event: delta %d", delta);
}

void* rpc_system_input_alloc(RpcSession* session) {
    furi_assert(session);

    RpcSystemInput* instance = malloc(sizeof(RpcSystemInput));
    instance->session = session;

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = instance,
    };

    rpc_handler.message_handler = rpc_system_input_process_button_event;
    rpc_add_handler(session, PB_Main_button_event_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_input_process_switch_event;
    rpc_add_handler(session, PB_Main_switch_event_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_input_process_encoder_event;
    rpc_add_handler(session, PB_Main_encoder_event_tag, &rpc_handler);

    return instance;
}

void rpc_system_input_free(void* context) {
    RpcSystemInput* instance = context;
    free(instance);
}
