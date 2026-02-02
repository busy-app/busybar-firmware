#include "ble_command_engine.h"

struct BleCommandEngine {
    uint8_t commands_count;
    const BleCommandItem* commands;
    BleEngineCommandPreProcess pre_process;
    BleEngineCommandPostProcess post_process;
};

///TODO: Here we should make some factory which
//will return certain type of engine depending on System/Service parameter
//In that case we can do extern command lists here and not in upper layers
BleCommandEngine* ble_command_engine_alloc(
    const BleCommandItem* commands,
    uint8_t commands_count,
    BleEngineCommandPreProcess pre_process,
    BleEngineCommandPostProcess post_process) {
    furi_assert(commands);
    furi_assert(commands_count > 0);

    BleCommandEngine* instance = malloc(sizeof(BleCommandEngine));
    instance->commands = commands;
    instance->commands_count = commands_count;

    instance->pre_process = pre_process;
    instance->post_process = post_process;
    return instance;
}

///TODO: possibly frame should be extracted in pre_process step and not put from the outside
bool ble_command_engine_run(
    BleCommandEngine* instance,
    BleIntercomFrameGeneric* frame,
    void* context) {
    furi_assert(instance);
    furi_assert(frame);
    furi_assert(context);

    const BleIntercomFrameType frame_type = frame->header.frame_type;

    const BleCommandCode command = (BleCommandCode)frame->header.command;
    const BleCommandCode unknown_command = 0;
    furi_check(command != unknown_command);
    furi_check(command < instance->commands_count);
    const BleCommandItem* const item = &instance->commands[command];

    bool result = false;

    do {
        if(instance->pre_process) instance->pre_process(frame, context);

        if(frame_type == BleIntercomFrameTypeRequest && item->request) {
            result = item->request(frame, context);
        } else if(frame_type == BleIntercomFrameTypeResponse && item->response) {
            result = item->response(frame, context);
        } else {
            __furi_crash("Unknown frame");
        }

        if(instance->post_process) instance->post_process(frame, context);
    } while(false);

    return result;
}
